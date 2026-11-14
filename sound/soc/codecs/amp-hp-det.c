// SPDX-License-Identifier: GPL-2.0-only
/*
 * Headphone detect for a speaker amplifier that shares its I2S link with a
 * line output and has to fall silent while a plug is in.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/lockdep.h>
#include <linux/module.h>

#include "amp-hp-det.h"

#define AMP_HP_DET_DEBOUNCE_MS	40
#define AMP_HP_DET_POLL_MS	300
#define AMP_HP_DET_BACKSTOP_MS	2000

/*
 * The pad interrupt cannot be asked for both edges: on this generation the
 * controller has no such mode and set_type answers -EINVAL, which is why the
 * vendor driver watches the falling one alone. So the trigger is turned around
 * after every change to face the transition that can still happen, and a slow
 * poll runs underneath to heal the one that slips through while the trigger is
 * being turned.
 *
 * confirm asks for the same reading twice, which is what the poll uses to sit
 * out the contact settling; the interrupt path has already waited instead.
 */
static void amp_hp_det_update(struct amp_hp_det *det, bool confirm)
{
	bool now = gpiod_get_value_cansleep(det->gpio) > 0;

	mutex_lock(det->lock);
	if ((!confirm || now == det->seen) && now != det->inserted) {
		det->inserted = now;
		dev_dbg(det->dev, "headphone jack %s\n",
			now ? "inserted, speaker muted" : "removed");
		det->changed(det->data);
	}
	det->seen = now;
	mutex_unlock(det->lock);

	if (det->irq > 0)
		irq_set_irq_type(det->irq, now ? IRQ_TYPE_EDGE_RISING :
						 IRQ_TYPE_EDGE_FALLING);
}

static irqreturn_t amp_hp_det_irq(int irq, void *data)
{
	struct amp_hp_det *det = data;

	msleep(AMP_HP_DET_DEBOUNCE_MS);
	amp_hp_det_update(det, false);

	return IRQ_HANDLED;
}

static void amp_hp_det_poll(struct work_struct *work)
{
	struct amp_hp_det *det = container_of(work, struct amp_hp_det,
					      work.work);
	unsigned int period = det->irq > 0 ? AMP_HP_DET_BACKSTOP_MS
					   : AMP_HP_DET_POLL_MS;

	amp_hp_det_update(det, true);
	schedule_delayed_work(&det->work, msecs_to_jiffies(period));
}

static void amp_hp_det_stop(void *data)
{
	struct amp_hp_det *det = data;

	cancel_delayed_work_sync(&det->work);
}

void amp_hp_det_resync(struct amp_hp_det *det)
{
	lockdep_assert_held(det->lock);

	if (!det->gpio)
		return;

	det->inserted = gpiod_get_value_cansleep(det->gpio) > 0;
	det->seen = det->inserted;
}
EXPORT_SYMBOL_GPL(amp_hp_det_resync);

int devm_amp_hp_det_init(struct device *dev, struct amp_hp_det *det, int irq,
			 struct mutex *lock, void (*changed)(void *data),
			 void *data)
{
	const char *name;
	int ret;

	det->dev = dev;
	det->lock = lock;
	det->changed = changed;
	det->data = data;

	det->gpio = devm_gpiod_get_optional(dev, "hp-det", GPIOD_IN);
	if (IS_ERR(det->gpio))
		return dev_err_probe(dev, PTR_ERR(det->gpio),
				     "Failed to get jack detect gpio\n");
	if (!det->gpio)
		return 0;

	det->inserted = gpiod_get_value_cansleep(det->gpio) > 0;
	det->seen = det->inserted;

	INIT_DELAYED_WORK(&det->work, amp_hp_det_poll);
	ret = devm_add_action_or_reset(dev, amp_hp_det_stop, det);
	if (ret)
		return ret;

	if (irq > 0) {
		name = devm_kasprintf(dev, GFP_KERNEL, "%s-hp-det",
				      dev_driver_string(dev));
		if (!name)
			return -ENOMEM;

		det->irq = irq;
		ret = devm_request_threaded_irq(dev, irq, NULL, amp_hp_det_irq,
						IRQF_ONESHOT, name, det);
		if (ret)
			return dev_err_probe(dev, ret,
					     "Failed to claim jack detect\n");

		irq_set_irq_type(irq, det->inserted ? IRQ_TYPE_EDGE_RISING :
						      IRQ_TYPE_EDGE_FALLING);
	} else {
		dev_info(dev, "no jack interrupt, polling the line\n");
	}

	schedule_delayed_work(&det->work, msecs_to_jiffies(AMP_HP_DET_POLL_MS));

	return 0;
}
EXPORT_SYMBOL_GPL(devm_amp_hp_det_init);

MODULE_DESCRIPTION("Headphone detect for speaker amplifiers");
MODULE_LICENSE("GPL");
