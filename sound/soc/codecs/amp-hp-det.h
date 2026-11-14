/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __AMP_HP_DET_H
#define __AMP_HP_DET_H

#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/workqueue.h>

struct device;
struct gpio_desc;

struct amp_hp_det {
	struct device *dev;
	struct gpio_desc *gpio;
	int irq;
	struct mutex *lock;		/* owner's, held around ->changed() */
	void (*changed)(void *data);
	void *data;
	struct delayed_work work;
	bool seen;
	bool inserted;
};

int devm_amp_hp_det_init(struct device *dev, struct amp_hp_det *det, int irq,
			 struct mutex *lock, void (*changed)(void *data),
			 void *data);
void amp_hp_det_resync(struct amp_hp_det *det);

static inline bool amp_hp_det_inserted(const struct amp_hp_det *det)
{
	return det->inserted;
}

#endif /* __AMP_HP_DET_H */
