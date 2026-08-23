/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer interface function/interface use by driver layer
 *
 *
 ****************************************************************************************
 */

#ifndef _WIFI_MAC_ACTION_H_
#define _WIFI_MAC_ACTION_H_

#include "osdep.h"

#define MAX_ACTION_LEN 384

/* Action category code */
enum aml_ieee80211_category {
    AML_CATEGORY_SPECTRUM_MGMT = 0,
    AML_CATEGORY_QOS = 1,
    AML_CATEGORY_DLS = 2,
    AML_CATEGORY_BACK = 3,
    AML_CATEGORY_PUBLIC = 4, /* IEEE 802.11 public action frames */
    AML_CATEGORY_RADIO_MEAS = 5,
    AML_CATEGORY_FT = 6,
    AML_CATEGORY_HT = 7,
    AML_CATEGORY_SA_QUERY = 8,
    AML_CATEGORY_WNM = 10,
    AML_CATEGORY_UNPROTECTED_WNM = 11, /* add for CONFIG_IEEE80211W, none 11w also can use */
    AML_CATEGORY_TDLS = 12,
    AML_CATEGORY_MESH = 13,
    AML_CATEGORY_MULTIHOP = 14,
    AML_CATEGORY_SELF_PROTECTED = 15,
    AML_CATEGORY_WMM = 17,
    AML_CATEGORY_VHT = 21,
    AML_CATEGORY_P2P = 0x7f,/* P2P action frames */
};

//common header
struct wifi_mac_action
{
    unsigned char ia_category;
    unsigned char ia_action;
} __packed;

struct wifi_mac_action_mgt_args
{
    unsigned char category;
    unsigned char action;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int arg3;
};

//AML_CATEGORY_BACK action field
#define WIFINET_ACTION_BA_ADDBA_REQUEST 0
#define WIFINET_ACTION_BA_ADDBA_RESPONSE 1
#define WIFINET_ACTION_BA_DELBA 2

struct wifi_mac_ba_parameterset
{
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned short buffersize:10,
        tid:4,
        bapolicy:1,
        amsdusupported:1;
#else
    unsigned short amsdusupported:1,
        bapolicy:1,
        tid:4,
        buffersize:10;
#endif
} __packed;

struct wifi_mac_ba_seqctrl
{
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned short startseqnum:12,
        fragnum:4;
#else
    unsigned short fragnum:4,
        startseqnum:12;
#endif
} __packed;

struct wifi_mac_delba_parameterset
{
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned short tid:4,
        initiator:1,
        reserved0:11;
#else
    unsigned short reserved0:11,
        initiator:1,
        tid:4;
#endif
} __packed;

struct wifi_mac_action_ba_addbarequest
{
    struct wifi_mac_action rq_header;
    unsigned char rq_dialogtoken;
    struct wifi_mac_ba_parameterset rq_baparamset;
    unsigned short rq_batimeout;
    struct wifi_mac_ba_seqctrl rq_basequencectrl;
} __packed;

struct wifi_mac_action_ba_addbaresponse
{
    struct wifi_mac_action rs_header;
    unsigned char rs_dialogtoken;
    unsigned short rs_statuscode;
    struct wifi_mac_ba_parameterset rs_baparamset;
    unsigned short rs_batimeout;
} __packed;

struct wifi_mac_action_ba_delba
{
    struct wifi_mac_action dl_header;
    struct wifi_mac_delba_parameterset dl_delbaparamset;
    unsigned short dl_reasoncode;
} __packed;

struct wifi_mac_pub_gas_act_frame
{
    unsigned char   category;
    unsigned char   action;
    unsigned char   dialog_token;
    unsigned char   tag_num;
    unsigned char   tag_len;
    unsigned char   elts[1];
} __packed;


//AML_CATEGORY_PUBLIC action field
enum WIFINET_PUBLIC_ACTION
{
    WIFINET_ACT_PUBLIC_BSSCOEXIST = 0,
    WIFINET_ACT_PUBLIC_CSA = 4,
    WIFINET_ACT_PUBLIC_MP = 7,
    WIFINET_ACT_PUBLIC_P2P = 9,
    WIFINET_ACT_PUBLIC_GAS_REQ = 10,
    WIFINET_ACT_PUBLIC_GAS_RSP = 11,
};

struct wifi_mac_ie_csa
{
    unsigned char chan_switch_mode;
    unsigned char new_operation_class;
    unsigned char new_chan_num;
    unsigned char chan_switch_count;
}__packed;

struct wifi_mac_action_csa_frame
{
    struct wifi_mac_action ac_header;
    struct wifi_mac_ie_csa csa;
} __packed;

#define OUI_TYPE_P2P            0x09
#define OUI_TYPE_DPP            0x1a


struct wifi_mac_ie_bss_coex
{
    unsigned char elem_id;
    unsigned char elem_len;
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned char reserved1:1,
        reserved2:1,
        reserved3:1,
        obss_exempt_grant:1,
        obss_exempt_req:1,
        ht20_width_req:1,
        ht40_intolerant:1,
        inf_request:1;
#else
    unsigned char inf_request:1,
        ht40_intolerant:1,
        ht20_width_req:1,
        obss_exempt_req:1,
        obss_exempt_grant:1,
        reserved3:1,
        reserved2:1,
        reserved1:1;
#endif
} __packed;

struct wifi_mac_ie_intolerant_report
{
    unsigned char elem_id;
    unsigned char elem_len;
    unsigned char reg_class;
    unsigned char chan_list[0];
} __packed;

struct wifi_mac_action_bss_coex_frame
{
    struct wifi_mac_action ac_header;
    struct wifi_mac_ie_bss_coex coex;
    struct wifi_mac_ie_intolerant_report chan_report;
} __packed;

//AML_CATEGORY_HT  action field
#define WIFINET_ACTION_HT_TXCHWIDTH 0
#define WIFINET_ACTION_HT_SMPOWERSAVE 1

struct wifi_mac_action_ht_txchwidth
{
    struct wifi_mac_action at_header;
    unsigned char at_chwidth;
} __packed;

struct wifi_mac_action_ht_smpowersave
{
    struct wifi_mac_action as_header;
    unsigned char as_control;
} __packed;

//AML_CATEGORY_SA_QUERY action field
#define WIFINET_ACTION_SA_QUERY_REQ 0
#define WIFINET_ACTION_SA_QUERY_RSP 1

struct wifi_mac_action_sa_query
{
    struct wifi_mac_action sa_header;
    unsigned short sa_transaction_identifier;
} __packed;

//AML_CATEGORY_WMM  action field
enum WIFINET_WMM_ACTION
{
    WIFINET_ACTION_ADDTS_REQ= 0,
    WIFINET_ACTION_ADDTS_RESP = 1,
    WIFINET_ACTION_DELTS = 2,
};

struct wifi_mac_wmm_tspec_element {
    unsigned char eid; /* 221 = 0xdd */
    unsigned char length; /* 6 + 55 = 61 */
    unsigned char oui[3]; /* 00:50:f2 */
    unsigned char oui_type; /* 2 */
    unsigned char oui_subtype; /* 2 */
    unsigned char version; /* 1 */

    /* WMM TSPEC body (55 octets): */
    unsigned char ts_info[3];
    unsigned short nominal_msdu_size;
    unsigned short maximum_msdu_size;
    unsigned int minimum_service_interval;
    unsigned int maximum_service_interval;
    unsigned int inactivity_interval;
    unsigned int suspension_interval;
    unsigned int service_start_time;
    unsigned int minimum_data_rate;
    unsigned int mean_data_rate;
    unsigned int peak_data_rate;
    unsigned int maximum_burst_size;
    unsigned int delay_bound;
    unsigned int minimum_phy_rate;
    unsigned short surplus_bandwidth_allowance;
    unsigned short medium_time;
}__packed;

struct wifi_mac_action_addts_req_frame {
    struct wifi_mac_action ac_header;
    unsigned char dialog_token;
    unsigned char status_code;
    struct wifi_mac_wmm_tspec_element wmm_tspec_element;
};

struct wifi_mac_action_delts_req_frame {
    struct wifi_mac_action ac_header;
    unsigned char dialog_token;
    unsigned char status_code;
    struct wifi_mac_wmm_tspec_element wmm_tspec_element;
};

//AML_CATEGORY_VHT  action field
enum WIFINET_VHT_ACTION
{
    WIFINET_ACTION_VHT_CMPRSSD_BMFRM = 0,
    WIFINET_ACTION_VHT_GROUP_ID = 1,
    WIFINET_ACTION_VHT_OPERATE_MODE_NOTIFICATION = 2,
};

struct wifi_mac_ie_op_mode {
#if _BYTE_ORDER == _BIG_ENDIAN
    unsigned char rx_nss_type:1,
        rx_nss:3,
        reserved:2,
        ch_width:2;
#else
    unsigned char ch_width:2,
        reserved:2,
        rx_nss:3,
        rx_nss_type:1;
#endif
} __packed;

struct wifi_mac_ie_op_mode_ntfy {
    unsigned char elem_id;
    unsigned char elem_len;
    struct wifi_mac_ie_op_mode opmode;
} __packed;


/* VHT - recommended Channel width and Nss */
struct wifi_mac_action_vht_opmode {
    struct wifi_mac_action at_header;
    struct wifi_mac_ie_op_mode at_op_mode;
} __packed;

//802.11w
#define CATEGORY_IS_NON_ROBUST(cat) \
    (cat == AML_CATEGORY_PUBLIC \
    || cat == AML_CATEGORY_HT \
    || cat == AML_CATEGORY_UNPROTECTED_WNM \
    || cat == AML_CATEGORY_SELF_PROTECTED \
    || cat == AML_CATEGORY_VHT \
    || cat == AML_CATEGORY_P2P)

#define CATEGORY_IS_ROBUST(cat) !CATEGORY_IS_NON_ROBUST(cat)

int wifi_mac_send_addba_req(char *buf, int tid);
int wifi_mac_send_coexist_mgmt(const char* buf);
int wifi_mac_send_wmm_ac_addts(char** buf);
int wifi_mac_send_wmm_ac_delts(const char* buf);

#endif//_WIFI_MAC_ACTION_H_
