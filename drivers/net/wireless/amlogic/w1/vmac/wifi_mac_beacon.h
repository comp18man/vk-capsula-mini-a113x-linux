/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2015-2018
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac beacon alloc/initial/update module
 *
 *
 ****************************************************************************************
 */

struct sk_buff *_wifi_mac_beacon_alloc(struct wifi_station *sta, struct wifi_mac_beacon_offsets *bo);
int _wifi_mac_beacon_update(struct wifi_station *sta,            struct wifi_mac_beacon_offsets *bo, struct sk_buff *skb, int mcast);
int wifi_mac_beacon_alloc(void * ieee, int wnet_vif_id);
void wifi_mac_beacon_alloc_ex(SYS_TYPE param1,             SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
int wifi_mac_sta_beacon_init(struct wlan_net_vif *wnet_vif);
void wifi_mac_sta_beacon_init_ex (SYS_TYPE param1,               SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
void wifi_mac_beacon_free(void * ieee, int wnet_vif_id);
void wifi_mac_beacon_config(void * ieee, int wnet_vif_id);
void wifi_mac_beacon_sync(void * ieee, int wnet_vif_id);
int wifi_mac_update_beacon(void * ieee, int wnet_vif_id,            struct sk_buff * skbbuf, int mcast);
int wifi_mac_process_beacon_miss(struct wlan_net_vif *wnet_vif);
void wifi_mac_process_beacon_miss_ex(SYS_TYPE arg);
int wifi_mac_set_beacon_miss_ex(struct wlan_net_vif *wnet_vif, unsigned char enable, int period);
void wifi_mac_set_beacon_miss(SYS_TYPE param1,             SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
int wifi_mac_set_vsdb(SYS_TYPE param1,         SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
void wifi_mac_beacon_update_csaie(struct wifi_station *sta,
                           struct wifi_mac_beacon_offsets *bo, struct sk_buff *skb, struct wifi_channel *switch_chan);
void wifi_mac_set_vsdb_task(struct wifi_mac *wifimac, struct wlan_net_vif *wnet_vif, enum VsdbState state);
void wifi_mac_set_vsdb_task(struct wifi_mac *wifimac, struct wlan_net_vif *wnet_vif, enum VsdbState state);
