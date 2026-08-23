/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer the first .h need include
 *
 *
 ****************************************************************************************
 */
#ifndef _NET80211_IEEE80211_VAR_H_
#define _NET80211_IEEE80211_VAR_H_

#include "wifi_mac_sta.h"
#include "wifi_mac_power.h"
#include "wifi_mac_scan.h"
#include "wifi_mac_action.h"

struct wifi_mac_statistic
{
    unsigned int sts_rx_bad_ver;
    unsigned int sts_rx_too_short;
    unsigned int sts_rx_wrong_bss;
    unsigned int sts_rx_dup;
    unsigned int sts_rx_wrong_dir;
    unsigned int sts_rx_mcast_echo;
    unsigned int sts_rx_not_assoc;
    unsigned int sts_no_priv_ac;
    unsigned int sts_rx_uncrypted;
    unsigned int sts_rx_decap;
    unsigned int sts_mng_discard;
    unsigned int sts_rx_ctrl;
    unsigned int sts_rx_bcn;
    unsigned int sts_rx_elem_miss;
    unsigned int sts_rx_elem_too_long;
    unsigned int sts_rx_elem_too_short;
    unsigned int sts_rx_elem_err;
    unsigned int sts_rx_sta_all_fail;
    unsigned int sts_rx_ssid_mismatch;
    unsigned int sts_auth_unsprt;
    unsigned int sts_rx_auth_fail;
    unsigned int sts_rx_ps_uncnnt;
    unsigned int sts_rx_ps_aid_err;
    unsigned int sts_rx_assoc_err;
    unsigned int sts_rx_assoc_unauth;
    unsigned int sts_rx_assoc_cap_mismatch;
    unsigned int sts_rx_assoc_rate_mismatch;
    unsigned int sts_rx_assoc_wpa_mismatch;
    unsigned int sts_rx_deauth;
    unsigned int sts_rx_dis_assoc;
    unsigned int sts_rx_action;
    unsigned int sts_rx_buf_full;
    unsigned int sts_rx_decrypt_err;
    unsigned int sts_rx_auth_mismatch;
    unsigned int sts_rx_unauth;
    unsigned int sts_rx_tkip_mic_err;
    unsigned int sts_rx_acl_mismatch;
    unsigned int sts_tx_no_buf;
    unsigned int sts_tx_no_sta;
    unsigned int sts_tx_unsprt_mngt;
    unsigned int sts_tx_key_err;
    unsigned int sts_headroom_err;
    unsigned int sts_tx_sta_aged;
    unsigned int sts_tx_tkip_sw_mic_err;
    unsigned int sts_tx_ps_no_data;
    unsigned int sts_rx_tkip_sw_mic_err;
    unsigned int sts_key_drop;
    unsigned int sts_key_id_err;
    unsigned int sts_key_type_err;

    unsigned int sts_tx_total_msdu;
    unsigned int sts_tx_out_msdu;
    unsigned int sts_tx_drop_msdu;

    unsigned int sts_tx_tid_drop_bf_in_msdu[8];
    unsigned int sts_tx_tid_in_msdu[8];
    unsigned int sts_tx_tid_out_msdu[8];
    unsigned int sts_tx_tid_drop_msdu[8];

    unsigned int sts_tx_tcp_msdu;
    unsigned int sts_tx_udp_msdu;
    unsigned int sts_tx_arp_msdu;
    unsigned int sts_tx_ns_msdu;

    unsigned int sts_tx_non_amsdu_in_msdu[8];
    unsigned int sts_tx_non_amsdu_drop_msdu[8];
    unsigned int sts_tx_non_amsdu_out_msdu[8];

    unsigned int sts_tx_amsdu_in_msdu[8];
    unsigned int sts_tx_amsdu_drop_msdu[8];
    unsigned int sts_tx_amsdu_out_msdu[8];

    unsigned int sts_rx_msdu;
    unsigned long sts_rx_msdu_time_stamp;

};

enum
{
    WIFINET_APPIE_FRAME_BEACON     = 0,
    WIFINET_APPIE_FRAME_PROBE_REQ  = 1,
    WIFINET_APPIE_FRAME_PROBE_RESP = 2,
    WIFINET_APPIE_FRAME_ASSOC_REQ  = 3,
    WIFINET_APPIE_FRAME_ASSOC_RESP = 4,
    WIFINET_APPIE_NUM_OF_FRAME     = 5
};

struct vm_wlan_net_vif_params
{
    char vm_param_name[IFNAMSIZ + 1];
    unsigned short vm_param_opmode;
};

#define WIFINET_SIGNAL_POWER_WEAK_THRESH_NARROW -75
//When RSSI is less than -70, selecting a fixed rate when entering a weak signal
// will lead to a sudden drop in throughput
#define WIFINET_SIGNAL_POWER_WEAK_THRESH_WIDE -80

#define WIFINET_SIGNAL_POWER_BW_CHANGE_THRESH_NARROW -74
#define WIFINET_SIGNAL_POWER_BW_CHANGE_THRESH_WIDE -63


#define WIFINET_TXPOWER_MAX  100

#define WIFINET_TXSTREAM  1
#define WIFINET_RXSTREAM  1
#define WIFINET_DTIM_DEFAULT  1

#define WIFINET_BINTVAL_DEFAULT  100
#define DEFAULT_MGMT_RETRY_TIMES 3
#define DEFAULT_P2P_ACTION_RETRY_TIMES 3
#define DEFAULT_MGMT_RETRY_INTERVAL 512
#define DEFAULT_P2P_ACTION_RETRY_INTERVAL 50
#define DEFAULT_AUTH_RETRY_INTERVAL 512

#define WIFINET_PS_MAX_QUEUE  100

#define WIFINET_BMISS_THRS  (25*100)
#define WIFINET_BMISS_COUNT_MAX  4
//in ms, fake interval for fw power saving
#define WIFINET_INTER_BEACON_INTERVAL (500)

#define WIFINET_FRAGMT_THRESHOLD_MAX  2346


#define WIFINET_TU_TO_MS(x)  (((x) * 1024) / 1000)

#define WIFINET_APPIE_MAX  1024

#define WIFINET_PWRCONSTRAINT_VAL(wnet_vif) \
        (((wnet_vif)->vm_curchan->chan_maxpower > (wnet_vif->vm_wmac)->wm_curchanmaxpwr) ? \
         (wnet_vif)->vm_curchan->chan_maxpower - (wnet_vif->vm_wmac)->wm_curchanmaxpwr : 0)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

struct _wifi_mac_country_iso
{
    unsigned char iso[3];
} ;

#define RS_AMPDU_PKT (1 << 0)

struct wlan_net_vif;

struct nsta_entry
{
    struct list_head entry;
    struct wifi_station *nsta;
    unsigned long queue_time;
};


#define WIFINET_NODE_FREE_LOCK_INIT(_ic) spin_lock_init(&(_ic)->wm_free_lock)
#define WIFINET_NODE_FREE_LOCK(_ic) OS_SPIN_LOCK_IRQ(&(_ic)->wm_free_lock, (_ic)->wm_free_lock_flags)
#define WIFINET_NODE_FREE_UNLOCK(_ic) OS_SPIN_UNLOCK_IRQ(&(_ic)->wm_free_lock, (_ic)->wm_free_lock_flags)

#define NETCOM_NETIF_RUNNING  0x01
#define NETCOM_NETIF_IFF_RUNNING  0x08
#define NETCOM_NETIF_IFF_UP  0x10

#define NETCOM_NETIF_IFF_UP_RUNNING  (NETCOM_NETIF_IFF_UP|NETCOM_NETIF_IFF_RUNNING)
#define NET80211_IF_RUNNING(_ic)  (((_ic)->wm_dev_flags & NETCOM_NETIF_IFF_RUNNING))
#define NET80211_IF_RUN_UP(_ic)  (((_ic)->wm_dev_flags & NETCOM_NETIF_IFF_UP_RUNNING))

struct wifinet_vht_mcs {
    unsigned short mcs_map;      /* Max MCS for each SS */
    unsigned short data_rate;    /* Max data rate */
} ;

struct wifinet_vht_mcs_set {
    /* B0-B15 Max Rx MCS for each SS B16-B28 Max Rx data rate
       B29-B31 reserved */
    struct wifinet_vht_mcs rx_mcs_set;
    /* B32-B47 Max Tx MCS for each SS B48-B60 Max Tx data rate
       B61-B63 reserved */
    struct wifinet_vht_mcs tx_mcs_set;
};

#define WME_NUM_TID         8

/*shijie.chen add, for buffer msdu to aggregate amsdu */
struct msdu_list {
    struct list_head free_list;
};

#define INVALID_ICMAINVMAC_ID     0xffff
#define INVALID_ICP2PVMAC_ID     0xffff

enum
{
    ADDBA_SEND   = 0,
    ADDBA_STATUS = 1,
    DELBA_SEND   = 2,
    ADDBA_RESP   = 3,
    ADDBA_CLR_RESP = 4,
};

struct wifi_mac_addba_delba_request
{
    struct wifi_mac *wifimac;
    unsigned char action;
    unsigned char tid;
    uint16_t status;
    unsigned short aid;
    unsigned int arg1;
    unsigned int arg2;
};

struct wifi_mac_app_ie_t
{
    unsigned int length;
    unsigned char *ie;
};

#define AML_MCAST_QUEUE
#define IV_SSID_SCAN_AMOUNT     9
#define WIFINET_MAX_IV_OPT_IE   1024
#define NET80211_MAIN_VMAC  0
#define NET80211_P2P_VMAC  1

struct wifi_skb_callback;
/**
 *  per-frame tx control block
 */

struct wifi_mac_ops
{
    int (*wifi_mac_mac_entry)(struct wifi_mac *wifimac,void *  drv_prv);

    void (*wifi_mac_setup_channel_list)(void * ieee,  unsigned int wMode, int countrycode_ex);
    void (*wifi_mac_update_txpower)(void *ieee, unsigned short cfg_txpowlimit, unsigned short txpowlevel);
    int (*wifi_mac_update_beacon)(void *ieee, int wnet_vif_id,   struct sk_buff * skbbuf, int mcast);
    void (*wifi_mac_notify_qstatus)(void *ieee, unsigned short queue_qcnt);
    void (*wifi_mac_tx_complete)(struct wlan_net_vif *wnet_vif, struct sk_buff * skbbuf,  struct wifi_mac_tx_status *ts);
    void (*wifi_mac_tx_status)(void * sta,  struct wifi_mac_tx_status * ts);
    int (*wifi_mac_rx_complete)(void *ieee,  struct sk_buff * skbbuf, struct wifi_mac_rx_status* rs);
    int (*wifi_mac_input)(void * sta,  struct sk_buff * skbbuf, struct wifi_mac_rx_status* rs);
    void (*wifi_mac_ChangeChannel)(void * ieee, struct wifi_channel *chan, unsigned char flag, unsigned char vid, unsigned char opmode);
    void (*wifi_mac_switch_mode_static20)(void * ieee);
    void (*wifi_mac_switch_mode_static40)(void * ieee);
    void (*wifi_mac_switch_mode_static80)(void * ieee);

    /* Rate control related ops */
    void (*wifi_mac_rate_init)(void * ieee, enum wifi_mac_macmode,  const struct drv_rate_table *rt);
    void (*wifi_mac_rate_nsta_update)(void *  ieee, void * sta, int isnew);

    void (*wifi_mac_free_txamsdu_queue)(void *ieee, unsigned char vid);
    void (*wifi_mac_ap_chk_uapsd_trig)(void *  ieee,  struct sk_buff * skbbuf, void * sta,struct wifi_mac_rx_status* rs);
    void (*wifi_mac_uapsd_eosp_indicate)(void * sta ,  struct sk_buff * skbbuf, int txok);

    void (*wifi_mac_sta_check_ps_end)(void *  ieee,  struct sk_buff * skbbuf, void * sta,struct wifi_mac_rx_status* rs);

    void (*wifi_mac_rate_ratmod_attach)(void *drv_prv);
    void (*wifi_mac_rate_ratmod_detach)(void *drv_prv);
    void (*wifi_mac_rate_findrate)(void *drv_sta, int shortPreamble, size_t frameLen, struct aml_ratecontrol ratectrl[],int mcs_rate);

    int (*wifi_mac_create_vmac)(struct wifi_mac *wifimac, void *ifr,int cmdFromwhr);
    int (*wifi_mac_device_ip_config)(struct wlan_net_vif *wnet_vif, void *event);
    void (*wifi_mac_tbtt_handle)(struct wlan_net_vif *wnet_vif);
    void (*wifi_mac_channel_switch_complete)(struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_get_netif_cfg)(void *  ieee);
    int (*wifi_mac_mac_exit)(struct wifi_mac *wifimac);
    int (*wifi_mac_top_sm)(struct wlan_net_vif *wnet_vif, enum wifi_mac_state nstate, int arg);
    struct wifi_station *(*os_skb_get_nsta)(struct sk_buff *skb);
    int (*os_skb_get_tid)(struct sk_buff *skb);
    void (*wifi_mac_notify_mic_fail)(struct wlan_net_vif *wnet_vif,const struct wifi_frame *wh, unsigned int key_index);
    int (*wifi_mac_send_qosnulldata)(struct wifi_station *sta, int ac);
   // int (*vm_p2p_client_cancle_noa) (struct wifi_mac_p2p *p2p);
   // int (*vm_p2p_go_cancle_noa) (struct wifi_mac_p2p *p2p);
    int (*wifi_mac_pwrsave_fullsleep)(struct wlan_net_vif *wnet_vif, enum wifinet_ps_sleep_reason reason);
    int (*wifi_mac_pwrsave_is_sta_fullsleep) (struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_pwrsave_is_sta_sleeping) (struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_pwrsave_is_wnet_vif_fullsleep )(struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_pwrsave_is_wnet_vif_sleeping) (struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_pwrsave_networksleep) (struct wlan_net_vif *wnet_vif,enum wifinet_ps_netsleep_reason reason);
    void (*wifi_mac_pwrsave_psqueue_enqueue)(struct wifi_station *sta, struct sk_buff *skb);
    int (*wifi_mac_pwrsave_psqueue_flush) (struct wifi_station *sta);
    void (*wifi_mac_pwrsave_sta_trigger) (struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_buffer_txq_enqueue) (struct sk_buff_head *pstxqueue, struct sk_buff *skb);
    int (*wifi_mac_buffer_txq_send_pre )(struct wlan_net_vif *wnet_vif);
    void(* wifi_mac_pwrsave_wakeup_for_tx) (struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_pwrsave_wkup_and_NtfyAp) (struct wlan_net_vif *wnet_vif,enum wifinet_ps_wk_reason reason);
    void (*wifi_mac_notify_ap_success) (struct wlan_net_vif *wnet_vif);
    struct wifi_station * (*wifi_mac_get_sta)(struct wifi_station_tbl *nt, const unsigned char *macaddr,int wnet_vif_id);
    void (*wifi_mac_sta_attach)(struct wifi_mac *wifimac);
    int (*wifi_mac_beacon_miss)(struct wlan_net_vif *wnet_vif);
    void(*wifi_mac_get_sts)(struct wifi_mac *wifimac,unsigned int op_code, unsigned int ctrl_code);
    void (*wifi_mac_process_tx_error)(struct wlan_net_vif *wnet_vif);
    int (*wifi_mac_forward_txq_enqueue) (struct sk_buff_head *fwdtxqueue, struct sk_buff *skb);
};

enum
{
    CONCURRENT_SLOT_STA = 0,
    CONCURRENT_SLOT_P2P = 1,
    CONCURRENT_SLOT_NONE = 2,
};

enum
{
    CONCURRENT_CHANNEL_SWITCH = BIT(0),
    CONCURRENT_NOTIFY_AP = BIT(1),
    CONCURRENT_NOTIFY_AP_SUCCESS = BIT(2),
    CONCURRENT_AP_SWITCH_CHANNEL = BIT(3),
    CONCURRENT_SWITCH_TO_STA_CHANNEL = BIT(4),
};

struct chan_switch_target_t {
    unsigned char start;
    struct wifi_channel switch_chan;
};

struct wifi_mac_chan_overlapping {
    unsigned char chan_index;
    unsigned short overlapping;
};

enum VsdbState {
    VSDB_STATE_DISABLE,
    VSDB_STATE_ENABLE,
    VSDB_STATE_MAX,
};

struct wifi_mac
{
    unsigned char wm_mac_exit;
    unsigned int wm_dev_flags;
    unsigned int wm_syncbeacon;
    unsigned int wm_flags;
    unsigned int wm_flags_ext;
    unsigned int wm_flags_ext2;
    unsigned int wm_caps;
    unsigned char wm_nopened;
    unsigned char wm_nrunning;
    unsigned int wm_runningmask;

    short wm_signal_power_weak_thresh_narrow;
    short wm_signal_power_weak_thresh_wide;

    short wm_signal_power_bw_change_thresh_narrow;
    short wm_signal_power_bw_change_thresh_wide;

    struct wifi_mac_ops wmac_ops;
    struct drv_private *drv_priv;
    struct wifi_mac_wme_state wm_wme[DEFAULT_MAX_VMAC];
    int wm_ac2q[WME_NUM_AC];

    int wm_nchans;
    struct wifi_channel wm_channels[WIFINET_CHAN_MAX * 2 + 1];
    unsigned char wm_chan_avail[WIFINET_CHAN_BYTES];
    struct wifi_channel *wm_curchan;
    unsigned char wm_curchanmaxpwr;
    unsigned char wm_txpowlimit;
    spinlock_t  channel_lock;
    unsigned long channel_lock_flag;

    struct wifi_mac_chan_overlapping chan_overlapping_map[WIFINET_MAX_SCAN_CHAN];
    unsigned long chan_overlapping_last;
    unsigned long wm_scanplayercnt;
    struct wifi_mac_scan_state *wm_scan;
    unsigned long wm_lastscan;
    unsigned long wm_lastroaming;
    enum wifi_mac_roamingmode wm_roaming;
    int roaming_threshold_5g;
    int roaming_threshold_2g;

    /* 11g rates for p2p GO and vht mode */
    struct wifi_mac_rateset wm_11b_rates;
    struct wifi_mac_rateset wm_11g_rates;
    struct wifi_mac_rateset wm_sup_ht_rates;
    struct wifi_mac_rateset wm_sup_vht_rates;

    spinlock_t wm_comlock;
    spinlock_t wm_wnet_vifslock;
    spinlock_t wm_queuelock;
    unsigned long wm_queuelock_flags;
    spinlock_t wm_scanlock;
    unsigned long wm_scanlock_flags;
    OS_MUTEX wm_bcnbuflock;
    spinlock_t wm_pslock;
    unsigned long wm_pslock_flags;
    spinlock_t wm_stalock;
    spinlock_t wm_free_lock;
    unsigned long wm_free_lock_flags;
    spinlock_t wm_amsdu_tasklet_lock;

    int wnet_vif_num;
    struct list_head wm_wnet_vifs;
    struct list_head wm_free_entryq;
    long wm_wnet_vif_mask;

    struct os_timer_ext wm_free_timer;
    struct os_timer_ext wm_inact_timer;
    unsigned int wm_time_nonerp_present;
    unsigned int wm_time_noht_present;

    unsigned char wm_maxampdu;
    unsigned char wm_mpdudensity;

#ifdef CONFIG_CONCURRENT_MODE
    struct os_timer_ext wm_concurrenttimer;
    unsigned char wm_vsdb_sate;
    unsigned char wm_vsdb_slot;
    unsigned short wm_vsdb_flags;
    unsigned long wm_vsdb_switch_time;
#endif //CONFIG_CONCURRENT_MODE
    struct os_timer_ext wm_csa_trigger_timer;

    enum wifi_mac_protmode wm_protmode;
    unsigned short wm_nonerpsta;
    unsigned short wm_longslotsta;

    unsigned short wm_non_ht_sta;
    unsigned short wm_ht20_sta;
    unsigned short wm_ht_prot_sm;

    struct wifi_mac_ie_country wm_countryinfo;
    struct _wifi_mac_country_iso wm_country;
    struct wifi_mac_country_ie* wm_11dinfo;
    struct wifi_mac_vendor_ie wm_vendorinfo[VENDOR_IE_MAX];

    unsigned char scan_gain_thresh_unconnect;
    unsigned char scan_gain_thresh_connect;
    unsigned char scan_max_gain_thresh;
    enum wifi_scan_noise scan_noisy_status;
    unsigned char is_connect_set_gain;
    unsigned char bt_lk;

    unsigned char wm_doth_tbtt;
    unsigned char wm_doth_channel;
    unsigned char wm_tx_streams;
    unsigned char wm_rx_streams;

    struct list_head wm_amsdu_txq;
    struct list_head free_amsdu_list;
    struct os_timer_ext wm_amsdu_flush_timer;
    spinlock_t wm_amsdu_txq_lock;
    /*buffered msdu num of per tid*/
    int msdu_cnt[WME_NUM_TID ];
    struct msdu_list msdu_node_list;

    unsigned short wm_p2p_home_channel;
    unsigned char wm_p2p_connection_protect;
    unsigned long wm_p2p_connection_protect_period;
    unsigned char is_miracast_connect;
    unsigned char vsdb_mode_set_noa_enable;

    struct drv_txdesc *txdesc_bufptr;/* TX descriptors buffer point*/
    struct list_head txdesc_freequeue;/* transmit buffer */
    spinlock_t tx_desc_buf_lock;
    unsigned long tx_desc_buf_lock_flags;

    int wm_uapsd_qid;
    int wm_noqos_legacy_qid;
    int wm_mng_qid;
    int wm_mcast_qnum;

    /* suspend for all vif, so wifimac maintains suspend mode */
    unsigned int wm_suspend_mode;
    wait_queue_head_t wm_suspend_wq;
    unsigned char wm_esco_en;
    unsigned char wm_bt_en;

    enum wifi_mac_recovery_state recovery_stat;
    struct os_timer_ext wm_monitor_fw;
    spinlock_t fw_stat_lock;

#if defined(SU_BF) || defined(MU_BF)
    int max_spatial;
#endif
};

struct wifi_net_vif_ops
{
    void (*vm_set_tim)(struct wifi_station *, int);
    void (*vm_key_update_begin)(struct wlan_net_vif *);
    void (*vm_key_update_end)(struct wlan_net_vif *);

    void (*write_word)(unsigned int addr,unsigned int data);
    unsigned int (*read_word)(unsigned int addr);
    void (*bt_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*bt_read_word)(unsigned int addr);

    void (*pt_rx_start)(unsigned int qos);
    void (*pt_rx_stop)(void);
};
struct conn_chan_list
{
    struct wifi_channel *conn_chan[WIFINET_CHAN_MAX];
    unsigned char  da[WIFINET_ADDR_LEN];
    unsigned char  bssid[WIFINET_ADDR_LEN];
    unsigned num;
};

struct wifi_arp_info
{
    unsigned char  src_mac_addr[WIFINET_ADDR_LEN];
    unsigned char  src_ip_addr[IPV4_LEN];
    unsigned char  out;
};

struct wifi_mac_forward
{
    struct sk_buff *skb;
    int hdrspace;
    unsigned char fwd_flag;
};

struct wifi_mac_wmm_ac_params
{
    unsigned char direction;
    unsigned char psb[8];
    unsigned char fixed[8];
    unsigned char access_cat[4];
    unsigned char dialog_token;
    unsigned char tid;
    unsigned short size;
    unsigned short max_size;
    unsigned short sba;
    unsigned short medium_time;
    unsigned int mean_data_rate;
    unsigned int phyrate;
    unsigned int up;
    unsigned int min_srvc_intrvl;
    unsigned int inactivity;
    unsigned int suspension;
    unsigned int srvc_start_time;
    unsigned int min_data_rate;
    unsigned int peak_data_rate;
    unsigned int burst_size;
    unsigned int delay_bound;
};

struct wlan_net_vif
{
    struct list_head vm_next;
    struct net_device *vm_ndev;
    struct wifi_mac *vm_wmac;
    struct net_device_stats vm_devstats;
    struct iw_statistics vm_iwstats;
    struct vlan_group *vm_vlgrp;
    struct wireless_dev *vm_wdev;

    struct wifi_station *vm_mainsta;
    struct wifi_mac_statistic vif_sts;
    struct wifi_station_tbl vm_sta_tbl;
    struct wifi_mac_wme_state vm_wme;
    struct wifi_net_vif_ops vif_ops;
    unsigned int vm_debug;

    unsigned char wnet_vif_id;
    enum wifi_mac_opmode vm_opmode;
    enum wifi_mac_state vm_state;
    unsigned char remain_on_channel;
    struct ieee80211_channel remain_on_ch_channel;
    unsigned long long remain_on_ch_cookie;
    enum wifi_mac_pub_act_state pub_state;

    struct wifi_mac_ScanSSID vm_des_ssid[IV_SSID_SCAN_AMOUNT];
    unsigned char vm_des_bssid[WIFINET_ADDR_LEN];
    unsigned char vm_myaddr[WIFINET_ADDR_LEN];
    unsigned char vm_des_nssid;

    enum wifi_mac_macmode vm_mac_mode;
    struct wifi_channel *vm_curchan;
    struct wifi_channel *vm_switchchan;
    struct wifi_channel *vm_remainonchan;
    struct conn_chan_list vm_connchan;
    enum wifi_mac_bwc_width vm_bandwidth;
    unsigned char vm_scan_before_connect_flag;
    unsigned char vm_phase_flags; // bit0: connecting; bit1: disconnecting; bit2:flush_tx_buff_q
    int vm_scanchan_rssi;
    struct scaninfo_entry vm_connect_scan_entry;
    unsigned char vm_chan_simulate_scan_flag;
    unsigned char vm_chan_switch_scan_flag;
    unsigned char vm_chan_switch_scan_count;
    unsigned char vm_chan_roaming_scan_flag;
    unsigned char vm_chan_roaming_scan_count;

    struct wifi_arp_info  vm_arp_rx;
    void *vm_aclpriv;
    const struct wifi_mac_actuator *vm_aclop;

    unsigned int vm_flags;
    unsigned int vm_flags_ext;
    unsigned int vm_flags_ext2;
    unsigned int vm_caps;

    struct os_timer_ext vm_mgtsend;
    struct os_timer_ext vm_sm_switch;
    struct os_timer_ext vm_actsend;
    struct os_timer_ext vm_roc_timer;
    int vm_inact_init;
    int vm_inact_auth;
    int vm_inact_run;
    int vm_inact_probe;

    /*scan per channel wait time ms*/
    unsigned char vm_scan_time_idle;
    unsigned char vm_scan_time_connect;
    unsigned char vm_scan_time_before_connect;
    unsigned char vm_scan_hang;
    unsigned short vm_scan_time_chan_switch;

    unsigned int vm_aid_bitmap[howmany(WIFINET_AID_DEF, 32)];
    unsigned short vm_max_aid;
    unsigned short vm_sta_assoc;
    unsigned short vm_ps_sta;
    unsigned short vm_ps_pending;
    unsigned char* vm_tim_bitmap;
    unsigned short vm_tim_len;
    unsigned int vm_dtim_period;
    struct wifi_mac_beacon_offsets vm_beaconbuf_offset;
    void *vm_beaconbuf;
    int wnet_vif_replaycounter;
    unsigned short vm_bcn_intval;
    unsigned char mgmt_pkt_retry_count;

    unsigned char vm_uapsdinfo;
    struct wifi_mac_fixed_rate vm_fixed_rate;
    unsigned short vm_rtsthreshold;
    unsigned short vm_fragthreshold;
    unsigned char vm_opt_ie[WIFINET_MAX_IV_OPT_IE];
    unsigned short vm_opt_ie_len;

    /* for change channel when sta receive a channel change announce frame*/
    unsigned char vm_chanchange_count;
    unsigned char vm_bmiss_count;
    struct wifi_mac_rateset vm_legacy_rates;
    struct chan_switch_target_t csa_target;
    unsigned char csa_count;

    struct wifi_mac_wmm_ac_params vm_wmm_ac_params;
    struct wifi_mac_wmm_tspec_element tspecs[WME_AC_NUM][TS_DIR_IDX_COUNT];
    struct wifi_mac_app_ie_t app_ie[WIFINET_APPIE_NUM_OF_FRAME];
    struct wifi_mac_app_ie_t assocrsp_ie;
    struct wifi_mac_app_ie_t assocreq_ie;
    struct wifi_mac_pwrsave_t vm_pwrsave;
    unsigned char vm_ht40_intolerant;

    struct sk_buff_head vm_tx_buffer_queue;

    unsigned short vm_pstxqueue_flags;
    unsigned short vm_legacyps_txframes;

    struct sk_buff_head vm_forward_buffer_queue;
    struct wifi_mac_forward vm_forward;
    enum hal_op_mode vm_hal_opmode;
#ifdef  AML_MCAST_QUEUE
    int vm_mqueue_flag_send;
#endif

    unsigned char vm_def_txkey;
    unsigned char vm_def_mgmt_txkey;
    struct wifi_mac_key vm_nw_keys[WIFINET_WEP_NKID];
    unsigned int current_keytype;
    /* wpa2 pmk list */
    struct aml_pmk_list *pmk_list;

    /* Short Guard Interval Enable:1 Disable:0 */
    /* Short Guard Interval Enable:1 Disable:0 for VHT fixed rates */
    /* LDPC Enable Rx:1 TX: 2 ; Disable:0 */
    /* flag to indicate using default ratemask */
    /*For wakeup AP vmac when wds-sta connect to the AP only use
      when export UMAC_REPEATER_DELAYED_BRINGUP=1*/
    /* if performed the iwlist scanning */
    unsigned int vm_sgi:1,
                 vm_data_sgi:1,
                 vm_ldpc:2,
                 vm_ratemask_default:1,
                 vm_key_flag:1,
                 vm_list_scanning:1;

    unsigned short vm_htcap;
    /*only for ht info ie */
    int scnd_chn_offset;
    /* TX STBC Enable:1 Disable:0 */
    unsigned char vm_tx_stbc;
    /* RX STBC Enable:(1,2,3) Disable:0 */
    unsigned char vm_rx_stbc;

    unsigned int vm_vhtcap;
    /* VHT Supported MCS set */
    struct wifinet_vht_mcs_set vm_vhtcap_max_mcs;
    /* VHT Basic MCS set */
    unsigned short vm_vhtop_basic_mcs;

    unsigned short vm_tx_speed;
    unsigned short vm_rx_speed;
    unsigned char vm_change_rate_enable;
    unsigned long long pn_window[2][2];
#ifdef CONFIG_P2P
    struct wifi_mac_p2p* vm_p2p;
    int vm_p2p_support;
#endif
};

#define WIFINET_PSQUEUE_PS4QUIET 0x0001
#define WIFINET_PSQUEUE_OPPS 0x0002
#define WIFINET_PSQUEUE_NOA 0x0004
#define WIFINET_PSQUEUE_MASK (WIFINET_PSQUEUE_PS4QUIET|WIFINET_PSQUEUE_OPPS|WIFINET_PSQUEUE_NOA)

#define WIFINET_ADDR_EQ(a1,a2) (memcmp(a1,a2,WIFINET_ADDR_LEN) == 0)
#define WIFINET_ADDR_COPY(dst,src) memcpy(dst,src,WIFINET_ADDR_LEN)

#define WIFINET_F_RDG 0x00000001

#define WIFINET_F_PRIVACY 0x00000010
#define WIFINET_F_NOSCAN 0x00000040
#define WIFINET_F_SCAN 0x00000080
#define WIFINET_F_HT 0x00000100
#define WIFINET_F_SIBSS 0x00000200

#define WIFINET_F_SHSLOT 0x00000400
#define WIFINET_F_DESBSSID 0x00001000
#define WIFINET_F_WME 0x00002000
#define WIFINET_F_IGNORE_SSID 0x00004000
#define WIFINET_F_LDPC 0x00008000
#define WIFINET_F_SU_BF 0x00010000
#define WIFINET_F_MU_BF 0x00020000
#define WIFINET_F_SHPREAMBLE 0x00040000
#define WIFINET_F_DATAPAD 0x00080000
#define WIFINET_F_USEPROT 0x00100000
#define WIFINET_F_USEBARKER 0x00200000
#define WIFINET_F_TIMUPDATE 0x00400000
#define WIFINET_F_WPA1 0x00800000
#define WIFINET_F_WPA2 0x01000000
#define WIFINET_F_WPA 0x01800000
#define WIFINET_F_COUNTERM 0x04000000
#define WIFINET_F_HIDESSID 0x08000000
#define WIFINET_F_HIDESSID_TO_BIT0_OFST 27
#define WIFINET_F_WMEUPDATE 0x20000000
#define WIFINET_F_DOTH 0x40000000
#define WIFINET_F_CHANSWITCH 0x80000000


#define WIFINET_FEXT_SHORTGI_ENABLE 0x00000001
#define WIFINET_FEXT_SHORTGI20 0x00000002
#define WIFINET_FEXT_SHORTGI40 0x00000004
#define WIFINET_FEXT_SHORTGI80 0x00000008
#define WIFINET_FEXT_UAPSD 0x00000010
#define WIFINET_FEXT_SLEEP 0x00000020
#define WIFINET_FEXT_CHANNELCHANGE 0x00000080
#define WIFINET_FEXT_WDS 0x00000100
#define WIFINET_FEXT_ERPUPDATE 0x00000200
#define WIFINET_FEXT_SWBMISS 0x00000400
#define WIFINET_FEXT_APPIE_UPDATE 0x00001000
#define WIFINET_FEXT_AMPDU 0x00004000
#define WIFINET_FEXT_AMSDU 0x00008000
#define WIFINET_F_NONHT_AP 0x00040000
#define WIFINET_FEXT_HTUPDATE 0x00080000
#define WIFINET_C_WDS_AUTODETECT 0x00100000
#define WIFINET_C_STA_FORWARD 0x00400000
#define WIFINET_FEXT_BR_UPDATE 0x01000000
#define WIFINET_FEXT_COEXT_DISABLE 0x20000000
#define WIFINET_FEXT_COUNTRYIE 0x40000000
#define WIFINET_FEXT_WMETUN 0x80000000

#define WIFINET_FEXT2_PUREB 0x10000000
#define WIFINET_FEXT2_PUREG 0x20000000
#define WIFINET_FEXT2_PUREN 0x40000000
#define WIFINET_FEXT2_SWITCH_CHANNEL 0x80000000
#define WIFINET_FEXT2_ALLOW_SWITCH_CHANNEL 0x01000000


#define WIFINET_FEXT2_PUREBGN (WIFINET_FEXT2_PUREB|WIFINET_FEXT2_PUREG|WIFINET_FEXT2_PUREN)
#define WIFINET_FEXT2_PUREBG (WIFINET_FEXT2_PUREG|WIFINET_FEXT2_PUREB)
#define WIFINET_COM_UAPSD_ENABLE(_ic) ((_ic)->wm_flags_ext |= WIFINET_FEXT_UAPSD)

#define WIFINET_VMAC_UAPSD_ENABLE(_v) ((_v)->vm_flags_ext |= WIFINET_FEXT_UAPSD)
#define WIFINET_VMAC_UAPSD_DISABLE(_v) ((_v)->vm_flags_ext &= ~WIFINET_FEXT_UAPSD)
#define WIFINET_VMAC_UAPSD_ENABLED(_v) ((_v)->vm_flags_ext & WIFINET_FEXT_UAPSD)
#define WIFINET_VMAC_IS_SLEEPING(_v) ((_v)->vm_flags_ext & WIFINET_FEXT_SLEEP)

#define WIFINET_C_WEP 0x00000001
#define WIFINET_C_TKIP 0x00000002
#define WIFINET_C_AES 0x00000004
#define WIFINET_C_AES_CCM 0x00000008
#define WIFINET_C_WAPI 0x00000010
#define WIFINET_C_RDG 0x00000020
#define WIFINET_C_HT  0x00000040
#define WIFINET_C_HWCSUM 0x00000080
#define WIFINET_C_IBSS 0x00000100
#define WIFINET_C_HOSTAP 0x00000400
#define WIFINET_C_SHSLOT 0x00004000
#define WIFINET_C_SHPREAMBLE 0x00008000
#define WIFINET_C_MONITOR 0x00010000
#define WIFINET_C_TKIPMIC 0x00020000
#define WIFINET_C_BWCTRL 0x00080000
#define WIFINET_C_SU_BF 0x00100000
#define WIFINET_C_MU_BF 0x00200000
#define WIFINET_C_MFP 0x00400000
#define WIFINET_C_WPA 0x01800000
#define WIFINET_C_BURST 0x02000000
#define WIFINET_C_WME 0x04000000
#define WIFINET_C_WDS 0x08000000
#define WIFINET_C_BGSCAN 0x20000000
#define WIFINET_C_UAPSD 0x40000000


unsigned int   wifi_mac_Mhz2ieee(unsigned int, unsigned int);
int wifi_mac_ComSetCountryCode(struct wifi_mac *wifimac, char* isoName);

static __inline void
wifi_mac_KeyUpdateBegin(struct wlan_net_vif *wnet_vif)
{
    if (wnet_vif->vif_ops.vm_key_update_begin != NULL)
    {
        wnet_vif->vif_ops.vm_key_update_begin(wnet_vif);
    }
}

static __inline void
wifi_mac_KeyUpdateEnd(struct wlan_net_vif *wnet_vif)
{
    if (wnet_vif->vif_ops.vm_key_update_end != NULL)
    {
        wnet_vif->vif_ops.vm_key_update_end(wnet_vif);
    }
}

static __inline  int wifi_mac_hdrsize(const void *data)
{
    const struct wifi_frame *wh = data;
    int size = sizeof(struct wifi_frame);

    KASSERT((wh->i_fc[0]&WIFINET_FC0_TYPE_MASK) != WIFINET_FC0_TYPE_CTL,
            ("%s: control frame", __func__));
    if ((wh->i_fc[1] & WIFINET_FC1_DIR_MASK) == WIFINET_FC1_DIR_DSTODS)
        size += WIFINET_ADDR_LEN;

    if (WIFINET_QOS_HAS_SEQ(wh))
    {
        size += sizeof(unsigned short);
        if (WIFINET_QOS_HAS_HTC(wh))
            size += 4;
    }
    else if(WIFINET_IS_MGM(wh))
    {
        if (WIFINET_QOS_HAS_HTC(wh))
            size += 4;
    }

    return size;
}

static __inline  int wifi_mac_anyhdrsize(const void *data)
{
    const struct wifi_frame *wh = data;

    if (WIFINET_IS_CRTL(wh))
    {
        switch (wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK)
        {
            case WIFINET_FC0_SUBTYPE_CTS:
            case WIFINET_FC0_SUBTYPE_ACK:
                return sizeof(struct wifi_mac_frame_ack);
        }
        return sizeof(struct wifi_mac_frame_min);
    }
    else
        return wifi_mac_hdrsize(data);
}


static __inline int
wifi_mac_hdrspace(struct wifi_mac *wifimac, const void *data)
{
    int size = wifi_mac_hdrsize(data);
    if (wifimac->wm_flags & WIFINET_F_DATAPAD)
        size = roundup(size, sizeof(unsigned int));
    return size;
}

static __inline int
wifi_mac_anyhdrspace(struct wifi_mac *wifimac, const void *data)
{
    int size = wifi_mac_anyhdrsize(data);
    if (wifimac->wm_flags & WIFINET_F_DATAPAD)
        size = roundup(size, sizeof(unsigned int));
    return size;
}


static __inline int
wifi_mac_is_ht_enable(struct wifi_station *sta)
{
    struct wifi_mac_key *k = NULL;

    if (sta == NULL) {
        return 0;
    }

    //wep
    if ((sta->sta_ucastkey.wk_cipher == &wifi_mac_cipher_none)
        && (sta->sta_wnet_vif->vm_def_txkey != WIFINET_KEYIX_NONE)) {
        k = &sta->sta_wnet_vif->vm_nw_keys[sta->sta_wnet_vif->vm_def_txkey];

        if (k && k->wk_cipher->wm_cipher == WIFINET_CIPHER_WEP) {
            return 0;
        }
    }

    //tkip, check sta's & ap's cipher capability
    if ((sta->sta_wnet_vif->vm_flags & WIFINET_F_WPA)
        && (sta->sta_rsn.rsn_ucastcipherset == (1 << WIFINET_CIPHER_TKIP))) {
        return 0;
    }

    //check current connection's unicast cipher
    if (sta->sta_rsn.rsn_ucastcipher == WIFINET_CIPHER_TKIP) {
        return 0;
    }

    //mode
    if (sta->sta_wnet_vif->vm_mac_mode < WIFINET_MODE_11N) {
        return 0;
    }

    return 1;
}

static __inline int
wifi_mac_is_vht_enable(struct wlan_net_vif *wnet_vif)
{
    /*judge sequence: first HT then VHT*/
    if (wifi_mac_is_ht_enable(wnet_vif->vm_mainsta)
        && (wnet_vif->vm_mac_mode >= WIFINET_MODE_11AC)
        && (wnet_vif->vm_mac_mode <= WIFINET_MODE_11GNAC)) {
        return 1;
    }

    return 0;
}

#define IS_RUNNING(_dev)\
        (((_dev)->flags & (IFF_RUNNING|IFF_UP)) == (IFF_RUNNING|IFF_UP))

#define IS_UP(_dev) (((_dev)->flags & (IFF_UP)) == (IFF_UP))

struct wifi_mac_actuator
{
    const char *iac_name;
    int (*iac_attach)(struct wlan_net_vif *);
    void (*iac_detach)(struct wlan_net_vif *);
    int (*iac_check)(struct wlan_net_vif *, const unsigned char mac[WIFINET_ADDR_LEN]);
    int (*iac_add)(struct wlan_net_vif *, const unsigned char mac[WIFINET_ADDR_LEN]);
    int (*iac_remove)(struct wlan_net_vif *, const unsigned char mac[WIFINET_ADDR_LEN]);
    int (*iac_flush)(struct wlan_net_vif *);
    int (*iac_setpolicy)(struct wlan_net_vif *, int);
    int (*iac_getpolicy)(struct wlan_net_vif *);
};

#define wifi_mac_send_action(_sta, _arg)\
        wifi_mac_send_mgmt((_sta), WIFINET_FC0_SUBTYPE_ACTION, (void *)(_arg))

#define wifi_mac_send_qosnulldata_as_trigger(sta, qosinfo)\
    wifi_mac_send_qosnulldata((sta), wme_sta_uapsd_get_triggered_ac((qosinfo)))

#define VMAC_DEV_NAME(wnet_vif)    ((wnet_vif)->vm_ndev->name)
#define DEV_NAME(dev)    ((dev)->name)

#define RESCAN  1

int remove_app_ie(unsigned int frame_type_index,struct wlan_net_vif *wnet_vif);
extern const unsigned char BROADCAST_ADDRESS[WIFINET_ADDR_LEN];
#endif /* _NET80211_IEEE80211_VAR_H_ */
