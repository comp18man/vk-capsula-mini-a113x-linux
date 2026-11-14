// SPDX-License-Identifier: GPL-2.0-only
/*
 * VK Capsula Mini second-source parts
 *
 * The board is built with one of two speaker amplifiers on one I2C footprint.
 * Every candidate is marked "fail-needs-probe", the one that responds is enabled, and the sound card
 * loses its references to the rest, since a card waits for every codec it names.
 */

#include <linux/cleanup.h>
#include <linux/i2c-of-prober.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>

#define DRV_NAME	"vk-capsula-mini-prober"

#define CAPSULA_NTP_GIEC	"8910a_giec-capsula-mini.bin"

static char *factory;
module_param(factory, charp, 0444);
MODULE_PARM_DESC(factory, "Unifykey \"factory\" as read by the bootloader");

static const struct i2c_of_probe_cfg capsula_amplifier = {
	.type = "audio-codec",
};

static void capsula_pick_tuning(struct device *dev)
{
	const char *name;
	int ret;

	if (!factory || !strcmp(factory, "taide"))
		return;

	struct device_node *np __free(device_node) =
		of_find_compatible_node(NULL, NULL, "neofidelity,ntp8910a");
	if (!np)
		return;

	if (!of_property_read_string(np, "firmware-name", &name) &&
	    !strcmp(name, CAPSULA_NTP_GIEC))
		return;

	struct of_changeset *ocs __free(kfree) =
		kzalloc(sizeof(*ocs), GFP_KERNEL);
	if (!ocs)
		return;

	of_changeset_init(ocs);
	ret = of_changeset_update_prop_string(ocs, np, "firmware-name",
					      CAPSULA_NTP_GIEC);
	if (!ret)
		ret = of_changeset_apply(ocs);
	if (ret) {
		of_changeset_destroy(ocs);
		dev_warn(dev, "factory %s, giec tuning not set: %d\n",
			 factory, ret);
		return;
	}

	retain_and_null_ptr(ocs);
}

static bool capsula_not_fitted(struct device_node *np)
{
	const char *status;

	return !of_property_read_string(np, "status", &status) &&
	       !strcmp(status, "fail-needs-probe");
}


static int capsula_trim_link(struct device *dev, struct of_changeset *ocs,
			     struct device_node *link)
{
	bool fitted = false;
	int ret;

	for_each_child_of_node_scoped(link, codec) {
		struct device_node *dai __free(device_node) =
			of_parse_phandle(codec, "sound-dai", 0);

		if (dai && of_device_is_available(dai))
			fitted = true;
	}

	if (!fitted)
		return 0;

	for_each_child_of_node_scoped(link, codec) {
		struct device_node *dai __free(device_node) =
			of_parse_phandle(codec, "sound-dai", 0);

		if (!dai || !capsula_not_fitted(dai))
			continue;

		dev_dbg(dev, "dropping %pOF\n", codec);
		ret = of_changeset_detach_node(ocs, codec);
		if (ret)
			return ret;
	}

	return 0;
}

static int capsula_trim_card(struct device *dev)
{
	struct device_node *sound __free(device_node) =
		of_find_node_by_path("/sound");
	int ret;

	if (!sound)
		return 0;

	struct of_changeset *ocs __free(kfree) =
		kzalloc(sizeof(*ocs), GFP_KERNEL);
	if (!ocs)
		return -ENOMEM;

	of_changeset_init(ocs);

	for_each_child_of_node_scoped(sound, link) {
		ret = capsula_trim_link(dev, ocs, link);
		if (ret) {
			of_changeset_destroy(ocs);
			return ret;
		}
	}

	if (list_empty(&ocs->entries))
		return 0;

	ret = of_changeset_apply(ocs);
	if (ret) {
		of_changeset_destroy(ocs);
		return ret;
	}

	retain_and_null_ptr(ocs);

	return 0;
}

static int capsula_prober_probe(struct platform_device *pdev)
{
	int ret;

	capsula_pick_tuning(&pdev->dev);

	ret = i2c_of_probe_component(&pdev->dev, &capsula_amplifier, NULL);
	if (ret)
		return ret;

	return capsula_trim_card(&pdev->dev);
}

static struct platform_driver capsula_prober_driver = {
	.probe	= capsula_prober_probe,
	.driver	= {
		.name = DRV_NAME,
	},
};

static struct platform_device *capsula_prober_pdev;

static int __init capsula_prober_init(void)
{
	int ret;

	if (!of_machine_is_compatible("vk,capsula-mini"))
		return -ENODEV;

	ret = platform_driver_register(&capsula_prober_driver);
	if (ret)
		return ret;

	capsula_prober_pdev =
		platform_device_register_simple(DRV_NAME, PLATFORM_DEVID_NONE,
						NULL, 0);
	if (IS_ERR(capsula_prober_pdev)) {
		platform_driver_unregister(&capsula_prober_driver);
		return PTR_ERR(capsula_prober_pdev);
	}

	return 0;
}
module_init(capsula_prober_init);

static void __exit capsula_prober_exit(void)
{
	platform_device_unregister(capsula_prober_pdev);
	platform_driver_unregister(&capsula_prober_driver);
}
module_exit(capsula_prober_exit);

MODULE_DESCRIPTION("VK Capsula Mini second-source part prober");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS("I2C_OF_PROBER");
