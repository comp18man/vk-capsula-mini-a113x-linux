/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer power save  interface module
 *
 *
 ****************************************************************************************
 */
#ifndef _NET80211_IEEE80211_POWER_H_
#define _NET80211_IEEE80211_POWER_H_

#include "wifi_mac_rx_status.h"

//powersave presleep timer
#define WIFINET_PWRSAVE_TIMER_INTERVAL   200

//INACTIVITYTIME for WIFINET_PWRSAVE_LOW
#define WIFINET_PS_LOW_INACTIVITYTIME               800
//INACTIVITYTIME for WIFINET_PWRSAVE_NORMAL
#define WIFINET_PS_NORMAL_INACTIVITYTIME            1200
//INACTIVITYTIME for WIFINET_PWRSAVE_MAXIMUM
#define WIFINET_PS_MAXIMUM_INACTIVITYTIME           200

//after wakeup from beacon timer, sta need to wait for the incoming beacon
#define WIFINET_PWRSAVE_WAITBEACON_TIMER_INTERVAL       70
//after send trigger, sta need to wait for the response
#define WIFINET_PWRSAVE_PSTRIGGER_TIMER_INTERVAL    80

//null data with powersave=1
#define NULLDATA_PS              (1)
//null data with powersave=0
#define NULLDATA_NONPS        (0)

#define WIFINET_BCNMISS_TIME  (WIFINET_BMISS_THRS * 2)

//flag whether to enter sleep after nulldata is transmited completely
//just send nulldata with ps=1, but not enter sleep when txed. for example, powersave for other intf scanning
//send nulldata with ps=1 and also enter sleep after txed.

enum wifi_mac_psmode
{
//non-powersave mode
    WIFINET_PWRSAVE_NONE,
//powersave low mode, and save the minimum power, such as  INACTIVITYTIME is longer, trigger use wakeup
    WIFINET_PWRSAVE_LOW,
//powersave normal mode, and save the normal power
    WIFINET_PWRSAVE_NORMAL,
//powersave normal mode, and save the maximum power
    WIFINET_PWRSAVE_MAXIMUM,
    WIFINET_PWRSAVE_MODE_MAX
} ;

enum wifinet_ps_state
{
    //sta is recognized awake by ap
    WIFINET_PWRSAVE_AWAKE,
    //sta is recognized sleep by ap, and fw of sta is sleep too.
    WIFINET_PWRSAVE_FULL_SLEEP,
    //sta is recognized sleep by ap, but fw of sta is not sleep.
    //such as sta wakeup by beacon timer.
    WIFINET_PWRSAVE_NETWORK_SLEEP,
    WIFINET_PWRSAVE_STATE_MAX,
} ;

extern char *ips_state[WIFINET_PWRSAVE_STATE_MAX];

//reason for enter sleep, for debug
enum wifinet_ps_sleep_reason
{
    SLEEP_NONE,
    SLEEP_AFTER_VMAC_CREATE,
    SLEEP_AFTER_VMAC_UP,
    SLEEP_AFTER_VMAC_CONNECT,
    SLEEP_AFTER_VMAC_DISCONNECT,
    SLEEP_AFTER_VMAC_DOWN,
    SLEEP_AFTER_VMAC_DEL,
    SLEEP_AFTER_INACTIVITY_ENOUGH,
    SLEEP_AFTER_TX_NULL_WITH_PS,
    SLEEP_AFTER_BEACON,
    SLEEP_AFTER_WAIT_BEACON_TIMEOUT,
    SLEEP_AFTER_PSPOLL_FINISHED,
    SLEEP_AFTER_UAPSD_TRIGGER_FINISHED,
    SLEEP_AFTER_PS_TRIGGER_TIMEOUT,
    SLEEP_AFTER_NOA_START,
    SLEEP_AFTER_NOA_END,
    SLEEP_AFTER_NOA_END_DUMMY,
    SLEEP_AFTER_OPPS_END,
    SLEEP_AFTER_SUSPEND,
    SLEEP_AFTER_UNKNOWN,
    SLEEP_REASON_MAX,
} ;

//reason for enter netsleep, for debug
enum wifinet_ps_netsleep_reason
{
    NETSLEEP_AFTER_WAKEUP,
    NETSLEEP_AFTER_BEACON_SEND,
    NETSLEEP_AFTER_OPPS_END,
    NETSLEEP_AFTER_NOA,
    NETSLEEP_REASON_MAX,
} ;

//reason for enter wakeup, for debug
 enum wifinet_ps_wk_reason
{
    WKUP_FROM_VMAC_CREATE,
    WKUP_FROM_VMAC_UP,
    WKUP_FROM_VMAC_CONNECT,
    WKUP_FROM_VMAC_DISCONNECT,
    WKUP_FROM_VMAC_DOWN,
    WKUP_FROM_VMAC_DEL,
    WKUP_FROM_TRANSMIT,
    WKUP_FROM_RECEIVE,
    WKUP_FROM_BCN_MISS,
    WKUP_FROM_REMAIN_ACTIVE,
    WKUP_FROM_PSMODE_EXIT,
    WKUP_FROM_PSEXIT_UNKNOWN,
    WKUP_FROM_PSNULL_FAIL,
    WAKEUP_UNKNOWN,
    WAKEUP_REASON_MAX,
} ;


struct wifi_mac_pwrsave_t
{
    enum wifi_mac_psmode  ips_sta_psmode;
    enum wifinet_ps_state ips_state;
    struct os_timer_ext       ips_timer_presleep;
    unsigned int               ips_inactivitytime;
    spinlock_t              ips_lock;
    unsigned long   ips_lockflags;
    OS_MUTEX                ips_mutex;

    struct os_timer_ext       ips_timer_sleep_wait;
    struct os_timer_ext     ips_timer_uapsd_trigger; /*timer, for user send triggers*/
    enum wifinet_ps_sleep_reason    ips_sleep_wait_reason;
    unsigned char           ips_flag_send_ps_trigger;
    unsigned char           ips_flag_uapsd_trigger; /*flag,  user send triggers to AP for ACs which support U-APSD */
    unsigned short          ips_ps_trigger_timeout;
    unsigned char           ips_flag_waitbeacon_timer_start;
    unsigned short          ips_ps_waitbeacon_timeout;
    struct work_struct ips_work_presleep;
};

#define WIFINET_PWRSAVE_MUTEX_INIT(_wnet_vif)       mutex_init(&(_wnet_vif)->vm_pwrsave.ips_mutex)
#define WIFINET_PWRSAVE_MUTEX_LOCK(_wnet_vif)       OS_MUTEX_LOCK(&(_wnet_vif)->vm_pwrsave.ips_mutex);
#define WIFINET_PWRSAVE_MUTEX_UNLOCK(_wnet_vif) OS_MUTEX_UNLOCK(&(_wnet_vif)->vm_pwrsave.ips_mutex);

#define WIFINET_PWRSAVE_LOCK_INIT(_wnet_vif)       spin_lock_init(&(_wnet_vif)->vm_pwrsave.ips_lock)
#define WIFINET_PWRSAVE_LOCK(_wnet_vif) OS_SPIN_LOCK_IRQ(&(_wnet_vif)->vm_pwrsave.ips_lock, (_wnet_vif)->vm_pwrsave.ips_lockflags)
#define WIFINET_PWRSAVE_UNLOCK(_wnet_vif) OS_SPIN_UNLOCK_IRQ(&(_wnet_vif)->vm_pwrsave.ips_lock, (_wnet_vif)->vm_pwrsave.ips_lockflags)

struct wifi_mac;
struct wlan_net_vif;
struct wifi_station;

#define PS_OVER_FLAG_INIT       0
#define PS_OVER_FLAG_PSPOLL  1
#define PS_OVER_FLAG_UAPSD    2
#define PS_OVER_FLAG_ALL        3

/* suspend and resume field */

enum wifi_mac_wow_mode
{
    WIFI_WOW_MODE_DISABLE,
    WIFI_WOW_MODE_ENABLE,
};

enum wifi_mac_suspend_state
{
    WIFI_SUSPEND_STATE_NONE,
    WIFI_SUSPEND_STATE_WOW,
    WIFI_SUSPEND_STATE_DEEPSLEEP,
};

/* suspend and resume field end*/
int wifi_mac_pwrsave_psqueue_clean(struct wifi_station *);
int wifi_mac_pwrsave_psqueue_age(struct wifi_station *,
                                struct sk_buff_head *);

void    wifi_mac_pwrsave_reason_init(void);

void    wifi_mac_pwrsave_vattach(struct wlan_net_vif *);
void    wifi_mac_pwrsave_latevattach(struct wlan_net_vif *);
void    wifi_mac_pwrsave_vdetach(struct wlan_net_vif *);
void    wifi_mac_pwrsave_attach(void);
void    wifi_mac_pwrsave_detach(void);
void    wifi_mac_pwrsave_wnet_vif_state_init(struct wlan_net_vif *wnet_vif);
void    wifi_mac_pwrsave_wnet_vif_connect(struct wlan_net_vif *wnet_vif);
void    wifi_mac_pwrsave_wnet_vif_disconnect(struct wlan_net_vif *wnet_vif);

void    wifi_mac_pwrsave_set_mode(struct wlan_net_vif *, unsigned int mode);

void    wifi_mac_pwrsave_proc_tim(struct wlan_net_vif *wnet_vif);
void     wifi_mac_pwrsave_proc_dtim(struct wlan_net_vif *wnet_vif);
void    wifi_mac_pwrsave_wakeup_for_tx (struct wlan_net_vif *wnet_vif);

void wifi_mac_pwrsave_set_tim(struct wifi_station *sta, int set);
void wifi_mac_pwrsave_restore_sleep(struct wlan_net_vif *wnet_vif);
void wifi_mac_pwrsave_sleep_wait_cancel (struct wlan_net_vif *wnet_vif);
int wifi_mac_pwrsave_wkup_and_NtfyAp (struct wlan_net_vif *wnet_vif, enum wifinet_ps_wk_reason reason);
int wifi_mac_pwrsave_fullsleep(struct wlan_net_vif *wnet_vif, enum wifinet_ps_sleep_reason reason);
int wifi_mac_pwrsave_wakeup(struct wlan_net_vif *wnet_vif, enum wifinet_ps_wk_reason reason);
int wifi_mac_pwrsave_networksleep (struct wlan_net_vif *wnet_vif, enum wifinet_ps_netsleep_reason reason);

int wifi_mac_pwrsave_txpre (struct sk_buff *skb);
void wifi_mac_pwrsave_state_change(struct wifi_station *sta, int enable);
void wifi_mac_pwrsave_recv_pspoll(struct wifi_station *sta, struct sk_buff *skb0);
void wifi_mac_pwrsave_psqueue_enqueue(struct wifi_station *sta, struct sk_buff *skb);
int wifi_mac_pwrsave_psqueue_flush (struct wifi_station *sta);
int wifi_mac_pwrsave_is_sta_sleeping (struct wlan_net_vif *wnet_vif);
int wifi_mac_pwrsave_is_wnet_vif_networksleep (struct wlan_net_vif *wnet_vif);
void wifi_mac_pwrsave_notify_txq_empty(struct wifi_mac *wifimac);
int wifi_mac_pwrsave_send_nulldata(struct wifi_station *sta, unsigned char pwr_save, unsigned char pwr_flag);
int wifi_mac_pwrsave_is_sta_fullsleep (struct wlan_net_vif *wnet_vif);
void wifi_mac_buffer_txq_flush(struct sk_buff_head *pstxqueue);
int wifi_mac_buffer_txq_enqueue (struct sk_buff_head *pstxqueue, struct sk_buff *skb);
int wifi_mac_forward_txq_enqueue (struct sk_buff_head *fwdtxqueue, struct sk_buff *skb);
int wifi_mac_pwrsave_is_wnet_vif_sleeping (struct wlan_net_vif *wnet_vif);
void wifi_mac_pwrsave_sta_trigger (struct wlan_net_vif *wnet_vif);
int wifi_mac_buffer_txq_send(struct sk_buff_head *txqueue);
int wifi_mac_buffer_txq_send_pre(struct wlan_net_vif *wnet_vif);
int wifi_mac_pwrsave_is_wnet_vif_fullsleep (struct wlan_net_vif *wnet_vif);
int wifi_mac_pwrsave_if_ap_can_opps (struct wlan_net_vif *wnet_vif);
int wifi_mac_pwrsave_sta_uapsd_trigger (void *arg);
void wifi_mac_pwrsave_check_ps_end(void* ieee,  struct sk_buff * skbbuf, void * nsta,struct wifi_mac_rx_status* rs);
void wifi_mac_pwrsave_chk_uapsd_trig(void* ieee,  struct sk_buff * skbbuf, void * nsta,struct wifi_mac_rx_status* rs);
void wifi_mac_pwrsave_eosp_indicate(void* nsta,  struct sk_buff * skbbuf, int txok);
void wifi_mac_pwrsave_send_pspoll(struct wifi_station *);

int wifi_mac_pwrsave_wow_suspend(SYS_TYPE param1,
                                SYS_TYPE param2,SYS_TYPE param3,
                                SYS_TYPE param4,SYS_TYPE param5);
int wifi_mac_pwrsave_wow_resume(SYS_TYPE param1,
                                SYS_TYPE param2,SYS_TYPE param3,
                                SYS_TYPE param4,SYS_TYPE param5);
int wifi_mac_pwrsave_wow_usr(struct wlan_net_vif *wnet_vif,
			  struct cfg80211_wowlan *wow, unsigned int *filter);
int wifi_mac_pwrsave_wow_sta(struct wlan_net_vif *wnet_vif);
int wifi_mac_pwrsave_hif_suspend(struct wlan_net_vif *wnet_vif,
    struct cfg80211_wowlan *wow);
int wifi_mac_pwrsave_wow_check(struct wlan_net_vif *wnet_vif);
void  wifi_mac_pwrsave_sleep_wait_ex(SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5);
void wifi_mac_send_nulldata_retry(void * data);
unsigned char sta_find_in_sta_tbl(struct wifi_station *find_sta);
#endif /* _NET80211_IEEE80211_POWER_H_ */
