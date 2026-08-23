/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer  struct and define
 *
 *
 ****************************************************************************************
 */
#ifndef _NET80211_IEEE80211_H_
#define _NET80211_IEEE80211_H_

enum wifi_mac_phytype
{
    WIFINET_T_DS=0,
    WIFINET_T_OFDM,
    WIFINET_T_HT,
    WIFINET_T_VHT,
    WIFINET_T_MAX
};
#define WIFINET_T_CCK	WIFINET_T_DS
/* XXX not really a mode; there are really multiple PHY's */

/* shorthands to compact tables for readability */
#define WOFDM	WIFINET_T_OFDM
#define CCK	WIFINET_T_CCK
#define HT	WIFINET_T_HT
#define VHT_PHY	WIFINET_T_VHT

enum wifi_mac_macmode
{
    WIFINET_MODE_AUTO = 0,
    WIFINET_MODE_11B = 1,
    WIFINET_MODE_11G = 2,
    WIFINET_MODE_11BG = 3,
    WIFINET_MODE_11N = 4,
    WIFINET_MODE_11GN = 6,
    WIFINET_MODE_11BGN = 7,
    WIFINET_MODE_11AC = 8,
    WIFINET_MODE_11NAC = 0xc,
    WIFINET_MODE_11GNAC = 0xe,
    WIFINET_MODE_MAX,
};

#define WIFINET_11BGMODE(mode) (((mode)==WIFINET_MODE_11BG)||((mode)==WIFINET_MODE_11BGN))
#define WIFINET_11GMODE(mode) (((mode)==WIFINET_MODE_11G)||((mode)==WIFINET_MODE_11GN)||((mode)==WIFINET_MODE_11GNAC))

#define WIFINET_INCLUDE_11N(mode) (((mode)>=WIFINET_MODE_11N))
#define WIFINET_INCLUDE_11G(mode) (WIFINET_11BGMODE(mode)||WIFINET_11GMODE(mode))


enum wifi_mac_opmode
{
    WIFINET_M_IBSS = 0,
    WIFINET_M_STA = 1,
    WIFINET_M_HOSTAP = 2,
    WIFINET_M_MONITOR = 4,
    WIFINET_M_WDS = 3,
    WIFINET_M_P2P_CLIENT = 5,
    WIFINET_M_P2P_GO = 6,
    WIFINET_M_P2P_DEV = 7,
};


enum wifi_mac_11nmode
{
    WIFINET_MIXN    =  0,
    WIFINET_PUREN       =  1
};

enum wifi_mac_bwc_mode
{
    WIFINET_BWC_MODE20,
    WIFINET_BWC_MODE2040,
    WIFINET_BWC_MODE40,
    WIFINET_BWC_MODE80,
    WIFINET_BWC_MODE80P80,
    WIFINET_BWC_MODE160,
    WIFINET_BWC_MODEMAX

};
enum wifi_mac_bwc_width
{
    WIFINET_BWC_WIDTH20,
    WIFINET_BWC_WIDTH40,
    WIFINET_BWC_WIDTH80,
    WIFINET_BWC_WIDTH160,
    WIFINET_BWC_WIDTH80P80,
    WIFINET_BWC_UNSET,
};

enum sta_connect_status
{
    CONNECT_IDLE,
    CONNECT_FOUR_WAY_HANDSHAKE_COMPLETED,
    CONNECT_DHCP_GET_ACK,
};

enum
{
    VHT_OPT_CHN_WD_2040M = 0,
    VHT_OPT_CHN_WD_80M = 1,
    VHT_OPT_CHN_WD_160M = 2,
    VHT_OPT_CHN_WD_80P80M =3,
};

enum wifi_mac_band
{
    WIFINET_BAND_2G,
    WIFINET_BAND_5G,
};
enum wifi_mac_bwc_state
{
    NET80211_BWC_STATE_CLEAR,
    NET80211_BWC_STATE_BUSY,
    NET80211_BWC_STATE_STOP,
    NET80211_BWC_STATE_MAX
};

enum wifi_mac_fixed_rate_mode
{
    WIFINET_FIXED_RATE_NONE  = 0,
    WIFINET_FIXED_RATE_MCS   = 1
};


struct wifi_mac_fixed_rate
{
    enum wifi_mac_fixed_rate_mode    mode;
    unsigned int         rateinfo;
    unsigned int         retrynum;
};

enum wifi_mac_protmode
{
    WIFINET_PROT_NONE   = 0,
    WIFINET_PROT_CTSONLY    = 1,
    WIFINET_PROT_RTSCTS = 2,
};


enum wifi_mac_authmode
{
    WIFINET_AUTH_OPEN   = 0,
    WIFINET_AUTH_SHARED = 1,
    WIFINET_AUTH_FT     = 2,
    WIFINET_AUTH_SAE    = 3,
};

enum wifi_mac_roamingmode
{
    WIFINET_ROAMING_DISABLE = 0,
    WIFINET_ROAMING_BASIC   = 1,
    WIFINET_ROAMING_FAST    = 2,
};

#define DEFAULT_ROAMING_THRESHOLD_2G -75
#define DEFAULT_ROAMING_THRESHOLD_5G -78
#define ROAMING_TRIGER_COUNT         10


enum wifi_mac_tx_status_mode
{
    WIFINET_TX_STATUS_FAIL = 0,
    WIFINET_TX_STATUS_SUCC = 1,
    WIFINET_TX_STATUS_NOSET = 2,
};

enum wifi_mac_ht_prot_update_flag
{
    WIFINET_STA_DISCONNECT = 0,
    WIFINET_STA_CONNECT    = 1,
    WIFINET_BEACON_UPDATE  = 2,
};

struct wifi_channel
{
    unsigned short chan_cfreq1;		/* center frequency1 for 20/40/80/160, in Mhz */
    //unsigned short     chan_cfreq2;   /* center frequency2 for 80P80, in Mhz */
    unsigned short chan_flags;
    unsigned short chan_pri_num;	/* primary channel number */
    int8_t chan_maxpower;		/* maximum tx power in dBm */
    int8_t chan_minpower;		/* minimum tx power in dBm */
    enum wifi_mac_bwc_width chan_bw;	/* bandwidth */
    unsigned char global_operating_class;
};

struct wifi_candidate_channel
{
    struct wifi_channel *channel;
    int avg_rssi;
};

struct class_chan_set
{
    int opt_idx;
    int bw;
    int sub_num;
    struct wifi_channel chan_sub_set[32];
};


struct county_global_map
{
    char operating_class;
    char g_operating_class;
};

#define MAX_CLASS_NUM 	(32)
struct country_set
{
    int country;
    struct county_global_map opt_idx_map[MAX_CLASS_NUM];
};

#define MAX_NA_FREQ_NUM (8)
struct country_na_freq_info
{
    char g_operating_class;
    unsigned short na_freq[MAX_NA_FREQ_NUM];
};

struct country_na_freq_set
{
    int na_freq_class_num;
    struct country_na_freq_info na_freq_info[10];
};

#define MAX_P2PIE_NUM		4

#define WIFINET_CHAN_MAX	256
#define WIFINET_CHAN_MAX_NUM	165
#define WIFINET_CHAN_BYTES	32
#define WIFINET_CHAN_INVALUE	0
#define WIFINET_CHAN_ERR	NULL
#define WIFINET_MAX_SCAN_CHAN	64

#define WIFINET_CHAN_2GHZ	0x4000  /* 2.4 GHz  channel. */
#define WIFINET_CHAN_5GHZ	0x8000  /* 5 GHz  channel */
#define WIFINET_CHAN_DFS	0x2000  /* DFS */
#define WIFINET_CHAN_AWARE	0x1000  /* chan aware */

#define  WIFINET_CHAN_MODE_MASK	0xff

#define WIFINET_IS_CHAN_ERR(_c) ((_c) == WIFINET_CHAN_ERR)

#define WIFINET_IS_CHAN_2GHZ(_c) \
        ((!WIFINET_IS_CHAN_ERR(_c)) && (((_c)->chan_flags & WIFINET_CHAN_2GHZ) != 0))

#define WIFINET_IS_CHAN_5GHZ(_c) \
        ((!WIFINET_IS_CHAN_ERR(_c)) && (((_c)->chan_flags & WIFINET_CHAN_5GHZ) != 0))

#define WIFINET_IS_CHAN_11N_HT40(_c) \
        ((!WIFINET_IS_CHAN_ERR(_c)) && (((_c)->chan_bw == WIFINET_BWC_WIDTH40)))

#define WIFINET_RATE_SIZE	8
#define WIFINET_RATE_MAXSIZE	57
#define WIFINET_HT_RATE_SIZE	77
#define WIFINET_RATE_BASIC	0x80
#define WIFINET_RATE_VAL	0x7f
#define WIFINET_GET_RATE_VAL(_rate)	((_rate)&WIFINET_RATE_VAL)
#define WIFINET_RATE_MCS	0x80
#define WIFINET_RATE_VHT_MCS	0xC0
#define WIFINET_AMPDU_LIMIT_MAX	(32768) /* (16 * 1024 - 1) */

struct wifi_mac_rateset
{
    unsigned char dot11_rate_num;
    unsigned char dot11_rate[WIFINET_HT_RATE_SIZE];
};

enum wifi_mac_state
{
    WIFINET_S_INIT = 0,
    WIFINET_S_SCAN = 1,
    WIFINET_S_CONNECTING = 2,
    WIFINET_S_AUTH = 3,
    WIFINET_S_ASSOC = 4,
    WIFINET_S_CONNECTED = 5,
    WIFINET_S_MAX,
};

enum wifi_mac_main_state
{
    WIFINET_MAIN_INIT = 0,
    WIFINET_MAIN_SCAN = 1,
    WIFINET_MAIN_CONNECTING = 2,
    WIFINET_MAIN_CONNECTED = 5,
    WIFINET_MAIN_MAX,
};

enum wifi_mac_connect_state
{
    WIFINET_CON_INIT = 1,
    WIFINET_CON_AUTH = 2,
    WIFINET_CON_ASSOC = 3,
    WIFINET_CON_DONE = 4,
    WIFINET_CON_MAX,
};

enum wifi_mac_pub_act_state
{
    WIFINET_PUB_INIT = 0,
    WIFINET_DPP_AUTH_REQ,
    WIFINET_DPP_AUTH_RSP,
    WIFINET_DPP_AUTH_CNF,
    WIFINET_PUB_GAS_REQ,
    WIFINET_PUB_GAS_RSP,
    WIFINET_PUB_MAX,
};

#define PHASE_CONNECTING	1
#define PHASE_DISCONNECTING	2
#define PHASE_TX_BUFF_QUEUE	4
#define PHASE_DISCONNECT_DELAY	8

enum wifi_mac_recovery_state
{
    WIFINET_RECOVERY_INIT = 0,
    WIFINET_RECOVERY_START = 1,
    WIFINET_RECOVERY_UNDER_CONNECT = 2,
    WIFINET_RECOVERY_VIF_UP = 4,
    WIFINET_RECOVERY_END,
};

#define WIFINET_F_DOSORT	0x00000001
#define WIFINET_F_DOFRATE	0x00000002
#define WIFINET_F_DOXSECT	0x00000004
#define WIFINET_F_DOBRS		0x00000008

struct wmeParams
{
    unsigned char wmep_acm;
    unsigned char wmep_aifsn;
    unsigned char wmep_logcwmin;
    unsigned char wmep_logcwmax;
    unsigned short wmep_txopLimit;
    unsigned char wmep_noackPolicy;
};

#define WME_NUM_AC	4
#define VENDOR_IE_MAX	3

struct chanAccParams
{
    unsigned char cap_info_count;
    struct wmeParams cap_wmeParams[WME_NUM_AC];
};

struct wifi_mac_wme_state
{
    unsigned int wme_flags;
#define WME_F_AGGRESSIVE    0x00000001

    unsigned int wme_hipri_traffic;
    unsigned int wme_hipri_switch_thresh;
    unsigned int wme_hipri_switch_hysteresis;

    struct chanAccParams    wme_wmeChanParams;
    struct chanAccParams    wme_wmeBssChanParams;
    struct chanAccParams    wme_chanParams; /*this param is set to self chip*/
    struct chanAccParams    wme_bssChanParams;  /*this param is set beacon use to let sta set to it's chip*/
    unsigned char           wme_nonAggressiveMode;

    //int   (*wme_update)(struct WIFINET_MAC *,struct WIFINET_VMAC *);
};

struct wifi_mac_beacon_offsets
{
    unsigned char *bo_ssid;
    unsigned short *bo_caps;
    unsigned char *bo_rates;
    unsigned char *bo_channel;
    unsigned char *bo_tim;
    unsigned char *bo_wme;
    unsigned char *bo_tim_trailer;
    unsigned short bo_tim_len;
    unsigned short bo_tim_trailerlen;
    unsigned char *bo_chanswitch;
    unsigned char *bo_extchanswitch;
    unsigned char *bo_erp;
    unsigned char *bo_appie_buf;
    unsigned short bo_appie_buf_len;
    unsigned char *bo_wsc;
    unsigned char *bo_htinfo;
    unsigned char *bo_htcap;
    unsigned char *bo_vhtop;
    unsigned char *bo_vhtcap;
    unsigned char *bo_obss_scan;
    unsigned char *bo_extcap;
    unsigned char *bo_ch_sw_wrp;
    unsigned char *bo_vendor_ie[VENDOR_IE_MAX];
    unsigned short bo_chanswitch_trailerlen;
    unsigned short bo_extchanswitch_trailerlen;
    unsigned short bo_chswwrp_trailerlen;
    unsigned char bo_initial;
    /* beacon sequence number */
    unsigned short bo_bcn_seq;
};

#define WME_NUM_AC      4

#ifndef __packed
#define __packed   __attribute__((__packed__))
#endif

#define WIFINET_ADDR_LEN    6
#define WIFINET_IS_MULTICAST(_a)    (*(_a) & 0x01)
struct wifi_frame
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
    unsigned char  i_addr3[WIFINET_ADDR_LEN];
    unsigned char  i_seq[2];
} __packed;

struct wifi_qos_frame
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
    unsigned char  i_addr3[WIFINET_ADDR_LEN];
    unsigned char  i_seq[2];
    unsigned char  i_qos[2];
} __packed;

struct wifi_htc_frame
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
    unsigned char  i_addr3[WIFINET_ADDR_LEN];
    unsigned char  i_seq[2];
    unsigned char  i_htc[4];
} __packed;

struct wifi_qos_htc_frame
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
    unsigned char  i_addr3[WIFINET_ADDR_LEN];
    unsigned char  i_seq[2];
    unsigned char  i_qos[2];
    unsigned char  i_htc[4];
} __packed;

struct WIFINET_S_FRAME_ADDR4
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
    unsigned char  i_addr3[WIFINET_ADDR_LEN];
    unsigned char  i_seq[2];
    unsigned char  i_addr4[WIFINET_ADDR_LEN];
} __packed;

struct WIFINET_S_FRAME_QOS_ADDR4
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
    unsigned char  i_addr3[WIFINET_ADDR_LEN];
    unsigned char  i_seq[2];
    unsigned char  i_addr4[WIFINET_ADDR_LEN];
    unsigned char  i_qos[2];
} __packed;

struct WIFINET_S_FRAME_ADDR2
{
    unsigned char  i_fc[2];
    unsigned char  i_aidordur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
} __packed;

#define WIFINET_FC0_VERSION_MASK            0x03
#define WIFINET_FC0_VERSION_0               0x00
#define WIFINET_FC0_TYPE_MASK               0x0c
#define WIFINET_FC0_TYPE_MGT                0x00
#define WIFINET_FC0_TYPE_CTL                0x04
#define WIFINET_FC0_TYPE_DATA               0x08

#define WIFINET_FC0_SUBTYPE_MASK            0xf0
#define WIFINET_FC0_SUBTYPE_ASSOC_REQ       0x00
#define WIFINET_FC0_SUBTYPE_ASSOC_RESP  0x10
#define WIFINET_FC0_SUBTYPE_REASSOC_REQ 0x20
#define WIFINET_FC0_SUBTYPE_REASSOC_RESP    0x30
#define WIFINET_FC0_SUBTYPE_PROBE_REQ       0x40
#define WIFINET_FC0_SUBTYPE_PROBE_RESP  0x50
#define WIFINET_FC0_SUBTYPE_BEACON      0x80
#define WIFINET_FC0_SUBTYPE_DISASSOC        0xa0
#define WIFINET_FC0_SUBTYPE_AUTH            0xb0
#define WIFINET_FC0_SUBTYPE_DEAUTH      0xc0
#define WIFINET_FC0_SUBTYPE_ACTION      0xd0
#define WIFINET_FC0_SUBTYPE_BAR         0x80
#define WIFINET_FC0_SUBTYPE_PS_POLL     0xa0
#define WIFINET_FC0_SUBTYPE_CTS         0xc0
#define WIFINET_FC0_SUBTYPE_ACK         0xd0
#define WIFINET_FC0_SUBTYPE_CF_END      0xe0
#define WIFINET_FC0_SUBTYPE_CF_ACK      0x10
#define WIFINET_FC0_SUBTYPE_NODATA      0x40
#define WIFINET_FC0_SUBTYPE_QOS     0x80
#define WIFINET_FC0_SUBTYPE_QOS_NULL        0xc0

#define WIFINET_FC1_DIR_MASK            0x03
#define WIFINET_FC1_DIR_NODS            0x00
#define WIFINET_FC1_DIR_TODS            0x01
#define WIFINET_FC1_DIR_FROMDS      0x02
#define WIFINET_FC1_DIR_DSTODS      0x03

#define WIFINET_FC1_MORE_FRAG           0x04
#define WIFINET_FC1_RETRY           0x08
#define WIFINET_FC1_PWR_MGT         0x10
#define WIFINET_FC1_MORE_DATA           0x20
#define WIFINET_FC1_WEP         0x40
#define WIFINET_FC1_ORDER           0x80

#define WIFINET_SEQ_FRAG_MASK           0x000f
#define WIFINET_SEQ_FRAG_SHIFT      0
#define WIFINET_SEQ_SEQ_SHIFT           4
#define WIFINET_SEQ_MAX               4096

#define WIFINET_NWID_LEN            32

#define WIFINET_QOS_AMSDU                     0x80
#define WIFINET_QOS_AMSDU_S                   7
#define WIFINET_QOS_ACKPOLICY           0x60
#define WIFINET_QOS_ACKPOLICY_S     5
#define WIFINET_QOS_EOSP            0x10
#define WIFINET_QOS_TID         0x0f

#define WLAN_FC_PVER 0x0003
#define WLAN_FC_TODS 0x0100
#define WLAN_FC_FROMDS 0x0200
#define WLAN_FC_MOREFRAG 0x0400
#define WLAN_FC_RETRY 0x0800
#define WLAN_FC_PWRMGT 0x1000
#define WLAN_FC_MOREDATA 0x2000
#define WLAN_FC_ISWEP 0x4000
#define WLAN_FC_ORDER 0x8000

#define WLAN_FC_TYPE_DATA 0x0008
#define WLAN_FC_TYPE_MGMT 0x0000

#define WLAN_FC_STYPE_QOS_DATA 0x0080

#define WLAN_FC_GET_TYPE(fc) ((fc) & WIFINET_FC0_TYPE_MASK)
#define WLAN_FC_GET_STYPE(fc) ((fc) & WIFINET_FC0_SUBTYPE_MASK)
#define WLAN_GET_SEQ_FRAG(seq) ((seq) & 0x000F)
#define WLAN_GET_SEQ_SEQ(seq)  ((seq) & 0xFFF0)

#define WIFINET_IS_CRTL(_frame) (((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_CTL)

#define WIFINET_IS_PSPOLL(_frame) (WIFINET_IS_CRTL(_frame) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_PS_POLL))

#define WIFINET_IS_BAR(_frame) (WIFINET_IS_CRTL(_frame) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_BAR))

#define WIFINET_IS_MGM(_frame) ((((struct wifi_frame *)(_frame))->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT )

#define WIFINET_IS_BEACON(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_BEACON))

#define WIFINET_IS_ACTION(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_ACTION))

#define WIFINET_IS_PROBERSP(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_PROBE_RESP))

#define WIFINET_IS_PROBEREQ(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_PROBE_REQ))

#define WIFINET_IS_DISASSOC(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_DISASSOC))

#define WIFINET_IS_AUTH(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_AUTH))

#define WIFINET_IS_ASSOC_REQ(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_ASSOC_REQ))

#define WIFINET_IS_DEAUTH(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_DEAUTH))

#define WIFINET_IS_DATA(_wh) ((((struct wifi_frame *)(_wh))->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_DATA )

#define WIFINET_IS_MFP_FRAME(_frame) ((((_frame)->i_fc[0] & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_MGT) && \
    ((_frame)->i_fc[1] & WIFINET_FC1_WEP) && ((((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_DEAUTH) || \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_DISASSOC) || \
    (((_frame)->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_ACTION)))

#define WIFINET_IS_CLASS3(_wh) (WIFINET_IS_DATA(_wh) || WIFINET_IS_ACTION(_wh))

#define WIFINET_RX_MCS_SINGLE_STREAM_BYTE_OFFSET 0
#define WIFINET_RX_MCS_DUAL_STREAM_BYTE_OFFSET 1
#define WIFINET_RX_MCS_ALL_NSTREAM_RATES 0xff
#define WIFINET_TX_MCS_OFFSET 12

#define WIFINET_TX_MCS_SET_DEFINED 0x80
#define WIFINET_TX_RX_MCS_SET_NOT_EQUAL 0x40
#define WIFINET_TX_2_SPATIAL_STREAMS 0x10
#define WIFINET_TX_3_SPATIAL_STREAMS 0x20
#define WIFINET_TX_4_SPATIAL_STREAMS 0x30

#define WIFINET_TX_MCS_SET 0xf8

enum wifi_mac_reason_code {
    WIFINET_REASON_UNSPECIFIED = 1,
    WIFINET_REASON_AUTH_EXPIRE = 2,
    WIFINET_REASON_AUTH_LEAVE = 3,
    WIFINET_REASON_ASSOC_EXPIRE = 4,
    WIFINET_REASON_ASSOC_TOOMANY = 5,
    WIFINET_REASON_NOT_AUTHED = 6,
    WIFINET_REASON_NOT_ASSOCED = 7,
    WIFINET_REASON_ASSOC_LEAVE = 8,
    WIFINET_REASON_ASSOC_NOT_AUTHED = 9,
    WIFINET_REASON_RSN_REQUIRED = 11,
    WIFINET_REASON_RSN_INCONSISTENT = 12,
    WIFINET_REASON_IE_INVALID = 13,
    WIFINET_REASON_MIC_FAILURE = 14,
};

enum wifi_mac_status_code {
    WIFINET_STATUS_SUCCESS = 0,
    WIFINET_STATUS_UNSPECIFIED = 1,
    WIFINET_STATUS_CAPINFO = 10,
    WIFINET_STATUS_NOT_ASSOCED = 11,
    WIFINET_STATUS_OTHER = 12,
    WIFINET_STATUS_ALG = 13,
    WIFINET_STATUS_SEQUENCE = 14,
    WIFINET_STATUS_CHALLENGE = 15,
    WIFINET_STATUS_TIMEOUT = 16,
    WIFINET_STATUS_TOOMANY = 17,
    WIFINET_STATUS_BASIC_RATE = 18,
    WIFINET_STATUS_SP_REQUIRED = 19,
    WIFINET_STATUS_PBCC_REQUIRED = 20,
    WIFINET_STATUS_CA_REQUIRED = 21,
    WIFINET_STATUS_TOO_MANY_STATIONS = 22,
    WIFINET_STATUS_RATES = 23,
    WIFINET_STATUS_SHORTSLOT_REQUIRED = 25,
    WIFINET_STATUS_DSSSOFDM_REQUIRED = 26,
    WIFINET_STATUS_REFUSED_TEMPORARILY = 30,
    WIFINET_STATUS_REFUSED = 37,
    WIFINET_STATUS_INVALID_PARAM = 38,
    WIFINET_STATUS_INVALID_PMKID = 53,

};

#define WIFINET_COUNTRY_MAX_TRIPLETS (83)
struct wifi_mac_ie_country
{
    unsigned char  country_id;
    unsigned char  country_len;
    unsigned char  country_str[3];
    unsigned char  country_triplet[WIFINET_COUNTRY_MAX_TRIPLETS*3];
} __packed;

#define WIFINET_QOS_HAS_SEQ(wh) \
        (((wh)->i_fc[0] & \
          (WIFINET_FC0_TYPE_MASK | WIFINET_FC0_SUBTYPE_QOS)) == \
         (WIFINET_FC0_TYPE_DATA | WIFINET_FC0_SUBTYPE_QOS))


#define WIFINET_QOS_HAS_HTC(wh) \
        (((wh)->i_fc[1] & WIFINET_FC1_ORDER) == WIFINET_FC1_ORDER)

#define WME_QOSINFO_COUNT   0x0f
struct wifi_mac_ie_wme
{
    unsigned char  wme_id;
    unsigned char  wme_len;
    unsigned char  wme_oui[3];
    unsigned char  wme_type;
    unsigned char  wme_subtype;
    unsigned char  wme_version;
    unsigned char  wme_info;
} __packed;

struct wifi_mac_wme_acparams
{
    unsigned char  acp_aci_aifsn;
    unsigned char  acp_logcwminmax;
    unsigned short acp_txop;
} __packed;


#define WME_AC_BE   0
#define WME_AC_BK   1
#define WME_AC_VI   2
#define WME_AC_VO   3
#define WME_AC_NUM  4

#define WIFINET_WME_INFO_LEN    7

#define WME_PARAM_ACI       0x60
#define WME_PARAM_ACI_S     5
#define WME_PARAM_ACM       0x10
#define WME_PARAM_ACM_S     4
#define WME_PARAM_AIFSN     0x0f
#define WME_PARAM_AIFSN_S   0
#define WME_PARAM_LOGCWMIN  0x0f
#define WME_PARAM_LOGCWMIN_S    0
#define WME_PARAM_LOGCWMAX  0xf0
#define WME_PARAM_LOGCWMAX_S    4
#define WME_PARAM_ACI_GET(aciafsn)      (((aciafsn)&WME_PARAM_ACI)>>WME_PARAM_ACI_S)
#define WME_PARAM_ACM_GET(aciafsn)      (((aciafsn)&WME_PARAM_ACM)>>WME_PARAM_ACM_S)
#define WME_PARAM_AIFSN_GET(aciafsn)        (((aciafsn)&WME_PARAM_AIFSN)>>WME_PARAM_AIFSN_S)
#define WME_PARAM_LOGCWMIN_GET(ecw) (((ecw)&WME_PARAM_LOGCWMIN)>>WME_PARAM_LOGCWMIN_S)
#define WME_PARAM_LOGCWMAX_GET(ecw) (((ecw)&WME_PARAM_LOGCWMAX)>>WME_PARAM_LOGCWMAX_S)

#define WME_AC_TO_TID(_ac) (       \
                                   ((_ac) == WME_AC_VO) ? 6 : \
                                   ((_ac) == WME_AC_VI) ? 5 : \
                                   ((_ac) == WME_AC_BK) ? 1 : \
                                   0)

#define TID_TO_WME_AC(_tid) (      \
                                   (((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE : \
                                   (((_tid) == 1) || ((_tid) == 2)) ? WME_AC_BK : \
                                   (((_tid) == 4) || ((_tid) == 5)) ? WME_AC_VI : \
                                   WME_AC_VO)
struct wifi_mac_wme_param
{
    unsigned char  param_id;
    unsigned char  param_len;
    unsigned char  param_oui[3];
    unsigned char  param_oui_type;
    unsigned char  param_oui_sybtype;
    unsigned char  param_version;
    unsigned char  param_qosInfo;
    unsigned char  param_reserved;
    struct wifi_mac_wme_acparams params_acParams[WME_NUM_AC];
} __packed;

enum ts_dir_idx {
    TS_DIR_IDX_UPLINK,
    TS_DIR_IDX_DOWNLINK,
    TS_DIR_IDX_BIDI,

    TS_DIR_IDX_COUNT
};


#define WMM_OUI_TYPE 2
#define WMM_VERSION 1
#define WMM_OUI_SUBTYPE_TSPEC_ELEMENT 2
#define WMM_AC_ACCESS_POLICY_EDCA  1
#define WMM_AC_FIXED_MSDU_SIZE  BIT(15)

#define WME_CAPINFO_UAPSD_EN            0x00000080
#define WME_CAPINFO_UAPSD_VO            0x00000001
#define WME_CAPINFO_UAPSD_VI            0x00000002
#define WME_CAPINFO_UAPSD_BK            0x00000004
#define WME_CAPINFO_UAPSD_BE            0x00000008
#define WME_CAPINFO_UAPSD_ACFLAGS_SHIFT     0
#define WME_CAPINFO_UAPSD_ACFLAGS_MASK      0xF
#define WME_CAPINFO_UAPSD_MAXSP_SHIFT       5
#define WME_CAPINFO_UAPSD_MAXSP_MASK        0x3
#define WME_CAPINFO_IE_OFFSET           8
#define WME_STA_UAPSD_MAXSP(_qosinfo) (((_qosinfo) >> WME_CAPINFO_UAPSD_MAXSP_SHIFT) & WME_CAPINFO_UAPSD_MAXSP_MASK)
#define WME_STA_UAPSD_AC_ENABLED(_ac, _qosinfo) ( (1<<(3 - (_ac))) &   \
                (((_qosinfo) >> WME_CAPINFO_UAPSD_ACFLAGS_SHIFT) & WME_CAPINFO_UAPSD_ACFLAGS_MASK) )
#define WME_STA_UAPSD_ENABLED(_qosinfo) \
        ((((_qosinfo) >> WME_CAPINFO_UAPSD_ACFLAGS_SHIFT) & WME_CAPINFO_UAPSD_ACFLAGS_MASK))
#define WME_STA_UAPSD_ALL_AC_ENABLED(_qosinfo)  (WME_STA_UAPSD_ENABLED(_qosinfo) == WME_CAPINFO_UAPSD_ACFLAGS_MASK)


inline static int wme_sta_uapsd_get_triggered_ac (int qosinfo)
{
    int uapsd_acs = WME_STA_UAPSD_ENABLED(qosinfo);

    if (uapsd_acs & WME_CAPINFO_UAPSD_VO)
        return WME_AC_VO;

    if (uapsd_acs & WME_CAPINFO_UAPSD_VI)
        return WME_AC_VI;

    if (uapsd_acs & WME_CAPINFO_UAPSD_BE)
        return WME_AC_BE;

    if (uapsd_acs & WME_CAPINFO_UAPSD_BK)
        return WME_AC_BK;

    return 0;
}

#define WME_AP_UAPSD_ENABLED(_qosinfo)  ((_qosinfo)&WME_CAPINFO_UAPSD_EN)

#define WIFINET_A_HT_TXCHWIDTH_20 0
#define WIFINET_A_HT_TXCHWIDTH_2040 1
#define WIFINET_A_HT_SMPOWERSAVE_ENABLED 0x01
#define WIFINET_A_HT_SMPOWERSAVE_MODE 0x02
#define WIFINET_BA_POLICY_IMMEDIATE 1
#define WIFINET_BA_AMSDU_SUPPORTED 1

struct wifi_mac_frame_min
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_addr1[WIFINET_ADDR_LEN];
    unsigned char  i_addr2[WIFINET_ADDR_LEN];
} __packed;

#define WIFINET_BAR_CTL_TID_M     0xF000
#define WIFINET_BAR_CTL_TID_S         12
struct wifi_mac_frame_bar
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_ra[WIFINET_ADDR_LEN];
    unsigned char  i_ta[WIFINET_ADDR_LEN];
    unsigned short   i_ctl;
    unsigned short   i_seq;
} __packed;

struct wifi_mac_frame_rts
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_ra[WIFINET_ADDR_LEN];
    unsigned char  i_ta[WIFINET_ADDR_LEN];
} __packed;

struct wifi_mac_frame_cts
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_ra[WIFINET_ADDR_LEN];
} __packed;

struct wifi_mac_frame_ack
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_ra[WIFINET_ADDR_LEN];
} __packed;

struct wifi_mac_frame_pspoll
{
    unsigned char  i_fc[2];
    unsigned char  i_aid[2];
    unsigned char  i_bssid[WIFINET_ADDR_LEN];
    unsigned char  i_ta[WIFINET_ADDR_LEN];
} __packed;

struct wifi_mac_frame_cfend
{
    unsigned char  i_fc[2];
    unsigned char  i_dur[2];
    unsigned char  i_ra[WIFINET_ADDR_LEN];
    unsigned char  i_bssid[WIFINET_ADDR_LEN];
} __packed;

#define WIFINET_CAPINFO_ESS             0x0001
#define WIFINET_CAPINFO_IBSS            0x0002
#define WIFINET_CAPINFO_PRIVACY     0x0010
#define WIFINET_CAPINFO_SHORT_PREAMBLE  0x0020
#define WIFINET_CAPINFO_SPECTRUM_MGMT       0x0100
#define WIFINET_CAPINFO_SHORT_SLOTTIME  0x0400

struct wifi_mac_ie_wpa
{
    unsigned char  wpa_id;
    unsigned char  wpa_len;
    unsigned char  wpa_oui[3];
    unsigned char  wpa_type;
    unsigned short wpa_version;
    unsigned int wpa_mcipher[1];
    unsigned short wpa_uciphercnt;
    unsigned int wpa_uciphers[8];
    unsigned short wpa_authselcnt;
    unsigned int wpa_authsels[8];
    unsigned short wpa_caps;
    unsigned short wpa_pmkidcnt;
    unsigned short wpa_pmkids[8];
} __packed;

struct wifi_mac_ie_rsn
{
    unsigned char  rsn_id;
    unsigned char  rsn_len;
    unsigned short rsn_version;
    unsigned int rsn_gcipher[1];
    unsigned short rsn_pciphercnt;
    unsigned int rsn_pciphers[8];
} __packed;
#ifndef _BYTE_ORDER
#error "Don't know native byte order"
#endif

struct wifi_mac_ie_htcap_cmn
{
    unsigned short hc_cap;
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned char  hc_reserved     : 3,
    hc_mpdudensity  : 3,
    hc_maxampdu     : 2;
#else
    unsigned char  hc_maxampdu     : 2,
    hc_mpdudensity  : 3,
    hc_reserved     : 3;
#endif
    unsigned char  hc_mcsset[16];
    unsigned short hc_extcap;
    unsigned int hc_txbf;
    unsigned char  hc_antenna;
} __packed;

struct wifi_mac_ie_htcap
{
    unsigned char hc_id;
    unsigned char hc_len;
    struct wifi_mac_ie_htcap_cmn hc_ie;
} __packed;


/* ldpc */

#define WIFINET_HTCAP_C_LDPC_RX		0x1
#define WIFINET_HTCAP_C_LDPC_TX		0x2

#define WIFINET_HTCAP_SUPPORTCBW40            0x0002

#define WIFINET_HTCAP_STATIC_SMPS      0x0000
#define WIFINET_HTCAP_LDPC             0x0001
#define WIFINET_HTCAP_DYNAMIC_SMPS     0x0004
#define WIFINET_HTCAP_DISABLE_SMPS       0x000c

#define WIFINET_HTCAP_C_SHORTGI20             0x0020
#define WIFINET_HTCAP_C_SHORTGI40         0x0040

#define WIFINET_HTCAP_C_RXSTBC_1SS     0x0100
#define WIFINET_HTCAP_C_MAXAMSDUSIZE      0x0800

#define WIFINET_HTCAP_USEDSSSCCK_40M          0x1000
#define WIFINET_HTCAP_40M_INTOLERANT          0x4000

#define WIFINET_HTCAP_C_SM_MASK                 0x000c
enum
{
    WIFINET_HTCAP_MAXRXAMPDU_8192,
    WIFINET_HTCAP_MAXRXAMPDU_16384,
    WIFINET_HTCAP_MAXRXAMPDU_32768,
    WIFINET_HTCAP_MAXRXAMPDU_65536,
};
#define WIFINET_HTCAP_MAXRXAMPDU_FACTOR   13
enum
{
    WIFINET_HTCAP_MPDUDENSITY_NA,
    WIFINET_HTCAP_MPDUDENSITY_0_25,
    WIFINET_HTCAP_MPDUDENSITY_0_5,
    WIFINET_HTCAP_MPDUDENSITY_1,
    WIFINET_HTCAP_MPDUDENSITY_2,
    WIFINET_HTCAP_MPDUDENSITY_4,
    WIFINET_HTCAP_MPDUDENSITY_8,
    WIFINET_HTCAP_MPDUDENSITY_16,
};

#define WIFINET_HTCAP_EXTC_HTC_SUPPORT       0x0400
#define WIFINET_HTCAP_EXTC_RDG_RESPONDER  0x0800


struct wifi_mac_ie_htinfo_cmn
{
    unsigned char hi_ctrlchannel;
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned char hi_serviceinterval: 3,
    hi_ctrlaccess: 1,
    hi_rifsmode: 1,
    hi_txchwidth: 1,
    hi_extchoff: 2;
#else
    unsigned char hi_extchoff: 2,
    hi_txchwidth: 1,
    hi_rifsmode: 1,
    hi_ctrlaccess: 1,
    hi_serviceinterval: 3;
#endif
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned short reserved1: 11,
    hi_obssnonhtpresent: 1,
    reserved2: 1,
    hi_nongfpresent: 1,
    hi_opmode: 2;
#else
    unsigned short hi_opmode: 2,
    hi_nongfpresent: 1,
    reserved2: 1,
    hi_obssnonhtpresent: 1,
    reserved1: 11;
#endif
    unsigned short hi_miscflags;
    unsigned char hi_basicmcsset[16];
} __packed;

struct wifi_mac_ie_htinfo
{
    unsigned char hi_id;
    unsigned char hi_len;
    struct wifi_mac_ie_htinfo_cmn hi_ie;
} __packed;

enum
{
    WIFINET_HTINFO_EXTOFFSET_NA  = 0,
    WIFINET_HTINFO_EXTOFFSET_ABOVE = 1,
    WIFINET_HTINFO_EXTOFFSET_UNDEF = 2,
    WIFINET_HTINFO_EXTOFFSET_BELOW = 3
};

enum
{
    WIFINET_HTINFO_TXWIDTH_20,
    WIFINET_HTINFO_TXWIDTH_2040
};
#define WIFINET_HTINFO_OPMODE_PURE          0x00
#define WIFINET_HTINFO_OPMODE_MIXED_PROT_OPT    0x01
#define WIFINET_HTINFO_OPMODE_MIXED_PROT_ALL    0x03

enum
{
    WIFINET_HTINFO_NON_GF_NOT_PRESENT,
    WIFINET_HTINFO_NON_GF_PRESENT,
};

enum
{
    WIFINET_HTINFO_TXBURST_UNLIMITED,
    WIFINET_HTINFO_TXBURST_LIMITED,
};
enum
{
    WIFINET_HTINFO_OBBSS_NONHT_NOT_PRESENT,
    WIFINET_HTINFO_OBBSS_NONHT_PRESENT,
};

enum
{
    WIFINET_HTINFO_RIFSMODE_PROHIBITED,
    WIFINET_HTINFO_RIFSMODE_ALLOWED,
};

enum
{
    WIFINET_ELEMID_SSID = 0,
    WIFINET_ELEMID_RATES = 1,
    WIFINET_ELEMID_FHPARMS = 2,
    WIFINET_ELEMID_DSPARMS = 3,
    WIFINET_ELEMID_CFPARMS = 4,
    WIFINET_ELEMID_TIM = 5,
    WIFINET_ELEMID_IBSSPARMS = 6,
    WIFINET_ELEMID_COUNTRY = 7,
    WIFINET_ELEMID_REQINFO = 10,
    WIFINET_ELEMID_BSS_LOAD = 11,
    WIFINET_ELEMID_CHALLENGE = 16,
    WIFINET_ELEMID_PWRCNSTR = 32,
    WIFINET_ELEMID_PWRCAP = 33,
    WIFINET_ELEMID_TPCREQ = 34,
    WIFINET_ELEMID_TPCREP = 35,
    WIFINET_ELEMID_SUPPCHAN = 36,
    WIFINET_ELEMID_CHANSWITCHANN = 37,
    WIFINET_ELEMID_MEASREQ = 38,
    WIFINET_ELEMID_MEASREP = 39,
    WIFINET_ELEMID_QUIET = 40,
    WIFINET_ELEMID_IBSSDFS = 41,
    WIFINET_ELEMID_ERP = 42,
    WIFINET_ELEMID_HTCAP = 45,
    WIFINET_ELEMID_RSN = 48,
    WIFINET_ELEMID_XRATES = 50,
    WIFINET_ELEMID_TIMEOUT = 56,
    WIFINET_ELEMID_SUPP_REG_CLASS = 59,
    WIFINET_ELEMID_EXTCHANSWITCHANN = 60,
    WIFINET_ELEMID_HTINFO = 61,
    WIFINET_ELEMID_2040_COEXT = 72,
    WIFINET_ELEMID_2040_INTOL = 73,
    WIFINET_ELEMID_OBSS_SCAN = 74,
    WIFINET_ELEMID_TIM_BROADCAST_REQ = 94,
    WIFINET_ELEMID_EXTCAP = 127,
    WIFINET_ELEMID_TPC = 150,
    WIFINET_ELEMID_CCKM = 156,

    WIFINET_ELEMID_VHTCAP = 191,  /* VHT Capabilities */
    WIFINET_ELEMID_VHTOP = 192,  /* VHT Operation */
    WIFINET_ELEMID_EXT_BSS_LOAD = 193,  /* Extended BSS Load */
    WIFINET_ELEMID_WIDE_BAND_CHAN_SWITCH = 194,  /* Wide Band Channel Switch */
    WIFINET_ELEMID_VHT_TX_PWR_ENVLP = 195,  /* VHT Transmit Power Envelope */
    WIFINET_ELEMID_CHAN_SWITCH_WRAP = 196,  /* Channel Switch Wrapper */
    WIFINET_ELEMID_AID = 197,  /* AID */
    WIFINET_ELEMID_QUIET_CHANNEL = 198,  /* Quiet Channel */
    WIFINET_ELEMID_OP_MODE_NOTIFY = 199,  /* Operating Mode Notification */
    WIFINET_ELEMID_VENDOR = 221,
    WIFINET_ELEMID_RSNX = 244,

#ifdef CONFIG_WAPI
    WIFINET_ELEMID_WAI = 68,
#endif //#ifdef CONFIG_WAPI
};

#define WIFINET_CHANSWITCHANN_BYTES 5
#define WIFINET_WIDEBANDCHANSW_BYTES 5
#define WIFINET_EXTCHANSWITCHANN_BYTES 6

struct wifi_mac_tim_ie
{
    unsigned char  tim_ie;
    unsigned char  tim_len;
    unsigned char  tim_count;
    unsigned char  tim_period;
    unsigned char  tim_bitctl;
    unsigned char  tim_bitmap[1];
} __packed;

struct wifi_mac_country_ie
{
    unsigned char  ie;
    unsigned char  len;
    unsigned char  cc[3];
    struct
    {
        unsigned char schan;
        unsigned char nchan;
        unsigned char maxtxpwr;
    } __packed band[4];
} __packed;

struct wifi_mac_quiet_ie
{
    unsigned char  ie;
    unsigned char  len;
    unsigned char  tbttcount;
    unsigned char  period;
    unsigned char  duration;
    unsigned char  offset;
} __packed;

struct wifi_mac_channelswitch_ie
{
    unsigned char  ie;
    unsigned char  len;
    unsigned char  switchmode;
    unsigned char  newchannel;
    unsigned char  tbttcount;
} __packed;

struct wifi_mac_tpc_ie
{
    unsigned char    ie;
    unsigned char    len;
    unsigned char    pwrlimit;
} __packed;

struct wifi_mac_erp_ie
{
    unsigned char  erp_ie;
    unsigned char  erp_len;
    unsigned char  erp;
} __packed;

struct wifi_mac_vendor_ie
{
    unsigned char    ie;
    unsigned char    len;
    unsigned char    buf[256];
} __packed;

#define WIFINET_CHALLENGE_LEN       128

#define WIFINET_SUPPCHAN_LEN        26


#define WIFINET_ERP_NON_ERP_PRESENT 0x01
#define WIFINET_ERP_USE_PROTECTION  0x02
#define WIFINET_ERP_LONG_PREAMBLE   0x04

#define WPA_OUI_BYTES       0x00, 0x50, 0xf2
#define RSN_OUI_BYTES       0x00, 0x0f, 0xac
#define WME_OUI_BYTES       0x00, 0x50, 0xf2

#define WPA_OUI         0xf25000
#define WPA_OUI_BE          0x0050f201
#define WPA_OUI_TYPE        0x01
#define WPA_VERSION     1


#define WPA_CSE_NULL 0x00
#define WPA_CSE_WEP40 0x01
#define WPA_CSE_TKIP 0x02
#define WPA_CSE_CCMP 0x04
#define WPA_CSE_WEP104 0x05

#define WPA_ASE_NONE 0x00
#define WPA_ASE_8021X_UNSPEC 0x01
#define WPA_ASE_8021X_PSK 0x02

#define RSN_OUI         0xac0f00
#define RSN_VERSION     1

#define RSN_CSE_NULL 0x00
#define RSN_CSE_WEP40 0x01
#define RSN_CSE_TKIP 0x02
#define RSN_CSE_WRAP 0x03
#define RSN_CSE_CCMP 0x04
#define RSN_CSE_WEP104 0x05
#define RSN_CSE_PSK_256 0x06
#define RSN_CSE_SAE 0x08

#define RSN_ASE_NONE        0x00
#define RSN_ASE_8021X_UNSPEC    0x01
#define RSN_ASE_8021X_PSK   0x02
#define RSN_ASE_8021X_EAP_SHA256 0x05
#define RSN_ASE_8021X_PSK_SHA256 0x06
#define RSN_ASE_8021X_SAE   0x08

#define RSN_CAP_PREAUTH     0x01

#define WME_OUI         0xf25000
#define WME_OUI_BE          0x0050f202
#define WME_OUI_TYPE        0x02
#define WME_INFO_OUI_SUBTYPE    0x00
#define WME_PARAM_OUI_SUBTYPE   0x01
#define WME_VERSION     1

#define WSC_OUI_BE                 0x0050f204
#define WPS_OUI               0xf25000


#define P2P_OUI_BE         0x506f9a09
#define WFD_OUI_BE         0x506f9a0a

#define IE_HDR_LEN    2
#define IE_LEN_OFFSET 1
#define OUI_LEN       4
#define WPA_PMKID_LEN 16



enum
{
    WIFINET_AUTH_OPEN_REQUEST       = 1,
    WIFINET_AUTH_OPEN_RESPONSE      = 2,
};

enum
{
    WIFINET_AUTH_SHARED_REQUEST = 1,
    WIFINET_AUTH_SHARED_CHALLENGE = 2,
    WIFINET_AUTH_SHARED_RESPONSE = 3,
    WIFINET_AUTH_SHARED_PASS = 4,
};

#define WIFINET_WEP_IVLEN       3
#define WIFINET_WEP_KIDLEN      1
#define WIFINET_WEP_CRCLEN      4
#define WIFINET_WEP_NKID        4
#define WIFINET_WEP_EXTIV       0x20
#define WIFINET_WEP_EXTIVLEN        4
#define WIFINET_WEP_MICLEN      8
#ifdef CONFIG_WAPI
#define WIFINET_WAPI_PN_LEN         16
#define WIFINET_WAPI_MIC_LEN        16
#define WIFINET_WAPI_EXHDR_LEN    (WIFINET_WAPI_PN_LEN+2)

#endif //#ifdef CONFIG_WAPI
#define WIFINET_CRC_LEN     4
#define WIFINET_MTU_MAX     2290
#define WIFINET_MTU_MIN     32

#define WIFINET_MIN_LEN (sizeof(struct wifi_mac_frame_min))
#define WIFINET_N_MAX_FRAMELEN  3839

#define WIFINET_AID_MAX     2007
#define WIFINET_AID_DEF     (WIFI_MAX_STA+1)

#define WIFINET_AID(b)  ((b) &~ 0xc000)
#define WIFINET_RTS_MAX     2347

#define WIFINET_OBSS_SCAN_PASSIVE_DWELL_DEF  20
#define WIFINET_OBSS_SCAN_ACTIVE_DWELL_DEF   10
#define WIFINET_OBSS_SCAN_INTERVAL_DEF       300
#define WIFINET_OBSS_SCAN_PASSIVE_TOTAL_DEF  200
#define WIFINET_OBSS_SCAN_ACTIVE_TOTAL_DEF   20
#define WIFINET_OBSS_SCAN_THRESH_DEF   25
#define WIFINET_OBSS_SCAN_DELAY_DEF   5

struct wifi_mac_ie_obss_scan
{
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned short scan_passive_dwell;
    unsigned short scan_active_dwell;
    unsigned short bss_cw_trigger_scan_interval;
    unsigned short scan_passive_total;
    unsigned short scan_active_total;
    unsigned short scan_delay;
    unsigned short scan_thresh;
} __packed;

struct wifi_mac_ie_ext_cap
{
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char ext_capflags;
    unsigned char ext2_capflags[3];
    unsigned char ext3_capflags[4];
} __packed;

#define SET_HT_CAP_INFO_LGSIG_TXOP_PRCT(dst, val) do {(dst) &= ~(0x1 <<15) ; (dst) |= (val) << 15;}while(0)
	#define HT_CAP_INFO_NOT_SPT_LGSIG_TXOP_PRCT	(0)

/* VHT capability flags */
/* B0-B1 Maximum MPDU Length */
#define WIFINET_VHTCAP_MAX_MPDU_LEN_7991     0x00000001 /* A-MSDU Length 7991 octets */
#define WIFINET_VHTCAP_MAX_MPDU_LEN_11454    0x00000002 /* A-MSDU Length 11454 octets */

/* B2-B3 Supported Channel Width */
#define WIFINET_VHTCAP_SUP_CHAN_WIDTH_80     0x00000000 /* Does not support 160 or 80+80 */

#define WIFINET_VHTCAP_RX_LDPC             0x00000010 /* B4 RX LDPC */
#define WIFINET_VHTCAP_SHORTGI_80          0x00000020 /* B5 Short GI for 80MHz */
#define WIFINET_VHTCAP_TX_STBC             0x00000080 /* B7 Tx STBC */

#define WIFINET_VHTCAP_RX_STBC             0x00000700 /* B8-B10 Rx STBC */

#define WIFINET_VHTCAP_SU_BFMER           0x00000800  /* Bit11 su beamformer */
#define WIFINET_VHTCAP_SU_BFMEE           0x00001000  /* Bit12 su beamformee */
#define WIFINET_VHTCAP_MU_BFMER           0x00080000  /* Bit19 mu beamformer */
#define WIFINET_VHTCAP_MU_BFMEE           0x00100000  /* Bit20 mu beamformee */

#define WIFINET_VHTCAP_BF_MAX_ANT          0x0000E000 /* B13-B15 Compressed steering number of
                                                         * beacon former Antennas supported */

#define WIFINET_VHTCAP_SOUND_DIMENSIONS    0x00070000 /* B16-B18 Sounding Dimensions */


#define WIFINET_VHTCAP_MAX_AMPDU_LEN_FACTOR  13
#define WIFINET_VHTCAP_MAX_AMPDU_LEN_EXP   0x03800000 /* B23-B25 maximum AMPDU Length Exponent */
#define WIFINET_VHTCAP_MAX_AMPDU_LEN_EXP_S 23


/*
 * 802.11ac VHT Capability IE
 */
struct wifi_mac_ie_vht_cap {
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned int vht_cap_info;
    unsigned short rx_mcs_map;          /* B0-B15 Max Rx MCS for each SS */
    unsigned short rx_high_data_rate;   /* B16-B28 Max Rx data rate,
                                            Note:  B29-B31 reserved */
    unsigned short tx_mcs_map;          /* B32-B47 Max Tx MCS for each SS */
    unsigned short tx_high_data_rate;   /* B48-B60 Max Tx data rate,
                                            Note: B61-B63 reserved */
} __packed;

/* VHT Operation  */
#define WIFINET_VHTOP_CHWIDTH_2040 0 /* 20/40 MHz Operating Channel */
#define WIFINET_VHTOP_CHWIDTH_80 1 /* 80 MHz Operating Channel */
#define WIFINET_VHTOP_CHWIDTH_160 2 /* 160 MHz Operating Channel */
#define WIFINET_VHTOP_CHWIDTH_80_80 3 /* 80 + 80 MHz Operating Channel */

/*
 * 802.11ac VHT Operation IE
 */
struct wifi_mac_ie_vht_opt {
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char vht_op_chwidth;              /* BSS Operational Channel width */
    unsigned char vht_op_ch_freq_seg1;         /* Channel Center frequency */
    unsigned char vht_op_ch_freq_seg2;         /* Channel Center frequency applicable
                                                  * for 80+80MHz mode of operation */
    unsigned short vhtop_basic_mcs_set;         /* Basic MCS set */
} __packed;


/*
 * 802.11n Secondary Channel Offset element
 */
struct wifi_mac_ie_sec_chan_offset {
    unsigned char elem_id;
    unsigned char len;
    unsigned char sec_chan_offset;
} __packed;

/*
 * 802.11ac Transmit Power Envelope element
 */
struct wifi_mac_ie_vht_txpwr_env {
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char txpwr_info;       /* Transmit Power Information */
    unsigned char local_max_txpwr[4]; /* Local Max TxPower for 20,40,80,160MHz */
} __packed;

#define SET_LMAX_TXPW_CNT(dst, val) do {(dst) &= ~(0x7 <<0) ; (dst) |= (val) << 0;}while(0)
#define GET_LMAX_TXPW_CNT(dst) ( ((dst)>>0) &0x7 )
#define SET_LMAX_TXPW_UNIT_INTP_CNT(dst,val)  do {(dst) &= ~(0x7 <<3) ; (dst) |= (val) << 3;}while(0)
#define GET_LMAX_TXPW_UNIT_INIT_CNT(dst)  ( ((dst)>>3) &0x7 )
#define LC_MAX_TXPW_LEN   (4)

#define SUB_IE_MAX_LEN (32)
struct wifi_mac_ie_vht_ch_sw_wrp
{
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char new_country_sub_ie[SUB_IE_MAX_LEN];
    unsigned char wide_bw_ch_sw_sub_ie[SUB_IE_MAX_LEN];
    unsigned char new_vht_tx_pw_sub_ie[SUB_IE_MAX_LEN];
} __packed;
/*
 * 802.11ac Wide Bandwidth Channel Switch Element
 */
enum wifi_mac_new_channel_width {
    NEW_CHANNEL_BANDWIDTH40 = 0,
    NEW_CHANNEL_BANDWIDTH80 = 1,
};

struct wifi_mac_ie_vht_wide_bw_switch {
    unsigned char    elem_id;
    unsigned char    elem_len;
    unsigned char    new_ch_width;       /* New channel width */
    unsigned char    new_ch_freq_seg1;   /* Channel Center frequency 1 */
    unsigned char    new_ch_freq_seg2;   /* Channel Center frequency 2 */
} __packed;

struct wifi_mac_ie_vht_ext_bss_ld{
    unsigned char 	 elem_id;
    unsigned char    elem_len;
    unsigned short  mu_mimo_sta_cnt;
    unsigned char    ss_ult;
    unsigned char    obs_scnd_20mhz;
    unsigned char    obs_scnd_40mhz;
    unsigned char    obs_scnd_80mhz;
}__packed;


struct wifi_mac_ie_vht_quiet_chn{
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char ap_quiet_md;
    unsigned char quiet_cnt;
    unsigned char quiet_prd;
    unsigned short quiet_drt;
    unsigned short quiet_offset;
} __packed;

struct wifi_mac_ie_vht_opt_md_ntf{
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char opt_mode;
} __packed;

struct wifi_mac_ie_timeout {
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char timeout_interval_type;
    unsigned int timeout_val;
} __packed;

enum wifi_mac_ie_timeout_type {
    REASSOC_DEDLINE_INTERVAL = 1,
    KEY_LIFETIME_INTERVAL = 2,//seconds
    ASSOC_COMEBACK_TIME = 3,
    TIME_TO_START = 4,
};

#define SET_OPMD_RX_NSS(dst, val) do {(dst) &= ~(0x7 <<4) ; (dst) |= (val) << 4;}while(0)
#define GET_OPMD_RX_NSS(dst) ( ((dst)>>4) &0x7 )

/*VHT capability element field set and get*/

/*VHT capability maximum MPDU length 0:3895B; 1:7991B; 2:11454B;3:RSV*/
#define SET_VHT_CAP_MAX_MPDU(dst, val) do {(dst) &= ~(0x3 <<0) ; (dst) |= (val) << 0;}while(0)
#define VHT_CAP_MAX_MPDU_LEN_3895 (0)
#define VHT_CAP_MAX_MPDU_LEN_7991 (1)
#define VHT_CAP_MAX_MPDU_LEN_11454 (2)


/*VHT capability support channel width set*/
#define VHT_CAP_160M (1)   //only 160M

/*rx ldpc*/
#define SET_VHT_CAP_RX_LPDC(dst, val) do {(dst) &= ~(0x1 <<4) ; (dst) |= (val) << 4;}while(0)
#define GET_VHT_CAP_RX_LPDC(dst) (((dst) >>4) & 0x1)
#define VHT_CAP_RX_LDPC_SUPPORTED (1)


/*short gi for 80MHZ*/
#define SET_VHT_CAP_80M_SGI(dst, val) do {(dst) &= ~(0x1 <<5) ; (dst) |= (val) << 5;}while(0)
#define VHT_CAP_SUPPORT_80M_SGI (1)


/*short gi for 160M and 80+80 MHz*/


/*TX STBC*/
#define SET_VHT_CAP_TX_STBC(dst, val) do {(dst) &= ~(0x1 <<7) ; (dst) |= (val) << 7;}while(0)
#define GET_VHT_CAP_TX_STBC(dst) (((dst) >> 7) & 0x1 )
#define VHT_CAP_NOT_SUPPORT_TX_STBC (0)


/*RX STBC*/
#define SET_VHT_CAP_RX_STBC(dst, val) do {(dst) &= ~(0x7 <<8) ; (dst) |= (val) << 8;}while(0)
#define GET_VHT_CAP_RX_STBC(dst) (((dst) >>8) & 0x7)
#define VHT_CAP_RX_STBC_MAX_1SS (1)   // 1 SS

/*su beamformer capable*/
#define SET_VHT_CAP_SU_BFMER(dst, val) do {(dst) &= ~(0x1 <<11) ; (dst) |= (val) << 11;}while(0)
#define VHT_CAP_NOT_SUPPORT_SU_BFMER    (0)
#define VHT_CAP_SUPPORT_SU_BFMER            (1)

/*su beanformee capable*/
#define SET_VHT_CAP_SU_BFMEE(dst, val) do {(dst) &= ~(0x1 <<12) ; (dst) |= (val) << 12;}while(0)
#define VHT_CAP_NOT_SUPPORT_SU_BFMEE   (0)
#define VHT_CAP_SUPPORT_SU_BFMEE           (1)


/*beamformee sts capability*/
#define SET_VHT_CAP_BFMEE_STS(dst, val) do {(dst) &= ~(0x7 <<13) ; (dst) |= (val) << 13;}while(0)
    //?

/* number of sounding dimension */
#define SET_VHT_CAP_SND_DMS_NUM(dst, val) do {(dst) &= ~(0x7 <<16) ; (dst) |= (val) << 16;}while(0)
   //?

/*mu beaformer capable*/
#define SET_VHT_CAP_MU_BFMER(dst, val) do {(dst) &= ~(0x1 <<19) ; (dst) |= (val) << 19;}while(0)
#define VHT_CAP_NOT_SUPPORT_MU_BFMER    (0)
#define VHT_CAP_SUPPORT_MU_BFMER            (1)


/*mu beam former capable*/
#define SET_VHT_CAP_MU_BFMEE(dst, val) do {(dst) &= ~(0x1 <<20) ; (dst) |= (val) << 20;}while(0)
#define VHT_CAP_NOT_SUPPORT_MU_BFMEE    (0)
#define VHT_CAP_SUPPORT_MU_BFMEE            (1)

/*VHT TXOP PS*/
#define SET_VHT_CAP_TXOP_PS(dst, val) do {(dst) &= ~(0x1 <<21) ; (dst) |= (val) << 21;}while(0)
#define VHT_CAP_TXOP_PS_NOT_SUPPORT		(0)

/*+HTC-VHT capable*/
#define SET_VHT_CAP_HTC_VHT(dst, val) do {(dst) &= ~(0x1 <<22) ; (dst) |= (val) << 22;}while(0)
#define VHT_CAP_NOT_SUPPORT_HTCVHT	(0)

/*MAX-AMPDU length exponet*/
#define SET_VHT_CAP_MAX_APMDU_LEN_EXP(dst, val) do {(dst) &= ~(0x7 <<23) ; (dst) |= (val) << 23;}while(0)
enum
{
    MAX_VHT_AMPDU_8K =0,
    MAX_VHT_AMPDU_16K,
    MAX_VHT_AMPDU_32K,
    MAX_VHT_AMPDU_64K,
    MAX_VHT_AMPDU_128K,
    MAX_VHT_AMPDU_256K,
    MAX_VHT_AMPDU_512K,
    MAX_VHT_AMPDU_1024K,
};

/*VHT link adaptation capable*/
#define SET_VHT_CAP_LK_ADP(dst, val) do {(dst) &= ~(0x3 <<26) ; (dst) |= (val) << 26;}while(0)
#define VHT_CAP_LK_ADP_NO_FB (0)

/*RX Antena pattern consistency*/
#define SET_VHT_CAP_RX_ATN_CONSTN(dst, val) do {(dst) &= ~(0x1 <<28) ; (dst) |= (val) << 28;}while(0)
#define VHT_CAP_RX_ATN_CONSTN_NOT_CHANGE (1)

/*TX Antena pattern consistency*/
#define SET_VHT_CAP_TX_ATN_CONSTN(dst, val) do {(dst) &= ~(0x1 <<29) ; (dst) |= (val) << 29;}while(0)
#define VHT_CAP_TX_ATN_CONSTN_NOT_CHANGE (1)

/*VHT  CAP RSV*/
#define SET_VHT_CAP_RSV(dst, val) do {(dst) &= ~(0x3 <<30) ; (dst) |= (val) << 30;}while(0)


struct wifi_scan_info;
struct wlan_net_vif;

struct WmeParamDefault
{
    unsigned char aifsn;
    unsigned char logcwmin;
    unsigned char logcwmax;
    unsigned short txopLimit;
    unsigned char acm;
} ;

enum wifi_mac_wmm_phymode
{
    WIFINET_WMM_PHYMODE_B = 0,
    WIFINET_WMM_PHYMODE_G = 1,
    WIFINET_WMM_PHYMODE_MAX,
};

static struct WmeParamDefault const ap_phy_param_AC_BK[WIFINET_WMM_PHYMODE_MAX] =
{
    {          7,                4,               10,                 0,              0 },
    {          7,                4,               10,                 0,              0 },
};

static struct WmeParamDefault const ap_phy_param_AC_BE[WIFINET_WMM_PHYMODE_MAX] =
{
    {          3,                4,                6,                  0,              0 },
    {          3,                4,                6,                  0,              0 },
};

static struct WmeParamDefault const ap_phy_param_AC_VI[WIFINET_WMM_PHYMODE_MAX] =
{
    {          1,                3,               4,                 188,              0 },
    {          1,                3,               4,                  94,              0 },
};

static struct WmeParamDefault const ap_phy_param_AC_VO[WIFINET_WMM_PHYMODE_MAX] =
{
    {          1,                2,               3,                 102,              0 },
    {          1,                2,               3,                  47,              0 },
};

static struct WmeParamDefault const sta_phy_param_AC_BK[WIFINET_WMM_PHYMODE_MAX] =
{
    {          7,                4,               10,                 0,              0 },
    {          7,                4,               10,                 0,              0 },
};

static struct WmeParamDefault const sta_phy_param_AC_BE[WIFINET_WMM_PHYMODE_MAX] =
{
    {          3,                4,              10,                  0,              0 },
    {          3,                4,              10,                  0,              0 },
};

static struct WmeParamDefault const sta_phy_param_AC_VI[WIFINET_WMM_PHYMODE_MAX] =
{
    {          2,                3,               4,                 188,              0 },
    {          2,                3,               4,                  94,              0 },
};

static struct WmeParamDefault const sta_phy_param_AC_VO[WIFINET_WMM_PHYMODE_MAX] =
{
    {          2,                2,               3,                 102,              0 },
    {          2,                2,               3,                  47,              0 },
};

static struct WmeParamDefault const phy_param_BE_nonqos[WIFINET_WMM_PHYMODE_MAX] =
{
    {          2,                5,               10,          0,  0 },
    {          2,                4,               10,          0,  0 },
};

static const unsigned char log_cwwin_BE_agg_log[WIFINET_WMM_PHYMODE_MAX] =
{
    4,
    3,
};

#define AML_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))

#endif /* _NET80211_IEEE80211_H_ */
