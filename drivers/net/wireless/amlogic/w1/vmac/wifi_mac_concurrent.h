#ifndef NET80211_CONCURRENT_H
#define NET80211_CONCURRENT_H

static inline unsigned char wifi_mac_is_wm_running (struct wifi_mac *wifimac)
{
    if (wifimac && (wifimac->wm_nrunning > 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline unsigned char wifi_mac_is_only_wnet_vif_running (struct wlan_net_vif *wnet_vif)
{
    if (wnet_vif && wnet_vif->vm_wmac && (wnet_vif->vm_wmac->wm_nrunning == 1)
        &&(wnet_vif->vm_wmac->wm_runningmask & BIT(wnet_vif->wnet_vif_id)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline unsigned char wifi_mac_is_others_wnet_vif_running (struct wlan_net_vif *wnet_vif)
{
    ASSERT(wnet_vif);
    if (wnet_vif->vm_wmac && (wnet_vif->vm_wmac->wm_nrunning > 0)
        && (wnet_vif->vm_wmac->wm_runningmask & (~BIT(wnet_vif->wnet_vif_id))))
    {
        return true;
    }
    else
    {
        return false;
    }
}

#define COMCURRENT_CHANNCHANGE_TIME     500

#ifdef  CONFIG_CONCURRENT_MODE
int concurrent_change_channel_timeout(void * data);
void concurrent_channel_restore(int target_channel,struct wlan_net_vif *wnet_vif,int times);
void concurrent_channel_protection(struct wlan_net_vif *wnet_vif,int times);
int concurrent_vsdb_change_channel(void * data);
void concurrent_vsdb_prepare_change_channel(struct wifi_mac *wifimac);
void concurrent_vsdb_do_channel_change(void * data);
void concurrent_vsdb_init(struct wifi_mac *wifimac);
int csa_trigger_timeout(void *data);
#endif
unsigned char concurrent_check_is_vmac_same_pri_channel(struct wifi_mac *wifimac);
unsigned char concurrent_check_vmac_is_AP(struct wifi_mac *wifimac);
struct wlan_net_vif *wifi_mac_running_main_wnet_vif(struct wifi_mac *wifimac);
struct wlan_net_vif *wifi_mac_running_wnet_vif(struct wifi_mac *wifimac);
void channel_switch_announce_trigger(struct wifi_mac *wifimac, struct wifi_channel *switch_chan);
#endif //NET80211_CONCURRENT_H
