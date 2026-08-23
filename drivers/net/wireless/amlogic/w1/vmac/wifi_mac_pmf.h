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

#ifndef _WIFI_MAC_PMF_H_
#define _WIFI_MAC_PMF_H_

#include "wifi_mac_action.h"
#include "wifi_mac_encrypt.h"

#define WEP_KEYS 4
//#define PMF_PKT_PRINT 1

#define SET_ICE_IV_LEN(iv_len, icv_len, encrypt)\
    do {\
        switch (encrypt) {\
            case WIFINET_CIPHER_WEP:\
                *iv_len = 4;\
                *icv_len = 4;\
                break;\
            case WIFINET_CIPHER_TKIP:\
                *iv_len = 8;\
                *icv_len = 4;\
                break;\
            case WIFINET_CIPHER_AES_OCB:\
            case WIFINET_CIPHER_AES_CCM:\
            case WIFINET_CIPHER_AES_CMAC:\
                *iv_len = 8;\
                *icv_len = 8;\
                break;\
            default:\
                *iv_len = 0;\
                *icv_len = 0;\
                break;\
        } \
    } while (0)

struct wifi_mac_ie_mmic
{
    unsigned char id;
    unsigned char len;
    unsigned short key_id;
    unsigned char ipn[6];
    unsigned char mic[16];
} __packed;

#define MME_IE_ID 76
#define BIP_MAX_KEYID 5
#define BIP_AAD_SIZE  20
#define MME_IE_LENGTH 26
#define MME_IE_LENGTH_CMAC 18

#define MFP_MASK 0xc0
#define MFP_ENABLED 0x80
#define MFP_REQUIRED 0x40

#define clear_retry(pbuf) \
    do { \
        *(unsigned short *)(pbuf) &= (~cpu_to_le16(BIT(11))); \
    } while (0)

#define clear_pwr_mgt(pbuf) \
    do { \
        *(unsigned short *)(pbuf) &= (~cpu_to_le16(BIT(12))); \
    } while (0)

#define clear_more_data(pbuf) \
    do { \
        *(unsigned short *)(pbuf) &= (~cpu_to_le16(BIT(13))); \
    } while (0)

int wifi_mac_send_sa_query(struct wifi_station *sta, unsigned char action, unsigned short seq);
int wifi_mac_parse_mmie(struct wifi_station *sta, struct sk_buff *skb, unsigned char subtype);
unsigned char is_robust_management_frame(struct wifi_frame *wh);
int wifi_mac_add_mmie(struct wifi_station *sta, struct sk_buff *skb);
int wifi_mac_sw_encrypt(struct wifi_station *sta, struct sk_buff *skb);
void wifi_mac_disable_hw_mgmt_decrypt(void);
int wifi_mac_unprotected_mgmt_pkt_handle(struct wifi_station *sta, struct sk_buff *skb, unsigned char subtype);
int wifi_mac_mgmt_validate(struct wifi_station *sta, struct sk_buff *skb, unsigned char subtype);

#endif//_WIFI_MAC_PMF_H_
