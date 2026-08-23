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


#include "wifi_mac_com.h"
#include "wifi_drv_power.h"

static char drv_pwrsave_cmd_arbitration(struct drv_private *drv_priv,
    enum drv_ps_state newstate, int wnet_vif_id)
{
    unsigned char vid = 0;

    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d vid:%d, newstate:%d, dot11VmacPsState[0]:%d, dot11VmacPsState[1]:%d, dot11ComPsState:%d\n",
        __func__, __LINE__, wnet_vif_id, newstate, drv_priv->dot11VmacPsState[0], drv_priv->dot11VmacPsState[1], drv_priv->dot11ComPsState);

    if (wnet_vif_id == PHY_VMAC_ID) {
        drv_priv->drv_PhyPsState = newstate;
    } else {
        //if (drv_priv->dot11VmacPsState[wnet_vif_id] == newstate) {
        //    return 0;//state not change,don't down cmd
        //}
        drv_priv->dot11VmacPsState[wnet_vif_id] = newstate;

        if (newstate == DRV_PWRSAVE_AWAKE) {
            if (drv_priv->dot11ComPsState == DRV_PWRSAVE_FULL_SLEEP) {
                drv_priv->dot11ComPsState = DRV_PWRSAVE_AWAKE;
            }
            return DRV_PWRSAVE_AWAKE;//send awake cmd

        } else if (newstate == DRV_PWRSAVE_FULL_SLEEP) {
            unsigned char all_wnet_vif_same = 1;

            for (vid = 0; vid < WIFI_MAX_VID; vid++) {
                if (drv_priv->dot11VmacPsState[vid] != DRV_PWRSAVE_FULL_SLEEP) {
                    all_wnet_vif_same = 0;
                    break;
                }
            }

            if (all_wnet_vif_same == 1) {
                drv_priv->dot11ComPsState = DRV_PWRSAVE_FULL_SLEEP;
                return DRV_PWRSAVE_FULL_SLEEP;
            } else {
                return DRV_PWRSAVE_NETWORK_SLEEP;
            }

        } else if (newstate == DRV_PWRSAVE_NETWORK_SLEEP) {
            if (drv_priv->dot11ComPsState == DRV_PWRSAVE_FULL_SLEEP) {
                drv_priv->dot11ComPsState = DRV_PWRSAVE_AWAKE;
            }
            return 0;//fw is awake by itself, and hal_fw_ps_status changed in interrupt.no cmd need down
        }
    }

    return 0;
}

static void drv_pwrsave_set_state(struct drv_private *drv_priv,
    enum drv_ps_state newstate,int wnet_vif_id)
{
    //newstate = drv_pwrsave_cmd_arbitration(drv_priv,newstate,wnet_vif_id);

    switch (newstate)
    {
        case DRV_PWRSAVE_AWAKE:
            drv_hal_setpower(PS_ACTIVE, wnet_vif_id);
            break;
        case DRV_PWRSAVE_FULL_SLEEP:
            drv_hal_setpower(PS_DOZE, wnet_vif_id);
            break;
        case DRV_PWRSAVE_NETWORK_SLEEP:
            //drv_hal_setpower(PS_NETWORK_SLEEP, wnet_vif_id);
            break;
        default:
            break;
    }

}

void drv_pwrsave_awake( struct drv_private *drv_priv, int wnet_vif_id)
{
    drv_pwrsave_set_state(drv_priv, DRV_PWRSAVE_AWAKE, wnet_vif_id);
}

void drv_pwrsave_fullsleep( struct drv_private *drv_priv, int wnet_vif_id)
{
    if (aml_wifi_is_enable_rf_test()) {
        drv_pwrsave_set_state(drv_priv, DRV_PWRSAVE_AWAKE, wnet_vif_id); //xman modified for hal thr test.
        return;
    }

    if (drv_priv->drv_scanning)
        return;

    drv_pwrsave_set_state(drv_priv, DRV_PWRSAVE_FULL_SLEEP, wnet_vif_id);
}

void drv_pwrsave_netsleep( struct drv_private *drv_priv, int wnet_vif_id)
{
    if (drv_priv->drv_scanning)
        return;

    drv_pwrsave_set_state(drv_priv, DRV_PWRSAVE_NETWORK_SLEEP, wnet_vif_id);
}

void driv_ps_wakeup (struct drv_private *drv_priv)
{
/*
    DRV_PS_LOCK(drv_priv);
    drv_priv->drv_ps.ps_hal_usecount++;
    DRV_PS_UNLOCK(drv_priv);
#if 1
    if (drv_priv->drv_ps.ps_hal_usecount == 1)
    {
        drv_pwrsave_awake(drv_priv, PHY_VMAC_ID);
    }
#endif
*/
}

void driv_ps_sleep(struct drv_private *drv_priv)
{
/*
    DRV_PS_LOCK(drv_priv);
    drv_priv->drv_ps.ps_hal_usecount--;
    DRV_PS_UNLOCK(drv_priv);
#if 1
    if (drv_priv->drv_ps.ps_hal_usecount == 0)
    {
        drv_pwrsave_fullsleep(drv_priv, PHY_VMAC_ID);
    }
#endif
*/
}

void drv_pwrsave_init(struct drv_private *drv_priv)
{
    drv_priv->drv_ps.ps_hal_usecount  = 0;
    DRV_PS_LOCK_INIT(drv_priv);
}

void drv_pwrsave_wake_req (void *dpriv, int power_save)
{
    struct drv_private *drv_priv = (struct drv_private *)dpriv;
    struct wlan_net_vif *wnet_vif = NULL;
    int wnet_vif_id = 0;

    if (drv_priv == NULL)
        return;

    if (power_save)
    {
        DRV_PS_LOCK(drv_priv);
        if (drv_priv->add_wakeup_work == 1)
        {
            DRV_PS_UNLOCK(drv_priv);
            return;
        }
        DRV_PS_UNLOCK(drv_priv);

        for (wnet_vif_id = 0; wnet_vif_id < DEFAULT_MAX_VMAC; wnet_vif_id++)
        {
            wnet_vif = drv_priv->drv_wnet_vif_table[wnet_vif_id];
            if ((wnet_vif != NULL)
                && (drv_priv->net_ops->wifi_mac_pwrsave_is_wnet_vif_sleeping(wnet_vif) == 0))
            {
                DRV_PS_LOCK(drv_priv);
                drv_priv->add_wakeup_work = 1;
                DRV_PS_UNLOCK(drv_priv);

                drv_priv->net_ops->wifi_mac_pwrsave_wakeup_for_tx(wnet_vif);
                /* wake a wnet_vif operation can wake hal and firmware */
                break;
            }
        }
    }
    else
    {
        DRV_PS_LOCK(drv_priv);
        drv_priv->add_wakeup_work = 0;
        DRV_PS_UNLOCK(drv_priv);
    }

}
