#include "wifi_mac_com.h"
#include "wifi_iwpriv_cmd.h"
#include "wifi_cmd_func.h"
#include "chip_intf_reg.h"
#include <net/cfg80211.h>
#include "version.h"
#include "wifi_drv_capture.h"
#include "wifi_csi.h"


extern void print_driver_version(void);
extern char **aml_cmd_char_phrase(char sep, const char *str, int *size);
extern struct udp_info aml_udp_info[];
extern struct udp_timer aml_udp_timer;
extern int udp_cnt;
extern unsigned char g_tx_power_change_disable;
extern unsigned char g_initial_gain_change_disable;


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
extern int vm_cfg80211_set_bitrate_mask(struct wiphy *wiphy, struct net_device *dev,
    const unsigned char *peer, const struct cfg80211_bitrate_mask *mask);
#else
extern int vm_cfg80211_set_bitrate_mask(struct wiphy *wiphy, struct net_device *dev,
                unsigned int link_id, const unsigned char *peer,
                const struct cfg80211_bitrate_mask *mask);
#endif


void wifi_mac_pwrsave_set_inactime(struct wlan_net_vif *wnet_vif, unsigned int time);

static struct wlan_net_vif *aml_iwpriv_get_vif(char *name)
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct drv_private *drv_priv = NULL;
    struct wifi_mac *wifimac = NULL;
    int idx = 0;

    wifimac = wifi_mac_get_mac_handle();
    drv_priv = wifimac->drv_priv;

    for (idx = 0; idx < 2; idx++) {
        wnet_vif = drv_priv->drv_wnet_vif_table[idx];
        if (strncmp(wnet_vif->vm_ndev->name, name, sizeof(wnet_vif->vm_ndev->name)) == 0) {
            pr_debug("%s, %s, %s\n", __func__, wnet_vif->vm_ndev->name, name);
            return wnet_vif;
        }
    }
    return NULL;
}


static int aml_ap_set_amsdu_state(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    pr_debug("%s, %s\n", __func__,extra);
    aml_set_mac_amsdu_switch_state(extra);

    return 0;
}

static int aml_ap_set_ampdu_state(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    pr_debug("%s, %s\n", __func__,extra);
    aml_set_drv_ampdu_switch_state(extra);

    return 0;
}


static int aml_ap_get_amsdu_state(void)
{
    pr_debug("%s\n", __func__);
    aml_set_mac_amsdu_switch_state(NULL);
    return 0;
}

static int aml_ap_get_ampdu_state(void)
{
    pr_debug("%s\n", __func__);
    aml_set_drv_ampdu_switch_state(NULL);
    return 0;
}

static int aml_ap_set_11h(unsigned char channel)
{
    wifi_mac_ap_set_11h(channel);

    return 0;
}


static int aml_ap_send_addba_req(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{

    int *param = (int *)extra;
    int val = 0;
    char addr[MAX_MAC_BUF_LEN]={0};

    pr_debug("%s(%d), %d, %d, %d, %d, %d, %d, %d\n", __func__, __LINE__,
            param[0], param[1], param[2],param[3],param[4],param[5],param[6]);

    snprintf(addr, MAX_MAC_BUF_LEN, "%02d:%02d:%02d:%02d:%02d:%02d",param[0],param[1],param[2],param[3],param[4],param[5]);
    val = param[6];
    wifi_mac_send_addba_req(addr, val);

    return 0;

}

unsigned int get_reg(struct wlan_net_vif *wnet_vif, unsigned int set)
{

    unsigned int usr_data = 0;
    unsigned int reg_val = 0;

    usr_data = set;
    pr_debug("%s: Reg addr: 0x%08x\n", __func__, usr_data);

    if (((usr_data >> 24) & 0xff) == 0xff ) {
#ifdef USE_T902X_RF
        reg_val = rf_i2c_read(usr_data & 0x00ffffff);
#endif
    } else if (((usr_data >> 24) & 0xf0) == 0xf0 ) {
#ifdef USE_T902X_RF
        reg_val = rf_i2c_read(usr_data & 0xffffffff);
#endif
    } else {
        reg_val = wnet_vif->vif_ops.read_word(usr_data);
    }

    pr_debug("Drv info: Reg data=&0x%08x\n",reg_val);

    return 0;
}


unsigned int set_reg(struct wlan_net_vif *wnet_vif, unsigned int set1, unsigned int set2)
{

    unsigned int usr_data = 0;
    unsigned int usr_data_ext = 0;
    usr_data = set1;
    usr_data_ext = set2;

    pr_debug("Cfg80211: Reg addr: val:0x%08x,val:0x%08x\n",usr_data,usr_data_ext);
    if (((usr_data >> 24) & 0xff) == 0xff) {
#ifdef USE_T902X_RF
        rf_i2c_write( usr_data & 0x00ffffff,usr_data_ext );//access t902x rf reg
#endif
    } else if (((usr_data >> 24) & 0xf0) == 0xf0) {
#ifdef USE_T902X_RF
        rf_i2c_write( usr_data & 0xffffffff,usr_data_ext );//access t902x rf reg
#endif
    } else {
        wnet_vif->vif_ops.write_word(usr_data , usr_data_ext);
    }

    return 0;
}

unsigned int get_latest_tx_status(struct wifi_mac *wifimac)
{
    unsigned int addr = 0x00000038;
    unsigned int val_con = 0;

    memcpy((unsigned char*)&val_con, (unsigned char*)&addr, sizeof(unsigned int));
    wifimac->drv_priv->net_ops->wifi_mac_get_sts(wifimac, 0, ntohl(val_con));
    return 0;
}

int aml_beacon_intvl_set(struct wlan_net_vif *wnet_vif, unsigned int set)
{
    unsigned int regdata = 0;
    int bcn_intvl =0;
    int usr_data = 0;

    usr_data = set;

    if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL)) {
        bcn_intvl = usr_data * 100;
        if (bcn_intvl == 0) {
            bcn_intvl = WIFINET_BINTVAL_DEFAULT;
        }
        pr_debug("%s:%d, bcn intvl %d\n", __func__, __LINE__, bcn_intvl);
        wnet_vif->vm_wmac->drv_priv->drv_ops.Phy_beaconinit(wnet_vif->vm_wmac->drv_priv,
                wnet_vif->wnet_vif_id, bcn_intvl);

            /* change max sleep time */
        if (usr_data != 0) {
           regdata = wnet_vif->vif_ops.read_word(RG_AON_A37);
           regdata &= ~0x00ff0000;
           regdata |= usr_data << 16;
           wnet_vif->vif_ops.write_word(RG_AON_A37, regdata);
        }
    }
    return 0;
}

static void aml_iwpriv_enable_fw_log(struct wlan_net_vif *wnet_vif)
{
    pr_debug("%s: fw log enabled from iwpriv cmd\n", __func__);
    set_reg(wnet_vif, 0x00f00004, 0x0ffbf0ff);
    msleep(100);
    set_reg(wnet_vif, 0x00f00008, 0x00040f00);
    msleep(100);
    set_reg(wnet_vif, 0x00f00020, 0x00000001);
}

int aml_set_ldpc(struct wlan_net_vif *wnet_vif, unsigned int set)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta = wnet_vif->vm_mainsta;

    if (aml_wifi_is_enable_rf_test()) {
        if (1 == set) {
            gB2BTestCasePacket.ldpc_enable = 1;
            pr_debug("Enable tx LDPC\n");
        } else if (0 == set){
            gB2BTestCasePacket.ldpc_enable = 0;
            pr_debug("Disable tx LDPC\n");
        } else {
            ERROR_DEBUG_OUT("Invalid parameter\n");
        }
        return 0;
    }

    if (1 == set) {
        sta->sta_vhtcap |= WIFINET_VHTCAP_RX_LDPC;
        wifimac->wm_flags |=WIFINET_F_LDPC;
        pr_debug("Enable LDPC, if need to change, the action must be executed before connecting to ap or creating ap\n");
    } else if (0 == set) {
        sta->sta_vhtcap &= ~WIFINET_VHTCAP_RX_LDPC;
        wifimac->wm_flags &=~WIFINET_F_LDPC;
        pr_debug("Disable LDPC, if need to change, the action must be executed before connecting to ap or creating ap\n");
    } else {
        ERROR_DEBUG_OUT("Invalid parameter\n");
    }
    return 0;
}

#if defined(SU_BF) || defined(MU_BF)
int aml_set_beamforming(struct wlan_net_vif *wnet_vif, unsigned int set1,unsigned int set2)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int usr_data1 = 0;
    int usr_data2 = 0;

    usr_data1 = set1;
    usr_data2 = set2;

    if (usr_data2 > 4) {
        pr_debug("support max spatial is 4 !\n");
        wifimac->max_spatial = 4;
    } else if (usr_data2 <= 0) {
        pr_debug("min spatial is 1 !\n");
        wifimac->max_spatial = 1;
    } else {
        wifimac->max_spatial = usr_data2;
    }
    pr_debug("%s:%d, set spatial %d \n", __func__, __LINE__, wifimac->max_spatial);

    if(usr_data1 < 4) {
        if (((usr_data1 & 0xF) & BIT(0)) == BIT(0)) {
            wifimac->wm_flags_ext2 |= WIFINET_VHTCAP_SU_BFMEE;
            pr_debug("%s:%d, enable su mimo\n", __func__, __LINE__);
        } else {
            wifimac->wm_flags_ext2 &= ~WIFINET_VHTCAP_SU_BFMEE;
            pr_debug("%s:%d, disable su mimo\n", __func__, __LINE__);
        }

        if (((usr_data1 & 0xF) & BIT(1)) == BIT(1)) {
            wifimac->wm_flags_ext2 |= WIFINET_VHTCAP_MU_BFMEE;
                pr_debug("%s:%d, enable mu mimo\n", __func__, __LINE__);
        } else {
            wifimac->wm_flags_ext2 &= ~WIFINET_VHTCAP_MU_BFMEE;
            pr_debug("%s:%d, disable mu mimo\n", __func__, __LINE__);
        }
    } else {
        ERROR_DEBUG_OUT("initial parameter!\n");
    }
    return 0;

}
#endif


static unsigned int
aml_iwpriv_legacy_2g_rate_to_bitmap(int legacy)
{
    switch(legacy)
    {
        case 1:
            return 0x00;
        case 2:
            return 0x01;
        case 5:
            return 0x02;
        case 11:
            return 0x03;
        case 6:
            return 0x04;
        case 9:
            return 0x05;
        case 12:
            return 0x06;
        case 18:
            return 0x07;
        case 24:
            return 0x08;
        case 36:
            return 0x09;
        case 48:
            return 0x0a;
        case 54:
            return 0x0b;
        default:
            return 0;
    }
}

static unsigned int
aml_iwpriv_legacy_5g_rate_to_bitmap(int legacy)
{
    switch(legacy)
    {
        case 6:
            return 0x00;//6M;
        case 9:
            return 0x01;//9M;
        case 12:
            return 0x02;//12M;
        case 18:
            return 0x03;//18M;
        case 24:
            return 0x04;//24M;
        case 36:
            return 0x05;//36M;
        case 48:
            return 0x06;//48M;
        case 54:
            return 0x07;//54M;
        default:
            return 0;
    }
}


static unsigned int
aml_iwpriv_ht_rate_to_bitmap(int ht_mcs)
{
    switch(ht_mcs)
    {
        case 0:
            return 0x00;
        case 1:
            return 0x01;
        case 2:
            return 0x02;
        case 3:
            return 0x03;
        case 4:
            return 0x04;
        case 5:
            return 0x05;
        case 6:
            return 0x06;
        case 7:
            return 0x07;
        default:
            return 0;
    }
}


static unsigned int
aml_iwpriv_vm_vht_rate_to_bitmap(int vht_mcs)
{
    switch(vht_mcs)
    {
        case 0:
            return 0x00;
        case 1:
            return 0x01;
        case 2:
            return 0x02;
        case 3:
            return 0x03;
        case 4:
            return 0x04;
        case 5:
            return 0x05;
        case 6:
            return 0x06;
        case 7:
            return 0x07;
        case 8:
            return 0x08;
        case 9:
            return 0x09;
        default:
            return 0;
    }
}


int
aml_iwpriv_set_lagecy_bitrate_mask(struct net_device *dev, unsigned int set)
{
    int band = 0;
    struct cfg80211_bitrate_mask mask;
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    memset(&mask, 0, sizeof(struct cfg80211_bitrate_mask));
    mask.control[band].legacy = (1<<aml_iwpriv_legacy_2g_rate_to_bitmap(set));
    pr_debug("%s %d, opmode %d, band %d\n", __func__, __LINE__, wnet_vif->vm_opmode, band);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
    vm_cfg80211_set_bitrate_mask(NULL, dev, NULL, &mask);
#else
    vm_cfg80211_set_bitrate_mask(NULL, dev, 0, NULL, &mask);
#endif

    return 0;
}


int
aml_iwpriv_set_ht_bitrate_mask(struct net_device *dev, unsigned int set)
{
    int band = 0;
    struct cfg80211_bitrate_mask mask;
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    memset(&mask, 0, sizeof(struct cfg80211_bitrate_mask));

    if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        band = WIFINET_IS_CHAN_5GHZ(wnet_vif->vm_curchan)? NL80211_BAND_5GHZ:NL80211_BAND_2GHZ;
    } else {
        band = NL80211_BAND_2GHZ;
    }

    pr_debug("%s %d, opmode %d, band %d\n", __func__, __LINE__, wnet_vif->vm_opmode, band);
    mask.control[band].ht_mcs[0] = (1<<aml_iwpriv_ht_rate_to_bitmap(set));
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
    vm_cfg80211_set_bitrate_mask(NULL, dev, NULL, &mask);
#else
    vm_cfg80211_set_bitrate_mask(NULL, dev, 0, NULL, &mask);
#endif

    return 0;
}


int
aml_iwpriv_set_vht_bitrate_mask(struct net_device *dev, unsigned int set)
{
    int band = 0;
    struct cfg80211_bitrate_mask mask;
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    memset(&mask, 0, sizeof(struct cfg80211_bitrate_mask));

    if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        band = WIFINET_IS_CHAN_5GHZ(wnet_vif->vm_curchan)? NL80211_BAND_5GHZ:NL80211_BAND_2GHZ;
    } else {
        band = NL80211_BAND_2GHZ;
    }

    pr_debug("%s %d, opmode %d, band %d\n", __func__, __LINE__, wnet_vif->vm_opmode, band);
    mask.control[band].vht_mcs[0] = (1<<aml_iwpriv_vm_vht_rate_to_bitmap(set));
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
    vm_cfg80211_set_bitrate_mask(NULL, dev, NULL, &mask);
#else
    vm_cfg80211_set_bitrate_mask(NULL, dev, 0, NULL, &mask);
#endif

    return 0;
}


void aml_iwpriv_set_rate_auto(struct wlan_net_vif *wnet_vif)
{
    wnet_vif->vm_fixed_rate.rateinfo = 0;
    wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_NONE;
    wnet_vif->vm_change_rate_enable = 1;
    pr_debug("%s %d, enable autorate\n", __func__,__LINE__);
}

void aml_iwpriv_set_uapsd(struct wlan_net_vif *wnet_vif, unsigned int set)
{
    if ((unsigned char)set != 0) {
        WIFINET_VMAC_UAPSD_ENABLE(wnet_vif);
        pr_debug("%s(%d) enable ap uapsd\n ", __func__, __LINE__);

    } else {
        WIFINET_VMAC_UAPSD_DISABLE(wnet_vif);
        pr_debug("%s(%d) disable ap uapsd\n ", __func__, __LINE__);
    }
    wnet_vif->vm_flags |= WIFINET_F_WMEUPDATE;
}

unsigned char aml_iwpriv_set_band(unsigned int set)
{
    struct drv_private *drv_priv = drv_get_drv_priv();
    drv_priv->drv_config.cfg_band = set;
    pr_debug("%s(%d) band %d\n ", __func__, __LINE__, set);
    return 0;
}

static unsigned char aml_iwpriv_set_initial_gain_change_hang(unsigned int set)
{
    g_initial_gain_change_disable = set;

    if (g_initial_gain_change_disable) {
        pr_err("%s(%d) initial_gain_change invalid\n ", __func__, __LINE__);

    } else {
        pr_debug("%s(%d) initial_gain_change valid\n ", __func__, __LINE__);
    }
    return 0;
}

static unsigned char aml_iwpriv_set_tx_power_change_hang(unsigned int set)
{
    g_tx_power_change_disable = set;

    if (g_tx_power_change_disable) {
        pr_err("%s(%d) tx_power_change invalid\n ", __func__, __LINE__);

    } else {
        pr_debug("%s(%d) tx_power_change valid\n ", __func__, __LINE__);
    }
    return 0;
}

unsigned char aml_iwpriv_get_band(void)
{
    struct drv_private *drv_priv = drv_get_drv_priv();
    return drv_priv->drv_config.cfg_band;
}

static unsigned char aml_iwpriv_set_mac_mode(unsigned int set)
{
    struct drv_private *drv_priv = drv_get_drv_priv();
    drv_priv->drv_config.cfg_mac_mode = set;
    return 0;
}

static int aml_iwpriv_send_para1(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{

    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;

    int *param = (int *)extra;
    char* str[2] = {0};
    int sub_cmd = param[0];
    int set = param[1];
    char buf[30] = {0};
    int len = 0;


    pr_debug("%s, sub_cmd %d, value %d\n", __func__,param[0], param[1]);

    wifimac = wifi_mac_get_mac_handle();
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    switch (sub_cmd) {
        case AML_IWP_11H:
            aml_ap_set_11h(set);
            break;

        case AML_IWP_ARP_RX:
            break;

        case AML_IWP_ROAM_THRESH_2G:
            /*e.g '-80' need 3 char in string, added '\0', so need + 4 */
            len = strlen("set_roam_thr_2g ") + 4;
            snprintf(buf, len, "set_roam_thr_2g %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_roaming_threshold_2g(wnet_vif, buf, len);
            break;

        case AML_IWP_ROAM_THRESH_5G:
            /*e.g '-80' need 3 char in string, added '\0', so need + 4 */
            len = strlen("set_roam_thr_5g ") + 4;
            snprintf(buf, len, "set_roam_thr_5g %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_roaming_threshold_5g(wnet_vif, buf, len);
            break;

        case AML_IWP_ROAM_MODE:
            len = strlen("set_roam_mode ") + 4;
            snprintf(buf, len, "set_roam_mode %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_roaming_mode(wnet_vif, buf, len);
            break;

        case AML_IWP_MARK_DFS_CHAN:
            len = strlen("mark_dfs_chan ") + 4;
            snprintf(buf, len, "mark_dfs_chan %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_mark_dfs_channel(wnet_vif, buf, len);
            break;

        case AML_IWP_UNMARK_DFS_CHAN:
            len = strlen("unmark_dfs_chan ") + 4;
            snprintf(buf, len, "unmark_dfs_chan %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_unmark_dfs_channel(wnet_vif, buf, len);
            break;

        case AML_IWP_WEAK_THR_NARROW:
            len = strlen("set_weak_thr_nb ") + 4;
            snprintf(buf, len, "set_weak_thr_nb %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_signal_power_weak_thresh_for_narrow_bandwidth(wnet_vif, buf, len);
            break;

        case AML_IWP_WEAK_THR_WIDE:
            len = strlen("set_weak_thr_wb ") + 4;
            snprintf(buf, len, "set_weak_thr_wb %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_signal_power_weak_thresh_for_wide_bandwidth(wnet_vif, buf, len);
            break;

        case AML_IWP_EAT_COUNT:
            len = strlen("set_eat_count ") + 4;
            snprintf(buf, len, "set_eat_count %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_eat_count_max(wnet_vif, buf, len);
            break;

        case AML_IWP_AGGR_THRESH:
            len = strlen("set_aggr_thresh ") + 4;
            snprintf(buf, len, "set_aggr_thresh %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_aggr_thresh(wnet_vif, buf, len);
            break;

        case AML_IWP_HEART_INTERVAL:
            len = strlen("set_hrt_int ") + 4;
            snprintf(buf, len, "set_hrt_int %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_hrtimer_interval(wnet_vif, buf, len);
            break;

        case AML_IWP_BSS_COEX:
            len = strlen("send_bss_coex");
            snprintf(buf, len, "send_bss_coex");
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_sta_send_coexist_mgmt(wnet_vif, buf, len);
            break;

        case AML_IWP_WMM_AC_DELTS:
            len = strlen("wmm_ac_delts ") + 4;
            snprintf(buf, len, "wmm_ac_delts %d", set);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_wmm_ac_delts(wnet_vif, buf, len);
            break;

        case AML_IWP_SHORT_GI:
            str[0] = "off";
            str[1] = "on";
            len = strlen("set_short_gi ") + strlen(str[set]) + 1;
            snprintf(buf, len, "set_short_gi %s", str[set]);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_short_gi(wnet_vif, buf, len);
            break;

        case AML_IWP_DYNAMIC_BW:
            str[0] = "off";
            str[1] = "on";
            len = strlen("set_dynamic_bw ") + strlen(str[set]) + 1;
            snprintf(buf, len, "set_dynamic_bw %s", str[set]);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_set_dynamic_bw(wnet_vif, buf, len);
            break;

        case AML_IWP_TXAGGR_STA:
            str[0] = "all";
            str[1] = "reset";
            len = strlen("get_txaggr_sta ") + strlen(str[set]) + 1;
            snprintf(buf, len, "get_txaggr_sta %s", str[set]);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_get_drv_txaggr_status(wnet_vif, buf, len);
            break;

        case AML_IWP_GET_REG:
            if (set == 0x7fffffff) {
                pr_debug("******************************************************************************\n");
                pr_debug("You are using a legacy iwpriv tool, strongly suggest using a latest iwpriv one\n");
                pr_debug("You also can using the legacy tool as below:\n iwpriv wlan0 get_reg 0xff000c80 -> iwpriv wlan0 get_reg_legacy 0xff00 0x00c80\n");
                pr_debug("******************************************************************************\n");
            }
            get_reg(wnet_vif, set);
            break;

        case AML_IWP_SET_BCN_INTERVAL:
            aml_beacon_intvl_set(wnet_vif, set);
            break;

        case AML_IWP_SET_LDPC:
            aml_set_ldpc(wnet_vif, set);
            break;

        case AML_IWP_SET_SCAN_TIME_IDLE:
            if (set) {
                wnet_vif->vm_scan_time_idle = (unsigned char)set;
                wifi_mac_set_scan_time(wnet_vif);
            }
            pr_debug("%s, vid:%d set scan_time_idle = %d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_time_idle);
            break;

        case AML_IWP_SET_SCAN_TIME_CONNECT:
            if (set) {
                wnet_vif->vm_scan_time_connect = (unsigned char)set;
                wifi_mac_set_scan_time(wnet_vif);
            }
            pr_debug("%s, vid:%d set scan_time_connect = %d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_time_connect);
            break;

        case AML_IWP_SET_SCAN_HANG:
            wnet_vif->vm_scan_hang = (unsigned char)set;
            pr_debug("%s, vid:%d vm_scan_hang:%d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_hang);
            break;

        case AML_IWP_EN_BTWIFI_COEX:
            pr_debug("%s, coexist en= %d\n ", __func__, set);
            wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_en(set);
            break;

        case AML_IWP_SET_COEXIST_MAX_MISS_BCN_CNT:
            wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_max_miss_bcn(set);
            break;

        case AML_IWP_SET_COEXIST_REQ_TIMEOUT:
            wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_req_timeslice_timeout_value(set);
            pr_debug("%s, set req timeout value= %d\n ", __func__, set);
            break;

        case AML_IWP_SET_COEXIST_NOT_GRANT_WEIGHT:
            wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_not_grant_weight(set);
            pr_debug("%s, set coexist_not_grant_weight= %d\n ", __func__, set);
            break;

        case AML_IWP_SET_RATE_LEGACY:
            aml_iwpriv_set_lagecy_bitrate_mask(dev, set);
            break;

        case AML_IWP_SET_RATE_HT:
            aml_iwpriv_set_ht_bitrate_mask(dev, set);
            break;

        case AML_IWP_SET_RATE_VHT:
            aml_iwpriv_set_vht_bitrate_mask(dev, set);
            break;

        case AML_IWP_SET_POWER:
            wifi_mac_pwrsave_set_inactime(wnet_vif, set);
            break;

        case AML_IWP_SET_CHL_RSSI:
            wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, set);
            break;

        case AML_IWP_SET_BURST:
            wifimac->drv_priv->drv_config.cfg_burst_ack = set;
            pr_debug("iwpriv set burst %d\n", set);
            break;

        case AML_IWP_SET_UAPSD:
            aml_iwpriv_set_uapsd(wnet_vif, set);
            break;

        case AML_IWP_SET_PT_RX_START:
            wnet_vif->vif_ops.pt_rx_start(set);
            break;

        case AML_IWP_SET_SCAN_PRI:
            /* bit31-bit16 : minimal  priority
               bit15:bit0: max priority */
            wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_scan_priority_range(set);
            break;

        case AML_IWP_SET_BE_BK_NOQOS_PRI:
            /* bit31-bit16 : minimal  priority
               bit15:bit0: max priority */
            wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_be_bk_noqos_priority_range(set);
            break;

        case AML_IWP_SET_FETCH_PKT_METHOD:
            wnet_vif->vm_mainsta->sta_fetch_pkt_method = (unsigned char)set;
            pr_debug("iwpriv set pkt method %d\n", set);
            break;

        case AML_IWP_SET_FRAG_THRESHOLD:
            if ((unsigned short)set > 0) {
                wnet_vif->vm_fragthreshold = (unsigned short)set;
            }
            pr_debug("iwpriv set frag thr %d\n", wnet_vif->vm_fragthreshold);
            break;

        case AML_IWP_SET_PREAMBLE_TYPE:
            phy_set_preamble_type((unsigned char)set);
            pr_debug("iwpriv set preamble type %d\n", set);
            break;

        case AML_IWP_SET_FIX_BAND:
            aml_iwpriv_set_band(set);
            break;

        case AML_IWP_SET_GAIN:
            aml_iwpriv_set_initial_gain_change_hang(set);
            break;

        case AML_IWP_SET_TPC:
            aml_iwpriv_set_tx_power_change_hang(set);
            break;

        case AML_IWP_SET_TXPW_PLAN:
            wifimac_set_tx_pwr_plan(set);
            break;

        case AML_IWP_SET_MAC_MODE:
            aml_iwpriv_set_mac_mode(set);
            break;
    }

    return 0;
}

static int aml_iwpriv_send_para2(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{

    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;

    int *param = (int *)extra;
    int sub_cmd = param[0];
    int set1 = param[1];
    int set2 = param[2];
    int legacy_set = 0;


    pr_debug("%s, sub_cmd %d, value %d %d\n", __func__,param[0], param[1], param[2]);

    wifimac = wifi_mac_get_mac_handle();
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    switch (sub_cmd) {
        case AML_IWP_SET_REG:
            if ((set1 == 0x7fffffff) || (set2 == 0x7fffffff)) {
                pr_debug("********************************************************************************************************\n");
                pr_debug("You are using a legacy iwpriv tool, strongly suggest using a latest iwpriv tool\n");
                pr_debug("You also can using the legacy tool as below:\n iwpriv wlan0 set_reg 0xff000c80 0xff000c80 -> iwpriv wlan0 set_reg_legacy 0xff00 0x00c80 0xff0 0x00c80\n");
                pr_debug("********************************************************************************************************\n");
        }
        set_reg(wnet_vif, set1, set2);
        break;

        case AML_IWP_LEGACY_GET_REG:
            legacy_set = set2 | set1 << 16;
            get_reg(wnet_vif, legacy_set);
            break;
#if defined(SU_BF) || defined(MU_BF)
        case AML_IWP_SET_BEAMFORMING:
            aml_set_beamforming(wnet_vif, set1,set2);
            break;
#endif
    }

    return 0;
}


static int aml_iwpriv_set_reg_legacy(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{

    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;

    int *param = (int *)extra;
    int sub_cmd = 0;
    int set0 = param[0];
    int set1 = param[1];
    int set2 = param[2];
    int set3 = param[3];
    int legacy_set1 = 0;
    int legacy_set2 = 0;


    pr_debug("%s, sub_cmd %d, value1 %d, value2 %d, value3 %d\n", __func__,param[0], param[1], param[2], param[3]);

    wifimac = wifi_mac_get_mac_handle();
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    sub_cmd = AML_IWP_LEGACY_SET_REG;
    switch (sub_cmd) {
        case AML_IWP_LEGACY_SET_REG:
            legacy_set1 = set1 | set0 << 16;
            legacy_set2 = set3 | set2 << 16;
            set_reg(wnet_vif, legacy_set1, legacy_set2);
            break;
    }
    return 0;
}


static int aml_iwpriv_get(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{

    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;
    struct wifi_channel *c = NULL;
    int *param = (int *)extra;
    int sub_cmd = param[0];
    int i = 0;
    char buf[30] = {0};
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;

    pr_debug("%s, sub cmd %d\n", __func__, param[0]);

    wifimac = wifi_mac_get_mac_handle();
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    /*if we need feed back the value to user space, we need these 2 lines code, this is a sample*/
    //wrqu->data.length = sizeof(int);
    //*param = 110;
    /*if we need feed back the value to user space, we need these 2 lines code, this is a sample*/

    switch (sub_cmd) {
        case AML_IWP_AMSDU_STATE:
            aml_ap_get_amsdu_state();
            break;

        case AML_IWP_AMPDU_STATE:
            aml_ap_get_ampdu_state();
            break;

        case AML_IWP_UDP_INFO:
            for (i = 0; i < udp_cnt; i++) {
                pr_debug("%s streamid=%d tx is %d, rx is %d\n", __func__, aml_udp_info[i].streamid, aml_udp_info[i].tx, aml_udp_info[i].rx);
            }
            aml_udp_timer.udp_timer_stop = 1;
            aml_udp_timer.run_flag = 0;
            udp_cnt = 0;
            os_timer_ex_del(&aml_udp_timer.udp_send_timeout, CANCEL_SLEEP);
            break;

        case AML_IWP_COUNTRY:
            pr_debug("country code: %s\n", wifimac->wm_country.iso);
            break;

        case AML_IWP_GET_DEV_SN:
            pr_debug("aml module SN is:%04x \n", efuse_manual_read(0xf));
            break;

        case AML_IWP_GET_WIFI_MAC:
            efuse_data_l = efuse_manual_read(0x1);
            efuse_data_h = efuse_manual_read(0x2);
            pr_debug("aml WIFI MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
                    (efuse_data_l & 0x00ff0000) >> 16,(efuse_data_l & 0xff00) >> 8,efuse_data_l & 0xff);
            break;

        case AML_IWP_GET_BT_MAC:
            efuse_data_l = efuse_manual_read(0x2);
            efuse_data_h = efuse_manual_read(0x3);
            pr_debug("aml BT MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff000000) >> 24, (efuse_data_h & 0x00ff0000) >> 16,
                    (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0xff,
                    (efuse_data_l & 0xff000000) >> 24,(efuse_data_l & 0x00ff0000) >> 16);
            break;

        case AML_IWP_CHAN_LIST:
            WIFI_CHANNEL_LOCK(wifimac);
            for (i = 0; i < wifimac->wm_nchans; i++) {
                c = &wifimac->wm_channels[i];
                pr_debug("channel:%d\t frequency:%d \t bandwidth:%dMHz \n", c->chan_pri_num, c->chan_cfreq1, ((1 << c->chan_bw) * 20));
            }
            WIFI_CHANNEL_UNLOCK(wifimac);
            break;

        case AML_IWP_CHIP_ID:
            snprintf(buf, 21, "get_chip_id 0x%x 0x%x",0x8, 0x9);
            pr_debug("%s: buf %s\n", __func__, buf);
            aml_get_chip_id(NULL, buf, 0);
            break;

        case AML_IWP_AP_IP:
            aml_get_ap_ip(wnet_vif, NULL, 0);
            break;

        case AML_IWP_ROAM_CHAN:
            aml_get_roaming_candidate_chans(wnet_vif, NULL, 0);
            break;

        case AML_IWP_SESSION:
            aml_sta_get_wfd_session(wnet_vif, NULL, 0);
            break;
#ifdef CONFIG_P2P
        case AML_IWP_P2P_DEV_ID:
            aml_get_p2p_device_addr(wnet_vif, buf, 0);
            break;
#endif
        case AML_IWP_WIFI_MAC:
            aml_get_wifi_mac_addr(wnet_vif, buf, 30);
            break;

        case AML_IWP_CCA_BUSY_CHECK:
            wifimac->drv_priv->drv_ops.cca_busy_check();
            break;

        case AML_IWP_PHY_STATISTIC:
            wifimac->drv_priv->drv_ops.phy_stc();
            break;

        case AML_IWP_PRINT_VERSION:
            print_driver_version();
            pr_debug("driver version: %s\n", DRIVERVERSION);
            AML_OUTPUT("=============bus type:%s fw_chip:%s=============\n",aml_wifi_get_bus_type(),aml_wifi_get_fw_type());
            break;

        case AML_IWP_GET_TX_STATUS:
            get_latest_tx_status(wifimac);
            break;

        case AML_IWP_ENABLE_FW_LOG:
            aml_iwpriv_enable_fw_log(wnet_vif);
            break;

        case AML_IWP_SET_RATE_AUTO:
            aml_iwpriv_set_rate_auto(wnet_vif);
            break;

        case AML_IWP_SET_PT_RX_STOP:
            wnet_vif->vif_ops.pt_rx_stop();
            break;

    }

    return 0;
}

static int aml_iwpriv_start_capture(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    dut_start_capture(0x00005e00);//start capture on 0x5e
    return 0;
}


static int aml_iwpriv_get_csi_info(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;
    csi_stream_t *csi_info = NULL;
    unsigned int band = 0;
    unsigned int csi_len = 0;
    unsigned int mac_mode = 0;
    short phase_incr = 0;
    unsigned int arr[8] = {0};
    static unsigned int pkg_idx = 0;

    AML_OUTPUT("%s start++\n", __func__);

    wifimac = wifi_mac_get_mac_handle();
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    if ((wnet_vif->vm_state != WIFINET_S_CONNECTED)
        || (wnet_vif->vm_curchan == WIFINET_CHAN_ERR)
        || (wnet_vif->vm_mainsta == NULL)) {

        ERROR_DEBUG_OUT("%s curchan or mainsta not available\n", __func__);
        return 0;
    }

    csi_info = ZMALLOC(sizeof(csi_stream_t), "csi_info", GFP_KERNEL);
    if (csi_info == NULL) {
        ERROR_DEBUG_OUT("%s no memory!\n", __func__);
        return 0;
    }

    /*get band*/
    if (WIFINET_IS_CHAN_5GHZ(wnet_vif->vm_curchan)) {
        band = 2;
    } else if (WIFINET_IS_CHAN_2GHZ(wnet_vif->vm_curchan)) {
        band = 1;
    } else {
        band = 0;
    }

    /*get snr and noise*/
    get_phy_stc_info(arr);

    /*get protocol mode*/
    if (wnet_vif->vm_mac_mode >= WIFINET_MODE_11AC) {
        mac_mode = CSI_FRAME_TYPE_11AC;
    } else if (wnet_vif->vm_mac_mode >= WIFINET_MODE_11N) {
        mac_mode = CSI_FRAME_TYPE_11n;
    } else if (wnet_vif->vm_mac_mode >= WIFINET_MODE_11G) {
        mac_mode = CSI_FRAME_TYPE_11BA;
    }

    /*get csi len*/
    if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH20) {
        csi_len = 56;
    } else if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH40) {
        csi_len = 114;
    } else if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH80) {
        csi_len = 242;
    }

    /*get phase_incr from bit 23:12 of reg 0x00a092a0 */
    phase_incr = wnet_vif->vif_ops.read_word(0x00a092a0);
    phase_incr = (phase_incr >> 12) & 0x0fff;

    pkg_idx++;
    memset(csi_info, 0, sizeof(csi_stream_t));
    csi_info->time_stamp = wnet_vif->vm_mainsta->bcn_stamp;
    WIFINET_ADDR_COPY(csi_info->mac_ra, wnet_vif->vm_myaddr);
    WIFINET_ADDR_COPY(csi_info->mac_ta, wnet_vif->vm_des_bssid);
    csi_info->frequency_band = band;
    csi_info->bw = wnet_vif->vm_bandwidth;
    csi_info->rssi = wnet_vif->vm_mainsta->sta_avg_bcn_rssi;
    csi_info->protocol_mode = mac_mode;
    csi_info->frame_type = wnet_vif->vm_mainsta->cur_fratype;
    csi_info->chain_num = 1;
    csi_info->csi_len = csi_len;
    csi_info->snr = arr[1];
    csi_info->primary_channel_index = wifi_mac_Mhz2ieee(wnet_vif->vm_curchan->chan_cfreq1, 0);
    csi_info->noise = arr[4];
    csi_info->phyerr = 0;
    csi_info->rate = wnet_vif->vm_mainsta->sta_vendor_rate_code & 0xf;
    csi_info->extra_information = 0;
    csi_info->agc_code = 0;
    csi_info->phase_incr = phase_incr;
    csi_info->channel = wnet_vif->vm_curchan->chan_pri_num;
    //csi_info->reserved = 0;
    csi_info->packet_idx = pkg_idx;
    iwp_stop_tbus_to_get_sram(csi_info->csi);

    wrqu->data.length = sizeof(csi_stream_t);
    if (copy_to_user(wrqu->data.pointer,  (void*)csi_info, wrqu->data.length)) {
        ERROR_DEBUG_OUT("copy to user failed %s %d\n", __func__, __LINE__);
        FREE(csi_info,"csi_info");
        return -EFAULT;
    }
    FREE(csi_info,"csi_info");
    AML_OUTPUT("%s end++\n", __func__);
    return 0;
}


static int aml_ap_set_udp_info(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    char **arg;
    int cmd_arg;
    char sep = ',';

    char buf[41] = {0};
    if (copy_from_user(buf, wrqu->data.pointer, 40)) {
        return -EFAULT;
    }
    buf[40] = '\0';
    pr_debug("%s: %s\n", __func__, buf);

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    wifi_mac_set_udp_info(arg);
    kfree(arg);

    return 0;

}

static int aml_ap_get_udp_info(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{

    int i = 0;
    for (i = 0; i < udp_cnt; i++) {
        pr_debug("%s streamid=%d tx is %d, rx is %d\n", __func__, aml_udp_info[i].streamid, aml_udp_info[i].tx, aml_udp_info[i].rx);
    }
    aml_udp_timer.udp_timer_stop = 1;
    aml_udp_timer.run_flag = 0;
    udp_cnt = 0;
    os_timer_ex_del(&aml_udp_timer.udp_send_timeout, CANCEL_SLEEP);

    return 0;
}

static int aml_set_country_code(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    pr_debug("%s, %s\n", __func__,extra);
    wifi_mac_set_country_code(extra);

    return 0;

}

static void aml_iwpriv_set_dev_sn(char* arg_iw)
{
    char **mac_cmd;
    int i,cmd_arg;
    char sep = ':';
    unsigned int efuse_data = 0;

    mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
    if (mac_cmd) {
        efuse_data = (simple_strtoul(mac_cmd[0],NULL,16) << 8) | (simple_strtoul(mac_cmd[1],NULL,16));
        for (i = 0; i < 16; i++) {
            if (efuse_data & (1 << i)) {
                efuse_manual_write(i, 0xf);
            }
        }
        pr_debug("iwpriv write module SN is: %02x %02x\n", ((efuse_data & 0xff00) >> 8), (efuse_data & 0x00ff));
    }
    kfree(mac_cmd);

}

static void aml_iwpriv_set_mac_addr(char* arg_iw)
{
    char **mac_cmd;
    int i,cmd_arg;
    char sep = ':';
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;

    mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
    if (mac_cmd) {
        efuse_data_l = (simple_strtoul(mac_cmd[2],NULL,16) << 24) | (simple_strtoul(mac_cmd[3],NULL,16) << 16)
                       | (simple_strtoul(mac_cmd[4],NULL,16) << 8) | simple_strtoul(mac_cmd[5],NULL,16);
        efuse_data_h = (simple_strtoul(mac_cmd[0],NULL,16) << 8) | (simple_strtoul(mac_cmd[1],NULL,16));
        for (i = 0; i < 32; i++) {
            if (efuse_data_l & (1 << i)) {
                efuse_manual_write(i, 1);
            }
        }
        for (i = 0; i < 16; i++) {
            if (efuse_data_h & (1 << i)) {
                efuse_manual_write(i, 2);
            }
        }
        pr_debug("iwpriv write WIFI MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
                (efuse_data_l & 0x00ff0000) >> 16,(efuse_data_l & 0xff00) >> 8,efuse_data_l & 0xff);
    }
    kfree(mac_cmd);
}

static void aml_iwpriv_set_bt_dev_id(char* arg_iw)
{
    char **mac_cmd;
    int i,cmd_arg;
    char sep = ':';
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;

    mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
    if (mac_cmd) {
        efuse_data_h = (simple_strtoul(mac_cmd[0],NULL,16) << 24) | (simple_strtoul(mac_cmd[1],NULL,16) << 16)
                       | (simple_strtoul(mac_cmd[2],NULL,16) << 8) | simple_strtoul(mac_cmd[3],NULL,16);
        efuse_data_l = (simple_strtoul(mac_cmd[4],NULL,16) << 24) | (simple_strtoul(mac_cmd[5],NULL,16) << 16);
        for (i = 0; i < 32; i++) {
            if (efuse_data_h & (1 << i)) {
                efuse_manual_write(i, 3);
            }
        }
        for (i = 16; i < 32; i++) {
            if (efuse_data_l & (1 << i)) {
                efuse_manual_write(i, 2);
            }
        }
        pr_debug("iwpriv write BT MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                (efuse_data_h & 0xff000000) >> 24, (efuse_data_h & 0x00ff0000) >> 16,
                (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0xff,
                (efuse_data_l & 0xff000000) >> 24,(efuse_data_l & 0x00ff0000) >> 16);
    }
    kfree(mac_cmd);
}

static int aml_set_dev_sn(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    pr_debug("%s, %s\n", __func__,extra);
    aml_iwpriv_set_dev_sn(extra);

    return 0;
}

static int aml_set_wifi_mac_addr(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    char buf[19] = {0};
    if (copy_from_user(buf, wrqu->data.pointer, 18)) {
        return -EFAULT;
    }
    buf[18] = '\0';
    pr_debug("%s: %s\n", __func__, buf);
    aml_iwpriv_set_mac_addr(buf);

    return 0;
}

static int aml_set_bt_dev_id(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    char buf[19] = {0};

    if (copy_from_user(buf, wrqu->data.pointer, 18)) {
        return -EFAULT;
    }
    buf[18] = '\0';
    pr_debug("%s: %s\n", __func__, buf);
    aml_iwpriv_set_bt_dev_id(buf);

    return 0;
}

static int aml_iwpriv_set_debug_switch(char *switch_str)
{
    int debug_switch = 0;
    if(strstr(switch_str,"_off")!=NULL)
        debug_switch = AML_DBG_OFF;
    else if(strstr(switch_str,"_on")!=NULL)
        debug_switch = AML_DBG_ON;
    else
        ERROR_DEBUG_OUT("input error\n");
    return debug_switch;

}

int aml_set_debug_modules(char *debug_str)
{
    if(debug_str == NULL || strlen(debug_str) <= 0) {
        ERROR_DEBUG_OUT("debug modules is NULL\n");
        return -1;
    }
    if(strstr(debug_str,"p2p")!=NULL) {
        if(aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~1;
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_P2P;
    } else if(strstr(debug_str,"mir")!=NULL) {
        if(aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(BIT(1));
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_RATE_CTR;
    } else if(strstr(debug_str,"wtx")!=NULL) {
        if(aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(BIT(2));
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_TX;
    } else if(strstr(debug_str,"htx")!=NULL) {
        if(aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(BIT(3));
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_HAL_TX;
    } else if(strstr(debug_str,"txe") != NULL) {
        if(aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(BIT(4));
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_TX_ERROR;
    } else if(strstr(debug_str,"scn") != NULL) {
        if(aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(BIT(5));
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_SCAN;
    } else {
        ERROR_DEBUG_OUT("input error\n");
    }
    return 0;
}

static int aml_iwpriv_set_debug(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    pr_debug("%s, %s\n", __func__,extra);
    aml_set_debug_modules(extra);
    return 0;

}

static int aml_get_country_code(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    pr_debug("country code: %s\n", wifimac->wm_country.iso);

    return 0;
}

static int aml_get_channel_list(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra) {
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct wifi_channel *c;
    int i = 0 ;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++) {
        c = &wifimac->wm_channels[i];
        pr_debug("channel:%d\t frequency:%d \t bandwidth:%dMHz \n", c->chan_pri_num, c->chan_cfreq1, ((1 << c->chan_bw) * 20));
    }
    WIFI_CHANNEL_UNLOCK(wifimac);

     return 0;
}

static int aml_ap_set_arp_rx(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    char **arg;
    int cmd_arg;
    char sep = ',';

    char buf[41] = {0};
    if (copy_from_user(buf, wrqu->data.pointer, 40)) {
        return -EFAULT;
    }
    buf[40] = '\0';
    pr_debug("%s: %s\n", __func__, buf);

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    wifi_mac_ap_set_arp_rx(arg);
    kfree(arg);

    return 0;
}

static int iw_standard_get_stats(struct net_device *dev, struct iw_request_info *info,
    union iwreq_data *wrqu, char *extra)
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct iw_statistics stats = {0};
    unsigned int arr[8] = {0};

    pr_debug("%s\n", __func__);

    wnet_vif = aml_iwpriv_get_vif(dev->name);
    if ((wnet_vif->vm_opmode != WIFINET_M_STA) && (wnet_vif->vm_state != WIFINET_S_CONNECTED)) {
        return 0;
    }

    get_phy_stc_info(arr);
    stats.qual.level = wnet_vif->vm_mainsta->sta_avg_bcn_rssi;
    stats.qual.noise = arr[4];
    stats.qual.qual = arr[1];
    stats.qual.updated = IW_QUAL_ALL_UPDATED;

    /* Copy statistics to extra */
    memcpy(extra, &stats, sizeof(struct iw_statistics));
    wrqu->data.length = sizeof(struct iw_statistics);

    return 0;
}

static void sap_change_channel(struct wlan_net_vif *wnet_vif, unsigned int channel)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_channel *c = NULL;
    int center_chan = 0;
    int i = 0;

    if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH20) {
        center_chan = channel;
    } else if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH40) {
        if (wnet_vif->scnd_chn_offset == WIFINET_HTINFO_EXTOFFSET_ABOVE)
            center_chan = channel + 2;
        else if (wnet_vif->scnd_chn_offset == WIFINET_HTINFO_EXTOFFSET_ABOVE)
            center_chan = channel - 2;
        else
            center_chan = channel;
    } else if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH80) {
        WIFI_CHANNEL_LOCK(wifimac);
        for (i = 0; i < wifimac->wm_nchans; i++) {
            c = &wifimac->wm_channels[i];
            if ((c->chan_pri_num== channel) && (c->chan_bw == WIFINET_BWC_WIDTH80)) {
                center_chan = wifi_mac_Ieee2mhz(c->chan_cfreq1, 0);
                break;
            }
        }
        WIFI_CHANNEL_UNLOCK(wifimac);
    }

    wifi_mac_set_wnet_vif_channel(wnet_vif, channel, wnet_vif->vm_bandwidth, center_chan, CHANNEL_CONNECT_FLAG | CHANNEL_RESTORE_FLAG);
}

static int iw_standard_sap_set_freq(struct net_device *dev, struct iw_request_info *info,
    union iwreq_data *wrqu, char *extra)
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;
    struct wifi_channel *c = NULL;
    unsigned int set_chl = 0;

    pr_debug("%s, freq.m:%d, freq.e:%d\n", __func__, wrqu->freq.m, wrqu->freq.e);

    wifimac = wifi_mac_get_mac_handle();

    /* Settings by Frequency as input */
    if(wrqu->freq.e == 1) {
        unsigned int freq = wrqu->freq.m / 100000;

        if ((wrqu->freq.m < (unsigned int)2.412e8) || (wrqu->freq.m > (unsigned int)5.825e8))
            return -EINVAL;

        c = wifi_mac_find_chan(wifimac, wifi_mac_Mhz2ieee(freq, 0), WIFINET_BWC_WIDTH20, wifi_mac_Mhz2ieee(freq, 0));
    }

    /* Settings by Channel as input */
    if (wrqu->freq.e == 0) {
        unsigned int chan = wrqu->freq.m;

        if((wrqu->freq.m < 1) || (wrqu->freq.m > 165))
            return -EINVAL;

        c = wifi_mac_find_chan(wifimac, chan, WIFINET_BWC_WIDTH20, chan);
    }

    if (c == NULL) {
        return -EINVAL;
    }

    set_chl = c->chan_pri_num;
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    pr_debug("%s, opmode:%d, nrunning:%d\n", __func__, wnet_vif->vm_opmode, wifimac->wm_nrunning);
    if ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) && (wifimac->wm_nrunning == 1)) {
        sap_change_channel(wnet_vif, set_chl);
    }

    return 0;
}

void wifi_mac_pwrsave_set_inactime(struct wlan_net_vif *wnet_vif, unsigned int time)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (wnet_vif->vm_opmode != WIFINET_M_STA) {
        return;
    }

    pr_debug("<running> %s %d, time %d\n",__func__,__LINE__, time);

    if(time == 0) {
        wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_NONE);
        return;
    }

    wnet_vif->vm_pwrsave.ips_inactivitytime = time;

    if (wnet_vif->vm_state == WIFINET_S_CONNECTED && wifimac->wm_syncbeacon == 0) {
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        wifi_mac_beacon_sync(wifimac->drv_priv->wmac, wnet_vif->wnet_vif_id);
    }

    if (time >= WIFINET_PS_LOW_INACTIVITYTIME) {
        wnet_vif->vm_pwrsave.ips_sta_psmode = WIFINET_PWRSAVE_LOW;
    } else {
        wnet_vif->vm_pwrsave.ips_sta_psmode = WIFINET_PWRSAVE_NORMAL;
    }
    os_timer_ex_start_period(&wnet_vif->vm_pwrsave.ips_timer_presleep, wnet_vif->vm_pwrsave.ips_inactivitytime);
}


static int iw_standard_set_pwr(struct net_device *dev, struct iw_request_info *info,
    union iwreq_data *wrqu, char *extra)
{
    struct wlan_net_vif *wnet_vif = NULL;
    unsigned int time = 0;
    wnet_vif = aml_iwpriv_get_vif(dev->name);

    pr_debug("%s, disable:%d\n", __func__, wrqu->power.disabled);
    if (wrqu->power.disabled) {
        wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_NONE);
        if (wnet_vif->vm_wdev) {
            wnet_vif->vm_wdev->ps = 0;
        }
        return 0;
    } else {
        if (wrqu->power.flags & IW_POWER_PERIOD) {
            time = wrqu->power.value / 1000;
            pr_debug("%s, pwr perio value: %d\n", __func__, wrqu->power.value / 1000);
        }

        switch (wrqu->power.flags & IW_POWER_MODE) {
            case IW_POWER_UNICAST_R:
            case IW_POWER_MULTICAST_R:
            case IW_POWER_ALL_R:
                pr_debug("%s, pwr mode value: %d\n", __func__, wrqu->power.value / 1000);
                break;

            case IW_POWER_ON:
                break;

            default:
                return 0;
        }

        wifi_mac_pwrsave_set_inactime(wnet_vif, time);
        if (wnet_vif->vm_wdev) {
            wnet_vif->vm_wdev->ps = 1;
        }
    }

    return 0;
}

static int iw_standard_get_essid(struct net_device *dev,
                                 struct iw_request_info *info,
                                 union iwreq_data *wrqu, char *extra)
{
    struct wifi_mac_ScanSSID *ssid;
    struct wlan_net_vif *wnet_vif;
    size_t ssid_len;

    wnet_vif = aml_iwpriv_get_vif(dev->name);

    if (!wnet_vif)
        return -EFAULT;

    ssid = &wnet_vif->vm_des_ssid[0];
    ssid_len = min(ssid->len, IW_ESSID_MAX_SIZE);

    /* Both 'extra' and 'wrqu' are kernel memory. */
    memcpy(extra, ssid->ssid, ssid_len);
    extra[ssid_len] = '\0';
    wrqu->essid.length = ssid_len;

    return 0;
}

static int iw_standard_get_ap_addr(struct net_device *dev,
                                   struct iw_request_info *info,
                                   union iwreq_data *wrqu, char *extra)
{
    struct wlan_net_vif *wnet_vif;

    wnet_vif = aml_iwpriv_get_vif(dev->name);

    if (!wnet_vif)
        return -EFAULT;

    BUILD_BUG_ON(sizeof(wnet_vif->vm_des_bssid) != ETH_ALEN);

    /* 'wrqu->ap_addr.sa_data' is kernel memory. */
    wrqu->ap_addr.sa_family = ARPHRD_ETHER;
    memcpy(wrqu->ap_addr.sa_data, wnet_vif->vm_des_bssid,
               sizeof(wnet_vif->vm_des_bssid));

    return 0;
}

static const iw_handler standard_handler[] = {
    IW_HANDLER(SIOCGIWSTATS,    (iw_handler)iw_standard_get_stats),
    IW_HANDLER(SIOCSIWFREQ,     (iw_handler)iw_standard_sap_set_freq),
    IW_HANDLER(SIOCSIWPOWER,    (iw_handler)iw_standard_set_pwr),
    IW_HANDLER(SIOCGIWESSID,    (iw_handler)iw_standard_get_essid),
    IW_HANDLER(SIOCGIWAP,       (iw_handler)iw_standard_get_ap_addr),
};


#if defined(CONFIG_WEXT_PRIV) || LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32)
static iw_handler aml_iwpriv_private_handler[] = {
    aml_ap_send_addba_req,
    aml_iwpriv_send_para1,
    //NULL,
    /*if we need feed back the value to user space, we need jump command for large buffer*/
    aml_iwpriv_get,
    NULL,
    aml_ap_set_udp_info,
    NULL,
    aml_ap_set_arp_rx,
    NULL,
    aml_ap_set_amsdu_state,
    aml_ap_set_ampdu_state,
    aml_set_country_code,
    aml_iwpriv_send_para2,
    aml_iwpriv_set_reg_legacy,
    aml_set_dev_sn,
    aml_set_wifi_mac_addr,
    aml_set_bt_dev_id,
    aml_iwpriv_set_debug,
    aml_iwpriv_start_capture,
    aml_iwpriv_get_csi_info,
};

static const struct iw_priv_args aml_iwpriv_private_args[] = {
/*iwpriv set command, there is more parameters*/
{
    SIOCIWFIRSTPRIV,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 7, 0, "set_addba_req"},

/*iwpriv set command, there is one parameters*/
{
    SIOCIWFIRSTPRIV + 1,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, ""},
{
    AML_IWP_11H,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_11h"},
{
    AML_IWP_ROAM_THRESH_2G,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_roam_2gthr"},
{
    AML_IWP_ROAM_THRESH_5G,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_roam_5gthr"},
{
    AML_IWP_ROAM_MODE,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_roam_mode"},
{
    AML_IWP_MARK_DFS_CHAN,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_dfs_mark"},
{
    AML_IWP_UNMARK_DFS_CHAN,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_dfs_unmark"},
{
    AML_IWP_WEAK_THR_NARROW,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_nb_thr"},
{
    AML_IWP_WEAK_THR_WIDE,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_wb_thr"},
{
    AML_IWP_EAT_COUNT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_eat_count"},
{
    AML_IWP_AGGR_THRESH,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_aggr_thr"},
{
    AML_IWP_HEART_INTERVAL,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_hrt_intv"},
{
    AML_IWP_BSS_COEX,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_bss_coex"},
{
    AML_IWP_TXAGGR_STA,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_txaggr_sta"},
{
    AML_IWP_WMM_AC_DELTS,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_wmm_delts"},
{
    AML_IWP_SHORT_GI,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_short_gi"},
{
    AML_IWP_DYNAMIC_BW,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_dynamic_bw"},
{
    AML_IWP_GET_REG,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_reg"},
{
    AML_IWP_SET_BCN_INTERVAL,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_bcn_intv"},
{
    AML_IWP_SET_LDPC,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_ldpc"},
{
    AML_IWP_SET_SCAN_TIME_IDLE,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_sc_idltime"},
{
    AML_IWP_SET_SCAN_TIME_CONNECT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_sc_contime"},
{
    AML_IWP_SET_SCAN_HANG,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_sc_hang"},
{
    AML_IWP_EN_BTWIFI_COEX,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_coex_btwifi"},
{
    AML_IWP_SET_COEXIST_MAX_MISS_BCN_CNT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_coex_bcnmis"},
{
    AML_IWP_SET_COEXIST_REQ_TIMEOUT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_wifi_slcreq"},
{
    AML_IWP_SET_COEXIST_NOT_GRANT_WEIGHT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_wifi_grtwgt"},
{
    AML_IWP_SET_RATE_LEGACY,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_rate_legacy"},
{
    AML_IWP_SET_RATE_HT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_rate_ht"},
{
    AML_IWP_SET_RATE_VHT,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_rate_vht"},
{
    AML_IWP_SET_POWER,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_pwr_save"},
{
    AML_IWP_SET_CHL_RSSI,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_chl_rssi"},
{
    AML_IWP_SET_BURST,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_burst"},
{
    AML_IWP_SET_UAPSD,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_uapsd"},
{
    AML_IWP_SET_PT_RX_START,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_pt_rxstart"},
{
    AML_IWP_SET_SCAN_PRI,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_scan_pri"},
{
    AML_IWP_SET_BE_BK_NOQOS_PRI,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_bebk_pri"},
{
    AML_IWP_SET_FETCH_PKT_METHOD,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_pkt_fetch"},
{
    AML_IWP_SET_FRAG_THRESHOLD,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_frag_thr"},
{
    AML_IWP_SET_PREAMBLE_TYPE,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_preamble"},
{
    AML_IWP_SET_FIX_BAND,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_band"},
{
    AML_IWP_SET_GAIN,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_gain_hang"},
{
    AML_IWP_SET_TPC,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tpc_hang"},
{
    AML_IWP_SET_TXPW_PLAN,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_txpwr_plan"},
{
    AML_IWP_SET_MAC_MODE,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_mac_mode"},


/*iwpriv get command*/
{
    /*if we need feed back the value to user space, we need jump command for large buffer*/
    SIOCIWFIRSTPRIV + 2,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, ""},
{
    AML_IWP_AMSDU_STATE,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_amsdu"},
{
    AML_IWP_AMPDU_STATE,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_ampdu"},
{
    AML_IWP_UDP_INFO,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_udp_info"},
{
    AML_IWP_COUNTRY,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_country"},
{
    AML_IWP_CHAN_LIST,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_chan_list"},
{
    AML_IWP_CHIP_ID,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_chip_id"},
{
    AML_IWP_AP_IP,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "set_arp_req"},
{
    AML_IWP_ROAM_CHAN,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_roam_chan"},
{
    AML_IWP_SESSION,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_wfd_session"},
#ifdef CONFIG_P2P
{
    AML_IWP_P2P_DEV_ID,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_p2p_dev_id"},
#endif
{
    AML_IWP_WIFI_MAC,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_wifi_mac"},
{
    AML_IWP_CCA_BUSY_CHECK,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_cca_stat"},
{
    AML_IWP_PHY_STATISTIC,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_agc_stat"},
{
    AML_IWP_PRINT_VERSION,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_drv_ver"},
{
    AML_IWP_GET_TX_STATUS,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_tx_stat"},
{
    AML_IWP_ENABLE_FW_LOG,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "set_fwlog_en"},
{
    AML_IWP_SET_RATE_AUTO,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "set_rate_auto"},
{
    AML_IWP_SET_PT_RX_STOP,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "set_pt_rxstop"},
{
    AML_IWP_GET_DEV_SN,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_dev_sn"},
{
    AML_IWP_GET_WIFI_MAC,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_mac_addr"},
{
    AML_IWP_GET_BT_MAC,
    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_bt_dev_id"},


{
    SIOCIWFIRSTPRIV + 4,
    IW_PRIV_TYPE_CHAR | 40, 0, "set_udp_info"},
{
    SIOCIWFIRSTPRIV + 6,
    IW_PRIV_TYPE_CHAR | 40, 0, "set_arp_rx"},
{
    SIOCIWFIRSTPRIV + 8,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 3, 0, "set_amsdu"},
{
    SIOCIWFIRSTPRIV + 9,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 3, 0, "set_ampdu"},
{
    SIOCIWFIRSTPRIV + 10,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 3, 0, "set_country"},




/*iwpriv set command, there is 2 parameters*/
{
    SIOCIWFIRSTPRIV + 11,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, ""},
{
    AML_IWP_SET_REG,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_reg"},
{
    AML_IWP_LEGACY_GET_REG,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "get_reg_legacy"},





#if defined(SU_BF) || defined(MU_BF)
{
    AML_LWP_SET_BEAMFORMING,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_beamforming"},
#endif


    /*iwpriv set command, there is 4 parameters*/
{
    SIOCIWFIRSTPRIV + 12,
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 4, 0, "set_reg_legacy"},

{
    SIOCIWFIRSTPRIV + 13,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 6,0, "set_dev_sn"},

{
    SIOCIWFIRSTPRIV + 14,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18,0, "set_mac_addr"},
{
    SIOCIWFIRSTPRIV + 15,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18,0, "set_bt_dev_id"},
{
    SIOCIWFIRSTPRIV + 16,
    IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 7, 0, "set_debug"},
{
     SIOCIWFIRSTPRIV + 17,
    0, IW_PRIV_TYPE_CHAR | 0, "set_capture"},
{
     SIOCIWFIRSTPRIV + 18,
    0, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_MASK, "get_csi_info"},

};
#endif


#ifdef CONFIG_WIRELESS_EXT
struct iw_handler_def w1_iw_handle = {
#if defined(CONFIG_WEXT_PRIV) || LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32)
    .num_standard = sizeof(standard_handler) / sizeof(standard_handler[0]),
    .num_private = ARRAY_SIZE(aml_iwpriv_private_handler),
    .num_private_args = ARRAY_SIZE(aml_iwpriv_private_args),
    .standard = (iw_handler *)standard_handler,
    .private = aml_iwpriv_private_handler,
    .private_args = aml_iwpriv_private_args,
#endif
};
#endif
