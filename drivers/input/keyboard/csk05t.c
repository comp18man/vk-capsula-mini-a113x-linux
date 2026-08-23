// SPDX-License-Identifier: GPL-2.0-only
/*
 * Hynitron CSK05T capacitive key controller
 *
 * Five self-capacitance channels behind an I2C register file. The part
 * scans on its own and keeps the key bitmap in a register.
 *
 * A register read is not a combined transaction: address written, bus
 * released with a STOP, read at least 30us later. A repeated START does
 * not work, which rules out regmap-i2c.
 *
 * Configuration writes are read back and the init retried on mismatch,
 * and the scan register is rechecked on every poll: the part can come up
 * unconfigured or lose its settings.
 *
 * No datasheet available. Registers and init sequence come from the only
 * published description https://blog.csdn.net/qq_62015338/article/details/143771432.
 */
#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/unaligned.h>

#define CSK05T_REG_CHANNEL_EN		0x14
#define CSK05T_REG_SCAN_PARAMS		0x18	/* seven bytes, to 0x1e */
#define CSK05T_REG_SCAN_CTRL		0x1f
#define   CSK05T_SCAN_ENABLE		BIT(0)
#define CSK05T_REG_KEY_STATE		0x22	/* two bytes, little endian */
#define CSK05T_REG_VERSION		0xff

#define CSK05T_NUM_CHANNELS		5
#define CSK05T_INIT_TRIES		10

/* The gap the part needs between the address write and the read. */
#define CSK05T_READ_GAP_US		30

#define CSK05T_RESET_LOW_MS		10
#define CSK05T_RESET_SETTLE_MS		50

#define CSK05T_DEFAULT_POLL_MS		30

struct csk05t {
	struct i2c_client *client;
	struct input_dev *input;
	struct gpio_desc *reset;
	unsigned int keycodes[CSK05T_NUM_CHANNELS];
	u16 last_state;
};

/*
 * Scan configuration, written as one block at 0x18. The part has no
 * published defaults and does not scan usefully without these.
 */
static const u8 csk05t_scan_params[] = {
	6,		/* 0x18 scan clock divider */
	0xff,		/* 0x19 fixed by the reference */
	25,		/* 0x1a scan current */
	100,		/* 0x1b noise threshold */
	200 & 0xff,	/* 0x1c touch threshold, low byte */
	200 >> 8,	/* 0x1d touch threshold, high byte */
	100,		/* 0x1e global sensitivity */
};

static int csk05t_read(struct csk05t *csk, u8 reg, u8 *buf, size_t len)
{
	struct i2c_client *client = csk->client;
	int ret;

	ret = i2c_master_send(client, &reg, 1);
	if (ret < 0)
		return ret;
	if (ret != 1)
		return -EIO;

	/* STOP has happened; the part wants a breather before the read. */
	udelay(CSK05T_READ_GAP_US);

	ret = i2c_master_recv(client, buf, len);
	if (ret < 0)
		return ret;
	if (ret != (int)len)
		return -EIO;

	return 0;
}

static int csk05t_write(struct csk05t *csk, u8 reg, const u8 *buf, size_t len)
{
	u8 tmp[1 + sizeof(csk05t_scan_params)];
	int ret;

	if (len > sizeof(tmp) - 1)
		return -EINVAL;

	tmp[0] = reg;
	memcpy(&tmp[1], buf, len);

	ret = i2c_master_send(csk->client, tmp, len + 1);
	if (ret < 0)
		return ret;

	return ret == (int)len + 1 ? 0 : -EIO;
}

static int csk05t_write_verify(struct csk05t *csk, u8 reg, const u8 *buf,
			       size_t len)
{
	u8 check[sizeof(csk05t_scan_params)];
	int ret;

	ret = csk05t_write(csk, reg, buf, len);
	if (ret)
		return ret;

	usleep_range(100, 200);

	ret = csk05t_read(csk, reg, check, len);
	if (ret)
		return ret;

	return memcmp(check, buf, len) ? -EIO : 0;
}

static void csk05t_reset(struct csk05t *csk)
{
	if (!csk->reset)
		return;

	gpiod_set_value_cansleep(csk->reset, 1);
	msleep(CSK05T_RESET_LOW_MS);
	gpiod_set_value_cansleep(csk->reset, 0);
}

static int csk05t_configure(struct csk05t *csk)
{
	struct device *dev = &csk->client->dev;
	int ret = -EIO;
	unsigned int i;
	u8 val;

	for (i = 0; i < CSK05T_INIT_TRIES; i++) {
		csk05t_reset(csk);

		/*
		 * Settle time grows with each attempt: a part slow to come up after
		 * reset is the failure this rides out.
		 */
		msleep(CSK05T_RESET_SETTLE_MS + i * 2);

		val = 0;
		ret = csk05t_write_verify(csk, CSK05T_REG_SCAN_CTRL, &val, 1);
		if (ret)
			continue;

		msleep(20);

		val = GENMASK(CSK05T_NUM_CHANNELS - 1, 0);
		ret = csk05t_write_verify(csk, CSK05T_REG_CHANNEL_EN, &val, 1);
		if (ret)
			continue;

		ret = csk05t_write_verify(csk, CSK05T_REG_SCAN_PARAMS,
					  csk05t_scan_params,
					  sizeof(csk05t_scan_params));
		if (ret)
			continue;

		val = CSK05T_SCAN_ENABLE;
		ret = csk05t_write_verify(csk, CSK05T_REG_SCAN_CTRL, &val, 1);
		if (ret)
			continue;

		if (i)
			dev_dbg(dev, "configured on attempt %u\n", i + 1);

		return 0;
	}

	dev_err(dev, "failed to configure the scan: %d\n", ret);

	return ret;
}

static void csk05t_poll(struct input_dev *input)
{
	struct csk05t *csk = input_get_drvdata(input);
	unsigned int i;
	u16 state;
	u8 buf[2];
	int ret;

	/*
	 * Scanning off means the part lost its configuration, which it does on
	 * a glitch and then reports nothing for ever.
	 */
	ret = csk05t_read(csk, CSK05T_REG_SCAN_CTRL, buf, 1);
	if (ret)
		return;

	if (!(buf[0] & CSK05T_SCAN_ENABLE)) {
		dev_warn_ratelimited(&csk->client->dev,
				     "scan stopped, reconfiguring\n");
		if (csk05t_configure(csk))
			return;

		csk->last_state = 0;
		return;
	}

	ret = csk05t_read(csk, CSK05T_REG_KEY_STATE, buf, sizeof(buf));
	if (ret)
		return;

	state = get_unaligned_le16(buf);
	if (state == csk->last_state)
		return;

	for (i = 0; i < CSK05T_NUM_CHANNELS; i++) {
		if ((state ^ csk->last_state) & BIT(i))
			input_report_key(csk->input, csk->keycodes[i],
					 !!(state & BIT(i)));
	}

	input_sync(csk->input);
	csk->last_state = state;
}

static int csk05t_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct input_dev *input;
	unsigned int poll_ms;
	struct csk05t *csk;
	unsigned int i;
	int ret;
	u8 ver;

	csk = devm_kzalloc(dev, sizeof(*csk), GFP_KERNEL);
	if (!csk)
		return -ENOMEM;

	csk->client = client;

	/* Held in reset until configure() releases it. */
	csk->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(csk->reset))
		return dev_err_probe(dev, PTR_ERR(csk->reset),
				     "failed to get reset gpio\n");

	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;

	csk->input = input;
	input_set_drvdata(input, csk);

	input->name = "Hynitron CSK05T";
	input->id.bustype = BUS_I2C;
	input->keycode = csk->keycodes;
	input->keycodesize = sizeof(csk->keycodes[0]);
	input->keycodemax = ARRAY_SIZE(csk->keycodes);

	/*
	 * F1 to F5 until a board says otherwise with linux,keycodes.
	 */
	for (i = 0; i < CSK05T_NUM_CHANNELS; i++)
		csk->keycodes[i] = KEY_F1 + i;

	device_property_read_u32_array(dev, "linux,keycodes", csk->keycodes,
				       ARRAY_SIZE(csk->keycodes));

	for (i = 0; i < CSK05T_NUM_CHANNELS; i++)
		input_set_capability(input, EV_KEY, csk->keycodes[i]);

	ret = csk05t_configure(csk);
	if (ret)
		return dev_err_probe(dev, ret, "initialisation failed\n");

	ret = csk05t_read(csk, CSK05T_REG_VERSION, &ver, 1);
	if (ret)
		return dev_err_probe(dev, ret, "failed to read version\n");

	dev_info(dev, "CSK05T firmware version %#04x\n", ver);

	/*
	 * Polled on purpose: the part has an INT pin but is reported never to
	 * produce an edge on it.
	 */
	poll_ms = CSK05T_DEFAULT_POLL_MS;
	device_property_read_u32(dev, "poll-interval", &poll_ms);

	ret = input_setup_polling(input, csk05t_poll);
	if (ret)
		return dev_err_probe(dev, ret, "failed to set up polling\n");

	input_set_poll_interval(input, poll_ms);

	return input_register_device(input);
}

static const struct i2c_device_id csk05t_id[] = {
	{ "csk05t" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, csk05t_id);

static const struct of_device_id csk05t_of_match[] = {
	{ .compatible = "hynitron,csk05t" },
	{ }
};
MODULE_DEVICE_TABLE(of, csk05t_of_match);

static struct i2c_driver csk05t_driver = {
	.driver = {
		.name = "csk05t",
		.of_match_table = csk05t_of_match,
	},
	.probe = csk05t_probe,
	.id_table = csk05t_id,
};
module_i2c_driver(csk05t_driver);

MODULE_DESCRIPTION("Hynitron CSK05T capacitive key controller driver");
MODULE_LICENSE("GPL");
