// SPDX-License-Identifier: GPL-2.0
/*
 * The W1 has no Bluetooth enable line: its BT core sits behind the chip's
 * own PMU, which is reached over this SDIO link. It is offered as a
 * regulator so that hci_aml can switch it like any other supply.
 *
 * The register sequences are Amlogic's, from config_bt_pmu_reg() in their
 * sdio_bt driver.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>

#include "w1_sdio.h"
#include "chip_bt_pmu_reg.h"
#include "chip_intf_reg.h"
#include "wifi_aon_addr.h"

#define W1_BT_HOST_REQ			0x231
#define W1_BT_HOST_REQ_SET		BIT(0)
#define W1_BT_HOST_REQ_STATE		GENMASK(4, 1)
#define W1_BT_HOST_REQ_KEEP_ALIVE	BIT(5)

#define W1_PMU_STATE			GENMASK(3, 0)

/* bit of w1_sdio_wifi_bt_alive that stands for Bluetooth */
#define W1_SDIO_USER_BT			0

static bool w1_bt_on;

static u32 w1_bt_read(u32 reg)
{
	return w1_g_w1_hif_ops.bt_hi_read_word(reg);
}

static void w1_bt_write(u32 reg, u32 val)
{
	w1_g_w1_hif_ops.bt_hi_write_word(reg, val);
}

static void w1_bt_update(u32 reg, u32 clr, u32 set)
{
	w1_bt_write(reg, (w1_bt_read(reg) & ~clr) | set);
}

static void w1_bt_host_req_update(u8 clr, u8 set)
{
	u8 val = w1_g_w1_hif_ops.hi_bottom_read8(SDIO_FUNC1, W1_BT_HOST_REQ);

	w1_g_w1_hif_ops.hi_bottom_write8(SDIO_FUNC1, W1_BT_HOST_REQ,
					 (val & ~clr) | set);
}

/* Amlogic's way out of a PMU state machine that does not move on. */
static void w1_bt_pmu_kick(u32 state)
{
	u32 reg;

	if (state == PMU_ACT_MODE)
		reg = RG_BT_PMU_A11;
	else if (state >= PMU_PWR_XOSC && state <= PMU_WAKE_XOSC)
		reg = CHIP_BT_PMU_REG_BASE + (state - 1) * 4;
	else
		return;

	w1_bt_write(reg, (w1_bt_read(reg) & ~BIT(31)) + 1);
	msleep(50);
}

static int w1_bt_pmu_on(void)
{
	u32 a12, state;

	a12 = w1_bt_read(RG_BT_PMU_A12) & ~GENMASK(7, 6);

	/* BT work flag; xosc, bbpll, ao_iso and pmu masks */
	w1_bt_write(RG_BT_PMU_A14, 0x1f);
	w1_bt_host_req_update(W1_BT_HOST_REQ_SET, 0);
	msleep(10);

	w1_bt_write(RG_BT_PMU_A17, 0x700);
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A20, 0x0);
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A18, 0x1700);
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A22, 0x704);
	msleep(10);

	/* rg_bb_reset_man */
	w1_bt_write(RG_BT_PMU_A12, a12 | BIT(7));
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A12, a12 | BIT(7) | BIT(6));
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A12, a12 | BIT(7));
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A12, a12);
	usleep_range(1000, 2000);

	w1_bt_host_req_update(0, FIELD_PREP(W1_BT_HOST_REQ_STATE, PMU_PWR_OFF) |
				 W1_BT_HOST_REQ_SET);
	w1_bt_host_req_update(W1_BT_HOST_REQ_STATE | W1_BT_HOST_REQ_SET, 0);
	msleep(20);

	return read_poll_timeout(w1_bt_read, state,
				 FIELD_GET(W1_PMU_STATE, state) == PMU_ACT_MODE,
				 5 * USEC_PER_MSEC, 2500 * USEC_PER_MSEC, false,
				 RG_BT_PMU_A15);
}

static void w1_bt_pmu_off(void)
{
	u32 a12, a15, state;

	/* keep the firmware out of power-down while it is being stopped */
	w1_bt_update(RG_BT_PMU_A16, 0, BIT(30));

	a12 = w1_bt_read(RG_BT_PMU_A12);
	a15 = w1_bt_read(RG_BT_PMU_A15);

	w1_bt_update(RG_BT_PMU_A16, BIT(0), 0);
	msleep(10);
	w1_bt_update(RG_BT_PMU_A16, BIT(1), 0);
	msleep(10);
	if (a15 == PMU_SLEEP_MODE) {
		w1_bt_update(RG_BT_PMU_A16, 0, BIT(1));
		msleep(10);
	}
	w1_bt_update(RG_BT_PMU_A16, BIT(31), 0);
	usleep_range(1000, 2000);

	if (read_poll_timeout(w1_bt_read, state,
			      FIELD_GET(W1_PMU_STATE, state) == PMU_ACT_MODE,
			      10 * USEC_PER_MSEC, 500 * USEC_PER_MSEC, false,
			      RG_BT_PMU_A15))
		w1_bt_pmu_kick(FIELD_GET(W1_PMU_STATE, state));

	w1_bt_write(RG_BT_PMU_A12, a12 | BIT(7) | BIT(6));
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A22, 0x707);
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A18, 0x1787);
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A20, 0x0);
	usleep_range(1000, 2000);
	w1_bt_write(RG_BT_PMU_A17, 0x707);
	usleep_range(1000, 2000);

	w1_bt_write(RG_BT_PMU_A14, 0x0);
	w1_bt_host_req_update(0, FIELD_PREP(W1_BT_HOST_REQ_STATE, PMU_SLEEP_MODE) |
				 W1_BT_HOST_REQ_SET);
}

static int w1_bt_pmu_set(bool on)
{
	unsigned char (*wake)(void);
	unsigned char busy, awake;
	int ret;

	ret = read_poll_timeout(READ_ONCE, busy, !busy, 10 * USEC_PER_MSEC,
				5 * USEC_PER_SEC, false, w1_wifi_in_insmod);
	if (ret)
		return ret;

	w1_bt_host_req_update(0, W1_BT_HOST_REQ_KEEP_ALIVE);

	wake = READ_ONCE(w1_host_wake_w1_req);
	if (wake) {
		w1_aml_wifi_sdio_power_lock();
		ret = read_poll_timeout(wake, awake, awake, 10 * USEC_PER_MSEC,
					5 * USEC_PER_SEC, false);
		w1_aml_wifi_sdio_power_unlock();
	}

	if (!ret) {
		if (on)
			ret = w1_bt_pmu_on();
		else
			w1_bt_pmu_off();
	}

	w1_bt_host_req_update(W1_BT_HOST_REQ_KEEP_ALIVE, 0);

	/* sdio_bt clears both once the core is up and sets 31 across suspend */
	if (!ret && on)
		w1_bt_update(RG_AON_A15, BIT(31) | BIT(30), 0);

	return ret;
}

static int w1_bt_power_enable(struct regulator_dev *rdev)
{
	int ret;

	ret = w1_bt_pmu_set(true);
	if (ret)
		return ret;

	w1_bt_on = true;
	return 0;
}

static int w1_bt_power_disable(struct regulator_dev *rdev)
{
	int ret;

	/* an error here would leave the regulator core counting it as on */
	ret = w1_bt_pmu_set(false);
	if (ret)
		dev_warn(rdev_get_dev(rdev), "BT core not stopped: %d\n", ret);

	w1_bt_on = false;
	return 0;
}

static int w1_bt_power_is_enabled(struct regulator_dev *rdev)
{
	return w1_bt_on;
}

static const struct regulator_ops w1_bt_power_ops = {
	.enable = w1_bt_power_enable,
	.disable = w1_bt_power_disable,
	.is_enabled = w1_bt_power_is_enabled,
};

static const struct regulator_desc w1_bt_power_desc = {
	.name = "w1-bt",
	.type = REGULATOR_VOLTAGE,
	.owner = THIS_MODULE,
	.ops = &w1_bt_power_ops,
};

int w1_bt_power_register(struct sdio_func *func)
{
	struct regulator_config cfg = { };
	struct regulator_dev *rdev;
	struct device_node *np;

	if (!func)
		return 0;

	np = of_get_child_by_name(func->dev.of_node, "regulator");
	if (!np)
		return 0;

	cfg.dev = &func->dev;
	cfg.of_node = np;
	cfg.init_data = of_get_regulator_init_data(&func->dev, np,
						   &w1_bt_power_desc);
	if (!cfg.init_data) {
		of_node_put(np);
		return -ENOMEM;
	}

	w1_bt_on = false;
	rdev = devm_regulator_register(&func->dev, &w1_bt_power_desc, &cfg);
	of_node_put(np);
	if (IS_ERR(rdev))
		return dev_err_probe(&func->dev, PTR_ERR(rdev),
				     "failed to register BT power\n");

	/*
	 * hci_aml holds this regulator for as long as it is bound. Marking BT
	 * alive stops a Wi-Fi unload from unregistering the SDIO driver, and
	 * the regulator with it, underneath.
	 */
	w1_set_wifi_bt_sdio_driver_bit(true, W1_SDIO_USER_BT);
	return 0;
}
