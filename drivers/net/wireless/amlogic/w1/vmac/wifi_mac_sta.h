/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer station node control module
 *
 *
 ****************************************************************************************
 */
#ifndef _NET80211_IEEE80211_NODE_H_
#define _NET80211_IEEE80211_NODE_H_

#include <asm/byteorder.h>
#ifdef CONFIG_NET_WIRELESS
#include <linux/wireless.h>
#endif
#include "wifi_mac_encrypt.h"
#include <net/mac80211.h>
#include "rc80211_minstrel.h"

#define WIFINET_INACT_WAIT 1
#define WIFINET_INACT_INIT (30/WIFINET_INACT_WAIT)
#define WIFINET_INACT_AUTH (180/WIFINET_INACT_WAIT)
#define WIFINET_INACT_RUN (300/WIFINET_INACT_WAIT)
#define WIFINET_INACT_PROBE (30/WIFINET_INACT_WAIT)
#define WIFINET_INACT_NONERP 10
#define WIFINET_INACT_HT 10

#define WIFINET_TRANS_WAIT 5000
#define WIFINET_NODE_FREE_WAIT 5000
#define WIFINET_NODE_HASHSIZE 32
#define WIFINET_NODE_HASH(addr) (((const unsigned char *)(addr))[WIFINET_ADDR_LEN - 1] % WIFINET_NODE_HASHSIZE)
#define WIFINET_KEEP_ALIVE 28000
#define ENABLE  1
#define DISABLE  0

struct wifi_mac_Rsnparms
{
    unsigned char rsn_mcastcipher;
    unsigned char rsn_mcastkeylen;
    unsigned char rsn_ucastcipherset;
    unsigned char rsn_ucastcipher;
    unsigned char rsn_ucastkeylen;
    unsigned short rsn_keymgmtset;
    unsigned short rsn_caps;
    unsigned char rsn_caps_offset;
    unsigned char rsn_pmkid_count;
    unsigned char rsn_pmkid_offset;
    unsigned char rsn_pmkid[16];
    unsigned char rsn_gmcipher;
    unsigned char sa_valid;
    unsigned char gtksa;
    unsigned char ptksa;
};

struct wifi_station_tbl;
struct wifi_mac;
struct wlan_net_vif;
struct wifi_scan_info;

//sta flag
#define WIFINET_NODE_AUTH 0x0001
#define WIFINET_NODE_QOS 0x0002
#define WIFINET_NODE_ERP 0x0004
#define WIFINET_NODE_HT 0x0008

#define WIFINET_NODE_PWR_MGT 0x0010
#define WIFINET_NODE_AREF 0x0020
#define WIFINET_NODE_UAPSD 0x0040
#define WIFINET_NODE_UAPSD_TRIG 0x0080

#define WIFINET_NODE_UAPSD_SP 0x0100
#define WIFINET_NODE_LDPC 0x0200
#define WIFINET_NODE_OWL_WDSWAR 0x0400
#define WIFINET_NODE_WDS 0x0800

#define WIFINET_NODE_VHT 0x2000

//sta ext flag
#define WIFINET_NODE_MFP 0x0001
#define WIFINET_NODE_MFP_CONFIRM_DEAUTH 0x0002
#define WIFINET_NODE_REASSOC 0x0004
#define WIFINET_NODE_RDG 0x0008

#define WIFINET_NODE_40_INTOLERANT 0x0400
#define WIFINET_NODE_PS_FLUSH_WAIT_NOA_END 0x0800
#define WIFINET_NODE_UAPSD_WAIT_NOA_END 0x1000
#define WIFINET_NODE_UAPSD_FLUSH_WAIT_NOA_END 0x2000
#define WIFINET_NODE_TRIGGER_WAIT_NOA_END 0x4000

//other flag
#define WIFINET_NODE_SM_EN BIT(0)
#define WIFINET_NODE_SM_PWRSAV_STAT BIT(1)
#define WIFINET_NODE_SM_PWRSAV_DYN BIT(2)
#define WIFINET_NODE_RATECHG BIT(3)

struct wifi_sta_statistic
{
    unsigned int ns_rx_data;
    unsigned int ns_rx_mgmt;
    unsigned int ns_rx_ctrl;
    unsigned int ns_rx_ucast;
    unsigned int ns_rx_mcast;
    unsigned int ns_rx_dup_cnt;
    u_int64_t ns_rx_bytes;
    u_int64_t ns_rx_beacons;
    unsigned int ns_rx_proberesp;

    unsigned int ns_rx_dup;
    unsigned int ns_rx_noprivacy;
    unsigned int ns_rx_wepfail;
    unsigned int ns_rx_demicfail;
    unsigned int ns_rx_decap;
    unsigned int ns_rx_defrag;
    unsigned int ns_rx_disassoc;
    unsigned int ns_rx_deauth;
    unsigned int ns_rx_action;
    unsigned int ns_rx_decryptcrc;
    unsigned int ns_rx_unauth;
    unsigned int ns_rx_unencrypted;

    unsigned int ns_tx_data;
    unsigned int ns_tx_mgmt;
    unsigned int ns_tx_ucast;
    unsigned int ns_tx_mcast;
    u_int64_t ns_tx_bytes;
    unsigned int ns_tx_probereq;
    unsigned int ns_tx_uapsd;

    unsigned int ns_tx_novlantag;
    unsigned int ns_tx_vlanmismatch;
    unsigned int ns_tx_eosplost;
    unsigned int ns_ps_discard;

    unsigned int ns_uapsd_triggers;
    unsigned int ns_uapsd_duptriggers;
    unsigned int ns_uapsd_ignoretriggers;
    unsigned int ns_uapsd_active;
    unsigned int ns_uapsd_triggerenabled;

    unsigned int ns_tx_assoc;
    unsigned int ns_tx_assoc_fail;
    unsigned int ns_tx_auth;
    unsigned int ns_tx_auth_fail;
    unsigned int ns_tx_deauth;
    unsigned int ns_tx_deauth_code;
    unsigned int ns_tx_disassoc;
    unsigned int ns_tx_disassoc_code;
    unsigned int ns_psq_drops;
};

struct wifi_station
{
    struct wlan_net_vif *sta_wnet_vif;
    struct wifi_mac *sta_wmac;
    struct list_head sta_list;

    void *drv_sta;
    struct wifi_mac_amsdu *sta_amsdu;
    enum wifi_mac_macmode sta_bssmode; /* current phy mode */

    unsigned char wnet_vif_id;
    unsigned char sta_esslen;
    unsigned char sta_essid[WIFINET_NWID_LEN];
    unsigned char sta_macaddr[WIFINET_ADDR_LEN];
    unsigned char sta_bssid[WIFINET_ADDR_LEN];
    unsigned short sta_listen_intval;
    unsigned char sta_fetch_pkt_method;
    unsigned short sta_capinfo;
    unsigned char ip_acquired;
    unsigned char sta_ap_ip[IPV4_LEN];
    unsigned char sta_ap_ipv6[IPV6_LEN];
    unsigned char sta_arp_flag;

    enum wifi_mac_bwc_width sta_chbw;
    enum sta_connect_status connect_status;

    struct wifi_mac_rateset sta_rates;
    struct wifi_mac_rateset sta_htrates;
    struct wifi_mac_rateset sta_vhtrates;

    unsigned char sta_authmode;
    unsigned short sta_flags;
    unsigned short sta_flags_ext;

    unsigned short sta_associd;
    unsigned char sta_txpower;
    unsigned short sta_vlan;
    unsigned int *sta_challenge;
    unsigned char *sta_wpa_ie;
    unsigned char *sta_wme_ie;
    unsigned char *sta_wps_ie;
    unsigned short sta_txseqs[17];
    unsigned short sta_rxseqs[17];

    unsigned long sta_rxfragstamp;
    struct sk_buff *sta_rxfrag[3];
    int8_t sta_tmp_nsta;

    unsigned short sta_htcap;
    unsigned short sta_maxampdu;
    unsigned int sta_mpdudensity;

    unsigned char sta_update_rate_flag;
    unsigned char cur_fratype;
    unsigned long bcn_stamp;
    unsigned long sta_rstamp;

    union {
        unsigned char data[8];
        u_int64_t tsf;
    } sta_tstamp;

    unsigned char sta_erp;
    unsigned short sta_timoff;

    struct sk_buff_head sta_pstxqueue;
    short sta_inact;
    short sta_inact_reload;
    unsigned long sta_inact_time;

    struct wifi_sta_statistic sta_stats;
    unsigned char sta_uapsd;
    unsigned char sta_uapsd_maxsp;
    unsigned short sta_uapsd_trigseq[WME_NUM_AC];
    unsigned char sta_uapsd_ac_trigena[WME_NUM_AC];
    unsigned char sta_uapsd_ac_delivena[WME_NUM_AC];

    unsigned char *sta_rsn_ie;
    struct wifi_mac_Rsnparms sta_rsn;
    struct wifi_mac_key sta_ucastkey;
    struct wifi_mac_key pmf_key;
    unsigned char sta_encrypt_flag;
    unsigned short sa_query_seq;
    unsigned short sa_query_try_count;

    int32_t sta_avg_bcn_rssi;    // dbm
    int32_t sta_avg_rssi;
    int32_t sta_avg_snr;
    unsigned int sta_last_txrate;  //kbps
    unsigned int sta_last_rxrate;  //kbps
    unsigned int sta_last_rx_vendor_rate;
    unsigned int sta_last_rx_bw;

    unsigned char sta_maxrate_vht; /* b0-b3: mcs idx; b4-b7: # streams */
    unsigned int sta_vhtcap;        /* VHT capability */
    unsigned short vht_op_ch_freq_seg2;
    unsigned short sta_tx_vhtrates;   /* Negotiated Tx VHT rates */
    unsigned short sta_tx_max_rate;   /* Max VHT Tx Data rate */
    unsigned short sta_rx_vhtrates;   /* Negotiated Rx VHT rates */
    unsigned short sta_rx_max_rate;   /* Max VHT Rx Data rate */
    unsigned short sta_vht_basic_mcs; /* Basic VHT MCS map */
    unsigned char sta_streams;            /* number of streams supported */
    unsigned char sta_lmax_tx_pwr_cnt;
    unsigned char sta_lmax_tx_pwr_unt_intrp;
    unsigned char sta_lmax_tx_pwr_20M;
    unsigned char sta_lmax_tx_pwr_40M;
    unsigned char sta_lmax_tx_pwr_80M;
    unsigned char sta_lmax_tx_pwr_80P80M;

    //ch sw element
    unsigned char sta_new_country_sub_ie[SUB_IE_MAX_LEN];
    unsigned char sta_wide_bw_ch_sw_sub_ie[SUB_IE_MAX_LEN];
    unsigned char sta_new_vht_tx_pw_sub_ie[SUB_IE_MAX_LEN];
    unsigned char sta_channel_switch_mode;
    int sta_chan_switch_chan;
    struct os_timer_ext csa_timer;

    //ext bss ld element
    unsigned short sta_mu_mimo_sta_cnt;
    unsigned char sta_ss_ult;
    unsigned char sta_obs_scnd_20mhz;
    unsigned char sta_obs_scnd_40mhz;
    unsigned char sta_obs_scnd_80mhz;

    //quiet channel element
    unsigned char sta_ap_quiet_md;
    unsigned char sta_quiet_cnt;
    unsigned char sta_quiet_prd;
    unsigned char sta_quiet_drt;
    unsigned char sta_quiet_offset;

    //operation mode notify element
    unsigned char sta_opt_mode;

#ifdef CONFIG_WAPI
    unsigned char *sta_wai_ie;
#endif /* CONFIG_WAPI */

#ifdef CONFIG_P2P
    unsigned char *sta_p2p_ie[MAX_P2PIE_NUM];
    unsigned char sta_p2p_dev_addr[ETH_ALEN];

    unsigned char sta_p2p_dev_cap;
    unsigned short sta_p2p_config_methods;
    #ifdef CONFIG_WFD
        unsigned char *sta_wfd_ie;
    #endif/* CONFIG_WFD */
#endif /* CONFIG_P2P */
    unsigned char minstrel_init_flag;
    struct ieee80211_sta_rates sta_ieee_rates;
    struct minstrel_ht_sta_priv *sta_minstrel_ht_priv;
    struct minstrel_priv *sta_minstel_pri;
    struct minstrel_sta_info *sta_minstrel_info;
    struct ieee80211_sta_aml ieee_sta;
    unsigned char sta_vendor_bw;
    unsigned char sta_vendor_rate_code;
    unsigned char is_disconnecting;
};

#define WDS_AGING_TIME 600
#define WDS_AGING_COUNT 2

struct wifi_mac_WdsAddr
{
    struct list_head wds_hash;
    unsigned char wds_macaddr[WIFINET_ADDR_LEN];
    struct wifi_station *wds_ni;
    unsigned short wds_agingcount;
};

struct wifi_station_tbl
{
    struct wifi_mac *nt_wmac;
    rwlock_t nt_nstalock;
    unsigned long nt_nsta_lock_flags;
    struct list_head nt_nsta;
    struct list_head nt_wds_hash[WIFINET_NODE_HASHSIZE];
    const char *nt_name;
    int nt_inact_init;
};
#define WIFINET_NODE_AID(sta)   WIFINET_AID(sta->sta_associd)

#define WIFINET_NODE_STAT(sta,stat) (sta->sta_stats.ns_##stat++)
#define WIFINET_NODE_STAT_ADD(sta,stat,v)   (sta->sta_stats.ns_##stat += v)
#define WIFINET_NODE_STAT_SET(sta,stat,v)   (sta->sta_stats.ns_##stat = v)

#define WME_UAPSD_NODE_AC_CAN_TRIGGER(_ac, _sta)  \
        (((_sta)->sta_flags & WIFINET_NODE_UAPSD_TRIG) && ((_sta)->sta_uapsd_ac_delivena[(_ac)]))
#define WME_UAPSD_NODE_ALL_AC_CAN_TRIGGER(_sta) (WME_STA_UAPSD_ALL_AC_ENABLED((_sta)->sta_uapsd))
#define WME_UAPSD_NODE_MAXQDEPTH    8
#define WME_UAPSD_NODE_TRIGSEQINIT(_sta)    (memset(&(_sta)->sta_uapsd_trigseq[0], 0xff, sizeof((_sta)->sta_uapsd_trigseq)))


#define WIFINET_SAVEQ_INIT(_pstxqueue, _name) do {      \
                skb_queue_head_init(_pstxqueue);            \
        } while (0)
#define WIFINET_SAVEQ_DESTROY(_pstxqueue)
#define WIFINET_SAVEQ_QLEN(_pstxqueue)  skb_queue_len(_pstxqueue)
#define WIFINET_SAVEQ_LOCK(_pstxqueue) do {         \
                unsigned long __sqlockflags;                \
                OS_SPIN_LOCK_IRQ(&(_pstxqueue)->lock, __sqlockflags);
#define WIFINET_SAVEQ_UNLOCK(_pstxqueue)            \
        OS_SPIN_UNLOCK_IRQ(&(_pstxqueue)->lock, __sqlockflags);\
        } while (0)
#define WIFINET_SAVEQ_DEQUEUE(_pstxqueue, _skb, _qlen) do { \
                _skb = __skb_dequeue(_pstxqueue);       \
                (_qlen) = skb_queue_len(_pstxqueue);        \
        } while (0)
#define WIFINET_SAVEQ_ENQUEUE(_pstxqueue, _skb) do {\
                struct sk_buff *tail = skb_peek_tail(_pstxqueue);\
                if (tail != NULL) {                 \
                        __skb_queue_after(_pstxqueue, tail, _skb);          \
                } else {                        \
                        __skb_queue_head(_pstxqueue, _skb); \
                }                           \
        } while (0)
#define WIFINET_SAVEQ_ENQUEUE_AGE(_pstxqueue, _skb, _age) do {\
                struct sk_buff *tail = skb_peek_tail(_pstxqueue);\
                if (tail != NULL) {                 \
                        __skb_queue_after(_pstxqueue, tail, _skb);          \
                } else {                        \
                        __skb_queue_head(_pstxqueue, _skb); \
                }                           \
                M_AGE_SET(_skb, _age);                  \
        } while (0)

#define WIFINET_SAVEQ_GET_TAIL(_pstxqueue) skb_peek_tail(_pstxqueue)

#define vm_StaSetAid(_wnet_vif, _aid) ((_wnet_vif)->vm_aid_bitmap[WIFINET_AID(_aid) / 32] |= (1 << (WIFINET_AID(_aid) % 32)))
#define vm_StaClearAid(_wnet_vif, _aid) ((_wnet_vif)->vm_aid_bitmap[WIFINET_AID(_aid) / 32] &= ~(1 << (WIFINET_AID(_aid) % 32)))
#define vm_StaIsSetAid(_wnet_vif, _aid) ((_wnet_vif)->vm_aid_bitmap[WIFINET_AID(_aid) / 32] & (1 << (WIFINET_AID(_aid) % 32)))

void wifi_mac_sta_attach(struct wifi_mac *);
void wifi_mac_StationDetach(struct wifi_mac *);
void wifi_mac_sta_vattach(struct wlan_net_vif *);
void wifi_mac_station_init(struct wlan_net_vif *);
void wifi_mac_sta_vdetach(struct wlan_net_vif *);
void wifi_mac_sta_auth(struct wifi_station *);
void wifi_mac_StationUnauthorize(struct wifi_station *);
void wifi_mac_create_wifi(struct wlan_net_vif*, struct wifi_channel *);
void wifi_mac_rst_bss(struct wlan_net_vif *);
void wifi_mac_rst_main_sta(struct wlan_net_vif *wnet_vif);
int wifi_mac_connect(struct wlan_net_vif *, struct wifi_scan_info *);
void wifi_mac_sta_leave(struct wifi_station *, int reassoc);
struct wifi_station *wifi_mac_get_sta_node(struct wifi_station_tbl *, struct wlan_net_vif *, const unsigned char *);
struct wifi_station *wifi_mac_get_new_sta_node(struct wifi_station_tbl *nt, struct wlan_net_vif *wnet_vif, const unsigned char *macaddr);
struct wifi_station *wifi_mac_tmp_nsta(struct wlan_net_vif *, const unsigned char *);
struct wifi_station *wifi_mac_bup_bss(struct wlan_net_vif *, const unsigned char *);
void wifi_mac_free_sta(struct wifi_station *sta);
void wifi_mac_free_sta_from_list(struct wifi_station *sta);
int wifi_mac_rm_sta(struct wifi_station_tbl *nt, const unsigned char *macaddr);
struct wifi_station *wifi_mac_get_sta(struct wifi_station_tbl *, const unsigned char *,int);
struct wifi_station * wifi_mac_find_rx_sta(struct wifi_mac *, const struct wifi_mac_frame_min *,int);
struct wifi_station *wifi_mac_find_tx_sta(struct wlan_net_vif *, const unsigned char *);
int wifi_mac_add_wds_addr(struct wifi_station_tbl *, struct wifi_station *, const unsigned char *);
int wifi_mac_rm_sta_from_wds_by_addr(struct wifi_station_tbl *, const unsigned char *);
int wifi_mac_rm_wds_addr_list(struct wifi_station_tbl *, const unsigned char *);
struct wifi_station *wifi_mac_find_wds_sta(struct wifi_station_tbl *, const unsigned char *);
int wifi_mac_rm_sta_from_wds_by_sta(struct wifi_station_tbl *, struct wifi_station *);
void wifi_mac_func_to_task_cb(SYS_TYPE param1, SYS_TYPE param2, SYS_TYPE param3, SYS_TYPE param4, SYS_TYPE param5);

typedef void wifi_mac_IterFunc(void *, struct wifi_station *);
void wifi_mac_func_to_task(struct wifi_station_tbl *, wifi_mac_IterFunc *, void *,unsigned char btask);
void wifi_mac_disassoc_all_sta(struct wlan_net_vif*, wifi_mac_IterFunc*, void*);
void wifi_mac_dump_sta(struct wifi_station_tbl *, struct wifi_station *);
struct wifi_station *wifi_mac_fake_adhos_sta(struct wlan_net_vif *wnet_vif, const unsigned char *macaddr);
struct wifi_mac_scan_param;
struct wifi_station *wifi_mac_add_neighbor(struct wlan_net_vif *, const struct wifi_frame *, const struct wifi_mac_scan_param *);

void wifi_mac_sta_connect(struct wifi_station *, int);
void wifi_mac_sta_disconnect(struct wifi_station *);
void wifi_mac_sta_disconnect_from_ap(struct wifi_station *);
void wifi_mac_sta_deauth(void *arg, struct wifi_station *sta);
void wifi_mac_sta_disassoc(void *arg, struct wifi_station *sta);
void wifi_mac_sta_disassoc_in_task(struct wlan_net_vif *wnet_vif);
struct wifi_station *wifi_mac_find_mgmt_tx_sta(struct wlan_net_vif *wnet_vif, const unsigned char *mac);

void wifi_mac_list_sta( struct wlan_net_vif *wnet_vif);
void wifi_mac_sta_keep_alive(struct wlan_net_vif *wnet_vif, int enable, int period);
void wifi_mac_sta_keep_alive_ex (SYS_TYPE param1, SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
void wifi_mac_set_arp_agent(struct wlan_net_vif *wnet_vif, int enable);
int wifi_mac_sta_arp_agent_ex (SYS_TYPE param1, SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);

#define WIFINET_LOCK_INIT(_ic, _name) spin_lock_init(&(_ic)->wm_comlock)
#define WIFINET_LOCK_DESTROY(_ic)
#define WIFINET_LOCK(_ic) do { unsigned long __ilockflags; OS_SPIN_LOCK_IRQ(&(_ic)->wm_comlock, __ilockflags);
#define WIFINET_UNLOCK(_ic) OS_SPIN_UNLOCK_IRQ(&(_ic)->wm_comlock, __ilockflags); } while (0)

#define WIFINET_QLOCK_INIT(_ic, _name) spin_lock_init(&(_ic)->wm_queuelock)
#define WIFINET_QLOCK(_ic) OS_SPIN_LOCK_IRQ(&(_ic)->wm_queuelock, (_ic)->wm_queuelock_flags);
#define WIFINET_QUNLOCK(_ic) OS_SPIN_UNLOCK_IRQ(&(_ic)->wm_queuelock,  (_ic)->wm_queuelock_flags);

#define WIFINET_BEACONLOCK_INIT(_ic, _name) spin_lock_init(&(_ic)->wm_scanlock)
#define WIFINET_BEACONLOCK(_ic) OS_SPIN_LOCK_IRQ(&(_ic)->wm_scanlock, (_ic)->wm_scanlock_flags);
#define WIFINET_BEACONUNLOCK(_ic) OS_SPIN_UNLOCK_IRQ(&(_ic)->wm_scanlock,  (_ic)->wm_scanlock_flags);

#define WIFINET_BEACONBUFLOCK_INIT(_ic, _name) mutex_init(&(_ic)->wm_bcnbuflock)
#define WIFINET_BEACONBUF_LOCK(_ic) OS_MUTEX_LOCK(&(_ic)->wm_bcnbuflock);
#define WIFINET_BEACONBUF_UNLOCK(_ic) OS_MUTEX_UNLOCK(&(_ic)->wm_bcnbuflock);

#define WIFINET_PSLOCK_INIT(_ic, _name) spin_lock_init(&(_ic)->wm_pslock)
#define WIFINET_PSLOCK(_ic) OS_SPIN_LOCK_IRQ(&(_ic)->wm_pslock, (_ic)->wm_pslock_flags);
#define WIFINET_PSUNLOCK(_ic) OS_SPIN_UNLOCK_IRQ(&(_ic)->wm_pslock,  (_ic)->wm_pslock_flags);

#define WIFINET_FW_STAT_LOCK_INIT(_ic, _name) spin_lock_init(&(_ic)->fw_stat_lock)
#define WIFINET_FW_STAT_LOCK_DESTROY(_ic, _name)
#define WIFINET_FW_STAT_LOCK(_ic) { OS_SPIN_LOCK(&(_ic)->fw_stat_lock); }
#define WIFINET_FW_STAT_UNLOCK(_ic) { OS_SPIN_UNLOCK(&(_ic)->fw_stat_lock); }

#if defined(CONFIG_SMP)
#define WIFINET_LOCK_ASSERT(_ic) KASSERT(spin_is_locked(&(_ic)->wm_comlock),("wifi_mac not locked!"))
#else
#define WIFINET_LOCK_ASSERT(_ic)
#endif

#define WIFINET_VMACS_LOCK_INIT(_ic, _name) spin_lock_init(&(_ic)->wm_wnet_vifslock)
#define WIFINET_VMACS_LOCK_DESTROY(_ic)
#define WIFINET_VMACS_LOCK(_ic) do { OS_SPIN_LOCK(&(_ic)->wm_wnet_vifslock); } while (0)
#define WIFINET_VMACS_UNLOCK(_ic) do { OS_SPIN_UNLOCK(&(_ic)->wm_wnet_vifslock); } while (0)

#if defined(CONFIG_SMP)
#define WIFINET_VMACS_LOCK_ASSERT(_ic) KASSERT(spin_is_locked(&(_ic)->wm_wnet_vifslock),("wifi_mac_com_wnet_vifs not locked!"))
#else
#define WIFINET_VMACS_LOCK_ASSERT(_ic)
#endif

#define WIFINET_BEACON_LOCK(_ic) WIFINET_BEACONLOCK(_ic)
#define WIFINET_BEACON_UNLOCK(_ic) WIFINET_BEACONUNLOCK(_ic)
#define WIFINET_BEACON_LOCK_ASSERT(_ic)
#define WIFINET_UAPSD_LOCK(_ic) WIFINET_PSLOCK(_ic)
#define WIFINET_UAPSD_UNLOCK(_ic) WIFINET_PSUNLOCK(_ic)

#define WIFINET_NODE_LOCK_INIT(_nt) rwlock_init(&(_nt)->nt_nstalock)
#define WIFINET_NODE_LOCK_DESTROY(_nt)
#define WIFINET_NODE_LOCK(_nt) OS_WRITE_LOCK_IRQ((&(_nt)->nt_nstalock), (_nt)->nt_nsta_lock_flags)
#define WIFINET_NODE_UNLOCK(_nt) OS_WRITE_UNLOCK_IRQ((&(_nt)->nt_nstalock), (_nt)->nt_nsta_lock_flags)

#define ACL_LOCK_INIT(_as, _name) spin_lock_init(&(_as)->as_lock)
#define ACL_LOCK_DESTROY(_as)
#define ACL_LOCK(_as) OS_SPIN_LOCK_BH(&(_as)->as_lock)
#define ACL_UNLOCK(_as) OS_SPIN_UNLOCK_BH(&(_as)->as_lock)

#if defined(CONFIG_SMP)
#define ACL_LOCK_ASSERT(_as) KASSERT(spin_is_locked(&(_as)->as_lock), ("ACL not locked!"))
#else
#define ACL_LOCK_ASSERT(_as)
#endif

#define M_LINK0 0x01
#define M_PWR_SAV 0x04
#define M_UAPSD 0x08
#define M_AMSDU 0x20
#define M_NULL_PWR_SAV 0x40
#define M_TKIPMICHW 0x100
#define M_CHECKSUMHW 0x200
#define M_PWR_SAV_BYPASS 0x400

 struct wifi_mac_rx_cb
{
    unsigned short keyid;
    unsigned short wnet_vif_id;
} ;

#define M_FLAG_SET(_skb, _flag) \
        (((struct wifi_skb_callback *)(_skb)->cb)->flags |= (_flag))
#define M_FLAG_CLR(_skb, _flag) \
        (((struct wifi_skb_callback *)(_skb)->cb)->flags &= ~(_flag))
#define M_FLAG_GET(_skb, _flag) \
        (((struct wifi_skb_callback *)(_skb)->cb)->flags & (_flag))
#define M_FLAG_KEEP_ONLY(_skb, _flag) \
        (((struct wifi_skb_callback *)(_skb)->cb)->flags &= (_flag))

#define M_PWR_SAV_SET(skb) M_FLAG_SET((skb), M_PWR_SAV)
#define M_PWR_SAV_GET(skb) M_FLAG_GET((skb), M_PWR_SAV)

#define M_NULL_PWR_SAV_CLR(skb) M_FLAG_CLR((skb), M_NULL_PWR_SAV)
#define M_NULL_PWR_SAV_GET(skb) M_FLAG_GET((skb), M_NULL_PWR_SAV)

#define M_AGE_SET(skb,v)    (skb->csum = v)
#define M_AGE_GET(skb)      (skb->csum)
#define M_AGE_SUB(skb,adj)  (skb->csum -= adj)


#define le16toh(_x) le16_to_cpu(_x)
#define htole16(_x) cpu_to_le16(_x)
#define le32toh(_x) le32_to_cpu(_x)
#define htole32(_x) cpu_to_le32(_x)
#define be32toh(_x) be32_to_cpu(_x)
#define htobe32(_x) cpu_to_be32(_x)

struct wifi_mac;
extern  const char *ether_sprintf(const unsigned char *);
extern  const char* ssidie_sprintf(const unsigned char *ssidie);
extern  const char* ssid_sprintf(const unsigned char *ssid, unsigned char ssid_len);

extern int my_mod_use;

#define MY_MOD_INC_USE(_m, _err) do{\
                if (my_mod_use>2) {\
                        pr_warn("%s: try_module_get failed\n",__func__); \
                        _err;\
                }\
                my_mod_use= 1;\
        }while(0);
#define MY_MOD_DEC_USE(_m)      my_mod_use=0;
#define os_msecs_to_jiffies(a) msecs_to_jiffies(a)

#ifndef module_put_and_exit
#define module_put_and_exit(code) do {  \
                MY_MOD_DEC_USE(THIS_MODULE);    \
                do_exit(code);          \
        } while (0)
#endif

#define _LITTLE_ENDIAN  1234
#define _BIG_ENDIAN 4321
#define _BYTE_ORDER _LITTLE_ENDIAN

void wifi_mac_create_adhoc_bssid(struct wlan_net_vif *wnet_vif, unsigned char *sta_bssid);
struct sk_buff *wifi_mac_get_mgmt_frm(struct wifi_mac *wifimac, unsigned int pktlen);
void wifi_mac_notify_nsta_connect(struct wifi_station *, int newassoc);
void wifi_mac_notify_nsta_disconnect(struct wifi_station *, int reassoc);
void wifi_mac_notify_scan_done(struct wlan_net_vif *);
void wifi_mac_notify_mic_fail(struct wlan_net_vif *wnet_vif, const struct wifi_frame *wh, unsigned int key_index);
int wifi_mac_is_ht_forbidden(struct wifi_scan_info *se);

struct wifi_station * os_skb_get_nsta(struct sk_buff *skb);
void os_skb_set_nsta(struct sk_buff *skb, struct wifi_station *sta);
int os_skb_get_tid(struct sk_buff *skb);
void os_skb_set_tid(struct sk_buff *skb, unsigned char tid);
void os_skb_set_amsdu(struct sk_buff *skb);
int os_skb_is_amsdu(struct sk_buff *skb);
int  os_skb_is_uapsd(struct sk_buff *skb);
void aml_skb_unlink(struct sk_buff *skb, struct sk_buff_head *list);
#endif /* _NET80211_IEEE80211_NODE_H_ */
