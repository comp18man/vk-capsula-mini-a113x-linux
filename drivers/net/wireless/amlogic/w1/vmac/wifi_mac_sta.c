/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer station node control module
 *
 *
 ****************************************************************************************
 */

#include "wifi_mac_com.h"
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include "wifi_mac_rate.h"

void wifi_mac_station_init(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_Rsnparms *rsn;

    DPRINTF(AML_DEBUG_WARNING, "%s %d\n",__func__,__LINE__);
    wifi_mac_rst_bss(wnet_vif);

    rsn = &wnet_vif->vm_mainsta->sta_rsn;
    rsn->rsn_ucastcipherset |= (1 << WIFINET_CIPHER_WEP);
    rsn->rsn_ucastcipherset |= (1 << WIFINET_CIPHER_TKIP);
    rsn->rsn_ucastcipherset |= (1 << WIFINET_CIPHER_AES_CCM);

    if (wifimac->wm_caps & WIFINET_C_AES)
        rsn->rsn_ucastcipherset |= (1 << WIFINET_CIPHER_AES_OCB);
    if (wifimac->wm_caps & WIFINET_C_WAPI)
        rsn->rsn_ucastcipherset |= (1 << WIFINET_CIPHER_WPI);

    rsn->rsn_ucastcipher = WIFINET_CIPHER_WEP;
    rsn->rsn_ucastkeylen = 104 / NBBY;

    rsn->rsn_mcastcipher = WIFINET_CIPHER_TKIP;
    rsn->rsn_mcastkeylen = 128 / NBBY;

    rsn->rsn_keymgmtset = (1 << WPA_ASE_8021X_UNSPEC);
    rsn->rsn_keymgmtset |= (1 << WPA_ASE_8021X_PSK);
}

void
wifi_mac_sta_auth(struct wifi_station *sta)
{
    sta->sta_flags |= WIFINET_NODE_AUTH;
    sta->sta_inact_reload = sta->sta_wnet_vif->vm_inact_run;
}

void
wifi_mac_StationUnauthorize(struct wifi_station *sta)
{
    sta->sta_flags &= ~WIFINET_NODE_AUTH;
}

static __inline void
copy_bss(struct wifi_station *nbss, const struct wifi_station *obss)
{
    nbss->sta_authmode = obss->sta_authmode;
    nbss->sta_flags_ext = obss->sta_flags_ext;
    nbss->sta_txpower = obss->sta_txpower;
    nbss->sta_vlan = obss->sta_vlan;
    nbss->sta_rsn = obss->sta_rsn;
    nbss->sta_rates = obss->sta_rates;
    nbss->sta_htrates = obss->sta_htrates;
    nbss->sta_vhtrates = obss->sta_vhtrates;
}

static __inline int
ssid_equal(const struct wifi_station *a, const struct wifi_station *b)
{
    return (a->sta_esslen == b->sta_esslen &&
            memcmp(a->sta_essid, b->sta_essid, a->sta_esslen) == 0);
}


static int
wifi_mac_start_bss_ex(unsigned long arg)
{
    struct wifi_station *selbs = (struct wifi_station *)arg;
    struct wlan_net_vif *wnet_vif = selbs->sta_wnet_vif;
    struct wifi_mac *wifimac = selbs->sta_wmac;
    struct net_device *dev = wnet_vif->vm_ndev;
    struct wifi_station *obss;
    int canreassoc, cnt = 0;
    struct hal_private * hal_priv = hal_get_priv();
    struct wlan_net_vif *p2p_vmac = wifimac->drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];

    WIFINET_VMACS_LOCK(wifimac);

    if (!(dev->flags & IFF_RUNNING)) {
        ERROR_DEBUG_OUT("wifi mac stop ,just return! \n");
        WIFINET_VMACS_UNLOCK(wifimac);
        return 1;
    }

    DPRINTF(AML_DEBUG_WARNING, "%s ori:%p, add main_sta:%p\n",__func__, wnet_vif->vm_mainsta, selbs);
    obss = wnet_vif->vm_mainsta;
    canreassoc = ((obss != NULL) && (wnet_vif->vm_state == WIFINET_S_CONNECTED) && ssid_equal(obss, selbs));

    wnet_vif->vm_mainsta = selbs;
    if ((obss != NULL) && (wnet_vif->vm_mainsta != obss)) {
        DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d obss = %p,wnet_vif->vm_state=%d ssid_equal(obss, selbs))=%d\n",
            __func__,__LINE__,obss, wnet_vif->vm_state, ssid_equal(obss, selbs));
        DPRINTF(AML_DEBUG_CONNECT,"obss sta_essid=%s,sta_esslen=%d,  selbs sta_essid=%s, sta_esslen=%d\n",
            (char*)obss->sta_essid, obss->sta_esslen,(char*) selbs->sta_essid, selbs->sta_esslen);

        if (WIFINET_M_IBSS == wnet_vif->vm_opmode) {
            copy_bss(wnet_vif->vm_mainsta, obss);
        }

        if (WIFINET_M_STA == wnet_vif->vm_opmode) {
            memcpy(wnet_vif->vm_mainsta->sta_txseqs, obss->sta_txseqs, sizeof(obss->sta_txseqs));
        }

        wifi_mac_rm_sta_from_wds_by_sta(&wnet_vif->vm_sta_tbl, obss);

        pr_debug("%s, obss:%p\n", __func__, obss);
        WIFINET_VMACS_UNLOCK(wifimac);
        wifi_mac_free_sta_from_list(obss);
        WIFINET_VMACS_LOCK(wifimac);
    }

     WIFINET_VMACS_UNLOCK(wifimac);

#ifndef FW_RF_CALIBRATION
    rf_calibration_before_connect(wifi_mac_Mhz2ieee(wnet_vif->vm_curchan->chan_cfreq1, 0),
        wnet_vif->vm_curchan->chan_bw, wnet_vif->wnet_vif_id);
#endif
    if (wnet_vif->vm_curchan != WIFINET_CHAN_ERR) {
        while (hal_priv->bhalPowerSave != 0 && cnt < 50) {
            msleep(20);
            cnt++;
        }
        DPRINTF(AML_DEBUG_CONNECT,"%s bhalPowerSave %d, cnt %d\n",__func__, hal_priv->bhalPowerSave, cnt);

        if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())
            && concurrent_check_vmac_is_AP(wifimac)
            && (wnet_vif->vm_opmode == WIFINET_M_STA)
            && (p2p_vmac->vm_state == WIFINET_S_CONNECTED)
            && concurrent_check_is_vmac_same_pri_channel(wifimac)) {
            wifi_mac_ChangeChannel(wifimac, wnet_vif->vm_curchan, 3, wnet_vif->wnet_vif_id, p2p_vmac->vm_opmode);
        } else {
            wifi_mac_ChangeChannel(wifimac, wnet_vif->vm_curchan, 3, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);
        }

        is_connect_need_set_gain(wnet_vif);
        if (wnet_vif->vm_opmode == WIFINET_M_STA) {
            wifi_mac_set_channel_rssi(wifimac, (unsigned char)(wnet_vif->vm_mainsta->sta_avg_bcn_rssi));
        }

    } else {
        return 1;
    }

#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_GO) {
        wnet_vif->vm_mac_mode =  p2p_phy_mode_filter(wnet_vif->vm_mac_mode);
    }
#endif

    //wifi_mac_reset_erp(wifimac, wifimac->wm_bssmode);
    wifi_mac_reset_ht(wifimac);
    wifi_mac_reset_vht(wifimac);// T.B.D this func, zqh
    wifi_mac_wme_initparams(wnet_vif);

    if (wnet_vif->vm_opmode == WIFINET_M_STA) {
        // when the first time, vm_state != connected, ==> canreassoc = 0
        if (canreassoc) {
            DPRINTF(AML_DEBUG_WARNING, "<running> %s %d \n",__func__,__LINE__);
            if (wnet_vif->vm_state == WIFINET_S_SCAN) {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);
            }

        } else {
            DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d wnet_vif->vm_state  %d can reassoc %d\n",
                    __func__,__LINE__,wnet_vif->vm_state,canreassoc );
            /*just change connecting to auth and send authentication , NOT deauth when the first time*/
            if (wnet_vif->vm_state == WIFINET_S_SCAN) {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_AUTH, WIFINET_FC0_SUBTYPE_DEAUTH);
            }
        }

    } else {
        DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d \n",__func__,__LINE__);
        wifi_mac_top_sm(wnet_vif, WIFINET_S_CONNECTED, -1);
    }

    return 1;
}

static void wifi_mac_start_bss_ex_task(SYS_TYPE param1,
                                  SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_station *selbs= (struct wifi_station *)param1;
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;
    if(wnet_vif->wnet_vif_replaycounter != (int)param5)
        return;

    wifi_mac_start_bss_ex((unsigned long)selbs);

    return;
}

static int wifi_mac_start_bss (struct wifi_station *selbs)
{
    struct wlan_net_vif *wnet_vif = selbs->sta_wnet_vif;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    wifi_mac_add_work_task(wifimac, wifi_mac_start_bss_ex_task,
        NULL, (SYS_TYPE)selbs,0,0, (SYS_TYPE)wnet_vif, (SYS_TYPE)wnet_vif->wnet_vif_replaycounter);
    return 1;
}

int wifi_mac_is_ht_forbidden(struct wifi_scan_info *se) {
    struct wifi_mac_ie_rsn *ap_rsn_ie = NULL;
    struct wifi_mac_ie_wpa *ap_wpa_ie = NULL;
    unsigned char is_forbidden = 1;
    unsigned char count = 0;
    unsigned char i;
    unsigned char *buf;
    unsigned char cipher = 0;

    if (se->SI_wpa_ie[1] != 0) {
        ap_wpa_ie = (struct wifi_mac_ie_wpa *)(se->SI_wpa_ie);
    }
    if (se->SI_rsn_ie[1] != 0) {
        ap_rsn_ie = (struct wifi_mac_ie_rsn *)(se->SI_rsn_ie);
    }

    if (ap_wpa_ie != NULL) {
        count = ap_wpa_ie->wpa_uciphercnt;
        buf = (unsigned char*)(ap_wpa_ie->wpa_uciphers);
        for (i = 0; i < count; i++) {
            cipher = wpa_cipher((unsigned char *)buf, 0);
            buf += 4;

            //if ap have stronger cipher, then the final cipher depends on sta
            if (cipher > WIFINET_CIPHER_TKIP) {
                is_forbidden = 0;
            }
        }
        //pr_debug("chris wpa count:%d, id:%02x, len:%02x, version:%04x\n", count, ap_wpa_ie->wpa_id, ap_wpa_ie->wpa_len, ap_wpa_ie->wpa_version);
    }

    if (ap_rsn_ie != NULL) {
        count = ap_rsn_ie->rsn_pciphercnt;
        buf = (unsigned char*)(ap_rsn_ie->rsn_pciphers);
        for (i = 0; i < count; i++) {
            cipher = rsn_cipher((unsigned char *)buf, 0);
            buf += 4;

            if (cipher > WIFINET_CIPHER_TKIP) {
                is_forbidden = 0;
            }
        }
        //pr_debug("chris rsn count:%d, id:%02x, len:%02x, version:%02x\n", count, ap_rsn_ie->rsn_id, ap_rsn_ie->rsn_len, ap_rsn_ie->rsn_version);
    }

    if ((ap_wpa_ie == NULL) && (ap_rsn_ie == NULL)) {
        if (!((se->SI_capinfo) & 0x10)) {
            is_forbidden = 0;
        }
    }

    DPRINTF(AML_DEBUG_CONNECT, "wifi_mac_is_ht_forbidden:%d, remote ap doesn't support ht due to TKIP/WEP! se->SI_capinfo:%08x, se->SI_wpa_ie:%d, se->SI_rsn_ie:%d\n",
		is_forbidden, se->SI_capinfo, se->SI_wpa_ie[1], se->SI_rsn_ie[1]);
    return is_forbidden;
}

int wifi_mac_connect(struct wlan_net_vif *wnet_vif, struct wifi_scan_info *se)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta = NULL;
    struct wifi_channel *work_channel = NULL;
    int index = 0;

    /*sta side allocate a sta buffer to save ap capabilities and information */
    sta = wifi_mac_get_new_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, se->SI_macaddr);
    if (sta == NULL) {
        ERROR_DEBUG_OUT("ERROR:: alloc sta FAIL!!! \n");
        return 0;
    }

    AML_OUTPUT("%s sta:%p\n",__func__, sta);
#if 0
    if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())) {
        struct drv_private *drv_priv = wifimac->drv_priv;
        struct wlan_net_vif *p2p_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        if (p2p_wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            /*debug code, after ap sta concurrent stable, delete this*/
            DPRINTF(AML_DEBUG_INFO, "%s (%d) sta band %d, pri chan %d, cfreq1 %d(%d)\n",__func__,__LINE__,se->SI_chan->chan_bw,
                se->SI_chan->chan_pri_num,wifi_mac_Mhz2ieee(se->SI_chan->chan_cfreq1,0),se->SI_chan->chan_cfreq1);

            p2p_wnet_vif->vm_wmac->wm_flags |= WIFINET_F_DOTH;
            p2p_wnet_vif->vm_wmac->wm_flags |= WIFINET_F_CHANSWITCH;
            p2p_wnet_vif->vm_wmac->wm_doth_channel = se->SI_chan->chan_pri_num;
            p2p_wnet_vif->vm_wmac->wm_doth_tbtt = 10;
        }
    }
#endif
    wifi_mac_connect_start(wifimac);

    if(aml_wifi_get_platform_verid() == 2) {
        /*this is for gva_mrt version, fix 2.4G on 20M bandwidth*/
        if (WIFINET_IS_CHAN_2GHZ(se->SI_chan)) {
                pr_debug("%s %d set bw 20M\n", __func__, __LINE__);
                work_channel = wifi_mac_find_chan(wifimac, se->SI_chan->chan_pri_num,
                    WIFINET_BWC_WIDTH20, se->SI_chan->chan_pri_num);
        } else {
            work_channel = wifi_mac_find_chan(wifimac, se->SI_chan->chan_pri_num,
                se->SI_chan->chan_bw, wifi_mac_Mhz2ieee(se->SI_chan->chan_cfreq1, 0));
        }

    } else {
        work_channel = wifi_mac_find_chan(wifimac, se->SI_chan->chan_pri_num,
            se->SI_chan->chan_bw, wifi_mac_Mhz2ieee(se->SI_chan->chan_cfreq1, 0));
    }

    if (work_channel == NULL) {
        ERROR_DEBUG_OUT("WARNING<%s>:: can't support set this channel\n",(wnet_vif)->vm_ndev->name);
        wifi_mac_connect_end(wifimac);
        return false;
    }
    wnet_vif->vm_curchan = work_channel;

    /*then,  sta != vm_mainsta */
    if(aml_wifi_get_platform_verid() == 2) {
        /*this is for gva_mrt version, fix 2.4G on 20M bandwidth*/
        if (WIFINET_IS_CHAN_2GHZ(se->SI_chan)) {
            pr_debug("%s %d set bw 20M\n", __func__, __LINE__);
            sta->sta_chbw = WIFINET_BWC_WIDTH20;
        } else {
            sta->sta_chbw = se->SI_chan->chan_bw;
        }

    } else {
        sta->sta_chbw = se->SI_chan->chan_bw;
    }
    sta->sta_flags = wnet_vif->vm_mainsta->sta_flags;
    sta->sta_authmode = wnet_vif->vm_mainsta->sta_authmode;
    WIFINET_ADDR_COPY(sta->sta_bssid, se->SI_bssid);
    WIFINET_ADDR_COPY(wnet_vif->vm_des_bssid, se->SI_bssid);
    sta->sta_esslen = se->SI_ssid[1];
    memcpy(sta->sta_essid, se->SI_ssid+2, sta->sta_esslen);
    sta->sta_rstamp = jiffies;
    sta->sta_tstamp.tsf = se->SI_tstamp.tsf;
    sta->sta_capinfo = se->SI_capinfo;
    sta->sta_rsn = wnet_vif->vm_mainsta->sta_rsn;
    sta->sta_listen_intval = DEFAULT_LISTEN_INTERVAL;
    sta->sta_avg_bcn_rssi = translate_to_dbm(se->SI_rssi);

#ifdef AML_WPA3
    if (wnet_vif->vm_mainsta->sta_flags_ext & WIFINET_NODE_MFP) {
        pr_debug("ap pmf:%04x, before set rsn_caps:%04x ", se->si_rsn_capa, wnet_vif->vm_opt_ie[sta->sta_rsn.rsn_caps_offset]);
        wnet_vif->vm_opt_ie[sta->sta_rsn.rsn_caps_offset] |= (unsigned char)(se->si_rsn_capa & 0xc0);

        pr_debug("after set rsn_caps:%04x\n", wnet_vif->vm_opt_ie[sta->sta_rsn.rsn_caps_offset]);
        if ((se->si_rsn_capa & 0xc0) != 0) {
            sta->sta_flags_ext = (wnet_vif->vm_mainsta->sta_flags_ext & WIFINET_NODE_MFP);
            if (sta->sta_flags_ext & WIFINET_NODE_MFP) {
                wifi_mac_disable_hw_mgmt_decrypt();
            }
        }
    }
#endif

    if (se->ie_vht_opt_md_ntf[1]) {
        sta->sta_opt_mode = se->ie_vht_opt_md_ntf[2];
    }
    DPRINTF(AML_DEBUG_CONNECT,"<%s> %s  chan_pri_num %d,  chan_flags=0x%x, chan_cfreq1=%d\n",
        VMAC_DEV_NAME(wnet_vif),__func__, se->SI_chan->chan_pri_num,  se->SI_chan->chan_flags,  se->SI_chan->chan_cfreq1);

    sta->sta_timoff = se->SI_timoff;
    sta->sta_erp = se->SI_erp;

    sta->sta_bssmode = wifi_mac_get_sta_mode((struct wifi_scan_info *)se);
    if (sta->sta_bssmode <= WIFINET_MODE_11BG) {
        //special situation: use wep/tkip cipher, but with HT IE, and in 40Mhz(wifi31)
        sta->sta_chbw = WIFINET_BWC_WIDTH20;
    }

    DPRINTF(AML_DEBUG_CONNECT,"<%s> %s %p sta_flags %d \n",
            VMAC_DEV_NAME(wnet_vif),__func__,wnet_vif->vm_mainsta,wnet_vif->vm_mainsta->sta_flags);

#ifdef CONFIG_WAPI
    if ((se->SI_wai_ie[1]!=0))
        wifi_mac_saveie(&sta->sta_wai_ie, se->SI_wai_ie, "sta->sta_wai_ie");
#endif //#ifdef CONFIG_WAPI

#ifdef CONFIG_P2P
    for (index = 0; index < MAX_P2PIE_NUM; index++)
    {
        if ((se->SI_p2p_ie[index][1] != 0))
            wifi_mac_saveie(&sta->sta_p2p_ie[index], se->SI_p2p_ie[index], "sta->sta_p2p_ie");
    }
#endif //#ifdef CONFIG_P2P

#ifdef CONFIG_WFD
    if ((se->SI_wfd_ie[1]!=0))
        wifi_mac_saveie(&sta->sta_wfd_ie, se->SI_wfd_ie, "sta->sta_wfd_ie");
#endif //#ifdef CONFIG_WFD

    if ((se->SI_wme_ie[1]!=0))
    {
        wifi_mac_saveie(&sta->sta_wme_ie, se->SI_wme_ie, "sta->sta_wme_ie");
    }

    if (se->SI_wpa_ie[1]!=0)
    {
        wifi_mac_saveie(&sta->sta_wpa_ie, se->SI_wpa_ie, "sta->sta_wpa_ie");
    }
    if ((se->SI_rsn_ie[1]!=0))
    {
        wifi_mac_saveie(&sta->sta_rsn_ie, se->SI_rsn_ie, "sta->sta_rsn_ie");
    }
    if ((se->SI_wps_ie[1]!=0))
    {
        wifi_mac_saveie(&sta->sta_wps_ie, se->SI_wps_ie, "sta->sta_wps_ie");
    }

    wifi_mac_reset_vmac(wnet_vif);
    wnet_vif->vm_bcn_intval = se->SI_intval;
    wnet_vif->vm_mac_mode = sta->sta_bssmode;

    wifi_mac_sta_set_basic_rates(wnet_vif,se->SI_rates, se->SI_exrates);
    wifi_mac_set_legacy_rates(&wnet_vif->vm_legacy_rates, wnet_vif);
    wifi_mac_setup_rates(sta, se->SI_rates, se->SI_exrates, WIFINET_F_DOXSECT);

    if ((se->SI_country_ie[1]!=0))
        wifimac->wm_11dinfo = (struct wifi_mac_country_ie *)se->SI_country_ie;
    else
        wifimac->wm_11dinfo = NULL;

    pr_debug("%s start connect to ssid:%s, pri_channel:%d, center_chan:%d, bw:%d, BSSID:%02x:%02x:%02x:%02x:%02x:%02x, mode:%d\n",
        __func__, se->SI_ssid+2, se->SI_chan->chan_pri_num, se->SI_chan->chan_cfreq1, se->SI_chan->chan_bw, sta->sta_bssid[0],
        sta->sta_bssid[1], sta->sta_bssid[2], sta->sta_bssid[3], sta->sta_bssid[4], sta->sta_bssid[5], sta->sta_bssmode);
    wnet_vif->vm_phase_flags |= PHASE_CONNECTING;
    return wifi_mac_start_bss(sta);
}

void wifi_mac_create_adhoc_bssid(struct wlan_net_vif *wnet_vif, unsigned char *sta_bssid)
{
    unsigned char i;
    unsigned long long tmp=  jiffies*0x1234567;
    for (i = 2; i < WIFINET_ADDR_LEN; i++)
    {
        sta_bssid[i] = tmp&0xff;
        tmp = tmp>>8;
    }
    sta_bssid[0] = wnet_vif->vm_myaddr[0];
    sta_bssid[1] = wnet_vif->vm_myaddr[1];
}

//create AP and adhoc
void wifi_mac_create_wifi(struct wlan_net_vif* wnet_vif, struct wifi_channel *chan)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta = NULL;
    struct wifi_channel *work_channel = NULL;

    DPRINTF(AML_DEBUG_CONNECT, "%s %d\n",__func__,__LINE__);
    sta = wifi_mac_get_new_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, wnet_vif->vm_myaddr);
    if (sta == NULL)
    {
        DPRINTF(AML_DEBUG_ERROR, "ERROR::%s %d alloc sta FAIL!!! \n",__func__,__LINE__);
        return;
    }

    work_channel = wifi_mac_find_chan(wifimac, chan->chan_pri_num,
        chan->chan_bw, wifi_mac_Mhz2ieee(chan->chan_cfreq1, 0));
    if (work_channel == NULL) {
        DPRINTF(AML_DEBUG_ERROR, "<%s>: %s can't support set this channel\n", __func__, wnet_vif->vm_ndev->name);
        return;
    }
    wnet_vif->vm_curchan = work_channel;
    sta->sta_chbw = chan->chan_bw;

    pr_debug("%s(%d): pri_chan %d, bw %d, chan_cfreq1 %d\n",  __func__, __LINE__,
        chan->chan_pri_num, chan->chan_bw, wifi_mac_Mhz2ieee(chan->chan_cfreq1, 0));

    WIFINET_ADDR_COPY(sta->sta_bssid, wnet_vif->vm_myaddr);
    WIFINET_ADDR_COPY(wnet_vif->vm_des_bssid, wnet_vif->vm_myaddr);
    sta->sta_esslen = wnet_vif->vm_des_ssid[0].len;
    memcpy(sta->sta_essid, wnet_vif->vm_des_ssid[0].ssid, sta->sta_esslen);

    if (wnet_vif->vm_mainsta) {
        copy_bss(sta, wnet_vif->vm_mainsta);
        sta->sta_vhtcap = wnet_vif->vm_mainsta->sta_vhtcap;
        sta->sta_authmode = wnet_vif->vm_mainsta->sta_authmode;
        sta->sta_rsn = wnet_vif->vm_mainsta->sta_rsn;
        sta->sta_flags_ext = (wnet_vif->vm_mainsta->sta_flags_ext & WIFINET_NODE_MFP);
    }

    if (wnet_vif->vm_flags & WIFINET_F_PRIVACY)
        sta->sta_capinfo |= WIFINET_CAPINFO_PRIVACY;

    if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
    {
        wnet_vif->vm_flags |= WIFINET_F_SIBSS;
        sta->sta_capinfo |= WIFINET_CAPINFO_IBSS;   /* XXX */
        if (wnet_vif->vm_flags & WIFINET_F_DESBSSID)
        {
            WIFINET_ADDR_COPY(sta->sta_bssid, wnet_vif->vm_des_bssid);
        }
        else
        {
            wifi_mac_create_adhoc_bssid(wnet_vif, sta->sta_bssid);
        }
    }

    wifi_mac_ap_set_basic_rates(wnet_vif, wnet_vif->vm_mac_mode);
    wifi_mac_set_legacy_rates(&wnet_vif->vm_legacy_rates, wnet_vif);
    sta->sta_rates = wnet_vif->vm_legacy_rates;
    sta->sta_htrates = wifimac->wm_sup_ht_rates;
    sta->sta_vhtrates = wifimac->wm_sup_vht_rates;

    if (WIFINET_INCLUDE_11N(wnet_vif->vm_mac_mode)) {
        sta->sta_flags |= WIFINET_NODE_HT;
    }
    wifi_mac_build_country_ie(wnet_vif);
    wifi_mac_start_bss(sta);
    pr_debug("<%s>  L:%d, mac mode %d dot11_rate_num %d\n",
            __func__,__LINE__,sta->sta_bssmode,sta->sta_htrates.dot11_rate_num);
}

static void nsta_cleanup(struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int i;
    int total_delay = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private *drv_priv = wifimac->drv_priv;
    wifi_mac_sta_clean(sta);
    if (sta->sta_flags & WIFINET_NODE_PWR_MGT)
    {
        if (wnet_vif->vm_opmode != WIFINET_M_STA)
            wnet_vif->vm_ps_sta--;
        sta->sta_flags &= ~WIFINET_NODE_PWR_MGT;
        WIFINET_DPRINTF_STA( AML_DEBUG_PWR_SAVE, sta,
                             "power save mode off, %u sta's in ps mode", wnet_vif->vm_ps_sta);

        if (sta->sta_flags & WIFINET_NODE_UAPSD_TRIG)
        {
            WIFINET_UAPSD_LOCK(sta->sta_wmac);
            sta->sta_flags &= ~WIFINET_NODE_UAPSD_TRIG;
            WIFINET_UAPSD_UNLOCK(sta->sta_wmac);
        }
    }

    sta->sta_flags &= ~WIFINET_NODE_AREF;
    os_timer_ex_del(&sta->csa_timer, CANCEL_SLEEP);

    if (wifi_mac_pwrsave_psqueue_clean(sta) != 0 && wnet_vif->vif_ops.vm_set_tim != NULL)
        wnet_vif->vif_ops.vm_set_tim(sta, 0);

    pr_debug("----------%d assoc id \n",sta->sta_associd);
    if (sta->sta_challenge != NULL)
    {
        FREE(sta->sta_challenge,"sta->sta_challenge");
        sta->sta_challenge = NULL;
    }

    for (i = 0; i < ARRAY_LENGTH(sta->sta_rxfrag); i++)
    {
        if (sta->sta_rxfrag[i] != NULL)
        {
            kfree_skb(sta->sta_rxfrag[i]);
            sta->sta_rxfrag[i] = NULL;
        }
    }

    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP && sta->sta_associd == 0)
        return ;
    if (!(sta->sta_ucastkey.wk_flags & WIFINET_KEY_NOFREE))
    {
        while (total_delay < 1000 && !drv_priv->hal_priv->hal_ops.hal_tx_empty()) {
            mdelay(10);
            total_delay += 10;
        }
        DPRINTF(AML_DEBUG_WARNING, "<running>disconnect %d %s sta:%p total_delay %d empty %d\n",sta->is_disconnecting, __func__, sta,
        total_delay,drv_priv->hal_priv->hal_ops.hal_tx_empty());
        wifi_mac_sec_delt_key(wnet_vif, &sta->sta_ucastkey, sta);
        for (i = 0; i < WIFINET_WEP_NKID; i++)
            wifi_mac_security_resetkey(wnet_vif, &wnet_vif->vm_nw_keys[i], WIFINET_KEYIX_NONE);
    }
}


void wifi_mac_sta_leave(struct wifi_station *sta, int reassoc)
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;
    unsigned short sta_associd = 0;

    if (sta == NULL)
        return;

    wnet_vif = sta->sta_wnet_vif;
    wifimac = wnet_vif->vm_wmac;
    wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_NONE;
    wnet_vif->vm_change_rate_enable = 1;
    wnet_vif->vm_scanchan_rssi = MAC_MIN_GAIN;

    pr_debug("wifi_mac_sta_leave:%d, sta:%p, main_sta:%p\n", wnet_vif->wnet_vif_id, sta, wnet_vif->vm_mainsta);
    if (wnet_vif->vm_opmode == WIFINET_M_STA)
    {
        wifimac->drv_priv->drv_ops.drv_set_pkt_drop(wifimac->drv_priv, wnet_vif->wnet_vif_id, 1);
        wnet_vif->vm_curchan = WIFINET_CHAN_ERR;
        if (wnet_vif->vm_flags_ext & WIFINET_FEXT_SWBMISS) {
            wifi_mac_add_work_task(wifimac,wifi_mac_set_beacon_miss,
                NULL,(SYS_TYPE)wifimac, (SYS_TYPE)wnet_vif->wnet_vif_id, DISABLE, (SYS_TYPE)wnet_vif, 0);
        }

        wifi_mac_add_work_task(wifimac,wifi_mac_sta_keep_alive_ex,
            NULL,(SYS_TYPE)wifimac, (SYS_TYPE)wnet_vif->wnet_vif_id, DISABLE, (SYS_TYPE)wnet_vif, WIFINET_KEEP_ALIVE);

        sta_associd = drv_hal_wnet_vif_staid(wnet_vif->vm_opmode,sta->sta_associd);
        if (sta_associd != 0) {
            wifimac->drv_priv->drv_ops.clear_staid_and_bssid(wifimac->drv_priv,wnet_vif->wnet_vif_id, sta_associd);
        }

        memset(&wnet_vif->vm_devstats, 0, sizeof(wnet_vif->vm_devstats));
        if (wifimac->wm_flags & WIFINET_F_SCAN) {
            wifimac->wm_scan->scan_StateFlags &= SCANSTATE_F_DISCONNECT_REQ_CANCEL;
        }
        sta->sta_opt_mode = 0;
        wnet_vif->vm_mainsta->sta_flags_ext &= ~WIFINET_NODE_MFP;
        wifi_mac_reset_tspecs(wnet_vif);
        wifi_mac_reset_vmac(wnet_vif);
        wifi_mac_rate_disassoc(sta);
        wifi_mac_buffer_txq_flush(&wnet_vif->vm_tx_buffer_queue);
        wifi_mac_buffer_txq_flush(&wifimac->drv_priv->retransmit_queue);
        wifimac->drv_priv->drv_ops.drv_free_normal_buffer_queue(wifimac->drv_priv, wnet_vif->wnet_vif_id);
        wifimac->drv_priv->drv_ops.drv_flush_txdata(wifimac->drv_priv, wnet_vif->wnet_vif_id);
        if (wifimac->recovery_stat != (WIFINET_RECOVERY_START|WIFINET_RECOVERY_UNDER_CONNECT)) {
            wifi_mac_notify_nsta_disconnect(sta, reassoc);
        }
        wifimac->drv_priv->drv_ops.drv_set_pkt_drop(wifimac->drv_priv, wnet_vif->wnet_vif_id, 0);
        wifimac->drv_priv->drv_ops.drv_set_is_mother_channel(wifimac->drv_priv, wnet_vif->wnet_vif_id, 1);
        wifi_mac_rst_bss(wnet_vif);
        wifi_mac_rst_main_sta(wnet_vif);
        wifimac->scan_noisy_status = WIFINET_S_SCAN_ENV_NOISE;
        wifimac->is_connect_set_gain = 1;
        wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, 174);

        os_timer_ex_start_period(&wifimac->wm_csa_trigger_timer, 10000);

        if (wifimac->recovery_stat == WIFINET_RECOVERY_END) {
            wnet_vif->vm_des_nssid = 0;
            memset(wnet_vif->vm_des_ssid, 0, IV_SSID_SCAN_AMOUNT*sizeof(struct wifi_mac_ScanSSID));
        }
    }
    wnet_vif->vm_phase_flags &= ~PHASE_DISCONNECTING;

    /*
    * p2p mode disconnect, need reset vm_mac_mode
    */
    if(wnet_vif->vm_p2p_support) {
        vm_p2p_initial(wnet_vif->vm_p2p);
        wnet_vif->vm_mac_mode = WIFINET_MODE_11GN;
    }
}

static void wifi_mac_StationTableInit(struct wifi_mac *wifimac,
    struct wifi_station_tbl *nt, const char *name, int inact)
{
    int Num=0;
    nt->nt_wmac = wifimac;

    WIFINET_NODE_LOCK_INIT(nt);
    INIT_LIST_HEAD(&nt->nt_nsta);

    for (Num = 0; Num < WIFINET_NODE_HASHSIZE; Num++)
    {
        INIT_LIST_HEAD(&nt->nt_wds_hash[Num]);
    }
    nt->nt_name = name;
    nt->nt_inact_init = inact;
}

static void
nsta_free(struct wifi_station *sta)
{
    int index = 0;
    struct sk_buff *skb = NULL, *tmp = NULL;
    struct wlan_net_vif *wnet_vif = NULL;
    struct sk_buff_head *txqueue = NULL;
    struct wifi_station *txqueue_sta = NULL;

    ASSERT(sta);

    if (sta) {
        wnet_vif = sta->sta_wnet_vif;

        if (wnet_vif && sta == wnet_vif->vm_mainsta) {
            pr_debug("free vm_mainsta:%p\n", sta);
            wnet_vif->vm_mainsta = NULL;
        }
    } else {
        return ;
    }

    wifi_mac_rate_disassoc(sta);

    wifi_mac_sta_free(sta);
    if (sta->sta_wpa_ie != NULL)
        FREE(sta->sta_wpa_ie,"sta->sta_wpa_ie");

    if (sta->sta_wme_ie != NULL)
        FREE(sta->sta_wme_ie,"sta->sta_wme_ie");

#ifdef CONFIG_P2P
    for (index = 0; index < MAX_P2PIE_NUM; index++)
    {
        if (sta->sta_p2p_ie[index] != NULL) {
            FREE(sta->sta_p2p_ie[index],"sta->sta_p2p_ie");
        }
    }
#endif /*CONFIG_P2P*/

#ifdef CONFIG_WFD
    if (sta->sta_wfd_ie != NULL)
        FREE(sta->sta_wfd_ie,"sta->sta_wfd_ie");
#endif /*CONFIG_WFD*/

    if (sta->sta_rsn_ie != NULL)
        FREE(sta->sta_rsn_ie,"sta->sta_rsn_ie");
    if (sta->sta_wps_ie != NULL)
        FREE(sta->sta_wps_ie,"sta->sta_wps_ie");

#ifdef CONFIG_WAPI
    if (sta->sta_wai_ie != NULL)
        FREE(sta->sta_wai_ie,"sta->sta_wai_ie");
#endif /*CONFIG_WAPI*/

    txqueue = &wnet_vif->vm_tx_buffer_queue;
    WIFINET_SAVEQ_LOCK(txqueue);
    skb_queue_walk_safe(txqueue,skb,tmp) {
        txqueue_sta = os_skb_get_nsta(skb);
        if (txqueue_sta == sta) {
            AML_OUTPUT("as free sta: %p so free skb\n",txqueue_sta);
            aml_skb_unlink(skb,txqueue);
            wifi_mac_free_skb(skb);

        }
    }
    WIFINET_SAVEQ_UNLOCK(txqueue);

    WIFINET_SAVEQ_DESTROY(&(sta->sta_pstxqueue));
    pr_debug("%s free %p\n", __func__, sta);
    FREE(sta,"alloc_sta_node.sta");
}

static struct wifi_station *
alloc_sta_node(struct wifi_station_tbl *nt,struct wlan_net_vif *wnet_vif)
{
    struct wifi_station *sta = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    ASSERT(wnet_vif);

    sta = (struct wifi_station *)NET_MALLOC(sizeof(struct wifi_station),
        GFP_ATOMIC, "alloc_sta_node.sta");

    sta->sta_wnet_vif = wnet_vif;
    sta->sta_wmac = wnet_vif->vm_wmac;
    sta->sta_avg_bcn_rssi  = -100;
    sta->sta_avg_snr = 25;
    sta->sta_avg_rssi   = 156;
    sta->sta_last_txrate = 1000; /*1M*/
    sta->sta_last_rxrate = 1000; /*1M*/
    sta->minstrel_init_flag = 0;
    sta->is_disconnecting = 0;

    sta->drv_sta = wifimac->drv_priv->drv_ops.alloc_nsta(wifimac->drv_priv, wnet_vif->wnet_vif_id, sta);

    if (sta->drv_sta == NULL) {
        nsta_free(sta);
        sta = NULL;
        return sta;
    }

    wifi_mac_alloc_amsdu_node(wifimac, wnet_vif->wnet_vif_id, sta);
    pr_debug("%s vid:%d, sta:%p\n", __func__, wnet_vif->wnet_vif_id, sta);
    return sta;
}

//if sta node is in the table, get it when addr and mac match
//if the sta node is not in the table, allocate one and append the queue, and get it
struct wifi_station *wifi_mac_get_new_sta_node(struct wifi_station_tbl *nt,
    struct wlan_net_vif *wnet_vif, const unsigned char *macaddr)
{
    struct wifi_mac *wifimac = nt->nt_wmac;
    struct wifi_station *sta = NULL;
    int hash;

    sta = alloc_sta_node(nt,wnet_vif);
    if (sta == NULL) {
        wnet_vif->vif_sts.sts_rx_sta_all_fail++;
        DPRINTF(AML_DEBUG_NODE, "WARNING:%s %d \n", __func__, __LINE__);
        return NULL;
    }
    sta->sta_tmp_nsta = 0;
    os_timer_ex_initialize(&sta->csa_timer, 0, wifi_mac_csa_handle_timeout, sta);

    WIFINET_ADDR_COPY(sta->sta_macaddr, macaddr);
    //dump_memory_internal(macaddr, 6);
    hash = WIFINET_NODE_HASH(macaddr);
    sta->sta_authmode = WIFINET_AUTH_OPEN;
    sta->sta_txpower = wifimac->wm_txpowlimit;
    wifi_mac_security_resetkey(wnet_vif, &sta->sta_ucastkey, WIFINET_KEYIX_NONE);
    wifi_mac_security_resetkey(wnet_vif, &sta->pmf_key, WIFINET_KEYIX_NONE);
    sta->sta_inact_reload = nt->nt_inact_init;
    sta->sta_inact = sta->sta_inact_reload;
    WIFINET_SAVEQ_INIT(&(sta->sta_pstxqueue), "unknown");

    sta->sta_chbw = 0;
    DPRINTF(AML_DEBUG_BWC, "%s(%d),sta_chbw 0x%x\n", __func__, __LINE__,sta->sta_chbw );
    sta->connect_status = CONNECT_IDLE;
    sta->sta_update_rate_flag = 0;
    sta->wnet_vif_id = wnet_vif->wnet_vif_id;
    sta->sta_bssmode = wnet_vif->vm_mac_mode;

    WIFINET_NODE_LOCK(nt);
    list_add_tail(&sta->sta_list, &nt->nt_nsta);
    WIFINET_NODE_UNLOCK(nt);

    WME_UAPSD_NODE_TRIGSEQINIT(sta);

    return sta;
}

struct wifi_station *wifi_mac_get_sta_node(struct wifi_station_tbl *nt,
    struct wlan_net_vif *wnet_vif, const unsigned char *macaddr)
{
    struct wifi_station *sta = NULL;
    struct wifi_station *sta_next = NULL;

    DPRINTF(AML_DEBUG_NODE, "%s %d \n",__func__,__LINE__);

    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        if (sta != NULL) {
            DPRINTF(AML_DEBUG_NODE, "%s %d \n",__func__,__LINE__);

            if ((WIFINET_ADDR_EQ(macaddr, sta->sta_macaddr))&&(sta->wnet_vif_id == wnet_vif->wnet_vif_id)) {
                WIFINET_NODE_UNLOCK(nt);
                return sta;
            }
        }
    }
    WIFINET_NODE_UNLOCK(nt);

    sta = wifi_mac_get_new_sta_node(nt, wnet_vif, macaddr);
    if (sta == NULL) {
        wnet_vif->vif_sts.sts_rx_sta_all_fail++;
        DPRINTF(AML_DEBUG_NODE, "WARNING:%s %d \n",__func__,__LINE__);
        return NULL;
    }

    return sta;
}

int wifi_mac_add_wds_addr(struct wifi_station_tbl *nt,
    struct wifi_station *sta, const unsigned char *macaddr)
{
    int hash;
    struct wifi_mac_WdsAddr *wds;

    wds = (struct wifi_mac_WdsAddr *)NET_MALLOC(sizeof(struct wifi_mac_WdsAddr),
        GFP_KERNEL, "wifi_mac_add_wds_addr.wds");

    if (wds == NULL) {
        DPRINTF(AML_DEBUG_NODE, "WARNING:%s %d \n",__func__,__LINE__);
        return 1;
    }

    wds->wds_agingcount = WDS_AGING_COUNT;
    hash = WIFINET_NODE_HASH(macaddr);
    WIFINET_ADDR_COPY(wds->wds_macaddr, macaddr);
    wds->wds_ni = sta;
    WIFINET_NODE_LOCK(nt);
    list_add(&wds->wds_hash, &nt->nt_wds_hash[hash]);
    WIFINET_NODE_UNLOCK(nt);
    return 0;
}

int wifi_mac_rm_sta_from_wds_by_addr(struct wifi_station_tbl *nt, const unsigned char *macaddr)
{
    int hash;
    int ret = 1;
    struct wifi_mac_WdsAddr *wds;

    hash = WIFINET_NODE_HASH(macaddr);
    list_for_each_entry(wds, &nt->nt_wds_hash[hash], wds_hash) {
        if (WIFINET_ADDR_EQ(wds->wds_macaddr, macaddr)) {
            list_del_init(&wds->wds_hash);
            FREE(wds,"wds");
            ret = 0;
            break;
        }
    }

    return ret;
}

int wifi_mac_rm_sta_from_wds_by_sta(struct wifi_station_tbl *nt,struct wifi_station *sta)
{
    unsigned int hash;
    struct wifi_mac_WdsAddr *wds;

    WIFINET_NODE_LOCK(nt);
    for (hash = 0; hash < WIFINET_NODE_HASHSIZE; hash++) {
        list_for_each_entry(wds, &nt->nt_wds_hash[hash], wds_hash) {
            if (wds->wds_ni == sta) {
                list_del_init(&wds->wds_hash);
                FREE(wds,"wds");
                break;
            }
        }
    }
    WIFINET_NODE_UNLOCK(nt);

    return 0;
}

struct wifi_station *
wifi_mac_find_wds_sta(struct wifi_station_tbl *nt, const unsigned char *macaddr)
{
    struct wifi_station *sta;
    struct wifi_mac_WdsAddr *wds;
    int hash;

    hash = WIFINET_NODE_HASH(macaddr);
    WIFINET_NODE_LOCK(nt);
    list_for_each_entry(wds, &nt->nt_wds_hash[hash], wds_hash) {
        if (WIFINET_ADDR_EQ(wds->wds_macaddr, macaddr)) {
            sta = wds->wds_ni;
            wds->wds_agingcount = WDS_AGING_COUNT;
            WIFINET_NODE_UNLOCK(nt);
            return sta;
        }
    }
    WIFINET_NODE_UNLOCK(nt);

    return NULL;
}

int wifi_mac_rm_sta(struct wifi_station_tbl *nt, const unsigned char *macaddr)
{
    int hash;
    int ret = 1;
    struct wifi_mac_WdsAddr *wds;

    WIFINET_NODE_LOCK(nt);
    hash = WIFINET_NODE_HASH(macaddr);
    list_for_each_entry(wds, &nt->nt_wds_hash[hash], wds_hash) {
        if (WIFINET_ADDR_EQ(wds->wds_macaddr, macaddr)) {
            list_del_init(&wds->wds_hash);
            list_del_init(&wds->wds_ni->sta_list);
            WIFINET_NODE_UNLOCK(nt);
            wifi_mac_free_sta(wds->wds_ni);
            WIFINET_NODE_LOCK(nt);
            FREE(wds,"wds");
            ret = 0;
            break;
        }
    }
    WIFINET_NODE_UNLOCK(nt);

    return ret;
}

struct wifi_station *
wifi_mac_tmp_nsta(struct wlan_net_vif *wnet_vif, const unsigned char *macaddr)
{
    struct wifi_station *sta;

    pr_debug("%s\n", __func__);

    sta = alloc_sta_node(NULL ,wnet_vif);
    if (sta != NULL) {
        sta->sta_tmp_nsta = 1;
        WIFINET_ADDR_COPY(sta->sta_macaddr, macaddr);
        WIFINET_ADDR_COPY(sta->sta_bssid, wnet_vif->vm_mainsta->sta_bssid);
        sta->sta_txpower = wnet_vif->vm_mainsta->sta_txpower;

        DPRINTF(AML_DEBUG_NODE,"<running> %s %d \n",__func__,__LINE__);
        wifi_mac_security_resetkey(wnet_vif, &sta->sta_ucastkey, WIFINET_KEYIX_NONE);
        WIFINET_SAVEQ_INIT(&(sta->sta_pstxqueue), "unknown");

    } else {
        DPRINTF(AML_DEBUG_NODE, "WARNING:%s %d \n",__func__,__LINE__);
        wnet_vif->vif_sts.sts_rx_sta_all_fail++;
    }

    return sta;
}


struct wifi_station *wifi_mac_bup_bss(struct wlan_net_vif *wnet_vif, const unsigned char *macaddr)
{
    struct wifi_station *sta;
    DPRINTF(AML_DEBUG_CONNECT, "%s %d\n",__func__,__LINE__);

    sta = wifi_mac_get_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, macaddr);
    if (sta != NULL) {
        sta->sta_authmode = wnet_vif->vm_mainsta->sta_authmode;
        sta->sta_txpower = wnet_vif->vm_mainsta->sta_txpower;
        sta->sta_vlan = wnet_vif->vm_mainsta->sta_vlan;
        WIFINET_ADDR_COPY(sta->sta_bssid, wnet_vif->vm_mainsta->sta_bssid);

        sta->sta_rsn = wnet_vif->vm_mainsta->sta_rsn;
    }

    return sta;
}

static struct wifi_station *wifi_mac_find_sta(struct wifi_station_tbl *nt,
    const unsigned char *macaddr,int wnet_vif_id)
{
    struct wifi_station *sta = NULL;
    struct wifi_station *sta_next = NULL;
    int hash;
    struct wifi_mac_WdsAddr *wds;

    hash = WIFINET_NODE_HASH(macaddr);
    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        if((WIFINET_ADDR_EQ(sta->sta_macaddr, macaddr))&&(sta->wnet_vif_id == wnet_vif_id))  {
            WIFINET_NODE_UNLOCK(nt);
            return sta;
        }
    }

    list_for_each_entry(wds, &nt->nt_wds_hash[hash], wds_hash) {
        if (WIFINET_ADDR_EQ(wds->wds_macaddr, macaddr)) {
            sta = wds->wds_ni;
            WIFINET_NODE_UNLOCK(nt);
            return sta;
        }
    }
    WIFINET_NODE_UNLOCK(nt);

    return NULL;
}

struct wifi_station *
wifi_mac_get_sta(struct wifi_station_tbl *nt, const unsigned char *macaddr,int wnet_vif_id)
{
    struct wifi_station *sta;

    sta = wifi_mac_find_sta(nt, macaddr,wnet_vif_id);
    return sta;
}

struct wifi_station *
wifi_mac_fake_adhos_sta(struct wlan_net_vif *wnet_vif,
                        const unsigned char *macaddr)
{
    struct wifi_station *sta;
    pr_debug("<running> %s %d \n",__func__,__LINE__);
    sta = wifi_mac_bup_bss(wnet_vif, macaddr);
    if (sta != NULL)
    {
        pr_debug("<running> %s %d NI_RATE....................\n",__func__,__LINE__);
        sta->sta_rates = wnet_vif->vm_mainsta->sta_rates;
        wifi_mac_new_assoc(sta, 1);
        wifi_mac_sta_auth(sta);
    }
    return sta;
}

struct wifi_station *
wifi_mac_add_neighbor(struct wlan_net_vif *wnet_vif,
                    const struct wifi_frame *wh,
                    const struct wifi_mac_scan_param *sp)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta;

    struct wifi_mac_ie_htinfo *htinfo = (struct wifi_mac_ie_htinfo *)sp->htinfo;
    struct wifi_mac_ie_htinfo_cmn *ie = &htinfo->hi_ie;
    struct wifi_mac_ie_vht_opt *vhtop = (struct wifi_mac_ie_vht_opt *) sp->vht_opt;
    int bw = 0, center_chan = 0;
    int index = 0;

    if (ie->hi_extchoff == WIFINET_HTINFO_EXTOFFSET_ABOVE)
    {
        center_chan = sp->chan + 2;
        bw = WIFINET_BWC_WIDTH40;
    }
    else if (ie->hi_extchoff == WIFINET_HTINFO_EXTOFFSET_BELOW)
    {
        center_chan = sp->chan - 2;
        bw = WIFINET_BWC_WIDTH40;
    }
    else
    {
        center_chan = sp->chan;
        bw = WIFINET_BWC_WIDTH20;
    }

    if (vhtop){
        if (vhtop->vht_op_chwidth == VHT_OPT_CHN_WD_80M)
        {
            center_chan = vhtop->vht_op_ch_freq_seg1;
            bw = WIFINET_BWC_WIDTH80;
        }
        else if (vhtop->vht_op_chwidth > VHT_OPT_CHN_WD_80M)
        {
            pr_debug("%s:%d, not support bandwidth %d yet \n", __func__, __LINE__, vhtop->vht_op_chwidth);
        }
    }

    if (wifi_mac_set_wnet_vif_channel(wnet_vif, sp->chan, bw, center_chan, CHANNEL_CONNECT_FLAG | CHANNEL_RESTORE_FLAG) == false)
    {
        return NULL;
    }

    sta = wifi_mac_bup_bss(wnet_vif, wh->i_addr2);
    if (sta != NULL)
    {
        sta->sta_esslen = sp->ssid[1];
        memcpy(sta->sta_essid, sp->ssid + 2, sp->ssid[1]);
        WIFINET_ADDR_COPY(sta->sta_bssid, wh->i_addr3);
        memcpy(sta->sta_tstamp.data, sp->tstamp, sizeof(sta->sta_tstamp));
        sta->sta_capinfo = sp->capinfo;
        sta->sta_erp = sp->erp;
        sta->sta_timoff = sp->timoff;
        if (sp->wme != NULL)
            wifi_mac_saveie(&sta->sta_wme_ie, sp->wme, "sta->sta_wme_ie");
        if (sp->wpa != NULL)
            wifi_mac_saveie(&sta->sta_wpa_ie, sp->wpa, "sta->sta_wpa_ie");
        if (sp->rsn != NULL)
            wifi_mac_saveie(&sta->sta_rsn_ie, sp->rsn, "sta->sta_rsn_ie");
        if (sp->wps != NULL)
            wifi_mac_saveie(&sta->sta_wps_ie, sp->wps, "sta->sta_wps_ie");
#ifdef CONFIG_P2P
        for (index = 0; index < MAX_P2PIE_NUM; index++) {
            if (sp->p2p[index] != NULL)
                wifi_mac_saveie(&sta->sta_p2p_ie[index], sp->p2p[index], "sta->sta_p2p_ie");
        }
#ifdef CONFIG_WFD
        if (sp->wfd != NULL)
            wifi_mac_saveie(&sta->sta_wfd_ie, sp->wfd, "sta->sta_wfd_ie");
#endif//#ifdef CONFIG_WFD
#endif//#ifdef CONFIG_P2P
        wifi_mac_setup_rates(sta, sp->rates, sp->xrates, WIFINET_F_DOSORT);
        wifi_mac_new_assoc(sta, 1);
        wifi_mac_sta_auth(sta);
    }

    chan_dbg(wifimac->wm_curchan,"X13", 953);
    return sta;
}


struct wifi_station *
wifi_mac_find_rx_sta(struct wifi_mac *wifimac,
                   const struct wifi_mac_frame_min *wh,int wnet_vif_id)
{
    struct wifi_station_tbl *nt;
    struct wifi_station *sta;
    struct wlan_net_vif *wnet_vif = wifi_mac_get_wnet_vif_by_vid(wifimac, wnet_vif_id);

    if (wnet_vif == NULL)
        return NULL;

    nt =  &wnet_vif->vm_sta_tbl;
    if (WIFINET_IS_CRTL(wh) && !WIFINET_IS_PSPOLL(wh) && !WIFINET_IS_BAR(wh))
        sta = wifi_mac_find_sta(nt, wh->i_addr1, wnet_vif_id);
    else
        sta = wifi_mac_find_sta(nt, wh->i_addr2, wnet_vif_id);

    return sta;
}

struct wifi_station * wifi_mac_find_mgmt_tx_sta(struct wlan_net_vif *wnet_vif, const unsigned char *mac)
{
    struct wifi_station_tbl *nt;
    struct wifi_station *sta;

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) || WIFINET_IS_MULTICAST(mac))
    {
        return wnet_vif->vm_mainsta;
    }

    nt = &wnet_vif->vm_sta_tbl;
    sta = wifi_mac_find_sta(nt, mac,wnet_vif->wnet_vif_id);

    if (sta == NULL)
    {
        if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
        {
            sta = wifi_mac_fake_adhos_sta(wnet_vif, mac);
        }
        else
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_XMIT|AML_DEBUG_DEBUG, mac,
                                     "no nsta, discard frame (%s)", __func__);
            wnet_vif->vif_sts.sts_tx_no_sta++;
        }
    }
    else
    {
        if (sta->sta_wnet_vif != wnet_vif)
        {
            WIFINET_DPRINTF_MACADDR(AML_DEBUG_XMIT, mac, "no nsta, discard frame (%s)", __func__);
            wnet_vif->vif_sts.sts_tx_no_sta++;
            return NULL;
        }
    }
    return sta;
}

struct wifi_station * wifi_mac_find_tx_sta(struct wlan_net_vif *wnet_vif, const unsigned char *mac)
{
    struct wifi_station_tbl *nt;
    struct wifi_station *sta;

    if (wnet_vif->vm_opmode == WIFINET_M_STA)
    {
        //DPRINTF(AML_DEBUG_WARNING,"<WARNING> %s %d \n",__func__,__LINE__);
        return wnet_vif->vm_mainsta;
    }

    if (WIFINET_IS_MULTICAST(mac))
    {
        if ((wnet_vif->vm_sta_assoc > 0)||(wnet_vif->vm_opmode == WIFINET_M_IBSS))
        {
            //DPRINTF(AML_DEBUG_WARNING,"<WARNING> %s %d \n",__func__,__LINE__);
            return wnet_vif->vm_mainsta;
        }
        else
        {
            wnet_vif->vif_sts.sts_tx_no_sta++;
            DPRINTF(AML_DEBUG_WARNING,"<WARNING> %s %d \n",__func__,__LINE__);
            return NULL;
        }
    }

    nt = &wnet_vif->vm_sta_tbl;
    sta = wifi_mac_find_sta(nt, mac,wnet_vif->wnet_vif_id);

    if (sta == NULL)
    {
        if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
        {
            sta = wifi_mac_fake_adhos_sta(wnet_vif, mac);
        }
        else
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_XMIT, mac,
                                     "no nsta, discard frame (%s)", __func__);
            wnet_vif->vif_sts.sts_tx_no_sta++;
        }
    }
    else
    {
        if (sta->sta_wnet_vif != wnet_vif)
        {
            WIFINET_DPRINTF_MACADDR(AML_DEBUG_XMIT, mac, "no nsta, discard frame (%s)", __func__);
            wnet_vif->vif_sts.sts_tx_no_sta++;
            ERROR_DEBUG_OUT("<running> error \n");
            return NULL;
        }
    }

    return sta;
}

static int wifi_mac_free_sta_delay(void * arg)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)arg;
    struct nsta_entry *del_nsta;
    struct wifi_station *sta;

    WIFINET_NODE_FREE_LOCK(wifimac);
    while (!list_empty(&wifimac->wm_free_entryq))
    {
        del_nsta = list_first_entry(&wifimac->wm_free_entryq, struct nsta_entry, entry);
        if ((jiffies - del_nsta->queue_time) > ((WIFINET_NODE_FREE_WAIT*HZ) / 1000))
        {
            list_del_init(&del_nsta->entry);
            WIFINET_NODE_FREE_UNLOCK(wifimac);

            sta = del_nsta->nsta;
            nsta_free(sta);
            FREE(del_nsta,"del_nsta");
            WIFINET_NODE_FREE_LOCK(wifimac);

        } else {
            break;
        }
    }
    WIFINET_NODE_FREE_UNLOCK(wifimac);
    return OS_TIMER_NOT_REARMED;
}

static void wifi_mac_free_sta_now(struct wifi_mac *wifimac)
{
    struct nsta_entry *del_nsta;
    struct wifi_station *sta;

    WIFINET_NODE_FREE_LOCK(wifimac);
    while (!list_empty(&wifimac->wm_free_entryq))
    {
        del_nsta = list_first_entry(&wifimac->wm_free_entryq, struct nsta_entry, entry);
        list_del_init(&del_nsta->entry);
        WIFINET_NODE_FREE_UNLOCK(wifimac);

        sta = del_nsta->nsta;
        /* Clear 'sta_wnet_vif', since this function is called
         * only during driver unload and both interfaces are
         * released, so we can't touch them in 'nsta_free()'.
         */
        sta->sta_wnet_vif = NULL;
        nsta_free(sta);
        FREE(del_nsta,"del_nsta");

        WIFINET_NODE_FREE_LOCK(wifimac);
    }
    WIFINET_NODE_FREE_UNLOCK(wifimac);
}

static int wifi_mac_sta_is_in_free_queue(struct wifi_mac *wifimac, struct wifi_station *sta)
{
    struct nsta_entry *del_nsta = NULL;
    struct nsta_entry *del_nsta_next = NULL;

    list_for_each_entry_safe(del_nsta, del_nsta_next, &wifimac->wm_free_entryq, entry) {
        if (sta == del_nsta->nsta) {
            ERROR_DEBUG_OUT("sta:%p already in free queue\n", sta);
            return 1;
        }
    }

    return 0;
}

void wifi_mac_free_sta(struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac = (struct wifi_mac *)sta->sta_wmac;
    struct nsta_entry *del_nsta;
    unsigned char new_delete = 0;

    del_nsta = (struct nsta_entry *)NET_MALLOC(sizeof(struct nsta_entry *),
    GFP_ATOMIC, "del_nsta");

    if (del_nsta) {
        WIFINET_NODE_FREE_LOCK(wifimac);
        if (!wifi_mac_sta_is_in_free_queue(wifimac, sta)) {
            del_nsta->nsta = sta;
            del_nsta->queue_time = jiffies;
            new_delete = 1;

            list_add_tail(&del_nsta->entry, &wifimac->wm_free_entryq);

        } else {
            FREE(del_nsta,"del_nsta");
        }
        WIFINET_NODE_FREE_UNLOCK(wifimac);

        pr_debug("%s sta:%p, new:%d\n", __func__, sta, new_delete);
        if (new_delete) {
        if (sta == wnet_vif->vm_mainsta) {
            pr_debug("free vm_mainsta:%p\n", sta);
            wnet_vif->vm_mainsta = NULL;
        }

        os_timer_ex_start(&wifimac->wm_free_timer);
        nsta_cleanup(sta);
    }
    return;
    }

}

void wifi_mac_free_sta_from_list(struct wifi_station *sta)
{
    struct wifi_station_tbl *nt = &(sta->sta_wnet_vif->vm_sta_tbl);

    WIFINET_NODE_LOCK(nt);
    list_del_init(&sta->sta_list);
    WIFINET_NODE_UNLOCK(nt);

    wifi_mac_free_sta(sta);
}

static void wifi_mac_sta_table_rst(struct wifi_station_tbl *nt, struct wlan_net_vif *match,
                                   bool rst_mainsta)
{
    struct wifi_station *sta = NULL, *next = NULL;

    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, next, &nt->nt_nsta, sta_list) {
        struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

        if ((match != NULL) && (wnet_vif != match))
            continue;

        if (sta->sta_associd != 0) {
            vm_StaClearAid(wnet_vif, sta->sta_associd);
        }

        if (sta != wnet_vif->vm_mainsta || rst_mainsta) {
            wifi_mac_rm_sta_from_wds_by_addr(nt,sta->sta_macaddr);
            list_del_init(&sta->sta_list);

            WIFINET_NODE_UNLOCK(nt);
            wifi_mac_free_sta(sta);
            WIFINET_NODE_LOCK(nt);
        }
    }
    WIFINET_NODE_UNLOCK(nt);
}

static void wifi_mac_clear_sta_table(struct wifi_station_tbl *nt)
{
    struct wifi_station *sta = NULL, *next = NULL;

    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, next, &nt->nt_nsta, sta_list) {
        if (sta->sta_associd != 0) {
            vm_StaClearAid(sta->sta_wnet_vif, sta->sta_associd);
        }

        list_del_init(&sta->sta_list);
        WIFINET_NODE_UNLOCK(nt);
        wifi_mac_free_sta(sta);
        WIFINET_NODE_LOCK(nt);
    }
    WIFINET_NODE_UNLOCK(nt);

    WIFINET_NODE_LOCK_DESTROY(nt);
}

void wifi_mac_list_sta( struct wlan_net_vif *wnet_vif)
{
    struct wifi_station_tbl *nt = &wnet_vif->vm_sta_tbl;
    struct wifi_station *sta = NULL;
    struct wifi_station *next = NULL;

    pr_debug("station list==========>: \n");
    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, next, &nt->nt_nsta, sta_list) {
        if (wnet_vif != sta->sta_wnet_vif) {
            continue;
        }

        if(sta->sta_associd != 0) {
            pr_debug("station staid %d, ",sta->sta_associd&0xff);
            pr_debug("mac addr %s \n",ether_sprintf(sta->sta_macaddr));
            wifi_mac_dump_sta(nt,sta);
        }
    }
    WIFINET_NODE_UNLOCK(nt);
    pr_debug("=========end================\n");
}

static void wifi_mac_TimeoutStations(struct wifi_station_tbl *nt)
{
    struct wifi_station *sta = NULL, *sta_next = NULL;
    struct sk_buff_head skb_freeq;
    struct sk_buff *skb;
    int arg = 0;
    struct wlan_net_vif *wnet_vif = NULL;
    skb_queue_head_init(&skb_freeq);

    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        if ((sta->sta_flags & WIFINET_NODE_AREF) == 0)
            continue;

        if (sta->sta_associd != 0) {
            struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

            if (wifi_mac_pwrsave_psqueue_age(sta, &skb_freeq) != 0
                && WIFINET_SAVEQ_QLEN(&(sta->sta_pstxqueue)) == 0
                && (wnet_vif->vif_ops.vm_set_tim != NULL))
                wnet_vif->vif_ops.vm_set_tim(sta, 0);
        }
        /*
            if within 5s since last packet, we haven't receive new packets:
            sta send null data, ap send deauth until sta_inact = 0.
        */
        if (jiffies < (sta->sta_rstamp + 5 * HZ))
            continue;

        if ((sta->sta_rxfrag[0] != NULL) && (jiffies > sta->sta_rxfragstamp + HZ)) {
            kfree_skb(sta->sta_rxfrag[0]);
            sta->sta_rxfrag[0] = NULL;
        }

        if (sta == sta->sta_wnet_vif->vm_mainsta) {
            if (sta->sta_inact > 0)
                sta->sta_inact--;
            continue;
        }

        sta->sta_inact--;
        if (sta->sta_associd != 0) {
            struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

            if ((0 < sta->sta_inact) && (sta->sta_inact <= wnet_vif->vm_inact_probe)) {
                WIFINET_DPRINTF_STA(AML_DEBUG_ANY , sta, "%s", "probe station due to inactivity");
                wifi_mac_send_nulldata_for_ap(sta, 0, 0, 0, 0);
            }
        }

        if (sta->sta_inact <= 0) {
            WIFINET_DPRINTF_STA(AML_DEBUG_ANY , sta, "station timed out due to inactivity:%p", sta);

            sta->sta_wnet_vif->vif_sts.sts_tx_sta_aged++;
            if (sta->sta_associd != 0) {
                arg = WIFINET_REASON_AUTH_EXPIRE;
                wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&arg);
            }
            wnet_vif = sta->sta_wnet_vif;
            WIFINET_NODE_UNLOCK(nt);
            wifi_softap_allsta_stopping(wnet_vif,1);
            wifi_mac_notify_nsta_disconnect(sta, 0);
            WIFINET_NODE_LOCK(nt);
        }
    }
    WIFINET_NODE_UNLOCK(nt);

    while ((skb = skb_peek(&skb_freeq)) != NULL) {
        skb = __skb_dequeue(&skb_freeq);
        kfree_skb(skb);
    }
}

void wifi_mac_set_arp_agent(struct wlan_net_vif *wnet_vif, int enable)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct in_device *in_dev;
    struct in_ifaddr *ifa_v4;
    struct inet6_dev *idev_v6 = NULL;
    struct inet6_ifaddr *ifa_v6 = NULL;
    struct in6_addr *ipv6_ptr = NULL;
    __be32 ipv4;

    unsigned char ipv6[IPV6_ADDR_BUF_LEN] = {0};
    int i = 0, j = 0;

    if (wnet_vif->vm_hal_opmode != WIFI_M_STA)
    {
        ERROR_DEBUG_OUT("not support opmode %d\n", wnet_vif->vm_hal_opmode);
        return;
    }

    if (enable == 0)
    {
        /*just disable arp agent */
        wnet_vif->vm_wmac->drv_priv->drv_ops.drv_set_arp_agent(wifimac->drv_priv,
            wnet_vif->wnet_vif_id, enable, 0, NULL);
        return;
    }
    /* get ipv4 addr */
    in_dev = __in_dev_get_rtnl(wnet_vif->vm_ndev);
    if (!in_dev)
        return ;

    ifa_v4 = in_dev->ifa_list;
    if (!ifa_v4) {
        return;
    }
    memset(&ipv4, 0, sizeof(ipv4));
    ipv4 = ifa_v4->ifa_local;

    /*get ipv6 addr */
    idev_v6 = __in6_dev_get(wnet_vif->vm_ndev);
    if (!idev_v6)
    {
        ERROR_DEBUG_OUT("not support ipv6\n");
        return ;
    }

    read_lock_bh(&idev_v6->lock);
    list_for_each_entry(ifa_v6, &idev_v6->addr_list, if_list)
    {
        unsigned int addr_type = __ipv6_addr_type(&ifa_v6->addr);

        if ((ifa_v6->flags & IFA_F_TENTATIVE) &&
                (!(ifa_v6->flags & IFA_F_OPTIMISTIC)))
            continue;

        if (unlikely(addr_type == IPV6_ADDR_ANY ||
                addr_type & IPV6_ADDR_MULTICAST))
            continue;

        ipv6_ptr = &ifa_v6->addr;
        if (ipv6_ptr->in6_u.u6_addr8[0] != 0)
        {
            memcpy(ipv6 + j * sizeof(struct in6_addr), ipv6_ptr->in6_u.u6_addr8, sizeof(struct in6_addr));
            j++;
        }
        i++;
        /* we just support 3 ipv6 addr at most. */
        if (i > 2)
            break;
    }
    read_unlock_bh(&idev_v6->lock);

    wnet_vif->vm_wmac->drv_priv->drv_ops.drv_set_arp_agent(wifimac->drv_priv,
                                                wnet_vif->wnet_vif_id, enable, ipv4, ipv6);
}

int wifi_mac_sta_arp_agent_ex (SYS_TYPE param1,
                                SYS_TYPE param2,SYS_TYPE param3,
                                SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;
    int enable = (int)param3;

    wifi_mac_set_arp_agent(wnet_vif, enable);
    return 0;
}

void wifi_mac_sta_keep_alive(struct wlan_net_vif *wnet_vif, int enable, int period)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct NullDataCmd null_data;
    int len = 0;

    memset(&null_data, 0, sizeof(struct NullDataCmd));
    if (enable == 0)
    {
        /*just disable keep alive */
        wnet_vif->vm_wmac->drv_priv->drv_ops.drv_keep_alive(wifimac->drv_priv, null_data, len, enable, period);
        return;
    }
    if ((wnet_vif->vm_opmode != WIFINET_M_STA)
        || (wnet_vif->vm_mainsta->sta_associd == 0))
    {
        return;
    }
    null_data.vid = wnet_vif->wnet_vif_id;
    null_data.pwr_save= 0;
    null_data.tid = 0;
    null_data.qos = 0;
    null_data.staid = (wnet_vif->vm_opmode == WIFINET_M_STA) ? 1 : wnet_vif->vm_mainsta->sta_associd;
    if (wifimac->drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH20)
    {
        null_data.bw = CHAN_BW_20M;
    }
    else if(wifimac->drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH40)
    {
        null_data.bw = (wnet_vif->vm_mainsta->sta_chbw == WIFINET_BWC_WIDTH20)
                            ? CHAN_BW_20M : CHAN_BW_40M;
    }
    else if(wifimac->drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH80)
    {
        null_data.bw = (wnet_vif->vm_mainsta->sta_chbw == WIFINET_BWC_WIDTH20) ? CHAN_BW_20M :
            ((wnet_vif->vm_mainsta->sta_chbw == WIFINET_BWC_WIDTH40) ? CHAN_BW_40M : CHAN_BW_80M);
    }

    if((wnet_vif->vm_mainsta->sta_flags & WIFINET_NODE_HT)
        || (wnet_vif->vm_mainsta->sta_flags & WIFINET_NODE_VHT))
    {
        null_data.rate = WIFI_11G_6M;
    }
    else
    {
        null_data.rate = WIFI_11B_1M;
        null_data.bw = CHAN_BW_20M;
    }

    len = sizeof(struct wifi_frame) + FCS_LEN;

    memcpy(&null_data.dest_addr, wnet_vif->vm_mainsta->sta_macaddr, WIFINET_ADDR_LEN);
    null_data.sn = 0;
    wnet_vif->vm_wmac->drv_priv->drv_ops.drv_keep_alive(wifimac->drv_priv, null_data, len, enable, period);
}

void wifi_mac_sta_keep_alive_ex (SYS_TYPE param1,
                                SYS_TYPE param2,SYS_TYPE param3,
                                SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;
    int period = (int)param5;
    int enable = (int)param3;

    if ((period == 0) || (wnet_vif->vm_mainsta == NULL))
        return ;

    wifi_mac_sta_keep_alive(wnet_vif, enable, period);
    return ;
}

static void wifi_mac_StationTimeoutEx(SYS_TYPE param1,
    SYS_TYPE param2, SYS_TYPE param3, SYS_TYPE param4, SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *) param1;
    struct wlan_net_vif *wnet_vif = NULL;

    ASSERT(wifimac != NULL);


    list_for_each_entry(wnet_vif,&wifimac->wm_wnet_vifs,vm_next)
    {
        if(wnet_vif->vm_state == WIFINET_S_CONNECTED)
        {
            ASSERT(wnet_vif->vm_mainsta != NULL);
            wifi_mac_TimeoutStations(&wnet_vif->vm_sta_tbl);
        }
    }
    os_timer_ex_start(&wifimac->wm_inact_timer);

}

static int wifi_mac_StationTimeout(void *arg)
{
    ASSERT(arg != 0);

    wifi_mac_add_work_task(arg,wifi_mac_StationTimeoutEx,NULL,(SYS_TYPE)arg,0,0,0,0);
    return OS_TIMER_NOT_REARMED;
}

void wifi_mac_func_to_task_cb(SYS_TYPE param1, SYS_TYPE param2, SYS_TYPE param3, SYS_TYPE param4, SYS_TYPE param5)
{
    wifi_mac_IterFunc *f = (wifi_mac_IterFunc *)param1;
    void *arg = (void *)param2;
    struct wifi_station *sta = (struct wifi_station *)param3;
    if (f != NULL)
        (*f)(arg, sta);
}

void wifi_mac_sta_disassoc(void *arg, struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = arg;	
    struct wifi_station_tbl *nt = &(wnet_vif->vm_sta_tbl);
    int mgmt_arg = WIFINET_REASON_ASSOC_LEAVE;

    if ((sta->sta_wnet_vif == wnet_vif) && (sta->sta_associd != 0)) {
        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DISASSOC, (void *)&mgmt_arg);
        pr_debug("<running> %s %d \n",__func__,__LINE__);

        wifi_mac_sta_disconnect(sta);
        wifi_mac_rm_sta_from_wds_by_addr(nt, sta->sta_macaddr);
        list_del_init(&sta->sta_list);
        wifi_mac_free_sta(sta);
    }
}

void wifi_mac_sta_deauth(void *arg, struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = arg;
    int mgmt_arg = WIFINET_REASON_ASSOC_LEAVE;

    if (sta->sta_wnet_vif == wnet_vif)
    {
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&mgmt_arg);
    }
}

static void wifi_mac_disassoc_cb(SYS_TYPE param1, SYS_TYPE param2, SYS_TYPE param3,
                                 SYS_TYPE param4, SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;
    struct wifi_station *sta = (struct wifi_station *)param2;
    struct wifi_station_tbl *nt = &(wnet_vif->vm_sta_tbl);
    int mgmt_arg = WIFINET_REASON_ASSOC_LEAVE;

    wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DISASSOC, (void *)&mgmt_arg);
    pr_debug("<running> %s %d \n",__func__,__LINE__);

    wifi_mac_sta_disconnect(sta);
    wifi_mac_rm_sta_from_wds_by_addr(nt, sta->sta_macaddr);

    wifi_mac_free_sta(sta);
}

void wifi_mac_sta_disassoc_in_task(struct wlan_net_vif *wnet_vif)
{
    struct wifi_station *sta = NULL, *sta_next = NULL;
    struct wifi_station_tbl *nt = &(wnet_vif->vm_sta_tbl);

    WIFINET_NODE_LOCK(nt);

    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        if ((sta->sta_wnet_vif == wnet_vif) && sta->sta_associd) {
            /* Remove 'sta' with lock from global list here. So
             * nobody can access it later. 'wifi_mac_disassoc_cb()'
             * will free it later.
             */
            list_del_init(&sta->sta_list);

            wifi_mac_add_work_task(nt->nt_wmac, wifi_mac_disassoc_cb, NULL,
                                   (SYS_TYPE)wnet_vif, (SYS_TYPE)sta, NULL,
                                   NULL, NULL);
        }
    }

    WIFINET_NODE_UNLOCK(nt);
}

void wifi_mac_func_to_task(struct wifi_station_tbl *nt, wifi_mac_IterFunc *f, void *arg,unsigned char btask)
{
    struct wifi_station *sta = NULL, *sta_next = NULL;

    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        if (btask) {
            wifi_mac_add_work_task(nt->nt_wmac,wifi_mac_func_to_task_cb,
                NULL,(SYS_TYPE)f,(SYS_TYPE)arg,(SYS_TYPE)sta,(SYS_TYPE)sta->sta_wnet_vif,(SYS_TYPE)sta->sta_wnet_vif->wnet_vif_replaycounter);

        } else {
            if (f != NULL)
                (*f)(arg, sta);
        }
    }
    WIFINET_NODE_UNLOCK(nt);
}

void wifi_mac_disassoc_all_sta(struct wlan_net_vif *reqwnet_vif, wifi_mac_IterFunc *f, void *arg)
{
    struct wifi_station_tbl *nt = &reqwnet_vif->vm_sta_tbl;
    struct wifi_station *sta = NULL, *sta_next = NULL;

    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
        if(wnet_vif && (wnet_vif != reqwnet_vif)) {
            continue ;
        }

        if (sta->sta_associd != 0) {
            WIFINET_NODE_UNLOCK(nt);
            (*f)(arg, sta);
            WIFINET_NODE_LOCK(nt);
            wifi_mac_sta_disconnect(sta);
            wifi_mac_rm_sta_from_wds_by_addr(nt,sta->sta_macaddr);
            list_del_init(&sta->sta_list);
            WIFINET_NODE_UNLOCK(nt);
            wifi_mac_free_sta(sta);
            WIFINET_NODE_LOCK(nt);
        }
    }
    WIFINET_NODE_UNLOCK(nt);
}

void wifi_mac_dump_sta(struct wifi_station_tbl *nt, struct wifi_station *sta)
{
    int i;

    DPRINTF(AML_DEBUG_DEBUG,"0x%p: mac %s\n", sta, ether_sprintf(sta->sta_macaddr));
    DPRINTF(AML_DEBUG_DEBUG,"\tauthmode %u flags 0x%x\n",
            sta->sta_authmode, sta->sta_flags);
    DPRINTF(AML_DEBUG_DEBUG,"\tassocid 0x%x txpower %u vlan %u\n",
            sta->sta_associd, sta->sta_txpower, sta->sta_vlan);
    DPRINTF(AML_DEBUG_DEBUG,"rxfragstamp %lu\n", sta->sta_rxfragstamp);
    for (i=0; i<17; i++)
    {
        DPRINTF(AML_DEBUG_DEBUG,"\t%d: txseq %u rxseq %u fragno %u\n", i,
                sta->sta_txseqs[i],
                sta->sta_rxseqs[i] >> WIFINET_SEQ_SEQ_SHIFT,
                sta->sta_rxseqs[i] & WIFINET_SEQ_FRAG_MASK);
    }
    DPRINTF(AML_DEBUG_DEBUG,"\trstamp %llu intval %u capinfo 0x%x\n",
            (unsigned long long)sta->sta_rstamp, sta->sta_listen_intval, sta->sta_capinfo);
    DPRINTF(AML_DEBUG_DEBUG,"\tbssid %s essid \"%.*s\" channel %u:0x%x\n",
            ether_sprintf(sta->sta_bssid),
            sta->sta_esslen, sta->sta_essid,
            sta->sta_wnet_vif->vm_curchan != WIFINET_CHAN_ERR ?
            sta->sta_wnet_vif->vm_curchan->chan_cfreq1 : WIFINET_CHAN_INVALUE,
            sta->sta_wnet_vif->vm_curchan != WIFINET_CHAN_ERR ? sta->sta_wnet_vif->vm_curchan->chan_flags : 0);
    DPRINTF(AML_DEBUG_DEBUG,"\tinact %u\n", sta->sta_inact);

    DPRINTF(AML_DEBUG_DEBUG,"\tchwidth %u\n", sta->sta_chbw);
}

static void wifi_mac_11g_init_param(struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;

    WIFINET_LOCK_ASSERT(wifimac);

    if ((sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME) == 0)
    {
        wifimac->wm_longslotsta++;
        WIFINET_DPRINTF_STA(AML_DEBUG_CONNECT, sta,
            "station needs long slot time, count %d", wifimac->wm_longslotsta);
        wifi_mac_set_shortslottime(wifimac, 0);
    }

    if (!wifi_mac_iserp_rateset(wifimac, &sta->sta_rates))
    {
        wifimac->wm_nonerpsta++;
        WIFINET_DPRINTF_STA( AML_DEBUG_CONNECT, sta,
            "station is !ERP, %d non-ERP stations associated", wifimac->wm_nonerpsta);

        if (wifimac->wm_protmode != WIFINET_PROT_NONE)
        {
            wifimac->wm_flags |= WIFINET_F_USEPROT;
            wifi_mac_update_protmode(wifimac);
        }

        if ((sta->sta_capinfo & WIFINET_CAPINFO_SHORT_PREAMBLE) == 0)
        {
            WIFINET_DPRINTF_STA( AML_DEBUG_CONNECT, sta, "%s", "station needs long preamble");
            wifimac->wm_flags |= WIFINET_F_USEBARKER;
            wifimac->wm_flags &= ~WIFINET_F_SHPREAMBLE;
        }

        if (wifimac->wm_nonerpsta == 1)
            wifimac->wm_flags_ext |= WIFINET_FEXT_ERPUPDATE;
    }
    else
    {
        sta->sta_flags |= WIFINET_NODE_ERP;
    }
}

static void wifi_mac_11g_rst_param(struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    WIFINET_LOCK_ASSERT(wifimac);
    if ((sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME) == 0) {
        DPRINTF(AML_DEBUG_INFO, "%s (%d), wm_longslotsta %d\n", __func__, __LINE__, wifimac->wm_longslotsta);
        if (wifimac->wm_longslotsta > 0)
            wifimac->wm_longslotsta--;

        WIFINET_DPRINTF_STA(AML_DEBUG_CONNECT, sta, "long slot time station leaves, count now %d", wifimac->wm_longslotsta);

        if (wifimac->wm_longslotsta == 0) {
            if ((wifimac->wm_caps & WIFINET_C_SHSLOT) && wnet_vif->vm_opmode != WIFINET_M_IBSS) {
                wifi_mac_set_shortslottime(wifimac, 1);
            }
        }
    }

    if ((sta->sta_flags & WIFINET_NODE_ERP) == 0) {
        if (wifimac->wm_nonerpsta == 0)
            return;

        wifimac->wm_nonerpsta--;
        WIFINET_DPRINTF_STA(AML_DEBUG_CONNECT, sta, "non-ERP station leaves, count now %d", wifimac->wm_nonerpsta);

        if (wifimac->wm_nonerpsta == 0) {
            if ((jiffies - wifimac->wm_time_nonerp_present) >= WIFINET_INACT_NONERP * HZ) {
                wifimac->wm_flags &= ~WIFINET_F_USEPROT;
                wifi_mac_update_protmode(wifimac);
            }

            if (wifimac->wm_caps & WIFINET_C_SHPREAMBLE) {
                wifimac->wm_flags |= WIFINET_F_SHPREAMBLE;
                wifimac->wm_flags &= ~WIFINET_F_USEBARKER;
            }
            wifimac->wm_flags_ext |= WIFINET_FEXT_ERPUPDATE;
        }
    }
}

void wifi_mac_sta_connect(struct wifi_station *sta, int resp)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int newassoc;
    long long arg;

    if (sta->sta_associd == 0)
    {
        unsigned short aid;

        for (aid = 1; aid < wnet_vif->vm_max_aid; aid++)
        {
            if (!vm_StaIsSetAid(wnet_vif, aid))
                break;
        }
        if (aid >= wnet_vif->vm_max_aid)
        {
            void * para = NULL;

            arg = WIFINET_REASON_ASSOC_TOOMANY;
            para = (void *)&arg;
            wifi_mac_send_mgmt(sta, resp, para);
            wifi_mac_sta_disconnect_from_ap(sta);
            return;
        }

        WIFINET_LOCK(wifimac);
        sta->sta_associd = aid | 0xc000;
        vm_StaSetAid(wnet_vif, sta->sta_associd);
        wnet_vif->vm_sta_assoc++;
        WIFINET_UNLOCK(wifimac);

#ifdef CONFIG_P2P
        if (wnet_vif->vm_wdev->iftype == NL80211_IFTYPE_P2P_GO)
        {
            if (wnet_vif->vm_p2p->p2p_flag & P2P_OPPPS_START_FLAG_HI)
            {
                vm_p2p_go_cancel_opps(wnet_vif->vm_p2p);
            }
            if (wnet_vif->vm_p2p->p2p_flag & P2P_NOA_START_FLAG_HI)
            {
                vm_p2p_go_cancel_noa(wnet_vif->vm_p2p);
            }
        }
#endif
        if ((sta->sta_flags & WIFINET_NODE_HT) &&
            (sta->sta_flags_ext & WIFINET_NODE_40_INTOLERANT))
        {
            sta->sta_chbw = WIFINET_BWC_WIDTH20;
            sta->sta_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;

            sta->sta_wnet_vif->vm_curchan->chan_bw = WIFINET_BWC_WIDTH20;
            sta->sta_wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH20;
            sta->sta_wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_NA;
            sta->sta_wnet_vif->vm_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;

            wifi_mac_change_cbw(wifimac, 1);
        }

        if (WIFINET_INCLUDE_11G(wnet_vif->vm_mac_mode))
        {
            WIFINET_LOCK(wifimac);
            wifi_mac_11g_init_param(sta);
            WIFINET_UNLOCK(wifimac);
        }
        /*record sta work operation mode */
        wifi_mac_ht_prot(wifimac, sta, WIFINET_STA_CONNECT);

        newassoc = 1;
    }
    else
    {
        newassoc = 0;
    }

    if (sta->sta_ucastkey.wk_keyix != WIFINET_KEYIX_NONE)
    {
        DPRINTF(AML_DEBUG_KEY, "<%s>:%s %d delete key \n", VMAC_DEV_NAME(wnet_vif),__func__,__LINE__);
        wifi_mac_sec_delt_key(sta->sta_wnet_vif, &sta->sta_ucastkey, sta);
    }

    /*init or memset sta buffer parameters */
    sta->sta_inact_reload = wnet_vif->vm_inact_auth;
    sta->sta_inact = sta->sta_inact_reload;
    {
        void *para = NULL;

        arg = WIFINET_STATUS_SUCCESS;
        para = (void *)&arg;
        if (wifi_mac_send_mgmt(sta, resp, para) != 0) {
            wifi_mac_sta_disconnect_from_ap(sta);
            return;
        }
    }
    wifi_mac_new_assoc(sta, newassoc);
    wifi_mac_notify_nsta_connect(sta, newassoc);

    if (wnet_vif->vm_curchan == WIFINET_CHAN_ERR) {
        wifi_mac_sta_disconnect_from_ap(sta);
        return;
    }

    AML_OUTPUT("****************************************************\n");
    AML_OUTPUT("sta associated!!! STA CHANNEL:%d, BW:%d, SSID:%s, BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",
        wnet_vif->vm_curchan->chan_pri_num, sta->sta_chbw, sta->sta_essid,
        sta->sta_macaddr[0], sta->sta_macaddr[1], sta->sta_macaddr[2],
        sta->sta_macaddr[3], sta->sta_macaddr[4], sta->sta_macaddr[5]);
    pr_debug("****************************************************\n");
}

void
wifi_mac_sta_disconnect(struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta,
        "station with aid %d leaves", WIFINET_NODE_AID(sta));

    if (sta->sta_associd == 0)
    {
        DPRINTF(AML_DEBUG_NODE, "%s %d\n", __func__, __LINE__);
        goto done;
    }

    DPRINTF(AML_DEBUG_NODE, "%s %d\n", __func__, __LINE__);
    WIFINET_LOCK(wifimac);
    vm_StaClearAid(wnet_vif, sta->sta_associd);
    //sta->sta_associd = 0;
    wnet_vif->vm_sta_assoc--;
    WIFINET_UNLOCK(wifimac);

    if ((sta->sta_flags & WIFINET_NODE_HT) && (sta->sta_flags_ext & WIFINET_NODE_40_INTOLERANT))
    {
        DPRINTF(AML_DEBUG_NODE, "%s %d\n", __func__, __LINE__);
        wifi_mac_change_cbw(wifimac, 0);
    }

    if (WIFINET_INCLUDE_11G(wnet_vif->vm_mac_mode))
    {
        DPRINTF(AML_DEBUG_NODE, "%s %d\n", __func__, __LINE__);

        WIFINET_LOCK(wifimac);
        wifi_mac_11g_rst_param(sta);
        WIFINET_UNLOCK(wifimac);
    }
    wifi_mac_ht_prot(wifimac, sta, WIFINET_STA_DISCONNECT);

    DPRINTF(AML_DEBUG_NODE, "%s %d\n", __func__, __LINE__);

    wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_NONE;
    wifi_mac_notify_nsta_disconnect(sta, 0);

done:
    DPRINTF(AML_DEBUG_NODE, "%s %d\n", __func__, __LINE__);
}

void
wifi_mac_sta_disconnect_from_ap(struct wifi_station *sta)
{
    struct wifi_station_tbl *nt = &(sta->sta_wnet_vif->vm_sta_tbl);
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    wifi_mac_sta_disconnect(sta);

    WIFINET_NODE_LOCK(nt);
    wifi_mac_rm_sta_from_wds_by_addr(nt, sta->sta_macaddr);
    list_del_init(&sta->sta_list);
    WIFINET_NODE_UNLOCK(nt);
    wifi_mac_free_sta(sta);
    wifi_softap_allsta_stopping(wnet_vif,0);
}

void wifi_mac_rst_bss(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    pr_debug("%s, wnet_vif->wnet_vif_id= %d\n", __func__, wnet_vif->wnet_vif_id);

    wifi_mac_sta_table_rst(&wnet_vif->vm_sta_tbl, wnet_vif, false);
    wifi_mac_reset_erp(wifimac, wnet_vif->vm_mac_mode);
    wifi_mac_reset_ht(wifimac);

    if (!wnet_vif->vm_mainsta) {
        wnet_vif->vm_mainsta = wifi_mac_get_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, wnet_vif->vm_myaddr);
        KASSERT(wnet_vif->vm_mainsta != NULL, ("unable to create vm_mainsta"));
        pr_debug("%s add vm_mainsta:%p\n", __func__, wnet_vif->vm_mainsta);
    }

    wnet_vif->vm_ps_sta = 0;
}

void wifi_mac_rst_main_sta(struct wlan_net_vif *wnet_vif)
{
    struct wifi_station *sta = wnet_vif->vm_mainsta;

    pr_debug("%s, wnet_vif_id= %d, main_sta:%p\n", __func__, wnet_vif->wnet_vif_id, sta);

    if (sta) {
        sta->sta_associd = 0;
        sta->sta_authmode = WIFINET_AUTH_OPEN;
        memset(sta->sta_bssid, 0, WIFINET_ADDR_LEN);
        memset(sta->sta_macaddr, 0, WIFINET_ADDR_LEN);

        sta->sta_chbw = 0;
        sta->connect_status = CONNECT_IDLE;
        sta->ip_acquired = 0;
        memset(sta->sta_ap_ip, 0, IPV4_LEN);
        memset(sta->sta_ap_ipv6, 0, IPV6_LEN);
        sta->sta_update_rate_flag = 0;
        sta->sta_bssmode = wnet_vif->vm_mac_mode;

        sta->sta_flags = 0;
        sta->sta_flags_ext = 0;
        memset(&sta->sta_rsn, 0, sizeof(struct wifi_mac_Rsnparms));
    }
}

void wifi_mac_sta_vattach(struct wlan_net_vif *wnet_vif)
{
    DPRINTF(AML_DEBUG_INIT,"<%s> %s wnet_vif->vm_mainsta= %p \n",VMAC_DEV_NAME(wnet_vif),__func__, wnet_vif->vm_mainsta);

    wnet_vif->vm_inact_init = WIFINET_INACT_INIT;
    wnet_vif->vm_inact_auth = WIFINET_INACT_AUTH;
    wnet_vif->vm_inact_run = WIFINET_INACT_RUN;
    wnet_vif->vm_inact_probe = WIFINET_INACT_PROBE;

    if (wnet_vif->vm_max_aid == 0) {
        wnet_vif->vm_max_aid = WIFINET_AID_DEF;

    } else if (wnet_vif->vm_max_aid > WIFINET_AID_MAX) {
        wnet_vif->vm_max_aid = WIFINET_AID_MAX;
    }
}

void wifi_mac_sta_vdetach(struct wlan_net_vif *wnet_vif)
{
    pr_debug("%s, vm_mainsta:%p\n", __func__, wnet_vif->vm_mainsta);
    wifi_mac_sta_table_rst(&wnet_vif->vm_sta_tbl, wnet_vif, true);
}

void wifi_mac_sta_attach(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *wnet_vif = NULL;

    list_for_each_entry(wnet_vif, &wifimac->wm_wnet_vifs, vm_next) {
         wifi_mac_StationTableInit(wifimac, &wnet_vif->vm_sta_tbl, "station", WIFINET_INACT_INIT);
         wifi_mac_station_init(wnet_vif);
    }

    os_timer_ex_initialize(&wifimac->wm_inact_timer, WIFINET_INACT_WAIT*1000, wifi_mac_StationTimeout, wifimac);
    os_timer_ex_start(&wifimac->wm_inact_timer);
    os_timer_ex_initialize(&wifimac->wm_free_timer, WIFINET_NODE_FREE_WAIT, wifi_mac_free_sta_delay, wifimac);
}

void
wifi_mac_StationDetach(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *wnet_vif = NULL;

    DPRINTF(AML_DEBUG_WARNING, "wifi_mac_StationDetach++ \n");

    list_for_each_entry(wnet_vif, &wifimac->wm_wnet_vifs, vm_next)
    {
        wifi_mac_clear_sta_table(&wnet_vif->vm_sta_tbl);
    }
    wifi_mac_free_sta_now(wifimac);
    os_timer_ex_del(&wifimac->wm_free_timer, CANCEL_SLEEP);
    os_timer_ex_del(&wifimac->wm_inact_timer, CANCEL_SLEEP);
}

struct sk_buff *wifi_mac_get_mgmt_frm(struct wifi_mac *wifimac, unsigned int pktlen)
{
    const unsigned int align = sizeof(unsigned int);
    struct wifi_skb_callback *cb;
    struct sk_buff *skb;
    unsigned int len;

    len = roundup(sizeof(struct wifi_htc_frame) + pktlen, 4);
    skb = wifi_mac_alloc_skb(wifimac, len + align-1);
    if (skb != NULL)
    {
        unsigned int off = ((unsigned long)os_skb_data(skb)) % align;
        if (off != 0)
            skb_reserve(skb, align - off);

        cb = (struct wifi_skb_callback *)skb->cb;
        cb->sta = NULL;
        cb->flags = 0;
        cb->hdrsize = sizeof(struct wifi_frame);
        skb_reserve(skb, sizeof(struct wifi_frame));
    }
    return skb;
}


void
wifi_mac_notify_nsta_connect(struct wifi_station *sta, int newassoc)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct net_device *dev = wnet_vif->vm_ndev;
    union iwreq_data wreq;

    if (sta == wnet_vif->vm_mainsta)
    {
        DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d\n",__func__,__LINE__);

        if (newassoc)
            netif_carrier_on(dev);
        memset(&wreq, 0, sizeof(wreq));
        WIFINET_ADDR_COPY(wreq.addr.sa_data, sta->sta_bssid);
        wreq.addr.sa_family = ARPHRD_ETHER;
        wireless_send_event(dev, SIOCGIWAP, &wreq, NULL);

       vm_cfg80211_indicate_connect(wnet_vif);

    }
    else
    {
        DPRINTF(AML_DEBUG_CONNECT, "<running> %s %d\n",__func__,__LINE__);

        memset(&wreq, 0, sizeof(wreq));
        WIFINET_ADDR_COPY(wreq.addr.sa_data, sta->sta_macaddr);
        wreq.addr.sa_family = ARPHRD_ETHER;
        wireless_send_event(dev, IWEVREGISTERED, &wreq, NULL);

        vm_cfg80211_indicate_sta_assoc(sta);
    }
}

void wifi_mac_notify_nsta_disconnect(struct wifi_station *sta, int reassoc)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct net_device *dev = wnet_vif->vm_ndev;
    union iwreq_data wreq;

    if (sta == wnet_vif->vm_mainsta)
    {
        if (!reassoc) {
            netif_carrier_off(dev);
        }

        memset(wreq.ap_addr.sa_data, 0, MAC_ADDR_LEN);
        wreq.ap_addr.sa_family = ARPHRD_ETHER;
        wireless_send_event(dev, SIOCGIWAP, &wreq, NULL);
        vm_cfg80211_indicate_disconnect(wnet_vif);

    } else {
        memset(&wreq, 0, sizeof(wreq));
        WIFINET_ADDR_COPY(wreq.addr.sa_data, sta->sta_macaddr);
        wreq.addr.sa_family = ARPHRD_ETHER;
        wireless_send_event(dev, IWEVEXPIRED, &wreq, NULL);
        vm_cfg80211_indicate_sta_disassoc(sta, 0);
    }
}

void wifi_mac_notify_scan_done(struct wlan_net_vif *wnet_vif)
{
    bool scan_abort = true;
    struct wifi_mac* wifimac = NULL;
    struct wifi_mac_scan_state *scan_state = NULL;

    if (wnet_vif == NULL) {
        ERROR_DEBUG_OUT("wnet_vif is null !\n");
        return;
    }

    if (wnet_vif->vm_wmac == NULL) {
        ERROR_DEBUG_OUT("wifimac is null !\n");
        return;
    }

    if (wnet_vif->vm_wmac->wm_scan == NULL) {
        ERROR_DEBUG_OUT("wm_scan is null !\n");
        return;
    }

    wifimac = wnet_vif->vm_wmac;
    scan_state = wifimac->wm_scan;

    if (!(scan_state->scan_StateFlags & SCANSTATE_F_CANCEL)) {
        scan_abort = false;
        vm_cfg80211_inform_bss(wnet_vif);
    }

    vm_cfg80211_indicate_scan_done(wdev_to_priv(wnet_vif->vm_wdev), scan_abort);
}

void wifi_mac_notify_mic_fail(struct wlan_net_vif *wnet_vif, const struct wifi_frame *wh, unsigned int key_index)
{
    enum nl80211_key_type key_type = 0;
    static const char *tag = "MLME-MICHAELMICFAILURE.indication";
    union iwreq_data wrqu;
    char buf[128];

    WIFINET_DPRINTF_MACADDR( AML_DEBUG_KEY, wh->i_addr2,
                             "Michael MIC verification failed <key_index %d>", key_index);
    wnet_vif->vif_sts.sts_rx_tkip_mic_err++;

    if ( WIFINET_IS_MULTICAST(wh->i_addr1) )
    {
        key_type |= NL80211_KEYTYPE_GROUP;
    }
    else
    {
        key_type |= NL80211_KEYTYPE_PAIRWISE;
    }

    cfg80211_michael_mic_failure(wnet_vif->vm_ndev,wh->i_addr2, key_type, -1, NULL, GFP_ATOMIC);

    snprintf(buf, sizeof(buf), "%s(keyid=%d addr=%s)", tag, key_index, ether_sprintf(wh->i_addr2));
    memset(&wrqu, 0, sizeof(wrqu));
    wrqu.data.length = strlen(buf);
}

const char *ether_sprintf(const unsigned char *mac) {
    static char etherbuf[18];
    snprintf(etherbuf, sizeof(etherbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return etherbuf;
}

const char *ssidie_sprintf(const unsigned char *ssidie) {
    static unsigned char lssid[WIFINET_NWID_LEN+1];
    memset(lssid, 0, WIFINET_NWID_LEN+1);
    if (ssidie && ssidie[1]>0 && ssidie[1]<=WIFINET_NWID_LEN)
        memcpy(lssid, &(ssidie[2]), ssidie[1]);
    return lssid;
}

const char *ssid_sprintf(const unsigned char *ssid, unsigned char ssid_len) {
    static unsigned char slssid[WIFINET_NWID_LEN+1];
    memset(slssid, 0, WIFINET_NWID_LEN+1);
    if (ssid && ssid_len>0 && ssid_len<=WIFINET_NWID_LEN)
        memcpy(slssid, ssid, ssid_len);
    return slssid;
}

struct wifi_station *os_skb_get_nsta(struct sk_buff *skb) {
    return ((struct wifi_skb_callback *)skb->cb)->sta;
}

void os_skb_set_nsta(struct sk_buff *skb, struct wifi_station *sta) {
    ((struct wifi_skb_callback *)skb->cb)->sta = sta;
}

int os_skb_get_tid(struct sk_buff *skb) {
    return ((struct wifi_skb_callback *)skb->cb)->u_tid;
}

void os_skb_set_tid(struct sk_buff *skb, unsigned char tid) {
    ((struct wifi_skb_callback *)skb->cb)->u_tid = tid;
}

void os_skb_set_amsdu(struct sk_buff *skb) {
    M_FLAG_SET(skb, M_AMSDU);
}

int os_skb_is_amsdu(struct sk_buff *skb) {
    if (M_FLAG_GET(skb, M_AMSDU))
        return 1;
    else
        return 0;
}

int os_skb_is_uapsd(struct sk_buff *skb) {
    if (M_FLAG_GET(skb, M_UAPSD))
        return 1;
    else
        return 0;
}

void aml_skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
    struct sk_buff *next, *prev;

    next	   = skb->next;
    prev	   = skb->prev;
    skb->next  = skb->prev = NULL;
    next->prev = prev;
    prev->next = next;
}
