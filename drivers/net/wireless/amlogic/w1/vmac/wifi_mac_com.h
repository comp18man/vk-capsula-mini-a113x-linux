/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer common need include *.h
 *
 *
 ****************************************************************************************
 */


#ifndef _NET80211_COMMON_H_
#define _NET80211_COMMON_H_


#include <linux/version.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <net/ieee80211_radiotap.h>
#include <linux/wireless.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/if_vlan.h>
#include <linux/time.h>
#include <net/iw_handler.h>
#include <linux/init.h>
#include <linux/sysctl.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>
#include <net/inet_ecn.h>

#include "wifi_drv_config.h"
#include "wifi_debug.h"
#include "wifi_hal_com.h"
#include "wifi_mac.h"
#include "wifi_mac_encrypt.h"
#include "wifi_mac_sta.h"
#include "wifi_mac_pmf.h"
#include "wifi_mac_action.h"
#include "wifi_mac_power.h"
#include "wifi_mac_scan.h"
#include "wifi_mac_var.h"
#include "wifi_mac_p2p.h"
#include "wifi_mac_xmit.h"
#include "wifi_mac_recv.h"
#include "wifi_drv_recv.h"
#include "wifi_mac_if.h"
#include "wifi_mac_acl.h"
#include "wifi_mac_amsdu.h"
#include "wifi_cfg80211.h"
#include "wifi_mac_monitor.h"
#include "wifi_mac_rate.h"
#include "wifi_mac_chan.h"
#include "wifi_android.h"
#include "wifi_rate_ctrl.h"
#include "rc80211_minstrel_init.h"
#include "wifi_hal_txdesc.h"
#include "wifi_mac_arp.h"

#include "wifi_hal.h"
#include "wifi_mac_beacon.h"
#include "wifi_mac_concurrent.h"
#include "wifi_pkt_desc.h"

extern unsigned long long g_dbg_info_enable;
extern const char *wifi_mac_state_name[WIFINET_S_MAX];

#define WIFINET_DEBUG_LEVEL (AML_DEBUG_STATE|AML_DEBUG_P2P|AML_DEBUG_CFG80211|AML_DEBUG_WARNING)

#define WIFINET_DPRINTF(_m, _fmt, ...) do {         \
                if (wnet_vif->vm_debug & (_m))                    \
                        pr_debug("<%s> %s %d  "_fmt"\n", wnet_vif->vm_ndev->name ,__func__, __LINE__, __VA_ARGS__);       \
        } while (0)
#define WIFINET_DPRINTF_MACADDR( _m, _mac, _fmt, ...) do {      \
                if (wnet_vif->vm_debug & (_m))                    \
                        pr_debug("<%s> %s %d mac[%s] "_fmt"\n",wnet_vif->vm_ndev->name ,  __func__, __LINE__,ether_sprintf(_mac),__VA_ARGS__);  \
        } while (0)
#define WIFINET_DPRINTF_STA( _m, _sta, _fmt, ...)do {           \
                if ((_sta)->sta_wnet_vif->vm_debug & (_m))                    \
                        pr_debug("<%s> %s %d  mac[%s] "_fmt"\n", (_sta)->sta_wnet_vif->vm_ndev->name , __func__, __LINE__, ether_sprintf((_sta)->sta_macaddr),__VA_ARGS__);  \
        } while (0)

#endif //_NET80211_COMMON_H_
