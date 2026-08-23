
#ifndef __NET80211_RATE_H__
#define __NET80211_RATE_H__

/* Definitions for valid VHT MCS map */
#define VHT_MCSMAP_NSS1_MCS0_8  0xfffd  /* NSS=1 MCS 0-8, NSS=2 not supported, NSS=3 not supported */
#define VHT_MCSMAP_NSS2_MCS0_8  0xfff5  /* NSS=1 MCS 0-8, NSS=2       MCS 0-8, NSS=3 not supported */
#define VHT_MCSMAP_NSS1_MCS0_9  0xfffe  /* NSS=1 MCS 0-9, NSS=2 not supported, NSS=3 not supported */
#define VHT_MCSMAP_NSS2_MCS0_9  0xfffa  /* NSS=1 MCS 0-9, NSS=2       MCS 0-9, NSS=3 not supported */
#define VHT_MCSMAP_NSS3_MCS0_9  0xffea  /* NSS=1 MCS 0-9, NSS=2       MCS 0-9, NSS=3       MCS 0-9 */

/* Definitions for valid VHT MCS mask */
#define VHT_MCSMAP_NSS1_MASK   0xfffc   /* Single stream mask */
#define VHT_MCSMAP_NSS2_MASK   0xfff0   /* Dual stream mask */
#define VHT_MCSMAP_NSS3_MASK   0xffc0   /* Tri stream mask */

/* Default VHT80 rate mask support all MCS rates except NSS=3 MCS 6 */


#define VHT_MCS0_MC9 2
#define VHT_MCS0_MC8 1
#define VHT_MCS0_MC7 0


#define  MAX_VHT_STREAMS 8

#define RATE_ALGORITHM_NUMS 1


struct wifi_mac_vht_rate_s {
    int num_streams;
    unsigned char rates[MAX_VHT_STREAMS];
};

void wifi_mac_rate_ratmod_attach(void *drv_priv);
void wifi_mac_rate_ratmod_detach(void *drv_priv);
void wifi_mac_rate_newassoc(struct wifi_station *sta, int isnew);
void wifi_mac_rate_disassoc(struct wifi_station *sta);
int wifi_mac_rate_vattach (struct wlan_net_vif *wnet_vif);
int check_rate(struct wlan_net_vif *wnet_vif, const struct wifi_scan_info *scaninfo);
int check_ht_rate(struct wlan_net_vif *wnet_vif, const struct wifi_scan_info *scaninfo);
int wifi_mac_setup_vht_rates(struct wifi_station *sta, unsigned char *ie, int flags);
void wifi_mac_vht_rate_init(struct wlan_net_vif *wnet_vif, unsigned short mcs_map,unsigned short max_datarate, unsigned short basic_mcs);
void wifi_mac_rate_init(void * ieee, enum wifi_mac_macmode mode, const struct drv_rate_table *rt);
void  wifi_mac_sort_rate(struct wifi_mac_rateset *);
void  wifi_mac_xsect_rate(struct wifi_mac_rateset *, struct wifi_mac_rateset *, struct wifi_mac_rateset *);
int wifi_mac_brs_rate_check(struct wifi_mac_rateset *, struct wifi_mac_rateset *);
int wifi_mac_fixed_rate_check(struct wifi_station *, struct wifi_mac_rateset *);
int wifi_mac_setup_rates(struct wifi_station *, const unsigned char *, const unsigned char *, int);
int wifi_mac_setup_ht_rates(struct wifi_station *, unsigned char *,int);
void    wifi_mac_setup_basic_ht_rates(struct wifi_station *,unsigned char *);
int wifi_mac_iserp_rateset(struct wifi_mac *,struct wifi_mac_rateset *);
void wifi_mac_set_legacy_rates(struct wifi_mac_rateset *rs, struct wlan_net_vif *wnet_vif);
void wifi_mac_sta_set_basic_rates(struct wlan_net_vif *wnet_vif, char *si_rates, char *si_exrates);
void wifi_mac_ap_set_basic_rates(struct wlan_net_vif *wnet_vif, enum wifi_mac_macmode macmode);

#endif
