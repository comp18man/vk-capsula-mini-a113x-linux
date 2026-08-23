#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include "wifi_mac_com.h"
#include "wifi_cmd_func.h"
#include "wifi_drv_reg_ops.h"
#include "wifi_cfg80211.h"
#include "wifi_mac_if.h"
#include "wifi_iwpriv_cmd.h"


struct udp_info aml_udp_info[32];
int udp_cnt = 0;
struct udp_timer aml_udp_timer;
extern struct _B2B_Test_Case_Packet gB2BTestCasePacket;
extern unsigned char g_tx_power_change_disable;
extern unsigned char g_initial_gain_change_disable;

cmd_to_func_table_t cmd_to_func[] =
{
    {"set_chip_id", aml_set_chip_id},
    {"get_chip_id", aml_get_chip_id},
    {"set_mac_addr", aml_set_mac_addr},
    {"get_mac_addr", aml_get_mac_addr},
    {"get_wifi_mac", aml_get_wifi_mac_addr},
    {"set_bt_dev_id", aml_set_bt_device_id},
    {"get_bt_dev_id", aml_get_bt_device_id},
    {"sta_send_bareq", aml_sta_send_addba_req},
    {"set_bss_coex", aml_sta_send_coexist_mgmt},
    {"get_txaggr_sta", aml_wpa_get_txaggr_status},
    {"sta_get_session", aml_sta_get_wfd_session},
    {"wmm_ac_addts",  aml_wmm_ac_addts},
    {"wmm_ac_delts",  aml_wmm_ac_delts},
#ifdef CONFIG_P2P
    {"get_p2p_dev_id", aml_get_p2p_device_addr},
#endif
    {"set_amsdu", aml_set_mac_amsdu},
    {"set_ampdu", aml_set_drv_ampdu},
    {"update_wmm_arg", aml_update_wmm_arg},
    {"set_dynamic_bw", aml_wpa_set_dynamic_bw},
    {"set_short_gi", aml_wpa_set_short_gi},
    {"set_eat_count", aml_set_eat_count_max},
    {"set_aggr_thr", aml_set_aggr_thresh},
    {"set_hrt_intv", aml_set_hrtimer_interval},
    {"get_ap_ip", aml_get_ap_ip},
    {"set_roam_2gthr", aml_set_roaming_threshold_2g}, /* wpa_cli driver set_roam_thr_2g -80 */
    {"set_roam_5gthr", aml_set_roaming_threshold_5g},
    {"get_roam_chan", aml_get_roaming_candidate_chans},
    {"set_roam_chan", aml_set_roaming_candidate_chans},
    {"set_roam_mode", aml_set_roaming_mode},
    {"set_udp_info", aml_set_udp_info},
    {"get_udp_info", aml_get_udp_info},
    {"set_dfs_mark",aml_mark_dfs_channel},
    {"set_dfs_unmark",aml_unmark_dfs_channel},
    {"set_dev_sn", aml_set_device_sn},
    {"get_dev_sn", aml_get_device_sn},
    {"set_nb_thr", aml_set_signal_power_weak_thresh_for_narrow_bandwidth},
    {"set_wb_thr", aml_set_signal_power_weak_thresh_for_wide_bandwidth},
    {"set_rate_legacy", aml_set_lagecy_bitrate_mask},
    {"set_rate_ht", aml_set_ht_bitrate_mask},
    {"set_rate_vht", aml_set_vht_bitrate_mask},
    {"set_rate_auto", aml_set_rate_auto},
    {"set_wifi_slcreq", aml_set_coex_req_timeslice_timeout},
    {"set_coex_bcnmis", aml_set_coex_max_miss_bcn},
    {"set_sc_hang", aml_set_scan_hang},
    {"set_sc_contime", aml_set_scan_connect_time},
    {"set_sc_idltime", aml_set_scan_idle_time},
    {"set_reg", aml_wpa_set_reg},
    {"get_reg", aml_wpa_get_reg},
    {"get_cca_stat", aml_wpa_get_cca_status},
    {"get_agc_stat", aml_wpa_get_agc_status},
    {"get_tx_stat", aml_wpa_get_latest_tx_status},
    {"get_drv_ver", aml_wpa_get_drv_ver},
    {"set_country", aml_wpa_set_country_code},
    {"get_country", aml_wpa_get_country_code},
    {"set_pwr_save", aml_wpa_set_power},
    {"set_chl_rssi", aml_wpa_set_channel_rssi},
    {"set_burst", aml_wpa_set_burst},
    {"set_coex_btwifi", aml_wpa_set_coex_btwifi},
    {"set_coex_bcnmis", aml_wpa_set_coex_bcnmis},
    {"set_bcn_intv", aml_wpa_set_bcn_intv},
    {"set_ldpc", aml_wpa_set_ldpc},
#if defined(SU_BF) || defined(MU_BF)
    {"set_beamforming", aml_wpa_set_beamforming},
#endif
    {"get_chan_list", aml_wpa_get_chan_list},
    {"set_uapsd", aml_wpa_set_uapsd},
    {"set_pt_rxstart", aml_wpa_set_pt_rxstart},
    {"set_pt_rxstop", aml_wpa_set_pt_rxstop},
    {"set_scan_pri", aml_wpa_set_scan_pri},
    {"set_bebk_pri", aml_wpa_set_bebk_pri},
    {"set_pkt_fetch", aml_wpa_set_pkt_fetch},
    {"set_frag_thr", aml_wpa_set_frag_thr},
    {"set_preamble", aml_wpa_set_preamble},
    {"set_band", aml_wpa_set_band},
    {"set_mac_mode", aml_wpa_set_mac_mode},
    {"set_gain_hang", aml_wpa_set_initial_gain_change_hang},
    {"set_tpc_hang", aml_wpa_set_tx_power_change_hang},
    {"set_tx_pw_plan", aml_set_tx_power_plan},
    {"set_debug", aml_wpa_set_debug},
    {"", NULL},
};

/* Returns a char * arr [] and size is the length of the returned array */
char **aml_cmd_char_phrase(char sep, const char *str, int *size)
{
    int count = 0;
    int i;
    char **ret;
    int lastindex = -1;
    int j = 0;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == sep) {
            count++;
        }
    }

    ret = (char **)kzalloc((++count) * sizeof(char *), GFP_KERNEL);

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == sep) {
            /* kzalloc the memory space of substring length + 1 */
            ret[j] = (char *)kzalloc((i - lastindex) * sizeof(char), GFP_KERNEL);
            memcpy(ret[j], str + lastindex + 1, i - lastindex - 1);
            j++;
            lastindex = i;
        }
    }

    /* Processing the last substring */
    if (lastindex <= strlen(str) - 1) {
        ret[j] = (char *)kzalloc((strlen(str) - lastindex) * sizeof(char), GFP_KERNEL);
        memcpy(ret[j], str + lastindex + 1, strlen(str) - 1 - lastindex);
        j++;
    }

    *size = j;

    return ret;
}

/*
 *  cmd: sta_send_bareq 11:22:33:44:55:66 5
 */
int aml_sta_send_addba_req(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int skip;
    char mac[18];
    int tid = 0;

    skip = strlen("sta_send_bareq") + 1;
    sscanf(buf + skip, "%s %d", mac, &tid);
    wifi_mac_send_addba_req(buf + skip, tid);

    return 0;
}

int aml_set_chip_id(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        int i;

        for (i = 0; i < 32; i++) {
            if (simple_strtoul(arg[2], NULL, 16) & (1 << i)) {
                efuse_manual_write(i, 8);
            }
        }

        for (i = 0; i < 16; i++) {
            if (simple_strtoul(arg[1], NULL, 16) & (1 << i)) {
                efuse_manual_write(i, 9);
            }
        }

        pr_debug("write chip_id is :%04x%08x\n",
               simple_strtoul(arg[1], NULL, 16) & 0xffff,
               simple_strtoul(arg[2], NULL, 16));

        FREE(arg, "cmd_arg");
    }

    return 0;
}

int aml_get_chip_id(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        unsigned int efuse_data_l;
        unsigned int efuse_data_h;

        efuse_data_l = efuse_manual_read(simple_strtoul(arg[1], NULL, 16));
        efuse_data_h = efuse_manual_read(simple_strtoul(arg[2], NULL, 16));

        pr_debug("efuse addr:%08x,%08x, chip_id is :%04x%08x\n",
               simple_strtoul(arg[1], NULL, 16), simple_strtoul(arg[2], NULL, 16),
               efuse_data_h & 0xffff, efuse_data_l);
        FREE(arg, "cmd_arg");
    }

    return 0;
}

int aml_set_mac_addr(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        char **mac_cmd;

        sep = ':';
        mac_cmd = aml_cmd_char_phrase(sep, arg[1], &cmd_arg);
        if (mac_cmd) {
            unsigned int efuse_data_l;
            unsigned int efuse_data_h;
            int i;

            efuse_data_l = (simple_strtoul(mac_cmd[2], NULL, 16) << 24) |
                           (simple_strtoul(mac_cmd[3], NULL, 16) << 16) |
                           (simple_strtoul(mac_cmd[4], NULL, 16) << 8) |
                           simple_strtoul(mac_cmd[5], NULL, 16);
            efuse_data_h = (simple_strtoul(mac_cmd[0], NULL, 16) << 8) |
                           (simple_strtoul(mac_cmd[1], NULL, 16));

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

            pr_debug("write MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                   (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0x00ff,
                   (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16,
                   (efuse_data_l & 0xff00) >> 8, efuse_data_l & 0xff);
        }

        kfree(mac_cmd);
        FREE(arg, "cmd_arg");
    }

    return 0;
}

int aml_get_mac_addr(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        unsigned int efuse_data_l;
        unsigned int efuse_data_h;

        efuse_data_l = efuse_manual_read(simple_strtoul(arg[1], NULL, 16));
        efuse_data_h = efuse_manual_read(simple_strtoul(arg[2], NULL, 16));
        pr_debug("efuse addr:%08x,%08x, MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                simple_strtoul(arg[1], NULL, 16), simple_strtoul(arg[2], NULL, 16),
                (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0x00ff,
                (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16,
                (efuse_data_l & 0xff00) >> 8, efuse_data_l & 0xff);
        FREE(arg, "cmd_arg");
    }

    return 0;
}

int aml_get_wifi_mac_addr(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int bytes_written;

    bytes_written = snprintf(buf, len, MAC_FMT, MAC_ARG(wnet_vif->vm_myaddr));
    pr_debug("%s,buf:  %s\n", __func__, buf);

    return bytes_written;
}

int aml_set_bt_device_id(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        char **mac_cmd;

        sep = ':';
        mac_cmd = aml_cmd_char_phrase(sep, arg[1], &cmd_arg);
        if (mac_cmd) {
            unsigned int efuse_data_l;
            unsigned int efuse_data_h;
            int i;

            efuse_data_h = (simple_strtoul(mac_cmd[0], NULL, 16) << 24) |
                           (simple_strtoul(mac_cmd[1], NULL, 16) << 16) |
                           (simple_strtoul(mac_cmd[2], NULL, 16) << 8) |
                           simple_strtoul(mac_cmd[3], NULL, 16);
            efuse_data_l = (simple_strtoul(mac_cmd[4], NULL, 16) << 24) |
                           (simple_strtoul(mac_cmd[5], NULL, 16) << 16);

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
            pr_debug("write BT device id  is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff000000) >> 24, (efuse_data_h & 0x00ff0000) >> 16,
                    (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0xff,
                    (efuse_data_l & 0xff000000) >> 24,(efuse_data_l & 0x00ff0000) >> 16);
        }

        kfree(mac_cmd);
        FREE(arg, "cmd_arg");
    }

    return 0;

}

int aml_get_bt_device_id(struct wlan_net_vif *wnet_vif, char* buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        unsigned int efuse_data_l;
        unsigned int efuse_data_h;

        efuse_data_l = efuse_manual_read(simple_strtoul(arg[1], NULL, 16));
        efuse_data_h = efuse_manual_read(simple_strtoul(arg[2], NULL, 16));
        pr_debug("efuse addr:%08x,%08x, get BT device id is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                simple_strtoul(arg[1], NULL, 16), simple_strtoul(arg[2], NULL, 16),
                (efuse_data_h & 0xff000000) >> 24, (efuse_data_h & 0x00ff0000) >> 16,
                (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0xff,
                (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16);
        FREE(arg, "cmd_arg");
    }

    return 0;
}

int aml_sta_send_coexist_mgmt(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int skip;

    skip = strlen("send_bss_coex") + 1;
    wifi_mac_send_coexist_mgmt(buf + skip);

    return 0;
}

int aml_wpa_get_txaggr_status(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char* str[2] = {"all", "reset"};
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    len = strlen("get_txaggr_sta ") + strlen(str[data]) + 1;
    snprintf(buf, len, "get_txaggr_sta %s", str[data]);
    pr_debug("%s: buf %s\n", __func__, buf);
    aml_get_drv_txaggr_status(wnet_vif, buf, len);

    FREE(arg, "cmd_arg");
    return 0;
}


int aml_get_drv_txaggr_status(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int i, j, cmd_arg;
    unsigned int nframes;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);

    /* get queue-0 statistics */
    if (likely(!arg[1])) {
        i = 0;

        for (j = 0; j < 16; j++) {
            nframes = drv_priv->drv_txlist_table[i].tx_aggr_status[j];
            pr_debug("Q:%d; aggr %d packets times: %d\n", i, j + 1, nframes);
        }
        pr_debug("\n");

    } else {
        /* get all statistics */
        if (strnicmp(arg[1], "all", strlen("all")) == 0) {
            for (i = 0; i < HAL_NUM_TX_QUEUES; i++) {
                for (j = 0; j < 16; j++) {
                    nframes = drv_priv->drv_txlist_table[i].tx_aggr_status[j];
                    pr_debug("Q:%d; aggr %d packets times: %d\n", i, j + 1, nframes);
                }
                pr_debug("\n");
            }
        }
        /* reset */
        if (strnicmp(arg[1], "reset", strlen("reset")) == 0) {
            for (i = 0; i < HAL_NUM_TX_QUEUES; i++) {
                for (j = 0; j < 16; j++) {
                    drv_priv->drv_txlist_table[i].tx_aggr_status[j] = 0;
                }
            }
        }
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_sta_get_wfd_session(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    pr_debug("%s, wfd_session_id=%s\n", __func__, wnet_vif->vm_p2p->wfd_session_id);
    return 0;
}

int aml_get_p2p_device_addr(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int bytes_written;

    memcpy(buf, wnet_vif->vm_ndev->dev_addr, ETH_ALEN);
    pr_debug("%s,p2p_device_addr= %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
           *buf, *(buf + 1), *(buf + 2), *(buf + 3), *(buf + 4), *(buf + 5));

    bytes_written = ETH_ALEN;

    return bytes_written;

}

int aml_wmm_ac_addts(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int skip, cmd_arg;
    char **arg;
    char sep = ' ';

    skip = strlen("wmm_ac_addts") + 1;

    arg = aml_cmd_char_phrase(sep, buf + skip, &cmd_arg);
    wifi_mac_send_wmm_ac_addts(arg);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wmm_ac_delts(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int skip;

    skip = strlen("wmm_ac_delts") + 1;
    wifi_mac_send_wmm_ac_delts(buf + skip);

    return 0;
}

void aml_set_mac_amsdu_switch_state(char *arg)
{
    struct drv_private *drv_priv = drv_get_drv_priv();

    if (!arg) {
        if (drv_priv->drv_config.cfg_txamsdu) {
            pr_debug("AMSDU is ON.\n");
        }
        else {
            pr_debug("AMSDU is OFF.\n");
        }
    }
    else {
        if (strnicmp(arg, "on", strlen("on")) == 0) {
            drv_priv->drv_config.cfg_txamsdu = 1;
            pr_debug("AMSDU set ON!\n");

        }
        else if (strnicmp(arg, "off", strlen("off")) == 0) {
            drv_priv->drv_config.cfg_txamsdu = 0;
            pr_debug("AMSDU set OFF!\n");

        }
        else {
            if (drv_priv->drv_config.cfg_txamsdu) {
                pr_debug("AMSDU is ON.\n");
            }
            else {
                pr_debug("AMSDU is OFF.\n");
            }
        }
    }
}

void aml_set_drv_ampdu_switch_state(char *arg)
{
    struct drv_private *drv_priv = drv_get_drv_priv();

    if (!arg) {
        if (drv_priv->drv_config.cfg_txaggr) {
            pr_debug("AMPDU is ON.\n");
        }
        else {
            pr_debug("AMPDU is OFF.\n");
        }
    }
    else {
        if (strnicmp(arg, "on", strlen("on")) == 0) {
            drv_priv->drv_config.cfg_txaggr = 1;
            pr_debug("AMPDU set ON!\n");
        }
        else if (strnicmp(arg, "off", strlen("off")) == 0) {
            drv_priv->drv_config.cfg_txaggr = 0;
            pr_debug("AMPDU set OFF!\n");

        }
        else {
            if (drv_priv->drv_config.cfg_txaggr) {
                pr_debug("AMPDU is ON.\n");
            }
            else {
                pr_debug("AMPDU is OFF.\n");
            }
        }
    }
}

int aml_set_mac_amsdu(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);

    aml_set_mac_amsdu_switch_state(arg[1]);

    FREE(arg, "cmd_arg");
    return 0;
}

static struct wifi_channel *wifi_mac_find_chan_unlock(struct wifi_mac *wifimac,
                                                      unsigned short cur_chan,
                                                      enum wifi_mac_bwc_width cur_bw,
                                                      unsigned short cur_freq)
{
    int i;

    for (i = 0; i < wifimac->wm_nchans; i++) {
        struct wifi_channel *c;

        c = &wifimac->wm_channels[i];
        if ((c->chan_pri_num== cur_chan) && (c->chan_bw == cur_bw) &&
            (c->chan_cfreq1 == cur_freq)) {
            DPRINTF(AML_DEBUG_BWC, "%s(%d) !!!: find a chan=%d, bw=%d and freq=%d.\n",
                    __func__, __LINE__, cur_chan, cur_bw, cur_freq);
            return c;
        }
    }

    DPRINTF(AML_DEBUG_BWC, "%s(%d) !!!: NOT find a chan=%d, bw=%d and freq=%d.\n",
            __func__, __LINE__, cur_chan, cur_bw, cur_freq);
    return NULL;
}

void wifi_mac_set_country_code(char *arg)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
    unsigned short cur_chan = 0;
    enum wifi_mac_bwc_width cur_bw = 0;
    unsigned short cur_freq = 0;
    unsigned char cur_txpwrplan = 0;

    if ((wifimac->wm_nrunning == 1) &&
        (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED)) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
    }

    if (wifimac->wm_nrunning > 1) {
        pr_debug("Setting country_code in this mode is not yet supported!\n");
        return ;
    }

    pr_debug("%s,%d, arg=%s\n", __func__, __LINE__, arg);

    if (arg && (arg[0] == wifimac->wm_country.iso[0]) &&
        (arg[1] == wifimac->wm_country.iso[1])) {
        ERROR_DEBUG_OUT("no need to set country code due to the same country code\n");
        return;
    }

    preempt_scan(selected_wnet_vif->vm_ndev, 100, 100);

    WIFI_CHANNEL_LOCK(wifimac);

    if (wifimac->wm_nrunning == 1) {
        cur_chan = selected_wnet_vif->vm_curchan->chan_pri_num;
        cur_bw = selected_wnet_vif->vm_curchan->chan_bw;
        cur_freq = selected_wnet_vif->vm_curchan->chan_cfreq1;
    }

    cur_txpwrplan = drv_priv->drv_config.cfg_txpoweplan;

    wifi_mac_set_country(wifimac, arg);

    if (wifimac->wm_nrunning == 1) {
        selected_wnet_vif->vm_curchan = wifi_mac_find_chan_unlock(wifimac, cur_chan,
                                                                  cur_bw, cur_freq);

        if (!selected_wnet_vif->vm_curchan) {
            wifi_mac_top_sm(selected_wnet_vif, WIFINET_S_INIT, 0);
        }
    }

    if (cur_txpwrplan != drv_priv->drv_config.cfg_txpoweplan) {
        wifi_mac_set_tx_power_coefficient(drv_priv, selected_wnet_vif->vm_curchan,
                                          drv_priv->drv_config.cfg_txpoweplan);
    }

    WIFI_CHANNEL_UNLOCK(wifimac);

    return;
}

void wifi_mac_ap_set_11h(unsigned char channel)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *selected_wnet_vif = NULL;

    if (wifimac->wm_nrunning == 1) {
        if (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        } else {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
        }
    } else {
        ERROR_DEBUG_OUT("no sta connected\n");
        return;
    }

    wifimac->wm_doth_tbtt = 255; /* simple_strtoul(*buf, NULL, 0); */
    /* wifimac->wm_doth_channel = simple_strtoul(*(buf + 1), NULL, 0); */
    wifimac->wm_doth_channel = channel;
    pr_debug("%s, wm_dott_tbtt=%d, wm_doth_channel=%d\n", __func__, wifimac->wm_doth_tbtt, wifimac->wm_doth_channel);
    wifimac->wm_flags |= WIFINET_F_CHANSWITCH;
    wifimac->wm_flags |= WIFINET_F_DOTH;
    selected_wnet_vif->vm_chanchange_count = 0;

    return;
}

void wifi_mac_ap_set_arp_rx(char** buf)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *selected_wnet_vif = NULL;

    if (wifimac->wm_nrunning == 1) {
        if (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        } else {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
        }

    } else {
        ERROR_DEBUG_OUT("no sta connected\n");
        return;
    }
    selected_wnet_vif->vm_arp_rx.src_mac_addr[0] = simple_strtoul(*buf, NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_mac_addr[1] = simple_strtoul(*(buf + 1), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_mac_addr[2] = simple_strtoul(*(buf + 2), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_mac_addr[3] = simple_strtoul(*(buf + 3), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_mac_addr[4] = simple_strtoul(*(buf + 4), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_mac_addr[5] = simple_strtoul(*(buf + 5), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_ip_addr[0] = simple_strtoul(*(buf + 6), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_ip_addr[1] = simple_strtoul(*(buf + 7), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_ip_addr[2] = simple_strtoul(*(buf + 8), NULL, 0);
    selected_wnet_vif->vm_arp_rx.src_ip_addr[3] = simple_strtoul(*(buf + 9), NULL, 0);
    selected_wnet_vif->vm_arp_rx.out = simple_strtoul(*(buf + 10), NULL, 0);
    pr_debug("set mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
           selected_wnet_vif->vm_arp_rx.src_mac_addr[0],
           selected_wnet_vif->vm_arp_rx.src_mac_addr[1],
           selected_wnet_vif->vm_arp_rx.src_mac_addr[2],
           selected_wnet_vif->vm_arp_rx.src_mac_addr[3],
           selected_wnet_vif->vm_arp_rx.src_mac_addr[4],
           selected_wnet_vif->vm_arp_rx.src_mac_addr[5]);
    pr_debug("set ip=%d.%d.%d.%d\n", selected_wnet_vif->vm_arp_rx.src_ip_addr[0],
           selected_wnet_vif->vm_arp_rx.src_ip_addr[1],
           selected_wnet_vif->vm_arp_rx.src_ip_addr[2],
           selected_wnet_vif->vm_arp_rx.src_ip_addr[3]);
    wifi_mac_set_arp_rsp(selected_wnet_vif);
    return;
}

int aml_set_drv_ampdu(struct wlan_net_vif *wnet_vif, char* buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);

    aml_set_drv_ampdu_switch_state(arg[1]);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_update_wmm_arg(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg, cmd_param;
    char sep = ' ';
    struct wifi_mac_wme_state *wme = &wnet_vif->vm_wmac->wm_wme[wnet_vif->wnet_vif_id];

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);

    if ((wme != NULL) && (arg[1] != NULL)) {
        int ac;

        ac = simple_strtoul(arg[1], NULL, 0);
        if ((ac < WME_NUM_AC) && (arg[2] != NULL)) {
            struct wmeParams *wmep = &wme->wme_wmeChanParams.cap_wmeParams[ac];
            int i;

            for (i = 2; i < cmd_arg; i++) {
                char **param;

                sep = ':';
                param = aml_cmd_char_phrase(sep, arg[i], &cmd_param);
                if (strnicmp(arg[i], "acm", strlen("acm")) == 0) {
                    wmep->wmep_acm = simple_strtoul(param[1], NULL, 0);

                } else if (strnicmp(arg[i], "aifsn", strlen("aifsn")) == 0) {
                    wmep->wmep_aifsn = simple_strtoul(param[1], NULL, 0);

                } else if (strnicmp(arg[i], "cwmin", strlen("cwmin")) == 0) {
                    wmep->wmep_logcwmin = simple_strtoul(param[1], NULL, 0);

                } else if (strnicmp(arg[i], "cwmax", strlen("cwmax")) == 0) {
                    wmep->wmep_logcwmax = simple_strtoul(param[1], NULL, 0);

                } else if (strnicmp(arg[i], "limit", strlen("limit")) == 0) {
                    wmep->wmep_txopLimit = simple_strtoul(param[1], NULL, 0);

                } else if (strnicmp(arg[i], "noackPolicy", strlen("noackPolicy")) == 0) {
                    wmep->wmep_noackPolicy = simple_strtoul(param[1], NULL, 0);
                } else {
                    ERROR_DEBUG_OUT("Parameter [%s] is error.\n", arg[i]);
                }
                kfree(param);
            }

        } else {
            ERROR_DEBUG_OUT("Parameter is error.\n");
            FREE(arg, "cmd_arg");
            return 0;
        }

        wifi_mac_wme_updateparams(wnet_vif);
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_dynamic_bw(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char* str[2] = {"off", "on"};
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data = 0;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    len = strlen("get_txaggr_sta ") + strlen(str[data]) + 1;
    snprintf(buf, len, "get_txaggr_sta %s", str[data]);
    pr_debug("%s: buf %s\n", __func__, buf);
    aml_set_dynamic_bw(wnet_vif, buf, len);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_dynamic_bw(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((drv_priv != NULL) && (arg[1] != NULL)) {
        int data;

        if (strnicmp(arg[1], "on", strlen("on")) == 0) {
            data = 1;
            pr_debug("dynamic_bw set ON!\n");

        } else if (strnicmp(arg[1], "off", strlen("off")) == 0) {
            data = 0;
            pr_debug("dynamic_bw set OFF!\n");

        } else {
            ERROR_DEBUG_OUT("Parameter is error,now cfg_dynamic_bw=%d.\n", drv_priv->drv_config.cfg_dynamic_bw);
            FREE(arg, "cmd_arg");
            return 0;
        }
        drv_set_config((void *)drv_priv, CHIP_PARAM_DYNAMIC_BW, data);

    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_short_gi(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char* str[2] = {"off", "on"};
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    len = strlen("get_txaggr_sta ") + strlen(str[data]) + 1;
    snprintf(buf, len, "get_txaggr_sta %s", str[data]);
    pr_debug("%s: buf %s\n", __func__, buf);
    aml_set_short_gi(wnet_vif, buf, len);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_short_gi(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int usr_data;
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (!arg[1]) {
        FREE(arg, "cmd_arg");
        ERROR_DEBUG_OUT("--Bad parameter\n");
        return 0;
    }
    pr_debug("%s,cmd param: %s, %s\n", __func__, arg[0], arg[1]);

    if (strnicmp(arg[1], "on", strlen("on")) == 0) {
        usr_data = 1;
        pr_debug("%s: enable short GI.\n", __func__);

    } else if (strnicmp(arg[1], "off", strlen("off")) == 0) {
        usr_data = 0;
        pr_debug("%s: disable short GI.\n", __func__);

    } else {
        pr_debug("%s--Bad parameter\n", __func__);
        FREE(arg, "cmd_arg");
        return 0;
    }

    if (aml_wifi_is_enable_rf_test())
        gB2BTestCasePacket.if_shortGI = usr_data;

    if (1 == usr_data) {
        wnet_vif->vm_wmac->wm_flags_ext |= WIFINET_FEXT_SHORTGI_ENABLE;
        pr_debug("%s: enable short GI done\n", __func__);

    } else {
        wnet_vif->vm_wmac->wm_flags_ext &= ~WIFINET_FEXT_SHORTGI_ENABLE;
        pr_debug("%s: disable short GI done.\n", __func__);
    }


    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_eat_count_max(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((drv_priv != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtoul(arg[1], NULL, 0);
        drv_set_config((void *)drv_priv, CHIP_PARAM_EAT_COUNT, data);

    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_aggr_thresh(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((drv_priv != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtoul(arg[1], NULL, 0);
        drv_set_config((void *)drv_priv, CHIP_PARAM_AGGR_THRESH, data);
    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_hrtimer_interval(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((drv_priv != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtoul(arg[1], NULL, 0);
        drv_set_config((void *)drv_priv, CHIP_PARAM_HRTIMER_INTERVAL, data);

    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_get_ap_ip(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    if (!(wnet_vif->vm_mainsta->sta_arp_flag & WIFI_ARP_GET_AP_IP)) {
        wifi_mac_send_arp_req(wnet_vif);
    } else {
        pr_debug("ap ip is:%d:%d:%d:%d\n", wnet_vif->vm_mainsta->sta_ap_ip[0],
               wnet_vif->vm_mainsta->sta_ap_ip[1], wnet_vif->vm_mainsta->sta_ap_ip[2],
               wnet_vif->vm_mainsta->sta_ap_ip[3]);
    }

    return 0;
}

int aml_set_roaming_threshold_2g(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((wnet_vif != NULL)&& (wnet_vif->vm_wmac != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtol(arg[1], NULL, 0);
        wnet_vif->vm_wmac->roaming_threshold_2g = data;
        pr_debug("vm_wmac->roaming_threshold_2g:%d\n", wnet_vif->vm_wmac->roaming_threshold_2g);

    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_roaming_threshold_5g(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtol(arg[1], NULL, 0);
        wnet_vif->vm_wmac->roaming_threshold_5g = data;
        pr_debug("vm_wmac->roaming_threshold_5g:%d\n", wnet_vif->vm_wmac->roaming_threshold_5g);
    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_get_roaming_candidate_chans(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int i;
    int chan_cnt = wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans_cnt;

    pr_debug("get roaming candidate chans [%d]:\n", chan_cnt);

    for (i=0; i < chan_cnt; i++) {
        struct wifi_channel * chan;

        chan = wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans[i].channel;

        if (chan == NULL) {
            ERROR_DEBUG_OUT("erro pointer \n");
            return 0;
        }

        pr_debug("chan_cfreq1:%d chan_flags:%d chan_pri_num:%d chan_maxpower:%d chan_minpower:%d chan_bw:%d global_operating_class %d Rssi:%d\n",
            chan->chan_cfreq1, chan->chan_flags, chan->chan_pri_num, chan->chan_maxpower,
            chan->chan_minpower, chan->chan_bw, chan->global_operating_class,
            (wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans[i].avg_rssi - 256));
    }

    return 0;
}

int aml_set_roaming_candidate_chans(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int i;
    struct wifi_mac *wifimac;
    struct wifi_mac_scan_state *ss;

    if (wnet_vif == NULL) {
        return 0;
    }

    wifimac = wnet_vif->vm_wmac;
    ss = wifimac->wm_scan;

    WIFI_ROAMING_CHANNLE_LOCK(wnet_vif->vm_wmac->wm_scan);

    wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans_cnt = 0;
    memset(wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans, 0,
           sizeof(wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans));

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);

    for (i = 1; i < ROAMING_CANDIDATE_CHAN_MAX; i++) {
        if (arg[i] != NULL) {
            int data;
            int j;

            data = simple_strtoul(arg[i], NULL, 0);
            pr_debug("get channel:%d \n",data);

            WIFI_CHANNEL_LOCK(wifimac);

            for (j = 0; j < wifimac->wm_nchans; j++) {
                struct wifi_channel *c;

                c = &wifimac->wm_channels[j];
                if (c->chan_bw == WIFINET_BWC_WIDTH20 && c->chan_pri_num == data) {
                    ss->roaming_candidate_chans[ss->roaming_candidate_chans_cnt].channel = c;
                    ss->roaming_candidate_chans[ss->roaming_candidate_chans_cnt].avg_rssi = 156;
                    ss->roaming_candidate_chans_cnt++;
                }
            }

            WIFI_CHANNEL_UNLOCK(wifimac);
        } else {
           WIFI_ROAMING_CHANNLE_UNLOCK(wnet_vif->vm_wmac->wm_scan);
           FREE(arg, "cmd_arg");
           return 0;
        }
    }

    WIFI_ROAMING_CHANNLE_UNLOCK(wnet_vif->vm_wmac->wm_scan);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_roaming_mode(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtoul(arg[1], NULL, 0);
        wnet_vif->vm_wmac->wm_roaming = data;
        pr_debug("vm_wmac->wm_roaming:%d\n", wnet_vif->vm_wmac->wm_roaming);
    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int wifi_mac_set_udp_info(char **buf)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *selected_wnet_vif = NULL;
    unsigned int param;

    if (wifimac->wm_nrunning == 1) {
        if (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        } else {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
        }
    } else {
        ERROR_DEBUG_OUT("no sta connected\n");
        return -1;
    }

    aml_udp_info[udp_cnt].dst_port = simple_strtoul(*buf, NULL, 0);
    aml_udp_info[udp_cnt].src_port = simple_strtoul(*(buf + 1), NULL, 0);
    aml_udp_info[udp_cnt].dst_ip = simple_strtoul(*(buf + 2), NULL, 0);
    aml_udp_info[udp_cnt].src_ip = simple_strtoul(*(buf + 3), NULL, 0);
    aml_udp_info[udp_cnt].pkt_len = simple_strtoul(*(buf + 4), NULL, 0);
    param = simple_strtoul(*(buf + 5), NULL, 0);
    aml_udp_info[udp_cnt].out = param >> 8;
    aml_udp_info[udp_cnt].streamid = param & 0xff;
    aml_udp_info[udp_cnt].seq = 1000;
    aml_udp_info[udp_cnt].rx = 0;
    aml_udp_info[udp_cnt].tx = 0;
    aml_udp_timer.udp_timer_stop = 0;

    DPRINTF(AML_DEBUG_WARNING,
            "dst_port=%04x, src_port=%04x, dst_ip=%08x, src_ip:%08x, pkt_len:%d, out:%d, streamid:%d, param=%d\n",
            aml_udp_info[udp_cnt].dst_port, aml_udp_info[udp_cnt].src_port,
            aml_udp_info[udp_cnt].dst_ip, aml_udp_info[udp_cnt].src_ip,
            aml_udp_info[udp_cnt].pkt_len, aml_udp_info[udp_cnt].out,
            aml_udp_info[udp_cnt].streamid, param);

    if (aml_udp_info[udp_cnt].out && aml_udp_timer.run_flag == 0) {
        os_timer_ex_initialize(&aml_udp_timer.udp_send_timeout, 0,
                               wifi_mac_udp_send_timeout_ex, selected_wnet_vif);
        os_timer_ex_start_period(&aml_udp_timer.udp_send_timeout, 10);
        aml_udp_timer.run_flag = 1;
    }

    udp_cnt++;
    return 0;
}

/* set_udp_info dst_port src_port dst_ip in/out */
int aml_set_udp_info(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    int skip = 0, cmd_arg;
    char **arg;
    char sep = ' ';
    skip = strlen("set_udp_info") + 1;

    arg = aml_cmd_char_phrase(sep, buf + skip, &cmd_arg);
    wifi_mac_set_udp_info(arg);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_get_udp_info(struct wlan_net_vif *wnet_vif, char* buf, int len)
{
    int i;

    for (i = 0; i < udp_cnt; i++) {
        pr_debug("%s streamid=%d tx is %d, rx is %d\n", __func__, aml_udp_info[i].streamid, aml_udp_info[i].tx, aml_udp_info[i].rx);
    }
    aml_udp_timer.udp_timer_stop = 1;
    aml_udp_timer.run_flag = 0;
    os_timer_ex_del(&aml_udp_timer.udp_send_timeout, CANCEL_SLEEP);
    return 0;
}

int aml_unmark_dfs_channel(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL)) {
        int data = 0;

        if (arg[1] != NULL) {
            data = simple_strtoul(arg[1], NULL, 0);
        }

        pr_debug("unmarked dfs channel :%d\n",data);
        wifi_mac_unmark_dfs_channel(wnet_vif, data);

   } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
   }

   FREE(arg, "cmd_arg");
   return 0;
}

int aml_mark_dfs_channel(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
     char **arg;
     char sep = ' ';
     int cmd_arg;

     arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
     if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL)) {
         int data = 0;

         if (arg[1] != NULL) {
             data = simple_strtoul(arg[1], NULL, 0);
         }

         pr_debug("marked dfs channel :%d\n",data);
         wifi_mac_mark_dfs_channel(wnet_vif, data);

    } else {
         ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_device_sn(struct wlan_net_vif *wnet_vif, char* buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        char **mac_cmd;

        sep = ':';
        mac_cmd = aml_cmd_char_phrase(sep, arg[1], &cmd_arg);
        if (mac_cmd) {
            unsigned int efuse_data;
            int i;

            efuse_data = (simple_strtoul(mac_cmd[0], NULL, 16) << 8) |
                         (simple_strtoul(mac_cmd[1], NULL, 16));

            for (i = 0; i < 16; i++) {
                if (efuse_data & (1 << i)) {
                    /* pr_debug("set_dev_sn ===>>> efuse_manual_write: %d 0xf\n", i); */
                    efuse_manual_write(i, 0xf);
                }
            }
            pr_debug("set_dev_sn ===>>> write SN/ID is: %02x %02x\n",
                   ((efuse_data & 0xff00) >> 8), (efuse_data & 0x00ff));
        }
        kfree(mac_cmd);
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_get_device_sn(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    int cmd_arg;
    char sep = ' ';

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg) {
        unsigned int efuse_data;

        efuse_data = efuse_manual_read(simple_strtoul(arg[1], NULL, 16));
        pr_debug("get_dev_sn ===>>> efuse addr:%08x, get SN/ID is: %02x %02x\n",
               simple_strtoul(arg[1], NULL, 16), ((efuse_data & 0xff00) >> 8),
               (efuse_data & 0x00ff));

        FREE(arg, "cmd_arg");
    }

    return 0;
}

int aml_set_signal_power_weak_thresh_for_narrow_bandwidth(struct wlan_net_vif *wnet_vif,
                                                          char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtol(arg[1], NULL, 0);
        wnet_vif->vm_wmac->wm_signal_power_weak_thresh_narrow = data;
        pr_debug("set signal power weak thresh to: %d\n",
               wnet_vif->vm_wmac->wm_signal_power_weak_thresh_narrow);
    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_signal_power_weak_thresh_for_wide_bandwidth(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((wnet_vif != NULL) && (wnet_vif->vm_wmac != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtol(arg[1], NULL, 0);
        wnet_vif->vm_wmac->wm_signal_power_weak_thresh_wide = data;
        pr_debug("set signal power weak thresh to: %d\n",
               wnet_vif->vm_wmac->wm_signal_power_weak_thresh_wide);
    } else {
        ERROR_DEBUG_OUT("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

extern int aml_iwpriv_set_lagecy_bitrate_mask(struct net_device *dev, unsigned int set);
int aml_set_lagecy_bitrate_mask(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    aml_iwpriv_set_lagecy_bitrate_mask(wnet_vif->vm_ndev, data);

    FREE(arg, "cmd_arg");
    return 0;
}

extern int aml_iwpriv_set_ht_bitrate_mask(struct net_device *dev, unsigned int set);
int aml_set_ht_bitrate_mask(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    aml_iwpriv_set_ht_bitrate_mask(wnet_vif->vm_ndev, data);

    FREE(arg, "cmd_arg");
    return 0;
}

extern int aml_iwpriv_set_vht_bitrate_mask(struct net_device *dev, unsigned int set);
int aml_set_vht_bitrate_mask(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    aml_iwpriv_set_vht_bitrate_mask(wnet_vif->vm_ndev, data);

    FREE(arg, "cmd_arg");
    return 0;
}

extern void aml_iwpriv_set_rate_auto(struct wlan_net_vif *wnet_vif);
int aml_set_rate_auto(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    aml_iwpriv_set_rate_auto(wnet_vif);
    return 0;
}

int aml_set_coex_req_timeslice_timeout(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac = NULL;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifimac = wifi_mac_get_mac_handle();
    wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_req_timeslice_timeout_value(data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_coex_max_miss_bcn(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac = NULL;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifimac = wifi_mac_get_mac_handle();
    wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_max_miss_bcn(data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_scan_hang(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wnet_vif->vm_scan_hang = (unsigned char)data;
    pr_debug("%s, vid:%d vm_scan_hang:%d\n ", __func__, wnet_vif->wnet_vif_id,
           wnet_vif->vm_scan_hang);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_scan_connect_time(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wnet_vif->vm_scan_time_connect = (unsigned char)data;
    wifi_mac_set_scan_time(wnet_vif);
    pr_debug("%s, vid:%d vm_scan_time_connect:%d\n ", __func__, wnet_vif->wnet_vif_id,
           wnet_vif->vm_scan_time_connect);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_scan_idle_time(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wnet_vif->vm_scan_time_idle = (unsigned char)data;
    wifi_mac_set_scan_time(wnet_vif);
    pr_debug("%s, vid:%d vm_scan_idle:%d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_time_idle);

    FREE(arg, "cmd_arg");
    return 0;
}

unsigned int get_reg(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_wpa_get_reg(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    get_reg(wnet_vif, data);

    FREE(arg, "cmd_arg");
    return 0;
}

unsigned int set_reg(struct wlan_net_vif *wnet_vif, unsigned int set1, unsigned int set2);
int aml_wpa_set_reg(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data1;
    int data2;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data1 = simple_strtol(arg[1], NULL, 0);
    data2 = simple_strtol(arg[2], NULL, 0);
    set_reg(wnet_vif, data1, data2);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_get_cca_status(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;

    wifimac = wifi_mac_get_mac_handle();
    wifimac->drv_priv->drv_ops.cca_busy_check();

    return 0;
}

int aml_wpa_get_agc_status(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;

    wifimac = wifi_mac_get_mac_handle();
    wifimac->drv_priv->drv_ops.phy_stc();

    return 0;
}

unsigned int get_latest_tx_status(struct wifi_mac *wifimac);
int aml_wpa_get_latest_tx_status(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;

    wifimac = wifi_mac_get_mac_handle();
    get_latest_tx_status(wifimac);

    return 0;
}

extern void print_driver_version(void);
int aml_wpa_get_drv_ver(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    print_driver_version();
    return 0;
}

int aml_wpa_set_country_code(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }
    wifi_mac_set_country_code(arg[1]);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_get_country_code(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;

    wifimac = wifi_mac_get_mac_handle();
    pr_debug("country code: %s\n", wifimac->wm_country.iso);

    return 0;
}

extern void wifi_mac_pwrsave_set_inactime(struct wlan_net_vif *wnet_vif, unsigned int time);
int aml_wpa_set_power(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        ERROR_DEBUG_OUT("char_phrase fail\n");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifi_mac_pwrsave_set_inactime(wnet_vif, data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_channel_rssi(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_burst(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifimac->drv_priv->drv_config.cfg_burst_ack = data;
    pr_debug("wpa cli set burst %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

extern void aml_iwpriv_set_uapsd(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_wpa_set_uapsd(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    aml_iwpriv_set_uapsd(wnet_vif, data);
    pr_debug("wpa cli set uapsd %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_pt_rxstart(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wnet_vif->vif_ops.pt_rx_start(data);
    pr_debug("wpa cli set rxstart %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_pt_rxstop(struct wlan_net_vif *wnet_vif, char* buf, int len)
{
    wnet_vif->vif_ops.pt_rx_stop();
    return 0;
}

int aml_wpa_set_scan_pri(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_scan_priority_range(data);
    pr_debug("wpa cli set scan pri %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_bebk_pri(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_be_bk_noqos_priority_range(data);
    pr_debug("wpa cli set bebk noqos pri %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_coex_btwifi(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    pr_debug("%s, coexist en= %d\n ", __func__, data);
    wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_en(data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_coex_bcnmis(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac;
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    wifimac = wifi_mac_get_mac_handle();
    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    pr_debug("%s, coexist value=%d\n ", __func__, data);
    wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_max_miss_bcn(data);

    FREE(arg, "cmd_arg");
    return 0;
}

extern int aml_beacon_intvl_set(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_wpa_set_bcn_intv(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    pr_debug("%s, coexist value=%d\n", __func__, data);
    aml_beacon_intvl_set(wnet_vif, data);

    FREE(arg, "cmd_arg");
    return 0;
}

extern int aml_set_ldpc(struct wlan_net_vif *wnet_vif, unsigned int set);
int aml_wpa_set_ldpc(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    pr_debug("%s, coexist value=%d\n", __func__, data);
    aml_set_ldpc(wnet_vif, data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_get_chan_list(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    struct wifi_mac *wifimac = NULL;
    int i;

    wifimac = wifi_mac_get_mac_handle();

    WIFI_CHANNEL_LOCK(wifimac);

    for (i = 0; i < wifimac->wm_nchans; i++) {
        struct wifi_channel *c;

        c = &wifimac->wm_channels[i];
        pr_debug("channel:%d\t frequency:%d \t bandwidth:%dMHz \n",
               c->chan_pri_num, c->chan_cfreq1, ((1 << c->chan_bw) * 20));
    }

    WIFI_CHANNEL_UNLOCK(wifimac);

    return 0;
}

#if defined(SU_BF) || defined(MU_BF)
extern int aml_set_beamforming(struct wlan_net_vif *wnet_vif, unsigned int set1,unsigned int set2);
int aml_wpa_set_beamforming(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data1;
    int data2;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data1 = simple_strtol(arg[1], NULL, 0);
    data2 = simple_strtol(arg[2], NULL, 0);
    aml_set_beamforming(wnet_vif, data1,data1);

    FREE(arg, "cmd_arg");
    return 0;
}
#endif

int aml_wpa_set_pkt_fetch(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data = 0;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    wnet_vif->vm_mainsta->sta_fetch_pkt_method = (unsigned char)data;
    pr_debug("wpa cli set pkt fetch %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_frag_thr(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    if ((unsigned short)data > 0) {
        wnet_vif->vm_fragthreshold = (unsigned short)data;
    }
    pr_debug("wpa cli set frag thr %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_preamble(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    phy_set_preamble_type(data);
    pr_debug("wpa cli set preamble type %d\n", data);

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_band(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    drv_priv->drv_config.cfg_band = data;

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_mac_mode(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;
    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    drv_priv->drv_config.cfg_mac_mode = data;

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_initial_gain_change_hang(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }

    data = simple_strtol(arg[1], NULL, 0);
    g_initial_gain_change_disable = data;

    if (g_initial_gain_change_disable) {
        ERROR_DEBUG_OUT("initial_gain_change invalid\n ");

    } else {
        ERROR_DEBUG_OUT("initial_gain_change valid\n ");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_wpa_set_tx_power_change_hang(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;
    int data = 0;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }
    data = simple_strtol(arg[1], NULL, 0);
    g_tx_power_change_disable = data;

    if (g_tx_power_change_disable) {
        ERROR_DEBUG_OUT("tx_power_change invalid\n ");

    } else {
        ERROR_DEBUG_OUT("tx_power_change valid\n ");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

int aml_set_tx_power_plan(struct wlan_net_vif *wnet_vif, char* buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    struct drv_private *drv_priv = drv_get_drv_priv();

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if ((drv_priv != NULL) && (arg[1] != NULL)) {
        int data;

        data = simple_strtoul(arg[1], NULL, 0);
        wifimac_set_tx_pwr_plan(data);
    } else {
        pr_err("Parameter is error.\n");
    }

    FREE(arg, "cmd_arg");
    return 0;
}

static int aml_set_debug_switch(char *switch_str)
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

int aml_wpa_set_debug(struct wlan_net_vif *wnet_vif, char *buf, int len)
{
    char **arg;
    char sep = ' ';
    int cmd_arg;

    arg = aml_cmd_char_phrase(sep, buf, &cmd_arg);
    if (arg[1] == NULL) {
        FREE(arg, "cmd_arg");
        return -EINVAL;
    }
    aml_set_debug_modules(arg[1]);

    FREE(arg, "cmd_arg");
    return 0;
}
