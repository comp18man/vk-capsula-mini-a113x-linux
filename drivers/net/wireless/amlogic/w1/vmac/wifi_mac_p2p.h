/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer p2p  module
 *
 *
 ****************************************************************************************
 */
#ifndef IEEE_P2P_H
#define IEEE_P2P_H

enum p2p_attr_id
{
    P2P_ATTR_STATUS = 0,
    P2P_ATTR_MINOR_REASON_CODE = 1,
    P2P_ATTR_CAPABILITY = 2,
    P2P_ATTR_DEVICE_ID = 3,
    P2P_ATTR_GROUP_OWNER_INTENT = 4,
    P2P_ATTR_CONFIGURATION_TIMEOUT = 5,
    P2P_ATTR_LISTEN_CHANNEL = 6,
    P2P_ATTR_GROUP_BSSID = 7,
    P2P_ATTR_EXT_LISTEN_TIMING = 8,
    P2P_ATTR_INTENDED_INTERFACE_ADDR = 9,
    P2P_ATTR_MANAGEABILITY = 10,
    P2P_ATTR_CHANNEL_LIST = 11,
    P2P_ATTR_NOTICE_OF_ABSENCE = 12,
    P2P_ATTR_DEVICE_INFO = 13,
    P2P_ATTR_GROUP_INFO = 14,
    P2P_ATTR_GROUP_ID = 15,
    P2P_ATTR_INTERFACE = 16,
    P2P_ATTR_OPERATING_CHANNEL = 17,
    P2P_ATTR_INVITATION_FLAGS = 18,
    P2P_ATTR_VENDOR_SPECIFIC = 221
};

enum p2p_status_code
{
    P2P_SC_SUCCESS = 0,
    P2P_SC_FAIL_INFO_CURRENTLY_UNAVAILABLE = 1,
    P2P_SC_FAIL_INCOMPATIBLE_PARAMS = 2,
    P2P_SC_FAIL_LIMIT_REACHED = 3,
    P2P_SC_FAIL_INVALID_PARAMS = 4,
    P2P_SC_FAIL_UNABLE_TO_ACCOMMODATE = 5,
    P2P_SC_FAIL_PREV_PROTOCOL_ERROR = 6,
    P2P_SC_FAIL_NO_COMMON_CHANNELS = 7,
    P2P_SC_FAIL_UNKNOWN_GROUP = 8,
    P2P_SC_FAIL_BOTH_GO_INTENT_15 = 9,
    P2P_SC_FAIL_INCOMPATIBLE_PROV_METHOD = 10,
    P2P_SC_FAIL_REJECTED_BY_USER = 11,
};

#define P2P_GO_NEGO_REQ 0
#define P2P_GO_NEGO_RESP 1
#define P2P_GO_NEGO_CONF 2
#define P2P_INVITE_REQ 3
#define P2P_INVITE_RESP 4
#define P2P_DEVDISC_REQ 5
#define P2P_DEVDISC_RESP 6
#define P2P_PROVISION_DISC_REQ 7
#define P2P_PROVISION_DISC_RESP 8

#define P2P_NOA_IGNORE (1)
#define P2P_OPPS_IGNORE (1)
#define P2P_NOA_START_IN_FUTURE BIT(0)
#define P2P_NOA_START_IN_PAST BIT(1)
#define P2P_NOA_START_NEED_WRAP BIT(2)

enum p2p_act_frame_type
{
    P2P_NOA = 0,
    P2P_PRESENCE_REQ = 1,
    P2P_PRESENCE_RESP = 2,
    P2P_GO_DISC_REQ = 3
};

enum NET80211_P2P_ROLE
{
    NET80211_P2P_ROLE_DEVICE = 0,
    NET80211_P2P_ROLE_CLIENT = 1,
    NET80211_P2P_ROLE_GO = 2,
};

enum NET80211_P2P_STATE
{
    NET80211_P2P_STATE_NONE = 0,
    NET80211_P2P_STATE_IDLE = 1,
    NET80211_P2P_STATE_LISTEN = 2,
    NET80211_P2P_STATE_SCAN = 3,
    NET80211_P2P_STATE_FIND_PHASE_LISTEN = 4,
    NET80211_P2P_STATE_FIND_PHASE_SEARCH = 5,
};

enum NET80211_P2P_NEGO_STATE {
    NET80211_P2P_STATE_TX_IDLE = 0,
    NET80211_P2P_STATE_FIRST_TX_NEGO_REQ = 1,
    NET80211_P2P_STATE_FIRST_RX_NEGO_RSP = 2,
    NET80211_P2P_STATE_FIRST_RX_NEGO_REQ = 3,
    NET80211_P2P_STATE_FIRST_TX_NEGO_RSP = 4,
    NET80211_P2P_STATE_RX_INVITE_REQ = 5,
    NET80211_P2P_STATE_TX_INVITE_RSP = 6,
    NET80211_P2P_STATE_TX_INVITE_REQ = 7,
    NET80211_P2P_STATE_RX_INVITE_RSP = 8,
    NET80211_P2P_STATE_SECOND_TX_NEGO_REQ = 9,
    NET80211_P2P_STATE_SECOND_RX_NEGO_REQ = 10,
    NET80211_P2P_STATE_SECOND_TX_NEGO_RSP = 11,
    NET80211_P2P_STATE_SECOND_RX_NEGO_RSP = 12,
    NET80211_P2P_STATE_GONEGO_CONF = 13,
    NET80211_P2P_STATE_GO_COMPLETE = 14,
};

enum P2P_WPSINFO
{
    P2P_NO_WPSINFO = 0,
    P2P_GOT_WPSINFO_PEER_DISPLAY_PIN = 1,
    P2P_GOT_WPSINFO_SELF_DISPLAY_PIN = 2,
    P2P_GOT_WPSINFO_PBC = 3,
};

enum P2P_PS
{
    P2P_PS_DISABLE=0,
    P2P_PS_ENABLE=1,
    P2P_PS_SCAN=2,
    P2P_PS_SCAN_DONE=3,
    P2P_PS_ALLSTASLEEP=4,
};

enum P2P_FLAG
{
    P2P_OPPPS_CWEND_FLAG_HI = BIT(0),
    P2P_NOA_INDEX_INIT_FLAG = BIT(1),
    P2P_OPPPS_START_FLAG_HI = BIT(2),
    P2P_NOA_START_FLAG_HI = BIT(3),
    P2P_NOA_START_MATCH_BEACON_HI = BIT(4),
    P2P_NOA_END_MATCH_BEACON_HI = BIT(5),
    P2P_WAIT_SWITCH_CHANNEL = BIT(6),
    P2P_ALLOW_SWITCH_CHANNEL = BIT(7),
    P2P_CHECK_RX_CHANNEL_LIST_PASS = BIT(8),
    P2P_CHECK_TX_CHANNEL_LIST_PASS = BIT(9),
    P2P_CHANGE_CHANNEL_LIST = BIT(10),
    P2P_REQUEST_SSID = BIT(11),
#ifdef CTS_VERIFIER_GAS
    P2P_GAS_RSP = BIT(12),
#endif
};

#define SOCIAL_CHAN_1       (2412)
#define SOCIAL_CHAN_2       (2437)
#define SOCIAL_CHAN_3       (2462)
#define SOCIAL_CHAN_CNT 3
#define P2P_WILDCARD_SSID "DIRECT-"
#define P2P_WILDCARD_SSID_LEN  7

#define P2P_BEACON_IE       1
#define P2P_PROBE_RESP_IE   2
#define P2P_ASSOC_RESP_IE   4

#define P2P_PROBE_REQ_IE    3
#define P2P_ASSOC_REQ_IE    5

struct wifi_mac_p2p_action_frame
{
    unsigned char   category;
    unsigned char   OUI[3];
    unsigned char   type;
    unsigned char   subtype;
    unsigned char   dialog_token;
    unsigned char   elts[1];
} __packed;

struct wifi_mac_p2p_pub_act_frame
{
    unsigned char   category;
    unsigned char   action;
    unsigned char   oui[3];
    unsigned char   oui_type;
    unsigned char   subtype;
    unsigned char   dialog_token;
    unsigned char   elts[1];

} __packed;

struct wifi_mac_p2p_ie
{
    unsigned char   id;
    unsigned char   len;
    unsigned char   OUI[3];
    unsigned char   oui_type;
    unsigned char   subelts[1];
} __packed;

struct p2p_ie_listen_chan_attr
{
    unsigned char   attr_id;
    unsigned char   len[2];
    unsigned char   country_s[3];
    unsigned char   operat_class;
    unsigned char   channel;
} __packed;

struct p2p_ie_operate_chan_attr
{
    unsigned char   attr_id;
    unsigned char   len[2];
    unsigned char   country_s[3];
    unsigned char   operat_class;
    unsigned char   channel;
} __packed;

struct p2p_ie_intent_attr
{
    unsigned char   attr_id;
    unsigned char   len[2];
    unsigned char   go_intent;
} __packed;


struct p2p_ie_chanlist_attr_chan_entry
{
    unsigned char   operat_class;
    unsigned char   num;
    unsigned char   chan_list[1];
} __packed;

struct p2p_ie_chan_list_attr
{
    unsigned char     attr_id;
    unsigned short   len;
    unsigned char     country_s[3];
    struct   p2p_ie_chanlist_attr_chan_entry chan_entry;
} __packed;

struct p2p_ie_go_intent
{
    unsigned char   attr_id;
    unsigned char   len[2];
    unsigned char   go_intent;
} __packed;

#ifdef CONFIG_WFD

#define WFD_ATTR_DEVICE_INFO            0x00
#define WFD_ATTR_ASSOC_BSSID            0x01
#define WFD_ATTR_COUPLED_SINK_INFO  0x06
#define WFD_ATTR_SESSION_INFO       0x09

#define WFD_DEVINFO_SOURCE                  0x0000
#define WFD_DEVINFO_PSINK                   0x0001

#define WFD_DEVINFO_SESSION_AVAIL           0x0010
#define WFD_DEVINFO_WSD                     0x0040
#define WFD_DEVINFO_PC_TDLS                 0x0080

struct wifi_mac_wfd_info
{
    unsigned short wfd_enable;
    unsigned short rtsp_ctrlport;
    unsigned short peer_rtsp_ctrlport;
    unsigned char peer_session_avail;
    unsigned char ip_address[4];
    unsigned char peer_ip_address[4];
    unsigned char wfd_pc;
    unsigned char wfd_device_type;
    unsigned short max_thruput;
    unsigned char wfd_tdls_enable;
    unsigned char session_available;
};
#endif //CONFIG_WFD

struct type_ctw_opps_s
{
    unsigned char ctw:7,
    opps:1;
} ;

union type_ctw_opps_u
{
    struct type_ctw_opps_s ctw_opps_s;
    unsigned char ctw_opps_u8;
} ;

struct p2p_noa
{
    unsigned int start;
    unsigned int interval;
    unsigned int duration;
    unsigned char count;
} ;

#define NET80211_P2P_SCHED_RSVD   0
#define NET80211_P2P_SCHED_REPEAT 255

#define P2P_OPPS_CTW_MIN                (5)
#define P2P_NOA_DURATION_MIN        (5*1024)

#define P2P_NOA_START_TIME_WRAP     (1<<31)

#define P2P_LISTEN_TIMER_PERIOD_DEFAULT                 (300)
#define P2P_LISTEN_TIMER_PERIOD_DELAY_MORE         (200)

enum reason_restore_bsschan
{
    REASON_RESOTRE_BSSCHAN_NONE,
    REASON_RESOTRE_BSSCHAN_REMAIN,
    REASON_RESOTRE_BSSCHAN_TXMGNT,
    REASON_RESOTRE_BSSCHAN_MAX,
} ;

struct channel_list_param
{
    unsigned char operating_class;
    unsigned short chan_pri_num[16];
    unsigned char per_chan_num;
};

#define P2P_MAX_CHAN_LIST_LEN 32
struct channel_list
{
    struct channel_list_param opera_class_and_chan[P2P_MAX_CHAN_LIST_LEN];
    unsigned char oper_class_num;
};

#define P2P_MAX_CHANNELS 50
#define P2P_MAX_ACTION_LEN 1024
#define MAX_MAC_BUF_LEN 18
#define MAC_WFD_SESSION_LEN 16
struct wifi_mac_p2p
{
    struct wlan_net_vif *wnet_vif;
    enum NET80211_P2P_ROLE p2p_role;
    enum NET80211_P2P_NEGO_STATE p2p_negotiation_state;
    enum NET80211_P2P_STATE p2p_state;
    unsigned char social_channel;
    unsigned char oper_channel;
    unsigned char p2p_enable;
    unsigned char action_dialog_token;
    unsigned int  action_pkt_len;
#ifdef CTS_VERIFIER_GAS
    unsigned int  action_code;
#endif
    unsigned short action_retry_time;
    unsigned char action_pkt[P2P_MAX_ACTION_LEN];
    struct wifi_channel *work_channel;

    struct ieee80211_channel remain_on_ch_channel;
    enum nl80211_channel_type remain_on_ch_type;
    unsigned long long remain_on_ch_cookie;
    struct net_device *remain_on_ch_dev;
    enum reason_restore_bsschan need_restore_bsschan;
    unsigned char dev_addr[WIFINET_ADDR_LEN];
    unsigned char interface_addr[WIFINET_ADDR_LEN];
    struct os_timer_ext listen_timer;
    struct wifi_mac_app_ie_t p2p_app_ie[WIFINET_APPIE_NUM_OF_FRAME];
    struct wifi_mac_app_ie_t wps_beacon_ie;

#ifdef CONFIG_WFD
    struct wifi_mac_wfd_info wfd_info;
    struct wifi_mac_app_ie_t wfd_app_ie[WIFINET_APPIE_NUM_OF_FRAME];
#endif

    union type_ctw_opps_u ctw_opps_u;
    struct p2p_noa noa;
    unsigned char noa_index;
    struct wifi_mac_app_ie_t noa_app_ie[WIFINET_APPIE_NUM_OF_FRAME];
    unsigned short HiP2pNoaCountNow;

    unsigned short p2p_flag;
    unsigned char p2p_listen_count;
    unsigned char change_intent_flag;
    unsigned char act_pkt_retry_count;
    unsigned char send_tx_status_flag;
    unsigned char raw_action_pkt[P2P_MAX_ACTION_LEN];
    unsigned int  raw_action_pkt_len;
    enum wifi_mac_tx_status_mode tx_status_flag;
    unsigned long long cookie;
    struct channel_list peer_chan_list;
    struct channel_list self_chan_list;
    unsigned char peer_listen_channel;
    unsigned char peer_dev_addr[MAX_MAC_BUF_LEN];
    unsigned char ap_mode_set_noa_enable;
    unsigned char wfd_session_id[MAC_WFD_SESSION_LEN];
    unsigned char go_hidden_mode;
};

static inline void vm_p2p_set_enable(struct wifi_mac_p2p *p2p, int enable)
{
    p2p->p2p_enable = enable;
}

static inline unsigned char vm_p2p_enabled(struct wifi_mac_p2p *p2p)
{
    return (p2p->p2p_enable == 1);
}

static inline void vm_wfd_set_enable(struct wifi_mac_p2p *p2p, int enable)
{
    p2p->wfd_info.wfd_enable = enable;
}

static inline unsigned char vm_wfd_enabled(struct wifi_mac_p2p *p2p)
{
    return (p2p->wfd_info.wfd_enable  == 1);
}

static inline void vm_p2p_set_state(struct wifi_mac_p2p *p2p, enum NET80211_P2P_STATE state)
{
    if (p2p->p2p_state != state)
    {
        p2p->p2p_state = state;
    }
}

static inline void vm_p2p_set_role(struct wifi_mac_p2p *p2p, enum NET80211_P2P_ROLE p2p_role)
{

    if (p2p->p2p_role != p2p_role)
    {
        p2p->p2p_role = p2p_role;
        DPRINTF(AML_DEBUG_ANY, "%s %d p2p_role =%d\n", __func__, __LINE__, p2p_role);
        if( p2p->p2p_role == NET80211_P2P_ROLE_DEVICE)
        {
            p2p->oper_channel = 0;
        }
    }
}
static inline int vm_p2p_state(struct wifi_mac_p2p *p2p)
{
    return p2p->p2p_state;
}

static inline int vm_p2p_role(struct wifi_mac_p2p *p2p)
{
    return p2p->p2p_role;
}
static inline unsigned char vm_p2p_is_state(struct wifi_mac_p2p *p2p, enum NET80211_P2P_STATE state)
{
    return p2p->p2p_state == state;
}

static inline unsigned char vm_p2p_chk_p2p_role(struct wifi_mac_p2p *p2p, enum NET80211_P2P_ROLE p2p_role)
{
    return p2p->p2p_role == p2p_role;
}

#ifdef  CONFIG_P2P
static inline int NL80211_IFTYPE_2_p2p_role (enum nl80211_iftype NL80211_IFTYPE)
{
    if (NL80211_IFTYPE_P2P_GO == NL80211_IFTYPE)
    {
        return NET80211_P2P_ROLE_GO;
    }
    else if (NL80211_IFTYPE_P2P_CLIENT == NL80211_IFTYPE)
    {
        return NET80211_P2P_ROLE_CLIENT;
    }
    else
    {
        return NET80211_P2P_ROLE_DEVICE;
    }
}
#endif

int vm_p2p_go_cancel_opps (struct wifi_mac_p2p *p2p);
int vm_p2p_go_cancel_noa (struct wifi_mac_p2p *p2p);
int vm_p2p_client_cancel_opps (struct wifi_mac_p2p *p2p);
int vm_p2p_opps_start(struct wifi_mac_p2p *p2p, const union type_ctw_opps_u *ctw_opps_u);
int vm_p2p_noa_start(struct wifi_mac_p2p *p2p, const struct p2p_noa *noa);
unsigned char is_need_process_p2p_action(unsigned char* buf);
int vm_p2p_parse_negotiation_frames(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len, unsigned char tx);
int vm_p2p_client_cancel_noa (struct wifi_mac_p2p *p2p);
void vm_change_p2pie_channel(struct wifi_mac_p2p *p2p , const unsigned char *frm, unsigned int *len);
int vm_p2p_update_noa_ie (struct wifi_mac_p2p *p2p);
void vm_change_p2pie_listenchannel(struct wifi_mac_p2p *p2p , const unsigned char *frm, unsigned int len);
void vm_p2p_scanend(struct wifi_mac_p2p *p2p);
unsigned char *vm_get_wfd_ie(unsigned char *in_ie, unsigned int in_len, unsigned char *wfd_ie, unsigned int *wfd_ielen);
void vm_p2p_cancel_remain_channel(struct wifi_mac_p2p *p2p );
unsigned int vm_p2p_discover_listen(struct wifi_mac_p2p *p2p, int channel, unsigned int duration_ms);
unsigned int vm_wfd_add_probersp_ie(struct wifi_mac_p2p *p2p, unsigned char *frm);
unsigned int vm_wfd_add_probereq_ie(struct wifi_mac_p2p *p2p, unsigned char *frm);
unsigned int vm_wfd_add_assocrsp_ie(struct wifi_mac_p2p *p2p, unsigned char *frm);
unsigned int vm_wfd_add_assocreq_ie(struct wifi_mac_p2p *p2p, unsigned char *frm);
unsigned int vm_wfd_add_beacon_ie(struct wifi_mac_p2p *p2p, unsigned char *frm);
unsigned int vm_wfd_add_ie(struct wlan_net_vif *wnet_vif,const unsigned char *frm, unsigned int len);
int vm_p2p_update_noa_count_start (struct wifi_mac_p2p *p2p);
unsigned char * vm_p2p_get_p2pie_attrib_with_id (void *frm, unsigned char element_id);
unsigned char *vm_p2p_get_p2pie_noa_ie(struct wifi_mac_p2p *p2p ,const unsigned char *frm, unsigned int *len);
int vm_p2p_client_parse_noa_ie (struct wifi_mac_p2p *p2p, const unsigned char *pnoa);
const unsigned char *vm_get_p2p_ie(const unsigned char *in_ie, unsigned int in_len, unsigned char *p2p_ie, unsigned int *p2p_ielen);
unsigned char * vm_p2p_get_p2pie_attrib(const void *frm, unsigned char element_id);
unsigned int vm_p2p_rx_assocreq(struct wifi_mac_p2p *p2p, unsigned char *p2pie, struct wifi_station *sta);
void vm_p2p_scanstart(struct wiphy *wiphy, struct net_device *ndev,struct cfg80211_scan_request *request);
int vm_p2p_initial(struct wifi_mac_p2p *p2p);
int vm_p2p_attach(struct wlan_net_vif *wnet_vif);
void vm_p2p_dettach(void *vm_wdev_priv_ptr);
int vm_p2p_up(struct wlan_net_vif *wnet_vif);
void vm_p2p_down(struct wlan_net_vif *wnet_vif);
int vm_wfd_build_assocbssid_attrib(struct wlan_net_vif *wnet_vif, unsigned char  *wfdie);
int vm_p2p_bsschan_in_chanList(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int len);
void vm_set_p2pie_channelList(struct wifi_mac_p2p *p2p , const unsigned char *frm, unsigned int len,int channel);
void vm_p2p_print_attr(const void *frm);
//void vm_p2p_print_buf(const void *frm, int len);
void set_p2p_negotiation_status(struct wifi_station *sta, enum NET80211_P2P_NEGO_STATE status);
unsigned char is_p2p_negotiation_complete(struct wifi_mac *wifimac);

#endif //IEEE_P2P_H

