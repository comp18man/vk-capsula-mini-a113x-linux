/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer nl80211 iocontrol module
 *
 *
 ****************************************************************************************
 */

#ifndef __WIFI_CFG80211_H__
#define __WIFI_CFG80211_H__

#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <net/rtnetlink.h>
#include "wifi_mac_p2p.h"

#define AMLOGIC_VENDOR_ID 0x8899
#define AML_SCAN_IE_LEN_MAX sizeof(struct wifi_scan_info)
#define AML_MAX_NUM_PMKIDS 4
#define AML_MAX_REMAIN_ON_CHANNEL_DURATION 3000

#define AML_A_RATES_NUM 8
#define AML_G_RATES_NUM 12

#define CFG_CH_MAX_2G_CHANNEL 14
#define AML_MAX_NUM_BANDS 2

#define AML_2G_CHANNELS_NUM 14
#define AML_5G_CHANNELS_NUM 25


extern struct ieee80211_channel aml_2ghz_channels[AML_2G_CHANNELS_NUM];
extern struct ieee80211_channel aml_5ghz_channels[AML_5G_CHANNELS_NUM];
extern struct ieee80211_rate aml_rates[AML_G_RATES_NUM];

#define aml_a_rates (aml_rates + 4)
#define aml_g_rates (aml_rates + 0)
#define BE_MAP 0x0009
#define BK_MAP 0x0006
#define VI_MAP 0x0030
#define VO_MAP 0x00C0

#define MAX_BIT_RATE_40MHZ_MCS7     150
#define CANCLE_STEP_MS  10

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
#define STATION_INFO_INACTIVE_TIME  BIT(NL80211_STA_INFO_INACTIVE_TIME)
#define STATION_INFO_TX_BYTES       BIT(NL80211_STA_INFO_TX_BYTES)
#define STATION_INFO_LLID           BIT(NL80211_STA_INFO_LLID)
#define STATION_INFO_PLID           BIT(NL80211_STA_INFO_PLID)
#define STATION_INFO_PLINK_STATE    BIT(NL80211_STA_INFO_PLINK_STATE)
#define STATION_INFO_SIGNAL         BIT(NL80211_STA_INFO_SIGNAL)
#define STATION_INFO_TX_BITRATE     BIT(NL80211_STA_INFO_TX_BITRATE)
#define STATION_INFO_RX_BITRATE     BIT(NL80211_STA_INFO_RX_BITRATE)
#define STATION_INFO_RX_PACKETS     BIT(NL80211_STA_INFO_RX_PACKETS)
#define STATION_INFO_TX_PACKETS     BIT(NL80211_STA_INFO_TX_PACKETS)
#define STATION_INFO_TX_FAILED      BIT(NL80211_STA_INFO_TX_FAILED)
#define STATION_INFO_BSS_PARAM      BIT(NL80211_STA_INFO_BSS_PARAM)
#define STATION_INFO_LOCAL_PM       BIT(NL80211_STA_INFO_LOCAL_PM)
#define STATION_INFO_PEER_PM        BIT(NL80211_STA_INFO_PEER_PM)
#define STATION_INFO_NONPEER_PM     BIT(NL80211_STA_INFO_NONPEER_PM)
#define STATION_INFO_ASSOC_REQ_IES  0
#endif /* Linux kernel >= 4.0.0 */

//allign issue please pay attention to
 struct _iwVendorCmdStruct
{
    unsigned int first_eight_byte; // base addr
    unsigned int second_eight_byte; // mac write pointer
    unsigned char last_byte; // fw read pointer
} ;

/**vendor sub command*/
enum vendor_sub_command {
    sub_cmd_set_drvdbg = 0,
    sub_cmd_max,
};

/**vendor event*/
enum vendor_event {
    event_hang = 0,
    event_reg_value =1,
    event_max,
};

enum vm_vendor_command_attr{
    VM_NL80211_VENDER_CMD_ID_NONE,
    VM_NL80211_VENDER_SUBCMD_AMSDU=0x01,
    VM_NL80211_VENDER_SUBCMD_AMPDU=0x02,
    VM_NL80211_VENDER_SUBCMD_B2BNCTYPE=0x03,
    VM_NL80211_VENDER_SUBCMD_B2BTKIPMIC=0x04,
    VM_NL80211_VENDER_SUBCMD_B2BTCPCSUM=0x05,
    VM_NL80211_VENDER_SUBCMD_B2BPKTSNUM=0x06,
    VM_NL80211_VENDER_SUBCMD_B2BCHLBW=0x07,
    VM_NL80211_VENDER_SUBCMD_B2BTESTYPE=0x08,
    VM_NL80211_VENDER_SUBCMD_GETREG=0x09,
    VM_NL80211_VENDER_SUBCMD_SETREG=0x0a,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_LOCAL_MAC_ADDR=0x0c,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_GET_LOCAL_MAC_ADDR=0x0d,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_PEER_MAC_ADDR=0x0e,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_GET_PEER_MAC_ADDR=0x0f,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_BSSID_MAC_ADDR=0x10,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_GET_BSSID_MAC_ADDR=0x11,
    VM_NL80211_VENDER_SUBCMD_DRV_SET_SHORT_GI=0x13,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_CHL=0x14,
    VM_NL80211_VENDER_SUBCMD_DRV_B2B_PKT_LENGTH=0x15,
#ifdef WIFI_CAPTURE
    VM_NL80211_VDR_SUBCMD_START_CAPTURE=0x16,
    VM_NL80211_VDR_SUBCMD_STOP_CAPTURE=0x17,
#endif
    VM_NE80211_VDR_SUBCMD_TEST_SDIO=0x18,
    VM_NL80211_VENDER_I2C_GETREG=0x19,
    VM_NL80211_VENDER_I2C_SETREG=0x20,

    VM_NL80211_PHY_STATISTIC=0x28,
    VM_NL80211_CCA_BUSY_CHECK=0x2a,

    VM_NL80211_VENDOR_UPDATE_WIPHY_PARAMS=0x31,
    VM_NL80211_VENDOR_SET_PREAMBLE_TYPE=0x32,
    VM_NL80211_VENDOR_SET_BURST=0x33,
    VM_NL80211_VENDOR_SET_ACK_POLICY=0x34,

    VM_NL80211_SENDTEST=0x35,
    VM_NL80211_SET_LDPC = 0x36,
    VM_NL80211_VENDOR_ENABLE_AUTO_RATE = 0x38,
    VM_NL80211_BT_REG_READ = 0x39,
    VM_NL80211_BT_REG_WRITE = 0x3a,

    VM_NL80211_VENDER_DBG_INFO_ENABLE=0x40,
    VM_NL80211_VENDER_BEAMFORMING=0x41,
    VM_NL80211_VENDER_DYNAMIC_BW_CFG=0x42,

    VM_NL80211_VENDER_BCN_INTERVAL=0x43,
    VM_NL80211_VENDOR_GET_STA_RSSI_NOISE=0x44,
    VM_NL80211_T9026_DUMP_RXIRR_REG = 0x54,
#ifdef WIFI_CAPTURE
    VM_NL80211_SET_CAP_GAIN = 0x55,
    VM_NL80211_SET_BCAP_NAME = 0x56,
#endif
    VM_NL80211_SET_EN_COEX = 0x57,

    VM_NL80211_SET_COEXIST_MAX_MISS_BCN_CNT = 0x58,

    VM_NL80211_SET_COEXIST_REQ_TIMEOUT  = 0x59,
    VM_NL80211_SET_COEXIST_NOT_GRANT_WEIGHT  = 0x5a,
    VM_NL80211_SET_COEXIST_CONFIG = 0x5b,
    VM_NL80211_SET_COEXIST_MAX_NOT_GRANT_CNT= 0x5c,
    VM_NL80211_SET_COEXIST_IRQ_END_TIM  = 0x5d,
    VM_NL80211_SET_COEXIST_SCAN_PRI_RANGE = 0x5e,
    VM_NL80211_SET_COEXIST_BE_BK_NOQOS_PRI_RANGE = 0x5f,

    VM_NL80211_VENDER_SYS_INFO_STATISTIC_DEFAULT_CFG=0x96,
    VM_NL80211_VENDER_SYS_INFO_STATISTIC_UPDATE_CFG=0x97,
    VM_NL80211_VENDER_SYS_INFO_STATISTIC_OPT=0x98,
    /*a special code 0x99 for the vip function*/
    VM_NL80211_VENDER_STS_BY_LAYER=0x99,

    VM_NL80211_VENDER_PT_RX_START = 0x9a,
    VM_NL80211_VENDER_PT_RX_STOP = 0x9b,
    VM_NL80211_POWER_SAVE_INTERVAL = 0x9c,
    VM_NL80211_FETCH_PKT_METHOD = 0x9d,
    VM_NL80211_PKT_FRAGMENT_THRESHOLD = 0x9e,
    VM_NL80211_UAPSD_ENABLE = 0x9f,
    VM_NL80211_CLK_MEASURE = 0xa0,
    VM_NL80211_AUTORATE_ENABLE = 0xa1,
    VM_NL80211_SDIO_DLY_ADJUST = 0xa2,

#ifdef WIFI_CAPTURE
    VM_NL80211_VDR_SUBCMD_BT_START_CAPTURE=0xa3,
    VM_NL80211_VDR_SUBCMD_BT_STOP_CAPTURE=0xa4,
#endif
    VM_NL80211_SCAN_TIME_IDLE = 0xa5,
    VM_NL80211_SCAN_TIME_CONNECT = 0xa6,
    VM_NL80211_SCAN_HANG = 0xa7,
    VM_NL80211_PRINT_VERSION = 0xa8,
    VM_NL80211_GET_EFUSE_DATA = 0xaa,
    VM_NL80211_GET_FW_LOG = 0xab,

    VM_NL80211_VENDER_CMD_ID_MAX,
};

#pragma pack(1)
struct vendor_reg_cmd
	{
		char cmd;
		unsigned int addr;
		unsigned int data;
	};

struct vendor_com_cmd
	{
		char cmd;
		unsigned int data;
	};

struct vendor_chn_bw_cmd
{
    	char cmd;
       char chn;
       char bw;
};

struct vendor_sptr_snr_cmd
{
     char cmd;
     char chn;
     char bw;
};

union vendor_if
{
    char buf[0];
    struct vendor_reg_cmd vnd_reg_cmd;
    struct vendor_com_cmd vnd_com_cmd;
    struct vendor_chn_bw_cmd vnd_chn_bw_cmd;
    struct vendor_sptr_snr_cmd vendor_sptr_snr_cmd;
};
#pragma pack()

#define TYPE_COMMON                   1
#define TYPE_AMPDU                    2
#define TYPE_AMSDU                    3
#define TYPE_AMSDU_AMPDU      	      4
#define TYPE_COMMON_NOACK    	      5
#define TYPE_BURST_ACK                6
#define TYPE_BURST_BA                 7
#define TYPE_STOP_TX                  8

#define MAC_FMT "MAC_ADDR=%02x:%02x:%02x:%02x:%02x:%02x\n"
#define MAC_ARG(x) ((unsigned char*)(x))[0],((unsigned char*)(x))[1],\
                    ((unsigned char*)(x))[2],((unsigned char*)(x))[3],\
                    ((unsigned char*)(x))[4],((unsigned char*)(x))[5]

#define RATETAB_ENT(_rate, _rateid, _flags) \
 {                              \
                .bitrate    = (_rate),              \
                 .hw_value  = (_rateid),                \
                 .flags     = (_flags),             \
 }

#define CHAN2G(_channel, _freq, _flags) {           \
                .band           = (enum nl80211_band)IEEE80211_BAND_2GHZ,      \
                .center_freq        = (_freq),          \
                .hw_value       = (_channel),           \
                .flags          = (_flags),         \
                .max_antenna_gain   = 0,                \
                .max_power      = 30,               \
}

#define CHAN5G(_channel, _flags) {              \
                .band           = (enum nl80211_band)IEEE80211_BAND_5GHZ,      \
                .center_freq        = 5000 + (5 * (_channel)),  \
                .hw_value       = (_channel),           \
                .flags          = (_flags),         \
                 .max_antenna_gain  = 0,                \
                 .max_power     = 30,               \
 }

#define CFG80211_CONNECT_TIMER_OUT (20*1000)
struct vm_wdev_priv
{
    struct wireless_dev *vm_wdev;
    struct wlan_net_vif *wnet_vif;

    struct cfg80211_scan_request *scan_request;
    spinlock_t scan_req_lock;

    struct cfg80211_connect_params *connect_request;
    spinlock_t connect_req_lock;
    unsigned long connect_req_lock_flags;
    struct os_timer_ext connect_timeout;

    struct net_device *pmon_ndev;
    struct net_device *pGo_ndev;
    struct wireless_dev *pGo_wdev;

    char ifname_go[IFNAMSIZ + 1];
    char ifname_mon[IFNAMSIZ + 1];
    unsigned char block;
    int send_action_id;
#ifdef CONFIG_P2P
    struct wifi_mac_p2p p2p;
#endif //CONFIG_P2P
};

struct vm_netdev_priv_indicator
{
    void *priv;
    unsigned int sizeof_priv;
};
#define PM_OFF  0
#define PM_MAX  1

#define TU_DURATION 1024
#define wdev_to_priv(w) ((struct vm_wdev_priv *)(wdev_priv(w)))
#define wdev_to_p2p(w) (&(((struct vm_wdev_priv *)(wdev_priv(w)))->p2p))
#define wdev_to_ndev(w) ((w)->netdev)
#define wiphy_to_priv(w) ((struct vm_wdev_priv *)(wiphy_priv(w)))
#define wiphy_to_adapter(x) (struct wlan_net_vif *)(((struct vm_wdev_priv*)wiphy_priv(x))->wnet_vif)
#ifdef CONFIG_P2P
#define wiphy_to_p2p(x) (struct wifi_mac_p2p *)(&(((struct vm_wdev_priv*)wiphy_priv(x))->p2p))
#endif
#define wiphy_to_wdev(x) (struct wireless_dev *)(((struct vm_wdev_priv*)wiphy_priv(x))->vm_wdev)

enum
{
    WIFINET_MACCMD_POLICY_OPEN  = 0,
    WIFINET_MACCMD_POLICY_ALLOW = 1,
    WIFINET_MACCMD_POLICY_DENY  = 2,
    WIFINET_MACCMD_FLUSH        = 3,
    WIFINET_MACCMD_DETACH       = 4,
};//yishu

struct wifi_macreq_key
{
    unsigned char ik_type;
    unsigned char ik_pad;
    unsigned short ik_keyix;
    unsigned char ik_keylen;
    unsigned char ik_flags;
#define WIFINET_KEY_DEFAULT 0x80
    unsigned char ik_macaddr[WIFINET_ADDR_LEN];
    unsigned char ik_keyrsc[WIFINET_TID_SIZE];
    u_int64_t ik_keytsc;
    unsigned char ik_keydata[WIFINET_KEYBUF_SIZE + WIFINET_MICBUF_SIZE];
};//yishu

//yishu
#define WIFINET_MAX_OPT_IE  256
#define WIFINET_MAX_WDF_IE  256


int vm_cfg80211_vnd_cmd_set_para(struct wiphy *wiphy, struct wireless_dev *wdev,const void *data, int data_len);
struct device *wl_cfg80211_get_parent_dev(void);
void vm_cfg80211_set_parent_dev(void *dev);
void vm_cfg80211_clear_parent_dev(void);

int nl80211_iftype_2_drv_opmode (enum nl80211_iftype NL80211_IFTYPE);
int wait_for_ap_run (struct wlan_net_vif *wnet_vif, int total_ms, int step_ms);

int wifi_mac_alloc_wdev(struct wlan_net_vif *wnet_vif, struct device *dev);
void vm_wdev_free(struct wireless_dev *wdev, unsigned char vid);
int vm_cfg80211_up(struct wlan_net_vif *wnet_vif);
void vm_cfg80211_down(struct wlan_net_vif *wnet_vif);

void vm_cfg80211_indicate_scan_done(struct vm_wdev_priv *pwdev_priv, unsigned char aborted);
void vm_cfg80211_indicate_connect(struct wlan_net_vif *wnet_vif);
void vm_cfg80211_indicate_disconnect(struct wlan_net_vif *wnet_vif);
int vm_cfg80211_inform_bss (struct wlan_net_vif *wnet_vif);
int vm_cfg80211_notify_mgmt_rx(struct wlan_net_vif *wnet_vif,  unsigned short channel, void *data,int len);
int vm_p2p_set_wpsp2pie(struct net_device *net, char *buf, int len, int type);
int vm_p2p_update_beacon_app_ie (struct wlan_net_vif *wnet_vif);
int vm_p2p_set_p2p_noa(struct net_device *dev, char* buf, int len);
int vm_p2p_set_p2p_ps(struct net_device *dev, char* buf, int len);

int translate_to_dbm(int rssi);
struct device *vm_cfg80211_get_parent_dev(void);
void vm_cfg80211_indicate_sta_assoc(const struct wifi_station *sta);
void vm_cfg80211_indicate_sta_disassoc(const struct wifi_station *sta, unsigned short reason);
int vm_cfg80211_send_mgmt(struct wlan_net_vif *wnet_vif,const unsigned char * buf,int len);

int netdev_setcsum( struct net_device *dev,int data);
int wifi_mac_preempt_scan( struct wifi_mac *wifimac, int max_grace, int max_wait);
char preempt_scan(struct net_device *dev, int max_grace, int max_wait);
void vm_wlan_net_vif_mode_change(struct wlan_net_vif *wnet_vif, struct vm_wlan_net_vif_params *cp);
int wifi_mac_rm_app_ie(struct wifi_mac_app_ie_t  * app_ie);
int wifi_mac_save_app_ie(struct wifi_mac_app_ie_t  * app_ie, const unsigned char *app_buf,int app_buflen);
int cipher2cap(int cipher);
void batch_dump_reg(struct wiphy *wiphy,unsigned int addr[], unsigned int addr_num);
void wifi_softap_allsta_stopping(struct wlan_net_vif *wnet_vif, unsigned char is_disconnecting);
int softap_get_sta_num(struct wlan_net_vif *wnet_vif);
void vm_cfg80211_chan_switch_notify_task(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
int vm_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
    struct net_device *dev,
    const unsigned char *peer,
    const struct cfg80211_bitrate_mask *mask);
#else
int vm_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
    struct net_device *dev,
    unsigned int link_id,
    const unsigned char *peer,
    const struct cfg80211_bitrate_mask *mask);
#endif
#endif

