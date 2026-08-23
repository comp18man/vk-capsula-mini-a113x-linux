/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac android interface module
 *
 *
 ****************************************************************************************
 */
#ifndef _IEEE80211_ANDROID_H_
#define _IEEE80211_ANDROID_H_

struct android_wifi_priv_cmd
{
    char *buf;
    int used_len;
    int total_len;
} ;

#ifdef CONFIG_COMPAT
    typedef struct _compat_android_wifi_priv_cmd {
        compat_caddr_t buf;
        int used_len;
        int total_len;
    } compat_android_wifi_priv_cmd;
#endif /* CONFIG_COMPAT */

enum ANDROID_WIFI_CMD
{
    ANDROID_WIFI_CMD_START = 0,
    ANDROID_WIFI_CMD_STOP,
    ANDROID_WIFI_CMD_SCAN_ACTIVE,
    ANDROID_WIFI_CMD_SCAN_PASSIVE,
    ANDROID_WIFI_CMD_RSSI,
    ANDROID_WIFI_CMD_LINKSPEED = 5,
    ANDROID_WIFI_CMD_RXFILTER_START,
    ANDROID_WIFI_CMD_RXFILTER_STOP,
    ANDROID_WIFI_CMD_RXFILTER_ADD,
    ANDROID_WIFI_CMD_RXFILTER_REMOVE,
    ANDROID_WIFI_CMD_BTCOEXSCAN_START = 10,
    ANDROID_WIFI_CMD_BTCOEXSCAN_STOP,
    ANDROID_WIFI_CMD_BTCOEXMODE,
    ANDROID_WIFI_CMD_SETSUSPENDOPT,
    ANDROID_WIFI_CMD_SETFWPATH,
    ANDROID_WIFI_CMD_SETBAND = 15,
    ANDROID_WIFI_CMD_GETBAND,
    ANDROID_WIFI_CMD_COUNTRY,
    ANDROID_WIFI_CMD_P2P_SET_NOA,
    ANDROID_WIFI_CMD_P2P_GET_NOA,
    ANDROID_WIFI_CMD_P2P_SET_PS = 20,
    ANDROID_WIFI_CMD_SET_AP_WPS_P2P_IE,
#ifdef PNO_SUPPORT
    ANDROID_WIFI_CMD_PNOSSIDCLR_SET,
    ANDROID_WIFI_CMD_PNOSETUP_SET,
    ANDROID_WIFI_CMD_PNOENABLE_SET,
    ANDROID_WIFI_CMD_PNODEBUG_SET,
#endif

    ANDROID_WIFI_CMD_BLOCK,

    ANDROID_WIFI_CMD_WFD_ENABLE,
    ANDROID_WIFI_CMD_WFD_DISABLE,

    ANDROID_WIFI_CMD_WFD_SET_TCPPORT,
    ANDROID_WIFI_CMD_WFD_SET_MAX_TPUT,
    ANDROID_WIFI_CMD_WFD_SET_DEVTYPE,
    ANDROID_WIFI_CMD_FW_REPAIR,

    ANDROID_WIFI_CMD_MAX
};


int aml_android_priv_cmd(struct wlan_net_vif *wnet_vif, void __user *data, int cmd);
#endif //_IEEE80211_ANDROID_H_
