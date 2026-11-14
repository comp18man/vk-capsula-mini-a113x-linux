// SPDX-License-Identifier: GPL-2.0-only
/*
 * Awinic AW21036 36-channel LED driver
 *
 * Each channel is a plain LED: its PWM duty is the BR register, its DC scale
 * (COL) stays at full, and all of them share one global current (GCC).
 */

#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/math.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define AW21036_GCR		0x00
#define  AW21036_GCR_CHIPEN	BIT(0)
#define AW21036_BR(led)		(0x00 + (led))
#define AW21036_UPDATE		0x49
#define AW21036_COL(led)	(0x49 + (led))
#define AW21036_GCCR		0x6e
#define AW21036_RESET		0x7f
#define  AW21036_CHIP_ID	0x18

#define AW21036_LEDS		36
#define AW21036_FULL		0xff

struct aw21036;

struct aw21036_led {
	struct led_classdev cdev;
	struct aw21036 *chip;
	unsigned int num;
};

struct aw21036 {
	struct regmap *regmap;
	struct gpio_desc *enable;
	unsigned int current_divider;
	unsigned int num_leds;
	struct aw21036_led leds[] __counted_by(num_leds);
};

static int aw21036_brightness_set(struct led_classdev *cdev,
				  enum led_brightness brightness)
{
	struct aw21036_led *led = container_of(cdev, struct aw21036_led, cdev);
	struct regmap *regmap = led->chip->regmap;
	int ret;

	ret = regmap_write(regmap, AW21036_BR(led->num), brightness);
	if (ret)
		return ret;

	return regmap_write(regmap, AW21036_UPDATE, 0);
}

static int aw21036_write_current(struct aw21036 *chip)
{
	return regmap_write(chip->regmap, AW21036_GCCR,
			    DIV_ROUND_CLOSEST(AW21036_FULL,
					      chip->current_divider));
}

static ssize_t current_divider_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct aw21036 *chip = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", chip->current_divider);
}

static ssize_t current_divider_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct aw21036 *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 0, &value);
	if (ret)
		return ret;

	if (value < 1 || value > 4)
		return -ERANGE;

	chip->current_divider = value;

	ret = aw21036_write_current(chip);

	return ret ? ret : count;
}
static DEVICE_ATTR_RW(current_divider);

static struct attribute *aw21036_attrs[] = {
	&dev_attr_current_divider.attr,
	NULL
};
ATTRIBUTE_GROUPS(aw21036);

static int aw21036_init(struct device *dev, struct aw21036 *chip)
{
	u8 col[AW21036_LEDS];
	unsigned int id;
	int ret;

	ret = regmap_read(chip->regmap, AW21036_RESET, &id);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to read chip ID\n");

	if (id != AW21036_CHIP_ID)
		return dev_err_probe(dev, -ENODEV, "Unexpected chip ID %#x\n",
				     id);

	ret = regmap_write(chip->regmap, AW21036_RESET, 0);
	if (ret)
		return ret;

	/* No I2C for 2 ms after a reset, then 200 us for the oscillator. */
	fsleep(2000);

	memset(col, AW21036_FULL, sizeof(col));
	ret = regmap_bulk_write(chip->regmap, AW21036_COL(1), col,
				sizeof(col));
	if (ret)
		return ret;

	ret = aw21036_write_current(chip);
	if (ret)
		return ret;

	ret = regmap_write(chip->regmap, AW21036_GCR, AW21036_GCR_CHIPEN);
	if (ret)
		return ret;

	fsleep(200);

	return 0;
}

static void aw21036_power_off(void *data)
{
	struct aw21036 *chip = data;

	regmap_write(chip->regmap, AW21036_GCR, 0);
}

static const struct regmap_config aw21036_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = AW21036_RESET,
};

static int aw21036_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw21036 *chip;
	unsigned int count, i = 0;
	u32 divider;
	int ret;

	count = device_get_child_node_count(dev);
	if (!count || count > AW21036_LEDS)
		return dev_err_probe(dev, -EINVAL, "Expected 1 to %u LEDs\n",
				     AW21036_LEDS);

	chip = devm_kzalloc(dev, struct_size(chip, leds, count), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->num_leds = count;
	chip->current_divider = 1;
	i2c_set_clientdata(client, chip);

	if (!device_property_read_u32(dev, "awinic,current-divider",
				      &divider)) {
		if (divider < 1 || divider > 4)
			return dev_err_probe(dev, -EINVAL,
					     "awinic,current-divider must be 1 to 4\n");
		chip->current_divider = divider;
	}

	chip->regmap = devm_regmap_init_i2c(client, &aw21036_regmap);
	if (IS_ERR(chip->regmap))
		return dev_err_probe(dev, PTR_ERR(chip->regmap),
				     "Failed to allocate regmap\n");

	chip->enable = devm_gpiod_get_optional(dev, "enable", GPIOD_OUT_HIGH);
	if (IS_ERR(chip->enable))
		return dev_err_probe(dev, PTR_ERR(chip->enable),
				     "Failed to get enable GPIO\n");

	ret = aw21036_init(dev, chip);
	if (ret)
		return ret;

	ret = devm_add_action_or_reset(dev, aw21036_power_off, chip);
	if (ret)
		return ret;

	device_for_each_child_node_scoped(dev, child) {
		struct led_init_data init_data = { .fwnode = child };
		struct aw21036_led *led = &chip->leds[i];
		u32 num;

		ret = fwnode_property_read_u32(child, "reg", &num);
		if (ret || num < 1 || num > AW21036_LEDS)
			return dev_err_probe(dev, -EINVAL,
					     "%pfwP: reg must be 1 to %u\n",
					     child, AW21036_LEDS);

		led->chip = chip;
		led->num = num;
		led->cdev.max_brightness = LED_FULL;
		led->cdev.brightness_set_blocking = aw21036_brightness_set;

		ret = devm_led_classdev_register_ext(dev, &led->cdev,
						     &init_data);
		if (ret)
			return dev_err_probe(dev, ret,
					     "Failed to register %pfwP\n", child);

		i++;
	}

	return 0;
}

static const struct i2c_device_id aw21036_id[] = {
	{ "aw21036" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, aw21036_id);

static const struct of_device_id aw21036_of_match[] = {
	{ .compatible = "awinic,aw21036" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw21036_of_match);

static struct i2c_driver aw21036_driver = {
	.driver = {
		.name = "leds-aw21036",
		.of_match_table = aw21036_of_match,
		.dev_groups = aw21036_groups,
	},
	.probe = aw21036_probe,
	.id_table = aw21036_id,
};
module_i2c_driver(aw21036_driver);

MODULE_DESCRIPTION("Awinic AW21036 LED driver");
MODULE_LICENSE("GPL");
