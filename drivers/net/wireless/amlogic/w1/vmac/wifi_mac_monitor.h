/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     monitor interface
 *
 *
 ****************************************************************************************
 */

#ifndef __NET80211_MONITOR_H__
#define __NET80211_MONITOR_H__

#define SIOCANDROID_PRIV       (SIOCDEVPRIVATE+1)//yishu
#define	SIOCG80211STATS		(SIOCDEVPRIVATE+2)
#define	SIOC80211IFDESTROY	 	(SIOCDEVPRIVATE+8)


struct net_device *vm_cfg80211_add_p2p_go_if(struct wlan_net_vif *wnet_vif ,
	const char *name,

enum nl80211_iftype type);
struct net_device *vm_cfg80211_add_monitor_if(struct wlan_net_vif *wnet_vif ,
			const char *name);

#define vm_netdev_priv(netdev)((struct wlan_net_vif *) ( ((struct vm_netdev_priv_indicator *)netdev_priv(netdev))->priv ))

#endif
