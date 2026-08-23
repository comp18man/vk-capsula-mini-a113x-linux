/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     driver layer main function API
 *
 *
 ****************************************************************************************
 */

#ifndef _DRIV_MAIN_H_
#define _DRIV_MAIN_H_

#include "wifi_drv_config.h"
#include "wifi_drv_if.h"
#include "wifi_mac_rx_status.h"
#include "wifi_mac.h"
#include "wifi_mac_var.h"
#include "wifi_mac_action.h"
#include <linux/list.h>

#define DRV_RXDESC_NUM 256
#define DRV_TXDESC_RATE_NUM 4
#define DRIVER_NODE(_n) ((struct aml_driver_nsta *)(_n))


#define DRV_AGGR_DELIM_SZ       4       /* delimiter size   */
#define DRV_AGGR_MINPLEN        256     /* in bytes, minimum packet length */

#define AML_MCAST_QUEUE

#define MCAST_SEND_FLAG_BEACON_DTIM     (1)
#define MCAST_SEND_FLAG_NOA_END_RETRY       (2)

#define WME_NUM_TID         8
#define WME_BA_BMP_SIZE     64
#define WME_MAX_BA          DEFAULT_BLOCKACK_BITMAPSIZE //WME_BA_BMP_SIZE
#define DRV_TID_MAX_BUFS    (2 * WME_BA_BMP_SIZE)
#define DRV_RX_TIMEOUT      80
#define NET80211_AMSDU_TIMEOUT 4
#define DRV_GET_TIDTXINFO(_sta, _tid)   (&(_sta)->tx_agg_st.tid[(_tid)])

#define DRV_RXTID_LOCK_INIT(_rxtid)     spin_lock_init(&(_rxtid)->agg_tid_slock)
#define DRV_RXTID_LOCK_DESTROY(_rxtid)
#define DRV_RXTID_LOCK(_rxtid)          OS_SPIN_LOCK(&(_rxtid)->agg_tid_slock)
#define DRV_RXTID_UNLOCK(_rxtid)        OS_SPIN_UNLOCK(&(_rxtid)->agg_tid_slock)
#define DRV_RXTID_LOCK_IRQ(_rxtid,flag)          OS_SPIN_LOCK_IRQ(&(_rxtid)->agg_tid_slock,flag)
#define DRV_RXTID_UNLOCK_IRQ(_rxtid,flag)        OS_SPIN_UNLOCK_IRQ(&(_rxtid)->agg_tid_slock,flag)
#define DRV_RXTID_LOCK_BH(_rxtid)    OS_SPIN_LOCK_BH(&(_rxtid)->agg_tid_slock)
#define DRV_RXTID_UNLOCK_BH(_rxtid)    OS_SPIN_UNLOCK_BH(&(_rxtid)->agg_tid_slock)

#define DRV_TXQUEUE_VALUE(drv_priv, i)        ((drv_priv)->drv_txqueue_map & (1<<i))

#define DRV_TX_TIMEOUT_LOCK_INIT(_sc) spin_lock_init(&(_sc)->drv_tx_timeout_lock)
#define DRV_TX_TIMEOUT_LOCK_DESTROY(_sc)
#define DRV_TX_TIMEOUT_LOCK(_sc) do {OS_SPIN_LOCK_IRQ(&(_sc)->drv_tx_timeout_lock,(_sc)->drv_tx_timeout_lock_flags);}while(0)
#define DRV_TX_TIMEOUT_UNLOCK(_sc) do{OS_SPIN_UNLOCK_IRQ(&(_sc)->drv_tx_timeout_lock,(_sc)->drv_tx_timeout_lock_flags);}while(0)

#define DRV_TXQ_LOCK_INIT(_tq) spin_lock_init(&(_tq)->txlist_lock)
#define DRV_TXQ_LOCK_DESTROY(_tq)
#define DRV_TXQ_LOCK(_tq) do { OS_SPIN_LOCK_IRQ(&(_tq)->txlist_lock, (_tq)->txlist_lockflags);} while(0)
#define DRV_TXQ_UNLOCK(_tq) do { OS_SPIN_UNLOCK_IRQ(&(_tq)->txlist_lock, (_tq)->txlist_lockflags);} while(0)

#define DRV_TX_BACKUPQ_LOCK_INIT(_tq) spin_lock_init(&(_tq)->txlist_backupq_lock)
#define DRV_TX_BACKUPQ_LOCK_DESTROY(_tq)
#define DRV_TX_BACKUPQ_LOCK(_tq) do { OS_SPIN_LOCK_IRQ(&(_tq)->txlist_backupq_lock, (_tq)->txlist_backupq_lockflags);} while(0)
#define DRV_TX_BACKUPQ_UNLOCK(_tq) do { OS_SPIN_UNLOCK_IRQ(&(_tq)->txlist_backupq_lock, (_tq)->txlist_backupq_lockflags);} while(0)

#define DRV_TX_NORMAL_BUF_LOCK_INIT(_sc) spin_lock_init(&(_sc)->drv_normal_buf_lock)
#define DRV_TX_NORMAL_BUF_DESTROY(_sc)
#define DRV_TX_NORMAL_BUF_LOCK(_sc) do {OS_SPIN_LOCK_IRQ(&(_sc)->drv_normal_buf_lock, (_sc)->drv_normal_buf_lock_flags);}while(0)
#define DRV_TX_NORMAL_BUF_UNLOCK(_sc) do{OS_SPIN_UNLOCK_IRQ(&(_sc)->drv_normal_buf_lock,(_sc)->drv_normal_buf_lock_flags);}while(0)

#define DRV_TX_QUEUE_LOCK_INIT(_sc) spin_lock_init(&(_sc)->drv_tx_queue_lock)
#define DRV_TX_QUEUE_DESTROY(_sc)
#define DRV_TX_QUEUE_LOCK(_sc) do {OS_SPIN_LOCK_IRQ(&(_sc)->drv_tx_queue_lock, (_sc)->drv_tx_queue_lock_flags);}while(0)
#define DRV_TX_QUEUE_UNLOCK(_sc) do{OS_SPIN_UNLOCK_IRQ(&(_sc)->drv_tx_queue_lock,(_sc)->drv_tx_queue_lock_flags);}while(0)

#define DRV_HRTIMER_LOCK_INIT(_tq) spin_lock_init(&(_tq)->hr_timer_lock)
#define DRV_HRTIMER_LOCK_DESTROY(_tq)
#define DRV_HRTIMER_LOCK(_tq) do { OS_SPIN_LOCK(&(_tq)->hr_timer_lock);} while(0)
#define DRV_HRTIMER_UNLOCK(_tq) do { OS_SPIN_UNLOCK(&(_tq)->hr_timer_lock);} while(0)

#define DRV_MINSTREL_LOCK_INIT(_tq) spin_lock_init(&(_tq)->minstrel_lock)
#define DRV_MINSTREL_LOCK_DESTROY(_tq)
#define DRV_MINSTREL_LOCK(_tq) do { OS_SPIN_LOCK_IRQ(&(_tq)->minstrel_lock, (_tq)->minstrel_lockflags);} while(0)
#define DRV_MINSTREL_UNLOCK(_tq) do { OS_SPIN_UNLOCK_IRQ(&(_tq)->minstrel_lock, (_tq)->minstrel_lockflags);} while(0)

/*
 * returns delimiter padding required given the packet length
 */
#define DRV_AGGR_GET_NDELIM(_len)                                   \
        (((((_len) + DRV_AGGR_DELIM_SZ) < DRV_AGGR_MINPLEN) ?           \
          (DRV_AGGR_MINPLEN - (_len) - DRV_AGGR_DELIM_SZ) : 0) >> 2)

/*
 * minimum h/w qqcnt to be sustained to maximize aggregation
 */

#define BAW_WITHIN(_start, _bawsz, _seqno)      \
        (((unsigned short)((_seqno) - (_start)) & (unsigned short)4095) <= (_bawsz))

#define PADBYTES(_len)   ((4 - ((_len) % 4)) % 4)


/*
 * return block-ack bitmap index given sequence and starting sequence
 */
#define DRV_BA_INDEX(_st, _seq) (((_seq) - (_st)) & (WIFINET_SEQ_MAX - 1))

#define DRV_SET_TX_SET_BLOCKACK_POLICY(_sc, _wh) do {              \
                if (_wh) {                                                  \
                        if (((_wh)->i_fc[1] & WIFINET_FC1_DIR_MASK) !=        \
                                        WIFINET_FC1_DIR_DSTODS)                          \
                                ((struct wifi_qos_frame *)_wh)->i_qos[0]            \
                                |= (3 << WIFINET_QOS_ACKPOLICY_S)       \
                                   & WIFINET_QOS_ACKPOLICY;             \
                        else                                                    \
                                ((struct WIFINET_S_FRAME_QOS_ADDR4 *)_wh)->i_qos[0]      \
                                |= (3 << WIFINET_QOS_ACKPOLICY_S)       \
                                   & WIFINET_QOS_ACKPOLICY;             \
                }                                                           \
        } while (0)

struct drv_tx_scoreboard
{
    unsigned char vid;
    unsigned char tid_index;      /* TID  */
    unsigned char baw_size;   /* BA window size */
    unsigned char b_work;
    unsigned char paused;
    unsigned short seq_start;  /* start sn of BA window */
    unsigned short seq_next;   /* next sn */
    unsigned short baw_sn_last;
    unsigned short baw_head;
    unsigned short baw_tail;
    struct drv_txdesc *tx_desc[DRV_TID_MAX_BUFS];
    struct list_head txds_queue;// tx_ds node
    struct list_head tid_queue_elem; //tid node
    struct sk_buff_head tid_tx_buffer_queue;
    struct aml_driver_nsta *drv_sta;
    struct drv_queue_ac *ac;
    struct os_timer_ext addba_requesttimer;
    unsigned char cleanup_inprogress;
    unsigned char addba_exchangeinprogress;
    unsigned char addba_exchangeattempts;
    unsigned char addba_exchangecomplete;
    unsigned char addba_amsdusupported;
    unsigned char addba_exchangestatuscode;
    unsigned char tid_tx_buff_sending;
    unsigned long baw_map;
} ;

struct drv_queue_ac
{
    int b_work;
    int queue_id;
    struct list_head ac_queue;
    struct list_head tid_queue; // it's the head node of "struct drv_tx_scoreboard" by struct drv_tx_scoreboard->tid_queue_elem
};

struct drv_rxdesc
{
    struct sk_buff *rx_wbuf;   /* buffer */
    unsigned long rx_time;    /* system time when received */
    struct wifi_mac_rx_status rs;
};


struct drv_rx_scoreboard
{
    struct aml_driver_nsta *drv_sta;
    unsigned short seq_next;
    unsigned short baw_size;
    int baw_head;
    int baw_tail;
    int seq_reset;
    struct os_timer_ext timer;
    struct drv_rxdesc *pRxDesc;
    spinlock_t agg_tid_slock;
    unsigned short dialogtoken;
    unsigned short statuscode;
    struct wifi_mac_ba_parameterset baparamset;
    unsigned short batimeout;
    unsigned short userstatuscode;
    int rx_addba_exchangecomplete;
};


struct drv_aggr_tx_state
{
    unsigned short maxampdu;
    unsigned short mpdudensity;
    struct drv_tx_scoreboard tid[WME_NUM_TID];
    struct drv_queue_ac ac[WME_NUM_AC];
};

struct aml_driver_nsta
{
    struct drv_private *sta_drv_priv;
    void *net_nsta;

    void *sta_rc_nsta;/* opaque ptr to rc-private struct */
    unsigned char sta_cleanup;
    unsigned char sta_powesave;
    unsigned char sta_isuapsd;

    struct list_head sta_uapsd_queue;/* U-APSD tx queue */
    int sta_uapsd_queuecnt;/* U-APSD delivery tx cnt */

    struct drv_aggr_tx_state tx_agg_st;/* A-MPDU aggregation state */
    struct drv_rx_scoreboard rx_scb[WME_NUM_TID];

#ifdef DRV_SUPPORT_TX_WITHDRAW
    struct list_head sta_txwd_uapsd_q;
    int sta_txwd_uapsd_qqcnt;
    struct list_head sta_txwd_legacyps_q;
    int sta_txwd_legacyps_qqcnt;/* legacy ps delivery queue qcnt withdraw by fw */
#endif
};

struct drv_txlist
{
    unsigned int txlist_qnum;    /* hardware q number */
    struct list_head txlist_q;       /* transmit queue ptxdesc have send to hal*/
    spinlock_t txlist_lock;    /* lock on q and link */
    unsigned long txlist_lockflags;   /* intr state when must cli */

    spinlock_t txlist_backupq_lock;
    unsigned long txlist_backupq_lockflags;

    unsigned long txlist_lock_cnt;
    unsigned long txlist_unlock_cnt;

    unsigned int txlist_qcnt;        /* all frame have been send to hal qcnt */
    unsigned int txds_pending_cnt;       /* all frame have wait to send ptxdesc to hal qcnt in tid queue */
    unsigned long txlist_lasttime;
    struct list_head txlist_acq;/* wait to send ptxdesc to ac queue,not frame queue */
    struct list_head txlist_backup;         /* backup queue, such as tx but without hal buf, so need to backup */
    int txlist_backup_qcnt;    /* backup queue qcnt */
    unsigned char noack_flag;//yishu
    struct list_head tx_tcp_queue;
    unsigned int tx_aggr_status[16];
};

struct drv_statistic
{
    u_int64_t       tx_total_bytes;
    unsigned int   tx_pkts_cnt;
    unsigned int   tx_drops_cnt;
    unsigned int   tx_normal_cnt;
    unsigned int   tx_end_normal_cnt;
    unsigned int   tx_end_ampdu_cnt;
    unsigned int   tx_ampdu_cnt;
    unsigned int   tx_ampdu_one_frame_cnt;
    unsigned int   tx_end_fail_cnt;
    unsigned int   tx_retry_cnt;
    unsigned int   tx_short_retry_cnt;
    unsigned int   rx_indicate_cnt;
    unsigned int   rx_free_skb_cnt;
    unsigned int   rx_ampdu_cnt;
    unsigned int   rx_bar_cnt;
    unsigned int   rx_nonqos_cnt;
    unsigned int   rx_dup_cnt;
    unsigned int   rx_ok_cnt;
    unsigned int   rx_bardrop_cnt;
    unsigned int   rx_skipped_cnt;
    int                rx_ack_rssi;
};
/*
 * driver config  struct
 */
struct drv_config
{
    unsigned char cfg_txchainmask;
    unsigned char cfg_rxchainmask;
    unsigned char cfg_txchainmasklegacy;
    unsigned char cfg_rxchainmasklegacy;
    unsigned char cfg_chainmask_sel; /* enable automatic tx chainmask selection */
    unsigned char cfg_modesupport;
    int cfg_ampdu_limit;
    int cfg_ampdu_subframes;
    int cfg_ampdu_oneframe;/*send one frame use ampdu mode*/
    int cfg_ampdu_livetime; /*tx ampdu live time in fw*/
    unsigned int cfg_aggr_prot;
    int cfg_countrycode;    /* country code */
    unsigned short cfg_txpowlimit;
    unsigned char cfg_txpoweplan; /*0:default 1:CE 2:FCC*/
    unsigned char cfg_ampduackpolicy;                   /* 0: noack;1:ack */
    unsigned char cfg_txaggr;/* enable 11n tx aggregation */
    unsigned char cfg_rxaggr;         /* enable 11n rx aggregation */
    unsigned char cfg_burst_ack;              /* enable 11n tx burst */
    unsigned char cfg_htsupport;              /* enable ht support*/
    unsigned char cfg_vhtsupport;              /* enable ht support*/
    unsigned char cfg_haswme;                 /* wme priority queueing support */
    unsigned short cfg_cachelsz;               /*dev dma cache len size*/
    unsigned short cfg_40Msupport;             /*40M mode support*/
    unsigned char cfg_bw_ctrl;                        /* Enable BW auto control  */
    unsigned char cfg_shortpreamble;                       /* shor preamble*/
    unsigned short cfg_txamsdu;                /*tx amsdu*/
    unsigned short cfg_eap_lowest_rate;             /* Use lowest data rate for EAP packets. kbps */
    unsigned char cfg_uapsdsupported ;/* supports UAPSD */
    unsigned char cfg_aessupport;/*aes support*/
    unsigned char cfg_tkipsupport;/*tkip support*/
    unsigned char cfg_wepsupport;/*wep support*/
    unsigned char cfg_wapisupport;/*wapi support*/
    unsigned char cfg_tkipmicsupport;/*tkip support*/
    unsigned char cfg_checksumoffload;/* tcp/ip checksum offload support*/
    unsigned char cfg_retrynum;
    unsigned char cfg_dssupport;
    unsigned char cfg_disratecontrol;/*disable ratecontrol*/
    unsigned char cfg_rtcenable;/*tkip support*/
    int cqm_rssi_thold;
    unsigned int cqm_rssi_hyst;
    unsigned char ack_policy;
    unsigned char cfg_dynamic_bw;
    unsigned char cfg_wifi_bt_coexist_support;
    unsigned char cfg_mfp;
    unsigned char cfg_eat_count_max;
    unsigned short cfg_aggr_thresh;
    unsigned short cfg_no_aggr_thresh;
    unsigned char cfg_hrtimer_interval;
    unsigned char cfg_mac_mode;
    unsigned char cfg_band;
};

/* Reset flag */
struct driver_ops
{
    /* To initialize/de-initialize */
    int         (*drv_open)(struct drv_private *);
    int         (*drv_stop)(void *);

    /* To attach/detach virtual interface */
    int         (*add_interface)(struct drv_private *, int wnet_vif_id, void * if_data, enum hal_op_mode vm_opmode, unsigned char *myaddr, unsigned int ip);
    int         (*remove_interface)(struct drv_private *, int wnet_vif_id);
    int         (*change_interface)(struct drv_private *, int wnet_vif_id, void * if_data, enum hal_op_mode vm_opmode,unsigned char *myaddr, unsigned int ip);

    /* Notification of event/state of virtual interface */
    int         (*down_interface)(struct drv_private *, int wnet_vif_id);

    /* To attach/detach per-destination info (i.e., nsta) */
    void *  (*alloc_nsta)(struct drv_private *, int wnet_vif_id, void* sta);
    void        (*free_nsta)(struct drv_private *, void *);
    void        (*cleanup_nsta)(struct drv_private *, void *);
    void        (*update_nsta_pwrsave)(struct drv_private *, void *,int);

    /* notify a newly associated nsta */
    void        (*new_assoc)(struct drv_private *, void *, int isnew, int b_uapsd);

    /* reset */
    int         (*reset)(void *);

    /* set channel */
    void        (*set_channel)(struct drv_private *, struct hal_channel *, unsigned char flag, unsigned char vid, unsigned char opmode);
    void        (*rf_channel_restore)(struct drv_private *,  unsigned short channel, int bw);

    /* scan notifications */
    void        (*scan_start)(struct drv_private *);
    void        (*scan_end)(struct drv_private *);

    /* fw recovery */
    void        (*fw_repair)(struct drv_private *);

    /* connect notifications */
    void        (*connect_start)(struct drv_private *);
    void        (*connect_end)(struct drv_private *);
    void        (*set_channel_rssi)(struct drv_private *, unsigned char rssi);
    void        (*set_tx_power_accord_rssi)(struct drv_private *, struct hal_channel *, unsigned char rssi, unsigned char power_mode);

    /* tx callbacks */
    int         (*tx_init)(struct drv_private *, int nbufs);
    int         (*tx_cleanup)(struct drv_private *);
    int         (*tx_wmm_queue_update)(struct drv_private *,unsigned char wnet_vif_id, int queue_id, int  aifs,int cwmin,int cwmax,int txoplimit);
    int         (*tx_start)(struct drv_private *,  struct sk_buff *skbbuf);
    unsigned int        (*txlist_qcnt_handle)(struct drv_private *, int);
    unsigned int        (*txlist_all_qcnt)(struct drv_private *, int);
    int         (*txlist_isfull)( struct drv_private *,int queue_id,struct sk_buff * skbbuf, void *);
    /* rx callbacks */
    int         (*rx_init)(struct drv_private *, int nbufs);
    int         (*rx_proc_frame)(struct drv_private *, void *, struct sk_buff * skbbuf, struct wifi_mac_rx_status* rs);

    /* aggregation callbacks */
    int          (*check_aggr)(struct drv_private *, void *, unsigned char tid_index);
    int          (*aggr_tid_query)(struct drv_private *, void *, unsigned char tid_index);
    int          (*check_aggr_allow_to_send)(struct drv_private *, void *, unsigned char tid_index);
    void        (*set_ampdu_params)(struct drv_private *, void *, unsigned short maxampdu, unsigned int mpdudensity);
    void        (*addba_request_setup)(struct drv_private *, void *, unsigned char tid_index,
                                       struct wifi_mac_ba_parameterset *baparamset,
                                       unsigned short *batimeout,
                                       struct wifi_mac_ba_seqctrl *basequencectrl,
                                       unsigned short buffersize);
    void        (*addba_response_setup)(struct drv_private *, void * drv_sta,
                                        unsigned char tid_index);
    int         (*addba_request_process)(struct drv_private *, void * drv_sta,
                                         unsigned char dialogtoken,
                                         struct wifi_mac_ba_parameterset *baparamset,
                                         unsigned short batimeout,
                                         struct wifi_mac_ba_seqctrl basequencectrl);
    void        (*addba_response_process)(struct drv_private *, void *,
                                          unsigned short statuscode,
                                          struct wifi_mac_ba_parameterset *baparamset,
                                          unsigned short batimeout);
    void        (*addba_clear)(struct drv_private *, void *);
    void        (*delba_process)(struct drv_private *, void * drv_sta,
                                 struct wifi_mac_delba_parameterset *delbaparamset,
                                 unsigned short reasoncode);
    unsigned short         (*addba_status)(struct drv_private *, void *, unsigned char tid_index);
    void        (*drv_txrxampdu_del)(struct drv_private *, void *, unsigned char tid_index, unsigned char initiator);
    void        (*set_addbaresponse)(struct drv_private *, void *, unsigned char tid_index, unsigned short statuscode);
    void        (*clear_addbaresponsestatus)(struct drv_private *, void *);

    /* Misc runtime configuration */
    void        (*set_slottime)(struct drv_private *, int slottime);
    void        (*set_protmode)(struct drv_private *, enum prot_mode mode);
    void        (*set_cfg_txpowlimit)(struct drv_private *, unsigned short cfg_txpowlimit);
    void        (*set_macaddr)(struct drv_private *, unsigned char wnet_vif_id, unsigned char *macaddr);

    void        (*key_delete)(struct drv_private *, unsigned char wnet_vif_id, unsigned short key_index, int staid);
    int          (*key_set)(struct drv_private *, unsigned char wnet_vif_id, unsigned short key_index, struct hal_key_val *hk,
                           const unsigned char mac[WIFINET_ADDR_LEN]);
    int          (*rekey_data_set)(struct drv_private *, unsigned char wnet_vif_id, void *rekey_data);


    /* power management */
    void        (*awake)(struct drv_private *, int wnet_vif_id);
    void        (*netsleep)(struct drv_private *, int wnet_vif_id);
    void        (*fullsleep)(struct drv_private *, int wnet_vif_id);

    int          (*set_country)(struct drv_private *, char *iso_name);
    void        (*get_current_country)(struct drv_private *, unsigned char *iso_name);
    int          (*set_quiet)(struct drv_private *, unsigned short period, unsigned short duration,
                             unsigned short nextStart, unsigned short enabled);


    /* channel width management */
    void        (*set_bwmode)(struct drv_private *, enum wifi_mac_chanbw);
    int          (*GetSecondChanBusy)(struct drv_private *);


    /* Configuration Interface */
    int         (*drv_get_config_param)(void * drv_priv, enum cip_param_id id);
    int         (*drv_set_config_param)(void * drv_priv, enum cip_param_id ID, int data);

    /* Noise floor func */
    short       (*get_noisefloor)( struct drv_private *drv_priv, unsigned short   freq,  unsigned int chan_flags);
    /* TX Power Limit */
    void        (*drv_set_txPwrLimit)( struct drv_private *drv_priv, unsigned int limit, unsigned short txpowerdb);
    /* A-MSDU frame */
    int         (*get_amsdu_supported)(struct drv_private *, void *, int);

    /* UAPSD */
    int         (*process_uapsd_trigger)( struct drv_private *drv_priv, void * nsta, unsigned char maxsp,
                                          unsigned char ac, unsigned char flush);
    unsigned int   (*uapsd_qcnt)(void * nsta);

    unsigned int   (*Low_register_behindTask)( struct drv_private *drv_priv,void *func);
    unsigned int   (*Low_callRegisteredTask)( struct drv_private *drv_priv,SYS_TYPE taskid,SYS_TYPE param1);
    unsigned int   (*Low_addDHWorkTask)( struct drv_private *drv_priv,void *func,void *func_cb,SYS_TYPE param1,SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);

    unsigned int   (*RegisterStationID)( struct drv_private *drv_priv,unsigned char wnet_vif_id,unsigned short StaAid,unsigned char *pMac, unsigned char encrypt);
    int   (*clear_staid_and_bssid)( struct drv_private *drv_priv,unsigned char wnet_vif_id,unsigned short StaAid);
    unsigned int   (*UnRegisterAllStationID)( struct drv_private *drv_priv,unsigned char wnet_vif_id);
    void        (*phy_setchannelsupport)(struct drv_private *, int);
    void (*Phy_beaconinit)(struct drv_private *, const unsigned char ,unsigned int);
    void (*Phy_SetBeaconStart)(struct drv_private *, unsigned char wnet_vif_id,unsigned short intval, unsigned char dtim_count,unsigned short bsstype);
    void (*Phy_PutBeaconBuf)(struct drv_private *,unsigned char wnet_vif_id,unsigned char *pBeacon, unsigned short len,unsigned char Rate,unsigned short Flag);
    int (*drv_hal_set_p2p_opps_cwend_enable)( struct drv_private *drv_priv, unsigned char vid, unsigned char p2p_oppps_cw);
    int (*drv_hal_set_p2p_noa_enable)( struct drv_private *drv_priv, unsigned char vid, unsigned int duration, unsigned int interval, unsigned int starttime, unsigned char count, unsigned char flag);
    unsigned long long (*drv_hal_get_tsf)( struct drv_private *drv_priv, unsigned char vid);
    int  (*drv_hal_tx_frm_pause)(struct drv_private *drv_priv, int pause);
    int (*drv_send_null_data)(struct drv_private *drv_priv, struct NullDataCmd null_data, int len);
    int (*drv_keep_alive)(struct drv_private *drv_priv, struct NullDataCmd null_data, int len, int enable, int period);
    int (*drv_set_beacon_miss)(struct drv_private *drv_priv, unsigned char wnet_vif_id, unsigned char enable, int period);
    int (*drv_set_vsdb)(struct drv_private *drv_priv, unsigned char wnet_vif_id, unsigned char enable);
    int (*drv_set_arp_agent)(struct drv_private *drv_priv, unsigned char wnet_vif_id, unsigned char enable,
        unsigned int ipv4, unsigned char *ipv6);
    int (*drv_set_pattern)(struct drv_private *drv_priv, unsigned char vid, unsigned char offset,
        unsigned char len, unsigned char id, unsigned char *mask, unsigned char *pattern);
    int (*drv_set_suspend)(struct drv_private *drv_priv, unsigned char vid, unsigned char enable,
        unsigned char mode, unsigned int filters);
    int (*drv_p2p_client_opps_cwend_may_sleep) (struct wlan_net_vif *wnet_vif);
    int (*drv_txq_backup_send) (struct drv_private *drv_priv, struct drv_txlist *txlist);

    void (*dut_rx_start)(void);
    void (*dut_rx_stop)(void);
    void (*tx_show)(void);
    void (*tx_show_clear)(void);
    void (*phy_stc)(void);
    int (*get_snr)(void);
    unsigned int(* cca_busy_check)(void);

    void (*drv_get_sts)(struct drv_private *drv_priv, unsigned int op_code, unsigned int ctrl_code);
    void (*drv_flush_txdata)(struct drv_private *drv_priv, unsigned char vid);
    void (*drv_set_pkt_drop)(struct drv_private *drv_priv, unsigned char vid, unsigned char enable);
    void (*drv_set_is_mother_channel)(struct drv_private *drv_priv, unsigned char vid, unsigned char enable);
    void (*drv_flush_normal_buffer_queue)(struct drv_private *drv_priv, unsigned char vid);
    void (*drv_free_normal_buffer_queue)(struct drv_private *drv_priv, unsigned char vid);

    unsigned short (*drv_tx_pending_pkt)(struct drv_private *drv_priv);
#ifdef DRV_SUPPORT_TX_WITHDRAW
    int (*drv_tx_withdraw_legacyps_send) (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
#endif

    void (*drv_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*drv_read_word)(unsigned int addr);

    void (*drv_bt_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*drv_bt_read_word)(unsigned int addr);

    void (*drv_pt_rx_start)(unsigned int qos);
    void (*drv_pt_rx_stop)(void);

    /* beamforming */
    void (*drv_set_bmfm_info)(struct drv_private *drv_priv, int wnet_vif_id,
        unsigned char *group_id, unsigned char * user_position, unsigned char feedback_type);

    void (*drv_interface_enable)(unsigned char enable, unsigned char vid);
    void (*drv_cfg_cali_param)(void);
    void (*drv_cfg_txpwr_cffc_param)(struct wifi_channel * ,struct tx_power_plan *);
    void (*drv_print_fwlog)(unsigned char *logbuf_ptr, int databyte);
    int (*drv_set_bssid)(struct drv_private *drv_priv, unsigned char wnet_vif_id,unsigned char *bssid);
    void (*drv_set_tx_livetime)(unsigned int tx_livetime);

} ;

struct drv_powersave
{
    unsigned int ps_hal_usecount;
    OS_LOCK ps_pwrsave_lock;
    unsigned long __ilockflags;
};

struct drv_private
{
    struct wifi_mac *wmac;
    struct wifi_mac_ops *net_ops;
    struct driver_ops drv_ops;//exported
    struct hal_private *hal_priv;

    unsigned short drv_wnet_vif_num;
    struct wlan_net_vif *drv_wnet_vif_table[DEFAULT_MAX_VMAC];

    struct list_head drv_normal_buffer_queue[2];/* transmit buffer */
    unsigned short drv_normal_buffer_count[2];
    struct list_head drv_ampdu_buffer_queue[2][4];/* transmit buffer */
    unsigned short drv_ampdu_buffer_count[2];

    spinlock_t drv_normal_buf_lock;
    unsigned long drv_normal_buf_lock_flags;
    spinlock_t drv_tx_queue_lock;
    unsigned long drv_tx_queue_lock_flags;
    spinlock_t drv_tx_timeout_lock;
    unsigned long drv_tx_timeout_lock_flags;

    unsigned int drv_txqueue_map;
    struct drv_txlist drv_txlist_table[HAL_NUM_TX_QUEUES];
    struct drv_txlist *drv_mgmtxqueue;
    struct drv_txlist *drv_uapsdqueue;
    struct drv_txlist *drv_noqosqueue;
    struct drv_txlist *drv_mcasequeue;

    struct drv_config drv_config;  /* per-instance load-time parameters */
    struct drv_statistic drv_stats;

    unsigned int drv_not_init_flag  ;/* not be initial */
    unsigned char drv_scanning      ;/*wifi_mac layer at  scanning  */
    unsigned char drv_connetting      ;/*wifi_mac layer at  drv_connetting  */
    unsigned char wait_mpdu_timeout;
    unsigned char add_wakeup_work;
    unsigned char drv_pkt_drop[2];
    unsigned char is_mother_channel[2];
    unsigned char stop_noa_flag;

    struct hal_channel drv_curchannel;
    unsigned char drv_bssid[WIFINET_ADDR_LEN];

    unsigned short drv_curtxpower;
    unsigned char drv_PhyPsState;
    unsigned char dot11ComPsState;
    unsigned char dot11VmacPsState[DEFAULT_MAX_VMAC];
    struct drv_powersave drv_ps;
    struct os_timer_ext drv_txtimer;
    struct tasklet_struct ampdu_tasklet;
    struct tasklet_struct forward_tasklet;
    struct tasklet_struct retransmit_tasklet;
    struct sk_buff_head retransmit_queue;

    enum prot_mode drv_protect_mode;    /* protection mode */
    unsigned char drv_protect_rateindex;     /* protection rate index */

    struct ratecontrol_ops *ratctrl_ops;
    int drv_ratectrl_size;
    unsigned char drv_ratectrl_mrr;/* multi-rate retry support */
    const struct drv_rate_table *drv_currratetable;   /* current rate table */
    struct aml_ratecontrol minstrel_sample_rate[DRV_TXDESC_RATE_NUM];
    spinlock_t hr_timer_lock;
    spinlock_t minstrel_lock;
    unsigned long minstrel_lockflags;
};

#ifdef CONFIG_P2P
int drv_p2p_client_opps_cwend_may_sleep (struct wlan_net_vif *wnet_vif);
int drv_p2p_go_opps_cwend_may_sleep (struct wlan_net_vif *wnet_vif);
void p2p_noa_start_irq (struct wifi_mac_p2p *p2p, struct drv_private *drv_priv);
#endif

int aml_driv_attach( struct drv_private *drv_priv, struct wifi_mac* wmac);
void aml_driv_detach(struct drv_private * );
struct drv_private* drv_get_drv_priv(void);
struct aml_hal_call_backs * get_hal_call_back_table(void);
int drv_dev_remove(void);

int drv_rate_setup(struct drv_private *drv_priv, enum wifi_mac_macmode mode);
int drv_channel_init(struct drv_private *drv_priv, unsigned int cc);
int drv_add_wnet_vif(struct drv_private *drv_priv,
    int wnet_vif_id, void * if_data, enum hal_op_mode vm_opmode, unsigned char *myaddr, unsigned int ip);
void aml_set_mac_control_register(void);

#endif /* _DRIV_MAIN_H_ */
