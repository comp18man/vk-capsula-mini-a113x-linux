#include "wifi_mac_sae.h"
#include "wifi_cfg80211.h"

#ifdef AML_WPA3

unsigned char wifi_mac_pmkid_vattach(struct wlan_net_vif *wnet_vif) {
    wnet_vif->pmk_list = (void *)NET_MALLOC(sizeof(struct aml_pmk_list), GFP_KERNEL, "pmk_list");
    if (unlikely(!wnet_vif->pmk_list)) {
        ERROR_DEBUG_OUT("pmk list alloc failed\n");
        return -1;
    }

    return 0;
}

void wifi_mac_pmkid_detach(struct wlan_net_vif *wnet_vif) {
    NET_FREE(wnet_vif->pmk_list, "pmk_list");
    return;
}

int aml_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_pmksa *pmksa)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)(((struct vm_wdev_priv *)wiphy_priv(wiphy))->wnet_vif);
    int i;
    unsigned char zero_mac[6] = { 0x00 };
    unsigned char find_entry = 0;

    if ((pmksa->bssid != NULL) && memcmp((unsigned char *)pmksa->bssid, zero_mac, MAC_ADDR_LEN)) {
        DPRINTF(AML_DEBUG_CFG80211, "%s BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, pmksa->bssid[0],
            pmksa->bssid[1], pmksa->bssid[2], pmksa->bssid[3], pmksa->bssid[4], pmksa->bssid[5]);

    } else {
        ERROR_DEBUG_OUT( "bssid wrong\n");
        return -EINVAL;
    }

    for (i = 0; i < WL_NUM_PMKIDS_MAX; i++) {
        if (!memcmp(pmksa->bssid, &wnet_vif->pmk_list->pmkid_cache[i].bssid, MAC_ADDR_LEN)) {
            DPRINTF(AML_DEBUG_CFG80211, "find the bssid in pmkid_cache index:%d, and renew the pmkid\n", i);

            memcpy(wnet_vif->pmk_list->pmkid_cache[i].pmkid, pmksa->pmkid, WLAN_PMKID_LEN);
            wnet_vif->pmk_list->pmkid_cache[i].in_use = 1;
            wnet_vif->pmk_list->pmkid_count = i + 1;
            find_entry = 1;
            break;
        }
    }

    if (!find_entry) {
        DPRINTF(AML_DEBUG_CFG80211, "use new entry index:%d\n", wnet_vif->pmk_list->pmkid_count);

        memcpy(wnet_vif->pmk_list->pmkid_cache[wnet_vif->pmk_list->pmkid_count].bssid, pmksa->bssid, ETH_ALEN);
        memcpy(wnet_vif->pmk_list->pmkid_cache[wnet_vif->pmk_list->pmkid_count].pmkid, pmksa->pmkid, WLAN_PMKID_LEN);

        wnet_vif->pmk_list->pmkid_cache[wnet_vif->pmk_list->pmkid_count].in_use = 1;
        wnet_vif->pmk_list->pmkid_count++ ;
        if (wnet_vif->pmk_list->pmkid_count == 16)
            wnet_vif->pmk_list->pmkid_count = 0;
    }

    return 0;
}

int aml_pmkid_cache_index(struct wlan_net_vif *wnet_vif, const unsigned char *bssid)
{
    int i;
    unsigned char *pmkid;
    unsigned char zero_mac[6] = { 0x00 };

    /* check the bssid is null or not */
    if (!bssid) {
        goto not_found;
    }

    if (!memcmp(bssid, zero_mac, MAC_ADDR_LEN)) {
        goto not_found;
    }

    DPRINTF(AML_DEBUG_CFG80211, "%s BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, bssid[0],
        bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

    for (i = 0; i < WL_NUM_PMKIDS_MAX; i++) {
        if (!memcmp(bssid, &wnet_vif->pmk_list->pmkid_cache[i].bssid, MAC_ADDR_LEN)) {
            pmkid = wnet_vif->pmk_list->pmkid_cache[i].pmkid;
            DPRINTF(AML_DEBUG_CFG80211, "%s pmkid:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                __func__, pmkid[0], pmkid[1], pmkid[2], pmkid[3], pmkid[4], pmkid[5], pmkid[6], pmkid[7], pmkid[8], pmkid[9], pmkid[10],
                pmkid[11], pmkid[12], pmkid[13], pmkid[14], pmkid[15]);
            return i;
        }
    }

not_found:
    DPRINTF(AML_DEBUG_CFG80211, "pmkid not found\n");
    return -1;
}

int aml_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_pmksa *pmksa)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)(((struct vm_wdev_priv*)wiphy_priv(wiphy))->wnet_vif);
    int i;
    unsigned char zero_mac[6] = { 0x00 };
    int npmkids = wnet_vif->pmk_list->pmkid_count;
    unsigned char find_entry = 0;

    if (!pmksa) {
        ERROR_DEBUG_OUT("pmksa is not initialized\n");
        return -1;
    }

    if (!npmkids) {
        /* nmpkids = 0, nothing to delete */
        ERROR_DEBUG_OUT("npmkids=0. skip del\n");
        return 0;
    }

    if ((pmksa->bssid != NULL) && memcmp((unsigned char *)pmksa->bssid, zero_mac, MAC_ADDR_LEN)) {
        DPRINTF(AML_DEBUG_CFG80211, "%s BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, pmksa->bssid[0],
            pmksa->bssid[1], pmksa->bssid[2], pmksa->bssid[3], pmksa->bssid[4], pmksa->bssid[5]);

    } else {
        ERROR_DEBUG_OUT("bssid wrong\n");
        return -EINVAL;
    }

    for (i = 0; i < WL_NUM_PMKIDS_MAX; i++) {
        if (!memcmp(pmksa->bssid, &wnet_vif->pmk_list->pmkid_cache[i].bssid, MAC_ADDR_LEN)) {
            DPRINTF(AML_DEBUG_CFG80211, "%s i:%d, BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, i, wnet_vif->pmk_list->pmkid_cache[i].bssid[0],
                wnet_vif->pmk_list->pmkid_cache[i].bssid[1], wnet_vif->pmk_list->pmkid_cache[i].bssid[2], wnet_vif->pmk_list->pmkid_cache[i].bssid[3],
                wnet_vif->pmk_list->pmkid_cache[i].bssid[4], wnet_vif->pmk_list->pmkid_cache[i].bssid[5]);

            memset(wnet_vif->pmk_list->pmkid_cache[i].bssid, 0, ETH_ALEN);
            memset(wnet_vif->pmk_list->pmkid_cache[i].pmkid, 0, WLAN_PMKID_LEN);
            wnet_vif->pmk_list->pmkid_cache[i].in_use = 0;
            find_entry = 1;
            break;
        }
    }

    if (find_entry) {
        return 0;

    } else {
        return -EINVAL;
    }
}

void aml_del_pmksa_by_index(struct wlan_net_vif *wnet_vif, const unsigned char *bssid)
{
    int pmkid_index;

    pmkid_index = aml_pmkid_cache_index(wnet_vif, bssid);
    if (pmkid_index == -1) {
        return;

    } else {
        memset(wnet_vif->pmk_list->pmkid_cache[pmkid_index].pmkid, 0, sizeof(char)*16);
        wnet_vif->pmk_list->pmkid_count--;
    }

    return;
}

int aml_cfg80211_flush_pmksa(struct wiphy *wiphy, struct net_device *dev)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)(((struct vm_wdev_priv*)wiphy_priv(wiphy))->wnet_vif);
    DPRINTF(AML_DEBUG_CFG80211, "%s\n", __func__);

    memset(wnet_vif->pmk_list, 0, sizeof(*wnet_vif->pmk_list));
    return 0;
}

#if (KERNEL_VERSION(4, 17, 0) <= LINUX_VERSION_CODE) || (defined WPA3_PATCH)
/**
 * wifi_mac_trigger_sae() - Sends SAE info to supplicant
 * This API is used to send required SAE info to trigger SAE in supplicant.
 *
 * Return: None
 */
void wifi_mac_trigger_sae(struct wifi_station *sta)
{
    struct cfg80211_external_auth_params params = {0};

    pr_debug("%s\n", __func__);
    params.key_mgmt_suite = 0x00;
    params.key_mgmt_suite |= 0x0F << 8;
    params.key_mgmt_suite |= 0xAC << 16;
    params.key_mgmt_suite |= 0x8 << 24;

    params.action = NL80211_EXTERNAL_AUTH_START;
    memcpy(params.bssid, sta->sta_bssid, WIFINET_ADDR_LEN);
    memcpy(params.ssid.ssid, sta->sta_essid, sta->sta_esslen);
    params.ssid.ssid_len = sta->sta_esslen;

    cfg80211_external_auth_request(sta->sta_wnet_vif->vm_ndev, &params, GFP_ATOMIC);
}
#endif

#endif//AML_WPA3
