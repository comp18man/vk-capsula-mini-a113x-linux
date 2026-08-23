#ifndef __NET80211_CHAN_H__
#define __NET80211_CHAN_H__

#define WIFI_CHANNEL_LOCK(wifimac) OS_SPIN_LOCK_IRQ(&(wifimac)->channel_lock, (wifimac)->channel_lock_flag)
#define WIFI_CHANNEL_UNLOCK(wifimac) OS_SPIN_UNLOCK_IRQ(&(wifimac)->channel_lock, (wifimac)->channel_lock_flag)

void wifi_mac_update_country_chan_list(struct wifi_mac *wifimac);
void wifi_mac_chan_setup(void * ieee, unsigned int wMode, int countrycode_ex);
int wifi_mac_chan_attach(struct wifi_mac *wifimac);
int wifi_mac_chan_overlapping_map_init(struct wifi_mac *wifimac);
unsigned char wifi_mac_chan_num_avail (struct wifi_mac *wifimac, unsigned char channum);
int wifi_mac_recv_bss_intol_channelCheck(struct wifi_mac *wifimac, struct wifi_mac_ie_intolerant_report *intol_ie);
struct wifi_channel * wifi_mac_scan_sta_get_ap_channel(struct wlan_net_vif *wnet_vif, struct wifi_mac_scan_param *sp);
unsigned int wifi_mac_mhz2chan(unsigned int freq);
struct wifi_channel * wifi_mac_find_chan(struct wifi_mac *wifimac, int chan, int bw, int center_chan);
int wifi_mac_set_wnet_vif_channel(struct wlan_net_vif *wnet_vif,  int chan, int bw, int center_chan,unsigned char switch_flag);
void wifi_mac_set_wnet_vif_chan_ex(SYS_TYPE param1,SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
struct wifi_channel * wifi_mac_get_wm_chan (struct wifi_mac *wifimac);
struct wifi_channel * wifi_mac_get_connect_wnet_vif_channel(struct wlan_net_vif *wnet_vif);
void wifi_mac_restore_wnet_vif_channel(struct wlan_net_vif *wnet_vif);
unsigned int   wifi_mac_chan2ieee(struct wifi_mac *, const struct wifi_channel *);
unsigned int   wifi_mac_Ieee2mhz(unsigned int, unsigned int);
struct wifi_channel *wifi_mac_get_main_vmac_channel(struct wifi_mac *wifimac);
struct wifi_channel *wifi_mac_get_p2p_vmac_channel(struct wifi_mac *wifimac);
void chan_dbg(struct wifi_channel *chan,  char* str, int line);
void  wifi_mac_update_chan_list_by_country(int country_code, int support_opt[],int opt_num,struct wifi_mac *wifimac);
void wifi_mac_mark_dfs_channel(struct wlan_net_vif *wnet_vif, int chan_num);
void wifi_mac_unmark_dfs_channel(struct wlan_net_vif *wnet_vif, int chan_num);
int wifi_mac_if_dfs_channel(struct wifi_mac *wifimac, int chan_num);
int find_country_code(unsigned char *countryString);
int wifimac_set_tx_pwr_plan(int txpoweplan);
unsigned char wifimac_get_tx_pwr_plan(int country_code);
int wifi_mac_set_tx_power_coefficient(struct drv_private *drv_priv, struct wifi_channel *chan, int tx_power_plan);
int hal_cfg_txpwr_cffc_param_init(int tx_power_plan);
int update_tx_power_coefficient_plan(int tx_power_plan, unsigned short pwr_coefficient[]);
int update_tx_power_band(int tx_power_plan, unsigned short pwr_value[]);
int wifi_mac_find_80M_channel_center_chan(int chan);
unsigned char if_southamerica_country(unsigned char *countrycode);
unsigned char wifi_mac_p2p_home_channel_enabled(struct wlan_net_vif *wnet_vif);
bool wifi_mac_get_chandef(struct wifi_channel *vmac_chan, struct cfg80211_chan_def *chandef);
unsigned char wifi_mac_get_operation_class(struct cfg80211_chan_def chandef);
#endif//__NET80211_CHAN_H__
