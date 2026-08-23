#include "wifi_mac_com.h"
#include "wifi_mac_scan.h"
#include "wifi_mac_rate.h"
#include "wifi_iwpriv_cmd.h"

extern int g_auto_gain_base;

int saveie(unsigned char *iep, const unsigned char *ie)
{
    if (ie == NULL)
    {
        (iep)[1] = 0;
    }
    else
    {
        unsigned int ielen = ie[1]+2;
        if (iep != NULL)
            memcpy(iep, ie, ielen);
        return ielen;
    }
    return 0;
}

static int wifi_mac_scan_sta_compare(const struct scaninfo_entry *a, const struct scaninfo_entry *b)
{
    if ((a->scaninfo.SI_capinfo & WIFINET_CAPINFO_PRIVACY) &&
        (b->scaninfo.SI_capinfo & WIFINET_CAPINFO_PRIVACY) == 0)
        return 1;

    if ((a->scaninfo.SI_capinfo & WIFINET_CAPINFO_PRIVACY) == 0 &&
        (b->scaninfo.SI_capinfo & WIFINET_CAPINFO_PRIVACY))
        return -1;

    if (abs(b->connectcnt - a->connectcnt) > 1)
        return b->connectcnt - a->connectcnt;

    if (abs(b->scaninfo.SI_rssi - a->scaninfo.SI_rssi) < 5)
    {
        return wifi_mac_get_sta_mode((struct wifi_scan_info *)&b->scaninfo) - wifi_mac_get_sta_mode((struct wifi_scan_info *)&a->scaninfo);
    }
    return a->scaninfo.SI_rssi - b->scaninfo.SI_rssi;
}

static int match_ssid(const unsigned char *ie, int nssid, const struct wifi_mac_ScanSSID ssids[])
{
    int i;

    for (i = 0; i < nssid; i++) {
        if ((ie[1] == ssids[i].len) && (memcmp(ie+2, ssids[i].ssid, ie[1]) == 0)) {
            return 1;
        }
    }
    return 0;
}



static int match_bss(struct wlan_net_vif *wnet_vif,
    const struct wifi_mac_scan_state *ss, const struct scaninfo_entry *se)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    const struct wifi_scan_info *scaninfo = &se->scaninfo;
    unsigned int fail;

    fail = 0;
    if (wifi_mac_chan_num_avail(wifimac, wifi_mac_chan2ieee(wifimac, scaninfo->SI_chan)) == false) {
        fail |= STA_MATCH_ERR_CHAN;
        //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
    }

    if (wnet_vif->vm_opmode == WIFINET_M_IBSS) {
        if ((scaninfo->SI_capinfo & WIFINET_CAPINFO_IBSS) == 0) {
             fail |= STA_MATCH_ERR_BSS;
              //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
        }

    } else {
        if ((scaninfo->SI_capinfo & WIFINET_CAPINFO_ESS) == 0) {
            fail |= STA_MATCH_ERR_BSS;
            //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
        }
    }

    if (wnet_vif->vm_flags & WIFINET_F_PRIVACY) {
        if ((scaninfo->SI_capinfo & WIFINET_CAPINFO_PRIVACY) == 0) {
            fail |= STA_MATCH_ERR_PRIVACY;
             //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
        }

    } else {
        if (scaninfo->SI_capinfo & WIFINET_CAPINFO_PRIVACY) {
            fail |= STA_MATCH_ERR_PRIVACY;
             //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
        }
    }

    if (!check_rate(wnet_vif, scaninfo)) {
        fail |= STA_MATCH_ERR_RATE;
        //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
    }

    if ((wnet_vif->vm_fixed_rate.mode == WIFINET_FIXED_RATE_NONE)
        || (wnet_vif->vm_fixed_rate.rateinfo & WIFINET_RATE_MCS)) {
        if (!check_ht_rate(wnet_vif, scaninfo)) {
            fail |= STA_MATCH_ERR_HTRATE;
            //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
        }
    }

    /*get ssid, which we want to connect */
    if (!(wnet_vif->vm_flags & WIFINET_F_IGNORE_SSID)
        && ((ss->ss_nssid == 0) || (match_ssid(scaninfo->SI_ssid, ss->ss_nssid, ss->ss_ssid) == 0))) {
        fail |= STA_MATCH_ERR_SSID;
        //pr_err("%s(%d) fail 0x%x, ss->ss_nssid:%d\n", __func__, __LINE__, fail, ss->ss_nssid);
    }

    if ((wnet_vif->vm_flags & WIFINET_F_DESBSSID)
        && !WIFINET_ADDR_EQ(wnet_vif->vm_des_bssid, scaninfo->SI_bssid)) {
        fail |= STA_MATCH_ERR_BSSID;
         //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
    }

    if (se->connectcnt >= WIFINET_CONNECT_FAILS) {
        fail |= STA_MATCH_ERR_STA_FAILS_MAX;
         //pr_err("%s(%d) fail 0x%x\n", __func__, __LINE__, fail);
    }

    if (!(fail & STA_MATCH_ERR_SSID)) {
        DPRINTF(AML_DEBUG_WARNING, "%s flag = 0x%x, ssid=%s, chan_pri_num=%d, SI_rssi =%d, vm_flags=%x bssid=%s chan_flags=0x%x \n",
            __func__,fail, ssidie_sprintf(scaninfo->SI_ssid), scaninfo->SI_chan->chan_pri_num, scaninfo->SI_rssi,
            wnet_vif->vm_flags, ether_sprintf(scaninfo->SI_bssid), scaninfo->SI_chan->chan_flags);
    }
    return fail;
}

static void wifi_mac_save_ssid(struct wlan_net_vif *wnet_vif, struct wifi_mac_scan_state *ss,
    int nssid, const struct wifi_mac_ScanSSID ssids[])
{
    if ((nssid == 0) || (nssid > WIFINET_SCAN_MAX_SSID)) {
        return;
    }

    memcpy(ss->ss_ssid, ssids, nssid * sizeof(ssids[0]));
    ss->ss_nssid = nssid;
}


int wifi_mac_scan_chk_11g_bss(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
        struct scaninfo_table *st = ss->ScanTablePriv;
        struct scaninfo_entry *se;
        enum wifi_mac_macmode macmode = WIFINET_MODE_AUTO;
        int ret = 0;

        WIFI_SCAN_SE_LIST_LOCK(st);
        list_for_each_entry(se,&st->st_entry,se_list) {
        macmode = wifi_mac_get_sta_mode(&se->scaninfo);
        /*a 5G 11g only found, ignore it*/
        if((macmode == WIFINET_MODE_11G) && WIFINET_IS_CHAN_2GHZ((&se->scaninfo)->SI_chan)) {

            ret = 1;
            DPRINTF(AML_DEBUG_WARNING, "<running> %s %d mac mode %d addr:%x:%x:%x:%x:%x:%x\n",__func__,__LINE__, macmode,
                se->scaninfo.SI_macaddr[0],se->scaninfo.SI_macaddr[1],se->scaninfo.SI_macaddr[2],
                se->scaninfo.SI_macaddr[3],se->scaninfo.SI_macaddr[4],se->scaninfo.SI_macaddr[5]);
            break;
        } else {

            ret = 0;
        }
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);
    return ret;
}


static int
wifi_mac_scan_get_best_node(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif, struct scaninfo_entry *bestNode)
{
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int roaming_threshold = wifimac->roaming_threshold_5g;

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry(se,&st->st_entry,se_list) {
        if (match_bss(wnet_vif, ss, se) == 0) {
            if (bestNode->se_valid == 0) {
                memcpy(bestNode, se, sizeof(struct scaninfo_entry));

            } else if (wifi_mac_scan_sta_compare(se, bestNode) > 0) {
                memcpy(bestNode, se, sizeof(struct scaninfo_entry));
            }
        }
    }

    if (bestNode->se_valid && wnet_vif->vm_chan_roaming_scan_flag == 1) {
        if (WIFINET_IS_CHAN_2GHZ(bestNode->scaninfo.SI_chan)) {
            roaming_threshold = wifimac->roaming_threshold_2g;
        }

        if ((bestNode->se_avgrssi - 255) < roaming_threshold) {
            AML_OUTPUT("BestNode Rssi[%d] < Roaming threshols[%d] \n", (bestNode->se_avgrssi - 255), roaming_threshold);
            bestNode->se_valid = 0;
        }
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);
    return bestNode->se_valid;
}

static unsigned char
wifi_mac_scan_get_match_node(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL;
    struct scaninfo_entry *se_next = NULL;
    unsigned char ret = 0;

    wnet_vif->vm_connchan.num = 0;
    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se,se_next,&st->st_entry,se_list)
    {
        DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d se%p st_entry%p \n",__func__,__LINE__,se, &st->st_entry);
        if (match_bss(wnet_vif, ss, se) == 0)
        {
             wnet_vif->vm_scanchan_rssi = se->se_avgrssi - 255;
             wnet_vif->vm_connchan.conn_chan[wnet_vif->vm_connchan.num++] = se->scaninfo.SI_chan;
             WIFINET_ADDR_COPY(wnet_vif->vm_connchan.da, se->scaninfo.SI_macaddr);
             WIFINET_ADDR_COPY(wnet_vif->vm_connchan.bssid, se->scaninfo.SI_bssid);
             list_del_init(&se->se_list);
             list_del_init(&se->se_hash);
             FREE(se,"sta_add.se");
             ret = 1;
        }
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);
    return ret;
}

static unsigned char
wifi_mac_scan_set_match_node(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL;
    struct scaninfo_entry *se_next = NULL;
    unsigned char ret = 0;

    wnet_vif->vm_connchan.num = 0;
    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se,se_next,&st->st_entry,se_list) {
        if (match_bss(wnet_vif, ss, se) != 0) {
             list_del_init(&se->se_list);
             list_del_init(&se->se_hash);
             FREE(se,"sta_add.se");
             ret = 1;
        }
    }

    WIFI_SCAN_SE_LIST_UNLOCK(st);
    return ret;
}

static int
wifi_mac_scan_connect(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
    struct scaninfo_entry *best_node = &wnet_vif->vm_connect_scan_entry;
    struct wifi_channel *chan;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    const unsigned char zero_bssid[WIFINET_ADDR_LEN] = {0};

    /*AP doesn't get ssid and connect, so just return. */
    if (ss->scan_CfgFlags & WIFINET_SCANCFG_NOPICK) {
        return 0;
    }

    if (pwdev_priv->connect_request == NULL) {
        if (wnet_vif->vm_wmac->recovery_stat != WIFINET_RECOVERY_VIF_UP)
            return 0;
    }

    if (wnet_vif->vm_state > WIFINET_S_SCAN) {
        return 0;
    }

    KASSERT((wnet_vif->vm_opmode == WIFINET_M_IBSS)
        ||(wnet_vif->vm_opmode == WIFINET_M_STA)
        ||(wnet_vif->vm_opmode == WIFINET_M_P2P_DEV)
        ||(wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT),
        ("wifi_mac_scan_connect vmopmode error opmode=%d !!",wnet_vif->vm_opmode));

    memset(best_node, 0, sizeof(struct scaninfo_entry));
    /*get a best ap from scan list, by comparing rssi, connect times... */
    if (!wifi_mac_scan_get_best_node(ss, wnet_vif, best_node)) {
        ERROR_DEBUG_OUT("no bss match, goto notfound\n");
        goto notfound;
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_IBSS)
        && (WIFINET_ADDR_EQ(best_node->scaninfo.SI_bssid, &zero_bssid[0])))
    {
        /* get ap work mode. */
        if( WIFINET_IS_CHAN_2GHZ( best_node->scaninfo.SI_chan)  )
        {
            wnet_vif->vm_mac_mode = WIFINET_MODE_11BGN;
        }
        else
        {
            wnet_vif->vm_mac_mode = WIFINET_MODE_11GNAC;
        }

        DPRINTF(AML_DEBUG_SCAN, "<running> %s %d \n",__func__,__LINE__);
        wifi_mac_create_wifi(wnet_vif, best_node->scaninfo.SI_chan);
        return 1;

    } else {
        best_node->connectcnt++;
        best_node->ConnectTime = jiffies;
        wifi_mac_connect(wnet_vif, &best_node->scaninfo);
        best_node->se_valid = 0;
        return 1;
    }

    DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d \n",__func__,__LINE__);
    return 1;

notfound:
    //if adhoc mode, not find we must create a new
    if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
    {
         if (wnet_vif->vm_curchan != WIFINET_CHAN_ERR)
            chan = wnet_vif->vm_curchan;
        else
            chan = wifi_mac_get_wm_chan(wnet_vif->vm_wmac);
        wifi_mac_create_wifi(wnet_vif, chan);
    }
    DPRINTF(AML_DEBUG_CONNECT, "%s %d entry=NULL, return\n",__func__,__LINE__);
    return 0;
}

static int wifi_mac_chk_ap_chan(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int channum;
    struct wifi_channel *c = NULL;

     //AP default work in 5G channel 149,wait hostapd configure after init
    channum = DEFAULT_CHANNEL;
    DPRINTF(AML_DEBUG_SCAN|AML_DEBUG_CONNECT,
        "<%s> :  %s %d ++ vm_opmode %d\n",wnet_vif->vm_ndev->name,__func__,__LINE__,wnet_vif->vm_opmode);

    if (wnet_vif->vm_curchan != WIFINET_CHAN_ERR)
        c = wnet_vif->vm_curchan;
    else
        c = wifi_mac_find_chan(wifimac, channum, WIFINET_BWC_WIDTH80, channum + 6);

    if(c == NULL)
    {
        DPRINTF(AML_DEBUG_ERROR, "%s (%d) error channum=%d,  c=%p\n",
            __func__, __LINE__, channum, c);
        c  = ss->ss_chans[0];
    }

    DPRINTF(AML_DEBUG_SCAN|AML_DEBUG_CONNECT, "%s %d channel=%d  chan_flags = 0x%x\n",
        __func__, __LINE__,  channum, c->chan_flags);

    if (WIFINET_IS_CHAN_2GHZ(c)) {
        if (wifi_mac_scan_chk_11g_bss(ss, wnet_vif)) {
            wnet_vif->vm_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;
            wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_NA;
            wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH20;
            c = wifi_mac_find_chan(wifimac, c->chan_pri_num, WIFINET_BWC_WIDTH20, c->chan_pri_num);
        }
    }

    wifi_mac_create_wifi(wnet_vif, c);
    return 0;
}

static void get_ovlapping_chan_index(struct wifi_mac *wifimac, unsigned char center_chan, unsigned char bw,
    unsigned char *low, unsigned char *up) {
    unsigned char step = 0;
    unsigned char is_2g = 0;
    unsigned char i = 0;

    if ((0 < center_chan) && (center_chan < 15)) {
        step = 1;
        is_2g = 1;

    } else {
        step = 2;
    }

    for (i = 0; i < WIFINET_MAX_SCAN_CHAN; ++i) {
        if (wifimac->chan_overlapping_map[i].chan_index == center_chan) {
            break;
        }
    }

    if (i < WIFINET_MAX_SCAN_CHAN) {
        if (bw == WIFINET_BWC_WIDTH20) {
            step = 2 / step;

        } else if (bw == WIFINET_BWC_WIDTH40) {
            step = 4 / step;

        } else if (bw == WIFINET_BWC_WIDTH80) {
            step = 8 / step;
        }

        if (i < step) {
            *low = 0;
            *up = i + step;

        } else if (i + step > 63) {
            *low = i - step;
            *up = 63;

        } else {
            if (is_2g) {
                if (i + step > 14) {
                    *low = i - step;
                    *up = 13;

                } else {
                    *low = i - step;
                    *up = i + step;
                }

            } else {
                *low = i - step;
                *up = i + step;

                if (*low < 14) {
                    *low = 14;

                } else if ((*low < 30) && (i >= 30)) {
                    *low = 29;

                } else if ((*low < 53) && (i >= 53)) {
                    *low = 53;
                }

                if ((*up > 30) && (i <=30)) {
                    *up = 30;

                } else if ((*up > 53) && (i <= 53)) {
                    *up = 53;
                }
            }
        }

    } else {
        ERROR_DEBUG_OUT("can't find center_chan:%d\n", center_chan);
    }

}

static void wifi_mac_update_chan_overlapping_map(struct wlan_net_vif *wnet_vif) {
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL;
    struct scaninfo_entry *se_next = NULL;
    struct wifi_scan_info *lse = NULL;
    unsigned char bw = 0;
    unsigned char low_chan = 0;
    unsigned char up_chan = 0;
    unsigned char center_chan = 0;
    unsigned char i = 0;

    for (i = 0; i < WIFINET_MAX_SCAN_CHAN; ++i) {
        wifimac->chan_overlapping_map[i].overlapping = 0;
    }

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se, se_next, &st->st_entry, se_list) {
        lse = &se->scaninfo;
        center_chan = wifi_mac_Mhz2ieee(lse->SI_chan->chan_cfreq1, 0);
        bw = lse->SI_chan->chan_bw;

        get_ovlapping_chan_index(wifimac, center_chan, bw, &low_chan, &up_chan);
        for (i = low_chan; i <= up_chan; ++i) {
            wifimac->chan_overlapping_map[i].overlapping++;
        }

        //AML_OUTPUT("ssid:%s, center_chan:%d, bw:%d, low_chan:%d, up_chan:%d\n",
        //    lse->SI_ssid + 2, center_chan, bw, low_chan, up_chan);
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);

    //for (i = 0; i < WIFINET_MAX_SCAN_CHAN; ++i) {
    //    AML_OUTPUT("chan_index:%d, overlapping:%d\n",
    //        wifimac->chan_overlapping_map[i].chan_index, wifimac->chan_overlapping_map[i].overlapping);
    //}
}

void is_connect_need_set_gain(struct wlan_net_vif *wnet_vif) {
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned char overlapping_max = 0;
    unsigned char i = 0;
    unsigned char bw = wnet_vif->vm_curchan->chan_bw;
    unsigned char low_chan = 0;
    unsigned char up_chan = 0;
    unsigned char is_2g = 0;
    unsigned char center_chan = wifi_mac_Mhz2ieee(wnet_vif->vm_curchan->chan_cfreq1, 0);

    if ((0 < center_chan) && (center_chan < 15)) {
        is_2g = 1;
    }

    get_ovlapping_chan_index(wifimac, center_chan, bw, &low_chan, &up_chan);
    for (i = low_chan; i <= up_chan; ++i) {
        if (overlapping_max < wifimac->chan_overlapping_map[i].overlapping) {
            overlapping_max = wifimac->chan_overlapping_map[i].overlapping;
        }
    }

    if (is_2g && (overlapping_max > OVERLAPPING_24G_GIAN_THRESHOLD)) {
        wifimac->is_connect_set_gain = 1;

    } else if (!is_2g && (overlapping_max > OVERLAPPING_5G_GIAN_THRESHOLD)) {
        wifimac->is_connect_set_gain = 1;

    } else {
        wifimac->is_connect_set_gain = 0;
    }

    AML_OUTPUT("overlapping:%d, is_connect_set_gain:%d, low_chan:%d, up_chan:%d, bw:%d, center:%d\n",
        overlapping_max, wifimac->is_connect_set_gain, low_chan, up_chan, bw, center_chan);
}


int wifi_mac_scan_parse(struct wlan_net_vif *wnet_vif, wifi_mac_ScanIterFunc *f, void *arg)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL, *SI_next = NULL;
    unsigned char count = 0;

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se,SI_next,&st->st_entry,se_list)
    {
        se->scaninfo.SI_age = jiffies - se->LastUpdateTime;
        (*f)(arg, &se->scaninfo);
        count++;
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);

    /* if scan all channels */
    if (!wnet_vif->vm_chan_switch_scan_flag && !wnet_vif->vm_scan_before_connect_flag
        && !wnet_vif->vm_chan_roaming_scan_flag && ss->scan_next_chan_index > 20) {
        wifi_mac_update_chan_overlapping_map(wnet_vif);
        ss->scan_ssid_count = count;

        if (wifi_mac_is_in_noisy_environment(wifimac)) {
            wifimac->scan_noisy_status = WIFINET_S_SCAN_ENV_NOISE;

        } else if (wifi_mac_is_in_clear_environment(wifimac)) {
            //clear environment, set to max gain
            wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, 174);
            wifimac->scan_noisy_status = WIFINET_S_SCAN_ENV_CLEAR;

        } else {
            wifimac->scan_noisy_status = WIFINET_S_SCAN_ENV_MID;
        }
    }

    AML_OUTPUT("%s scan_res:%d, chan_count:%d, thresh_unconnect:%d, thresh_connect:%d, scan_noisy_status:%d\n",
        __func__, count, ss->scan_next_chan_index, wifimac->scan_gain_thresh_unconnect,
        wifimac->scan_gain_thresh_connect, wifimac->scan_noisy_status);
    return 0;
}

void wifi_mac_scan_flush(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se=NULL, *next=NULL;

    DPRINTF(AML_DEBUG_WARNING, "<running> %s %d \n",__func__,__LINE__);

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se,next,&st->st_entry,se_list)
    {
        list_del_init(&se->se_list);
        list_del_init(&se->se_hash);
        FREE(se,"sta_add.se");
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);
}

static void update_roaming_candidate_chan(struct wifi_mac_scan_state *ss, struct wifi_channel *apchan, int rssi)
{
    int i =0;
    struct wifi_candidate_channel * worst_chan = NULL;

    WIFI_ROAMING_CHANNLE_LOCK(ss);
    for (i = 0; i < ROAMING_CANDIDATE_CHAN_MAX; i++) {
        if (ss->roaming_candidate_chans[i].channel) {
            /*update rssi */
            if (ss->roaming_candidate_chans[i].channel->chan_pri_num == apchan->chan_pri_num) {
                ss->roaming_candidate_chans[i].avg_rssi = (ss->roaming_candidate_chans[i].avg_rssi + (rssi * 3)) >> 2;
                WIFI_ROAMING_CHANNLE_UNLOCK(ss);
                return ;
           }

            /*get worst channel*/
            if (!worst_chan) {
                worst_chan = &ss->roaming_candidate_chans[i];
            }
            else if (worst_chan->avg_rssi > ss->roaming_candidate_chans[i].avg_rssi) {
                worst_chan = &ss->roaming_candidate_chans[i];
            }
        }
    }

    if (ss->roaming_candidate_chans_cnt > 5) {
        if (rssi > worst_chan->avg_rssi) {
            worst_chan->channel = apchan;
            worst_chan->avg_rssi = rssi;
        }
    }
    else {
        ss->roaming_candidate_chans[ss->roaming_candidate_chans_cnt].channel = apchan;
        ss->roaming_candidate_chans[ss->roaming_candidate_chans_cnt].avg_rssi = rssi;
        ss->roaming_candidate_chans_cnt++;
    }
    WIFI_ROAMING_CHANNLE_UNLOCK(ss);

}

void wifi_mac_update_roaming_candidate_chan(struct wlan_net_vif *wnet_vif,const struct wifi_mac_scan_param *sp, int rssi)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    int nssid = sp->ssid[1];
    static struct wifi_channel * apchan = NULL;

    apchan= wifi_mac_scan_sta_get_ap_channel(wnet_vif, (struct wifi_mac_scan_param *)sp);
    if (apchan == NULL) {
         DPRINTF(AML_DEBUG_INFO, "%s(%d) apchan = %p\n", __func__,__LINE__,apchan);
        return;
    }

    if ((sp->ssid[2] != 0) && ss->ss_ssid->len && nssid
        && !(memcmp(sp->ssid+2, ss->roaming_ssid.ssid, nssid))) {
        update_roaming_candidate_chan(ss, apchan, rssi);
//        pr_debug("\n\n[Roaming ssid:%s] len:%d chan:%d\n\n", ssidie_sprintf(sp->ssid), nssid, apchan->chan_pri_num);
    }

}

void wifi_mac_scan_rx(struct wlan_net_vif *wnet_vif, const struct wifi_mac_scan_param *sp,
    const struct wifi_frame *wh, int rssi, struct scaninfo_entry *oldse)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct scaninfo_table *st = ss->ScanTablePriv;
    const unsigned char *macaddr = wh->i_addr2;
    struct scaninfo_entry *se = NULL;
    struct wifi_scan_info *ise;
    static struct wifi_channel *apchan =NULL;
    int hash;
    int index = 0;
    struct wifi_mac_Rsnparms rsn = {0};
    int err, i;
    unsigned char found_flag = 0;

    if (ss->scan_StateFlags & SCANSTATE_F_DISCARD) {
        DPRINTF(AML_DEBUG_SCAN,"%s %d drop \n",__func__,__LINE__);
        return;
    }

    if (sp->ssid) {
        AML_PRINT(AML_DBG_MODULES_SCAN, "<running> %d %s chan %d \n", sp->ssid[1], ssidie_sprintf(sp->ssid), sp->chan);
    }

    apchan = wifi_mac_scan_sta_get_ap_channel(wnet_vif, (struct wifi_mac_scan_param *)sp);
    if (apchan == NULL) {
        AML_PRINT(AML_DBG_MODULES_SCAN, "ssid %s ,apchan = %p\n", ssidie_sprintf(sp->ssid), apchan);
        return;
    }

    if (oldse != NULL) {
        se = oldse;
    } else {
        se = (struct scaninfo_entry *)NET_MALLOC(sizeof(struct scaninfo_entry), GFP_ATOMIC, "sta_add.se");
    }

    if (se == NULL) {
        return ;
    }

    ise = &se->scaninfo;
    ise->SI_frame_len = sp->frame_len;
    if (sp->rsn != NULL) {
        saveie(&ise->SI_rsn_ie[0], sp->rsn);

        err = wifi_mac_parse_counterpart_rsn(&rsn, ise->SI_rsn_ie, 0);
        if (err != 0) {
            goto fail;

        } else {
            ise->si_rsn_capa = rsn.rsn_caps;
        }
    }

    if (sp->rsnxe != NULL) {
        saveie(&ise->SI_rsnx_ie[0], sp->rsnxe);
    }

    if (sp->country != NULL) {
        saveie(&ise->SI_country_ie[0], sp->country);
    }

    if (sp->bss_load != NULL) {
        saveie(&ise->SI_bss_load_ie[0], sp->bss_load);
    }

    WIFINET_ADDR_COPY(se->scaninfo.SI_macaddr, macaddr);
    DPRINTF(AML_DEBUG_SCAN, "<running> %s se %p se->se_list 0x%p, %s, mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
        __func__, se, &se->se_list, ssidie_sprintf(sp->ssid), macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

    if (sp->ssid && (sp->ssid[1] != 0) && (WIFINET_IS_PROBERSP(wh) || ise->SI_ssid[1] == 0))
        memcpy(ise->SI_ssid, sp->ssid, 2 + sp->ssid[1]);

    if (sp->rates) {
        KASSERT(sp->rates[1] <= WIFINET_RATE_MAXSIZE, ("rate set too large: %u", sp->rates[1]));
        memcpy(ise->SI_rates, sp->rates, 2 + sp->rates[1]);
    }

    if (sp->xrates != NULL) {
        KASSERT(sp->xrates[1] <= WIFINET_RATE_MAXSIZE, ("rate set too large: %u", sp->xrates[1]));
        memcpy(ise->SI_exrates, sp->xrates, 2+sp->xrates[1]);

    } else {
        ise->SI_exrates[1] = 0;
    }

    WIFINET_ADDR_COPY(ise->SI_bssid, wh->i_addr3);
    if (se->LastUpdateTime == 0)
        se->se_avgrssi = rssi;
    else
        se->se_avgrssi = (se->se_avgrssi + rssi*3)>>2;
    se->scaninfo.SI_rssi = se->se_avgrssi;
    memcpy(ise->SI_tstamp.data, sp->tstamp, sizeof(ise->SI_tstamp));

    ise->SI_intval = sp->bintval;
    ise->SI_capinfo = sp->capinfo;
    ise->SI_chan = apchan;
    ise->SI_erp = sp->erp;
    ise->SI_timoff = sp->timoff;
    if (sp->tim != NULL) {
        ise->SI_dtimperiod =  ((const struct wifi_mac_tim_ie *) sp->tim)->tim_period;
    }

#ifdef CONFIG_WAPI
    saveie(&ise->SI_wai_ie[0], sp->wai);
#endif //#ifdef CONFIG_WAPI

#ifdef CONFIG_P2P
    for (index = 0; index < MAX_P2PIE_NUM; index++)
    {
        if (sp->p2p[index] != NULL) {
            saveie(&ise->SI_p2p_ie[index][0], sp->p2p[index]);
        }
    }
#endif //#ifdef CONFIG_P2P

#ifdef CONFIG_WFD
    saveie(&ise->SI_wfd_ie[0], sp->wfd);
#endif //#ifdef CONFIG_WFD

    saveie(&ise->SI_wme_ie[0], sp->wme);
    saveie(&ise->SI_htcap_ie[0], sp->htcap);
    saveie(&ise->SI_htinfo_ie[0], sp->htinfo);
    saveie(&ise->SI_country_ie[0], sp->country);
    saveie(&ise->SI_wpa_ie[0], sp->wpa);
    if (sp->wps != NULL) {
        saveie(&ise->SI_wps_ie[0], sp->wps);
    }

    if (se->connectcnt && ((jiffies - se->ConnectTime) > WIFINET_CONNECT_CNT_AGE*HZ)) {
        se->connectcnt = 0;
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT, macaddr,
            "%s: fails %u HZ %d 0x%lx", __func__, se->connectcnt,HZ,jiffies);
    }

    se->LastUpdateTime = jiffies;
    se->se_valid = 1;

    saveie(ise->ie_ext_cap, sp->ext_cap);
    saveie(ise->ie_vht_cap, sp->vht_cap);
    saveie(ise->ie_vht_opt, sp->vht_opt);
    saveie(ise->ie_vht_tx_pwr, sp->vht_tx_pwr);
    saveie(ise->ie_vht_ch_sw, sp->vht_ch_sw);
    saveie(ise->ie_vht_ext_bss_ld, sp->vht_ext_bss_ld);
    saveie(ise->ie_vht_quiet_ch, sp->vht_quiet_ch);
    saveie(ise->ie_vht_opt_md_ntf, sp->vht_opt_md_ntf);

    hash = STA_HASH(macaddr);

    if (oldse == NULL) {
        /* Need lock here, because when 'olsde' is NULL, this
         * function was called with unlocked list.
         */
        WIFI_SCAN_SE_LIST_LOCK(st);
        list_add(&se->se_hash, &st->st_hash[hash]);
        list_add_tail(&se->se_list, &st->st_entry);
        WIFI_SCAN_SE_LIST_UNLOCK(st);
    }

    for (i=0; i<ss->ss_nssid; i++) {
        if (sp->ssid && sp->ssid[1] > 0 && sp->ssid[1] <= WIFINET_NWID_LEN) {
            if ((ss->ss_ssid[i].len == sp->ssid[1]) && (memcmp(ss->ss_ssid[i].ssid, &sp->ssid[2], ss->ss_ssid[i].len) == 0)) {
                found_flag = 1;
                break;
            }
        }
    }

    if ((ss->scan_CfgFlags & WIFINET_SCANCFG_CONNECT) && found_flag) {
        ss->scan_StateFlags |= SCANSTATE_F_GET_CONNECT_AP;
    }

    return;

fail:
    if (oldse) {
        /* We failed to update some existing entry. Remove it from
         * list before freeing memory. We don't need lock here,
         * because list is locked by caller!
         */
        list_del_init(&se->se_list);
        list_del_init(&se->se_hash);
    }
    FREE(se,"sta_add.se");
    pr_debug("[Micro]%s_%d\n", __func__, __LINE__);
    return;
}

static void quiet_intf (struct wlan_net_vif *wnet_vif, unsigned char enable)
{
    unsigned int qlen_real = WIFINET_SAVEQ_QLEN(&wnet_vif->vm_tx_buffer_queue);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (wnet_vif->vm_opmode == WIFINET_M_STA) {
        if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s, vid:%d, enable:%d, qlen_real:%d\n", __func__, wnet_vif->wnet_vif_id, enable, qlen_real);

            if (enable) {
                wnet_vif->vm_pstxqueue_flags |= WIFINET_PSQUEUE_PS4QUIET;
                wifimac->drv_priv->drv_ops.drv_set_is_mother_channel(wifimac->drv_priv, wnet_vif->wnet_vif_id, 0);
                wifi_mac_pwrsave_send_nulldata(wnet_vif->vm_mainsta, NULLDATA_PS, 1);

            } else {
                wnet_vif->vm_pstxqueue_flags &= ~WIFINET_PSQUEUE_PS4QUIET;

                if (qlen_real == 0) {
                    wifi_mac_pwrsave_send_nulldata(wnet_vif->vm_mainsta, NULLDATA_NONPS, 0);

                } else {
                    wifi_mac_buffer_txq_send_pre(wnet_vif);
                }
            }

        } else {
            wnet_vif->vm_pstxqueue_flags &= ~WIFINET_PSQUEUE_PS4QUIET;
        }
    }
}

void quiet_all_intf (struct wifi_mac *wifimac, unsigned char enable)
{
    struct wlan_net_vif *tmpwnet_vif = NULL, *tmpwnet_vif_next = NULL;

    list_for_each_entry_safe(tmpwnet_vif,tmpwnet_vif_next, &wifimac->wm_wnet_vifs, vm_next)
    {
        quiet_intf(tmpwnet_vif, enable);
    }
}

void wifi_mac_scan_notify_leave_or_back(struct wlan_net_vif *wnet_vif, unsigned char enable) {
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if ((wifimac->wm_nrunning == 1)
        || ((wifimac->wm_nrunning == 2) && concurrent_check_is_vmac_same_pri_channel(wifimac))) {
        quiet_all_intf(wifimac, enable);

    } else if (wifimac->wm_nrunning == 2) {
        quiet_intf(wnet_vif, enable);
    }
}

void wifi_mac_set_scan_time(struct wlan_net_vif *wnet_vif) {
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    if (wnet_vif->vm_scan_before_connect_flag) {
        ss->scan_chan_wait = wnet_vif->vm_scan_time_before_connect;

    } else if (wifimac->wm_nrunning && !(wifimac->wm_nrunning == 1 && IS_APSTA_CONCURRENT(aml_wifi_get_con_mode()))) {
        ss->scan_chan_wait = wnet_vif->vm_scan_time_connect;

    } else {
        ss->scan_chan_wait = wnet_vif->vm_scan_time_idle;
    }

    if (wnet_vif->vm_chan_switch_scan_flag) {
        ss->scan_chan_wait = wnet_vif->vm_scan_time_chan_switch;
    }
    //pr_debug("%s change scan time to:%d\n", __func__, ss->scan_chan_wait);
    return;
}

static int vm_scan_setup_chan(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_channel *c;
    static unsigned char chan_aware_cnt = 0;
    int i = 0;

#ifdef CONFIG_P2P
    struct wlan_net_vif *tmpwnet_vif;
    tmpwnet_vif = wifi_mac_get_wnet_vif_by_vid(wifimac, NET80211_P2P_VMAC);
#endif

    ss->scan_last_chan_index = 0;
    DPRINTF(AML_DEBUG_SCAN, "%s %d wifimac->wm_nchans=%d \n",
        __func__, __LINE__, wifimac->wm_nchans);

    if (wnet_vif->vm_chan_simulate_scan_flag) {
        ERROR_DEBUG_OUT("scan for tx_error\n");

        WIFI_CHANNEL_LOCK(wifimac);
        for (i = 0; i < wifimac->wm_nchans; i++) {
            c = &wifimac->wm_channels[i];
            if (chan_aware_cnt >= 10) {
                c->chan_flags &= ~WIFINET_CHAN_AWARE;
                chan_aware_cnt = 0;
            }

            /* both 2.4G and 5G set WIFINET_BW_20 flag.*/
            if ((c->chan_bw == WIFINET_BWC_WIDTH20) && (wifimac->wm_curchan != c)) {
                if ((aml_iwpriv_get_band() == CFG_BAND_B) && (!WIFINET_IS_CHAN_2GHZ(c))) {
                    continue;
                } else if ((aml_iwpriv_get_band() == CFG_BAND_A) && (!WIFINET_IS_CHAN_5GHZ(c))) {
                    continue;
                }

                ss->ss_chans[ss->scan_last_chan_index++] = c;
                break;
            }
        }
        WIFI_CHANNEL_UNLOCK(wifimac);

        wnet_vif->vm_chan_simulate_scan_flag = 0;

    } else if (wnet_vif->vm_scan_before_connect_flag) {
        ss->scan_last_chan_index = wnet_vif->vm_connchan.num * 3;

        for (i = 0; i <  wnet_vif->vm_connchan.num; i++) {
            ss->ss_chans[i] = wnet_vif->vm_connchan.conn_chan[i];
            ss->ss_chans[i + wnet_vif->vm_connchan.num] = wnet_vif->vm_connchan.conn_chan[i];
            ss->ss_chans[i + (wnet_vif->vm_connchan.num * 2)] = wnet_vif->vm_connchan.conn_chan[i];
            pr_debug("add scan connect chans:%d\n", wnet_vif->vm_connchan.conn_chan[i]->chan_pri_num);
        }

    } else if (wnet_vif->vm_chan_roaming_scan_flag && !wnet_vif->vm_wmac->wm_scan->roaming_full_scan) {
        pr_debug("scan roaming_candidate_chans \n");
        WIFI_ROAMING_CHANNLE_LOCK(ss);
        ss->scan_last_chan_index = wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans_cnt;

        for (i = 0; i < ss->scan_last_chan_index; i++) {
            ss->ss_chans[i] = wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans[i].channel;
        }
        WIFI_ROAMING_CHANNLE_UNLOCK(ss);

    } else if (wnet_vif->vm_chan_switch_scan_flag) {
        ss->scan_last_chan_index = 1;
        ss->ss_chans[0] = wnet_vif->vm_switchchan;

    } else {
        pr_debug("scan all chans \n");

        /*clean roamin candidate channel*/
        WIFI_ROAMING_CHANNLE_LOCK(wnet_vif->vm_wmac->wm_scan);
        wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans_cnt = 0;
        memset(wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans, 0, sizeof(wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans));
        WIFI_ROAMING_CHANNLE_UNLOCK(wnet_vif->vm_wmac->wm_scan);

        WIFI_CHANNEL_LOCK(wifimac);
        for (i = 0; i < wifimac->wm_nchans; i++)
        {
            c = &wifimac->wm_channels[i];
            if (chan_aware_cnt >= 10) {
                c->chan_flags &= ~WIFINET_CHAN_AWARE;
                chan_aware_cnt = 0;
            }

            /* both 2.4G and 5G set WIFINET_BW_20 flag.*/
            if (c->chan_bw == WIFINET_BWC_WIDTH20)
            {
    #ifdef CONFIG_P2P
                if ((wnet_vif->vm_p2p != NULL) && (wnet_vif->vm_p2p->p2p_enable)
                    && (wnet_vif->vm_p2p->social_channel))
                {
                    if ((c->chan_cfreq1 != SOCIAL_CHAN_1)
                       && (c->chan_cfreq1 != SOCIAL_CHAN_2)
                       && (c->chan_cfreq1 != SOCIAL_CHAN_3))
                    {
                        continue;
                    }

                    if ((aml_iwpriv_get_band() == CFG_BAND_B) && (!WIFINET_IS_CHAN_2GHZ(c))) {
                        continue;
                    } else if ((aml_iwpriv_get_band() == CFG_BAND_A) && (!WIFINET_IS_CHAN_5GHZ(c))) {
                        continue;
                    }

                    ss->ss_chans[ss->scan_last_chan_index++] = c;
                }
                else
    #endif//CONFIG_P2P
                {
                    if ((aml_iwpriv_get_band() == CFG_BAND_B) && (!WIFINET_IS_CHAN_2GHZ(c))) {
                        continue;
                    } else if ((aml_iwpriv_get_band() == CFG_BAND_A) && (!WIFINET_IS_CHAN_5GHZ(c))) {
                        continue;
                    }

                    ss->ss_chans[ss->scan_last_chan_index++] = c;
                    DPRINTF(AML_DEBUG_SCAN,"%s %d chan_index=%d,chan_pri_num=%d,flag=%x,freq %d %p\n",
                        __func__,__LINE__,i,c->chan_pri_num,c->chan_flags,c->chan_cfreq1,c);
                }
            }
        }
        WIFI_CHANNEL_UNLOCK(wifimac);
    }

    chan_aware_cnt++;
    wifi_mac_set_scan_time(wnet_vif);
    DPRINTF(AML_DEBUG_SCAN, "%s vid:%d ss->scan_next_chan_index=%d \
        ss->scan_last_chan_index=%d, ss->scan_chan_wait=0x%xms, HZ = %d LINUX_VERSION_CODE =%x\n",
        __FUNCTION__, wnet_vif->wnet_vif_id, ss->scan_next_chan_index, ss->scan_last_chan_index,
        ss->scan_chan_wait, HZ, LINUX_VERSION_CODE);

    return 0;
}

#ifdef FW_RF_CALIBRATION
static void
wifi_mac_scan_send_probe_timeout(SYS_TYPE param1,SYS_TYPE param2,
    SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac_scan_state *ss = (struct wifi_mac_scan_state *)param1;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    unsigned char i;
    unsigned char j;

    os_timer_ex_cancel(&ss->ss_probe_timer, CANCEL_SLEEP);
    //pr_debug("%s, ss->scan_StateFlags:%08x, ss->ss_nssid:%d\n", __func__, ss->scan_StateFlags, ss->ss_nssid);

    if (ss->scan_CfgFlags & WIFINET_SCANCFG_ACTIVE)
    {
        struct net_device *dev = wnet_vif->vm_ndev;

        for (i = 0; i < ss->ss_nssid; i++)
            for (j = 0; j < 1; ++j)
                wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, dev->broadcast,
                    dev->broadcast, ss->ss_ssid[i].ssid, ss->ss_ssid[i].len, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);

        if (!wnet_vif->vm_p2p->p2p_enable) {
            wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, dev->broadcast,
                dev->broadcast, "", 0, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);
        }
    }
}

static int wifi_mac_scan_send_probe_timeout_ex(void *arg)
{
    struct wifi_mac_scan_state *ss = (struct wifi_mac_scan_state *) arg;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    //pr_debug("%s, ss->scan_StateFlags:%08x\n", __func__, ss->scan_StateFlags);

    WIFI_SCAN_LOCK(ss);
    if (ss->scan_StateFlags & SCANSTATE_F_SEND_PROBEREQ_AGAIN) {
        ss->scan_StateFlags &= ~SCANSTATE_F_SEND_PROBEREQ_AGAIN;
        wifi_mac_add_work_task(wifimac, wifi_mac_scan_send_probe_timeout, NULL,
            (SYS_TYPE)arg, (SYS_TYPE)wnet_vif, 0, 0, 0);

        DPRINTF(AML_DEBUG_SCAN, "%s %d ss->scan_next_chan_index = %d, ss->scan_StateFlags:%08x\n",
            __func__,__LINE__,ss->scan_next_chan_index, ss->scan_StateFlags);
    }
    WIFI_SCAN_UNLOCK(ss);

    return OS_TIMER_NOT_REARMED;
}


static enum hrtimer_restart
wifi_mac_scan_chk_leakap_done_process(struct hrtimer *timer)
{
    static unsigned char count = 0;
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();

    if (wifimac->wm_scan->scan_StateFlags & SCANSTATE_F_RX_LEAKAP_HAPPEN) {
        wifimac->wm_scan->scan_StateFlags &= ~SCANSTATE_F_RX_LEAKAP_HAPPEN;

        count ++;
        if (count < 5) {
            wifimac->wm_scan->scan_StateFlags |= SCANSTATE_F_RX_CHKING_LEAKAP_PKT;
            wifimac->wm_scan->scan_kt = ktime_set(0, 2000000);
            hrtimer_forward_now(&wifimac->wm_scan->scan_hr_timer, wifimac->wm_scan->scan_kt);
            return HRTIMER_RESTART;
        }
    }

    AML_PRINT(AML_DBG_MODULES_SCAN, "retry %d\n", count);
    wifimac->wm_scan->scan_StateFlags &= ~SCANSTATE_F_RX_CHKING_LEAKAP_PKT;
    if ((wifimac->wm_scan->scan_StateFlags & SCANSTATE_F_CHANNEL_SWITCH_COMPLETE) == 0) {
        os_timer_ex_cancel(&wifimac->wm_scan->ss_scan_timer, 1);
        wifi_mac_scan_timeout_ex(wifimac->wm_scan);
    }
    count = 0;
    return HRTIMER_NORESTART;
}


static void wifi_mac_scan_chk_leakap_hrtimer_attach(struct wifi_mac *wifimac)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
    hrtimer_setup(&wifimac->wm_scan->scan_hr_timer, wifi_mac_scan_chk_leakap_done_process,
                  CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
    hrtimer_init(&wifimac->wm_scan->scan_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    wifimac->wm_scan->scan_hr_timer.function = wifi_mac_scan_chk_leakap_done_process;
#endif
}


static void wifi_mac_scan_chk_leakap_hrtimer_start(struct wifi_mac *wifimac)
{
    wifimac->wm_scan->scan_kt = ktime_set(0, 2000000);
    hrtimer_start(&wifimac->wm_scan->scan_hr_timer, wifimac->wm_scan->scan_kt, HRTIMER_MODE_REL);
}

static void wifi_mac_scan_chk_leakap_hrtimer_cancel(struct wifi_mac *wifimac)
{
    hrtimer_cancel(&wifimac->wm_scan->scan_hr_timer);
}

void wifi_mac_scan_chking_leakap(void * station, struct wifi_frame *wh)
{
        struct wifi_station *sta = (struct wifi_station *)station;
        struct wifi_mac *wifimac = sta->sta_wmac;
        struct wifi_mac_scan_state *ss = wifimac->wm_scan;

        if ((ss->scan_StateFlags & SCANSTATE_F_START) && (ss->scan_StateFlags & SCANSTATE_F_NOTIFY_AP)
            && (ss->scan_StateFlags & SCANSTATE_F_RX_CHKING_LEAKAP_PKT)) {
            if (WIFINET_ADDR_EQ(wh->i_addr1, sta->sta_wnet_vif->vm_myaddr)) {
                ss->scan_StateFlags |= SCANSTATE_F_RX_LEAKAP_HAPPEN;
            }
        }
}


static void wifi_mac_scan_channel(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    unsigned char i = 0;
    struct wifi_channel *chan;
    enum wifi_mac_macmode last_mac_mode;

    //pr_debug("%s, ss->scan_StateFlags:%08x\n", __func__, ss->scan_StateFlags);
    if (!(ss->scan_StateFlags & SCANSTATE_F_CHANNEL_SWITCH_COMPLETE)) {
        return;
    }
    ss->scan_StateFlags &= ~SCANSTATE_F_WAIT_CHANNEL_SWITCH;
    ss->scan_StateFlags &= ~SCANSTATE_F_CHANNEL_SWITCH_COMPLETE;
    ss->scan_StateFlags &= ~SCANSTATE_F_DISCARD;

    chan = ss->ss_chans[ss->scan_next_chan_index++];

    if (!((wifimac->wm_nrunning == 2) && (!concurrent_check_is_vmac_same_pri_channel(wifimac)))) {
        if (chan->chan_flags & WIFINET_CHAN_DFS) {
            os_timer_ex_start_period(&ss->ss_scan_timer, WIFINET_SCAN_DEFAULT_INTERVAL);

        } else {
            os_timer_ex_start_period(&ss->ss_scan_timer, ss->scan_chan_wait);
        }
    }

    if ((wifimac->wm_nrunning == 1)
        || ((wifimac->wm_nrunning == 2) && (concurrent_check_is_vmac_same_pri_channel(wifimac)))) {
        ss->scan_StateFlags |= SCANSTATE_F_RESTORE;
        ss->scan_StateFlags &= ~SCANSTATE_F_NOTIFY_AP;
        ss->scan_StateFlags &= ~SCANSTATE_F_TX_DONE;
    }

    /* vm_mac_mode default is 11GNAC */
    last_mac_mode = wnet_vif->vm_mac_mode;
    if (chan->chan_pri_num >= 1 && chan->chan_pri_num <= 14) {
        if (wnet_vif->vm_p2p->p2p_enable) {
            wnet_vif->vm_mac_mode = WIFINET_MODE_11GN;
        } else {
            wnet_vif->vm_mac_mode = WIFINET_MODE_11BGN;
        }

    } else {
        wnet_vif->vm_mac_mode = WIFINET_MODE_11GNAC;
    }

    if (last_mac_mode != wnet_vif->vm_mac_mode)
        wifi_mac_set_legacy_rates(&wnet_vif->vm_legacy_rates, wnet_vif);

    if (((ss->scan_CfgFlags & WIFINET_SCANCFG_ACTIVE) && !(chan->chan_flags & WIFINET_CHAN_DFS))
        ||((ss->scan_CfgFlags & WIFINET_SCANCFG_ACTIVE) && (chan->chan_flags & WIFINET_CHAN_AWARE))) {
        struct net_device *dev = wnet_vif->vm_ndev;

        DPRINTF(AML_DEBUG_SCAN, "%s vid:%d, next_chan_index = %d, chan=%d freq=%d, p2p_enable:%d\n", __func__,
            wnet_vif->wnet_vif_id, ss->scan_next_chan_index, chan->chan_pri_num, chan->chan_cfreq1, wnet_vif->vm_p2p->p2p_enable);

        if (!wnet_vif->vm_p2p->p2p_enable && !wnet_vif->vm_chan_switch_scan_flag) {
             wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, dev->broadcast,
                 dev->broadcast, "", 0, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);
        }

        for (i = 0; i < ss->ss_nssid; i++) {
            if (wnet_vif->vm_scan_before_connect_flag) {
                wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, wnet_vif->vm_connchan.da,
                    wnet_vif->vm_connchan.bssid, ss->ss_ssid[i].ssid, ss->ss_ssid[i].len, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);

            } else if (wnet_vif->vm_chan_switch_scan_flag) {
                wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, wnet_vif->vm_des_bssid,
                    wnet_vif->vm_des_bssid, ss->ss_ssid[i].ssid, ss->ss_ssid[i].len, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);

            } else {
                wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, dev->broadcast,
                    dev->broadcast, ss->ss_ssid[i].ssid, ss->ss_ssid[i].len, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);
            }
        }

        ss->scan_StateFlags |= SCANSTATE_F_SEND_PROBEREQ_AGAIN;
        if (ss->scan_chan_wait == wnet_vif->vm_scan_time_connect) {
            os_timer_ex_start_period(&ss->ss_probe_timer, 8);

        } else {
            os_timer_ex_start_period(&ss->ss_probe_timer, 20);
        }
    }

    DPRINTF(AML_DEBUG_SCAN, "%s OS_SET_TIMER = %d next_chn\n", __func__, ss->scan_chan_wait);
}

static void wifi_mac_switch_scan_channel(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct wifi_channel *chan;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;

    //pr_debug("%s, ss->scan_StateFlags:%08x\n", __func__, ss->scan_StateFlags);
    if (ss->scan_next_chan_index >= ss->scan_last_chan_index) {
        DPRINTF(AML_DEBUG_ERROR, " %s (scan_next_chan_index >= scan_last_chan_index) drop!!!\n", __func__);
        return;
    }

    chan = ss->ss_chans[ss->scan_next_chan_index];
    if ((wifimac->wm_curchan != NULL) && (chan->chan_pri_num == wifimac->wm_curchan->chan_pri_num)) {
        ss->scan_StateFlags |= SCANSTATE_F_CHANNEL_SWITCH_COMPLETE;
        wifi_mac_scan_channel(wifimac);

    } else {
        ss->scan_StateFlags |= SCANSTATE_F_WAIT_CHANNEL_SWITCH;
        wifi_mac_ChangeChannel(wifimac, chan, 0, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);
        os_timer_ex_start_period(&ss->ss_scan_timer, 30);
    }
}
#endif

void scan_next_chan(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    unsigned char i = 0;
    unsigned char j = 0;
    struct wifi_channel *chan;
    enum wifi_mac_macmode last_mac_mode;
    unsigned int send_packet_num = 2;

    if (ss->scan_next_chan_index >= ss->scan_last_chan_index)
    {
        DPRINTF(AML_DEBUG_ERROR, " %s (scan_next_chan_index >= scan_last_chan_index) drop!!!\n", __func__);
        return;
    }

    if (!((wifimac->wm_nrunning == 2) && (!concurrent_check_is_vmac_same_pri_channel(wifimac)))) {
        //pr_debug("%s\n", __func__);
        os_timer_ex_start_period(&ss->ss_scan_timer, ss->scan_chan_wait);
    }

    if ((wifimac->wm_nrunning == 1)
        || ((wifimac->wm_nrunning == 2) && (concurrent_check_is_vmac_same_pri_channel(wifimac)))) {
        ss->scan_StateFlags |= SCANSTATE_F_RESTORE;
        ss->scan_StateFlags &= ~SCANSTATE_F_NOTIFY_AP;
        ss->scan_StateFlags &= ~SCANSTATE_F_TX_DONE;
    }
    chan = ss->ss_chans[ss->scan_next_chan_index++];
    wifi_mac_ChangeChannel(wifimac, chan, 0, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);

    ss->scan_StateFlags &= ~SCANSTATE_F_DISCARD;
    /* vm_mac_mode default is 11GNAC */
    last_mac_mode = wnet_vif->vm_mac_mode;
    if (chan->chan_pri_num >= 1 && chan->chan_pri_num <= 14) {
        if (wnet_vif->vm_p2p->p2p_enable) {
            wnet_vif->vm_mac_mode = WIFINET_MODE_11GN;
        } else {
            wnet_vif->vm_mac_mode = WIFINET_MODE_11BGN;
        }

    } else {
        wnet_vif->vm_mac_mode = WIFINET_MODE_11GNAC;
    }

    if (last_mac_mode != wnet_vif->vm_mac_mode)
        wifi_mac_set_legacy_rates(&wnet_vif->vm_legacy_rates, wnet_vif);

    if (ss->scan_CfgFlags & WIFINET_SCANCFG_ACTIVE)
    {
        struct net_device *dev = wnet_vif->vm_ndev;

        DPRINTF(AML_DEBUG_SCAN, "%s vid:%d, next_chan_index = %d, chan=%d freq=%d, p2p_enable:%d\n", __func__,
            wnet_vif->wnet_vif_id, ss->scan_next_chan_index, chan->chan_pri_num, chan->chan_cfreq1, wnet_vif->vm_p2p->p2p_enable);

        if (!wnet_vif->vm_p2p->p2p_enable) {
            for (i = 0; i < send_packet_num; i++)
                wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, dev->broadcast,
                    dev->broadcast, "", 0, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);
            send_packet_num = 1;
        }

        for (i = 0; i < ss->ss_nssid; i++)
            for (j = 0; j < send_packet_num; j++)
                wifi_mac_send_probereq(wnet_vif->vm_mainsta, wnet_vif->vm_myaddr, dev->broadcast,
                    dev->broadcast, ss->ss_ssid[i].ssid, ss->ss_ssid[i].len, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);
    }

    DPRINTF(AML_DEBUG_SCAN, "%s OS_SET_TIMER = %d next_chn\n", __func__, ss->scan_chan_wait);
}


static int wifi_mac_scan_buff_and_chk_tx(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *tmpwnet_vif = NULL, *tmpwnet_vif_next = NULL;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct drv_private *drv_priv = wifimac->drv_priv;

    list_for_each_entry_safe(tmpwnet_vif,tmpwnet_vif_next, &wifimac->wm_wnet_vifs, vm_next) {
        if ((tmpwnet_vif->vm_opmode == WIFINET_M_STA) && (tmpwnet_vif->vm_state == WIFINET_S_CONNECTED)) {
            tmpwnet_vif->vm_pstxqueue_flags |= WIFINET_PSQUEUE_PS4QUIET;
            wifimac->drv_priv->drv_ops.drv_set_is_mother_channel(wifimac->drv_priv, tmpwnet_vif->wnet_vif_id, 0);
        }
    }

    if (!drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
        ss->scan_StateFlags |= SCANSTATE_F_WAIT_PKT_CLEAR;
        ss->scan_StateFlags &= ~SCANSTATE_F_TX_DONE;
        return 1;

    } else {
        ss->scan_StateFlags |= SCANSTATE_F_TX_DONE;
        return 0;
    }
}


static void wifi_mac_scan_notify_ap(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *tmpwnet_vif = NULL, *tmpwnet_vif_next = NULL;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    list_for_each_entry_safe(tmpwnet_vif,tmpwnet_vif_next, &wifimac->wm_wnet_vifs, vm_next) {
        if ((tmpwnet_vif->vm_opmode == WIFINET_M_STA) && (tmpwnet_vif->vm_state == WIFINET_S_CONNECTED)) {
            wifi_mac_pwrsave_send_nulldata(tmpwnet_vif->vm_mainsta, NULLDATA_PS, 1);
            ss->scan_StateFlags |= SCANSTATE_F_NOTIFY_AP;
        }
    }
}


static void wifi_mac_scan_timeout(SYS_TYPE param1,SYS_TYPE param2,
    SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac_scan_state *ss = (struct wifi_mac_scan_state *) param1;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned char scandone;
    unsigned char need_notify_ap = 1;
    struct wifi_channel *chan = WIFINET_CHAN_ERR;

    os_timer_ex_cancel(&ss->ss_scan_timer, CANCEL_SLEEP);

    //check replayer count
    if (wifimac->wm_scanplayercnt != (unsigned long)param5) {
        DPRINTF(AML_DEBUG_WARNING, "%s wm_scanplayercnt %ld, ignore... \n", __func__, wifimac->wm_scanplayercnt);
        return;
    }

    if (!wnet_vif->vm_mainsta) {
        DPRINTF(AML_DEBUG_WARNING, "%s vid:%d vm_mainsta is NULL, cancel scan\n", __func__, wnet_vif->wnet_vif_id);
        ss->scan_StateFlags |= SCANSTATE_F_CANCEL;
    }

    if (!(wifimac->wm_flags & WIFINET_F_SCAN)) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d drop wm_flags  0x%x \n",__func__,__LINE__,wifimac->wm_flags );
        goto end;
    }

    if (ss->scan_StateFlags & SCANSTATE_F_DISCONNECT_REQ_CANCEL) {
        DPRINTF(AML_DEBUG_WARNING, "%s end scan due to disconnect, no need to restore channel\n", __func__);
        goto end;
    }

    if (ss->scan_StateFlags & SCANSTATE_F_GET_CONNECT_AP) {
        DPRINTF(AML_DEBUG_WARNING, "%s find connect ap\n", __func__);
        goto end;
    }

    if (!(ss->scan_StateFlags & SCANSTATE_F_START)) {
        DPRINTF(AML_DEBUG_WARNING, "%s SCANSTATE_F_START flag not set, end scan\n", __func__);
        goto end;
    }

    scandone = (ss->scan_next_chan_index >= ss->scan_last_chan_index) ||
        (ss->scan_StateFlags & SCANSTATE_F_CANCEL);

    DPRINTF(AML_DEBUG_SCAN,"%s(%d) nxt chn 0x%x, lst chn 0x%x ss_flag 0x%x scandone %d\n", __func__, __LINE__,
        ss->scan_next_chan_index, ss->scan_last_chan_index, ss->scan_StateFlags, scandone);

    if (ss->scan_next_chan_index < ss->scan_last_chan_index) {
        chan = ss->ss_chans[ss->scan_next_chan_index];
        if ((chan != WIFINET_CHAN_ERR) && (wifimac->wm_curchan != WIFINET_CHAN_ERR)
            && (chan->chan_pri_num == wifimac->wm_curchan->chan_pri_num) && (wifimac->wm_curchan->chan_bw == 0)) {
            need_notify_ap = 0;
        }
    }

    //pr_debug("%s, ss->scan_StateFlags:%08x, scandone:%d, need_notify_ap:%d\n", __func__, ss->scan_StateFlags, scandone, need_notify_ap);
    if (wifimac->wm_nrunning != 0) {
        if (ss->scan_StateFlags & SCANSTATE_F_RESTORE) {
            wifi_mac_restore_wnet_vif_channel(wnet_vif);
            ss->scan_StateFlags &= ~SCANSTATE_F_RESTORE;
            ss->scan_StateFlags |= SCANSTATE_F_DISCARD;

            if (scandone) {
                DPRINTF(AML_DEBUG_WARNING, "%s scandone, end scan\n", __func__);
                goto end;
            }

            ss->scan_StateFlags |= SCANSTATE_F_WAIT_TBTT;
            os_timer_ex_start_period(&ss->ss_scan_timer, WIFINET_SCAN_DEFAULT_INTERVAL);
            return;

        } else if  (!scandone && need_notify_ap) {

            struct wlan_net_vif *connect_wnet = wifi_mac_running_wnet_vif(wifimac);

            if (!((connect_wnet != NULL) && (connect_wnet->vm_opmode == WIFINET_M_HOSTAP))) {
                if ((wifimac->wm_nrunning == 1) || ((wifimac->wm_nrunning == 2) && concurrent_check_is_vmac_same_pri_channel(wifimac))) {                    

                    if (!(ss->scan_StateFlags & SCANSTATE_F_TX_DONE)) {
                        if (wifi_mac_scan_buff_and_chk_tx(wifimac)) {
                            return;
                        }
                    }

                    if (!(ss->scan_StateFlags & SCANSTATE_F_NOTIFY_AP)) {
                        wifi_mac_scan_notify_ap(wifimac);
                        os_timer_ex_start_period(&wifimac->wm_scan->ss_scan_timer, LEAKY_AP_DET_WIN);
                        return;
                    }
                }
            } else {
                //add NOA in the near future
            }
        }
    }

    DPRINTF(AML_DEBUG_SCAN,"%s done 0x%x, ss_flag 0x%x\n", __func__, scandone, ss->scan_StateFlags);
    if (!scandone) {
        #ifndef FW_RF_CALIBRATION
            scan_next_chan(wifimac);
            return;
        #else
            if (ss->scan_StateFlags & SCANSTATE_F_CHANNEL_SWITCH_COMPLETE) {
                wifi_mac_scan_channel(wifimac);

            } else {
                wifi_mac_switch_scan_channel(wifimac);
            }
            return;
        #endif
    }

end:
    wifi_mac_end_scan(ss);
    return;
}

static void scan_timeout_work(struct work_struct *work)
{
    struct wifi_mac_scan_state *ss = container_of(work,
                                                  struct wifi_mac_scan_state,
                                                  timeout_work);
    struct wlan_net_vif *wnet_vif;
    struct wifi_mac *wifimac;

    wnet_vif = ss->VMacPriv;
    wifimac = wnet_vif->vm_wmac;

    wifi_mac_scan_timeout((SYS_TYPE)ss, (SYS_TYPE)wnet_vif, 0, 0,
                          (SYS_TYPE)wifimac->wm_scanplayercnt);
}


int wifi_mac_scan_timeout_ex(void *arg)
{
    struct wifi_mac_scan_state *ss = (struct wifi_mac_scan_state *) arg;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    //pr_debug("%s, ss->scan_StateFlags:%08x\n", __func__, ss->scan_StateFlags);

    WIFI_SCAN_LOCK(ss);
    if (ss->scan_StateFlags & SCANSTATE_F_WAIT_TBTT) {
        ss->scan_StateFlags &= ~SCANSTATE_F_WAIT_TBTT;
    }

    if (ss->scan_StateFlags & SCANSTATE_F_WAIT_CHANNEL_SWITCH) {
        DPRINTF(AML_DEBUG_WARNING,"%s %d switch channel timeout \n",__func__,__LINE__);
        ss->scan_StateFlags &= ~SCANSTATE_F_WAIT_CHANNEL_SWITCH;
        ss->scan_StateFlags |= SCANSTATE_F_CHANNEL_SWITCH_COMPLETE;
    }

    if (ss->scan_StateFlags & SCANSTATE_F_SEND_PROBEREQ_AGAIN) {
        ss->scan_StateFlags &= ~SCANSTATE_F_SEND_PROBEREQ_AGAIN;
    }

    wifimac->wm_scanplayercnt++;

    if (wifimac->drv_priv->hal_priv->work_thread) {
        wifi_mac_add_work_task(wifimac, wifi_mac_scan_timeout, NULL,
           (SYS_TYPE)arg, (SYS_TYPE)wnet_vif, 0, 0, (SYS_TYPE)wifimac->wm_scanplayercnt);
    } else {
       /* If 'work_thread' is NULL, this means that we are
        * during module unloading process. Threads are killed,
        * but we need to execute callback 'wifi_mac_scan_timeout',
        * so use system workqueue to do that.
        * May be we can avoid using workqueue approach, but just
        * call 'wifi_mac_scan_timeout' here in current context,
        * but it requires more synchronization with scan timer,
        * scheduling because current function is also timer callback
        * thus called in softirq context, while 'wifi_mac_scan_timeout'
        * may sleep.
        */
        schedule_work(&ss->timeout_work);
    }

    WIFI_SCAN_UNLOCK(ss);

    DPRINTF(AML_DEBUG_SCAN, "%s %d ss->scan_next_chan_index = %d, ss->scan_StateFlags:%08x\n",
        __func__,__LINE__,ss->scan_next_chan_index, ss->scan_StateFlags);

    return OS_TIMER_NOT_REARMED;
}

int wifi_mac_scan_abort_ex(void *arg)
{
    struct wifi_mac_scan_state *ss = (struct wifi_mac_scan_state *) arg;
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;


    if (wifimac->wm_flags & WIFINET_F_SCAN)
    {
        ss->scan_StateFlags |= SCANSTATE_F_CANCEL;
        if (ss->scan_StateFlags & SCANSTATE_F_START)
        {
            os_timer_ex_cancel(&wifimac->wm_scan->ss_scan_timer, 1);
            wifi_mac_scan_timeout_ex(wifimac->wm_scan);
        }
    }

    DPRINTF(AML_DEBUG_WARNING, "%s %d ss->scan_StateFlags:%08x\n",
        __func__,__LINE__, ss->scan_StateFlags);

    return OS_TIMER_NOT_REARMED;
}

static void
scan_start_task(SYS_TYPE param1,SYS_TYPE param2,
    SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    WIFI_SCAN_LOCK(ss);
    if ((wnet_vif->wnet_vif_replaycounter != (int)param5) || (wifimac->wm_scanplayercnt != (unsigned long)param3)
        || (ss->scan_StateFlags & SCANSTATE_F_START) || !(wifimac->wm_flags & WIFINET_F_SCAN)) {
        pr_debug("%s scan_StateFlags:%04x, wm_scanplayercnt:%ld\n", __func__, ss->scan_StateFlags, wifimac->wm_scanplayercnt);
        WIFI_SCAN_UNLOCK(ss);
        return;
    }

    ss->scan_StateFlags = 0;
    ss->scan_StateFlags |= SCANSTATE_F_START;
    WIFI_SCAN_UNLOCK(ss);
    os_timer_ex_cancel(&ss->ss_scan_timer, CANCEL_SLEEP);

    if ((ss->scan_CfgFlags & WIFINET_SCANCFG_CHANSET) == 0)
    {
        DPRINTF(AML_DEBUG_SCAN, "%s(%d)\n", __func__, __LINE__);
        vm_scan_setup_chan(ss,wnet_vif);
    }
    wifimac->drv_priv->stop_noa_flag = 0;
    wifi_mac_scan_start(wifimac);

    //set up scan channel index to 0
    ss->scan_next_chan_index = 0;
    pr_debug("%s wm_nrunning:%d\n", __func__, wifimac->wm_nrunning);
    os_timer_ex_start_period(&ss->ss_scan_abort_timer, WIFINET_SCAN_ABORT_TIME);

    if (wifimac->wm_nrunning == 0) {
        wifi_mac_scan_timeout_ex(ss);

    } else {
        ss->scan_StateFlags |= SCANSTATE_F_WAIT_TBTT;
        os_timer_ex_start_period(&ss->ss_scan_timer, 110);
    }

    return;
}

void wifi_mac_check_switch_chan_result(struct wlan_net_vif * wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL, *SI_next = NULL;
    unsigned char find = 0;

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se,SI_next,&st->st_entry,se_list)
    {
        se->scaninfo.SI_age = jiffies - se->LastUpdateTime;
        if (!strcmp(se->scaninfo.SI_ssid + 2, wnet_vif->vm_des_ssid[0].ssid)
            && WIFINET_ADDR_EQ(wnet_vif->vm_des_bssid, se->scaninfo.SI_bssid)
            && wnet_vif->vm_switchchan->chan_pri_num == se->scaninfo.SI_chan->chan_pri_num) {
            find = 1;
        }
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);
    if (find == 1 || wnet_vif->vm_chan_switch_scan_count == 5) {
        if (wnet_vif->vm_mainsta) {
            wnet_vif->vm_mainsta->sta_channel_switch_mode = 0;
        }

        wnet_vif->vm_chan_switch_scan_count = 0;
        wnet_vif->vm_chan_switch_scan_flag = 0;
    }
}

void wifi_mac_end_scan( struct wifi_mac_scan_state *ss)
{
    struct wlan_net_vif *wnet_vif = ss->VMacPriv;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int find_roaming_node = 0;
    static int before_conn_scan_times = 0;

    if (!(wifimac->wm_flags & WIFINET_F_SCAN)) {
        ERROR_DEBUG_OUT("not in scan status\n");
        return;
    }

    os_timer_ex_cancel(&ss->ss_scan_timer, CANCEL_SLEEP);
    os_timer_ex_cancel(&ss->ss_scan_abort_timer, CANCEL_SLEEP);
    DPRINTF(AML_DEBUG_SCAN, "%s chan_index = %d ,scan_CfgFlags 0x%x, wifimac->wm_nrunning is:%d\n",
        __func__, ss->scan_next_chan_index, ss->scan_CfgFlags, wifimac->wm_nrunning);

    if (wifimac->wm_nrunning == 1) {
        struct wlan_net_vif *connect_wnet = wifi_mac_running_wnet_vif(wifimac);
        if (connect_wnet->vm_opmode == WIFINET_M_HOSTAP &&
            connect_wnet->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON].length &&
            connect_wnet->vm_p2p->p2p_enable == 1) {
            pr_debug("%s,%d, clear noa ie\n", __func__, __LINE__);
            connect_wnet->vm_p2p->ap_mode_set_noa_enable = 0;
            wifi_mac_rm_app_ie(&connect_wnet->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON]);
            vm_p2p_update_beacon_app_ie(connect_wnet);
            wifimac->drv_priv->stop_noa_flag = 1;
            if (P2P_NoA_START_FLAG(connect_wnet->vm_p2p->HiP2pNoaCountNow)) {
                p2p_noa_start_irq(connect_wnet->vm_p2p, wifimac->drv_priv);
            }
            connect_wnet->vm_p2p->HiP2pNoaCountNow = 0;
        }
    } else {
        wifimac->drv_priv->stop_noa_flag = 0;
    }

    wifi_mac_scan_end(wifimac);
    wifimac->wm_lastscan = jiffies;
    ss->scan_StateFlags = 0;

    /* restore mac mode after scanning */
    if (wnet_vif->vm_mainsta) {
        wnet_vif->vm_mac_mode = wnet_vif->vm_mainsta->sta_bssmode;
        wifi_mac_set_legacy_rates(&wnet_vif->vm_legacy_rates, wnet_vif);
    }

    /*if in roaming mode ,check if can roaming*/
    if (wnet_vif->vm_mainsta != NULL && wnet_vif->vm_chan_roaming_scan_flag) {
        struct scaninfo_entry *best_node = &wnet_vif->vm_connect_scan_entry;

        if (wifi_mac_scan_get_best_node(ss, wnet_vif, best_node)) {
            best_node->se_valid = 0;
            if (memcmp(best_node->scaninfo.SI_bssid, wnet_vif->vm_mainsta->sta_bssid, WIFINET_ADDR_LEN)) {
                wifi_mac_scan_set_match_node(ss, wnet_vif);
                best_node->se_valid = 1;
                wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
                find_roaming_node = 1;
                wnet_vif->vm_wmac->wm_scan->roaming_full_scan = 0;
            }
        }
    }

    if ((wnet_vif->vm_opmode != WIFINET_M_HOSTAP)
       && (wnet_vif->vm_opmode != WIFINET_M_P2P_GO)
       && (wnet_vif->vm_opmode != WIFINET_M_WDS)
       && (wnet_vif->vm_opmode != WIFINET_M_MONITOR)) {
        if (wifi_mac_scan_connect(ss, wnet_vif)) {
            wnet_vif->vm_chan_roaming_scan_flag = 0;
            before_conn_scan_times = 0;

        } else {
            if (wnet_vif->vm_scan_before_connect_flag) {
                before_conn_scan_times++;
            }
        }
    } else if ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
                ||(wnet_vif->vm_opmode == WIFINET_M_P2P_GO)) {
        DPRINTF(AML_DEBUG_SCAN,
                "%s(%d) vm_opmode %d scan_CfgFlags 0x%x\n",
                __func__,__LINE__,wnet_vif->vm_opmode,ss->scan_CfgFlags);
        if (ss->scan_CfgFlags  & WIFINET_SCANCFG_CREATE) {
            wifi_mac_chk_ap_chan(ss, wnet_vif);
        }
    }

    /*upload scanning result and notify scanning done to upper
    after processing scan result. */
    if (ss->scan_CfgFlags & WIFINET_SCANCFG_USERREQ && !find_roaming_node) {
        wifi_mac_notify_scan_done(wnet_vif);
        ss->scan_CfgFlags &= (~WIFINET_SCANCFG_USERREQ);
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && ((wnet_vif->vm_state == WIFINET_S_CONNECTED) ||
       (wnet_vif->vm_scan_before_connect_flag))) {
        if (g_auto_gain_base != 0) {
            wifi_mac_set_channel_rssi(wifimac, g_auto_gain_base);
        }
        else {
            wifi_mac_scan_set_gain(wifimac, 174);
        }
    }

    wifi_mac_restore_wnet_vif_channel(wnet_vif);
    wifimac->wm_flags &= ~WIFINET_F_SCAN;
    ss->scan_CfgFlags = 0;
    wifimac->wm_p2p_connection_protect = 0;
    wnet_vif->vm_pstxqueue_flags &= ~WIFINET_PSQUEUE_PS4QUIET;
    wnet_vif->vm_scan_before_connect_flag = 0;
    //clear saved_ssids
    ss->ss_nssid = 0;
    memset(ss->ss_ssid,0,sizeof(ss->ss_ssid));
    if (wnet_vif->vm_chan_switch_scan_flag) {
        wifi_mac_check_switch_chan_result(wnet_vif);
        if (wnet_vif->vm_mainsta && wnet_vif->vm_mainsta->sta_channel_switch_mode) {
            wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
                                WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
            wnet_vif->vm_chan_switch_scan_count++;
        }
    }

    /*scan in roaming candidate channels 2 times,and full scan 2 times*/
    if (wnet_vif->vm_chan_roaming_scan_flag && !find_roaming_node) {
        wnet_vif->vm_chan_roaming_scan_count++;
        if (wnet_vif->vm_wmac->wm_scan->roaming_full_scan == 0) {
            if (wnet_vif->vm_chan_roaming_scan_count >= 2) {
                wnet_vif->vm_wmac->wm_scan->roaming_full_scan = 1;
                wnet_vif->vm_chan_roaming_scan_count = 0;
             }
            wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
                                WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
        }
        else if (wnet_vif->vm_chan_roaming_scan_count < 2) {
            wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
                                WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
        }
        else {
            wnet_vif->vm_chan_roaming_scan_flag = 0;
        }
    }

    if (before_conn_scan_times == 1) {
        wnet_vif->vm_scan_before_connect_flag = 1;
        wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
        WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);

    } else if (before_conn_scan_times == 2 ) {
        wnet_vif->vm_scan_before_connect_flag = 0;
        wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
        WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
        before_conn_scan_times = 0;

    } else {
        before_conn_scan_times = 0;
    }

    pr_debug("%s---> scan finish, vid:%d, clean vm_flags 0x%x\n", __func__, wnet_vif->wnet_vif_id, wifimac->wm_flags);
    os_timer_ex_start_period(&wnet_vif->vm_pwrsave.ips_timer_presleep, wnet_vif->vm_pwrsave.ips_inactivitytime);
}

void wifi_mac_notify_ap_success(struct wlan_net_vif *wnet_vif) {
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private *drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *p2p_vmac = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
    struct wlan_net_vif *sta_vmac = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];

    DPRINTF(AML_DEBUG_SCAN, "%s vid:%d\n", __func__, wnet_vif->wnet_vif_id);

    //scan branch
    if ((wifimac->wm_nrunning > 0) && (wifimac->wm_flags & WIFINET_F_SCAN)
        && (wifimac->wm_scan->scan_StateFlags & SCANSTATE_F_NOTIFY_AP)) {
        if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            os_timer_ex_cancel(&wifimac->wm_scan->ss_scan_timer, 1);
            wifimac->wm_scan->scan_StateFlags |= SCANSTATE_F_RX_CHKING_LEAKAP_PKT;
            wifi_mac_scan_chk_leakap_hrtimer_start(wifimac);
            AML_PRINT(AML_DBG_MODULES_SCAN, "notify success and start check leakap timer\n");
        }
    }

    //p2p branch
    if ((wifimac->wm_nrunning == 1)
        && (p2p_vmac->vm_p2p->p2p_flag & P2P_WAIT_SWITCH_CHANNEL)) {
        if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            wifi_mac_ChangeChannel(wifimac, p2p_vmac->vm_p2p->work_channel, 0, p2p_vmac->wnet_vif_id, p2p_vmac->vm_opmode);
            p2p_vmac->vm_p2p->p2p_flag &= ~P2P_WAIT_SWITCH_CHANNEL;
        } else {
            //pr_debug("still have pkt in hal, wait\n");
            p2p_vmac->vm_p2p->p2p_flag &= ~P2P_WAIT_SWITCH_CHANNEL;
            p2p_vmac->vm_p2p->p2p_flag |= P2P_ALLOW_SWITCH_CHANNEL;
        }
    }

    if ((wifimac->wm_nrunning > 0)
        && (sta_vmac->vm_flags_ext2 & WIFINET_FEXT2_SWITCH_CHANNEL)) {
        if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            wifi_mac_ChangeChannel(wifimac, sta_vmac->vm_remainonchan, 0, sta_vmac->wnet_vif_id, sta_vmac->vm_opmode);
            sta_vmac->vm_flags_ext2 &= ~WIFINET_FEXT2_SWITCH_CHANNEL;
        } else {
            //pr_debug("still have pkt in hal, wait\n");
            sta_vmac->vm_flags_ext2 &= ~WIFINET_FEXT2_SWITCH_CHANNEL;
            sta_vmac->vm_flags_ext2 |= WIFINET_FEXT2_ALLOW_SWITCH_CHANNEL;
        }
    }

    //vsdb channel switch branch
    if (wifimac->wm_nrunning == 2) {
        #ifdef  CONFIG_CONCURRENT_MODE
            if (wifimac->wm_vsdb_flags & CONCURRENT_NOTIFY_AP) {
                wifimac->wm_vsdb_flags &= ~CONCURRENT_NOTIFY_AP;
                if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
                    concurrent_vsdb_do_channel_change(wifimac);
                } else {
                    //pr_debug("still have pkt in hal, wait\n");
                    wifimac->wm_vsdb_flags |= CONCURRENT_NOTIFY_AP_SUCCESS;
                }
            }
        #endif
    }
}

void wifi_mac_notify_pkt_clear(struct wifi_mac *wifimac) {
    struct drv_private *drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *p2p_vmac = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
    struct wlan_net_vif *sta_vmac = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];

    DPRINTF(AML_DEBUG_SCAN, "%s\n", __func__);

    if ((wifimac->wm_nrunning > 0) && (wifimac->wm_flags & WIFINET_F_SCAN)
        && (wifimac->wm_scan->scan_StateFlags & SCANSTATE_F_WAIT_PKT_CLEAR)) {
        if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            wifimac->wm_scan->scan_StateFlags &= ~SCANSTATE_F_WAIT_PKT_CLEAR;
            wifimac->wm_scan->scan_StateFlags |= SCANSTATE_F_TX_DONE;
            os_timer_ex_cancel(&wifimac->wm_scan->ss_scan_timer, 1);
            wifi_mac_scan_timeout_ex(wifimac->wm_scan);
        }
    }

    //p2p branch
    if ((wifimac->wm_nrunning == 1)
        && (p2p_vmac->vm_p2p->p2p_flag & P2P_ALLOW_SWITCH_CHANNEL)) {
        if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            wifi_mac_ChangeChannel(wifimac, p2p_vmac->vm_p2p->work_channel, 0, p2p_vmac->wnet_vif_id, p2p_vmac->vm_opmode);
            p2p_vmac->vm_p2p->p2p_flag &= ~P2P_ALLOW_SWITCH_CHANNEL;
        }
    }

    if ((wifimac->wm_nrunning > 0)
        && (sta_vmac->vm_flags_ext2 & WIFINET_FEXT2_ALLOW_SWITCH_CHANNEL)) {
        if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            wifi_mac_ChangeChannel(wifimac, sta_vmac->vm_remainonchan, 0, sta_vmac->wnet_vif_id, sta_vmac->vm_opmode);
            sta_vmac->vm_flags_ext2 &= ~WIFINET_FEXT2_ALLOW_SWITCH_CHANNEL;
        }
    }

    if (wifimac->wm_nrunning == 2) {
        #ifdef  CONFIG_CONCURRENT_MODE
            if ((wifimac->wm_vsdb_flags & CONCURRENT_NOTIFY_AP_SUCCESS)
                 || (wifimac->wm_vsdb_flags & CONCURRENT_AP_SWITCH_CHANNEL)) {
                if (drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
                    concurrent_vsdb_do_channel_change(wifimac);
                }
            }
        #endif
    }
}

void wifi_mac_cancel_scan(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
//    DPRINTF(AML_DEBUG_SCAN, "<%s> :  %s %d ++ vm_opmode %d\n",
//        wnet_vif->vm_ndev->name,__func__,__LINE__,wnet_vif->vm_opmode);

    if (wifimac->wm_flags & WIFINET_F_SCAN)
    {
        ss->scan_StateFlags |= SCANSTATE_F_CANCEL;
//        pr_debug("<running> %s %d wnet_vif_id = %d\n",__func__,__LINE__,wnet_vif->wnet_vif_id);
        if (ss->scan_StateFlags & SCANSTATE_F_START)
        {
            os_timer_ex_cancel(&wifimac->wm_scan->ss_scan_timer, 1);
            wifi_mac_scan_timeout_ex(wifimac->wm_scan);
        }
    }
}

int vm_is_p2p_connect_scan(struct wlan_net_vif *wnet_vif, struct cfg80211_scan_request *request)
{
    return (request->n_channels == 1 && !wnet_vif->vm_scan_before_connect_flag && wnet_vif->vm_p2p_support);
}

int vm_scan_user_set_chan(struct wlan_net_vif *wnet_vif,
    struct cfg80211_scan_request *request)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct wifi_channel *c;
    int i=0,j=0;
    static unsigned char user_chan_aware_cnt = 0;

    ss->scan_last_chan_index = 0;
    DPRINTF(AML_DEBUG_SCAN, "%s %d wm_nchans=%d request_ch %d\n",
        __func__, __LINE__, wifimac->wm_nchans,request->n_channels);

    WIFI_CHANNEL_LOCK(wifimac);
    for (j = 0; j < request->n_channels; j++) {
        for (i = 0; i < wifimac->wm_nchans; i++) {
            c = &wifimac->wm_channels[i];

            if (user_chan_aware_cnt >= 10) {
                c->chan_flags &= ~WIFINET_CHAN_AWARE;
                user_chan_aware_cnt = 0;
            }

            if (c->chan_bw == WIFINET_BWC_WIDTH20) {
                if (wnet_vif->vm_p2p_support && wnet_vif->vm_p2p->social_channel) {
                    if ((c->chan_cfreq1 != SOCIAL_CHAN_1) && (c->chan_cfreq1 != SOCIAL_CHAN_2) && (c->chan_cfreq1 != SOCIAL_CHAN_3)) {
                        continue;
                    }
                }

                if (c->chan_cfreq1 != request->channels[j]->center_freq) {
                    continue;
                }

                if ((aml_iwpriv_get_band() == CFG_BAND_B) && (!WIFINET_IS_CHAN_2GHZ(c))) {
                    continue;
                } else if ((aml_iwpriv_get_band() == CFG_BAND_A) && (!WIFINET_IS_CHAN_5GHZ(c))) {
                    continue;
                }

                ss->ss_chans[ss->scan_last_chan_index++] = c;
            }
        }
    }
    WIFI_CHANNEL_UNLOCK(wifimac);

    user_chan_aware_cnt++;
    wifi_mac_set_scan_time(wnet_vif);
    DPRINTF(AML_DEBUG_SCAN, "%s vid:%d ss->scan_next_chan_index=%d \
        ss->scan_last_chan_index=%d, ss->scan_chan_wait=0x%xms, HZ = %d LINUX_VERSION_CODE =%x\n",
        __FUNCTION__, wnet_vif->wnet_vif_id, ss->scan_next_chan_index, ss->scan_last_chan_index,
        ss->scan_chan_wait, HZ, LINUX_VERSION_CODE);

    return 0;
}

static int wifi_mac_scan_before_connect(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif, int flags)
{
    /* if roaming find node to connect ,don't scan*/
    if (wnet_vif->vm_connect_scan_entry.se_valid) {
        wifi_mac_connect(wnet_vif, &wnet_vif->vm_connect_scan_entry.scaninfo);
        wnet_vif->vm_connect_scan_entry.se_valid = 0;
        return 1;
    }

    if (wifi_mac_scan_get_match_node(ss, wnet_vif) == 0) {
         pr_err("%s not found bss in former scan results\n", __func__);

    } else {
        wnet_vif->vm_scan_before_connect_flag = 1;
    }

    return 0;
}

int wifi_mac_start_scan(struct wlan_net_vif *wnet_vif, int flags,
    unsigned int nssid, const struct wifi_mac_ScanSSID ssids[])
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    if (wifimac->wm_mac_exit) {
        DPRINTF(AML_DEBUG_WARNING, "<%s> : %s drop scan due to interface down\n", wnet_vif->vm_ndev->name, __func__);
        return 0;

    } else if (wifimac->wm_flags & WIFINET_F_SCAN) {
        DPRINTF(AML_DEBUG_WARNING, "<%s> : %s drop scan due to last scan not finish\n", wnet_vif->vm_ndev->name, __func__);
        return 0;

    } else if (wnet_vif->vm_scan_hang) {
        DPRINTF(AML_DEBUG_WARNING, "<%s> : %s drop scan due to scan hang\n", wnet_vif->vm_ndev->name, __func__);
        return 0;

    } else if (wifimac->recovery_stat == WIFINET_RECOVERY_START) {
        DPRINTF(AML_DEBUG_WARNING, "<%s> : %s drop scan due to fw recovering \n", wnet_vif->vm_ndev->name, __func__);
        return 0;
    }

#ifdef CONFIG_P2P
    if ((wifimac->wm_flags & WIFINET_F_NOSCAN) && (wnet_vif->vm_p2p->p2p_enable == 0)) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d  not allow start scan beacon p2p_enable!!!!! \n", __func__,__LINE__);
        return 0;
    }
#endif//CONFIG_P2P

    wifi_mac_save_ssid(wnet_vif, ss, nssid, ssids);
    if (ss->scan_CfgFlags & WIFINET_SCANCFG_CONNECT) {
        if ((wnet_vif->vm_opmode == WIFINET_M_STA) && time_before(jiffies, wifimac->wm_lastscan + SCAN_VALID_DEFAULT)) {
            if (wifi_mac_scan_before_connect(ss, wnet_vif, flags)) {
                ss->scan_CfgFlags = 0;
                return 0;
            }
        }
    }

    wifimac->wm_flags |= WIFINET_F_SCAN;

    if (wnet_vif->vm_opmode == WIFINET_M_STA) {
        pr_debug("%s vm_scanchan_rssi:%d \n", __func__, wnet_vif->vm_scanchan_rssi);
        if (wnet_vif->vm_scan_before_connect_flag) {
            if (wnet_vif->vm_scanchan_rssi > MAC_MIN_GAIN) {
                wnet_vif->vm_scanchan_rssi = MAC_MIN_GAIN;
            }

        } else {
            wifi_mac_get_channel_rssi_before_scan(wifimac, &wnet_vif->vm_scanchan_rssi);
        }

        wifi_mac_scan_set_gain(wifimac, wnet_vif->vm_scanchan_rssi);
    }

    ss->scan_CfgFlags |= (flags & WIFINET_SCANCFG_MASK);
    wifi_mac_pwrsave_wakeup_for_tx(wnet_vif);

    //if opmode is AP or monitor ,we not need connect bss at scan end
    if ((wnet_vif->vm_opmode != WIFINET_M_IBSS) && (wnet_vif->vm_opmode != WIFINET_M_STA)) {
        ss->scan_CfgFlags |= WIFINET_SCANCFG_NOPICK;
    }

    if (!wnet_vif->vm_scan_before_connect_flag && ((ss->VMacPriv != wnet_vif) || (ss->scan_CfgFlags & WIFINET_SCANCFG_FLUSH))) {
        wifi_mac_scan_flush(wifimac);
    }

    ss->VMacPriv = wnet_vif;

    pr_debug("%s vid:%d---> scan start, CfgFlags is:%08x, ss->ss_nssid:%d\n", __func__, wnet_vif->wnet_vif_id, ss->scan_CfgFlags, ss->ss_nssid);
    wifimac->wm_scanplayercnt++;
    wifi_mac_add_work_task(wifimac, scan_start_task, NULL, (SYS_TYPE)ss, 0, (SYS_TYPE)wifimac->wm_scanplayercnt,
        (SYS_TYPE)wnet_vif, (SYS_TYPE)wnet_vif->wnet_vif_replaycounter);

    os_timer_ex_start_period(&ss->ss_scan_timer, WIFINET_SCAN_PROTECT_TIME);
    return 1;
}

int wifi_mac_chk_scan(struct wlan_net_vif *wnet_vif, int flags,
    unsigned int nssid, const struct wifi_mac_ScanSSID ssids[])
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    if (wifimac->wm_flags & WIFINET_F_SCAN) {
        AML_OUTPUT("vid:%d, wm_flags:0x%08x, scan_CfgFlags:0x%08x, ss_flag:0x%08x, next_chn:%d, last_chn:%d\n",
                   wnet_vif->wnet_vif_id, wifimac->wm_flags, ss->scan_CfgFlags, ss->scan_StateFlags,
                   ss->scan_next_chan_index, ss->scan_last_chan_index);
        return 0;
    }

    ss->scan_CfgFlags |= WIFINET_SCANCFG_CONNECT;
    pr_debug("%s flags:%08x\n", __func__, flags);
    return wifi_mac_start_scan(wnet_vif, flags,  nssid, ssids);
}

void wifi_mac_scan_vattach(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    wnet_vif->vm_scan_time_idle = WIFINET_SCAN_TIME_IDLE_DEFAULT;
    wnet_vif->vm_scan_time_connect = WIFINET_SCAN_TIME_CONNECT_DEFAULT;
    wnet_vif->vm_scan_time_before_connect = WIFINET_SCAN_TIME_BEFORE_CONNECT;
    wnet_vif->vm_scan_time_chan_switch = WIFINET_SCAN_TIME_CHANNEL_SWITCH;
    ss->VMacPriv = wnet_vif;
    vm_scan_setup_chan(ss,wnet_vif);
}

void wifi_mac_scan_vdetach(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    if (ss->VMacPriv == wnet_vif)
    {
        if (wifimac->wm_flags & WIFINET_F_SCAN)
        {
            os_timer_ex_cancel(&ss->ss_scan_timer, CANCEL_SLEEP);
            wifimac->wm_flags &= ~WIFINET_F_SCAN;
            pr_debug("%s(%d):-->clean vm_flags 0x%x\n", __func__, __LINE__, wifimac->wm_flags);
        }
        wifi_mac_scan_flush(wifimac);
    }
}

void wifi_mac_scan_attach(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss;
    struct scaninfo_table *st;
    int i=0;

    wifimac->wm_roaming = WIFINET_ROAMING_BASIC;
    wifimac->roaming_threshold_2g = DEFAULT_ROAMING_THRESHOLD_2G;
    wifimac->roaming_threshold_5g = DEFAULT_ROAMING_THRESHOLD_5G;

    pr_debug("wifi_mac_scan_timeout is %p\n", wifi_mac_scan_timeout);
    ss = (struct wifi_mac_scan_state *)NET_MALLOC(sizeof(struct wifi_mac_scan_state),
        GFP_KERNEL, "wifi_mac_scan_attach.ss");
    if (ss != NULL)
    {
        wifimac->wm_scan = ss;
        INIT_WORK(&ss->timeout_work, scan_timeout_work);
    }
    else
    {
        DPRINTF(AML_DEBUG_ERROR, "<ERROR> %s %d\n",__func__,__LINE__);
        wifimac->wm_scan = NULL;
        return ;
    }
    st = (struct scaninfo_table *)NET_MALLOC(sizeof(struct scaninfo_table),
        GFP_KERNEL, "sta_attach.st");

    if (st == NULL)
    {
        FREE(wifimac->wm_scan, "wifi_mac_scan_attach.ss");
        wifimac->wm_scan = NULL;
        DPRINTF(AML_DEBUG_SCAN, "<ERROR> %s %d\n",__func__,__LINE__);
        return ;
    }
    spin_lock_init(&st->st_lock);
    spin_lock_init(&ss->scan_lock);
    spin_lock_init(&ss->roaming_chan_lock);

    INIT_LIST_HEAD(&st->st_entry);
    for(i = 0; i < STA_HASHSIZE; i++)
    {
        INIT_LIST_HEAD(&st->st_hash[i]);
    }
    os_timer_ex_initialize(&ss->ss_scan_timer, 0, wifi_mac_scan_timeout_ex, ss);
#ifdef FW_RF_CALIBRATION
    os_timer_ex_initialize(&ss->ss_probe_timer, 0, wifi_mac_scan_send_probe_timeout_ex, ss);
#endif
    os_timer_ex_initialize(&ss->ss_scan_abort_timer, 0, wifi_mac_scan_abort_ex, ss);
    wifi_mac_scan_chk_leakap_hrtimer_attach(wifimac);

    wifimac->wm_scan->ScanTablePriv = st;
}

void wifi_mac_scan_detach(struct wifi_mac *wifimac)
{
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    if (ss != NULL)
    {
        DPRINTF(AML_DEBUG_INIT, "<running> %s %d \n",__func__,__LINE__);
        os_timer_ex_del(&ss->ss_scan_timer, CANCEL_SLEEP);
        os_timer_ex_del(&ss->ss_scan_abort_timer, CANCEL_SLEEP);
        wifi_mac_scan_chk_leakap_hrtimer_cancel(wifimac);

#ifdef FW_RF_CALIBRATION
        os_timer_ex_del(&ss->ss_probe_timer, CANCEL_SLEEP);
#endif

        if (ss->ScanTablePriv != NULL)
        {
            wifi_mac_scan_flush(wifimac);
            FREE(ss->ScanTablePriv,"sta_attach.st");
        }

        wifimac->wm_flags &= ~WIFINET_F_SCAN;
        pr_debug("%s(%d):-->clean vm_flags 0x%x\n", __func__, __LINE__, wifimac->wm_flags);

        FREE(wifimac->wm_scan,"wifi_mac_scan_attach.ss");
        wifimac->wm_scan = NULL;
    }
}

static struct cfg80211_scan_request *request = NULL;
void wifi_mac_process_tx_error(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    int flag = WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_NOPICK
        | WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_FLUSH;
    int cnt = 0;

    AML_OUTPUT("process tx error\n");
    // make sure scan cmd download to fw and reset phy/mac
    if (wifimac->wm_flags & WIFINET_F_SCAN) {
        OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
        if (pwdev_priv->scan_request != NULL) {
            request = pwdev_priv->scan_request;
            pwdev_priv->scan_request = NULL;
        }
        OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);

        wifi_mac_cancel_scan(wifimac);
        /* waiting for completing scan process */
        while (wifimac->wm_flags & WIFINET_F_SCAN) {
            msleep(20);
            if (cnt++ > 20) {
                ERROR_DEBUG_OUT("<%s>:wait scan end fail when process tx error \n", wnet_vif->vm_ndev->name);
                if (request != NULL) {
                    OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
                    pwdev_priv->scan_request = request;
                    request = NULL;
                    OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);
                }
                return;
            }
        }

        if (request != NULL) {
            OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
            pwdev_priv->scan_request = request;
            request = NULL;
            OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);
        }

        wifi_mac_start_scan(wnet_vif, flag, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
    } else {
        AML_OUTPUT("simulate scan start\n");
        wifi_mac_scan_start(wifimac);
        msleep(5);
        wifi_mac_scan_end(wifimac);
        AML_OUTPUT("simulate scan end\n");
    }
}

struct wifi_channel*
wifi_mac_connect_get_target_chan(struct wifi_mac_scan_state *ss, struct wlan_net_vif *wnet_vif)
{
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se = NULL;
    struct scaninfo_entry *se_next = NULL;
    struct wifi_channel* target_chan = NULL;

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se, se_next, &st->st_entry, se_list)
    {
        DPRINTF(AML_DEBUG_CONNECT, "se:0x%p, st_entry:0x%p\n", se, &st->st_entry);

        if (WIFINET_ADDR_EQ(wnet_vif->vm_des_bssid, se->scaninfo.SI_bssid)
            && match_ssid(se->scaninfo.SI_ssid, 1, wnet_vif->vm_des_ssid))
        {
            target_chan = se->scaninfo.SI_chan;

            if (target_chan != NULL) {
                AML_OUTPUT("pri_chan_num:%d, center_chan_num:%d, bw:%d",target_chan->chan_pri_num,
                            wifi_mac_Mhz2ieee(target_chan->chan_cfreq1, 0), target_chan->chan_bw);
                WIFI_SCAN_SE_LIST_UNLOCK(st);
                return target_chan;
            }
        }
    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);

    return target_chan;
}
