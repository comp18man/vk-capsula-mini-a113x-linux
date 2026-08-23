/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     driver layer power save function
 *
 *
 ****************************************************************************************
 */


#ifndef _DRV__POWER_H
#define _DRV__POWER_H

#include "wifi_drv_timer.h"
#include "wifi_drv_main.h"

/*
 * Definitions for the amlogic power control module.
 */
#define DRV_PS_LOCK_INIT(_sc)  spin_lock_init(&(_sc)->drv_ps.ps_pwrsave_lock)
#define DRV_PS_LOCK(_sc)  OS_SPIN_LOCK_IRQ(&(_sc)->drv_ps.ps_pwrsave_lock, (_sc)->drv_ps.__ilockflags)
#define DRV_PS_UNLOCK(_sc)  OS_SPIN_UNLOCK_IRQ(&(_sc)->drv_ps.ps_pwrsave_lock, (_sc)->drv_ps.__ilockflags)

enum drv_ps_state
{
    DRV_PWRSAVE_AWAKE = 1,
    DRV_PWRSAVE_FULL_SLEEP,
    DRV_PWRSAVE_NETWORK_SLEEP,
};

/*
 * Support interfaces
 */

void drv_pwrsave_proc(struct drv_private *drv_priv);
int  drv_pwrsave_timer(struct drv_private *drv_priv);
void drv_pwrsave_init(struct drv_private *drv_priv);

void drv_pwrsave_awake(struct drv_private *drv_priv, int wnet_vif_id);
void drv_pwrsave_netsleep(struct drv_private *drv_priv, int wnet_vif_id);
void drv_pwrsave_fullsleep(struct drv_private *drv_priv, int wnet_vif_id);
void driv_ps_wakeup (struct drv_private *drv_priv);
void driv_ps_sleep(struct drv_private *drv_priv);
void drv_pwrsave_wake_req (void *dpriv, int power_save);

#endif /* _DRV__POWER_H */
