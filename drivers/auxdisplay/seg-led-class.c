// SPDX-License-Identifier: GPL-2.0
/*
 * Multi-digit 7-segment display built from LED class devices.
 *
 * seg-led-gpio drives one digit from a GPIO array; panels like this hang
 * every segment off a multi-channel LED controller, where the segments
 * are already LED class devices. Presented as a line display: "message"
 * lights segments, "map_seg7" redefines a character.
 *
 * No decimal point. A colon is not part of a character cell, so leave it
 * a plain LED and it keeps its triggers.
 */

#include <linux/bits.h>
#include <linux/container_of.h>
#include <linux/err.h>
#include <linux/kstrtox.h>
#include <linux/leds.h>
#include <linux/map_to_7segment.h>
#include <linux/minmax.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "line-display.h"

/* Segments a..g, in the bit order of map_to_7segment.h. */
#define SEGMENTS_PER_DIGIT	7

struct seg_led_class_priv {
	struct linedisp linedisp;
	struct delayed_work work;
	unsigned int num_digits;
	unsigned int brightness;
	unsigned int max_brightness;
	unsigned int num_segments;
	struct led_classdev *segment[] __counted_by(num_segments);
};

static void seg_led_class_update(struct work_struct *work)
{
	struct seg_led_class_priv *priv =
		container_of(work, struct seg_led_class_priv, work.work);
	struct linedisp *linedisp = &priv->linedisp;
	struct linedisp_map *map = linedisp->map;
	unsigned int digit, segment;

	for (digit = 0; digit < priv->num_digits; digit++) {
		int bits = map_to_seg7(&map->map.seg7, linedisp->buf[digit]);

		/* An unmapped character blanks its cell rather than failing. */
		if (bits < 0)
			bits = 0;

		for (segment = 0; segment < SEGMENTS_PER_DIGIT; segment++) {
			struct led_classdev *led =
				priv->segment[digit * SEGMENTS_PER_DIGIT + segment];

			led_set_brightness(led, (bits & BIT(segment)) ?
					   priv->brightness : LED_OFF);
		}
	}
}

static int seg_led_class_get_map_type(struct linedisp *linedisp)
{
	struct seg_led_class_priv *priv =
		container_of(linedisp, struct seg_led_class_priv, linedisp);

	INIT_DELAYED_WORK(&priv->work, seg_led_class_update);
	return LINEDISP_MAP_SEG7;
}

static void seg_led_class_linedisp_update(struct linedisp *linedisp)
{
	struct seg_led_class_priv *priv =
		container_of(linedisp, struct seg_led_class_priv, linedisp);

	/*
	 * linedisp calls this from a timer and must not sleep; I2C can.
	 */
	schedule_delayed_work(&priv->work, 0);
}

static const struct linedisp_ops seg_led_class_ops = {
	.get_map_type = seg_led_class_get_map_type,
	.update = seg_led_class_linedisp_update,
	.no_boot_message = true,
};

static ssize_t brightness_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct seg_led_class_priv *priv = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", priv->brightness);
}

static ssize_t brightness_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct seg_led_class_priv *priv = dev_get_drvdata(dev);
	unsigned int value;
	int err;

	err = kstrtouint(buf, 0, &value);
	if (err)
		return err;

	priv->brightness = min(value, priv->max_brightness);
	schedule_delayed_work(&priv->work, 0);

	return count;
}
static DEVICE_ATTR_RW(brightness);

static struct attribute *seg_led_class_attrs[] = {
	&dev_attr_brightness.attr,
	NULL,
};
ATTRIBUTE_GROUPS(seg_led_class);

static int seg_led_class_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct seg_led_class_priv *priv;
	unsigned int num_digits;
	int count, i;

	count = of_count_phandle_with_args(dev->of_node, "leds", NULL);
	if (count < 0)
		return dev_err_probe(dev, count, "no 'leds' property\n");

	if (count == 0 || count % SEGMENTS_PER_DIGIT)
		return dev_err_probe(dev, -EINVAL,
				     "%d LEDs given, expected a non-zero multiple of %d\n",
				     count, SEGMENTS_PER_DIGIT);

	num_digits = count / SEGMENTS_PER_DIGIT;

	priv = devm_kzalloc(dev, struct_size(priv, segment, count), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Both counters before the array is touched: __counted_by. */
	priv->num_segments = count;
	priv->num_digits = num_digits;

	/*
	 * Seven LEDs per digit, segments a..g, digits left to right.
	 */
	for (i = 0; i < count; i++) {
		struct led_classdev *led = devm_of_led_get(dev, i);

		if (IS_ERR(led))
			return dev_err_probe(dev, PTR_ERR(led),
					     "failed to get LED %d\n", i);
		priv->segment[i] = led;
	}

	priv->max_brightness = LED_FULL;
	for (i = 0; i < count; i++)
		priv->max_brightness = min(priv->max_brightness,
					   priv->segment[i]->max_brightness);

	priv->brightness = priv->max_brightness;
	device_property_read_u32(dev, "default-brightness-level",
				 &priv->brightness);
	priv->brightness = min(priv->brightness, priv->max_brightness);

	platform_set_drvdata(pdev, priv);

	return linedisp_register(&priv->linedisp, dev, num_digits,
				 &seg_led_class_ops);
}

static void seg_led_class_remove(struct platform_device *pdev)
{
	struct seg_led_class_priv *priv = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&priv->work);
	linedisp_unregister(&priv->linedisp);
}

static const struct of_device_id seg_led_class_of_match[] = {
	{ .compatible = "led-7-segment" },
	{}
};
MODULE_DEVICE_TABLE(of, seg_led_class_of_match);

static struct platform_driver seg_led_class_driver = {
	.probe = seg_led_class_probe,
	.remove = seg_led_class_remove,
	.driver = {
		.name = "seg-led-class",
		.of_match_table = seg_led_class_of_match,
		.dev_groups = seg_led_class_groups,
	},
};
module_platform_driver(seg_led_class_driver);

MODULE_DESCRIPTION("Multi-digit 7-segment display driven by LED class devices");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS("LINEDISP");
