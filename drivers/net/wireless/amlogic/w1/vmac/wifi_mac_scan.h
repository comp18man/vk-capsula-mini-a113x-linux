/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer scan module
 *
 *
 ****************************************************************************************
 */

#ifndef _WIFI_NET_SCAN_H_
#define _WIFI_NET_SCAN_H_

#define WIFINET_SCAN_MAX_SSID   16
#define ROAMING_CANDIDATE_CHAN_MAX 6

struct wifi_mac_ScanSSID
{
    int  len;
    unsigned char ssid[WIFINET_NWID_LEN];
};

enum
{
    WIFINET_SCANCFG_NOPICK =BIT(0),
    WIFINET_SCANCFG_ACTIVE =BIT(1),
    WIFINET_SCANCFG_USERREQ =BIT(2),
    WIFINET_SCANCFG_FLUSH =BIT(3),
    WIFINET_SCANCFG_FORCE =BIT(5),
    WIFINET_SCANCFG_CREATE =BIT(6),
    WIFINET_SCANCFG_CHANSET =BIT(7),
    WIFINET_SCANCFG_CONNECT =BIT(8),
};
#define WIFINET_SCANCFG_MASK 0xfff

enum
{
    SCANSTATE_F_DISCARD = BIT(0),
    SCANSTATE_F_CANCEL = BIT(1),
    SCANSTATE_F_START = BIT(2),
    SCANSTATE_F_WAIT_TBTT = BIT(3),
    SCANSTATE_F_TIMEOUT = BIT(4),
    SCANSTATE_F_RESTORE = BIT(5),
    SCANSTATE_F_NOTIFY_AP = BIT(6),
    SCANSTATE_F_TX_DONE = BIT(7),
    SCANSTATE_F_WAIT_PKT_CLEAR = BIT(8),
    SCANSTATE_F_DISCONNECT_REQ_CANCEL = BIT(9),
    SCANSTATE_F_WAIT_CHANNEL_SWITCH = BIT(10),
    SCANSTATE_F_CHANNEL_SWITCH_COMPLETE = BIT(11),
    SCANSTATE_F_SEND_PROBEREQ_AGAIN = BIT(12),
    SCANSTATE_F_GET_CONNECT_AP = BIT(13),
    SCANSTATE_F_RX_LEAKAP_HAPPEN = BIT(14),
    SCANSTATE_F_RX_CHKING_LEAKAP_PKT = BIT(15),
};


enum wifi_scan_noise
{
    WIFINET_S_SCAN_ENV_CLEAR  = 0,
    WIFINET_S_SCAN_ENV_NOISE  = 1,
    WIFINET_S_SCAN_ENV_MID  = 2,
};


#define STA_HASHSIZE 32
#define STA_HASH(addr) (((const unsigned char *)(addr))[WIFINET_ADDR_LEN - 1] % STA_HASHSIZE)

struct scaninfo_table
{
    spinlock_t  st_lock;
    struct list_head st_entry;
    struct list_head st_hash[STA_HASHSIZE];
};

struct wifi_mac_scan_state
{
    struct wlan_net_vif *VMacPriv;;
    struct scaninfo_table *ScanTablePriv;
    unsigned short scan_CfgFlags;
    unsigned char ss_nssid;

    /*save specified ssid to scan */
    struct wifi_mac_ScanSSID ss_ssid[WIFINET_SCAN_MAX_SSID];
    /*save channel to scan, include 2.4G and 5G */
    struct wifi_channel *ss_chans[WIFINET_CHAN_MAX];

    struct wifi_mac_ScanSSID roaming_ssid;
    struct wifi_candidate_channel roaming_candidate_chans[ROAMING_CANDIDATE_CHAN_MAX];
    int roaming_candidate_chans_cnt;
    unsigned char roaming_full_scan;
    spinlock_t  roaming_chan_lock;
    unsigned long roaming_chan_lock_flag;
    unsigned short scan_ssid_count;

    unsigned short scan_next_chan_index;
    unsigned short scan_last_chan_index;
    unsigned short scan_chan_wait;
    unsigned int scan_StateFlags;
    spinlock_t  scan_lock;
    unsigned long scan_lock_flag;

    struct os_timer_ext ss_scan_timer;
    struct os_timer_ext ss_probe_timer;
    struct os_timer_ext ss_scan_abort_timer;
    struct hrtimer scan_hr_timer;
    ktime_t scan_kt;
    struct work_struct timeout_work;
};


#define WIFI_SCAN_SE_LIST_LOCK(_st) OS_SPIN_LOCK(&(_st)->st_lock)
#define WIFI_SCAN_SE_LIST_UNLOCK(_st) OS_SPIN_UNLOCK(&(_st)->st_lock)

#define WIFI_SCAN_LOCK(_ss) OS_SPIN_LOCK_IRQ(&(_ss)->scan_lock, (_ss)->scan_lock_flag)
#define WIFI_SCAN_UNLOCK(_ss) OS_SPIN_UNLOCK_IRQ(&(_ss)->scan_lock, (_ss)->scan_lock_flag)

#define WIFI_ROAMING_CHANNLE_LOCK(_ss)OS_SPIN_LOCK_IRQ(&(_ss)->roaming_chan_lock, (_ss)->roaming_chan_lock_flag)
#define WIFI_ROAMING_CHANNLE_UNLOCK(_ss)OS_SPIN_UNLOCK_IRQ(&(_ss)->roaming_chan_lock, (_ss)->roaming_chan_lock_flag)

struct wifi_mac_scan_param
{
    unsigned short capinfo;
    unsigned short status_code;
    unsigned short assoc_id;
    unsigned char chan;
    unsigned char erp;
    unsigned char bintval;
    unsigned char timoff;
    unsigned char *tim;
    unsigned char *tstamp;
    unsigned char *country;
    unsigned char *bss_load;
    unsigned char *ssid;
    unsigned char *rates;
    unsigned char *xrates;
    unsigned char *doth;
    unsigned char *wpa;
    unsigned char *wme;
    unsigned char *htcap;
    unsigned char *htinfo;
    unsigned char *tpc;
    unsigned char *quiet;
    unsigned char *rsn;
    unsigned char *rsnxe;
    unsigned char *wps;
    unsigned char *timeout_ie;
#ifdef CONFIG_P2P
    unsigned char *p2p[MAX_P2PIE_NUM];
    unsigned char *p2p_noa;
    unsigned char *wfd;
#endif //#ifdef CONFIG_P2P

#ifdef CONFIG_WAPI
    unsigned char *wai;
#endif //#ifdef CONFIG_WAPI
    unsigned char *ext_cap;
    unsigned char *vht_cap;
    unsigned char *vht_opt;
    unsigned char *vht_tx_pwr;
    unsigned char *vht_ch_sw;
    unsigned char *vht_ext_bss_ld;
    unsigned char *vht_quiet_ch;
    unsigned char *vht_opt_md_ntf;
    unsigned int frame_len;
};

#define SCANINFO_IE_LENGTH     260
#define SCANINFO_IE_DATA_LENGTH         255
struct wifi_scan_info
{
    unsigned char  SI_macaddr[WIFINET_ADDR_LEN];
    unsigned char  SI_bssid[WIFINET_ADDR_LEN];
    unsigned char  SI_ssid[2+WIFINET_NWID_LEN];
    unsigned char  SI_rates[2+WIFINET_RATE_MAXSIZE];
    unsigned char  SI_exrates[2+WIFINET_RATE_MAXSIZE];
    union
    {
        unsigned char  data[8];
        u_int64_t   tsf;
    } SI_tstamp;
    unsigned short SI_intval;
    unsigned short SI_capinfo;
    struct wifi_channel *SI_chan;
    unsigned short SI_timoff;
    unsigned char  SI_erp;
    int SI_rssi;
    unsigned char  SI_dtimperiod;
    unsigned char  SI_wpa_ie[SCANINFO_IE_LENGTH];
    unsigned char  SI_wme_ie[SCANINFO_IE_LENGTH];
    unsigned char  SI_htcap_ie[SCANINFO_IE_LENGTH];
    unsigned char  SI_htinfo_ie[SCANINFO_IE_LENGTH];
    unsigned char  SI_country_ie[SCANINFO_IE_LENGTH];
    unsigned char  SI_bss_load_ie[SCANINFO_IE_LENGTH];
    unsigned int SI_age;
    unsigned char  SI_rsn_ie[SCANINFO_IE_LENGTH];
    unsigned char  SI_rsnx_ie[SCANINFO_IE_LENGTH];
    unsigned short si_rsn_capa;
    unsigned char  SI_wps_ie[SCANINFO_IE_LENGTH];

#ifdef CONFIG_WAPI
    unsigned char  SI_wai_ie[SCANINFO_IE_LENGTH];
#endif /*CONFIG_WAPI*/

#ifdef CONFIG_P2P
    unsigned char  SI_p2p_ie[MAX_P2PIE_NUM][SCANINFO_IE_LENGTH];
#endif /*CONFIG_P2P*/
#ifdef CONFIG_WFD
    unsigned char  SI_wfd_ie[SCANINFO_IE_LENGTH];
#endif /*CONFIG_WFD*/

    unsigned char ie_ext_cap[sizeof(struct wifi_mac_ie_ext_cap )];
    unsigned char ie_vht_cap[sizeof(struct wifi_mac_ie_vht_cap )];
    unsigned char ie_vht_opt[sizeof(struct wifi_mac_ie_vht_opt)];
    unsigned char ie_vht_tx_pwr[sizeof(struct wifi_mac_ie_vht_txpwr_env)];
    unsigned char ie_vht_ch_sw[sizeof(struct wifi_mac_ie_vht_ch_sw_wrp)];
    unsigned char ie_vht_ext_bss_ld[sizeof(struct wifi_mac_ie_vht_ext_bss_ld)];
    unsigned char ie_vht_quiet_ch[sizeof(struct wifi_mac_ie_vht_quiet_chn)];
    unsigned char ie_vht_opt_md_ntf[sizeof(struct wifi_mac_ie_vht_opt_md_ntf)];
    unsigned int SI_frame_len;
};

#define WIFINET_SCAN_AGE_NUM 3
#define WIFINET_CONNECT_FAILS 5
#define WIFINET_CONNECT_CNT_AGE (120)
#define SCAN_VALID_DEFAULT (30*HZ)
#define WIFINET_SCAN_MOTHER_CHANNEL_TIME 20

#define WIFINET_SCAN_DEFAULT_INTERVAL 100
#define WIFINET_SCAN_TIME_IDLE_DEFAULT 80
#define WIFINET_SCAN_PROTECT_TIME 1000
#define WIFINET_SCAN_TIME_CHANNEL_SWITCH 500
#define WIFINET_SCAN_TIME_BEFORE_CONNECT 200
#define WIFINET_SCAN_ABORT_TIME 10000

#ifndef FW_RF_CALIBRATION
#define WIFINET_SCAN_TIME_CONNECT_DEFAULT 50
#define WIFINET_SCAN_DEFAULT_INTERVAL_COEXIST 200
#define WIFINET_SCAN_CHANNEL_COST 20
#define WIFINET_SCAN_INTERVAL_LEFT 10
#else
#define WIFINET_SCAN_TIME_CONNECT_DEFAULT 20
#define WIFINET_SCAN_DEFAULT_INTERVAL_COEXIST 200
#define WIFINET_SCAN_CHANNEL_COST 30
#define WIFINET_SCAN_INTERVAL_LEFT 10
#endif

#define STA_MATCH_ERR_CHAN                  (1<<0)
#define STA_MATCH_ERR_BSS                   (1<<1)
#define STA_MATCH_ERR_PRIVACY               (1<<2)
#define STA_MATCH_ERR_RATE                  (1<<3)
#define STA_MATCH_ERR_HTRATE                (1<<4)
#define STA_MATCH_ERR_SSID                  (1<<5)
#define STA_MATCH_ERR_BSSID                 (1<<6)
#define STA_MATCH_ERR_STA_FAILS_MAX         (1<<7)
#define STA_MATCH_ERR_STA_PURGE_SCANS       (1<<8)

#define LEAKY_AP_DET_WIN                    20
struct scaninfo_entry
{
    struct wifi_scan_info scaninfo;
    struct list_head se_list;
    struct list_head se_hash;

    unsigned char connectcnt; //
    unsigned char se_valid;
    unsigned int se_avgrssi;
    unsigned long LastUpdateTime;
    unsigned long ConnectTime;
};

void wifi_mac_scan_attach(struct wifi_mac *);
void wifi_mac_scan_detach(struct wifi_mac *);
void wifi_mac_scan_vattach(struct wlan_net_vif *);
void wifi_mac_scan_vdetach(struct wlan_net_vif *);
int wifi_mac_start_scan(struct wlan_net_vif *, int flags, unsigned int nssid, const struct wifi_mac_ScanSSID ssids[]);
int wifi_mac_chk_scan(struct wlan_net_vif *, int flags, unsigned int nssid, const struct wifi_mac_ScanSSID ssids[]);
void wifi_mac_cancel_scan(struct wifi_mac *wifimac);
void wifi_mac_scan_rx(struct wlan_net_vif *, const struct wifi_mac_scan_param *, const struct wifi_frame *, int rssi, struct scaninfo_entry *oldse);
void wifi_mac_scan_flush(struct wifi_mac *);
int wifi_mac_scan_timeout_ex(void *arg);

typedef int wifi_mac_ScanIterFunc(void *, const struct wifi_scan_info *);
int wifi_mac_scan_parse(struct wlan_net_vif *, wifi_mac_ScanIterFunc, void *);
int saveie(unsigned char *iep, const unsigned char *ie);
void wifi_mac_set_scan_time(struct wlan_net_vif *wnet_vif);
int vm_scan_user_set_chan(struct wlan_net_vif *wnet_vif,struct cfg80211_scan_request *request);
int vm_is_p2p_connect_scan(struct wlan_net_vif *wnet_vif, struct cfg80211_scan_request *request);
void scan_next_chan(struct wifi_mac *wifimac);
void wifi_mac_end_scan( struct wifi_mac_scan_state *ss);
void wifi_mac_check_switch_chan_result(struct wlan_net_vif * wnet_vif);
void wifi_mac_notify_ap_success(struct wlan_net_vif *wnet_vif);
void wifi_mac_notify_pkt_clear(struct wifi_mac *wifimac);
void quiet_all_intf (struct wifi_mac *wifimac, unsigned char enable);
void wifi_mac_scan_notify_leave_or_back(struct wlan_net_vif *wnet_vif, unsigned char enable);
void wifi_mac_process_tx_error(struct wlan_net_vif *wnet_vif);
void wifi_mac_update_roaming_candidate_chan(struct wlan_net_vif *wnet_vif,const struct wifi_mac_scan_param *sp, int rssi);
int wifi_mac_scan_chk_11g_bss(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif);
void is_connect_need_set_gain(struct wlan_net_vif *wnet_vif);
void wifi_mac_scan_chking_leakap(void * station, struct wifi_frame *wh);
struct wifi_channel* wifi_mac_connect_get_target_chan(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif);
#endif /* _WIFI_NET_SCAN_H_ */
