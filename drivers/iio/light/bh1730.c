// SPDX-License-Identifier: GPL-2.0-only
/*
 * ROHM BH1730FVC ambient light sensor
 *
 * Two ADC channels behind one optical stack: DATA0 sees visible plus IR,
 * DATA1 mostly IR. Lux is a weighted DATA1 subtracted from DATA0, the
 * weights picked by their ratio. All three are exported.
 *
 * Not the BH1780GLI of bh1780.c: same vendor, different register map.
 *
 * Datasheet: BH1730FVC, TSZ22111-15-001 Rev.001, 8 Jun 2016
 */

#include <linux/bitfield.h>
#include <linux/cleanup.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/math64.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pm.h>
#include <linux/unaligned.h>
#include <linux/units.h>

#include <linux/iio/iio.h>

/*
 * Every access carries the command bit; the low five bits are the
 * register address and the part auto-increments from there.
 */
#define BH1730_CMD			BIT(7)

#define BH1730_REG_CONTROL		0x00
#define   BH1730_CTL_ADC_VALID		BIT(4)
#define   BH1730_CTL_ONE_TIME		BIT(3)
#define   BH1730_CTL_ADC_EN		BIT(1)
#define   BH1730_CTL_POWER		BIT(0)
#define BH1730_REG_TIMING		0x01
#define BH1730_REG_GAIN			0x07
#define   BH1730_GAIN_MASK		GENMASK(2, 0)
#define BH1730_REG_ID			0x12
#define   BH1730_ID_PART_MASK		GENMASK(7, 4)
#define   BH1730_ID_PART_BH1730		0x7
#define BH1730_REG_DATA0		0x14

/*
 * ITIME_ms = Tint * 964 * (256 - ITIME), Tint 2.8us, so one step is
 * 2699.2us. Register 0 is not an integration time, hence the range.
 */
#define BH1730_ITIME_STEP_NS		2699200
#define BH1730_ITIME_REG_MIN		1
#define BH1730_ITIME_REG_MAX		255
#define BH1730_ITIME_REG_DEFAULT	0xda	/* 102.6ms, the datasheet's reference */

/* Tmt = ITIME_ms + Tint * 714; the tail is 2ms, rounded up. */
#define BH1730_MEASURE_TAIL_US		2000

/*
 * Lx = weighted * 102.6 / (gain * itime_us); micro-lux is that times 1e6.
 */
#define BH1730_LUX_SCALE		102600000LL

struct bh1730_data {
	struct i2c_client *client;
	struct mutex lock;	/* serialises settings against a conversion */
	unsigned int itime_us;
	int gain;
	u8 itime_reg;
	u8 gain_reg;
};

static const int bh1730_gains[] = { 1, 2, 64, 128 };

static int bh1730_read_u8(struct bh1730_data *data, u8 reg)
{
	return i2c_smbus_read_byte_data(data->client, BH1730_CMD | reg);
}

static int bh1730_write_u8(struct bh1730_data *data, u8 reg, u8 val)
{
	return i2c_smbus_write_byte_data(data->client, BH1730_CMD | reg, val);
}

static unsigned int bh1730_itime_us(u8 reg)
{
	return DIV_ROUND_CLOSEST(BH1730_ITIME_STEP_NS * (256 - reg), 1000);
}

/*
 * A conversion runs whenever the part is powered, so the newest result
 * is at most one period old. ADC_VALID says whether it completed.
 */
static int bh1730_wait_for_sample(struct bh1730_data *data)
{
	unsigned int period = data->itime_us + BH1730_MEASURE_TAIL_US;
	int i, ret;

	for (i = 0; i < 2; i++) {
		ret = bh1730_read_u8(data, BH1730_REG_CONTROL);
		if (ret < 0)
			return ret;
		if (ret & BH1730_CTL_ADC_VALID)
			return 0;

		fsleep(period);
	}

	return -ETIMEDOUT;
}

static int bh1730_read_channels(struct bh1730_data *data, u16 *visible, u16 *ir)
{
	u8 buf[4];
	int ret;

	ret = bh1730_wait_for_sample(data);
	if (ret)
		return ret;

	/* One transaction, so the two channels come from one conversion. */
	ret = i2c_smbus_read_i2c_block_data(data->client,
					    BH1730_CMD | BH1730_REG_DATA0,
					    sizeof(buf), buf);
	if (ret < 0)
		return ret;
	if (ret != sizeof(buf))
		return -EIO;

	*visible = get_unaligned_le16(&buf[0]);
	*ir = get_unaligned_le16(&buf[2]);

	return 0;
}

/*
 * Datasheet section 6, weights scaled by 1000:
 *
 * 	Lx = (k0 * DATA0 - k1 * DATA1) / Gain * 102.6 / ITIME_ms
 *
 * ITIME divides: longer integration collects proportionally more counts,
 * and 102.6 is the time the weights were measured at. The vendor
 * reference code (from zephyr rtos) has this inverted.
 */
static s64 bh1730_to_lux_micro(struct bh1730_data *data, u16 visible, u16 ir)
{
	static const struct {
		unsigned int ratio;	/* upper bound, scaled by 1000 */
		unsigned int k0;
		unsigned int k1;
	} coeff[] = {
		{  260, 1290, 2733 },
		{  550,  795,  859 },
		{ 1090,  510,  345 },
		{ 2130,  276,  130 },
	};
	unsigned int ratio;
	s64 weighted;
	size_t i;

	if (!visible)
		return 0;

	ratio = DIV_ROUND_CLOSEST(ir * 1000U, visible);

	for (i = 0; i < ARRAY_SIZE(coeff); i++) {
		if (ratio >= coeff[i].ratio)
			continue;

		weighted = (s64)visible * coeff[i].k0 - (s64)ir * coeff[i].k1;
		if (weighted <= 0)
			break;

		return div_s64(weighted * BH1730_LUX_SCALE,
			       (s64)data->gain * data->itime_us);
	}

	/* Beyond the last ratio the part cannot tell light from noise. */
	return 0;
}

static const struct iio_chan_spec bh1730_channels[] = {
	{
		.type = IIO_LIGHT,
		.info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_INT_TIME) |
					   BIT(IIO_CHAN_INFO_HARDWAREGAIN),
		.info_mask_shared_by_all_available =
			BIT(IIO_CHAN_INFO_INT_TIME) |
			BIT(IIO_CHAN_INFO_HARDWAREGAIN),
	},
	{
		.type = IIO_INTENSITY,
		.modified = 1,
		.channel2 = IIO_MOD_LIGHT_CLEAR,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_INTENSITY,
		.modified = 1,
		.channel2 = IIO_MOD_LIGHT_IR,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
};

static int bh1730_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2, long mask)
{
	struct bh1730_data *data = iio_priv(indio_dev);
	u16 visible, ir;
	int ret;


	guard(mutex)(&data->lock);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = bh1730_read_channels(data, &visible, &ir);
		if (ret)
			return ret;

		*val = chan->channel2 == IIO_MOD_LIGHT_IR ? ir : visible;
		return IIO_VAL_INT;

	case IIO_CHAN_INFO_PROCESSED:
		ret = bh1730_read_channels(data, &visible, &ir);
		if (ret)
			return ret;

		*val = div_s64_rem(bh1730_to_lux_micro(data, visible, ir),
				   MICRO, val2);
		return IIO_VAL_INT_PLUS_MICRO;

	case IIO_CHAN_INFO_INT_TIME:
		*val = 0;
		*val2 = data->itime_us;
		return IIO_VAL_INT_PLUS_MICRO;

	case IIO_CHAN_INFO_HARDWAREGAIN:
		*val = data->gain;
		return IIO_VAL_INT;

	default:
		return -EINVAL;
	}
}

static int bh1730_set_int_time(struct bh1730_data *data, int val, int val2)
{
	unsigned int us, steps;
	int ret;
	u8 reg;

	if (val != 0 || val2 < 0)
		return -EINVAL;

	us = val2;
	steps = DIV_ROUND_CLOSEST(us * 1000U, BH1730_ITIME_STEP_NS);
	if (steps < 256 - BH1730_ITIME_REG_MAX || steps > 256 - BH1730_ITIME_REG_MIN)
		return -EINVAL;

	reg = 256 - steps;

	ret = bh1730_write_u8(data, BH1730_REG_TIMING, reg);
	if (ret < 0)
		return ret;

	data->itime_reg = reg;
	data->itime_us = bh1730_itime_us(reg);

	return 0;
}

static int bh1730_set_gain(struct bh1730_data *data, int val)
{
	size_t i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(bh1730_gains); i++) {
		if (bh1730_gains[i] != val)
			continue;

		ret = bh1730_write_u8(data, BH1730_REG_GAIN,
				      FIELD_PREP(BH1730_GAIN_MASK, i));
		if (ret < 0)
			return ret;

		data->gain_reg = i;
		data->gain = bh1730_gains[i];

		return 0;
	}

	return -EINVAL;
}

static int bh1730_write_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int val, int val2, long mask)
{
	struct bh1730_data *data = iio_priv(indio_dev);

	guard(mutex)(&data->lock);

	switch (mask) {
	case IIO_CHAN_INFO_INT_TIME:
		return bh1730_set_int_time(data, val, val2);
	case IIO_CHAN_INFO_HARDWAREGAIN:
		return bh1730_set_gain(data, val);
	default:
		return -EINVAL;
	}
}

static int bh1730_read_avail(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     const int **vals, int *type, int *length,
			     long mask)
{
	/* Whole microseconds; the step is 2699.2us, so this rounds. */
	static const int int_time_range[] = {
		0, 2699,	/* minimum: one step */
		0, 2699,	/* step */
		0, 688296,	/* maximum: 255 steps */
	};

	switch (mask) {
	case IIO_CHAN_INFO_INT_TIME:
		*vals = int_time_range;
		*type = IIO_VAL_INT_PLUS_MICRO;
		*length = ARRAY_SIZE(int_time_range);
		return IIO_AVAIL_RANGE;

	case IIO_CHAN_INFO_HARDWAREGAIN:
		*vals = bh1730_gains;
		*type = IIO_VAL_INT;
		*length = ARRAY_SIZE(bh1730_gains);
		return IIO_AVAIL_LIST;

	default:
		return -EINVAL;
	}
}

static const struct iio_info bh1730_info = {
	.read_raw = bh1730_read_raw,
	.write_raw = bh1730_write_raw,
	.read_avail = bh1730_read_avail,
};

static int bh1730_power(struct bh1730_data *data, bool on)
{
	u8 ctl = on ? BH1730_CTL_POWER | BH1730_CTL_ADC_EN : 0;

	return bh1730_write_u8(data, BH1730_REG_CONTROL, ctl);
}

static void bh1730_power_off(void *priv)
{
	bh1730_power(priv, false);
}

static int bh1730_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct bh1730_data *data;
	struct iio_dev *indio_dev;
	int ret;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA |
				     I2C_FUNC_SMBUS_I2C_BLOCK))
		return -EOPNOTSUPP;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	data->client = client;
	i2c_set_clientdata(client, indio_dev);

	ret = devm_mutex_init(dev, &data->lock);
	if (ret)
		return ret;

	/*
	 * Identify here so a missing or wrong chip fails at probe.
	 */
	ret = bh1730_read_u8(data, BH1730_REG_ID);
	if (ret < 0)
		return dev_err_probe(dev, ret, "failed to read part id\n");

	if (FIELD_GET(BH1730_ID_PART_MASK, ret) != BH1730_ID_PART_BH1730)
		return dev_err_probe(dev, -ENODEV,
				     "not a BH1730FVC, part id %#02x\n", ret);

	data->itime_reg = BH1730_ITIME_REG_DEFAULT;
	data->itime_us = bh1730_itime_us(data->itime_reg);
	data->gain_reg = 0;
	data->gain = bh1730_gains[0];

	ret = bh1730_write_u8(data, BH1730_REG_TIMING, data->itime_reg);
	if (ret < 0)
		return dev_err_probe(dev, ret, "failed to set integration time\n");

	ret = bh1730_write_u8(data, BH1730_REG_GAIN, data->gain_reg);
	if (ret < 0)
		return dev_err_probe(dev, ret, "failed to set gain\n");

	ret = bh1730_power(data, true);
	if (ret < 0)
		return dev_err_probe(dev, ret, "failed to power up\n");

	ret = devm_add_action_or_reset(dev, bh1730_power_off, data);
	if (ret)
		return ret;

	indio_dev->name = "bh1730";
	indio_dev->info = &bh1730_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = bh1730_channels;
	indio_dev->num_channels = ARRAY_SIZE(bh1730_channels);

	return devm_iio_device_register(dev, indio_dev);
}

static int bh1730_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));

	return bh1730_power(iio_priv(indio_dev), false);
}

static int bh1730_resume(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));

	return bh1730_power(iio_priv(indio_dev), true);
}

static DEFINE_SIMPLE_DEV_PM_OPS(bh1730_pm_ops, bh1730_suspend, bh1730_resume);

static const struct i2c_device_id bh1730_id[] = {
	{ "bh1730" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, bh1730_id);

static const struct of_device_id bh1730_of_match[] = {
	{ .compatible = "rohm,bh1730fvc" },
	{ }
};
MODULE_DEVICE_TABLE(of, bh1730_of_match);

static struct i2c_driver bh1730_driver = {
	.driver = {
		.name = "bh1730",
		.of_match_table = bh1730_of_match,
		.pm = pm_sleep_ptr(&bh1730_pm_ops),
	},
	.probe = bh1730_probe,
	.id_table = bh1730_id,
};
module_i2c_driver(bh1730_driver);

MODULE_DESCRIPTION("ROHM BH1730FVC ambient light sensor driver");
MODULE_LICENSE("GPL");
