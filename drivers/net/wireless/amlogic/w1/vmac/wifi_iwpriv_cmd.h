#ifndef __WIFI_IWPRIV_CMD_H
#define __WIFI_IWPRIV_CMD_H
#include <net/iw_handler.h>

extern struct iw_handler_def w1_iw_handle;

enum aml_iwpriv_subcmd
{
    AML_IWP_ADDBA_REQ = 1,
    AML_IWP_AMSDU_STATE = 2,
    AML_IWP_AMPDU_STATE = 3,
    AML_IWP_UDP_INFO = 4,
    AML_IWP_COUNTRY = 5,
    AML_IWP_11H = 6,
    AML_IWP_ARP_RX = 7,
    AML_IWP_CHAN_LIST = 8,
    AML_IWP_ROAM_THRESH_2G = 9,
    AML_IWP_ROAM_THRESH_5G = 10,
    AML_IWP_ROAM_MODE = 11,
    AML_IWP_MARK_DFS_CHAN = 12,
    AML_IWP_UNMARK_DFS_CHAN = 13,
    AML_IWP_WEAK_THR_NARROW = 14,
    AML_IWP_WEAK_THR_WIDE = 15,
    AML_IWP_EAT_COUNT = 16,
    AML_IWP_AGGR_THRESH = 17,
    AML_IWP_HEART_INTERVAL = 18,
    AML_IWP_CHIP_ID = 19,
    AML_IWP_WIFI_MAC = 20,
    AML_IWP_AP_IP = 21,
    AML_IWP_ROAM_CHAN = 22,
    AML_IWP_BSS_COEX = 23,
    AML_IWP_TXAGGR_STA = 24,
    AML_IWP_SESSION = 25,
    AML_IWP_P2P_DEV_ID = 26,
    AML_IWP_WMM_AC_DELTS = 27,
    AML_IWP_SHORT_GI = 28,
    AML_IWP_DYNAMIC_BW = 29,
    AML_IWP_CCA_BUSY_CHECK = 30,
    AML_IWP_PHY_STATISTIC = 31,
    AML_IWP_PRINT_VERSION = 32,
    AML_IWP_SET_REG = 33,
    AML_IWP_GET_REG = 34,
    AML_IWP_GET_TX_STATUS = 35,
    AML_IWP_ENABLE_FW_LOG = 36,
    AML_IWP_LEGACY_SET_REG = 37,
    AML_IWP_LEGACY_GET_REG = 38,
    AML_IWP_SET_BCN_INTERVAL = 39,
    AML_IWP_SET_LDPC = 40,
    AML_IWP_SET_BEAMFORMING = 41,
    AML_IWP_SET_SCAN_TIME_IDLE = 42,
    AML_IWP_SET_SCAN_TIME_CONNECT = 43,
    AML_IWP_SET_SCAN_HANG = 44,
    AML_IWP_EN_BTWIFI_COEX = 45,
    AML_IWP_SET_COEXIST_MAX_MISS_BCN_CNT = 46,
    AML_IWP_SET_COEXIST_REQ_TIMEOUT = 47,
    AML_IWP_SET_COEXIST_NOT_GRANT_WEIGHT = 48,
    AML_IWP_SET_RATE_LEGACY = 49,
    AML_IWP_SET_RATE_HT = 50,
    AML_IWP_SET_RATE_VHT = 51,
    AML_IWP_SET_RATE_AUTO = 52,
    AML_IWP_SET_POWER = 53,
    AML_IWP_SET_CHL_RSSI = 54,
    AML_IWP_SET_BURST = 55,
    AML_IWP_SET_UAPSD = 56,
    AML_IWP_SET_PT_RX_START = 57,
    AML_IWP_SET_PT_RX_STOP = 58,
    AML_IWP_SET_SCAN_PRI = 59,
    AML_IWP_SET_BE_BK_NOQOS_PRI = 60,
    AML_IWP_SET_FETCH_PKT_METHOD = 61,
    AML_IWP_SET_FRAG_THRESHOLD = 62,
    AML_IWP_SET_PREAMBLE_TYPE = 63,
    AML_IWP_SET_FIX_BAND = 64,
    AML_IWP_SET_GAIN = 65,
    AML_IWP_SET_TPC = 66,
    AML_IWP_SET_DEV_SN = 67,
    AML_IWP_GET_DEV_SN = 68,
    AML_IWP_SET_WIFI_MAC = 69,
    AML_IWP_GET_WIFI_MAC = 70,
    AML_IWP_SET_BT_MAC = 71,
    AML_IWP_GET_BT_MAC = 72,
    AML_IWP_SET_TXPW_PLAN = 73,
    AML_IWP_SET_MAC_MODE = 74,
};

extern unsigned char aml_iwpriv_get_band(void);
int aml_set_debug_modules(char *debug_str);

unsigned int get_latest_tx_status(struct wifi_mac *wifimac);
int aml_beacon_intvl_set(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_set_ldpc(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_iwpriv_set_lagecy_bitrate_mask(struct net_device *dev, unsigned int set);
int aml_iwpriv_set_ht_bitrate_mask(struct net_device *dev, unsigned int set);
void aml_iwpriv_set_rate_auto(struct wlan_net_vif *wnet_vif);
void aml_iwpriv_set_uapsd(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_iwpriv_set_vht_bitrate_mask(struct net_device *dev, unsigned int set);
unsigned char aml_iwpriv_set_band(unsigned int set);
unsigned int get_reg(struct wlan_net_vif *wnet_vif, unsigned int set);
unsigned int set_reg(struct wlan_net_vif *wnet_vif, unsigned int set1, unsigned int set2);

#endif
