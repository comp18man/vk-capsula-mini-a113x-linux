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

#ifndef _WIFI_MAC_SAE_H_
#define _WIFI_MAC_SAE_H_

#include "wifi_mac_if.h"

#define MAXPMKID 16/* max # PMKID cache entries NDIS */
#define WL_NUM_PMKIDS_MAX MAXPMKID

typedef struct _aml_pmkid_cache {
    unsigned char in_use;
    unsigned char bssid[6];
    unsigned char pmkid[16];
    unsigned char ssid[33];
    unsigned char *ssid_octet;
    unsigned short ssid_length;
} aml_pmkid_cache;

struct aml_pmk_list {
    unsigned char pmkid_count;
    aml_pmkid_cache pmkid_cache[MAXPMKID];
};

unsigned char wifi_mac_pmkid_vattach(struct wlan_net_vif *wnet_vif);
void wifi_mac_pmkid_detach(struct wlan_net_vif *wnet_vif);
int aml_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_pmksa *pmksa);
int aml_pmkid_cache_index(struct wlan_net_vif *wnet_vif, const unsigned char *bssid);
int aml_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_pmksa *pmksa);
void aml_del_pmksa_by_index(struct wlan_net_vif *wnet_vif, const unsigned char *bssid);
int aml_cfg80211_flush_pmksa(struct wiphy *wiphy, struct net_device *dev);
void wifi_mac_trigger_sae(struct wifi_station *sta);

#endif//_WIFI_MAC_SAE_H_

