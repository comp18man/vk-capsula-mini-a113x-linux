/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2018
 *
 * Project: 11BGNAC 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer power save  interface module
 *
 *
 ****************************************************************************************
 */
#include "wifi_mac_com.h"

struct workqueue_struct *pwrsave_sleep_wq = NULL;

static char *ips_sleep_reason[SLEEP_REASON_MAX];
static char *ips_netsleep_reason[NETSLEEP_REASON_MAX];
static char *ips_wakeup_reason[WAKEUP_REASON_MAX];

char *ips_state[WIFINET_PWRSAVE_STATE_MAX];

//sleep timer handler, to check if need to enter sleep
static void wifi_mac_pwrsave_presleep(struct work_struct *work)
{
    struct wifi_mac_pwrsave_t *ps = NULL;
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;

    ps = container_of(work, struct wifi_mac_pwrsave_t, ips_work_presleep);

    wnet_vif = container_of(ps, struct wlan_net_vif, vm_pwrsave);

    wifimac = wnet_vif->vm_wmac;
    if (wifimac == NULL)
        return;

    if (wnet_vif->vm_pwrsave.ips_sta_psmode == WIFINET_PWRSAVE_NONE)
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d power save mode not enabled\n",__func__,__LINE__);
        return;
    }

    if (wnet_vif->vm_mainsta == NULL)
    {
        pr_err("%s:%d, vm_mainsta is null, cancel sleep this time \n", __func__, __LINE__);
        os_timer_ex_start(&wnet_vif->vm_pwrsave.ips_timer_presleep);
        return;
    }
    //if conditions are stratified, will enter presleep. conditions must include:
    //a, ips_inactivitytime enough
    //b, not in WIFINET_PSQUEUE_PS4QUIET (mean fake powersave)
    //c, not in scan
    //d, not in listen (for p2p)
    //e, prestate is WIFINET_PWRSAVE_AWAKE
    //f, all queues have been sent completed
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (((wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_PS4QUIET) == 0)
         && ((wifimac->wm_flags & WIFINET_F_SCAN) == 0)
         && ((wnet_vif->vm_flags & WIFINET_F_NOSCAN) == 0)
         && (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_AWAKE)
         && (wifi_mac_all_txq_all_qcnt(wifimac) == 0)
         && (!(wnet_vif->vm_flags & WIFINET_F_WPA) || ((wnet_vif->vm_flags & WIFINET_F_WPA)
         && (wnet_vif->vm_mainsta->connect_status == CONNECT_DHCP_GET_ACK)))
         && !((wifimac->recovery_stat > WIFINET_RECOVERY_START) && (wifimac->recovery_stat < WIFINET_RECOVERY_END)))
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_state == WIFINET_S_CONNECTED))
        {
            //if sta is connected, then send nulldata with ps=1, and set flag for tx_completed
            //when tx_completed, will enter sleep
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d send nulldata to sleep\n",__func__,__LINE__);
            wifi_mac_pwrsave_send_nulldata(wnet_vif->vm_mainsta, NULLDATA_PS, 1);
        }
        else
        {
            if (wnet_vif->vm_opmode != WIFINET_M_HOSTAP)
            {
                //if sta is disconnected, just sleep
                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d disconnect, full sleep \n",__func__,__LINE__);
                wifi_mac_pwrsave_fullsleep(wnet_vif, SLEEP_AFTER_INACTIVITY_ENOUGH);
            }
        }
    } else {
        if (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_AWAKE) {
            DPRINTF(AML_DEBUG_PWR_SAVE, "not allow enter sleep this time:cnt %d, vm_pstxqueue_flags:%04x, wm_flags:%08x," \
                "vm_flags:%08x, state:%d, connect_status:%d\n",
                wifi_mac_all_txq_all_qcnt(wifimac), wnet_vif->vm_pstxqueue_flags, wifimac->wm_flags,
                wnet_vif->vm_flags, wnet_vif->vm_pwrsave.ips_state, wnet_vif->vm_mainsta->connect_status);
        }
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }
    os_timer_ex_start(&wnet_vif->vm_pwrsave.ips_timer_presleep);
}

static int wifi_mac_pwrsave_sleep_timer (void *arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *) arg;

    if (pwrsave_sleep_wq != NULL) {
        queue_work(pwrsave_sleep_wq, &(wnet_vif->vm_pwrsave.ips_work_presleep));
    }
    return OS_TIMER_NOT_REARMED;
}

static void  wifi_mac_pwrsave_wakeup_ex(SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;

    wifi_mac_pwrsave_sleep_wait_cancel(wnet_vif);
    wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_TRANSMIT);

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_state == WIFINET_S_CONNECTED))
    {
#ifdef CONFIG_P2P
        if (wnet_vif->vm_p2p->p2p_flag & P2P_OPPPS_START_FLAG_HI)
        {
            vm_p2p_client_cancel_opps(wnet_vif->vm_p2p);
        }
#endif
    }
}

void wifi_mac_pwrsave_wakeup_for_tx (struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    wifi_mac_add_work_task(wifimac, wifi_mac_pwrsave_wakeup_ex, NULL, (SYS_TYPE)wnet_vif, 0, 0, 0, 0);
}

void wifi_mac_pwrsave_restore_sleep(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if ((wnet_vif->vm_opmode != WIFINET_M_STA)
        || (wnet_vif->vm_pwrsave.ips_sta_psmode == WIFINET_PWRSAVE_NONE))
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d \n",__func__,__LINE__);
        wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_PSEXIT_UNKNOWN);
        return;
    }

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_NETWORK_SLEEP)
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        DPRINTF(AML_DEBUG_PWR_SAVE,"%s %d <%s> reason=%s\n",__func__,__LINE__,
                VMAC_DEV_NAME(wnet_vif),ips_sleep_reason[wnet_vif->vm_pwrsave.ips_sleep_wait_reason]);
#ifdef CONFIG_P2P
        if ((wnet_vif->vm_pwrsave.ips_sleep_wait_reason == SLEEP_AFTER_PS_TRIGGER_TIMEOUT)
            && (wnet_vif->vm_p2p->p2p_flag & P2P_OPPPS_CWEND_FLAG_HI))
        {
            wifimac->drv_priv->drv_ops.drv_p2p_client_opps_cwend_may_sleep(wnet_vif);
        }
        else
#endif
        {
            if (wifi_mac_all_txq_all_qcnt(wifimac) == 0)
            {
                wifi_mac_pwrsave_fullsleep(wnet_vif, wnet_vif->vm_pwrsave.ips_sleep_wait_reason);
            }
            else
            {
                wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_TRANSMIT);
            }
        }
    }
    else
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d reason %s, state %s, do nothing\n",
                __func__,__LINE__, ips_sleep_reason[wnet_vif->vm_pwrsave.ips_sleep_wait_reason],
                ips_state[wnet_vif->vm_pwrsave.ips_state]);
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }
}

void  wifi_mac_pwrsave_sleep_wait_ex(SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;

    if(wnet_vif->wnet_vif_replaycounter != (int)param5)
        return ;
    wifi_mac_pwrsave_restore_sleep(wnet_vif);
}

static int wifi_mac_pwrsave_sleep_wait (void *arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *) arg;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_flag_send_ps_trigger)
    {
        if (SLEEP_AFTER_PS_TRIGGER_TIMEOUT != wnet_vif->vm_pwrsave.ips_sleep_wait_reason)
        {
            DPRINTF(AML_DEBUG_PWR_SAVE,"%s %d ps trigger timer err\n",__func__,__LINE__);
        }
        wnet_vif->vm_pwrsave.ips_flag_send_ps_trigger = 0;
    }
    wnet_vif->vm_pwrsave.ips_flag_uapsd_trigger = 0;
    if (wnet_vif->vm_pwrsave.ips_flag_waitbeacon_timer_start)
    {
        if (SLEEP_AFTER_WAIT_BEACON_TIMEOUT != wnet_vif->vm_pwrsave.ips_sleep_wait_reason)
        {
            DPRINTF(AML_DEBUG_PWR_SAVE," %s %d beacon timer err\n",__func__,__LINE__);
        }
        wnet_vif->vm_pwrsave.ips_flag_waitbeacon_timer_start = 0;
    }
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);

    wifi_mac_add_work_task(wifimac,wifi_mac_pwrsave_sleep_wait_ex,NULL,
        (SYS_TYPE)arg,0,0,(SYS_TYPE)wnet_vif,(SYS_TYPE)wnet_vif->wnet_vif_replaycounter);
    return OS_TIMER_NOT_REARMED;
}

void wifi_mac_pwrsave_sleep_wait_cancel (struct wlan_net_vif *wnet_vif)
{
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_flag_waitbeacon_timer_start)
    {
        wnet_vif->vm_pwrsave.ips_flag_waitbeacon_timer_start = 0;
        os_timer_ex_cancel(&wnet_vif->vm_pwrsave.ips_timer_sleep_wait, CANCEL_NO_SLEEP);
        wnet_vif->vm_pwrsave.ips_sleep_wait_reason = SLEEP_NONE;
    }
    wnet_vif->vm_pwrsave.ips_flag_uapsd_trigger = 0;
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);
}

static int wifi_mac_pwrsave_set_state(struct wlan_net_vif *wnet_vif,
    enum wifinet_ps_state newstate)
{
    int status = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    enum wifinet_ps_state oldstate;

    if (wnet_vif->vm_pwrsave.ips_sta_psmode == WIFINET_PWRSAVE_NONE)
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s power save mode not enabled\n",__func__);
        return 0;
    }

    WIFINET_PWRSAVE_MUTEX_LOCK(wnet_vif);
    WIFINET_PWRSAVE_LOCK(wnet_vif);

    oldstate = wnet_vif->vm_pwrsave.ips_state;

    switch (oldstate)
    {
        case WIFINET_PWRSAVE_AWAKE:
            if (newstate == WIFINET_PWRSAVE_FULL_SLEEP)
            {
                status = 1;
            }
            break;
        case WIFINET_PWRSAVE_NETWORK_SLEEP:
            if ((newstate == WIFINET_PWRSAVE_AWAKE)
                || (newstate == WIFINET_PWRSAVE_FULL_SLEEP))
            {
                status = 1;
            }
            break;
        case WIFINET_PWRSAVE_FULL_SLEEP:
            if ((newstate == WIFINET_PWRSAVE_AWAKE)
                || (newstate == WIFINET_PWRSAVE_NETWORK_SLEEP))
            {
                status = 1;
            }
            break;
        default:
            break;
    }
    if (status)
    {
        wnet_vif->vm_pwrsave.ips_state = newstate;
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d <%s> , state %s -> %s\n",
            __func__,__LINE__, VMAC_DEV_NAME(wnet_vif), ips_state[oldstate], ips_state[newstate]);
    }
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    wifi_mac_com_ps_set_state(wifimac, newstate, wnet_vif->wnet_vif_id);
    WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
    return status;
}

int wifi_mac_pwrsave_networksleep (struct wlan_net_vif *wnet_vif,
        enum wifinet_ps_netsleep_reason reason)
{
    int status;

    status = wifi_mac_pwrsave_set_state(wnet_vif, WIFINET_PWRSAVE_NETWORK_SLEEP);
    return status;
}


int wifi_mac_pwrsave_fullsleep(struct wlan_net_vif *wnet_vif, 
    enum wifinet_ps_sleep_reason reason)
{
    int status;
    DPRINTF(AML_DEBUG_PWR_SAVE, "<%s>%s %d reason=%s\n",
        VMAC_DEV_NAME(wnet_vif), __func__,__LINE__, ips_sleep_reason[reason]);
    status = wifi_mac_pwrsave_set_state(wnet_vif, WIFINET_PWRSAVE_FULL_SLEEP);
    return status;
}


int wifi_mac_pwrsave_wakeup(struct wlan_net_vif *wnet_vif,
    enum wifinet_ps_wk_reason reason)
{
    int status;
    DPRINTF(AML_DEBUG_PWR_SAVE, "<%s>%s %d reason=%s\n",
        VMAC_DEV_NAME(wnet_vif), __func__,__LINE__, ips_wakeup_reason[reason]);
    status = wifi_mac_pwrsave_set_state(wnet_vif, WIFINET_PWRSAVE_AWAKE);
    return status;
}

int wifi_mac_pwrsave_wkup_and_NtfyAp (struct wlan_net_vif *wnet_vif,
        enum wifinet_ps_wk_reason reason)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "<%s>%s %d reason=%s\n",
        VMAC_DEV_NAME(wnet_vif), __func__,__LINE__, ips_wakeup_reason[reason]);
    wifi_mac_pwrsave_wakeup(wnet_vif, reason);

    //fix exception:send null data exit ps, in case no pkt back, restart timer here.
    os_timer_ex_start(&wnet_vif->vm_pwrsave.ips_timer_presleep);

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_state == WIFINET_S_CONNECTED))
    {
        wifi_mac_pwrsave_send_nulldata(wnet_vif->vm_mainsta, NULLDATA_NONPS, 1);
#ifdef CONFIG_P2P
        if (wnet_vif->vm_p2p->p2p_flag & P2P_OPPPS_START_FLAG_HI)
        {
            vm_p2p_client_cancel_opps(wnet_vif->vm_p2p);
        }
#endif
    }
    return 0;
}

void wifi_mac_pwrsave_set_mode(struct wlan_net_vif *wnet_vif, unsigned int mode)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "<%s>%s %d mode %d->%d\n",
        VMAC_DEV_NAME(wnet_vif),__func__,__LINE__, wnet_vif->vm_pwrsave.ips_sta_psmode, mode);

    if ((wnet_vif->vm_opmode != WIFINET_M_STA) || (mode > WIFINET_PWRSAVE_MAXIMUM)
        || (mode == wnet_vif->vm_pwrsave.ips_sta_psmode))
        return;

    if (mode == WIFINET_PWRSAVE_NONE)
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d stop ips_timer_presleep\n",__func__,__LINE__);
        os_timer_ex_cancel(&wnet_vif->vm_pwrsave.ips_timer_presleep, CANCEL_SLEEP);
#ifdef USER_UAPSD_TRIGGER
        os_timer_ex_cancel(&(wnet_vif->vm_pwrsave.ips_timer_uapsd_trigger), CANCEL_NO_SLEEP);
#endif
        wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_PSMODE_EXIT);
    }
    else
    {
        struct wifi_mac *wifimac = wnet_vif->vm_wmac;

        if (mode == WIFINET_PWRSAVE_LOW)
        {
            wnet_vif->vm_pwrsave.ips_inactivitytime = WIFINET_PS_LOW_INACTIVITYTIME;
        }
        else if (mode == WIFINET_PWRSAVE_NORMAL)
        {
            wnet_vif->vm_pwrsave.ips_inactivitytime = WIFINET_PS_NORMAL_INACTIVITYTIME;
        }
        else if (mode == WIFINET_PWRSAVE_MAXIMUM)
        {
            wnet_vif->vm_pwrsave.ips_inactivitytime = WIFINET_PS_MAXIMUM_INACTIVITYTIME;
        }

        if (wnet_vif->vm_state == WIFINET_S_CONNECTED && wifimac->wm_syncbeacon == 0)
        {
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            wifi_mac_beacon_sync(wifimac->drv_priv->wmac, wnet_vif->wnet_vif_id);
        }

        /*previous state is WIFINET_PWRSAVE_NONE, we will start timer now. */
        if (wnet_vif->vm_pwrsave.ips_sta_psmode == WIFINET_PWRSAVE_NONE)
        {
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d start ips_timer_presleep, ips_inactivitytime=%d\n",
                __func__,__LINE__, wnet_vif->vm_pwrsave.ips_inactivitytime);
            os_timer_ex_start_period(&wnet_vif->vm_pwrsave.ips_timer_presleep, wnet_vif->vm_pwrsave.ips_inactivitytime);
        }
    }
    wnet_vif->vm_pwrsave.ips_sta_psmode = mode;
}

void wifi_mac_pwrsave_wnet_vif_connect(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac_pwrsave_t        *ps = &wnet_vif->vm_pwrsave;

    //wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_VMAC_CONNECT);
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    ps->ips_ps_waitbeacon_timeout = WIFINET_PWRSAVE_WAITBEACON_TIMER_INTERVAL;
    ps->ips_ps_trigger_timeout = WIFINET_PWRSAVE_PSTRIGGER_TIMER_INTERVAL;
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    os_timer_ex_start_period(&wnet_vif->vm_pwrsave.ips_timer_presleep, wnet_vif->vm_pwrsave.ips_inactivitytime);
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d ips_ps_trigger_timeout=%dms, ips_ps_waitbeacon_timeout=%dms\n",
            __func__,__LINE__, ps->ips_ps_trigger_timeout, ps->ips_ps_waitbeacon_timeout);
}

void wifi_mac_pwrsave_wnet_vif_disconnect(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac_pwrsave_t  *ps = &wnet_vif->vm_pwrsave;
    
#ifdef CONFIG_P2P
    union type_ctw_opps_u ctw_opps_u;
    struct p2p_noa noa;
#endif

    //wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_VMAC_DISCONNECT);

#ifdef CONFIG_P2P
    if (wnet_vif->vm_opmode==WIFINET_M_STA)
    {
        struct wifi_mac_p2p *p2p = wnet_vif->vm_p2p;
        if (p2p->p2p_flag & P2P_OPPPS_START_FLAG_HI)
        {
            memset(&ctw_opps_u, 0 , sizeof(union type_ctw_opps_u));
            vm_p2p_opps_start(p2p, &ctw_opps_u);
        }
        if (p2p->p2p_flag & P2P_NOA_START_FLAG_HI)
        {
            memset(&noa, 0 , sizeof(struct p2p_noa));
            vm_p2p_noa_start(p2p, &noa);
        }
    }
#endif

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    ps->ips_ps_trigger_timeout = 0;
    ps->ips_ps_waitbeacon_timeout   = 0;
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    //os_timer_ex_start_period(&wnet_vif->vm_pwrsave.ips_timer_presleep, wnet_vif->vm_pwrsave.ips_inactivitytime);
}

void wifi_mac_pwrsave_attach(void)
{
    wifi_mac_pwrsave_reason_init();

    pwrsave_sleep_wq = create_singlethread_workqueue("aml_pwrsave_wq");
    if (pwrsave_sleep_wq == NULL)
        AML_OUTPUT("init wq err\n");
}

void wifi_mac_pwrsave_vattach(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac_pwrsave_t        *ps = &wnet_vif->vm_pwrsave;

    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d <%s>\n", __func__,__LINE__, VMAC_DEV_NAME(wnet_vif));

    wnet_vif->vif_ops.vm_set_tim = NULL;    //lg
    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP ||
        wnet_vif->vm_opmode == WIFINET_M_IBSS)
    {
        wnet_vif->vif_ops.vm_set_tim = wifi_mac_pwrsave_set_tim;
    }

    ps->ips_inactivitytime = WIFINET_PWRSAVE_TIMER_INTERVAL;
    ps->ips_sta_psmode = WIFINET_PWRSAVE_NONE;
    /*default value,  vo,vi support delivery-trigger enable, Max SP bit5bi6 = 0, receive all BUs from AP */
    wnet_vif->vm_uapsdinfo = WME_CAPINFO_UAPSD_VO | WME_CAPINFO_UAPSD_VI;
    ps->ips_state = WIFINET_PWRSAVE_AWAKE;

    os_timer_ex_initialize(&ps->ips_timer_presleep, ps->ips_inactivitytime,
                           wifi_mac_pwrsave_sleep_timer, wnet_vif);
    os_timer_ex_initialize(&ps->ips_timer_sleep_wait, 0,
                           wifi_mac_pwrsave_sleep_wait, wnet_vif);
#ifdef USER_UAPSD_TRIGGER
    os_timer_ex_initialize(&ps->ips_timer_uapsd_trigger, 30 /*30 ms */,
                           wifi_mac_pwrsave_sta_uapsd_trigger, wnet_vif);
#endif
    ps->ips_sleep_wait_reason = SLEEP_NONE;
    ps->ips_ps_trigger_timeout = 0;
    ps->ips_ps_waitbeacon_timeout  = 0;

    WIFINET_PWRSAVE_LOCK_INIT(wnet_vif);
    WIFINET_PWRSAVE_MUTEX_INIT(wnet_vif);

    INIT_WORK(&(ps->ips_work_presleep), wifi_mac_pwrsave_presleep);

    WIFINET_SAVEQ_INIT(&(wnet_vif->vm_tx_buffer_queue), "wnet_vif_tx_buffer_queue");
    wnet_vif->vm_pstxqueue_flags = 0;
    wnet_vif->vm_legacyps_txframes = 0;
    wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_VMAC_CREATE);

    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d ips_timer_presleep=%dms\n",
            __func__,__LINE__, WIFINET_PWRSAVE_TIMER_INTERVAL);
}

//cb for vmac created end
void wifi_mac_pwrsave_latevattach(struct wlan_net_vif *wnet_vif)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d <%s>\n", __func__,__LINE__, VMAC_DEV_NAME(wnet_vif));

    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
    {
        wnet_vif->vm_tim_len = howmany(wnet_vif->vm_max_aid, 8) * sizeof(unsigned char);
        wnet_vif->vm_tim_bitmap = (unsigned char *)NET_MALLOC(wnet_vif->vm_tim_len,
            GFP_ATOMIC, "wnet_vif->vm_tim_bitmap");

        if (wnet_vif->vm_tim_bitmap == NULL) {
            DPRINTF(AML_DEBUG_WARNING, "%s: no memory for TIM bitmap!\n", __func__);
            wnet_vif->vm_tim_len = 0;
        }
        wnet_vif->vm_ps_pending = 0;

        /* wlan0 change STA mode to HOSTAP, need wake up */
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
        {
            wnet_vif->vm_pwrsave.ips_sta_psmode = WIFINET_PWRSAVE_LOW;
            wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_VMAC_UP);
        }
        return;
    }

    wifi_mac_pwrsave_fullsleep(wnet_vif, SLEEP_AFTER_VMAC_CREATE);
}

void wifi_mac_pwrsave_detach(void)
{
    if (pwrsave_sleep_wq != NULL)
    {
        AML_OUTPUT("++\n");
        destroy_workqueue(pwrsave_sleep_wq);
        pwrsave_sleep_wq = NULL;
    }
}

void wifi_mac_pwrsave_vdetach(struct wlan_net_vif *wnet_vif)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d <%s>\n", __func__,__LINE__, VMAC_DEV_NAME(wnet_vif));
    //wifi_mac_pwrsave_fullsleep(wnet_vif, SLEEP_AFTER_VMAC_DEL);
    if (wnet_vif->vm_tim_bitmap != NULL)
    {
        FREE(wnet_vif->vm_tim_bitmap,"wnet_vif->vm_tim_bitmap");
        wnet_vif->vm_tim_bitmap = NULL;
    }
    wnet_vif->vm_ps_pending = 0;

    flush_work(&(wnet_vif->vm_pwrsave.ips_work_presleep));

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    os_timer_ex_cancel(&wnet_vif->vm_pwrsave.ips_timer_presleep, CANCEL_NO_SLEEP);
    os_timer_ex_cancel(&wnet_vif->vm_pwrsave.ips_timer_sleep_wait, CANCEL_NO_SLEEP);
    os_timer_ex_del(&wnet_vif->vm_pwrsave.ips_timer_presleep, CANCEL_SLEEP);
    os_timer_ex_del(&wnet_vif->vm_pwrsave.ips_timer_sleep_wait, CANCEL_SLEEP);
#ifdef USER_UAPSD_TRIGGER
    os_timer_ex_del(&(wnet_vif->vm_pwrsave.ips_timer_uapsd_trigger), CANCEL_SLEEP);
#endif
    wnet_vif->vm_pwrsave.ips_sleep_wait_reason = SLEEP_NONE;
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    WIFINET_NODE_LOCK_DESTROY(&wnet_vif->vm_pwrsave);
    WIFINET_SAVEQ_DESTROY(&wnet_vif->vm_tx_buffer_queue);
}

//cb for mac created
void wifi_mac_pwrsave_reason_init(void)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d \n", __func__,__LINE__);

    ips_sleep_reason[SLEEP_NONE] = "NONE";
    ips_sleep_reason[SLEEP_AFTER_VMAC_CREATE] = "VMAC_CREATE";
    ips_sleep_reason[SLEEP_AFTER_VMAC_UP] = "VMAC_UP";
    ips_sleep_reason[SLEEP_AFTER_VMAC_CONNECT] = "VMAC_CONNECT";
    ips_sleep_reason[SLEEP_AFTER_VMAC_DISCONNECT] = "VMAC_DISCONNECT";
    ips_sleep_reason[SLEEP_AFTER_VMAC_DOWN] = "VMAC_DOWN";
    ips_sleep_reason[SLEEP_AFTER_VMAC_DEL] = "VMAC_DEL";
    ips_sleep_reason[SLEEP_AFTER_INACTIVITY_ENOUGH] = "INACTIVITY_ENOUGH";
    ips_sleep_reason[SLEEP_AFTER_TX_NULL_WITH_PS] = "TX_NULL_WITH_PS";
    ips_sleep_reason[SLEEP_AFTER_BEACON] = "BEACON";
    ips_sleep_reason[SLEEP_AFTER_WAIT_BEACON_TIMEOUT] = "WAIT_BEACON_TIMEOUT";
    ips_sleep_reason[SLEEP_AFTER_PSPOLL_FINISHED] = "PSPOLL_FINISHED";
    ips_sleep_reason[SLEEP_AFTER_UAPSD_TRIGGER_FINISHED] = "UAPSD_TRIGGER_FINISHED";
    ips_sleep_reason[SLEEP_AFTER_PS_TRIGGER_TIMEOUT] = "PS_TRIGGER_TIMEOUT";
    ips_sleep_reason[SLEEP_AFTER_NOA_START] = "NOA_START";
    ips_sleep_reason[SLEEP_AFTER_NOA_END] = "NOA_END";
    ips_sleep_reason[SLEEP_AFTER_NOA_END_DUMMY] = "NOA_END_DUMMY";
    ips_sleep_reason[SLEEP_AFTER_OPPS_END] = "OPPS_END";
    ips_sleep_reason[SLEEP_AFTER_SUSPEND] = "SUSPEND";
    ips_sleep_reason[SLEEP_AFTER_UNKNOWN] = "UNKNOWN";

    ips_netsleep_reason[NETSLEEP_AFTER_WAKEUP] = "AFTER_WAKEUP";
    ips_netsleep_reason[NETSLEEP_AFTER_BEACON_SEND] = "BEACON_SEND";
    ips_netsleep_reason[NETSLEEP_AFTER_OPPS_END] = "OPPS_END";
    ips_netsleep_reason[NETSLEEP_AFTER_NOA] = "NOA";

    ips_wakeup_reason[WKUP_FROM_VMAC_CREATE] = "VMAC_CREATE";
    ips_wakeup_reason[WKUP_FROM_VMAC_UP] = "VMAC_UP";
    ips_wakeup_reason[WKUP_FROM_VMAC_CONNECT] = "VMAC_CONNECT";
    ips_wakeup_reason[WKUP_FROM_VMAC_DISCONNECT] = "VMAC_DISCONNECT";
    ips_wakeup_reason[WKUP_FROM_VMAC_DOWN] = "VMAC_DOWN";
    ips_wakeup_reason[WKUP_FROM_VMAC_DEL] = "VMAC_DEL";
    ips_wakeup_reason[WKUP_FROM_TRANSMIT] = "TRANSMIT";
    ips_wakeup_reason[WKUP_FROM_RECEIVE] = "RECEIVE";
    ips_wakeup_reason[WKUP_FROM_REMAIN_ACTIVE] = "REMAIN_ACTIVE";
    ips_wakeup_reason[WKUP_FROM_PSMODE_EXIT] = "PSMODE_EXIT";
    ips_wakeup_reason[WKUP_FROM_PSEXIT_UNKNOWN] = "PSEXIT_UNKNOWN";
    ips_wakeup_reason[WKUP_FROM_PSNULL_FAIL] = "PSNULL_FAIL";
    ips_wakeup_reason[WAKEUP_UNKNOWN] = "UNKNOWN";

    ips_state[WIFINET_PWRSAVE_AWAKE] = "AWAKE";
    ips_state[WIFINET_PWRSAVE_FULL_SLEEP] = "FULLSLEEP";
    ips_state[WIFINET_PWRSAVE_NETWORK_SLEEP] = "NETSLEEP";
}

/* sta operation functions  start */
int wifi_mac_pwrsave_is_sta_sleeping (struct wlan_net_vif *wnet_vif)
{
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if ((wnet_vif->vm_opmode == WIFINET_M_STA) &&
        (wnet_vif->vm_pwrsave.ips_sta_psmode > WIFINET_PWRSAVE_NONE) &&
        (wnet_vif->vm_pwrsave.ips_state > WIFINET_PWRSAVE_AWAKE))
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return 0;
    }
    else
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return -1;
    }
}

int wifi_mac_pwrsave_is_sta_fullsleep (struct wlan_net_vif *wnet_vif)
{
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if ((wnet_vif->vm_opmode == WIFINET_M_STA) &&
        (wnet_vif->vm_pwrsave.ips_sta_psmode > WIFINET_PWRSAVE_NONE) &&
        (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_FULL_SLEEP))
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return 0;
    }
    else
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return -1;
    }
}

int wifi_mac_pwrsave_is_wnet_vif_sleeping (struct wlan_net_vif *wnet_vif)
{
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_state > WIFINET_PWRSAVE_AWAKE)
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return 0;
    }
    else
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return -1;
    }
}

int wifi_mac_pwrsave_is_wnet_vif_fullsleep (struct wlan_net_vif *wnet_vif)
{
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_FULL_SLEEP)
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return 0;
    }
    else
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return -1;
    }
}

int wifi_mac_pwrsave_is_wnet_vif_networksleep (struct wlan_net_vif *wnet_vif)
{
    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_NETWORK_SLEEP)
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return 0;
    }
    else
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        return -1;
    }
}

int wifi_mac_pwrsave_if_ap_can_opps (struct wlan_net_vif *wnet_vif)
{
    if ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) &&
        (wnet_vif->vm_ps_sta > 0) && (wnet_vif->vm_ps_sta == wnet_vif->vm_sta_assoc))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void wifi_mac_buffer_txq_flush(struct sk_buff_head *pstxqueue)
{
    struct sk_buff *skb = NULL;
    unsigned int qlen_real = WIFINET_SAVEQ_QLEN(pstxqueue);

    if (qlen_real) {
        pr_debug("%s qlen_real:%d\n", __func__, qlen_real);
    }

    while (qlen_real)
    {
        WIFINET_SAVEQ_LOCK(pstxqueue);
        WIFINET_SAVEQ_DEQUEUE(pstxqueue, skb, qlen_real);
        WIFINET_SAVEQ_UNLOCK(pstxqueue);

        if (skb)
        {
            wifi_mac_free_skb(skb);
        }
    }

    return;
}


int wifi_mac_buffer_txq_enqueue (struct sk_buff_head *pstxqueue, struct sk_buff *skb)
{
    WIFINET_SAVEQ_LOCK(pstxqueue);
    WIFINET_SAVEQ_ENQUEUE(pstxqueue, skb);
    WIFINET_SAVEQ_UNLOCK(pstxqueue);
    return 0;
}

int wifi_mac_forward_txq_enqueue (struct sk_buff_head *fwdtxqueue, struct sk_buff *skb)
{
    WIFINET_SAVEQ_LOCK(fwdtxqueue);
    WIFINET_SAVEQ_ENQUEUE(fwdtxqueue, skb);
    WIFINET_SAVEQ_UNLOCK(fwdtxqueue);
    return 0;
}


int wifi_mac_buffer_txq_send(struct sk_buff_head *txqueue)
{
    struct sk_buff *skb = NULL, *tmp;
    struct wifi_station *sta;
    struct wifi_mac *wifimac;
    unsigned int qlen_real = WIFINET_SAVEQ_QLEN(txqueue);
    if (qlen_real == 0) {
        return qlen_real;
    }

    WIFINET_SAVEQ_LOCK(txqueue);
    skb_queue_walk_safe(txqueue,skb,tmp) {
        sta = os_skb_get_nsta(skb);
        if (!sta_find_in_sta_tbl(sta)) {
            AML_OUTPUT("not find sta: %p free skb\n",sta);
            aml_skb_unlink(skb,txqueue);
            wifi_mac_free_skb(skb);
        }
    }

    qlen_real = WIFINET_SAVEQ_QLEN(txqueue);
    WIFINET_SAVEQ_UNLOCK(txqueue);

    if (qlen_real == 0) {
        return qlen_real;
    }
#ifdef  CONFIG_CONCURRENT_MODE
    skb = WIFINET_SAVEQ_GET_TAIL(txqueue);
    if (skb) {
        sta = os_skb_get_nsta(skb);
        wifimac = sta->sta_wmac;
        if (wifimac->wm_vsdb_flags & CONCURRENT_CHANNEL_SWITCH) {
            DPRINTF(AML_DEBUG_CONNECT, "%s, break due to channel switch,vif:%d\n", __func__,sta->wnet_vif_id);
            return qlen_real;
        }
    }
#endif
    while (qlen_real) {
        WIFINET_SAVEQ_LOCK(txqueue);
        WIFINET_SAVEQ_DEQUEUE(txqueue, skb, qlen_real);
        WIFINET_SAVEQ_UNLOCK(txqueue);

        if (!skb) {
            break;
        }
        sta = os_skb_get_nsta(skb);
        wifimac = sta->sta_wmac;
        wifimac->drv_priv->drv_ops.tx_start(wifimac->drv_priv, skb);
    }
    return qlen_real;
}

int wifi_mac_buffer_txq_send_pre(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    if (!(wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_MASK))
    {
        wifimac->drv_priv->drv_ops.drv_hal_tx_frm_pause(wifimac->drv_priv, 0);
        wnet_vif->vm_phase_flags |= PHASE_TX_BUFF_QUEUE;
        wifi_mac_buffer_txq_send(&wnet_vif->vm_tx_buffer_queue);
        wnet_vif->vm_phase_flags &= ~PHASE_TX_BUFF_QUEUE;
        return 0;
    }

    return -1;
}


void wifi_mac_pwrsave_notify_txq_empty(struct wifi_mac *wifimac)
{

}

int wifi_mac_pwrsave_send_nulldata(struct wifi_station *sta,
    unsigned char pwr_save, unsigned char pwr_flag)
{
    if (sta == NULL) {
        return -1;
    }

    wifi_mac_send_nulldata(sta, pwr_save, pwr_flag, 0/*qos*/, 0/*ac*/);
    return 0;
}

static void wifi_mac_send_nulldata_retry_ex(SYS_TYPE param1,
                                            SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;
    wifi_mac_send_nulldata(wnet_vif->vm_mainsta, NULLDATA_PS, 1, 0, 0);
}

void wifi_mac_send_nulldata_retry(void * data)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)data;
    wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_send_nulldata_retry_ex,NULL,(SYS_TYPE)data,0,0,0,0);
}

void wifi_mac_pwrsave_send_pspoll(struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct sk_buff *skb;
    struct WIFINET_S_FRAME_ADDR2 *wh;
    struct wifi_skb_callback *cb;

    skb = wifi_mac_alloc_skb(wifimac, sizeof(struct WIFINET_S_FRAME_ADDR2));
    if (skb == NULL)
    {
        DPRINTF(AML_DEBUG_ERROR,"<%s> %s, alloc skb fail \n", VMAC_DEV_NAME(wnet_vif), __func__);
        return;
    }
    cb = (struct wifi_skb_callback *)skb->cb;
    cb->sta = sta;
    cb->flags = 0;
    cb->hdrsize = sizeof (struct WIFINET_S_FRAME_ADDR2);

    os_skb_set_priority(skb,WME_AC_VO);
    wh = (struct WIFINET_S_FRAME_ADDR2 *) os_skb_put(skb, sizeof(struct WIFINET_S_FRAME_ADDR2));
    *(unsigned short *)(&wh->i_aidordur) = htole16(0xc000 | WIFINET_NODE_AID(sta));

    WIFINET_ADDR_COPY(wh->i_addr1, sta->sta_bssid);
    WIFINET_ADDR_COPY(wh->i_addr2, wnet_vif->vm_myaddr);
    wh->i_fc[0] = 0;
    wh->i_fc[1] = 0;
    wh->i_fc[0] = WIFINET_FC0_VERSION_0 | WIFINET_FC0_TYPE_CTL | WIFINET_FC0_SUBTYPE_PS_POLL;

    if (WIFINET_VMAC_IS_SLEEPING(sta->sta_wnet_vif))
    {
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;
    }
    else if (wifi_mac_pwrsave_is_sta_sleeping(wnet_vif) == 0)
    {
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;
    }
    DPRINTF(AML_DEBUG_PWR_SAVE, "<%s>:%s aid=0x%x ps=%d\n",
        VMAC_DEV_NAME(wnet_vif), __func__, *(unsigned short *)(&wh->i_aidordur), (wh->i_fc[1]&WIFINET_FC1_PWR_MGT) != 0);

    wifi_mac_tx_mgmt_frm(wifimac, skb);
}

int wifi_mac_pwrsave_sta_uapsd_trigger (void *arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)arg;
    struct wifi_station *sta = wnet_vif->vm_mainsta;
    struct wifi_mac_pwrsave_t *ps = &wnet_vif->vm_pwrsave;
    int qosinfo = wnet_vif->vm_uapsdinfo;
    int send_trigger = 0;

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if ((ps->ips_flag_uapsd_trigger == 0)
        && (ps->ips_flag_send_ps_trigger == 0)
        && (wifi_mac_pwrsave_is_sta_sleeping(sta->sta_wnet_vif) == 0)
        && (sta ->sta_flags & WIFINET_NODE_UAPSD)
        && WME_STA_UAPSD_ENABLED(qosinfo))
    {
        ps->ips_flag_uapsd_trigger = 1;
        os_timer_ex_start_period(&ps->ips_timer_sleep_wait, ps->ips_ps_trigger_timeout);
        send_trigger = 1;
    }
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);

    if (send_trigger == 1)
    {
        pr_debug("%s:%d, send uapsd trigger\n", __func__, __LINE__);
        wifi_mac_send_qosnulldata_as_trigger(sta, qosinfo);
    }
    return OS_TIMER_REARMED;
}
void wifi_mac_pwrsave_sta_trigger (struct wlan_net_vif *wnet_vif)
{
    struct wifi_station *sta = wnet_vif->vm_mainsta;
    int qosinfo = wnet_vif->vm_uapsdinfo;
    struct wifi_mac_pwrsave_t        *ps = &wnet_vif->vm_pwrsave;
    char ps_timer_flag = 0;

    if ((sta ->sta_flags & WIFINET_NODE_UAPSD) &&
        WME_STA_UAPSD_ENABLED(qosinfo))
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s%d, sta send trigger\n", __func__, __LINE__);
        wifi_mac_send_qosnulldata_as_trigger(sta, qosinfo);
        ps_timer_flag = 1;
    }
    else
    {
        if (sta->sta_fetch_pkt_method == 1) {
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d send ps_poll\n",__func__,__LINE__);
            wifi_mac_pwrsave_send_pspoll(sta);
            ps_timer_flag = 1;
        } else if (sta->sta_fetch_pkt_method == 0) {
            wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_RECEIVE);
            DPRINTF(AML_DEBUG_PWR_SAVE,"%s %d wakeup due to ap cached pkt\n", __func__,__LINE__);
        }
    }

    if (ps_timer_flag) {
        WIFINET_PWRSAVE_LOCK(wnet_vif);
        ps->ips_sleep_wait_reason = SLEEP_AFTER_PS_TRIGGER_TIMEOUT;
        os_timer_ex_start_period(&ps->ips_timer_sleep_wait, ps->ips_ps_trigger_timeout);
        ps->ips_flag_send_ps_trigger = 1;
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }
}


void
wifi_mac_pwrsave_proc_tim(struct wlan_net_vif *wnet_vif)
{
    enum wifi_mac_psmode psmode = wnet_vif->vm_pwrsave.ips_sta_psmode;

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    if (wnet_vif->vm_pwrsave.ips_state == WIFINET_PWRSAVE_AWAKE)
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        //DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d recv tim in awake, recover\n", __func__, __LINE__);
        wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_RECEIVE);
        return;
    }
    else
    {
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }

    DPRINTF(AML_DEBUG_PWR_SAVE,"<%s> %s %d psmode=%d\n",
        VMAC_DEV_NAME(wnet_vif),__func__,__LINE__, psmode);
    if (WIFINET_PWRSAVE_LOW >= psmode)
    {
        wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_RECEIVE);
    }
    if (WIFINET_PWRSAVE_NORMAL <= psmode)
    {
        wifi_mac_pwrsave_sta_trigger(wnet_vif);
    }
}

void     wifi_mac_pwrsave_proc_dtim(struct wlan_net_vif *wnet_vif)
{
    DPRINTF(AML_DEBUG_PWR_SAVE,"<%s> %s %d \n",VMAC_DEV_NAME(wnet_vif),__func__,__LINE__);
    wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_RECEIVE);
}

void wifi_mac_pwrsave_check_ps_end(void * ieee,
        struct sk_buff * skbbuf, void * nsta,struct wifi_mac_rx_status* rs)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    struct wifi_station *sta = nsta;
    struct wifi_qos_frame *qwh;
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac_pwrsave_t        *ps = NULL;
    int ac_triggered_delivery_flag = 0;
    int ps_over_flag = PS_OVER_FLAG_INIT;
    int qosinfo;
    int ac = -1;

    if (sta == NULL)
    {
        sta = wifi_mac_find_rx_sta(wifimac, (struct wifi_mac_frame_min *)
                                 os_skb_data(skbbuf),rs->rs_wnet_vif_id);
        if (sta == NULL)
        {
            return;
        }
    }

    wnet_vif = sta->sta_wnet_vif;
    ps = &wnet_vif->vm_pwrsave;
    qwh = (struct wifi_qos_frame *) os_skb_data(skbbuf);
    if ((ps->ips_flag_send_ps_trigger == 0 && ps->ips_flag_uapsd_trigger == 0)
        || (wifi_mac_pwrsave_is_sta_sleeping(wnet_vif) != 0)
        || ((qwh->i_fc[0] & WIFINET_FC0_TYPE_MASK) != WIFINET_FC0_TYPE_DATA))
    {
        return;
    }

    WIFINET_PWRSAVE_LOCK(wnet_vif);
    os_timer_ex_cancel(&ps->ips_timer_sleep_wait, CANCEL_NO_SLEEP);
    ps->ips_sleep_wait_reason = SLEEP_NONE;
    WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    qosinfo = wnet_vif->vm_uapsdinfo;

    if (((qwh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_QOS)
        || (((qwh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_QOS_NULL)
                && (sta ->sta_flags & WIFINET_NODE_UAPSD)))
    {
        int tid = qwh->i_qos[0] & WIFINET_QOS_TID;
        ac = TID_TO_WME_AC(tid);
        if (WME_STA_UAPSD_AC_ENABLED(ac, qosinfo))
        {
            DPRINTF(AML_DEBUG_PWR_SAVE, "<running> %s %d uapsd tid=%d ac=%d more=%d eosp=%d\n",
                __func__,__LINE__, tid, ac, (qwh->i_fc[1] & WIFINET_FC1_MORE_DATA) != 0,
                (qwh->i_qos[0] & WIFINET_QOS_EOSP) != 0);

            ac_triggered_delivery_flag = 1;
            if (!(qwh->i_fc[1] & WIFINET_FC1_MORE_DATA))
            {
                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d U-APSD, no more data, uapsd trigger:%d\n",
                    __func__,__LINE__,ps->ips_flag_uapsd_trigger);
                if (ps->ips_flag_uapsd_trigger == 1)
                {
                    ps->ips_flag_uapsd_trigger = 0;
                    return;
                }
                else
                    ps_over_flag = PS_OVER_FLAG_UAPSD;
            }
            else
            {
                if (qwh->i_qos[0] & WIFINET_QOS_EOSP)
                {
                    wifi_mac_send_qosnulldata_as_trigger(sta, qosinfo);
                }
            }
        }
    }

    if (!ac_triggered_delivery_flag)
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "<running> %s %d legacy ac=%d more=%d\n",
            __func__,__LINE__, ac, qwh->i_fc[1] & WIFINET_FC1_MORE_DATA);
        if (!(qwh->i_fc[1] & WIFINET_FC1_MORE_DATA))
        {
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d legacy, no more data\n",__func__,__LINE__);
            ps_over_flag = PS_OVER_FLAG_PSPOLL;
        }
        else
        {
            wifi_mac_pwrsave_send_pspoll(sta);
        }
    }

    if (ps_over_flag == PS_OVER_FLAG_UAPSD)
    {
        if (WME_STA_UAPSD_ALL_AC_ENABLED(qosinfo))
        {
            WIFINET_PWRSAVE_LOCK(wnet_vif);
            ps->ips_sleep_wait_reason = SLEEP_AFTER_UAPSD_TRIGGER_FINISHED;
            WIFINET_PWRSAVE_UNLOCK(wnet_vif);
            wifi_mac_pwrsave_restore_sleep(wnet_vif);
            ps_over_flag = PS_OVER_FLAG_ALL;
        }
        else
        {
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d legacy, send ps-poll\n",__func__,__LINE__);
            wifi_mac_pwrsave_send_pspoll(sta);
        }
    }
    if (ps_over_flag == PS_OVER_FLAG_PSPOLL)
    {
        if ((sta ->sta_flags & WIFINET_NODE_UAPSD) &&
            WME_STA_UAPSD_ENABLED(qosinfo))
        {
            WIFINET_PWRSAVE_LOCK(wnet_vif);
            ps->ips_sleep_wait_reason = SLEEP_AFTER_PSPOLL_FINISHED;
            WIFINET_PWRSAVE_UNLOCK(wnet_vif);
            wifi_mac_pwrsave_restore_sleep(wnet_vif);
            ps_over_flag = PS_OVER_FLAG_ALL;
        }
        else
        {
            WIFINET_PWRSAVE_LOCK(wnet_vif);
            ps->ips_sleep_wait_reason = SLEEP_AFTER_PSPOLL_FINISHED;
            WIFINET_PWRSAVE_UNLOCK(wnet_vif);
            wifi_mac_pwrsave_restore_sleep(wnet_vif);
            ps_over_flag = PS_OVER_FLAG_ALL;
        }
    }

    if ((ps_over_flag == PS_OVER_FLAG_ALL)
#ifdef CONFIG_P2P
        || P2P_NoA_START_FLAG(wnet_vif->vm_p2p->HiP2pNoaCountNow)
#endif
       )
    {
        ps->ips_flag_send_ps_trigger = 0;
#ifdef CONFIG_P2P
        if (wnet_vif->vm_p2p->p2p_flag & P2P_OPPPS_CWEND_FLAG_HI)
        { 
             wifimac->drv_priv->drv_ops.drv_p2p_client_opps_cwend_may_sleep(wnet_vif);
        }
        if (ps_over_flag != PS_OVER_FLAG_ALL)
        {
            sta->sta_flags_ext |= WIFINET_NODE_TRIGGER_WAIT_NOA_END;
        }
#endif
    }
    else
    {
        WIFINET_PWRSAVE_LOCK(wnet_vif);
        ps->ips_sleep_wait_reason = SLEEP_AFTER_PS_TRIGGER_TIMEOUT;
        os_timer_ex_start_period(&ps->ips_timer_sleep_wait, ps->ips_ps_trigger_timeout);
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }

    return;
}

/* sta operation functions  end */


/* ap operation functions  start */
int wifi_mac_pwrsave_txpre (struct sk_buff *skb)
{
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *) skb->cb;
    struct wifi_station *sta = cb->sta;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int ret = -1;

    do
    {
        if (M_FLAG_GET(skb, M_PWR_SAV_BYPASS))
        {
            break;
        }
        if (WME_UAPSD_NODE_AC_CAN_TRIGGER(os_skb_get_priority(skb), sta))
        {
            M_FLAG_SET(skb, M_UAPSD);
            WIFINET_NODE_STAT(sta, tx_uapsd);
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d uapsd\n", __func__,__LINE__);
            if ((wnet_vif->vif_ops.vm_set_tim != NULL) && WME_UAPSD_NODE_ALL_AC_CAN_TRIGGER(sta))
            {
                wnet_vif->vif_ops.vm_set_tim(sta, 1);
            }
            ret = 1;
        }
        else if (sta->sta_flags & WIFINET_NODE_PWR_MGT)
        {
            M_PWR_SAV_SET(skb);
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d legacy ps\n", __func__,__LINE__);
            ret = 2;
        }
        break;
    }
    while (0);
    return ret;
}

int wifi_mac_pwrsave_psqueue_clean(struct wifi_station *sta)
{
    struct sk_buff *skb;
    int qlen;

    WIFINET_SAVEQ_LOCK(&(sta->sta_pstxqueue));
    qlen = skb_queue_len(&sta->sta_pstxqueue);

    while ((skb = __skb_dequeue(&sta->sta_pstxqueue)) != NULL)
        wifi_mac_free_skb(skb);

    WIFINET_SAVEQ_UNLOCK(&(sta->sta_pstxqueue));

    return qlen;
}


int
wifi_mac_pwrsave_psqueue_age(struct wifi_station *sta,
                            struct sk_buff_head *skb_freeqp)
{
    int discard = 0;
    int len = WIFINET_SAVEQ_QLEN(&(sta->sta_pstxqueue));

    if (len != 0)
    {
        struct sk_buff *skb;

        WIFINET_SAVEQ_LOCK(&(sta->sta_pstxqueue));
        while ((skb = skb_peek(&sta->sta_pstxqueue)) != NULL) {
            if (M_AGE_GET(skb) < WIFINET_INACT_WAIT) {
                WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                    "discard frame, age %u", M_AGE_GET(skb));

                skb = __skb_dequeue(&sta->sta_pstxqueue);
                WIFINET_SAVEQ_ENQUEUE(skb_freeqp, skb);
                discard++;

            } else {
                //go through the left sk_buff and decrease the age
                len -= discard;

                while (len--) {
                    M_AGE_SUB(skb, WIFINET_INACT_WAIT);
                    skb = skb->next;
                }
                break;
            }
        }

        WIFINET_SAVEQ_UNLOCK(&(sta->sta_pstxqueue));

        WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                             "discard %u frames for age", discard);
        WIFINET_NODE_STAT_ADD(sta, ps_discard, discard);
    }
    return discard;
}

void wifi_mac_pwrsave_set_tim(struct wifi_station *sta, int set)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    unsigned short aid;

    KASSERT(wnet_vif->vm_opmode == WIFINET_M_HOSTAP ||
            wnet_vif->vm_opmode == WIFINET_M_IBSS,
            ("operating mode %u", wnet_vif->vm_opmode));

    aid = WIFINET_AID(sta->sta_associd);
    KASSERT(aid < wnet_vif->vm_max_aid,
            ("bogus aid %u, max %u", aid, wnet_vif->vm_max_aid));

    WIFINET_BEACON_LOCK(sta->sta_wmac);
    if (set != (isset(wnet_vif->vm_tim_bitmap, aid) != 0))
    {
        if (set)
        {
            setbit(wnet_vif->vm_tim_bitmap, aid);
            wnet_vif->vm_ps_pending++;
        }
        else
        {
            clrbit(wnet_vif->vm_tim_bitmap, aid);
            wnet_vif->vm_ps_pending--;
        }
        wnet_vif->vm_flags |= WIFINET_F_TIMUPDATE;
    }
    WIFINET_BEACON_UNLOCK(sta->sta_wmac);
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d set=%d\n", __func__,__LINE__, set);
}

/*ap set tim for power save station */
void wifi_mac_pwrsave_psqueue_enqueue(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    int qlen, age;

    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d save data for tx to ps sta\n", __func__,__LINE__);
    if (WIFINET_SAVEQ_QLEN(&sta->sta_pstxqueue) >= WIFINET_PS_MAX_QUEUE)
    {
        WIFINET_NODE_STAT(sta,psq_drops);
        WIFINET_DPRINTF_STA(AML_DEBUG_ANY, sta, "pwr save q overflow, (size %d)", WIFINET_PS_MAX_QUEUE);
        wifi_mac_free_skb(skb);
        return;
    }

    WIFINET_SAVEQ_LOCK(&sta->sta_pstxqueue);
    age = (sta->sta_listen_intval * wnet_vif->vm_bcn_intval) / 1000;//wm_inact_timer is 1 seconds
    age = (age ? age : 1);

    WIFINET_SAVEQ_ENQUEUE_AGE(&sta->sta_pstxqueue, skb, age);
    qlen = WIFINET_SAVEQ_QLEN(&sta->sta_pstxqueue);
    WIFINET_SAVEQ_UNLOCK(&sta->sta_pstxqueue);

    WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta, "save frame, %u now queued", qlen);

    if (qlen == 1 && wnet_vif->vif_ops.vm_set_tim != NULL)
        wnet_vif->vif_ops.vm_set_tim(sta, 1);
}

static int wifi_mac_pwrsave_psqueue_send (struct wifi_station *sta, int force)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct sk_buff *skb;
    int qlen = 0;
    struct wifi_mac * wifimac = wnet_vif->vm_wmac;

#ifdef DRV_SUPPORT_TX_WITHDRAW
    struct driver_ops* ops = wifimac->drv_ops;
#endif

    struct wifi_mac_tx_info *txinfo = NULL;
    do
    {
#ifdef DRV_SUPPORT_TX_WITHDRAW
        {
            struct aml_driver_nsta *drv_sta = DRIVER_NODE(sta->drv_sta);

            if (drv_sta->sta_txwd_legacyps_qqcnt)
            {
                ops->drv_tx_withdraw_legacyps_send(wifimac->drv_priv, drv_sta);
                qlen = WIFINET_SAVEQ_QLEN(&(sta->sta_pstxqueue)) + drv_sta->sta_txwd_legacyps_qqcnt;
                break;
            }
        }
#endif
        WIFINET_SAVEQ_LOCK(&sta->sta_pstxqueue);
        WIFINET_SAVEQ_DEQUEUE(&sta->sta_pstxqueue, skb, qlen);
        WIFINET_SAVEQ_UNLOCK(&sta->sta_pstxqueue);
        if (skb == NULL)
        {
            if (force)
            {
                wifi_mac_send_nulldata_for_ap(sta, 0, 0, 0, 0);
                wnet_vif->vif_sts.sts_tx_ps_no_data++;
                pr_debug("tx null for pspoll\n");
            }
            qlen = 0;
            break;
        }

        if (sta->sta_flags & WIFINET_NODE_PWR_MGT)
        {
            M_FLAG_SET(skb, M_PWR_SAV_BYPASS);
        } else {
            M_FLAG_CLR(skb, M_PWR_SAV);
        }

        if (!qlen)
        {
            struct wifi_frame  *wh = (struct wifi_frame *)os_skb_data(skb);
            wh->i_fc[1] &= ~WIFINET_FC1_MORE_DATA;
        }
        WIFINET_PWRSAVE_LOCK(wnet_vif);
        txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);
        if (txinfo->b_buffered == 0)
        {
            /* for p2p noa and opps coexist*/
            txinfo->b_buffered = 1;
            wnet_vif->vm_legacyps_txframes++;
        }
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
        wifimac->drv_priv->drv_ops.tx_start(wifimac->drv_priv, skb);
    }
    while (0);

    return qlen;
}

int wifi_mac_pwrsave_psqueue_flush (struct wifi_station *sta)
{
    int qlen = 0;
#ifdef CONFIG_P2P
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
#endif

    for (;;)
    {
        qlen = wifi_mac_pwrsave_psqueue_send(sta, 0);
        if (qlen <= 0)
        {
            break;
        }
#ifdef CONFIG_P2P
        if (P2P_NoA_START_FLAG(wnet_vif->vm_p2p->HiP2pNoaCountNow))
        {
            if (qlen > 0)
            {
                sta->sta_flags_ext |= WIFINET_NODE_PS_FLUSH_WAIT_NOA_END;
            }
            break;
        }
#endif
    }
    return qlen;
}


void wifi_mac_pwrsave_state_change(struct wifi_station *sta, int enable)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d enable=%d\n", __func__,__LINE__, !!enable);
    KASSERT(wnet_vif->vm_opmode == WIFINET_M_HOSTAP ||
            wnet_vif->vm_opmode == WIFINET_M_IBSS,
            ("unexpected operating mode %u", wnet_vif->vm_opmode));

    if (enable)
    {
        if ((sta->sta_flags & WIFINET_NODE_PWR_MGT) == 0)
            wnet_vif->vm_ps_sta++;
        sta->sta_flags |= WIFINET_NODE_PWR_MGT;

        WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                             "power save mode on, %u sta's in ps mode", wnet_vif->vm_ps_sta);
        return;
    }

    if ((sta->sta_flags & WIFINET_NODE_PWR_MGT))
    {
        /*FIXME: if receive a frame with ps=0 when go opps end, return */
        if ((wnet_vif->vm_pstxqueue_flags & (WIFINET_PSQUEUE_OPPS | WIFINET_PSQUEUE_NOA)) != 0)
        {
            return;
        }
        wnet_vif->vm_ps_sta--;
        sta->sta_flags &= ~WIFINET_NODE_PWR_MGT;
        if (wifi_mac_pwrsave_is_wnet_vif_sleeping(wnet_vif) == 0)
        {
            wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_RECEIVE);
        }
    }

    WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                         "power save mode off, %u sta's in ps mode", wnet_vif->vm_ps_sta);
    wifi_mac_pwrsave_psqueue_flush(sta);

    wnet_vif->vif_ops.vm_set_tim(sta, 0);
}


void wifi_mac_pwrsave_recv_pspoll(struct wifi_station *sta, struct sk_buff *skb0)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_frame_min *wh;
    unsigned short aid;
    int qlen, arg;

    wh = (struct wifi_mac_frame_min *)skb0->data;
    if (sta->sta_associd == 0)
    {
        WIFINET_DPRINTF(
            AML_DEBUG_PWR_SAVE | AML_DEBUG_DEBUG,
            "ps-poll %s","unassociated station");
        wnet_vif->vif_sts.sts_rx_ps_uncnnt++;
        arg = WIFINET_REASON_NOT_ASSOCED;
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&arg);
        return;
    }

    aid = le16toh(*(unsigned short *)wh->i_dur);
    if (aid != sta->sta_associd)
    {
        WIFINET_DPRINTF(AML_DEBUG_PWR_SAVE | AML_DEBUG_DEBUG,
                        "ps-poll aid mismatch: sta aid 0x%x poll aid 0x%x", sta->sta_associd, aid);
        wnet_vif->vif_sts.sts_rx_ps_aid_err++;
        arg = WIFINET_REASON_NOT_ASSOCED;
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&arg);
        return;
    }

    qlen = wifi_mac_pwrsave_psqueue_send(sta, 1);

    if (qlen == 0)
    {
        WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                             "%s", "recv ps-poll, send packet, queue empty");
        if (wnet_vif->vif_ops.vm_set_tim != NULL)
            wnet_vif->vif_ops.vm_set_tim(sta, 0);
    }
    else
    {
        WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                             "recv ps-poll, send packet, %u still queued", qlen);
    }
}

void wifi_mac_pwrsave_chk_uapsd_trig(void * ieee,
        struct sk_buff * skbbuf, void * nsta,struct wifi_mac_rx_status* rs)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    struct wifi_station *sta = nsta;
    struct wifi_qos_frame *qwh;
    struct wlan_net_vif *wnet_vif = NULL;

    if (sta == NULL)
    {

        sta = wifi_mac_find_rx_sta(wifimac, (struct wifi_mac_frame_min *)
                                 os_skb_data(skbbuf),rs->rs_wnet_vif_id);
        if (sta == NULL)
        {
            return;
        }
    }

    wnet_vif = sta->sta_wnet_vif;
    qwh = (struct wifi_qos_frame *) os_skb_data(skbbuf);
    if (!WIFINET_VMAC_UAPSD_ENABLED(wnet_vif) ||
        ((WIFINET_M_HOSTAP != wnet_vif->vm_opmode) && (WIFINET_M_IBSS != wnet_vif->vm_opmode)) ||
        !(sta->sta_flags & WIFINET_NODE_UAPSD))
        goto end;

    if ((((qwh->i_fc[1] & WIFINET_FC1_PWR_MGT) == WIFINET_FC1_PWR_MGT) ^
         ((sta->sta_flags & WIFINET_NODE_UAPSD_TRIG) == WIFINET_NODE_UAPSD_TRIG)))
    {
        sta->sta_flags &= ~WIFINET_NODE_UAPSD_SP;
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d rx nsta ps change, ps=%d\n",
                __func__,__LINE__, !!(qwh->i_fc[1] & WIFINET_FC1_PWR_MGT));

        //if the first trigger, flag it
        if (qwh->i_fc[1] & WIFINET_FC1_PWR_MGT)
        {
            WME_UAPSD_NODE_TRIGSEQINIT(sta);
            sta->sta_flags |= WIFINET_NODE_UAPSD_TRIG;
        }
        else
        {
            //if exit powersave,
            sta->sta_flags &= ~WIFINET_NODE_UAPSD_TRIG;
            wifimac->drv_priv->drv_ops.process_uapsd_trigger(wifimac->drv_priv,
                                                    sta->drv_sta,
                                                    WME_UAPSD_NODE_MAXQDEPTH, 0, 1);
        }

        goto end;
    }

    if ( ((qwh->i_fc[0] & WIFINET_FC0_TYPE_MASK) != WIFINET_FC0_TYPE_DATA ) ||
         ( ((qwh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) != WIFINET_FC0_SUBTYPE_QOS) &&
           ((qwh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) != WIFINET_FC0_SUBTYPE_QOS_NULL)))
    {
        goto end;
    }

    {
        int tid = qwh->i_qos[0] & WIFINET_QOS_TID;
        int ac = TID_TO_WME_AC(tid);

        if (WME_UAPSD_NODE_AC_CAN_TRIGGER(ac, sta))
        {
            unsigned short frame_seq;
            int queue_qcnt;
            /*
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d rx nsta trigger tid=%d ac=%d\n",
                __func__,__LINE__, tid, ac);
            */
            pr_debug("rx uapsd trigger\n");

            frame_seq = le16toh(*(unsigned short *)qwh->i_seq);
            if ((qwh->i_fc[1] & WIFINET_FC1_RETRY) &&
                frame_seq == sta->sta_uapsd_trigseq[ac])
            {
                goto end;
            }


            if (sta->sta_flags & WIFINET_NODE_UAPSD_SP)
            {
                goto end;
            }

            sta->sta_flags |= WIFINET_NODE_UAPSD_SP;
            sta->sta_uapsd_trigseq[ac] = frame_seq;

            queue_qcnt = wifimac->drv_priv->drv_ops.process_uapsd_trigger(wifimac->drv_priv,
                         sta->drv_sta,
                         sta->sta_uapsd_maxsp, ac, 0);
            if (!queue_qcnt &&
                (sta->sta_wnet_vif->vif_ops.vm_set_tim != NULL) &&
                WME_UAPSD_NODE_ALL_AC_CAN_TRIGGER(sta))
            {
                sta->sta_wnet_vif->vif_ops.vm_set_tim(sta, 0);
            }
        }
    }
end:
    return;
}

void wifi_mac_pwrsave_eosp_indicate(void* nsta,  struct sk_buff * skbbuf, int txok)
{
    struct wifi_qos_frame *qwh;
    struct wifi_station *sta;

    qwh = (struct wifi_qos_frame *) os_skb_data(skbbuf);
    sta = (struct wifi_station *)nsta;

    if ((qwh->i_fc[0] == (WIFINET_FC0_SUBTYPE_QOS|WIFINET_FC0_TYPE_DATA)) ||
        (qwh->i_fc[0] == (WIFINET_FC0_SUBTYPE_QOS_NULL|WIFINET_FC0_TYPE_DATA)))
    {
        if ((qwh->i_qos[0] & WIFINET_QOS_EOSP) || 
            (sta->sta_flags_ext & WIFINET_NODE_UAPSD_WAIT_NOA_END))
        {
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d\n", __func__,__LINE__);
            sta->sta_flags &= ~WIFINET_NODE_UAPSD_SP;
            sta->sta_flags_ext &= ~WIFINET_NODE_UAPSD_WAIT_NOA_END;
        }
    }
    return;
}
/* ap operation functions  end */

int wifi_mac_pwrsave_wow_sta(struct wlan_net_vif *wnet_vif)
{
    /*
    * bitmask where to match pattern and where to ignore
    * bytes, one bit per byte
    */
    unsigned char mask = 0x3f;
    struct wifi_mac* wifimac = wnet_vif->vm_wmac;

    /* setup unicast pkt pattern */
    wifimac->drv_priv->drv_ops.drv_set_pattern(wifimac->drv_priv,
        wnet_vif->wnet_vif_id, 0, WIFINET_ADDR_LEN, 0, &mask, wnet_vif->vm_myaddr);

    return 0;
}

int wifi_mac_pwrsave_wow_usr(struct wlan_net_vif *wnet_vif,
			  struct cfg80211_wowlan *wow, unsigned int *filter)
{
    int i;
    struct wifi_mac* wifimac = wnet_vif->vm_wmac;

    /*
    * Configure the patterns that we received from the user.
    * And we save WOW_MAX_FILTERS patterns at most.
    */
    for (i = 0; i < wow->n_patterns; i++)
    {
        /*
        * Note: Pattern's offset is not passed as part of wowlan
        * parameter from CFG layer. So it's always passed as ZERO
        * to the firmware. It means, given WOW patterns are always
        * matched from the first byte of received pkt in the firmware.
        */
        wifimac->drv_priv->drv_ops.drv_set_pattern(wifimac->drv_priv,
            wnet_vif->wnet_vif_id, 0, wow->patterns[i].pattern_len, i,
            ( unsigned char *)wow->patterns[i].mask, (unsigned char *)wow->patterns[i].pattern);
    }

    /*get wakeup filter */
    if (wow->disconnect)
        *filter |= WOW_FILTER_OPTION_DISCONNECT;

    if (wow->magic_pkt)
        *filter |= WOW_FILTER_OPTION_MAGIC_PACKET;

    if (wow->gtk_rekey_failure)
        *filter |= WOW_FILTER_OPTION_GTK_ERROR;

    if (wow->eap_identity_req)
        *filter |= WOW_FILTER_OPTION_EAP_REQ;

    if (wow->four_way_handshake)
        *filter |= WOW_FILTER_OPTION_4WAYHS;

    return 0;
}

int wifi_mac_pwrsave_wow_suspend(SYS_TYPE param1,
                                SYS_TYPE param2,SYS_TYPE param3,
                                SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;
    struct wlan_net_vif *wnet_vif_next = NULL, *wnet_vif_tmp = NULL;
    struct cfg80211_wowlan *wow = (struct cfg80211_wowlan *)param3;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int listen_interval = 0, connect = 0;
    unsigned int filter = 0, cnt = 0;

    pr_debug("%s:%d\n", __func__, __LINE__);
    WIFINET_PWRSAVE_MUTEX_LOCK(wnet_vif);
    if (wifimac->wm_suspend_mode == WIFI_SUSPEND_STATE_WOW)
    {
        WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
        return 0;
    }

    DPRINTF(AML_DEBUG_PWR_SAVE, "<%s>:%s %d scan abort when host suspend \n",
        wnet_vif->vm_ndev->name,__func__, __LINE__);
    wnet_vif->vm_scan_hang = 1;
    wifi_mac_cancel_scan(wifimac);
    /* waiting for completing scan process */
    while (wifimac->wm_flags & WIFINET_F_SCAN)
    {
        msleep(20);
        if (cnt++ > 20)
        {
            ERROR_DEBUG_OUT("<%s>:wait scan end fail when host suspend \n",
                wnet_vif->vm_ndev->name);
            wnet_vif->vm_scan_hang = 0;
            WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
            return -1;
        }
    }

    /*stop kernel transmitting data to net dev */
    netif_stop_queue(wnet_vif->vm_ndev);

    /*check if tx buffer is clean. */
    if (wifi_mac_all_txq_all_qcnt(wifimac) != 0)
    {
        #if 1
        /* need flush all txdata(wlan0+p2p0) before suspend */
        wifimac->drv_priv->drv_ops.drv_flush_txdata(wifimac->drv_priv, 3/* wlan0+p2p0 */);
        #else
        /* need to wait for transmitting data out. waitting suspend
         * command again and try to suspend wifi.
         */
        wnet_vif->vm_scan_hang = 0;
        WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
        return -EINVAL;
        #endif
    }

    list_for_each_entry_safe(wnet_vif_tmp, wnet_vif_next, &wifimac->wm_wnet_vifs, vm_next)
    {
        /* now, not support suspend ap. */
        if ((wnet_vif_tmp->vm_opmode == WIFINET_M_HOSTAP)
            || (wnet_vif_tmp->vm_state != WIFINET_S_CONNECTED)) {
            continue;
        }
        connect++;
        /*
        * Skip the default WOW pattern configuration if the driver receives any
        * WOW patterns from the user.
        */
        if (wow != NULL)
            wifi_mac_pwrsave_wow_usr(wnet_vif_tmp, wow, &filter);
        else
            wifi_mac_pwrsave_wow_sta(wnet_vif_tmp);

        /*set arp agent */
        wifi_mac_set_arp_agent(wnet_vif_tmp, ENABLE);

        /* change beacon listen interval to dtim period. */
        if (wnet_vif_tmp->vm_dtim_period > 0)
            listen_interval = wnet_vif_tmp->vm_bcn_intval * wnet_vif_tmp->vm_dtim_period;
        else
            listen_interval = wnet_vif_tmp->vm_bcn_intval;
        wifimac->drv_priv->drv_ops.Phy_beaconinit(wifimac->drv_priv,wnet_vif_tmp->wnet_vif_id, (1<<16) | listen_interval);

        /* change beacon miss timer period */
        wifi_mac_set_beacon_miss_ex(wnet_vif_tmp, ENABLE, WIFINET_BCNMISS_TIME/* period, ms*/);

        /*enable wow and set filters */
        wifimac->drv_priv->drv_ops.drv_set_suspend(wifimac->drv_priv, wnet_vif_tmp->wnet_vif_id, ENABLE,
            WIFI_SUSPEND_STATE_WOW, filter);

        WIFINET_PWRSAVE_LOCK(wnet_vif);
        wnet_vif_tmp->vm_pwrsave.ips_state = WIFINET_PWRSAVE_FULL_SLEEP;
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }

    if (connect == 0)
    {
        /* if both wlan0 and p2p0 don't connect ap, just follow wlan0 operations. */
        if (wnet_vif->wnet_vif_id == 1)
        {
            WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
            return 0;
        }
        wifimac->drv_priv->drv_ops.drv_set_suspend(wifimac->drv_priv, wnet_vif->wnet_vif_id, ENABLE,
            WIFI_SUSPEND_STATE_WOW, filter);
        WIFINET_PWRSAVE_LOCK(wnet_vif);
        wnet_vif->vm_pwrsave.ips_state = WIFINET_PWRSAVE_FULL_SLEEP;
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }

    /*set suspend state */
    wifimac->wm_suspend_mode = WIFI_SUSPEND_STATE_WOW;

    WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
    return 0;
}

int wifi_mac_pwrsave_wow_resume(SYS_TYPE param1,
                                SYS_TYPE param2,SYS_TYPE param3,
                                SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;
    struct wlan_net_vif *wnet_vif_next = NULL, *wnet_vif_tmp = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int connect = 0;
    int ret = 0;

    pr_debug("%s:%d\n", __func__, __LINE__);
    WIFINET_PWRSAVE_MUTEX_LOCK(wnet_vif);
    if (wifimac->wm_suspend_mode == WIFI_SUSPEND_STATE_NONE)
    {
        WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
        return 0;
    }

    list_for_each_entry_safe(wnet_vif_tmp, wnet_vif_next, &wifimac->wm_wnet_vifs, vm_next)
    {
        /* now, not support suspend ap. */
        if ((wnet_vif_tmp->vm_opmode == WIFINET_M_HOSTAP)
            || (wnet_vif_tmp->vm_state != WIFINET_S_CONNECTED)) {
            continue;
        }
        connect++;
        /*
        * disable vmac wow and clear filters. patterns are cleaned in firmware
        * when firmware get wow disable command.
        */
        ret = wifimac->drv_priv->drv_ops.drv_set_suspend(wifimac->drv_priv, wnet_vif_tmp->wnet_vif_id, DISABLE,
            WIFI_SUSPEND_STATE_NONE, 0);

        wifi_mac_set_arp_agent(wnet_vif_tmp, DISABLE);

        /* change beacon listen interval to beacon interval. */
        wifimac->drv_priv->drv_ops.Phy_beaconinit(wifimac->drv_priv,wnet_vif_tmp->wnet_vif_id,
            wnet_vif_tmp->vm_bcn_intval/*listen_interval*/);

        /* change beacon miss timer period */
        wifi_mac_set_beacon_miss_ex(wnet_vif_tmp, ENABLE, WIFINET_BCNMISS_TIME/* period, ms*/);

        WIFINET_PWRSAVE_LOCK(wnet_vif);
        if (ret == 0)
            wnet_vif_tmp->vm_pwrsave.ips_state = WIFINET_PWRSAVE_AWAKE;
        else
            ERROR_DEBUG_OUT("ret -1 \n");
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }

    if (connect == 0)
    {
        /* if both wlan0 and p2p0 don't connect ap, just follow wlan0 operations. */
        if (wnet_vif->wnet_vif_id == 1)
        {
            WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
            wnet_vif->vm_scan_hang = 0;
            netif_wake_queue(wnet_vif->vm_ndev);
            return 0;
        }
        wifimac->drv_priv->drv_ops.drv_set_suspend(wifimac->drv_priv, wnet_vif->wnet_vif_id, DISABLE,
            WIFI_SUSPEND_STATE_NONE, 0);

        WIFINET_PWRSAVE_LOCK(wnet_vif);
        wnet_vif->vm_pwrsave.ips_state = WIFINET_PWRSAVE_AWAKE;
        WIFINET_PWRSAVE_UNLOCK(wnet_vif);
    }

    /*set suspend state */
    wifimac->wm_suspend_mode = WIFI_SUSPEND_STATE_NONE;

    WIFINET_PWRSAVE_MUTEX_UNLOCK(wnet_vif);
    wnet_vif->vm_scan_hang = 0;
    netif_wake_queue(wnet_vif->vm_ndev);
    return 0;
}

int wifi_mac_pwrsave_wow_check(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (wifimac->wm_suspend_mode == WIFI_SUSPEND_STATE_WOW)
    {
        wifi_mac_pwrsave_wow_resume(0, 0, 0, (SYS_TYPE)wnet_vif, 0);
    }
    return 0;
}

unsigned char sta_find_in_sta_tbl(struct wifi_station *find_sta)
{
    unsigned char vid;
    struct wifi_station *sta, *sta_next;
    struct wifi_station_tbl *vm_sta_tbl = NULL;
    struct drv_private *drv_priv = drv_get_drv_priv();

    for (vid = 0; vid < WIFI_MAX_VID; ++vid)
    {
        vm_sta_tbl = &drv_priv->drv_wnet_vif_table[vid]->vm_sta_tbl;
        WIFINET_NODE_LOCK(vm_sta_tbl);
        list_for_each_entry_safe(sta, sta_next, &vm_sta_tbl->nt_nsta, sta_list)
        {
            if (find_sta == sta)
            {
                WIFINET_NODE_UNLOCK(vm_sta_tbl);
                return 1;
            }
        }
        WIFINET_NODE_UNLOCK(vm_sta_tbl);
    }

    return 0;
}
