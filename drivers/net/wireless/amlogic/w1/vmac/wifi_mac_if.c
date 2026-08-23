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


#include "wifi_mac_com.h"
#include "wifi_debug.h"
#include "wifi_drv_config.h"
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include "wifi_pkt_desc.h"
#include "wifi_iwpriv_cmd.h"
#include "wifi_mac_sae.h"
#include "wifi_common.h"
#include "wifi_mac_action.h"
#include "wifi_mac_tx_reg.h"

unsigned char tpc_mode = 0;
extern unsigned char g_wftx_pwrtbl_en;
extern unsigned char  host_wake_w1_fail_cnt;
extern unsigned char w1_wifi_in_insmod;

static struct net_device_stats *wifi_mac_getstats(struct net_device *);
static  int wifi_mac_change_mtu(struct net_device *, int);
static int wifi_set_mac_address(struct net_device *dev, void *addr);


const char *wifi_mac_state_name[WIFINET_S_MAX] =
{
    "INIT",
    "SCAN",
    "CONNECTING",
    "AUTH",
    "ASSOC",
    "CONNECTED"
};
int g_auto_gain_base = 0;

static struct wifi_mac wm_mac;
struct wifi_mac* wifi_mac_get_mac_handle(void)
{
    return &wm_mac;
}

static unsigned char wifi_mac_get_host_wake_status(void)
{
    return host_wake_w1_fail_cnt;
}

static unsigned char wifi_mac_clear_host_wake_status(void)
{
    host_wake_w1_fail_cnt = 0;
    return 0;
}

unsigned char wifi_mac_set_wifi_insmod_status(void)
{
    w1_wifi_in_insmod = 1;
    return 0;
}

enum hal_op_mode wifi_mac_opmode_2_halmode(enum wifi_mac_opmode opmode)
{
    enum hal_op_mode halmode;
    switch(opmode)
    {
        case WIFINET_M_STA:
            halmode = WIFI_M_STA;
            break;
        case WIFINET_M_IBSS:
            halmode = WIFI_M_IBSS;
            break;
        case WIFINET_M_HOSTAP:
            halmode = WIFI_M_HOSTAP;
            break;
        case WIFINET_M_MONITOR:
            halmode = WIFI_M_MONITOR;
            break;
        case WIFINET_M_WDS:
            halmode = WIFI_M_WDS;
            break;
        case WIFINET_M_P2P_CLIENT:
            halmode = WIFI_M_STA;
            break;
        case WIFINET_M_P2P_GO:
            halmode = WIFI_M_HOSTAP;
            break;
        default:
            halmode = WIFI_M_STA;
            break;
    }
    return halmode;
}

void *wifi_mac_alloc_ndev(unsigned int len)
{
    struct net_device *dev;
    struct wlan_net_vif *wnet_vif;

    dev = alloc_etherdev(len);
    if (dev == NULL)
        return NULL;

    wnet_vif = netdev_priv(dev);
    wnet_vif->vm_ndev = dev;

    AML_OUTPUT("<running>\n");
    return wnet_vif;
}

void wifi_mac_free_vmac(void *netif)
{
     AML_OUTPUT("<running>\n");
}

void
wifi_mac_vmac_delt(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int ret;

    if(wnet_vif->vm_state != WIFINET_S_INIT)
    {
        wifi_mac_stop(wnet_vif->vm_ndev);
    }
    KASSERT(wnet_vif->vm_state == WIFINET_S_INIT, ("wnet_vif not stopped"));

    wifi_mac_beacon_free( wnet_vif->vm_wmac , wnet_vif->wnet_vif_id);
    memset(&wifimac->wm_vendorinfo, 0, sizeof(struct wifi_mac_vendor_ie) * VENDOR_IE_MAX);
    ret = wifimac->drv_priv->drv_ops.remove_interface(wifimac->drv_priv, wnet_vif->wnet_vif_id);
    KASSERT(ret == 0, ("invalid interface id"));

    wnet_vif->wnet_vif_replaycounter++;
    wifi_mac_free_vmac(wnet_vif);

    vm_wlan_net_vif_unregister(wnet_vif);
}

void wifi_mac_sta_free(struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    void * drv_sta = sta->drv_sta;

    if(drv_sta  == NULL) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d \n",__func__,__LINE__);
        return;
    }
    wifi_mac_amsdu_nsta_free(wifimac, sta);

    wifimac->drv_priv->drv_ops.free_nsta(wifimac->drv_priv, drv_sta);
    sta->drv_sta = NULL;
}

void wifi_mac_sta_clean(struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;

    wifimac->drv_priv->drv_ops.cleanup_nsta(wifimac->drv_priv, sta->drv_sta);
}

int wifi_mac_reset(struct wifi_mac *wifimac)
{
    int error;

    error = wifimac->drv_priv->drv_ops.reset(wifimac->drv_priv);

    return error;
}

unsigned int wifi_mac_mhz2chan(unsigned int freq)
{
    unsigned int  chan = 1;

    if (freq>5000) {
        chan = (freq - 5000) / (5);

    } else if ((freq > 2407) && (freq < 2484)) {
        chan = (freq - 2407) / 5;

    } else if (freq == 2484) {
        chan = 14;

    } else {
        ASSERT(0);
    }

    return chan;
}

static void wifi_mac_rate_nsta_update(void* ieee, void* nsta, int isnew)
{
    struct wifi_station *sta = (struct wifi_station *)nsta;

    wifi_mac_rate_newassoc(sta, isnew);
}

void
wifi_mac_new_assoc(struct wifi_station *sta, int isnew)
{
    struct wifi_mac *wifimac = sta->sta_wmac;

    wifi_mac_rate_nsta_update(wifimac, sta, isnew);
    wifimac->drv_priv->drv_ops.new_assoc( wifimac->drv_priv, sta->drv_sta, isnew,
            ((sta->sta_flags & WIFINET_NODE_UAPSD)? 1: 0) );
}

void wifi_mac_scan_start(struct wifi_mac *wifimac)
{
    DPRINTF(AML_DEBUG_SCAN,"%s\n",__func__);
    wifimac->wm_syncbeacon = 0;
    wifimac->drv_priv->drv_ops.scan_start(wifimac->drv_priv);
}

void
wifi_mac_scan_end(struct wifi_mac *wifimac)
{
    DPRINTF(AML_DEBUG_SCAN,"%s\n",__func__);
    wifimac->drv_priv->drv_ops.scan_end(wifimac->drv_priv);
}

void
wifi_mac_connect_start(struct wifi_mac *wifimac)
{
    DPRINTF(AML_DEBUG_SCAN,"%s\n",__func__);
    wifimac->drv_priv->drv_ops.connect_start(wifimac->drv_priv);
}

void
wifi_mac_connect_end(struct wifi_mac *wifimac)
{
    DPRINTF(AML_DEBUG_SCAN,"%s\n",__func__);
    wifimac->drv_priv->drv_ops.connect_end(wifimac->drv_priv);
}

void wifi_mac_scan_set_gain(struct wifi_mac *wifimac, unsigned char rssi)
{
    //AML_OUTPUT("scan rssi:%d\n", rssi);
    wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, rssi);
}

void wifi_mac_set_channel_rssi(struct wifi_mac *wifimac, unsigned char rssi)
{
    unsigned char rssi_set = rssi;

    if (wifimac->bt_lk && rssi >= 218) {
        //for bt wifi coexit need set -50 gian
        wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, 226);

    } else if (wifimac->is_connect_set_gain) {
        if (rssi_set > 191) {
            rssi_set = 191;
        }
        wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, rssi_set);

    } else {
        wifimac->drv_priv->drv_ops.set_channel_rssi(wifimac->drv_priv, 174); //-82 gain
    }
}

int wifi_mac_is_in_noisy_environment(struct wifi_mac *wifimac)
{
    int ret = 0;

    if ((wifimac->wm_nrunning == 0 && wifimac->wm_scan->scan_ssid_count >= wifimac->scan_gain_thresh_unconnect)
        || (wifimac->wm_nrunning != 0 && wifimac->wm_scan->scan_ssid_count >= wifimac->scan_gain_thresh_connect)) {
        ret = 1;
    }

    return ret;
}

int wifi_mac_is_in_clear_environment(struct wifi_mac *wifimac)
{
    int ret = 0;

    if (wifimac->wm_scan->scan_ssid_count < wifimac->scan_max_gain_thresh) {
        ret = 1;
    }

    return ret;
}


/* if wifimac->wm_scan->scan_ssid_count >= UNCONNECT_MIN_GIAN_THRESHOLD/CONNECT_MIN_GIAN_THRESHOLD always use MAC_MIN_GAIN
   if wifimac->wm_scan->scan_ssid_count <  MAX_GIAN_THRESHOLD always use MAC_MAX_GAIN
   if MAX_GIAN_THRESHOLD < wifimac->wm_scan->scan_ssid_count < UNCONNECT_MIN_GIAN_THRESHOLD/CONNECT_MIN_GIAN_THRESHOLD use MAC_MID_GAIN
*/
void wifi_mac_get_channel_rssi_before_scan(struct wifi_mac *wifimac, int *rssi)
{
    int channel_set_rssi = 0;

    DPRINTF(AML_DEBUG_WARNING, "%s last rssi:%d\n", __func__, *rssi);
    channel_set_rssi = MAC_MIN_GAIN;

    switch (*rssi) {
        case MAC_MAX_GAIN:
            if (wifimac->scan_noisy_status == WIFINET_S_SCAN_ENV_CLEAR) {
                channel_set_rssi = MAC_MAX_GAIN;

            } else {
                channel_set_rssi = MAC_MIN_GAIN;
            }
            break;

        case MAC_MID_GAIN:
            if (wifimac->scan_noisy_status == WIFINET_S_SCAN_ENV_CLEAR) {
                channel_set_rssi = MAC_MAX_GAIN;

            } else if (wifimac->scan_noisy_status == WIFINET_S_SCAN_ENV_NOISE) {
                channel_set_rssi = MAC_MIN_GAIN;

            } else {
                channel_set_rssi = MAC_MID_GAIN;
            }
            break;

        case MAC_MIN_GAIN:
            if (wifimac->scan_noisy_status == WIFINET_S_SCAN_ENV_NOISE) {
                channel_set_rssi = MAC_MIN_GAIN;

            } else {
                channel_set_rssi = MAC_MID_GAIN;
            }
            break;

        default:
            break;
    }

    DPRINTF(AML_DEBUG_WARNING, "%s current rssi:%d\n", __func__, channel_set_rssi);
    *rssi = channel_set_rssi;
}

void wifi_mac_set_tx_power_accord_rssi(struct wifi_mac *wifimac, unsigned char rssi)
{
    struct hal_channel hchan;
    unsigned char power_mode = 0;
    static unsigned char rssi_increase_num = 0;
    static unsigned char rssi_diminish_num = 0;

    if (rssi <= 186) { /* -70dbm */
        rssi_diminish_num++;
        rssi_increase_num = 0;

    } else if (rssi >= 196) { /* -60dbm */
        rssi_increase_num++;
        rssi_diminish_num = 0;

    } else {
        rssi_diminish_num = 0;
        rssi_increase_num = 0;
        return;
    }

    if (rssi_diminish_num == 10 && wifimac->wm_curchan->chan_bw == 2) {
        power_mode = 2;

    } else if (rssi_increase_num == 10) {
        power_mode = 1;
    }

    if (power_mode && (power_mode != tpc_mode)) {
        hchan.channel = wifi_mac_Mhz2ieee(wifimac->wm_curchan->chan_cfreq1, 0);
        hchan.chan_bw = wifimac->wm_curchan->chan_bw;
        DPRINTF(AML_DEBUG_INFO, "%s tx_power_accord rssi:%d\n", __func__, rssi - 256);
        wifimac->drv_priv->drv_ops.set_tx_power_accord_rssi(wifimac->drv_priv, &hchan, rssi, power_mode);
        tpc_mode = power_mode;
    }

    if (rssi_diminish_num > 20) {
        rssi_diminish_num = 20;
    }

    if (rssi_increase_num > 20) {
        rssi_increase_num = 20;
    }
}


int
wifi_mac_wmm_update2hal(struct wifi_mac *wifimac, struct wlan_net_vif *wnet_vif)
{
    int ac;
    struct wmeParams *wmep;

    for (ac = 0; ac < WME_NUM_AC; ac++)
    {
        wmep = wifi_mac_wmm_chanparams(wnet_vif,wifimac, ac);
        if (wifimac->drv_priv->drv_ops.tx_wmm_queue_update(wifimac->drv_priv,
                    wnet_vif->wnet_vif_id, ac,
                    wmep->wmep_aifsn,wmep->wmep_logcwmin,
                    wmep->wmep_logcwmax,wmep->wmep_txopLimit) != 0)
        {
            return -EIO;
        }
    }
    return 0;
}


void
wifi_mac_com_ps_set_state(struct wifi_mac *wifimac,
    enum wifinet_ps_state newstate, int wnet_vif_id)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d state-> %d\n",__func__,__LINE__, newstate);
    switch (newstate)
    {
        case WIFINET_PWRSAVE_AWAKE:
            wifimac->drv_priv->drv_ops.awake(wifimac->drv_priv, wnet_vif_id);
            break;
        case WIFINET_PWRSAVE_NETWORK_SLEEP:
            wifimac->drv_priv->drv_ops.netsleep(wifimac->drv_priv, wnet_vif_id);
            break;
        case WIFINET_PWRSAVE_FULL_SLEEP:
            wifimac->drv_priv->drv_ops.fullsleep(wifimac->drv_priv, wnet_vif_id);
            break;
        default:
            break;
    }
}

int wifi_mac_build_txinfo(struct wifi_mac *wifimac, struct sk_buff *skbbuf, struct wifi_mac_tx_info *txinfo)
{
    struct wifi_station *sta =  os_skb_get_nsta(skbbuf);
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_frame *wh;
    int key_index, pktlen;
    int type, subtype;
    enum hal_key_type key_type = HAL_KEY_TYPE_CLEAR;

    wh = (struct wifi_frame *)os_skb_data(skbbuf);

    txinfo->b_mcast = WIFINET_IS_MULTICAST(wh->i_addr1);
    txinfo->b_txfrag = (wh->i_fc[1] & WIFINET_FC1_MORE_FRAG) ||
        (((le16toh(*((unsigned short *)&(wh->i_seq[0]))) >> WIFINET_SEQ_FRAG_SHIFT) & WIFINET_SEQ_FRAG_MASK) > 0);

    type = wh->i_fc[0] & WIFINET_FC0_TYPE_MASK;
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    pktlen = os_skb_get_pktlen(skbbuf);
    txinfo->mpdulen= pktlen;

    if ((wh->i_fc[1] & WIFINET_FC1_WEP) && !txinfo->b_pmf)
    {
        const struct wifi_mac_security *cip;
        struct wifi_mac_key *k;

        k = wifi_mac_security_encap(sta, skbbuf);

        if (k == NULL)
        {
            WIFINET_DPRINTF(AML_DEBUG_DEBUG, "%s", "(k == NULL) err!!");
            return -EIO;
        }
        wh = (struct wifi_frame *)os_skb_data(skbbuf);

        cip = k->wk_cipher;
        pktlen += cip->wm_header + cip->wm_trailer;
        if ((k->wk_flags & WIFINET_KEY_SWMIC) == 0)
        {
            if ( ! txinfo->b_txfrag)
                pktlen += cip->wm_miclen;
            else
            {
                if (cip->wm_cipher != WIFINET_CIPHER_TKIP)
                    pktlen += cip->wm_miclen;
            }
        }

        switch (cip->wm_cipher)
        {
            case WIFINET_CIPHER_WEP:
                key_type = HAL_KEY_TYPE_WEP;
                break;
            case WIFINET_CIPHER_TKIP:
                key_type = HAL_KEY_TYPE_TKIP;
                pktlen += DP_SEC_TKIP_MIC_LEN;
                break;
            case WIFINET_CIPHER_AES_OCB:
            case WIFINET_CIPHER_AES_CCM:
                key_type = HAL_KEY_TYPE_AES;
                break;
#ifdef CONFIG_WAPI
            case WIFINET_CIPHER_WPI:
                key_type = HAL_KEY_TYPE_WPI;
                break;
#endif //#ifdef CONFIG_WAPI
            default:
                key_type = HAL_KEY_TYPE_CLEAR;
        }
        key_index = k->wk_keyix;
    }
    else
    {
        key_index = HAL_TXKEYIX_INVALID;
    }
    pktlen += WIFINET_CRC_LEN;

    txinfo->packetlen = pktlen;
    txinfo->key_index = key_index;
    txinfo->key_type = key_type;
    txinfo->tid_index =  os_skb_get_tid(skbbuf);

    if (wifi_mac_com_has_flag(wifimac, WIFINET_F_SHPREAMBLE) &&
        !wifi_mac_com_has_flag(wifimac, WIFINET_F_USEBARKER) &&
        wifi_macnsta_has_cap(sta, WIFINET_CAPINFO_SHORT_PREAMBLE))
    {
        txinfo->shortPreamble = 1;
    }

    switch (type)
    {
        case WIFINET_FC0_TYPE_MGT:
            txinfo->b_mgmt = 1;
            txinfo->queue_id = wifimac->wm_mng_qid;
            break;

        case WIFINET_FC0_TYPE_CTL:
            switch (subtype)
            {
                case WIFINET_FC0_SUBTYPE_PS_POLL:
                    txinfo->b_PsPoll = 1;
                    break;

                case WIFINET_FC0_SUBTYPE_BAR:
                    txinfo->b_BarPkt = 1;
                    break;

                default:
                    break;
            }
            txinfo->queue_id = wifimac->wm_mng_qid;;
            break;

        case WIFINET_FC0_TYPE_DATA:
            txinfo->b_datapkt = 1;
            txinfo->b_uapsd =  os_skb_is_uapsd(skbbuf);

            if (wh->i_fc[0] & WIFINET_FC0_SUBTYPE_QOS)
            {
                int ac =  os_skb_get_priority(skbbuf);
                txinfo->b_qosdata = 1;

                txinfo->queue_id = wifimac->wm_ac2q[ac & 0x3];
                if (wifi_mac_wmm_chanparams(wnet_vif,wifimac, ac)->wmep_noackPolicy)
                    txinfo->b_noack = 1;

            }
            else
            {
                txinfo->queue_id = wifimac->wm_noqos_legacy_qid;
                os_skb_set_priority(skbbuf, WME_AC_BE);
            }

            if (txinfo->b_uapsd)
            {
                txinfo->queue_id = wifimac->wm_uapsd_qid;
            }

            if (txinfo->b_mcast)
            {
                txinfo->queue_id = wifimac->wm_mcast_qnum;
            }

            if (!txinfo->b_mcast && WIFINET_NODE_USEAMPDU(sta))
            {
                if (WIFINET_QOS_HAS_SEQ(wh))
                {
                    txinfo->ht = 1;
                }
            }

            if ((subtype == WIFINET_FC0_SUBTYPE_NODATA)
                    || (subtype == WIFINET_FC0_SUBTYPE_QOS_NULL))
            {
                txinfo->queue_id = wifimac->wm_mng_qid;
                txinfo->ht = 0;
                txinfo->b_nulldata = 1;
            }

            if (M_FLAG_GET(skbbuf, M_CHECKSUMHW))
            {
                txinfo->b_hwchecksum = 1;
            }
            if (M_FLAG_GET(skbbuf, M_TKIPMICHW))
            {
                txinfo->b_hwtkipmic= 1;
            }
            break;
        default:
            return -EIO;
    }

    txinfo->wnet_vif_id = wnet_vif->wnet_vif_id;
    if (((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) || (wnet_vif->vm_opmode == WIFINET_M_P2P_GO))
        && (wnet_vif->vm_ps_sta != 0))
        txinfo->ps = 1;

    txinfo->b_buffered = 0;
    if (txinfo->b_mcast)
    {
        txinfo->b_noack = 1;
    }
    else if (pktlen > wnet_vif->vm_rtsthreshold)
    {
        txinfo->b_rsten = 1;
    }

    return 0;
}

void wifi_mac_tx_addba_check(struct wifi_station *sta, unsigned char tid_index)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    if (WIFINET_NODE_USEAMPDU(sta)
        && wifimac->drv_priv->drv_ops.check_aggr(wifimac->drv_priv, sta->drv_sta, tid_index)
        && (wnet_vif->vm_opmode != WIFINET_M_HOSTAP || softap_get_sta_num(wnet_vif) < 2))
    {
        struct wifi_mac_action_mgt_args actionargs;
        DPRINTF(AML_DEBUG_WARNING,"<running> %s %d tid_index = %d \n", __func__,__LINE__,tid_index);

        actionargs.category = AML_CATEGORY_BACK;
        actionargs.action = WIFINET_ACTION_BA_ADDBA_REQUEST;
        actionargs.arg1 = tid_index;
        actionargs.arg2 = WME_MAX_BA;
        actionargs.arg3 = 0;
        wifi_mac_send_action(sta, (void *)&actionargs);
    }
}

int wifi_mac_tx_send(struct sk_buff *skbbuf)
{
    struct wifi_station *sta = os_skb_get_nsta(skbbuf);
    struct wlan_net_vif *wnet_vif = netdev_priv(skbbuf->dev);
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct sk_buff *next_wbuf;
    unsigned char is_amsdu_support = 0;
    struct wifi_mac_tx_info *txinfo = NULL;
    struct wifi_mac_pkt_info *mac_pkt_info;

    txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    ASSERT(sizeof(struct wifi_mac_tx_info) <= OS_SKB_CB_MAXLEN);

    mac_pkt_info= &txinfo->ptxdesc->drv_pkt_info.pkt_info[0];
    if (mac_pkt_info->b_eap == 0 && mac_pkt_info->b_arp == 0 && mac_pkt_info->b_dhcp == 0) {
        wifi_mac_tx_addba_check(sta, os_skb_get_tid(skbbuf));
    }

    is_amsdu_support = wifimac->drv_priv->drv_ops.get_amsdu_supported(wifimac->drv_priv, sta->drv_sta, os_skb_get_tid(skbbuf));
    if (is_amsdu_support && !mac_pkt_info->b_dhcp && !mac_pkt_info->b_eap && !mac_pkt_info->b_arp) {
        skbbuf = wifi_mac_amsdu_send(skbbuf);
        if (skbbuf == NULL) {
            return NETDEV_TX_OK;
        }
    }

    wnet_vif->vif_sts.sts_tx_non_amsdu_in_msdu[os_skb_get_tid(skbbuf)]++;

    skbbuf = wifi_mac_encap(sta, skbbuf);
    if (skbbuf == NULL) {
        DPRINTF(AML_DEBUG_ANY,"<running> %s %d \n",__func__,__LINE__);
        goto bad;
    }

    while (skbbuf != NULL) {
        txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
        memset(&txinfo->wnet_vif_id, 0, sizeof(struct wifi_mac_tx_info)-sizeof(struct wifi_skb_callback)-sizeof(struct drv_txdesc *));
        next_wbuf = os_skb_next(skbbuf);

        if (wifi_mac_build_txinfo(wifimac, skbbuf, txinfo) != 0) {
            ERROR_DEBUG_OUT("<running>\n");
            goto bad;
        }

        wnet_vif->vif_sts.sts_tx_non_amsdu_out_msdu[os_skb_get_tid(skbbuf)]++;
        if (wifimac->drv_priv->drv_ops.tx_start(wifimac->drv_priv, skbbuf) != 0) {
            skbbuf = next_wbuf;
            while (skbbuf != NULL) {
                next_wbuf = os_skb_next(skbbuf);
                os_skb_next(skbbuf) = NULL;
                wifi_mac_complete_wbuf(skbbuf, 0);
                skbbuf = next_wbuf;
            }
            return NETDEV_TX_OK;
        }

        skbbuf = next_wbuf;
    }

    return NETDEV_TX_OK;

bad:
    DPRINTF(AML_DEBUG_ANY,"<%s> %s %d drop frame\n", VMAC_DEV_NAME(sta->sta_wnet_vif),__func__,__LINE__);

    while (skbbuf != NULL) {
        next_wbuf =  os_skb_next(skbbuf);
        os_skb_next(skbbuf) = NULL;
        wifi_mac_complete_wbuf(skbbuf, 0);

        wnet_vif->vm_devstats.tx_dropped++;
        wnet_vif->vif_sts.sts_tx_non_amsdu_drop_msdu[os_skb_get_tid(skbbuf)]++;
        skbbuf = next_wbuf;
    }

    return NETDEV_TX_OK;
}

int wifi_mac_tx_mgmt_frm(struct wifi_mac *wifimac, struct sk_buff *skbbuf)
{
    int error = 0;
    struct wifi_mac_tx_info *txinfo = NULL;
    struct wifi_frame *wh;
    struct wifi_station *sta;
    unsigned char pkt_len = 0;
    unsigned char is_robust_mgmt = 0;

    if (aml_wifi_is_enable_rf_test()) {
        wifi_mac_complete_wbuf(skbbuf, 0);
        return error;
    }

    txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    sta = txinfo->cb.sta;
    ASSERT(sizeof(struct wifi_mac_tx_info) <= OS_SKB_CB_MAXLEN);
    memset(&txinfo->wnet_vif_id, 0, sizeof(struct wifi_mac_tx_info)-sizeof(struct wifi_skb_callback)-sizeof(struct drv_txdesc *));
    wifi_mac_pwrsave_txpre(skbbuf);
    wh = (struct wifi_frame *)os_skb_data(skbbuf);
    is_robust_mgmt = is_robust_management_frame(wh);

    if ((sta->sta_flags_ext & WIFINET_NODE_MFP) && is_robust_mgmt && sta->pmf_key.wk_keylen) {
        if (WIFINET_IS_MULTICAST(wh->i_addr1)) {
            if (sta->pmf_key.wk_key_type == WIFINET_CIPHER_AES_CMAC) {
                pkt_len = MME_IE_LENGTH_CMAC;

            } else {
                pkt_len = MME_IE_LENGTH;
            }

            os_skb_put(skbbuf, pkt_len);
            if (wifi_mac_add_mmie(sta, skbbuf)) {
                goto fail;
            }
        }

        *(unsigned short *)&wh->i_seq[0] = htole16(sta->sta_txseqs[0] << WIFINET_SEQ_SEQ_SHIFT);
        sta->sta_txseqs[0]++;
        txinfo->b_pmf = 1;
        wh->i_fc[1] |= WIFINET_FC1_WEP;

        PN_LOCK();//for PN
        if (!wifi_mac_sw_encrypt(sta, skbbuf)) {
            PN_UNLOCK();
            goto fail;
        }
        PN_UNLOCK();
    }

    error = wifi_mac_build_txinfo(wifimac, skbbuf, txinfo);
    if (!error) {
        error = wifimac->drv_priv->drv_ops.tx_start(wifimac->drv_priv, skbbuf);
        return error;
    }

fail:
    wifi_mac_complete_wbuf(skbbuf, 0);
    return error;
}

unsigned int
wifi_mac_all_txq_all_qcnt(struct wifi_mac *wifimac)
{
    int ac, qqcnt = 0;

    for (ac = WME_AC_BE; ac <= WME_AC_VO; ac++)
    {
        qqcnt += wifimac->drv_priv->drv_ops.txlist_all_qcnt(wifimac->drv_priv, wifimac->wm_ac2q[ac]);
    }
    qqcnt += wifimac->drv_priv->drv_ops.txlist_all_qcnt(wifimac->drv_priv, wifimac->wm_uapsd_qid);
    qqcnt += wifimac->drv_priv->drv_ops.txlist_all_qcnt(wifimac->drv_priv, wifimac->wm_noqos_legacy_qid);
    qqcnt += wifimac->drv_priv->drv_ops.txlist_all_qcnt(wifimac->drv_priv, wifimac->wm_mng_qid);
    qqcnt += wifimac->drv_priv->drv_ops.txlist_all_qcnt(wifimac->drv_priv, wifimac->wm_mcast_qnum);
    return qqcnt;
}


static void
wifi_mac_tx_complete(struct wlan_net_vif *wnet_vif, struct sk_buff *skbbuf, struct wifi_mac_tx_status *ts)
{
    if (skbbuf == NULL) {
        return;
    }

    if ((wnet_vif != NULL) && (ts->ts_flags & WIFINET_TX_ERROR)) {
        wnet_vif->vm_devstats.tx_errors++;
    }

    wifi_mac_free_skb(skbbuf);
}

static void
wifi_mac_tx_status(void* nsta, struct wifi_mac_tx_status * ts)
{
    struct wifi_station *sta = (struct wifi_station *)nsta;
    struct wlan_net_vif *wnet_vif;

    if (sta == NULL) {
        return;
    }
    wnet_vif = sta->sta_wnet_vif;

    if ((wnet_vif != NULL) && (wnet_vif->vm_mainsta != NULL))
    {
        sta->sta_rstamp = jiffies;

        if (ts->ts_ratekbps > 0)
        {
            sta->sta_last_txrate = ts->ts_ratekbps;
            wnet_vif->vm_mainsta->sta_last_txrate = ts->ts_ratekbps;
        }
    }
}

static void
wifi_mac_updateslot_ex(SYS_TYPE param1,SYS_TYPE param2,SYS_TYPE param3,
        SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)param1;
    int slottime;

    slottime = (wifi_mac_com_has_flag(wifimac, WIFINET_F_SHSLOT)) ?
        HAL_SLOT_TIME_9 : HAL_SLOT_TIME_20;
    wifimac->drv_priv->drv_ops.set_slottime(wifimac->drv_priv, slottime);
}


void wifi_mac_updateslot(struct wifi_mac *wifimac)
{
    wifi_mac_add_work_task(wifimac,wifi_mac_updateslot_ex, NULL,
            (SYS_TYPE)wifimac, 0, 0, 0, 0);
}

void
wifi_mac_update_protmode(struct wifi_mac *wifimac)
{
    enum prot_mode mode = PROT_M_NONE;

    if (wifi_mac_com_has_flag(wifimac, WIFINET_F_USEPROT))
    {
        if (wifimac->wm_protmode == WIFINET_PROT_RTSCTS)
            mode = PROT_M_RTSCTS;
        else if (wifimac->wm_protmode == WIFINET_PROT_CTSONLY)
            mode = PROT_M_CTSONLY;
    }
    wifimac->drv_priv->drv_ops.set_protmode(wifimac->drv_priv, mode);
}

void wifi_mac_set_ampduparams(struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;

    wifimac->drv_priv->drv_ops.set_ampdu_params(wifimac->drv_priv, sta->drv_sta,
        sta->sta_maxampdu, sta->sta_mpdudensity);
}

void wifi_mac_addba_req_setup(struct wifi_station *sta, unsigned char tid_index,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short *batimeout, struct wifi_mac_ba_seqctrl *basequencectrl,
    unsigned short buffersize)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    wifimac->drv_priv->drv_ops.addba_request_setup(wifimac->drv_priv, sta->drv_sta, tid_index, baparamset,
        batimeout, basequencectrl, buffersize);
}

void wifi_mac_addba_rsp_setup(struct wifi_station *sta, unsigned char tid_index)
{
    struct wifi_mac *wifimac = sta->sta_wmac;

    wifimac->drv_priv->drv_ops.addba_response_setup(wifimac->drv_priv, sta->drv_sta, tid_index);
}


int wifi_mac_addba_req(struct wifi_station *sta, unsigned char dialogtoken,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short batimeout, struct wifi_mac_ba_seqctrl basequencectrl)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    return wifimac->drv_priv->drv_ops.addba_request_process(wifimac->drv_priv,
        sta->drv_sta, dialogtoken, baparamset, batimeout, basequencectrl);
}

void wifi_mac_addba_rsp(struct wifi_station *sta, unsigned short statuscode,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short batimeout)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    wifimac->drv_priv->drv_ops.addba_response_process(wifimac->drv_priv,
        sta->drv_sta, statuscode, baparamset, batimeout);
}

void wifi_mac_delba(struct wifi_station *sta, struct wifi_mac_delba_parameterset *delbaparamset,
    unsigned short reasoncode)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    wifimac->drv_priv->drv_ops.delba_process(wifimac->drv_priv, sta->drv_sta, delbaparamset, reasoncode);
}

void wifi_mac_addba_set_rsp(struct wifi_station *sta, unsigned char tid_index, unsigned short statuscode)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    wifimac->drv_priv->drv_ops.set_addbaresponse(wifimac->drv_priv, sta->drv_sta, tid_index, statuscode);
}

int wifi_mac_set_country(struct wifi_mac *wifimac, char *isoName)
{
    int error;

    error = wifimac->drv_priv->drv_ops.set_country(wifimac->drv_priv, isoName);
    if (!error)
        wifi_mac_update_country_chan_list(wifimac);

    return error;
}

void wifi_mac_get_country(struct wifi_mac *wifimac, struct _wifi_mac_country_iso *ciso)
{
    wifimac->drv_priv->drv_ops.get_current_country(wifimac->drv_priv, (char *)ciso);
}

int wifi_mac_set_quiet(struct wifi_station *sta, unsigned char *quiet_elm)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wifi_mac_quiet_ie *quiet = (struct wifi_mac_quiet_ie *)quiet_elm;

    return wifimac->drv_priv->drv_ops.set_quiet(wifimac->drv_priv, quiet->period, quiet->period,
        quiet->offset + quiet->tbttcount * sta->sta_wnet_vif->vm_bcn_intval, 1);
}

static void wifi_mac_update_txpower(void * ieee,
    unsigned short cfg_txpowlimit, unsigned short txpowlevel)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    struct wlan_net_vif *wnet_vif;

    wifimac->wm_txpowlimit = cfg_txpowlimit;

    wifi_mac_enumerate_wnet_vifs(wnet_vif, wifimac)
    {
        struct wifi_station *sta;

        sta = wnet_vif->vm_mainsta;
        ASSERT(sta);
        if (sta)
        {
            wifi_macnsta_set_txpower(sta, txpowlevel);
        }
    }
}

struct wlan_net_vif *
wifi_mac_get_wnet_vif_by_vid(struct wifi_mac *wifimac, int wnet_vif_id)
{
    struct wlan_net_vif *wnet_vif = NULL;

    list_for_each_entry(wnet_vif, &wifimac->wm_wnet_vifs, vm_next)
    {
        if (wnet_vif->wnet_vif_id == wnet_vif_id)
        {
            break;
        }
    }
    return wnet_vif;
}

#if 0
static int16_t
wifi_mac_get_noisefloor(struct wifi_mac *wifimac, struct wifi_channel *chan)
{
    return wifimac->drv_priv->drv_ops.get_noisefloor(wifimac->drv_priv,
            chan->chan_cfreq1,  chan->chan_flags);
}
#endif

void
wifi_mac_config(struct wifi_mac *wifimac, int paramid,int data)
{
    wifimac->drv_priv->drv_ops.drv_set_config_param(wifimac->drv_priv,
            (enum cip_param_id)paramid,data);
}

int wifi_mac_get_config(struct wifi_mac *wifimac, int paramid)
{
    return wifimac->drv_priv->drv_ops.drv_get_config_param(wifimac->drv_priv, (enum cip_param_id)paramid);
}

static void
wifi_mac_notify_qstatus(void * ieee, unsigned short qqcnt)
{
    wifi_mac_notify_queue_status(NET80211_HANDLE(ieee), qqcnt);
}

void wifi_mac_roaming_trigger(struct wlan_net_vif * wnet_vif)
{
    int ret_startscan =0;

    if (wnet_vif->vm_chan_roaming_scan_flag == 1) {
        return;
    }

    if (wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans_cnt <= 1) {
        ERROR_DEBUG_OUT("Can't find roaming candidate channel\n");
        return;
    }

    if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())) {
        ERROR_DEBUG_OUT("ap sta concurrent, not trigger roaming\n");
        return;
    }

    wnet_vif->vm_flags &= ~WIFINET_F_DESBSSID;
    wnet_vif->vm_chan_roaming_scan_flag = 1;
    wnet_vif->vm_chan_roaming_scan_count = 0;
    wnet_vif->vm_wmac->wm_scan->roaming_full_scan = 0;

    ret_startscan = wifi_mac_start_scan(wnet_vif,WIFINET_SCANCFG_USERREQ|WIFINET_SCANCFG_ACTIVE| WIFINET_SCANCFG_FLUSH|WIFINET_SCANCFG_NOPICK, 1,
         &wnet_vif->vm_wmac->wm_scan->roaming_ssid);

    if (ret_startscan == 0) {
        wnet_vif->vm_chan_roaming_scan_flag = 0;
        DPRINTF(AML_DEBUG_ANY, "%s %d start scan fail, abort\n",__func__, __LINE__);
    }
}

static void wifi_mac_roaming_check(  struct wifi_mac *wifimac, struct wifi_station *sta)
{
    static int roaming_cnt = 0;
    int roaming_threshold = wifimac->roaming_threshold_5g;

    if (WIFINET_IS_CHAN_2GHZ(wifimac->wm_curchan)) {
        roaming_threshold = wifimac->roaming_threshold_2g;
    }

    if (sta->sta_avg_bcn_rssi  < roaming_threshold) {
        if (++roaming_cnt > ROAMING_TRIGER_COUNT && time_before((wifimac->wm_lastroaming + (5 * HZ)), jiffies)) {
            pr_debug("Rssi[%d] < Threshold[%d]  trigger roaming \n", sta->sta_avg_bcn_rssi, roaming_threshold);
            roaming_cnt = 0;
            wifimac->wm_lastroaming = jiffies;
            wifi_mac_roaming_trigger(sta->sta_wnet_vif);
        }
    }
    else {
        roaming_cnt = 0;
    }
}

static int wifi_mac_average_rssi(int rssi)
{
    int rssi_n = 0;
    int count = 0;
    int rssi_avg = 0;
    static int rssi_value[10] = {0};
    static int rssi_num = 0;

    for (count = 0; count < 10; count++) {
        if (rssi_value[count] == 0) {
            rssi_value[count] = rssi;
            rssi_avg = (rssi_avg + rssi_value[count]) / (count + 1);
            return rssi_avg;
        } else {
            rssi_avg += rssi_value[count];
        }
    }

    rssi_avg = 0;
    for (rssi_n = 0; rssi_n < (count - 1); rssi_n++) {
        rssi_value[rssi_n] = rssi_value[rssi_n + 1];
        rssi_avg += rssi_value[rssi_n];
    }

    if (rssi_num < 5) {
        if ((rssi - rssi_value[count - 1]) > 10 || (rssi_value[count - 1] - rssi) > 10) {
                rssi_num++;
        } else {
            rssi_num = 0;
            rssi_value[count - 1] = rssi;
        }
    } else {
        rssi_num = 0;
        rssi_value[count - 1] = rssi;
    }
    rssi_avg = (rssi_avg + rssi_value[count - 1]) / count;

    return rssi_avg;
}

int wifi_mac_rx_complete(void *ieee,struct sk_buff *skbbuf, struct wifi_mac_rx_status *rs)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    struct wifi_station *sta;
    struct wlan_net_vif *wnet_vif;
    struct wifi_frame *wh;
    struct wifi_mac_rx_cb *rx_cb;
    int type;
    struct wifi_mac_ops *net_ops;
    unsigned char cur_snr;

    int frame_type, frame_subtype;
    static unsigned char recv_beacon_count = 0;

    if (os_skb_get_pktlen(skbbuf) < WIFINET_MIN_LEN)
    {
        DPRINTF(AML_DEBUG_RECV, "%s: short packet %d\n", __func__,  os_skb_get_pktlen(skbbuf));
        {
            wh = (struct wifi_frame *)os_skb_data(skbbuf);
            frame_type = wh->i_fc[0] & WIFINET_FC0_TYPE_MASK;
            frame_subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d len=%d too min, frame_type=0x%x frame_subtype=0x%x\n",
                __func__, __LINE__, os_skb_get_pktlen(skbbuf), frame_type, frame_subtype);
        }
        os_skb_free(skbbuf);
        return -1;
    }

    wh = (struct wifi_frame *)os_skb_data(skbbuf);
    frame_type = wh->i_fc[0] & WIFINET_FC0_TYPE_MASK;
    frame_subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;
    rx_cb = (struct wifi_mac_rx_cb *)os_skb_cb(skbbuf);
    rx_cb->keyid = rs->rs_keyid;
    rx_cb->wnet_vif_id = rs->rs_wnet_vif_id;

    sta = wifi_mac_find_rx_sta(wifimac, (struct wifi_mac_frame_min *)os_skb_data(skbbuf), rs->rs_wnet_vif_id);
    if (sta == NULL) {
        /*sta is null, this is first frame except probe req */
        return wifi_mac_input_all(wifimac, skbbuf, rs);
    }
    sta->cur_fratype = wh->i_fc[0];

    if ((frame_type == WIFINET_FC0_TYPE_MGT) && (frame_subtype == WIFINET_FC0_SUBTYPE_AUTH)
        && !WIFINET_ADDR_EQ(sta->sta_bssid, wh->i_addr3))
    {
        /*sta send auth to other ap, we shall free this sta buffer, if we have allocated. */
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        return wifi_mac_input_all(wifimac, skbbuf, rs);
    }

    wnet_vif = sta->sta_wnet_vif;
    if (WIFINET_IS_DATA(wh)) {
        if (rs->rs_datarate > 0) {
            sta->sta_last_rxrate = rs->rs_datarate;
            sta->sta_last_rx_vendor_rate = rs->rs_vendor_rate;
            sta->sta_last_rx_bw = rs->rs_bw;
        }
    }

    sta->sta_avg_rssi = (sta->sta_avg_rssi + rs->rs_rssi * 3) / 4;
    if (WIFINET_IS_BEACON(wh))
    {
        rs->rs_rssi = wifi_mac_average_rssi(rs->rs_rssi);
        sta->sta_avg_bcn_rssi =  (sta->sta_avg_bcn_rssi + (rs->rs_rssi -256) * 3) / 4; //dbm

        if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_state == WIFINET_S_CONNECTED)) {
            if (!(wifimac->wm_flags & WIFINET_F_SCAN)) {
                recv_beacon_count++;
                if (recv_beacon_count == 20) {
                    recv_beacon_count = 0;
                    if ((sta->sta_avg_bcn_rssi - g_auto_gain_base) > 5 || (g_auto_gain_base - sta->sta_avg_bcn_rssi) > 5) {
                        g_auto_gain_base = sta->sta_avg_bcn_rssi;
                        wifi_mac_set_channel_rssi(wifimac, (unsigned char)(g_auto_gain_base));
                    }
                }

                cur_snr = wifimac->drv_priv->drv_ops.get_snr() / 4;
                if (cur_snr != 0) {
                    sta->sta_avg_snr = (sta->sta_avg_snr * 3 + cur_snr) / 4;
                }
            }

            if (g_wftx_pwrtbl_en != 2) {
                wifi_mac_set_tx_power_accord_rssi(wifimac, (unsigned char)(sta->sta_avg_bcn_rssi));
            }
        }
    }

    net_ops = wifimac->drv_priv->net_ops;
    /* AP check triggers */
    net_ops->wifi_mac_ap_chk_uapsd_trig(wifimac->drv_priv->wmac, skbbuf, sta,rs);
    /* STA check more data and eosp */
    net_ops->wifi_mac_sta_check_ps_end(wifimac->drv_priv->wmac, skbbuf, sta, rs);

    /*check if need to roaming*/
    if (wifimac->wm_roaming == WIFINET_ROAMING_BASIC) {
        if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_state == WIFINET_S_CONNECTED)
            && (sta == wnet_vif->vm_mainsta)){
            wifi_mac_roaming_check(wifimac,sta);
        }
    }

    /*process mpdu in ampdu  or pwr mgt frames*/
    type = wifimac->drv_priv->drv_ops.rx_proc_frame(wifimac->drv_priv, sta->drv_sta, skbbuf, rs);

    return type;
}

static void wifi_mac_free_txamsdu_queue(void * ieee, unsigned char vid)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    wifi_mac_txamsdu_free_all(wifimac, vid);
}

unsigned int wifi_mac_sta_get_txrate(struct wifi_station *sta)
{
    unsigned int rate = 0;

    if (sta != NULL) {
        rate = sta->sta_last_txrate;
    }
    return rate;
}

unsigned int wifi_mac_register_behindTask(struct wifi_mac *wifimac, void *func)
{
    return wifimac->drv_priv->drv_ops.Low_register_behindTask(wifimac->drv_priv,func);
}

unsigned int wifi_mac_call_task(struct wifi_mac *wifimac,
        SYS_TYPE taskid,SYS_TYPE param1)
{
    return wifimac->drv_priv->drv_ops.Low_callRegisteredTask(wifimac->drv_priv, taskid,param1);
}

unsigned int wifi_mac_add_work_task(struct wifi_mac *wifimac,void *func,
    void *func_cb,SYS_TYPE param1, SYS_TYPE param2,SYS_TYPE param3,
    SYS_TYPE param4,SYS_TYPE param5)
{
    return wifimac->drv_priv->drv_ops.Low_addDHWorkTask(wifimac->drv_priv,
            func, func_cb, param1, param2, param3, param4, param5);
}


void wifi_mac_init_ops (struct wifi_mac* wmac)
{
    wmac->wmac_ops.wifi_mac_mac_entry = wifi_mac_entry;
    wmac->wmac_ops.wifi_mac_setup_channel_list = wifi_mac_chan_setup;
    wmac->wmac_ops.wifi_mac_update_txpower = wifi_mac_update_txpower;
    wmac->wmac_ops.wifi_mac_update_beacon = wifi_mac_update_beacon;
    wmac->wmac_ops.wifi_mac_notify_qstatus = wifi_mac_notify_qstatus;
    wmac->wmac_ops.wifi_mac_tx_complete = wifi_mac_tx_complete;
    wmac->wmac_ops.wifi_mac_tx_status = wifi_mac_tx_status;
    wmac->wmac_ops.wifi_mac_rx_complete = wifi_mac_rx_complete;
    wmac->wmac_ops.wifi_mac_input = wifi_mac_input;
    wmac->wmac_ops.wifi_mac_ChangeChannel = wifi_mac_ChangeChannel;
    wmac->wmac_ops.wifi_mac_rate_init = wifi_mac_rate_init;
    wmac->wmac_ops.wifi_mac_rate_nsta_update = wifi_mac_rate_nsta_update;
    wmac->wmac_ops.wifi_mac_free_txamsdu_queue = wifi_mac_free_txamsdu_queue;
    wmac->wmac_ops.wifi_mac_ap_chk_uapsd_trig = wifi_mac_pwrsave_chk_uapsd_trig;
    wmac->wmac_ops.wifi_mac_uapsd_eosp_indicate = wifi_mac_pwrsave_eosp_indicate;

    wmac->wmac_ops.wifi_mac_sta_check_ps_end = wifi_mac_pwrsave_check_ps_end;
    wmac->wmac_ops.wifi_mac_rate_ratmod_attach = wifi_mac_rate_ratmod_attach;
    wmac->wmac_ops.wifi_mac_rate_ratmod_detach = wifi_mac_rate_ratmod_detach;

    wmac->wmac_ops.wifi_mac_create_vmac = wifi_mac_create_vmac;
    wmac->wmac_ops.wifi_mac_device_ip_config = wifi_mac_device_ip_config;
    wmac->wmac_ops.wifi_mac_tbtt_handle = wifi_mac_tbtt_handle;
    wmac->wmac_ops.wifi_mac_channel_switch_complete = wifi_mac_channel_switch_complete;
    wmac->wmac_ops.wifi_mac_get_netif_cfg = wifi_mac_get_netif_cfg;
    wmac->wmac_ops.wifi_mac_mac_exit = wifi_mac_mac_exit;
    wmac->wmac_ops.wifi_mac_top_sm = wifi_mac_top_sm;
    wmac->wmac_ops.os_skb_get_nsta = os_skb_get_nsta;
    wmac->wmac_ops.os_skb_get_tid = os_skb_get_tid;
    wmac->wmac_ops.wifi_mac_notify_mic_fail = wifi_mac_notify_mic_fail;
    wmac->wmac_ops.wifi_mac_send_qosnulldata = wifi_mac_send_qosnulldata;

  //  wmac->wmac_ops.vm_p2p_client_cancle_noa = vm_p2p_client_cancle_noa,
  //  wmac->wmac_ops.vm_p2p_go_cancle_noa = vm_p2p_go_cancle_noa,

    wmac->wmac_ops.wifi_mac_pwrsave_fullsleep = wifi_mac_pwrsave_fullsleep;
    wmac->wmac_ops.wifi_mac_pwrsave_is_sta_fullsleep = wifi_mac_pwrsave_is_sta_fullsleep;
    wmac->wmac_ops.wifi_mac_pwrsave_is_sta_sleeping = wifi_mac_pwrsave_is_sta_sleeping;
    wmac->wmac_ops.wifi_mac_pwrsave_is_wnet_vif_fullsleep = wifi_mac_pwrsave_is_wnet_vif_fullsleep;
    wmac->wmac_ops.wifi_mac_pwrsave_is_wnet_vif_sleeping = wifi_mac_pwrsave_is_wnet_vif_sleeping;
    wmac->wmac_ops.wifi_mac_pwrsave_networksleep = wifi_mac_pwrsave_networksleep;
    wmac->wmac_ops.wifi_mac_pwrsave_psqueue_enqueue = wifi_mac_pwrsave_psqueue_enqueue;
    wmac->wmac_ops.wifi_mac_pwrsave_psqueue_flush = wifi_mac_pwrsave_psqueue_flush;
    wmac->wmac_ops.wifi_mac_pwrsave_sta_trigger = wifi_mac_pwrsave_sta_trigger;
    wmac->wmac_ops.wifi_mac_buffer_txq_enqueue = wifi_mac_buffer_txq_enqueue;
    wmac->wmac_ops.wifi_mac_buffer_txq_send_pre = wifi_mac_buffer_txq_send_pre;
    wmac->wmac_ops.wifi_mac_pwrsave_wakeup_for_tx = wifi_mac_pwrsave_wakeup_for_tx;
    wmac->wmac_ops.wifi_mac_pwrsave_wkup_and_NtfyAp = wifi_mac_pwrsave_wkup_and_NtfyAp;
    wmac->wmac_ops.wifi_mac_notify_ap_success = wifi_mac_notify_ap_success;

    wmac->wmac_ops.wifi_mac_get_sta = wifi_mac_get_sta;
    wmac->wmac_ops.wifi_mac_sta_attach = wifi_mac_sta_attach;

    wmac->wmac_ops.wifi_mac_beacon_miss = wifi_mac_process_beacon_miss;
    wmac->wmac_ops.wifi_mac_get_sts = wifi_mac_get_sts;
    wmac->wmac_ops.wifi_mac_process_tx_error = wifi_mac_process_tx_error;
    wmac->wmac_ops.wifi_mac_forward_txq_enqueue = wifi_mac_forward_txq_enqueue;
}


void wifi_mac_set_txpower_limit(struct wifi_mac *wifimac, unsigned int limit,
                                unsigned short txpowerdb)
{
    wifimac->drv_priv->drv_ops.drv_set_txPwrLimit(wifimac->drv_priv, limit, txpowerdb);
}


int wifi_mac_get_netif_cfg(void * ieee)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);

    return wifimac->wm_dev_flags;
}

int wifi_mac_netdev_open(void * ieee)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    int error =0;

    /*call ha layer funct to set fpga info, init rf and set channel. */
    error = wifimac->drv_priv->drv_ops.drv_open(wifimac->drv_priv);
    if (error) {
        ERROR_DEBUG_OUT("err= %d \n",error);
        return error;
    }

    wifimac->wm_dev_flags |= NETCOM_NETIF_IFF_UP_RUNNING;
    if (wifimac->wm_curchan != WIFINET_CHAN_ERR && wifimac->wm_curchanmaxpwr == 0)
        wifimac->wm_curchanmaxpwr = wifimac->wm_curchan->chan_maxpower;

    pr_debug("%s(%d) \n",__func__,__LINE__);

    chan_dbg(wifimac->wm_curchan, "X8",1332);
    if (wifimac->drv_priv->drv_config.cfg_txamsdu) {
        os_timer_ex_start_period(&wifimac->drv_priv->drv_txtimer, NET80211_AMSDU_TIMEOUT);
    }

    os_timer_ex_start(&wifimac->wm_monitor_fw);
    return error;
}

const struct net_device_ops wifi_mac_netdev_ops =
{
    .ndo_get_stats = wifi_mac_getstats,
    .ndo_open = wifi_mac_open, // ifconfig up
    .ndo_stop = wifi_mac_stop, // if config down
    .ndo_start_xmit = wifi_mac_hardstart, // TCP/IP TX
    //netif_rx(skb);
    .ndo_change_mtu = wifi_mac_change_mtu,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    .ndo_siocdevprivate     = wifi_mac_ioctrl_wrapper,
#else
    .ndo_do_ioctl           = wifi_mac_ioctrl,
#endif
    .ndo_set_mac_address = wifi_set_mac_address,
};

#define AMLWL_IPV4    1
#define AMLWL_IPV6    2

static int aml_notify_ip(struct wifi_station *sta, u8 vif_ctx_id, u8 ip_type, u8 *ip_addr)
{
    if (ip_type != AMLWL_IPV4 && ip_type != AMLWL_IPV6)
        return -EINVAL;

    if (ip_type == AMLWL_IPV4) {
        memcpy(sta->sta_ap_ip, ip_addr, IPV4_LEN);
        sta->connect_status = CONNECT_DHCP_GET_ACK;
        sta->ip_acquired |= ip_type;
        sta->sta_arp_flag |= WIFI_ARP_GET_AP_IP;

    } else {
        memcpy(sta->sta_ap_ipv6, ip_addr, IPV6_LEN);
    }
    return 0;
}


static int aml_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *ndev;
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_station *sta;
    struct wifi_station_tbl *nt = NULL;
    struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
    unsigned char *tmp = NULL;

    if (!ifa || !(ifa->ifa_dev->dev)) {
        return NOTIFY_DONE;
    }
    if (ifa->ifa_dev->dev->netdev_ops != &wifi_mac_netdev_ops) {
        return NOTIFY_DONE;
    }

    ndev = ifa->ifa_dev->dev;
    wnet_vif = netdev_priv(ndev);
    nt = &(wnet_vif->vm_sta_tbl);

    if (event == NETDEV_UP && wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        struct wireless_dev *pwdev = wnet_vif->vm_wdev;
        struct vm_wdev_priv *pwdev_priv = wdev_to_priv(pwdev);

        sta = wifi_mac_get_sta(nt, wnet_vif->vm_des_bssid, wnet_vif->wnet_vif_id);

        if (sta != NULL) {
            if (sta->wnet_vif_id != wnet_vif->wnet_vif_id) {
                ERROR_DEBUG_OUT("vid:%d mismatch:%d\n", sta->wnet_vif_id, wnet_vif->wnet_vif_id);
            }

        } else {
            ERROR_DEBUG_OUT("failed to find sta\n");
            return NOTIFY_DONE;
        }
        tmp = (unsigned char *)(&ifa->ifa_address);
        AML_OUTPUT("[%s]get ip:%02d.%02d.%02d.%02d\n", ndev->name, tmp[0],tmp[1], tmp[2],tmp[3]);
        if (sta->sta_wnet_vif->vm_p2p->p2p_enable) {
            set_p2p_negotiation_status(sta, NET80211_P2P_STATE_GO_COMPLETE);
        }
        os_timer_ex_cancel(&pwdev_priv->connect_timeout, CANCEL_SLEEP);
        aml_notify_ip(sta, wnet_vif->wnet_vif_id, AMLWL_IPV4, (u8 *)&ifa->ifa_address);
    }
    return NOTIFY_DONE;
}



static void wifi_mac_set_ipv6_addr(SYS_TYPE param1, SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_station *sta = (struct wifi_station *)param1;
    unsigned char vif_id = (unsigned char)param2;
    unsigned char type = (unsigned char)param3;
    unsigned char *ipv6_addr = (unsigned char *)param4;

    aml_notify_ip(sta, vif_id, type, ipv6_addr);
    return;
}

static int aml_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *ndev;
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_station *sta;
    struct wifi_station_tbl *nt = NULL;
    struct inet6_ifaddr *inet6_ifa = (struct inet6_ifaddr *)ptr;
    unsigned char *tmp = NULL;

    if (!inet6_ifa || !(inet6_ifa->idev->dev)) {
        return NOTIFY_DONE;
    }
    if (inet6_ifa->idev->dev->netdev_ops != &wifi_mac_netdev_ops) {
        return NOTIFY_DONE;
    }

    ndev = inet6_ifa->idev->dev;
    wnet_vif = netdev_priv(ndev);
    nt = &(wnet_vif->vm_sta_tbl);

    if (event == NETDEV_UP && wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        sta = wifi_mac_get_sta(nt, wnet_vif->vm_des_bssid, wnet_vif->wnet_vif_id);

        if (sta != NULL) {
            if (sta->wnet_vif_id != wnet_vif->wnet_vif_id) {
                ERROR_DEBUG_OUT("vid:%d mismatch:%d\n", sta->wnet_vif_id, wnet_vif->wnet_vif_id);
            }

        } else {
            ERROR_DEBUG_OUT("failed to find sta\n");
            return NOTIFY_DONE;
        }
        tmp = (unsigned char *)(inet6_ifa->addr.in6_u.u6_addr8);
        AML_OUTPUT("get ipv6:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
            tmp[0],tmp[1], tmp[2],tmp[3], tmp[4],tmp[5], tmp[6],tmp[7], tmp[8],tmp[9], tmp[10],tmp[11], tmp[12],tmp[13], tmp[14],tmp[15]);

        wifi_mac_add_work_task(sta->sta_wmac, wifi_mac_set_ipv6_addr,
            NULL,(SYS_TYPE)sta, (SYS_TYPE)wnet_vif->wnet_vif_id, AMLWL_IPV6, (SYS_TYPE)&inet6_ifa->addr, 0);
    }

    return NOTIFY_DONE;
}

struct notifier_block aml_inetaddr_cb = {
    .notifier_call = aml_inetaddr_event
};

struct notifier_block aml_inet6addr_cb = {
    .notifier_call = aml_inet6addr_event
};

int wifi_mac_netdev_stop(void * ieee)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    int ret;

    AML_OUTPUT("<running>++\n");
    if (NET80211_IF_RUNNING(wifimac)==0)
        return 0 ;

    wifimac->wm_dev_flags &= ~NETCOM_NETIF_IFF_RUNNING;
    ret = wifimac->drv_priv->drv_ops.drv_stop(wifimac->drv_priv);
    AML_OUTPUT("<running> wm_dev_flags  = %x \n",
        wifimac->wm_dev_flags );
    if (wifimac->drv_priv->drv_config.cfg_txamsdu) {
        os_timer_ex_cancel(&wifimac->drv_priv->drv_txtimer, CANCEL_SLEEP);
    }

    os_timer_ex_cancel(&wifimac->wm_monitor_fw, CANCEL_SLEEP);
    return ret;
}

static ktime_t lock_kt;
static struct hrtimer hr_lock_timer;
int g_hr_lock_timer_valid = 0;

static enum hrtimer_restart wifi_mac_set_tx_lock_timeout(struct hrtimer *timer)
{
    struct drv_private *drv_priv = drv_get_drv_priv();

    if (drv_priv != NULL) {
        drv_priv->wait_mpdu_timeout = 1;
        tasklet_schedule(&drv_priv->ampdu_tasklet);
    }

    g_hr_lock_timer_valid = 0;
    return HRTIMER_NORESTART;
}

void wifi_mac_tx_lock_timer_attach(void)
{
    struct drv_private *drv_priv = drv_get_drv_priv();

    lock_kt = ktime_set(0, 1500000 * drv_priv->drv_config.cfg_hrtimer_interval);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
    hrtimer_setup(&hr_lock_timer, wifi_mac_set_tx_lock_timeout, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
    hrtimer_init(&hr_lock_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hr_lock_timer.function = wifi_mac_set_tx_lock_timeout;
#endif
    hrtimer_start(&hr_lock_timer, lock_kt, HRTIMER_MODE_REL);
    g_hr_lock_timer_valid = 1;
}

void wifi_mac_tx_lock_timer_cancel(void)
{
    if (g_hr_lock_timer_valid) {
        hrtimer_cancel(&hr_lock_timer);
        g_hr_lock_timer_valid = 0;
    }
}

int wifi_mac_cap_attach(struct wifi_mac *wifimac, struct drv_private* drv_priv)
{
    struct driver_ops  *ops = NULL;
    int vid = 0;

    wifimac->wm_mac_exit =0;
    wifimac->drv_priv = drv_priv;

    wifimac->wm_wnet_vif_mask = 0;
    wifimac->wm_caps = 0;
#if defined(SU_BF) || defined(MU_BF)
    wifimac->max_spatial = 2;
#endif

    ops = &drv_priv->drv_ops;
    wifi_mac_amsdu_attach(wifimac);
    // wifi_mac_com_clear_cap(wifimac, -1);

    wifi_mac_ComSetCap(wifimac, WIFINET_C_IBSS | WIFINET_C_HOSTAP | WIFINET_C_SHPREAMBLE
        | WIFINET_C_SHSLOT | WIFINET_C_WPA | WIFINET_C_BGSCAN);


    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_WMM))
        wifi_mac_ComSetCap(wifimac, WIFINET_C_WME);

    wifimac->wm_ac2q[WME_AC_BE] = HAL_WME_AC_BE;
    wifimac->wm_ac2q[WME_AC_BK] = HAL_WME_AC_BK;
    wifimac->wm_ac2q[WME_AC_VI ] = HAL_WME_AC_VI;
    wifimac->wm_ac2q[WME_AC_VO] = HAL_WME_AC_VO;

    wifimac->wm_mng_qid = HAL_WME_MGT;
    wifimac->wm_noqos_legacy_qid = HAL_WME_NOQOS;
    wifimac->wm_uapsd_qid = HAL_WME_UAPSD;

    /*get capabilities from 'drv_priv->drv_config'
      initialize in aml_driv_attach()*/
    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_UAPSD))
    {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_UAPSD);
        //WIFINET_COM_UAPSD_ENABLE(wifimac);
    }

    wifimac->wm_mcast_qnum = HAL_WME_MCAST;

    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_CIPHER_WEP))
        wifi_mac_ComSetCap(wifimac, WIFINET_C_WEP);
    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_CIPHER_WPI))
        wifi_mac_ComSetCap(wifimac, WIFINET_C_WAPI);
    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_CIPHER_AES_CCM))
    {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_AES);
        wifi_mac_ComSetCap(wifimac, WIFINET_C_AES_CCM);
    }
    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_CIPHER_TKIP))
        wifi_mac_ComSetCap(wifimac, WIFINET_C_TKIP);

    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_HWTKIPMIC))
        wifi_mac_ComSetCap(wifimac, WIFINET_C_TKIPMIC);

    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_HWCSUM))
    {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_HWCSUM);
    }
    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_BWAUTOCONTROL))
    {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_BWCTRL);
    }

    wifi_mac_com_clear_flag(wifimac, WIFINET_F_DOTH);

    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_HT))
    {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_HT);

        if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_RDG))
        {
            wifi_mac_ComSetCap(wifimac, WIFINET_C_RDG);
        }
        wifi_mac_com_set_flag(wifimac, WIFINET_F_HT);
        wifi_mac_com_set_maxampdu(wifimac, WIFINET_HTCAP_MAXRXAMPDU_32768);
        wifi_mac_com_set_mpdudensity(wifimac, WIFINET_HTCAP_MPDUDENSITY_8);
        wifi_mac_com_set_extflag(wifimac, WIFINET_FEXT_AMPDU);
        wifi_mac_com_set_extflag(wifimac, WIFINET_FEXT_SHORTGI_ENABLE);
    }


    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_VHT))
    {
        if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_RDG))
        {
            wifi_mac_ComSetCap(wifimac, WIFINET_C_RDG);
        }

        wifi_mac_com_set_flag(wifimac, WIFINET_F_HT);
        wifi_mac_com_set_maxampdu(wifimac, WIFINET_HTCAP_MAXRXAMPDU_32768);
        wifi_mac_com_set_mpdudensity(wifimac, WIFINET_HTCAP_MPDUDENSITY_8);
        wifi_mac_com_set_extflag(wifimac, WIFINET_FEXT_AMPDU);
    }

    /*set burst flag, burst frames in txop. */
    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_BURST_ACK))
    {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_BURST);
    }

    if (ops->drv_get_config_param(drv_priv, CHIP_PARAM_MFP)) {
        wifi_mac_ComSetCap(wifimac, WIFINET_C_MFP);
    }

    if (ops->get_current_country)
    {
        ops->get_current_country(drv_priv, (char *)&wifimac->wm_country.iso);
    }
    else
    {
        ASSERT(0);
    }

    spin_lock_init(&wifimac->channel_lock);
    /*set channel num and set current use channel by country code.*/
    wifi_mac_chan_attach(wifimac);
    wifi_mac_chan_overlapping_map_init(wifimac);

    if (wifimac->wm_caps & WIFINET_C_WME)
        wifimac->wm_flags |= WIFINET_F_WME;

    wifi_mac_reset_erp(wifimac, WIFINET_MODE_11BGN);
    wifi_mac_reset_ht(wifimac);

    wifimac->wm_suspend_mode = WIFI_SUSPEND_STATE_NONE;
    init_waitqueue_head(&wifimac->wm_suspend_wq);

    WIFINET_LOCK_INIT(wifimac, "wifi_mac");
    WIFINET_PSLOCK_INIT(wifimac, "wifi_mac_ps");
    WIFINET_QLOCK_INIT(wifimac, " wifi_mac_queue ");
    WIFINET_BEACONLOCK_INIT(wifimac, "wifi_mac_beacon");
    WIFINET_BEACONBUFLOCK_INIT(wifimac, "wifi_mac_beaconbuf");
    WIFINET_VMACS_LOCK_INIT(wifimac, "wlan_net_vif");

    /*init vmac list. */
    INIT_LIST_HEAD(&wifimac->wm_wnet_vifs);
    INIT_LIST_HEAD(&wifimac->wm_free_entryq);
    WIFINET_NODE_FREE_LOCK_INIT(wifimac);

    wifimac->scan_gain_thresh_unconnect = UNCONNECT_MIN_GIAN_THRESHOLD;
    wifimac->scan_gain_thresh_connect = CONNECT_MIN_GIAN_THRESHOLD;
    wifimac->scan_max_gain_thresh = MAX_GIAN_THRESHOLD;
    wifimac->scan_noisy_status = WIFINET_S_SCAN_ENV_CLEAR;
    wifimac->is_connect_set_gain = 1;

    wifimac->wm_txpowlimit = WIFINET_TXPOWER_MAX;
    wifimac->wm_rx_streams = WIFINET_RXSTREAM;
    wifimac->wm_tx_streams = WIFINET_TXSTREAM;
    wifimac->wm_flags_ext |= WIFINET_C_WDS_AUTODETECT;
    wifimac->wm_flags_ext |= WIFINET_FEXT_COUNTRYIE;
    /*protect type, no protect, rts/cts or cts only */
    wifimac->wm_protmode = WIFINET_PROT_RTSCTS;
    for(vid = 0; vid < DEFAULT_MAX_VMAC; vid++)
    {
        wifimac->wm_wme[vid].wme_hipri_switch_hysteresis = 3;
    }
    wifi_mac_pwrsave_attach();
    /*prepare encryption cipher and de/encrypt handler */
    wifi_mac_security_attach(wifimac);
    /* initialize inact_timer for ap&sta, sta will keep alive
    to ap (send nulldata to ap), ap will monitor if sta leave bss.*/

    /*initialize scan param and scan_timer */
    wifi_mac_scan_attach(wifimac);
    wifimac->wm_esco_en    = 0;
    wifimac->wm_bt_en = 0;

#ifdef CONFIG_CONCURRENT_MODE
    wifimac->wm_vsdb_slot = CONCURRENT_SLOT_NONE;
    wifimac->wm_vsdb_flags = 0;
    wifimac->wm_vsdb_switch_time = 0;
    wifimac->vsdb_mode_set_noa_enable = 0;

    /*init a concurrent timer for vmac channel concurrent function. */
    os_timer_ex_initialize(&wifimac->wm_concurrenttimer, COMCURRENT_CHANNCHANGE_TIME,
        concurrent_change_channel_timeout, wifimac);
#endif

    wifimac->recovery_stat = WIFINET_RECOVERY_END;
    os_timer_ex_initialize(&wifimac->wm_monitor_fw, 2000,wifi_mac_trigger_recovery, wifimac);

    AML_OUTPUT("<running>\n");
    return 0;
}

int wifi_mac_cap_detach(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *wnet_vif;
    wifi_mac_stop_running(wifimac);

    AML_OUTPUT("<running>\n");

    while (!list_empty(&wifimac->wm_wnet_vifs))
    {
        wnet_vif = list_first_entry(&wifimac->wm_wnet_vifs,
                struct wlan_net_vif, vm_next);
        wifi_mac_vmac_delt(wnet_vif);
    }

    wifi_mac_scan_detach(wifimac);
    wifi_mac_security_detach(wifimac);
    wifi_mac_pwrsave_detach();
    wifi_mac_StationDetach(wifimac);

    WIFINET_VMACS_LOCK_DESTROY(wifimac);
    WIFINET_LOCK_DESTROY(wifimac);

    wifi_mac_acl_deattach();
    wifi_mac_amsdu_detach(wifimac);

#ifdef CONFIG_CONCURRENT_MODE
    os_timer_ex_del(&wifimac->wm_concurrenttimer, CANCEL_SLEEP);
#endif

    wifi_mac_tx_lock_timer_cancel();
    os_timer_ex_del(&wifimac->wm_monitor_fw, CANCEL_SLEEP);

    AML_OUTPUT("<running>\n");
    wifimac->drv_priv = NULL;

    return 0;
}


static unsigned char wifi_mac_tx_init(struct wifi_mac *wifimac) {
    struct list_head *head = &wifimac->txdesc_freequeue;
    struct drv_txdesc *ptxdesc;
    int i, bsize;

    TX_DESC_BUF_LOCK_INIT(wifimac);

    /* Setup tx descriptors */
    bsize = sizeof(struct drv_txdesc) * DRV_TXDESC_NUM;
#ifdef CONFIG_AML_USE_STATIC_BUF
    ptxdesc = wifi_mem_prealloc(AML_TX_DESC_BUF, bsize);
    memset(ptxdesc, 0, bsize);
#else
    ptxdesc = (struct drv_txdesc *)NET_MALLOC(bsize, GFP_KERNEL, "drv_tx_init.ptxdesc");
#endif
    if (ptxdesc == NULL)
    {
        return -ENOMEM;
    }
    wifimac->txdesc_bufptr = ptxdesc;

    INIT_LIST_HEAD(head);
    for (i = 0; i < DRV_TXDESC_NUM; i++, ptxdesc++)
    {
        list_add_tail(&ptxdesc->txdesc_queue, head);
    }

    return wifimac->drv_priv->drv_ops.tx_init(wifimac->drv_priv, DRV_TXDESC_NUM);
}

static void wifi_mac_tx_cleanup(struct wifi_mac *wifimac) {
    struct list_head *head = &wifimac->txdesc_freequeue;

    wifimac->drv_priv->drv_ops.tx_cleanup(wifimac->drv_priv);

    /* cleanup tx descriptors */
    if (wifimac->txdesc_bufptr != NULL)
    {
        INIT_LIST_HEAD(head);
#ifndef CONFIG_AML_USE_STATIC_BUF
        NET_FREE(wifimac->txdesc_bufptr, "drv_tx_init.ptxdesc");
#endif
        wifimac->txdesc_bufptr = NULL;
    }

    return;
}

int wifi_mac_entry(struct wifi_mac *wifimac, void *drv_priv)
{
    int error = 0;

    AML_OUTPUT("<running>++ \n");

    error = wifi_mac_cap_attach(wifimac,(struct drv_private*)drv_priv);
    if (error != 0)
        goto bad;

    error = wifi_mac_tx_init(wifimac);
    if (error != 0)
        goto bad1;

    error = wifimac->drv_priv->drv_ops.rx_init(drv_priv, DRV_RXDESC_NUM);
    if (error != 0)
        goto bad2;

    return 0;

bad2:
    wifi_mac_tx_cleanup(wifimac);
bad1:
    wifi_mac_cap_detach(wifimac);
bad:
    AML_OUTPUT("bad!\n");
    return error;
}


 void wifi_mac_get_sts(struct wifi_mac *wifimac,unsigned int op_code, unsigned int ctrl_code)
{
    int i ;
    struct wlan_net_vif *wnet_vif = NULL;
    ASSERT(wifimac != NULL);

    if((ctrl_code & STS_MOD_WMAC)  == STS_MOD_WMAC)
    {
            pr_debug("\n--------mac_statistic--------\n");
 /*vif tx path*/
    if ((ctrl_code & STS_TYP_TX) == STS_TYP_TX)
        {
            list_for_each_entry(wnet_vif,&wifimac->wm_wnet_vifs,vm_next)
                {
                    pr_debug("\n--------vif_id %d--------\n",  wnet_vif->wnet_vif_id);

                    pr_debug("tx_total_msdu %d\n", wnet_vif->vif_sts.sts_tx_total_msdu);
                    pr_debug("tx_total_out_msdu %d\n", wnet_vif->vif_sts.sts_tx_out_msdu);
                    pr_debug("tx_total_drop_msdu %d\n", wnet_vif->vif_sts.sts_tx_drop_msdu);

                    pr_debug("\ntx_total_tcp_msdu %d\n", wnet_vif->vif_sts.sts_tx_tcp_msdu);
                    pr_debug("tx_total_udp_msdu %d\n", wnet_vif->vif_sts.sts_tx_udp_msdu );
                    pr_debug("tx_total_arp_msdu %d\n",wnet_vif->vif_sts.sts_tx_arp_msdu);
                    pr_debug("tx_total_ns_msdu %d\n\n",wnet_vif->vif_sts.sts_tx_ns_msdu);


                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_tid_in_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_tid_in_msdu[i]);
                    }

                     for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_tid_drop_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_tid_drop_msdu[i]);
                    }

                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_tid_out_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_tid_out_msdu[i]);
                    }

                     for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_tid_drop_bf_in_amsdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_tid_drop_bf_in_msdu[i]);
                    }


                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_amsdu_in_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_amsdu_in_msdu[i]);
                    }

                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_amsdu_drop_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_amsdu_drop_msdu[i]);
                    }

                     for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_asmdu_pend_msdu[%d] %d\n", i,   wifimac->msdu_cnt[i]);
                    }

                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_amsdu_out_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_amsdu_out_msdu[i]);
                    }

                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_non_amsdu_in_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_non_amsdu_in_msdu[i]);

                    }

                     for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_non_amsdu_drop_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_non_amsdu_drop_msdu[i]);
                    }

                    for(i = 0; i < WME_NUM_TID; i++)
                    {
                        pr_debug("tx_non_amsdu_out_msdu[%d] %d\n", i,wnet_vif->vif_sts.sts_tx_non_amsdu_out_msdu[i]);
                    }
                }
        }

/*vif rx path*/
           if ((ctrl_code & STS_TYP_RX) == STS_TYP_RX)
            {
                 list_for_each_entry(wnet_vif,&wifimac->wm_wnet_vifs,vm_next)
                {
                    pr_debug("\n--------vif_id %d--------\n",  wnet_vif->wnet_vif_id);
                    pr_debug("rx_bad_ver %d\n", wnet_vif->vif_sts.sts_rx_bad_ver);

                    pr_debug("rx_too_short %d\n",wnet_vif->vif_sts.sts_rx_too_short);

                    pr_debug("rx_wrong_bss %d\n",wnet_vif->vif_sts.sts_rx_wrong_bss);

                    pr_debug("rx_dup %d\n",wnet_vif->vif_sts.sts_rx_dup);

                    pr_debug("rx_wrong_dir %d\n",wnet_vif->vif_sts.sts_rx_wrong_dir);

                    pr_debug("rx_mcast_echo %d\n",wnet_vif->vif_sts.sts_rx_mcast_echo);

                    pr_debug("rx_not_assoc %d\n",wnet_vif->vif_sts.sts_rx_not_assoc);

                    pr_debug("rx_no_priv_ac %d\n",wnet_vif->vif_sts.sts_no_priv_ac);

                    pr_debug("rx_uncrypted %d\n",wnet_vif->vif_sts.sts_rx_uncrypted);

                    pr_debug("rx_decap %d\n",wnet_vif->vif_sts.sts_rx_decap);

                    pr_debug("rx_mng_discard %d\n",wnet_vif->vif_sts.sts_mng_discard);

                    pr_debug("rx_ctrl %d\n",wnet_vif->vif_sts.sts_rx_ctrl);

                    pr_debug("rx_bcn %d\n",wnet_vif->vif_sts.sts_rx_bcn);

                    pr_debug("rx_elem_miss %d\n",wnet_vif->vif_sts.sts_rx_elem_miss);

                    pr_debug("rx_elem_too_long %d\n",wnet_vif->vif_sts.sts_rx_elem_too_long);

                    pr_debug("rx_elem_too_short %d\n",wnet_vif->vif_sts.sts_rx_elem_too_short);

                    pr_debug("rx_elem_err %d\n",wnet_vif->vif_sts.sts_rx_elem_err);

                    pr_debug("rx_sta_all_fail %d\n",wnet_vif->vif_sts.sts_rx_sta_all_fail);

                    pr_debug("rx_ssid_mismatch %d\n", wnet_vif->vif_sts.sts_rx_ssid_mismatch);

                    pr_debug("rx_auth_unsprt %d\n", wnet_vif->vif_sts.sts_auth_unsprt);

                    pr_debug("rx_auth_fail %d\n",wnet_vif->vif_sts.sts_rx_auth_fail);

                    pr_debug("rx_ps_uncnnt %d\n", wnet_vif->vif_sts.sts_rx_ps_uncnnt);

                    pr_debug("rx_ps_aid_err %d\n",wnet_vif->vif_sts.sts_rx_ps_aid_err);

                    pr_debug("rx_assoc_err %d\n", wnet_vif->vif_sts.sts_rx_assoc_err);

                    pr_debug("rx_assoc_unauth %d\n",wnet_vif->vif_sts.sts_rx_assoc_unauth);

                    pr_debug("rx_assoc_cap_mismatch %d\n", wnet_vif->vif_sts.sts_rx_assoc_cap_mismatch);

                    pr_debug("rx_assoc_rate_mismatch %d\n", wnet_vif->vif_sts.sts_rx_assoc_rate_mismatch);

                    pr_debug("rx_assoc_wpa_mismatch %d\n",wnet_vif->vif_sts.sts_rx_assoc_wpa_mismatch);

                    pr_debug("rx_deauth %d\n", wnet_vif->vif_sts.sts_rx_deauth);

                    pr_debug("rx_dis_assoc %d\n",wnet_vif->vif_sts.sts_rx_dis_assoc);

                    pr_debug("rx_action %d\n", wnet_vif->vif_sts.sts_rx_action);

                    pr_debug("rx_buf_full %d\n",wnet_vif->vif_sts.sts_rx_buf_full);

                    pr_debug("rx_decrypt_err %d\n", wnet_vif->vif_sts.sts_rx_decrypt_err);

                    pr_debug("rx_auth_mismatch %d\n", wnet_vif->vif_sts.sts_rx_auth_mismatch);

                    pr_debug("rx_unauth %d\n",wnet_vif->vif_sts.sts_rx_unauth);

                    pr_debug("rx_tkip_mic_err %d\n", wnet_vif->vif_sts.sts_rx_tkip_mic_err);

                    pr_debug("rx_acl_mismatch %d\n", wnet_vif->vif_sts.sts_rx_acl_mismatch);

                    pr_debug("tx_no_buf %d\n", wnet_vif->vif_sts.sts_tx_no_buf);

                    pr_debug("tx_no_sta %d\n", wnet_vif->vif_sts.sts_tx_no_sta);

                    pr_debug("tx_unsprt_mngt %d\n",wnet_vif->vif_sts.sts_tx_unsprt_mngt);

                    pr_debug("tx_key_err %d\n",wnet_vif->vif_sts.sts_tx_key_err);

                    pr_debug("tx_headroom_err %d\n",wnet_vif->vif_sts.sts_headroom_err);

                    pr_debug("tx_sta_aged %d\n",wnet_vif->vif_sts.sts_tx_sta_aged);

                    pr_debug("tx_tkip_sw_mic_err %d\n",wnet_vif->vif_sts.sts_tx_tkip_sw_mic_err);

                    pr_debug("tx_ps_no_data %d\n",wnet_vif->vif_sts.sts_tx_ps_no_data);

                    pr_debug("rx_tkip_sw_mic_err %d\n", wnet_vif->vif_sts.sts_rx_tkip_sw_mic_err);

                    pr_debug("key_drop %d\n", wnet_vif->vif_sts.sts_key_drop);

                    pr_debug("key_id_err %d\n", wnet_vif->vif_sts.sts_key_id_err);

                    pr_debug("key_type_err %d\n",wnet_vif->vif_sts.sts_key_type_err);

                    pr_debug("rx_msdu %d\n",wnet_vif->vif_sts.sts_rx_msdu);

                    pr_debug("rx_msdu_time_stamp %ld\n",wnet_vif->vif_sts.sts_rx_msdu_time_stamp);
                }
            }
        }


        wifimac->drv_priv->drv_ops.drv_get_sts(wifimac->drv_priv, op_code, ctrl_code);
}


int wifi_mac_mac_exit(struct wifi_mac *wifimac)
{
    AML_OUTPUT("------>\n");
    wifimac->wm_mac_exit =1;
    wifi_mac_tx_cleanup(wifimac);

    return wifi_mac_cap_detach(wifimac);
}

static void get_legacy_sta(void *arg, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = arg;

    if (sta->sta_associd == 0)
        return;

    if (WIFINET_INCLUDE_11N(sta->sta_wnet_vif->vm_mac_mode)&&
        !(sta->sta_flags & WIFINET_NODE_HT))
    {
        wifimac->wm_non_ht_sta++;
    }
}

static void get_ht20_sta(void *arg, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = arg;

    if (sta->sta_associd == 0)
        return;

    if ((sta->sta_flags & WIFINET_NODE_HT) &&
        (sta->sta_chbw == WIFINET_BWC_WIDTH20))
    {
        wifimac->wm_ht20_sta++;
    }
}

/*
 * record sta op_mode(ht or legacy), for ht_info filed in
 * beacon/probe resp, refer to wifi_mac_add_htinfo(). This
 * is used to operate 20/40Mhz coexistence.
 */
void wifi_mac_ht_prot(struct wifi_mac *wifimac, struct wifi_station *sta, enum wifi_mac_ht_prot_update_flag flag)
{
    unsigned short legacy_stations = 0;
    unsigned short ht20_stations = 0;
    unsigned short legacy_ap = (wifimac->wm_flags_ext & WIFINET_F_NONHT_AP)?1:0;
    unsigned short prior_sm = wifimac->wm_ht_prot_sm;

    if (!NET80211_IF_RUN_UP(wifimac)) {
        return;
    }

    if (WIFINET_INCLUDE_11N(sta->sta_wnet_vif->vm_mac_mode) && !(sta->sta_flags & WIFINET_NODE_HT)) {
        if (flag == WIFINET_STA_CONNECT) {
            wifimac->wm_non_ht_sta++;

        } else if (flag == WIFINET_STA_DISCONNECT) {
            wifimac->wm_non_ht_sta--;
        }
    }

    if ((sta->sta_flags & WIFINET_NODE_HT) && (sta->sta_chbw == WIFINET_BWC_WIDTH20)) {
        if (flag == WIFINET_STA_CONNECT) {
            wifimac->wm_ht20_sta++;

        } else if (flag == WIFINET_STA_DISCONNECT) {
            wifimac->wm_ht20_sta--;
        }
    }
    legacy_stations = wifimac->wm_non_ht_sta;
    ht20_stations = wifimac->wm_ht20_sta;

    if (legacy_stations) {
        wifimac->wm_ht_prot_sm = WIFINET_HTINFO_OPMODE_MIXED_PROT_ALL;

    } else if (legacy_ap) {
        wifimac->wm_ht_prot_sm = WIFINET_HTINFO_OPMODE_MIXED_PROT_OPT;

    } else {
        wifimac->wm_ht_prot_sm = WIFINET_HTINFO_OPMODE_PURE;
    }

    if (prior_sm != wifimac->wm_ht_prot_sm)
        wifimac->wm_flags_ext |= WIFINET_FEXT_HTUPDATE;
}

void
wifi_mac_change_cbw(struct wifi_mac *wifimac, int to20)
{
    if (wifimac->wm_flags_ext & WIFINET_FEXT_COEXT_DISABLE)
        return;

    wifimac->wm_flags_ext |= WIFINET_FEXT_HTUPDATE;
}

void
wifi_mac_reset_vmac(struct wlan_net_vif *wnet_vif)
{
    wnet_vif->vm_pstxqueue_flags = 0;
}


void wifi_mac_reset_tspecs(struct wlan_net_vif *wnet_vif)
{
    unsigned char ac;
    unsigned char idx;

    for (ac = 0; ac < WME_AC_NUM; ac++) {
        for (idx = 0; idx < TS_DIR_IDX_COUNT; idx++) {
            memset(&wnet_vif->tspecs[ac][idx], 0, sizeof(struct wifi_mac_wmm_tspec_element));
        }
    }
}


void
wifi_mac_reset_ht(struct wifi_mac *wifimac)
{

    if (wifimac->wm_nopened > 1)
        return;

    wifimac->wm_non_ht_sta = 0;
    wifimac->wm_ht20_sta = 0;
    wifimac->wm_ht_prot_sm = WIFINET_HTINFO_OPMODE_PURE;
    wifimac->wm_flags_ext &= ~WIFINET_F_NONHT_AP;
}

void
wifi_mac_reset_vht(struct wifi_mac *wifimac)
{
}

void
wifi_mac_reset_erp(struct wifi_mac *wifimac, enum wifi_mac_macmode mode)
{
    wifimac->wm_flags &= ~WIFINET_F_USEPROT;

    wifimac->wm_nonerpsta = 0;
    wifimac->wm_longslotsta = 0;

    wifi_mac_set_shortslottime(wifimac, ( (WIFINET_MODE_11B != mode) && (wifimac->wm_caps & WIFINET_C_SHSLOT)));

    if (wifimac->wm_caps & WIFINET_C_SHPREAMBLE)
    {
        wifimac->wm_flags |= WIFINET_F_SHPREAMBLE;
        wifimac->wm_flags &= ~WIFINET_F_USEBARKER;
    }
    else
    {
        wifimac->wm_flags &= ~WIFINET_F_SHPREAMBLE;
        wifimac->wm_flags |= WIFINET_F_USEBARKER;
    }

}

void
wifi_mac_set_shortslottime(struct wifi_mac *wifimac, int onoff)
{

    if (onoff)
        wifimac->wm_flags |= WIFINET_F_SHSLOT;
    else
        wifimac->wm_flags &= ~WIFINET_F_SHSLOT;

    wifi_mac_updateslot(wifimac);
}

void
wifi_mac_wme_initparams(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    WIFINET_BEACON_LOCK(wifimac);
    wifi_mac_wme_initparams_locked(wnet_vif);
    WIFINET_BEACON_UNLOCK(wifimac);
}

static void
wifi_mac_phyparam_2_wmeparam (struct wmeParams *ps_dst,
        const struct WmeParamDefault *ps_src)
{
    ps_dst->wmep_acm = ps_src->acm;
    ps_dst->wmep_aifsn = ps_src->aifsn;
    ps_dst->wmep_logcwmin = ps_src->logcwmin;
    ps_dst->wmep_logcwmax = ps_src->logcwmax;
    ps_dst->wmep_txopLimit = ps_src->txopLimit;
}

static void
wifi_mac_wmeparam_2_wmeparam (struct wmeParams *ps_dst,
        const struct wmeParams *ps_src)
{
    ps_dst->wmep_acm = ps_src->wmep_acm;
    ps_dst->wmep_aifsn = ps_src->wmep_aifsn;
    ps_dst->wmep_logcwmin = ps_src->wmep_logcwmin;
    ps_dst->wmep_logcwmax = ps_src->wmep_logcwmax;
    ps_dst->wmep_txopLimit = ps_src->wmep_txopLimit;
}

static enum
wifi_mac_wmm_phymode wifi_mac_wmm_phymode (enum wifi_mac_macmode mode)
{
    enum wifi_mac_wmm_phymode wmm_phymode = WIFINET_WMM_PHYMODE_G;

    if (mode == WIFINET_MODE_11B)
        wmm_phymode = WIFINET_WMM_PHYMODE_B;
    return wmm_phymode;
}

void
wifi_mac_wme_initparams_locked(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_wme_state *wme = &wifimac->wm_wme[wnet_vif->wnet_vif_id];
    enum wifi_mac_macmode mode;
    enum wifi_mac_wmm_phymode wmm_phymode;
    const struct WmeParamDefault *pApPhyParam, *pStaPhyParam;
    int i;

    DPRINTF(AML_DEBUG_WME, "%s %d\n",__func__,__LINE__);
    WIFINET_BEACON_LOCK_ASSERT(wifimac);

    if ((wifimac->wm_caps & WIFINET_C_WME) == 0)
        return;

    mode = wnet_vif->vm_mac_mode;
    wmm_phymode = wifi_mac_wmm_phymode(mode);
    DPRINTF(AML_DEBUG_WME, "%s %d macmode=%d, wmmphymode=%d\n",
            __func__,__LINE__, mode, wmm_phymode);
    for (i = 0; i < WME_NUM_AC; i++) {
        switch (i) {
            case WME_AC_BK:
                pApPhyParam = &ap_phy_param_AC_BK[wmm_phymode];
                pStaPhyParam = &sta_phy_param_AC_BK[wmm_phymode];
                break;
            case WME_AC_VI:
                pApPhyParam = &ap_phy_param_AC_VI[wmm_phymode];
                pStaPhyParam = &sta_phy_param_AC_VI[wmm_phymode];
                break;
            case WME_AC_VO:
                pApPhyParam = &ap_phy_param_AC_VO[wmm_phymode];
                pStaPhyParam = &sta_phy_param_AC_VO[wmm_phymode];
                break;
            case WME_AC_BE:
            default:
                pApPhyParam = &ap_phy_param_AC_BE[wmm_phymode];
                pStaPhyParam = &sta_phy_param_AC_BE[wmm_phymode];
                break;
        }

        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            wifi_mac_phyparam_2_wmeparam(&(wme->wme_wmeChanParams.cap_wmeParams[i]), pApPhyParam);

        } else {
            wifi_mac_phyparam_2_wmeparam(&(wme->wme_wmeChanParams.cap_wmeParams[i]), pStaPhyParam);
        }
        DPRINTF(AML_DEBUG_WME, "%s %d wmep_logcwmax =%d\n",__func__,__LINE__,
                wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmax);
    }

    wme->wme_wmeChanParams.cap_info_count = (unsigned char)(-1);
    wme->wme_wmeBssChanParams.cap_info_count = 0;

    if (wnet_vif->vm_mainsta != NULL) {
        wme->wme_hipri_switch_thresh = (10 * wnet_vif->vm_bcn_intval) / 100;
        wifi_mac_wme_updateparams_locked(wnet_vif);
    }
}

static void
wme_update_orig(SYS_TYPE param1,SYS_TYPE param2,
                    SYS_TYPE param3,SYS_TYPE param4,
                    SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *) param1;
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param4;

    if (wnet_vif->wnet_vif_replaycounter != (int)param5)
        return ;
    wifi_mac_wmm_update2hal(wifimac,wnet_vif);
}

void
wme_update_ex(struct wifi_mac *wifimac,struct wlan_net_vif *wnet_vif )
{
    wifi_mac_add_work_task(wifimac, wme_update_orig,
            NULL, (SYS_TYPE)wifimac,0,0,
            (SYS_TYPE)wnet_vif,wnet_vif->wnet_vif_replaycounter);
}


void
wifi_mac_wme_updateparams_locked(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_wme_state *wme = &wifimac->wm_wme[wnet_vif->wnet_vif_id];
    enum wifi_mac_macmode mode;
    enum wifi_mac_wmm_phymode wmm_phymode;
    int i;

    DPRINTF(AML_DEBUG_WME, "%s %d\n",__func__,__LINE__);

    for (i = 0; i < WME_NUM_AC; i++) {
        wifi_mac_wmeparam_2_wmeparam(&(wme->wme_chanParams.cap_wmeParams[i]),
                &(wme->wme_wmeChanParams.cap_wmeParams[i]));
        DPRINTF(AML_DEBUG_WME, "%s %d wmep_logcwmax =%d\n",__func__,__LINE__,
                wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmax);

        wifi_mac_wmeparam_2_wmeparam(&(wme->wme_bssChanParams.cap_wmeParams[i]),
                &(wme->wme_wmeBssChanParams.cap_wmeParams[i]));
    }

    mode = wnet_vif->vm_mac_mode;
    wmm_phymode = wifi_mac_wmm_phymode(mode);

    if (((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) && ((wme->wme_flags & WME_F_AGGRESSIVE) != 0))
        || ((wnet_vif->vm_opmode != WIFINET_M_HOSTAP) && (wnet_vif->vm_mainsta->sta_flags & WIFINET_NODE_QOS) == 0)
        || ((wnet_vif->vm_flags & WIFINET_F_WME) == 0)) {
        DPRINTF(AML_DEBUG_WME, "%s %d use phy_param_BE_nonqos for BE\n",__func__,__LINE__);

        wifi_mac_phyparam_2_wmeparam(&(wme->wme_chanParams.cap_wmeParams[WME_AC_BE]),
                &phy_param_BE_nonqos[wmm_phymode]);

        wifi_mac_phyparam_2_wmeparam(&(wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE]),
                &phy_param_BE_nonqos[wmm_phymode]);
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) && (wnet_vif->vm_sta_assoc < 2)
        && ((wme->wme_flags & WME_F_AGGRESSIVE) != 0)) {
        DPRINTF(AML_DEBUG_WME, "%s %d use log_cwwin_BE_agg_log for BE\n",__func__,__LINE__);
        wme->wme_chanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmin
            = wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmin
            = log_cwwin_BE_agg_log[wmm_phymode];
    }

    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
        wme->wme_bssChanParams.cap_info_count = (wme->wme_bssChanParams.cap_info_count+1) & WME_QOSINFO_COUNT;
        wnet_vif->vm_flags |= WIFINET_F_WMEUPDATE;
        DPRINTF(AML_DEBUG_WME, "%s %d update qosinfo count=%d\n",
                __func__, __LINE__, wme->wme_bssChanParams.cap_info_count);
    }

    wme_update_ex(wifimac,wnet_vif);
}

void
wifi_mac_wme_updateparams(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (wifimac->wm_caps & WIFINET_C_WME)
    {
        WIFINET_BEACON_LOCK(wifimac);
        wifi_mac_wme_updateparams_locked(wnet_vif);
        WIFINET_BEACON_UNLOCK(wifimac);
    }
}

static int wifi_mac_check_main_sta(struct wlan_net_vif *wnet_vif)
{
    int return_val = 0;
    if (!wnet_vif->vm_mainsta) {
        DPRINTF(AML_DEBUG_WARNING, "%s vid:%d vm_mainsta is NULL, alloc new sta\n", __func__, wnet_vif->wnet_vif_id);

        /*sta side allocate a sta buffer to save ap capabilities and information */
        wnet_vif->vm_mainsta = wifi_mac_get_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, wnet_vif->vm_myaddr);
        if (wnet_vif->vm_mainsta == NULL)
        {
            DPRINTF(AML_DEBUG_ERROR, "ERROR::%s %d alloc sta FAIL!!! \n",__func__,__LINE__);
            return_val = 1;
        }
    }
   return  return_val;
}


int wifi_mac_initial(struct net_device *dev, int forcescan)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int return_val = 0;
    struct hal_private *hal_priv = hal_get_priv();
    int total_delay = 0;

    DPRINTF(AML_DEBUG_WARNING,"<%s>:<running> %s\n", wnet_vif->vm_ndev->name, __func__);

    while ((total_delay < 1000) && (wifimac->recovery_stat != WIFINET_RECOVERY_END)) {
        msleep(10);
        total_delay +=10;
    }

    wifi_mac_preempt_scan(wifimac, 100, 100);

    if (1 == wifi_mac_check_main_sta(wnet_vif)) {
        return 1;
    }

    hal_priv->powersave_init_flag = 0;
    wifimac->scan_noisy_status = WIFINET_S_SCAN_ENV_NOISE;
    wifimac->is_connect_set_gain = 1;

    /*dev/interface is not up. */
    if ((dev->flags & IFF_RUNNING) == 0) {
        AML_OUTPUT("<%s>wm_nopened =%d\n", wnet_vif->vm_ndev->name, wifimac->wm_nopened);
        /*set power save state */
        wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_VMAC_UP);

        if ((wifimac->wm_nopened++ == 0) && (NET80211_IF_RUNNING(wifimac) == 0)) {
            DPRINTF(AML_DEBUG_CONNECT, "<%s>: %s, before calling wifi_mac_netdev_open\n", wnet_vif->vm_ndev->name, __func__);

            /*set running and up flag for wifimac*/
            return_val = wifi_mac_netdev_open(wifimac);

            if (return_val != 0)
                return return_val;
        }
        vm_cfg80211_up(wnet_vif);

        dev->flags |= IFF_RUNNING;
#if (DEFAULT_INITIAL_POWERMODE == 1)
        wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_LOW);
#endif
    }

    if (NET80211_IF_RUN_UP(wifimac))
    {
        if (wnet_vif->vm_opmode == WIFINET_M_STA)
        {
            DPRINTF(AML_DEBUG_CONNECT,"<%s>:<running> %s %d \n",
                    wnet_vif->vm_ndev->name,__func__,__LINE__);
            if ((wnet_vif->vm_state != WIFINET_S_CONNECTED) || forcescan)
            {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
            }
            else
            {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 1);
            }
        }
        else
        {
            if (wnet_vif->vm_opmode == WIFINET_M_MONITOR
                || wnet_vif->vm_opmode == WIFINET_M_WDS) {
                wnet_vif->vm_state = WIFINET_S_INIT;
                wifi_mac_top_sm(wnet_vif, WIFINET_S_CONNECTED, -1);
            } else {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
            }
        }
    }
    return 0;
}

int wifi_mac_open(struct net_device *dev)
{
    int retry;
    for (retry = 0; ++retry; ) {
        if (!aml_insmod_flag) {
            ERROR_DEBUG_OUT("<running> dev = %p, aml insmod not complete\n", dev);
            if (retry > 6) {
                return -1;
            } else {
                msleep(1000);
            }
        } else {
            break;
        }
    }

    pr_debug("<running> %s %d dev = %p\n",__func__,__LINE__,dev);
    return wifi_mac_initial(dev, 0);
}

int wifi_mac_stop(struct net_device *dev)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int index = 0;
    int retry;
    for (retry = 0; ++retry; ) {
        if (!aml_insmod_flag) {
            ERROR_DEBUG_OUT("<running> dev = %p, aml insmod not complete\n", dev);
            if (retry > 6) {
                return -1;
            } else {
                msleep(1000);
            }
        } else {
            break;
        }
    }

    pr_debug("%s wm_nopened %d, wm_dev_flags:%08x\n", __func__, wifimac->wm_nopened, wifimac->wm_dev_flags);
    preempt_scan(dev, 100, 100);
    cancel_work_sync(&wifimac->wm_scan->timeout_work);

    if (dev->flags & IFF_RUNNING)
    {
        dev->flags &= ~IFF_RUNNING;

        wifi_mac_top_sm(wnet_vif, WIFINET_S_INIT, -1);

        os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);
        os_timer_ex_cancel(&wnet_vif->vm_actsend, CANCEL_SLEEP);
#ifdef  AML_UNIT_TESTCASE
        aml_ifdown_unit_testcase_stop(dev);
#endif

        vm_cfg80211_down(wnet_vif);

        if ((--wifimac->wm_nopened == 0) && NET80211_IF_RUNNING(wifimac)) {
            WIFINET_DPRINTF(AML_DEBUG_DEBUG, "driver open ic_nopened %d\n", wifimac->wm_nopened);
            wifi_mac_netdev_stop(wifimac);
#if 0
            WIFINET_FW_STAT_LOCK(wifimac);
            if (wifimac->recovery_stat == WIFINET_RECOVERY_END) {
                wifimac->recovery_stat = WIFINET_RECOVERY_START;
                WIFINET_FW_STAT_UNLOCK(wifimac);

                msleep(200);
                wifimac->drv_priv->drv_ops.fw_repair(wifimac->drv_priv);
                WIFINET_FW_STAT_LOCK(wifimac);
                wifimac->recovery_stat = WIFINET_RECOVERY_END;
            }
            WIFINET_FW_STAT_UNLOCK(wifimac);
#endif
        }

        for (index=0; index<WIFINET_APPIE_NUM_OF_FRAME; index++) {
            wifi_mac_rm_app_ie(&wnet_vif->app_ie[index]);
        }
        wifi_mac_rm_app_ie(&wnet_vif->assocrsp_ie);
        wifi_mac_rm_app_ie(&wnet_vif->assocreq_ie);
        wnet_vif->vm_opt_ie_len =0;
    }

    return 0;
}


void
wifi_mac_stop_running(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *wnet_vif = NULL, *wnet_vif_next = NULL;
    struct net_device *dev;

    list_for_each_entry_safe(wnet_vif,wnet_vif_next,&wifimac->wm_wnet_vifs,vm_next)
    {
        dev = wnet_vif->vm_ndev;
        if (dev->flags & IFF_RUNNING)
        {
            wifi_mac_stop(dev);
        }
    }
}
static void wifi_mac_tx_act_timeout_ex (SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    pr_debug("%s vid:%d, act_pkt_retry_count:%d, token:%d\n",
        __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_p2p->act_pkt_retry_count, wnet_vif->vm_p2p->action_dialog_token);

    if (wnet_vif->vm_p2p->tx_status_flag == WIFINET_TX_STATUS_SUCC) {
        return;
    }

    if ((wnet_vif->vm_p2p->tx_status_flag == WIFINET_TX_STATUS_FAIL && wnet_vif->vm_p2p->act_pkt_retry_count == DEFAULT_P2P_ACTION_RETRY_TIMES)) {
        wnet_vif->vm_p2p->send_tx_status_flag = 1;
        cfg80211_mgmt_tx_status(wnet_vif->vm_wdev, wnet_vif->vm_p2p->cookie, wnet_vif->vm_p2p->raw_action_pkt, wnet_vif->vm_p2p->raw_action_pkt_len, WIFINET_TX_STATUS_FAIL, GFP_KERNEL);
        return;
    }

    if (wnet_vif->vm_p2p_support == 1) {
        if ((wifimac->wm_curchan != WIFINET_CHAN_ERR) && (wnet_vif->vm_p2p->work_channel != WIFINET_CHAN_ERR)
            && (wnet_vif->vm_p2p->work_channel->chan_pri_num == wifimac->wm_curchan->chan_pri_num)
            && (wnet_vif->vm_p2p->action_dialog_token != 0
#ifdef CTS_VERIFIER_GAS
            || wnet_vif->vm_p2p->action_code == WIFINET_ACT_PUBLIC_GAS_REQ || wnet_vif->vm_p2p->action_code == WIFINET_ACT_PUBLIC_GAS_RSP
#endif
            ) && (wnet_vif->vm_p2p->act_pkt_retry_count < DEFAULT_P2P_ACTION_RETRY_TIMES)) {
            pr_debug("%s p2p retry pri_chan:%d\n", __func__, wnet_vif->vm_p2p->work_channel->chan_pri_num);
            //need a lock
            wnet_vif->vm_p2p->act_pkt_retry_count++;
            vm_cfg80211_send_mgmt(wnet_vif, wnet_vif->vm_p2p->action_pkt, wnet_vif->vm_p2p->action_pkt_len);
            os_timer_ex_start_period(&wnet_vif->vm_actsend, wnet_vif->vm_p2p->action_retry_time);
            return;
        }

        if (wnet_vif->vm_p2p->tx_status_flag == WIFINET_TX_STATUS_FAIL) {
            wnet_vif->vm_p2p->send_tx_status_flag = 1;
            cfg80211_mgmt_tx_status(wnet_vif->vm_wdev, wnet_vif->vm_p2p->cookie, wnet_vif->vm_p2p->raw_action_pkt, wnet_vif->vm_p2p->raw_action_pkt_len, WIFINET_TX_STATUS_FAIL, GFP_KERNEL);
        }

        if (wnet_vif->vm_state == WIFINET_S_SCAN) {
            return;
        }
    }
}

static void wifi_mac_tx_timeout_ex (SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;

    pr_debug("vid:%d, mgmt_pkt_retry_count:%d, token:%d\n",
        wnet_vif->wnet_vif_id, wnet_vif->mgmt_pkt_retry_count, wnet_vif->vm_p2p->action_dialog_token);

    if (wnet_vif->mgmt_pkt_retry_count++ < DEFAULT_MGMT_RETRY_TIMES) {
        if (wnet_vif->vm_state == WIFINET_S_AUTH) {
            /* swap the auth type between open and shared when auth timeout after sending auth frame to a wep AP */
            if (wnet_vif->mgmt_pkt_retry_count == DEFAULT_MGMT_RETRY_TIMES -1) {
                if ((wnet_vif->vm_flags & WIFINET_F_PRIVACY) && !(wnet_vif->vm_flags & WIFINET_F_WPA)) {
                    if (wnet_vif->vm_mainsta->sta_authmode == WIFINET_AUTH_SHARED) {
                        wnet_vif->vm_mainsta->sta_authmode = WIFINET_AUTH_OPEN;

                    } else {
                        wnet_vif->vm_mainsta->sta_authmode = WIFINET_AUTH_SHARED;
                    }
                }
            }
            wifi_mac_top_sm(wnet_vif, WIFINET_S_AUTH, WIFINET_FC0_SUBTYPE_DEAUTH);

        } else if (wnet_vif->vm_state == WIFINET_S_ASSOC) {
            wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);

        }  else {
            wnet_vif->mgmt_pkt_retry_count = 0;
            return;
        }

        //pr_debug("wifi_mac->mgmt_pkt_retry_count:%d\n", wifimac->mgmt_pkt_retry_count);
        os_timer_ex_start_period(&wnet_vif->vm_mgtsend, wnet_vif->mgmt_pkt_retry_count * DEFAULT_MGMT_RETRY_INTERVAL);
        return;
    }

    wnet_vif->mgmt_pkt_retry_count = 0;
    if ((wnet_vif->vm_state == WIFINET_S_AUTH) || (wnet_vif->vm_state == WIFINET_S_ASSOC)) {
        vm_cfg80211_indicate_disconnect(wnet_vif);
        wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
    }
}

static int wifi_mac_mgmt_tx_timeout(void* arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *) arg;

    pr_debug("%s vm_state %s, vid:%d", __func__, wifi_mac_state_name[wnet_vif->vm_state], wnet_vif->wnet_vif_id);
    if ((wnet_vif->vm_p2p_support == 1) || (wnet_vif->vm_state > WIFINET_S_CONNECTING)) {
        wifi_mac_add_work_task(wnet_vif->vm_wmac,wifi_mac_tx_timeout_ex, NULL,(SYS_TYPE)arg,0,0,0, 0);
    }

    return OS_TIMER_NOT_REARMED;
}

void wifi_mac_sm_switch (SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;
    enum wifi_mac_state state = (enum wifi_mac_state)param2;

    pr_debug("%s state:%d\n", __func__, state);

    if(state != WIFINET_S_INIT) {
        wifi_mac_top_sm(wnet_vif, state, 0);
    }
}

static int wifi_mac_sm_switch_timeout(void* arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *) arg;

    wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
    return OS_TIMER_NOT_REARMED;
}

static int wifi_mac_act_tx_timeout(void* arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)arg;
    struct wifi_station_tbl *nt = &wnet_vif->vm_sta_tbl;
    struct wifi_station *sta = NULL;
    struct wifi_station *sta_next = NULL;

    if (wnet_vif->vm_p2p_support == 1) {
        WIFINET_DPRINTF(AML_DEBUG_WARNING,"vm_state %s", wifi_mac_state_name[wnet_vif->vm_state]);
        wifi_mac_add_work_task(wnet_vif->vm_wmac,wifi_mac_tx_act_timeout_ex, NULL,(SYS_TYPE)arg,0,0,0, 0);
    }

    if (wnet_vif->vm_opmode == WIFINET_M_STA) {
        if (wnet_vif->vm_mainsta->sta_flags_ext & WIFINET_NODE_MFP_CONFIRM_DEAUTH) {
            wifi_mac_send_sa_query(wnet_vif->vm_mainsta, WIFINET_ACTION_SA_QUERY_REQ, wnet_vif->vm_mainsta->sa_query_seq++);

            if (wnet_vif->vm_mainsta->sa_query_try_count++ < 20) {
                os_timer_ex_start_period(&wnet_vif->vm_actsend, 100);

            } else {
                pr_warn("sta not get pmf sa query rsp, just disconnect\n");
                wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
            }

        }

    } else if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
        WIFINET_NODE_LOCK(nt);
        list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
            if (sta->sta_flags_ext & WIFINET_NODE_MFP_CONFIRM_DEAUTH) {
                wifi_mac_send_sa_query(sta, WIFINET_ACTION_SA_QUERY_REQ, sta->sa_query_seq++);

                if (sta->sa_query_try_count++ < 10) {
                    os_timer_ex_start_period(&wnet_vif->vm_actsend, 100);

                } else {
                    pr_warn("sta:%p not get pmf sa query rsp, just disconnect\n", sta);
                    wifi_mac_sta_disconnect(sta);
                    wifi_mac_rm_sta_from_wds_by_addr(nt,sta->sta_macaddr);
                    list_del_init(&sta->sta_list);
                    WIFINET_NODE_UNLOCK(nt);
                    wifi_mac_free_sta(sta);
                    WIFINET_NODE_LOCK(nt);
                }
            }
        }
        WIFINET_NODE_UNLOCK(nt);
    }

    return OS_TIMER_NOT_REARMED;
}

static int wifi_mac_RocTimeout(void *arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)arg;

    pr_debug("wifi_mac_RocTimeout\n");

    cfg80211_remain_on_channel_expired(wnet_vif->vm_wdev, wnet_vif->remain_on_ch_cookie,
        &wnet_vif->remain_on_ch_channel, GFP_KERNEL);
    return OS_TIMER_NOT_REARMED;
}


static void wifi_mac_check_special_ap(struct wlan_net_vif *wnet_vif)
{
    int i;
    char special_dev_manu_addr[][3]={
        {0x88, 0xc3, 0x97},
        {0x64, 0x64, 0x4a},
        {0x50, 0x64, 0x2b},
    };

    wnet_vif->vm_wmac->wm_signal_power_weak_thresh_narrow = WIFINET_SIGNAL_POWER_WEAK_THRESH_NARROW;
    wnet_vif->vm_wmac->wm_signal_power_weak_thresh_wide = WIFINET_SIGNAL_POWER_WEAK_THRESH_WIDE;

    wnet_vif->vm_wmac->wm_signal_power_bw_change_thresh_narrow = WIFINET_SIGNAL_POWER_BW_CHANGE_THRESH_NARROW;
    wnet_vif->vm_wmac->wm_signal_power_bw_change_thresh_wide = WIFINET_SIGNAL_POWER_BW_CHANGE_THRESH_WIDE;

    for (i = 0; i < ARRAY_SIZE(special_dev_manu_addr); i++) {
        if (!(memcmp(wnet_vif->vm_des_bssid, special_dev_manu_addr[i], 3))) {
            wnet_vif->vm_wmac->wm_signal_power_weak_thresh_narrow = WIFINET_SIGNAL_POWER_WEAK_THRESH_NARROW + 8;
            wnet_vif->vm_wmac->wm_signal_power_weak_thresh_wide = WIFINET_SIGNAL_POWER_WEAK_THRESH_WIDE + 31;

            break;
        }
    }
}

static void
wifi_mac_trigger_sae_task(SYS_TYPE param1,SYS_TYPE param2,
    SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_station *sta = (struct wifi_station *)param1;

    wifi_mac_trigger_sae(sta);
    return;
}


static int
wifi_mac_sub_sm(struct wlan_net_vif *wnet_vif, enum wifi_mac_state nstate, int arg)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta;
    enum wifi_mac_state prestate;
    int mgmt_arg;

    prestate = wnet_vif->vm_state;
    wnet_vif->vm_state = nstate;

    if (prestate == WIFINET_S_CONNECTED && nstate != WIFINET_S_CONNECTED)
    {
        wifimac->wm_runningmask &= ~BIT(wnet_vif->wnet_vif_id);
        wifimac->wm_nrunning--;
    }

    WIFINET_DPRINTF(AML_DEBUG_STATE,"%s state %s->%s", __func__,
            wifi_mac_state_name[prestate],wifi_mac_state_name[nstate]);

    os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_NO_SLEEP);

    if (wnet_vif->vm_opmode != WIFINET_M_HOSTAP && prestate != WIFINET_S_SCAN)
    {
        wifi_mac_cancel_scan(wifimac);
    }

    if (wnet_vif->vm_mainsta == NULL) {
        DPRINTF(AML_DEBUG_WARNING, "%s vid:%d vm_mainsta is NULL, alloc new sta\n", __func__, wnet_vif->wnet_vif_id);

        /*sta side allocate a sta buffer to save ap capabilities and information */
        wnet_vif->vm_mainsta = wifi_mac_get_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, wnet_vif->vm_myaddr);
        if (wnet_vif->vm_mainsta == NULL) {
            DPRINTF(AML_DEBUG_ERROR, "ERROR::%s alloc sta FAIL!!! \n",__func__);
            return -EINVAL;
        }
    }
    sta = wnet_vif->vm_mainsta;

    switch (nstate)
    {
        case WIFINET_S_INIT:
            switch (prestate)
            {
                case WIFINET_S_INIT:
                    break;
                case WIFINET_S_CONNECTED:
                    switch (wnet_vif->vm_opmode)
                    {
                        case WIFINET_M_STA:
                            wifi_mac_sta_leave(sta, 0);
                            break;

                        case WIFINET_M_HOSTAP:
                            wifi_mac_sta_disassoc_in_task(wnet_vif);
                            goto reset;

                        case WIFINET_M_IBSS:
                            wifi_mac_func_to_task(&wnet_vif->vm_sta_tbl,NULL,wnet_vif,0);
                            goto reset;

                        default:
                            goto reset;
                    }
                    break;

                case WIFINET_S_ASSOC:
                    switch (wnet_vif->vm_opmode)
                    {
                        case WIFINET_M_STA:
                            mgmt_arg = WIFINET_REASON_AUTH_LEAVE;
                            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&mgmt_arg);
                            break;
                        case WIFINET_M_HOSTAP:
                            wifi_mac_func_to_task(&wnet_vif->vm_sta_tbl, wifi_mac_sta_deauth, wnet_vif,1);
                            break;
                        default:
                            break;
                    }
                    goto reset;

                case WIFINET_S_SCAN:
                    wifi_mac_cancel_scan(wifimac);
                    goto reset;

                case WIFINET_S_CONNECTING:
                case WIFINET_S_AUTH:
                reset:
                    DPRINTF(AML_DEBUG_CONNECT, "%s %d\n",__func__,__LINE__);
                    wifi_mac_rst_bss(wnet_vif);
                    break;

                case WIFINET_S_MAX:
                    pr_err("need to check twice\n");
                    break;
            }
            break;

        case WIFINET_S_SCAN:
#ifdef INTER_BEACON
            /* start inter-beacon */
            if ((wnet_vif->vm_opmode == WIFINET_M_STA) || (wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT)
                || (wnet_vif->vm_opmode == WIFINET_M_P2P_DEV) || (wnet_vif->vm_opmode == WIFINET_M_IBSS))
            {
                wnet_vif->vm_bcn_intval = WIFINET_INTER_BEACON_INTERVAL;
            }
#endif
            switch (prestate)
            {
                case WIFINET_S_INIT:
                createibss:
                    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
                    {
                        if ((wnet_vif->vm_curchan != WIFINET_CHAN_ERR)
                            && (WIFINET_IS_CHAN_5GHZ(wnet_vif->vm_curchan) || (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH20)))
                        {
                            WIFINET_DPRINTF(AML_DEBUG_STATE,"%s(%d)wifi_mac_create_ap channel = %d\n",
                                    __func__,__LINE__, wnet_vif->vm_curchan->chan_pri_num);
                            wifi_mac_create_wifi(wnet_vif, wnet_vif->vm_curchan);
                        } else {
                            WIFINET_DPRINTF(AML_DEBUG_STATE,"%s(%d) sm:ap mode, vm_des_nssid:%d",
                                __func__,__LINE__, wnet_vif->vm_des_nssid);

                            if (wnet_vif->vm_des_nssid != 0) {
                                wifi_mac_chk_scan(wnet_vif, WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
                                    WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
                            } else {
                                //back to WIFINET_S_INIT if not config ssid
                                wnet_vif->vm_state = prestate;
                            }
                        }
                    }
                    break;
                case WIFINET_S_SCAN:
                case WIFINET_S_CONNECTING:
                case WIFINET_S_AUTH:
                case WIFINET_S_ASSOC:
                    /* if (wifimac->wm_roaming == WIFINET_ROAMING_AUTO) */
                    {
                        WIFINET_DPRINTF(AML_DEBUG_STATE,"%s(%d) sm:scan/connecting/auth/assoc:",  __func__,__LINE__);

                        wifi_mac_chk_scan(wnet_vif, WIFINET_SCANCFG_ACTIVE| WIFINET_SCANCFG_FLUSH |arg,
                            wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
                    }
                    break;

                case WIFINET_S_CONNECTED:
                    if (wnet_vif->vm_opmode == WIFINET_M_STA)
                    {
                        wifi_mac_sta_leave(sta, 0);
                        wnet_vif->vm_flags &= ~WIFINET_F_SIBSS;
                    }
                    else if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
                    {
                        wifi_mac_func_to_task(&wnet_vif->vm_sta_tbl,NULL, wnet_vif,0);

                        WIFINET_DPRINTF(AML_DEBUG_STATE,"%s(%d) sm:ibss", __func__, __LINE__);
                        wifi_mac_chk_scan(wnet_vif, WIFINET_SCANCFG_ACTIVE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
                    }
                    else
                    {
                        wifi_mac_sta_disassoc_in_task(wnet_vif);
                        goto createibss;
                    }
                    break;
                case WIFINET_S_MAX:
                    pr_err("need to check twice\n");
                    break;
            }
#ifdef INTER_BEACON
            /* start inter-beacon */
            /* WIFINET_INTER_BEACON_INTERVAL is also inter-beacon flag */
            if (wnet_vif->vm_bcn_intval == WIFINET_INTER_BEACON_INTERVAL)
            {
                wifi_mac_add_work_task(wifimac, wifi_mac_sta_beacon_init_ex,
                    NULL, (SYS_TYPE)wifimac,
                    (SYS_TYPE)wnet_vif->wnet_vif_id, 0,
                    (SYS_TYPE)wnet_vif,
                    (SYS_TYPE)wnet_vif->wnet_vif_replaycounter);
            }
#endif
            break;

        case WIFINET_S_AUTH:
            KASSERT(wnet_vif->vm_opmode == WIFINET_M_STA, ("switch to %s state when operating in mode %u",
                wifi_mac_state_name[nstate], wnet_vif->vm_opmode));

            if (prestate != nstate) {
                wnet_vif->mgmt_pkt_retry_count = 0;
            }

            switch (prestate) {
                case WIFINET_S_SCAN:
                case WIFINET_S_CONNECTING:
                    if (sta->sta_authmode == WIFINET_AUTH_SAE) {
#if (KERNEL_VERSION(4, 17, 0) <= LINUX_VERSION_CODE) || (defined WPA3_PATCH)
                       //wifi_mac_trigger_sae(sta);
                       wifi_mac_add_work_task(wifimac, wifi_mac_trigger_sae_task, NULL, (SYS_TYPE)sta, 0, 0, 0, 0);
#endif

                    } else  {
                        mgmt_arg = WIFINET_AUTH_SHARED_REQUEST;
                        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&mgmt_arg);
                    }
                    break;

                case WIFINET_S_AUTH:
                case WIFINET_S_ASSOC:
                    if ((prestate == WIFINET_S_ASSOC)
                        && (sta->sta_authmode == WIFINET_AUTH_SAE)
                        && (arg == WIFINET_STATUS_INVALID_PMKID)) {
#if (KERNEL_VERSION(4, 17, 0) <= LINUX_VERSION_CODE) || (defined WPA3_PATCH)
                        //wifi_mac_trigger_sae(sta);
                        wifi_mac_add_work_task(wifimac, wifi_mac_trigger_sae_task, NULL, (SYS_TYPE)sta, 0, 0, 0, 0);
#endif

                    } else if (arg == WIFINET_FC0_SUBTYPE_AUTH) {
                        mgmt_arg = WIFINET_AUTH_SHARED_CHALLENGE;
                        WIFINET_DPRINTF(AML_DEBUG_STATE, "%d\n", mgmt_arg);
                        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&mgmt_arg);

                    } else if (arg == WIFINET_FC0_SUBTYPE_DEAUTH) {
                        mgmt_arg = WIFINET_AUTH_SHARED_REQUEST;
                        WIFINET_DPRINTF(AML_DEBUG_STATE, "%d\n", mgmt_arg);
                        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&mgmt_arg);

                    }

                    break;

                case WIFINET_S_CONNECTED:
                    switch (arg) {
                        case WIFINET_FC0_SUBTYPE_AUTH:
                            mgmt_arg = WIFINET_AUTH_SHARED_CHALLENGE;
                            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&mgmt_arg);
                            WIFINET_DPRINTF(AML_DEBUG_STATE,"%s\n","");
                            wnet_vif->vm_state = prestate;
                            break;

                        case WIFINET_FC0_SUBTYPE_DEAUTH:
                            mgmt_arg = WIFINET_REASON_ASSOC_LEAVE;
                            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&mgmt_arg);
                            wifi_mac_sta_leave(sta, 0);
                            wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
                            break;
                    }
                    break;

                default:
                    wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
                    break;
            }
            break;

        case WIFINET_S_ASSOC:
            KASSERT(wnet_vif->vm_opmode == WIFINET_M_STA, ("switch to %s state when operating in mode %u",
                wifi_mac_state_name[nstate], wnet_vif->vm_opmode));

            if (prestate != nstate) {
                wnet_vif->mgmt_pkt_retry_count = 0;
            }

            switch (prestate)
            {
                case WIFINET_S_AUTH:
                case WIFINET_S_ASSOC:
                    mgmt_arg = 0;
                    wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_ASSOC_REQ, (void *)&mgmt_arg);
                    break;

                case WIFINET_S_CONNECTED:
                    mgmt_arg = WIFINET_REASON_ASSOC_LEAVE;
                    wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DISASSOC, (void *)&mgmt_arg);
                    wifi_mac_sta_leave(sta, arg);
                    wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
                    break;

                default:
                    wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
                    pr_err("need to check twice\n");
                    break;
            }
            break;

        case WIFINET_S_CONNECTED:
            switch (prestate)
            {
                case WIFINET_S_INIT:
                    if (wnet_vif->vm_opmode == WIFINET_M_MONITOR ||
                        wnet_vif->vm_opmode == WIFINET_M_WDS ||
                        wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
                    {
                        wifi_mac_create_wifi(wnet_vif, wnet_vif->vm_curchan);
                    }
                    break;
                case WIFINET_S_CONNECTING:
                    if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
                    {
                        wifi_mac_notify_nsta_connect(sta, arg == WIFINET_FC0_SUBTYPE_ASSOC_RESP);
                    }
                    break;
                case WIFINET_S_MAX:
                    pr_err("need to check twice\n");
                    break;
                default:
                    break;
            }

            if (prestate != WIFINET_S_CONNECTED
                && ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_flags_ext & WIFINET_FEXT_SWBMISS)))
            {
                wifi_mac_add_work_task(wifimac,wifi_mac_set_beacon_miss, NULL, (SYS_TYPE)wifimac,
                    (SYS_TYPE)wnet_vif->wnet_vif_id, ENABLE, (SYS_TYPE)wnet_vif, WIFINET_BMISS_THRS/* period, ms*/);
                wifimac->wm_syncbeacon = 1;

                /*set Conect_AP beacon interval for fw */
                wifi_mac_add_work_task(wifimac, wifi_mac_sta_beacon_init_ex, NULL, (SYS_TYPE)wifimac,
                    (SYS_TYPE)wnet_vif->wnet_vif_id, 0, (SYS_TYPE)wnet_vif, (SYS_TYPE)wnet_vif->wnet_vif_replaycounter);

                /*Set keep alive(null data) period for fw*/
                wifi_mac_add_work_task(wifimac,wifi_mac_sta_keep_alive_ex, NULL, (SYS_TYPE)wifimac,
                    (SYS_TYPE)wnet_vif->wnet_vif_id, ENABLE, (SYS_TYPE)wnet_vif, WIFINET_KEEP_ALIVE/* period, ms*/);
                wifimac->wm_lastroaming = jiffies;

                wifi_mac_check_special_ap(wnet_vif);

                if (!(wnet_vif->vm_flags & WIFINET_F_PRIVACY)
                    && (wifimac->recovery_stat == WIFINET_RECOVERY_VIF_UP)) {
                    sta->connect_status = CONNECT_DHCP_GET_ACK;
                    wifimac->recovery_stat = WIFINET_RECOVERY_END;
                }
                wifi_mac_set_channel_rssi(wifimac, (unsigned char)(wnet_vif->vm_mainsta->sta_avg_bcn_rssi));

                pr_debug("****************************************************\n");
                pr_debug("sta connect ok!!! AP CHANNEL:%d, CENTER_CHAN:%d, BW:%d, SSID:%s, BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    wnet_vif->vm_curchan->chan_pri_num, wifi_mac_Mhz2ieee(wnet_vif->vm_curchan->chan_cfreq1, 0), sta->sta_chbw,
                    wnet_vif->vm_des_ssid[0].ssid, wnet_vif->vm_des_bssid[0], wnet_vif->vm_des_bssid[1], wnet_vif->vm_des_bssid[2],
                    wnet_vif->vm_des_bssid[3], wnet_vif->vm_des_bssid[4], wnet_vif->vm_des_bssid[5]);
                pr_debug("****************************************************\n");
                g_auto_gain_base = 0;
            }

            wifi_mac_sta_auth(sta);
            break;
        case WIFINET_S_MAX:
            pr_err("-------------error: need to check twice-------------------\n");
            break;

        default:
            break;
    }

    if (nstate == WIFINET_S_CONNECTED && prestate != WIFINET_S_CONNECTED)
    {
        wifimac->wm_runningmask |= BIT(wnet_vif->wnet_vif_id);
        wifimac->wm_nrunning++;

        wifi_mac_set_scan_time(wnet_vif);
    #ifdef CONFIG_CONCURRENT_MODE
        if ((wifimac->wm_nrunning > 1) && !concurrent_check_is_vmac_same_pri_channel(wifimac)) {
            if ((wnet_vif->vm_p2p_support == 1) || (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)) {
                wifimac->wm_vsdb_slot = CONCURRENT_SLOT_P2P;
            } else {
                wifimac->wm_vsdb_slot = CONCURRENT_SLOT_STA;
            }
#if 0
            if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode()) && concurrent_check_vmac_is_AP(wifimac)) {
                /*softap and sta concurrent as scc, no need vsdb*/
                wifimac->wm_vsdb_slot = CONCURRENT_SLOT_NONE;
            } else {
                wifi_mac_add_work_task(wifimac, wifi_mac_set_vsdb, NULL, (SYS_TYPE)wifimac, 0, ENABLE, (SYS_TYPE)wnet_vif, 0);
            }
#endif
            if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode()) && concurrent_check_vmac_is_AP(wifimac)) {
                struct wifi_channel *main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);

                channel_switch_announce_trigger(wifimac, main_vmac_chan);
            }
            wifi_mac_set_vsdb_task(wifimac, wnet_vif, ENABLE);
        }
    #endif
    }
    else if (prestate == WIFINET_S_CONNECTED && nstate != WIFINET_S_CONNECTED)
    {
        wifi_mac_set_scan_time(wnet_vif);
    #ifdef CONFIG_CONCURRENT_MODE
        if ((wifimac->wm_nrunning == 1) && (wifimac->wm_vsdb_slot != CONCURRENT_SLOT_NONE)) {
            wifimac->wm_vsdb_slot = CONCURRENT_SLOT_NONE;
            wifimac->wm_vsdb_flags = 0;
            wifimac->vsdb_mode_set_noa_enable = 0;
            wifi_mac_restore_wnet_vif_channel_task(wnet_vif);
            wifi_mac_set_vsdb_task(wifimac, wnet_vif, ENABLE);
        }
    #endif
    }

    return 0;
}

int wifi_mac_top_sm(struct wlan_net_vif *wnet_vif,
        enum wifi_mac_state nstate, int arg)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    enum wifi_mac_opmode opmode = wnet_vif->vm_opmode;
    enum wifi_mac_state prestate = wnet_vif->vm_state;
    int rc = 0;

    WIFINET_VMACS_LOCK(wifimac);

    switch (nstate) {
        case WIFINET_S_SCAN:
            wifi_mac_sub_sm(wnet_vif, nstate, arg);
            break;

        case WIFINET_S_CONNECTED:
            if (opmode == WIFINET_M_HOSTAP) {
                wifimac->wm_syncbeacon = 0;
                wifi_mac_add_work_task(wifimac, wifi_mac_beacon_alloc_ex, NULL, (SYS_TYPE)wifimac,
                    (SYS_TYPE)wnet_vif->wnet_vif_id,0, (SYS_TYPE)wnet_vif, (SYS_TYPE)wnet_vif->wnet_vif_replaycounter);

                pr_debug("****************************************************\n");
                pr_debug("start bss ok!!! AP CHANNEL:%d, BW:%d, SSID:%s, BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    wnet_vif->vm_curchan->chan_pri_num, wnet_vif->vm_curchan->chan_bw, wnet_vif->vm_des_ssid[0].ssid,
                    wnet_vif->vm_des_bssid[0], wnet_vif->vm_des_bssid[1], wnet_vif->vm_des_bssid[2],
                    wnet_vif->vm_des_bssid[3], wnet_vif->vm_des_bssid[4], wnet_vif->vm_des_bssid[5]);
                pr_debug("****************************************************\n");

            }
            wifi_mac_sub_sm(wnet_vif, nstate, arg);

            pr_debug("%s(%d) wm_running=%d, wnet_vif_mode %d vm_bandwidth:%d\n",
                __func__, __LINE__, wifimac->wm_nrunning, wnet_vif->vm_mac_mode, wnet_vif->vm_bandwidth);
            break;

        case WIFINET_S_INIT:
            wifi_mac_sub_sm(wnet_vif, nstate, arg);
            wnet_vif->vm_des_nssid = 0;
            if (opmode == WIFINET_M_HOSTAP) {
                pr_debug("%s(%d) interface down\n", __func__, __LINE__);
                wnet_vif->vm_curchan = WIFINET_CHAN_ERR;
                memset(wnet_vif->vm_des_ssid, 0, IV_SSID_SCAN_AMOUNT*sizeof(struct wifi_mac_ScanSSID));
                wifimac->drv_priv->drv_ops.down_interface(wifimac->drv_priv, wnet_vif->wnet_vif_id);
            }
            break;

        case WIFINET_S_CONNECTING:
            if ((opmode != WIFINET_M_STA) && (opmode != WIFINET_M_IBSS)) {
                WIFINET_DPRINTF(AML_DEBUG_STATE,"%s","");
                rc= -EINVAL;
                goto __unlock;
            }
            wifi_mac_sub_sm(wnet_vif, nstate, arg);
            break;

        default:
            wifi_mac_sub_sm(wnet_vif, nstate, arg);
            break;
    }

__unlock:
    WIFINET_VMACS_UNLOCK(wifimac);

    if ((nstate == WIFINET_S_CONNECTED) && (prestate != WIFINET_S_CONNECTED)) {
        wifi_mac_pwrsave_wnet_vif_connect(wnet_vif);

    } else if ((prestate == WIFINET_S_CONNECTED) && (nstate != WIFINET_S_CONNECTED)) {
        wifi_mac_pwrsave_wnet_vif_disconnect(wnet_vif);
    }

    if (VM_WIFI_CONNECT_STATE(nstate)  && !VM_WIFI_CONNECT_STATE(prestate)) {
        wifi_mac_connect_start(wifimac);
        if ((opmode == WIFINET_M_STA) || (opmode == WIFINET_M_P2P_CLIENT)) {
            pr_debug(" connect_start: %d\n",__LINE__);
        }

    } else if (VM_WIFI_CONNECT_STATE(prestate)  && !VM_WIFI_CONNECT_STATE(nstate)) {
        wifi_mac_connect_end(wifimac);
        pr_debug("%s(%d): connect end =%d, wnet_vif_mode %d\n", __func__,
            __LINE__, wifimac->wm_nrunning, wnet_vif->vm_mac_mode);
    }

    return rc;
}


void wifi_mac_com_vattach(struct wlan_net_vif *wnet_vif)
{
    wnet_vif->vm_rtsthreshold = WIFINET_RTS_MAX;
    wnet_vif->vm_fragthreshold = WIFINET_FRAGMT_THRESHOLD_MAX;

    os_timer_ex_initialize(&wnet_vif->vm_mgtsend, WIFINET_TRANS_WAIT, wifi_mac_mgmt_tx_timeout, wnet_vif);
    os_timer_ex_initialize(&wnet_vif->vm_sm_switch, WIFINET_TRANS_WAIT, wifi_mac_sm_switch_timeout, wnet_vif);
    os_timer_ex_initialize(&wnet_vif->vm_actsend, WIFINET_TRANS_WAIT, wifi_mac_act_tx_timeout, wnet_vif);
    os_timer_ex_initialize(&wnet_vif->vm_roc_timer, 200, wifi_mac_RocTimeout, wnet_vif);

    /*set tx rate(11n mcs0 or 11b/g rate) for mgmt or
      multicast frames, set default max retry times*/
    wifi_mac_rate_vattach(wnet_vif);
}

void wifi_mac_com_vdetach(struct wlan_net_vif *wnet_vif)
{
    os_timer_ex_del(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);
    os_timer_ex_del(&wnet_vif->vm_sm_switch, CANCEL_SLEEP);
    os_timer_ex_del(&wnet_vif->vm_actsend, CANCEL_SLEEP);
    os_timer_ex_del(&wnet_vif->vm_roc_timer, CANCEL_SLEEP);

    if (wnet_vif->vm_aclop != NULL)
        wnet_vif->vm_aclop->iac_detach(wnet_vif);
}

const char *wifi_mac_opmode_name[] =
{
    "IBSS",
    "STA",
    "HOSTAP",
    "MONITOR",
    "WDS",
    "CLIENT ",
    "P2P_GO",
    "P2P_DEV",
};

static int
__wifi_mac_ioctrl(struct net_device *dev, void __user *data, int cmd)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);

    DPRINTF(AML_DEBUG_IOCTL, "%s %d cmd 0x%x\n", __func__, __LINE__, cmd);
    switch (cmd) {
        case SIOCANDROID_PRIV:
            //return wifi_mac_stop(vmac->vm_dev);
            return  aml_android_priv_cmd(wnet_vif, data, cmd);

        case SIOCG80211STATS:
            return copy_to_user(data, &wnet_vif->vif_sts, sizeof (wnet_vif->vif_sts)) ? -EFAULT : 0;

        case SIOC80211IFDESTROY:
            if (!capable(CAP_NET_ADMIN))
                return -EPERM;

            wifi_mac_stop(wnet_vif->vm_ndev);
            wifi_mac_vmac_delt(wnet_vif);
            wifi_mac_delt_vmac_id(wnet_vif->vm_wmac,wnet_vif->wnet_vif_id);
            return 0;
        default:
            break;
    }
    return -EOPNOTSUPP;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
int wifi_mac_ioctrl_wrapper(struct net_device *dev, struct ifreq *ifr,
    void __user *data, int cmd)
{
    return __wifi_mac_ioctrl(dev,data,cmd);
}
#else

int wifi_mac_ioctrl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    return __wifi_mac_ioctrl(dev,ifr->ifr_data,cmd);
}

#endif

static int
wifi_set_mac_address(struct net_device *dev, void *addr)
{
    int ret = 0;
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    struct sockaddr *sa = (struct sockaddr *)addr;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (wnet_vif->vm_state > WIFINET_S_CONNECTING) {
        //other state need to reup interface
        return ret;
    }

    memcpy(wnet_vif->vm_myaddr, sa->sa_data, MAC_ADDR_LEN);
    if (wnet_vif->vm_mainsta != NULL) {
        memcpy(wnet_vif->vm_mainsta->sta_macaddr, sa->sa_data, MAC_ADDR_LEN);
    }
    dev_addr_mod(dev, 0, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
    wnet_vif->vm_wdev->wiphy->addresses = (struct mac_address *)(wnet_vif->vm_myaddr);

    wifimac->drv_priv->drv_ops.set_macaddr(wifimac->drv_priv, wnet_vif->wnet_vif_id, wnet_vif->vm_myaddr);
    pr_debug("%s change mac_addr to:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, wnet_vif->vm_myaddr[0],
        wnet_vif->vm_myaddr[1], wnet_vif->vm_myaddr[2], wnet_vif->vm_myaddr[3], wnet_vif->vm_myaddr[4], wnet_vif->vm_myaddr[5]);
    return ret;
}

void wnet_vif_vht_cap_init( struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    KASSERT(wnet_vif,( "vht cap init should not be null\n"));

    wifimac->wm_flags_ext2 = WIFINET_VHTCAP_MAX_MPDU_LEN_7991 \
                | WIFINET_VHTCAP_RX_LDPC \
                /* can't process stbc pkt, just remove stbc */
                | WIFINET_VHTCAP_RX_STBC \
                | WIFINET_VHTCAP_MAX_AMPDU_LEN_EXP;
    wifimac->wm_flags |=WIFINET_F_LDPC;

    wnet_vif->vm_sgi = wifi_mac_com_has_extflag(wifimac, WIFINET_FEXT_SHORTGI_ENABLE);
    wnet_vif->vm_ldpc = GET_VHT_CAP_RX_LPDC(wifimac->wm_flags_ext2);
    wnet_vif->vm_tx_stbc = GET_VHT_CAP_TX_STBC(wifimac->wm_flags_ext2);
    wnet_vif->vm_rx_stbc = GET_VHT_CAP_RX_STBC(wifimac->wm_flags_ext2);

    pr_debug("vht cap init: sgi 0x%x, ldpc 0x%x, tx_stbc 0x%x, rx_stbc 0x%x \n",
        wnet_vif->vm_sgi, wnet_vif->vm_ldpc, wnet_vif->vm_tx_stbc, wnet_vif->vm_rx_stbc);

    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
    {
        /*
        wnet_vif->vm_vhtcap |= (WIFINET_VHTCAP_SU_BFMER | WIFINET_VHTCAP_MU_BFMER);
        */
    }
    else
    {
#ifdef SU_BF
        wifimac->wm_flags_ext2 |= WIFINET_VHTCAP_SU_BFMEE;
#endif

#ifdef MU_BF
        wifimac->wm_flags_ext2 |= WIFINET_VHTCAP_MU_BFMEE;
#endif
    }
}

void wifi_mac_set_reg_val(unsigned int reg_addr, enum wifi_mac_bwc_width bw)
{
    unsigned int reg_val = 0;
    struct hw_interface* hw_if = hif_get_hw_interface();

    if (PHY_AGC_BUSY_FSM == reg_addr) {
        reg_val = hw_if->hif_ops.hi_read_word(PHY_AGC_BUSY_FSM);
        hw_if->hif_ops.hi_write_word(PHY_AGC_BUSY_FSM, reg_val | 0xf0);

    } else if (MAC_REG_BASE == reg_addr) {
        reg_val = hw_if->hif_ops.hi_read_word(MAC_REG_BASE);
        hw_if->hif_ops.hi_write_word(MAC_REG_BASE, reg_val & (~(1<<18)));

    } else if (MAC_RXPKT_CONTROL45 == reg_addr) {
        reg_val = hw_if->hif_ops.hi_read_word(MAC_RXPKT_CONTROL45);
        reg_val &= ~(1<<31);
        reg_val |= 0x800848;//bit23 & bit11 & bit6 & bit3 set to 1
        reg_val &= ~(1 << 17 | 1 << 18);
        hw_if->hif_ops.hi_write_word(MAC_RXPKT_CONTROL45, reg_val | (bw << 17));//bit17 & bit18 set ap's bw

    } else {
        pr_warn("%s, No valid reg addr\n", __func__);
    }
}

int wifi_mac_setup(struct wifi_mac *wifimac,
                    struct wlan_net_vif *wnet_vif, int opmode)
{
    wnet_vif->vm_wmac = wifimac;
    wnet_vif->vm_flags = wifimac->wm_flags;
    wnet_vif->vm_flags_ext = wifimac->wm_flags_ext;

    wnet_vif->vm_caps = wifimac->wm_caps &
        ~(WIFINET_C_IBSS | WIFINET_C_HOSTAP  |WIFINET_C_MONITOR);

    wnet_vif->vm_htcap = WIFINET_HTCAP_SUPPORTCBW40 \
        /* can't process stbc pkt, just remove stbc */
        | WIFINET_HTCAP_C_RXSTBC_1SS \
        | WIFINET_HTCAP_USEDSSSCCK_40M \
        | WIFINET_HTCAP_DISABLE_SMPS \
        | WIFINET_HTCAP_C_MAXAMSDUSIZE;

    /*Init bandwidth and second channel offset. */
    wnet_vif->vm_curchan = WIFINET_CHAN_ERR;
    wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_NA;
    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH80;

    pr_debug("<running> %s %d opmode=%d\n",__func__,__LINE__,opmode);
    switch (opmode)
    {
        case WIFINET_M_STA:
            /* set flag to start beacon miss timer for sta */
            wnet_vif->vm_flags_ext |= WIFINET_FEXT_SWBMISS;
            break;
        case WIFINET_M_IBSS:
            wnet_vif->vm_caps |= WIFINET_C_IBSS;
            wnet_vif->vm_flags &= ~WIFINET_F_RDG;
            break;
        case WIFINET_M_HOSTAP:
            wnet_vif->vm_caps |= WIFINET_C_HOSTAP;
            break;
        case WIFINET_M_MONITOR:
            wnet_vif->vm_caps |= WIFINET_C_MONITOR;
            wnet_vif->vm_flags &= ~WIFINET_F_RDG;
            break;
        case WIFINET_M_WDS:
            wnet_vif->vm_caps |= WIFINET_C_WDS;
            wnet_vif->vm_flags &= ~WIFINET_F_RDG;
            wnet_vif->vm_flags_ext |= WIFINET_FEXT_WDS;
            break;
    }
    wnet_vif->vm_opmode = opmode;
    wnet_vif->vm_chanchange_count = 0;
    wnet_vif->vm_dtim_period = WIFINET_DTIM_DEFAULT;
    wnet_vif->mgmt_pkt_retry_count = 0;

    wifi_mac_security_vattach(wnet_vif);
    wifi_mac_pmkid_vattach(wnet_vif);
    wifi_mac_sta_vattach(wnet_vif);
    /*init power save para&state and timer for ap&sta and p2p*/
    wifi_mac_pwrsave_vattach(wnet_vif);
    wifi_mac_com_vattach(wnet_vif);
    /*set default time for scan operation */
    wifi_mac_scan_vattach(wnet_vif);
    wnet_vif_vht_cap_init(wnet_vif);

    /* Set the VHT  rate information
    *  11ac spec states it is mandatory to support
    *  MCS 0-7 and NSS=1 */
    wifi_mac_vht_rate_init(wnet_vif,VHT_MCSMAP_NSS1_MCS0_9,
        0, VHT_MCSMAP_NSS1_MCS0_9 );
    return 1;
}



int vm_wlan_net_vif_setup_forchvif(struct wifi_mac *wifimac,
        struct wlan_net_vif *wnet_vif, const char *name,  int opmode)
{
    struct net_device *dev = wnet_vif->vm_ndev;
    if (name != NULL)
        strncpy(dev->name, name, sizeof(dev->name) - 1);
    wnet_vif->vm_caps = wifimac->wm_caps;
    wnet_vif->vm_flags_ext &= ~WIFINET_FEXT_SWBMISS;
    wnet_vif->vm_flags = wifimac->wm_flags;

    pr_debug("<running> %s %d opmode=%d\n",__func__,__LINE__,opmode);
    switch (opmode)
    {
        case WIFINET_M_STA:
            /* set flag to start beacon miss timer for sta */
            wnet_vif->vm_flags_ext |= WIFINET_FEXT_SWBMISS;
            break;
        case WIFINET_M_IBSS:
            wnet_vif->vm_flags &= ~WIFINET_F_RDG;
            break;
        case WIFINET_M_HOSTAP:
            break;
        case WIFINET_M_MONITOR:
            wnet_vif->vm_flags &= ~WIFINET_F_RDG;
            break;
        case WIFINET_M_WDS:
            wnet_vif->vm_flags &= ~WIFINET_F_RDG;
            wnet_vif->vm_flags_ext |= WIFINET_FEXT_WDS;
            break;
    }
    wnet_vif->vm_opmode = opmode;
    wnet_vif->vm_chanchange_count = 0;
    wifi_mac_sta_vattach(wnet_vif);
    wifi_mac_pwrsave_vattach(wnet_vif);
    wifi_mac_com_vattach(wnet_vif);
    wifi_mac_scan_vattach(wnet_vif);
    wnet_vif_vht_cap_init(wnet_vif);
    return 1;
}

static void aml_reg_notifier(struct wiphy *wiphy,
               struct regulatory_request *request)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    unsigned char source_code[5][10] = {"core", "user", "driver", "countryie"};
    unsigned int delay_ms = 0;

    if (!request)
        return;

    while ((wifimac->wm_flags & WIFINET_F_DOTH) && (wifimac->wm_flags & WIFINET_F_CHANSWITCH) && delay_ms < 1000) {
        msleep(50);
        delay_ms+=50;
    }

    AML_OUTPUT("vid:%d, regdom set by %s, country <%s>, wiphy alpha2 %s\n",
        wnet_vif->wnet_vif_id, source_code[request->initiator], request->alpha2, wiphy->regd->alpha2);

    switch (request->initiator) {
    case NL80211_REGDOM_SET_BY_CORE:
        break;
    case NL80211_REGDOM_SET_BY_DRIVER:
        break;
    case NL80211_REGDOM_SET_BY_USER:
        wifi_mac_set_country(wifimac, request->alpha2);
        break;
    case NL80211_REGDOM_SET_BY_COUNTRY_IE:
        break;
    }
}


static void aml_regd_init(
          struct wiphy *wiphy,
          void (*reg_notifier)(struct wiphy *wiphy,
              struct regulatory_request *request))
{
    wiphy->reg_notifier = reg_notifier;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
        /*
        *  To support GO working on DFS channel,
        *  enable REGULATORY_IGNORE_STALE_KICKOFF flag.
        *  It will not handle kernel regdomain change disconnect
        */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(6, 3, 12)
        wiphy->regulatory_flags |= (REGULATORY_IGNORE_STALE_KICKOFF);
#else
        wiphy->regulatory_flags |= (REGULATORY_WIPHY_SELF_MANAGED >> 1);
#endif
#endif
}
int
vm_wlan_net_vif_register(struct wlan_net_vif *wnet_vif, char* name)
{
    /* wireless net device */
    struct net_device *dev = wnet_vif->vm_ndev;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (name != NULL)
        strncpy(dev->name, name, sizeof(dev->name) - 1);

    /*set net device operations, refer to 'wifi_mac_netdev_ops' for details.
        For example, if wlan0 up/down, tcp/ip_xmit_start ...*/
    dev->netdev_ops = &wifi_mac_netdev_ops;
    dev->hard_header_len = DEFAULT_HARD_HDR_LEN;
#ifdef CONFIG_WIRELESS_EXT
    dev->wireless_handlers = &w1_iw_handle;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    dev->priv_destructor = free_netdev;
#else
    dev->destructor = free_netdev;
#endif

    if(wifimac->wm_caps & WIFINET_C_HWCSUM)
    {
        netdev_setcsum(dev,1);
    }
    else
    {
        netdev_setcsum(dev,0);
    }
    /*register a wireless net dev for cfg80211 */
    if (wifi_mac_alloc_wdev(wnet_vif, vm_cfg80211_get_parent_dev()) < 0)
        ERROR_DEBUG_OUT("ERROR alloc wdev\n");

    DPRINTF(AML_DEBUG_CONNECT, "%s %d\n",__func__,__LINE__);
    wifi_mac_pwrsave_latevattach(wnet_vif);

    WIFINET_QLOCK(wifimac);
    list_add_tail(&wnet_vif->vm_next, &wifimac->wm_wnet_vifs);
    wifimac->wnet_vif_num++;
    WIFINET_QUNLOCK(wifimac);

    dev_addr_mod(dev, 0, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
    AML_OUTPUT("<running>\n");
    aml_regd_init(wnet_vif->vm_wdev->wiphy, aml_reg_notifier);
    return 1;
}


void
vm_wlan_net_vif_unregister(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct net_device *dev = wnet_vif->vm_ndev;
    struct wireless_dev *wdev = wnet_vif->vm_wdev;
    unsigned char vid = wnet_vif->wnet_vif_id;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wdev);

    WIFINET_QLOCK(wifimac);
    list_del_init(&wnet_vif->vm_next);
    wifimac->wnet_vif_num--;
    WIFINET_QUNLOCK(wifimac);

    wifi_mac_com_vdetach(wnet_vif);
    wifi_mac_security_vdetach(wnet_vif);
    wifi_mac_pmkid_detach(wnet_vif);
    wifi_mac_pwrsave_vdetach(wnet_vif);
    wifi_mac_sta_vdetach(wnet_vif);

#ifdef CONFIG_P2P
    vm_p2p_dettach(pwdev_priv);
#endif //CONFIG_P2P

    unregister_netdev(dev);
    vm_wdev_free(wdev, vid);
}

static struct net_device_stats *
wifi_mac_getstats(struct net_device *dev)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    return &wnet_vif->vm_devstats;
}


static int
wifi_mac_change_mtu(struct net_device *dev, int mtu)
{
    if (!(WIFINET_MTU_MIN < mtu && mtu <= WIFINET_MTU_MAX))
        return -EINVAL;
    dev->mtu = mtu;
    return 0;
}

static void
wifi_mac_build_country_ie_2G(struct wifi_mac *wifimac)
{
    struct wifi_channel *c;
    int channel_num = 0;
    int first_channel = 0;
    int max_power = 0;
    int i = 0 ;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++) {
        c = &wifimac->wm_channels[i];

        if (WIFINET_IS_CHAN_2GHZ(c) && (c->chan_bw == WIFINET_BWC_WIDTH20)) {
            if (!first_channel) {
                first_channel = c->chan_pri_num;
                max_power = c->chan_maxpower;
            }
            channel_num ++;
        }
    }
    WIFI_CHANNEL_UNLOCK(wifimac);

    wifimac->wm_countryinfo.country_triplet[0] = first_channel;
    wifimac->wm_countryinfo.country_triplet[1] = channel_num;
    wifimac->wm_countryinfo.country_triplet[2] = max_power;
    wifimac->wm_countryinfo.country_len += 3;
}

static void
wifi_mac_build_country_ie_5G(struct wifi_mac *wifimac)
{
    struct wifi_channel *c;
    int i = 0 ;
    int triplet_index = 0;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++) {
        c = &wifimac->wm_channels[i];

        if (WIFINET_IS_CHAN_5GHZ(c) && (c->chan_bw == WIFINET_BWC_WIDTH20)) {
            wifimac->wm_countryinfo.country_triplet[triplet_index++] = c->chan_pri_num;
            wifimac->wm_countryinfo.country_triplet[triplet_index++] = 1;
            wifimac->wm_countryinfo.country_triplet[triplet_index++] = c->chan_maxpower;
            wifimac->wm_countryinfo.country_len += 3;
        }
    }
    WIFI_CHANNEL_UNLOCK(wifimac);
}

void
wifi_mac_build_country_ie_2G_and_5G(struct wifi_mac *wifimac)
{
    struct wifi_channel *c;
    int i = 0 ;
    int channel_num = 0;
    int first_channel = 0;
    int max_power = 0;
    int triplet_index = 0;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++) {
        c = &wifimac->wm_channels[i];

        if (WIFINET_IS_CHAN_2GHZ(c) && (c->chan_bw == WIFINET_BWC_WIDTH20)) {
            if (!first_channel) {
                first_channel = c->chan_pri_num;
                max_power = c->chan_maxpower;
            }

            if (max_power < c->chan_maxpower) {
                max_power = c->chan_maxpower;
            }
            channel_num ++;
        }
    }

    if (channel_num != 0) {
        wifimac->wm_countryinfo.country_triplet[triplet_index++] = first_channel;
        wifimac->wm_countryinfo.country_triplet[triplet_index++] = channel_num;
        wifimac->wm_countryinfo.country_triplet[triplet_index++] = max_power;
        wifimac->wm_countryinfo.country_len += 4;
    }

    for (i = 0; i < wifimac->wm_nchans; i++) {
        c = &wifimac->wm_channels[i];

        if (WIFINET_IS_CHAN_5GHZ(c) && (c->chan_bw == WIFINET_BWC_WIDTH20)) {
            if (!(c->chan_flags & WIFINET_CHAN_DFS)) {
                wifimac->wm_countryinfo.country_triplet[triplet_index++] = c->chan_pri_num;
                wifimac->wm_countryinfo.country_triplet[triplet_index++] = 1;
                wifimac->wm_countryinfo.country_triplet[triplet_index++] = c->chan_maxpower;
                wifimac->wm_countryinfo.country_len += 3;
            }
        }
    }
    WIFI_CHANNEL_UNLOCK(wifimac);
}

void
wifi_mac_build_country_ie(struct wlan_net_vif * wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct _wifi_mac_country_iso country;

    memset(&wifimac->wm_countryinfo, 0, sizeof(wifimac->wm_countryinfo));
    wifimac->wm_countryinfo.country_id = WIFINET_ELEMID_COUNTRY;
    wifimac->wm_countryinfo.country_len = 0;

    wifi_mac_get_country(wifimac, &country);

    wifimac->wm_countryinfo.country_str[0] = country.iso[0];
    wifimac->wm_countryinfo.country_str[1] = country.iso[1];
    wifimac->wm_countryinfo.country_str[2] = 0x20; //environment: Any
    wifimac->wm_countryinfo.country_len += 3;

    wifi_mac_build_country_ie_2G_and_5G(wifimac);

    DPRINTF(AML_DEBUG_INFO,"INFO::Country ie is %c%c%c\n", wifimac->wm_countryinfo.country_str[0],
        wifimac->wm_countryinfo.country_str[1], wifimac->wm_countryinfo.country_str[2]);
}

int wifi_mac_get_new_vmac_id(struct wifi_mac *wifimac)
{
    int id = 0;

    id = find_first_zero_bit(&wifimac->wm_wnet_vif_mask, WIFI_MAX_VID);
    if (id >= WIFI_MAX_VID)
    {
        id= -1;
        return id;
    }
    set_bit(id, (unsigned long *)&wifimac->wm_wnet_vif_mask);

    return id;
}

void wifi_mac_delt_vmac_id(struct wifi_mac *wifimac,int vid)
{
    ASSERT(vid < WIFI_MAX_VID);
    if (test_and_clear_bit(vid, (unsigned long *)&wifimac->wm_wnet_vif_mask))
    {
        AML_OUTPUT("<WARNING> vid %d free success\n", vid);
    }
    else
    {
        AML_OUTPUT("<WARNING> vid %d has been free\n", vid);
    }
}

void wifi_mac_channel_switch_complete(struct wlan_net_vif *wnet_vif)
{
    unsigned char vid = wnet_vif->wnet_vif_id;
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    DPRINTF(AML_DEBUG_SCAN, "%s, vid:%d, ss->scan_StateFlags:%08x\n", __func__, vid, ss->scan_StateFlags);

    if (!(wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_NOA) ) {
        wifimac->drv_priv->drv_ops.drv_hal_tx_frm_pause(wifimac->drv_priv, 0);
    }

    if (wifimac->wm_vsdb_flags & CONCURRENT_SWITCH_TO_STA_CHANNEL) {
        wifimac->wm_vsdb_slot = CONCURRENT_SLOT_NONE;
        wifimac->wm_vsdb_flags &= ~CONCURRENT_SWITCH_TO_STA_CHANNEL;
        /*switch to sta channel,we need notify ap*/
        wifi_mac_scan_notify_leave_or_back(wifimac->drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC],0);
        AML_OUTPUT("channel switch complete and sta notify ap back done\n");
    }

    WIFI_SCAN_LOCK(ss);
    if (ss->scan_StateFlags & SCANSTATE_F_WAIT_CHANNEL_SWITCH) {
        ss->scan_StateFlags &= ~SCANSTATE_F_WAIT_CHANNEL_SWITCH;
        ss->scan_StateFlags |= SCANSTATE_F_CHANNEL_SWITCH_COMPLETE;
        WIFI_SCAN_UNLOCK(ss);
        wifi_mac_scan_timeout_ex(wifimac->wm_scan);
        WIFI_SCAN_LOCK(ss);
    }
    WIFI_SCAN_UNLOCK(ss);

    return;
}

static void wifi_mac_set_noa(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    unsigned long long tsf;
    unsigned int ltsf;
    unsigned int next_tbtt;

    if (wifimac->wm_nrunning == 1) {
        struct wlan_net_vif *connect_wnet = wifi_mac_running_wnet_vif(wifimac);
        struct p2p_noa noa = {0};

        if (connect_wnet == NULL) {
            return;
        }

        if (connect_wnet->vm_opmode == WIFINET_M_HOSTAP &&
            wnet_vif->vm_p2p->p2p_negotiation_state == NET80211_P2P_STATE_GO_COMPLETE) {
            tsf = wifimac->drv_priv->drv_ops.drv_hal_get_tsf(wifimac->drv_priv, wnet_vif->wnet_vif_id);
            ltsf = (unsigned int)tsf;
            next_tbtt = roundup(ltsf, wnet_vif->vm_bcn_intval*TU_DURATION);
            noa.start = next_tbtt + WIFINET_SCAN_MOTHER_CHANNEL_TIME*TU_DURATION;
            noa.count = 255;
            noa.duration = 25*TU_DURATION;
            noa.interval = wnet_vif->vm_bcn_intval * TU_DURATION;
            wnet_vif->vm_p2p->noa_index++;
            wnet_vif->vm_p2p->ap_mode_set_noa_enable = 1;
            vm_p2p_noa_start(wnet_vif->vm_p2p, &noa);
            vm_p2p_update_noa_ie(wnet_vif->vm_p2p);
        }
    }
}

#ifndef FW_RF_CALIBRATION
void wifi_mac_tbtt_handle(struct wlan_net_vif *wnet_vif)
{
    unsigned char vid = wnet_vif->wnet_vif_id;
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    DPRINTF(AML_DEBUG_SCAN, "%s, vid:%d, ss->scan_StateFlags:%08x\n", __func__, vid, ss->scan_StateFlags);

    WIFI_SCAN_LOCK(ss);
    if (ss->scan_StateFlags & SCANSTATE_F_WAIT_TBTT) {
        ss->scan_StateFlags &= ~SCANSTATE_F_WAIT_TBTT;

        //pr_debug("%s start timer\n", __func__);
        if (ss->scan_chan_wait <= 60) {
            os_timer_ex_start_period(&ss->ss_scan_timer,
                (WIFINET_SCAN_DEFAULT_INTERVAL - ss->scan_chan_wait - WIFINET_SCAN_CHANNEL_COST - WIFINET_SCAN_INTERVAL_LEFT));

        } else if (ss->scan_chan_wait <= 140) {
            os_timer_ex_start_period(&ss->ss_scan_timer,
                (2 * WIFINET_SCAN_DEFAULT_INTERVAL - ss->scan_chan_wait - WIFINET_SCAN_CHANNEL_COST));

        } else {
            //not suggest scan channel time lager than 140
            os_timer_ex_start_period(&ss->ss_scan_timer,
                (2 * WIFINET_SCAN_DEFAULT_INTERVAL - ss->scan_chan_wait - WIFINET_SCAN_CHANNEL_COST));
        }
    }
    WIFI_SCAN_UNLOCK(ss);

    return;
}
#else
void wifi_mac_tbtt_handle(struct wlan_net_vif *wnet_vif)
{
    unsigned char vid = wnet_vif->wnet_vif_id;
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;

    DPRINTF(AML_DEBUG_SCAN, "%s, vid:%d, ss->scan_StateFlags:%08x, id=%d\n", __func__, vid, ss->scan_StateFlags, wnet_vif->wnet_vif_id);
    if ((ss->scan_StateFlags & SCANSTATE_F_WAIT_TBTT)
        && wnet_vif->vm_p2p->p2p_enable
        && !wnet_vif->vm_p2p->ap_mode_set_noa_enable) {
        wifi_mac_set_noa(wnet_vif);
    }
    WIFI_SCAN_LOCK(ss);
    if (ss->scan_StateFlags & SCANSTATE_F_WAIT_TBTT) {
        ss->scan_StateFlags &= ~SCANSTATE_F_WAIT_TBTT;
        //pr_debug("%s start timer\n", __func__);
        os_timer_ex_start_period(&ss->ss_scan_timer, WIFINET_SCAN_MOTHER_CHANNEL_TIME);
    }
    WIFI_SCAN_UNLOCK(ss);

    return;
}
#endif

int wifi_mac_device_ip_config(struct wlan_net_vif *wnet_vif, void *event)
{
    struct dhcp_offload_event *fw_event = (struct dhcp_offload_event *)event;
    struct in_device *in_dev;
    struct in_ifaddr *ifa_v4;

    DPRINTF(AML_DEBUG_CONNECT, "%s, vid:%d\n", __func__, fw_event->basic_info.vid);
    in_dev = __in_dev_get_rtnl(wnet_vif->vm_ndev);
    if (in_dev) {
      ifa_v4 = in_dev->ifa_list;
      if (ifa_v4 != NULL) {
        ifa_v4->ifa_local = *(__be32 *)(&fw_event->local_ip);
        pr_debug("local ip: %08x\n", ifa_v4->ifa_local);
      }
    }

    return 1;
}

int wifi_mac_create_vmac(struct wifi_mac *wifimac, void *ifr, int cmdFromwhr)
{
    struct vm_wlan_net_vif_params vmparam;
    char name[IFNAMSIZ];
    struct wlan_net_vif *wnet_vif = NULL;
    unsigned char myaddr[WIFINET_ADDR_LEN] = { mac_addr0, mac_addr1, mac_addr2, mac_addr3, mac_addr4, mac_addr5 };
    int vm_opmode = 0, vid = 0;
    struct in_device *in_dev;
    struct in_ifaddr *ifa_v4;
    __be32 ipv4 = 0;

    if (cmdFromwhr ==1) {
        if (!capable(CAP_NET_ADMIN))
            return -EPERM;

        if (copy_from_user(&vmparam, ((struct ifreq *)ifr)->ifr_data, sizeof(vmparam)))
            return -EFAULT;

    } else {
        memcpy(&vmparam, ifr, sizeof(vmparam));
    }

    /* interface name 'wlan0', 'p2p0'.... for net device*/
    strncpy(name, vmparam.vm_param_name, sizeof(name) - 1);
    vid = wifi_mac_get_new_vmac_id(wifimac);
    if (vid < 0) {
       ERROR_DEBUG_OUT("alloc wnet_vif id %d <%s>  < fail > \n",vid, vmparam.vm_param_name);
        return -EIO;
    }

    pr_debug(" %s %d cp.vm_param_opmode=%d vm_param_name %s vid = %d \n",
        __func__,__LINE__,vmparam.vm_param_opmode,vmparam.vm_param_name,vid);

    switch (vmparam.vm_param_opmode) {
        case WIFINET_M_STA:
            vm_opmode = vmparam.vm_param_opmode;
            break;
        case WIFINET_M_P2P_DEV:
            vm_opmode = WIFINET_M_STA;
            break;
        case WIFINET_M_IBSS:
        case WIFINET_M_MONITOR:
            if (wifimac->wnet_vif_num != 0)
            {
                ERROR_DEBUG_OUT("Can't Create opmode=%d.\n",vmparam.vm_param_opmode);
                return   -EIO;
            }
            vm_opmode = vmparam.vm_param_opmode;
            break;
        case WIFINET_M_HOSTAP:
        case WIFINET_M_WDS:
            vm_opmode = WIFINET_M_HOSTAP;
            break;
        default:
            ERROR_DEBUG_OUT("<running> opmode=%d\n",vmparam.vm_param_opmode);
            return  -EIO;
    }

    wnet_vif = (struct wlan_net_vif *)wifi_mac_alloc_ndev(sizeof(struct wlan_net_vif ));
    if (wnet_vif == NULL)
    {
        ERROR_DEBUG_OUT("allocate wnet_vif fail \n");
        wifi_mac_delt_vmac_id(wifimac,vid);
        return  -EIO;
    }

#ifdef CONFIG_P2P
    if(vmparam.vm_param_opmode == WIFINET_M_P2P_DEV)
        wnet_vif->vm_p2p_support = 1;
    else
        wnet_vif->vm_p2p_support = 0;
#endif

    wnet_vif->wnet_vif_id = vid;
    wnet_vif->vm_beaconbuf = NULL;
    wnet_vif->wnet_vif_replaycounter = 0;
    wnet_vif->vm_debug = WIFINET_DEBUG_LEVEL;
    wnet_vif->vm_bcn_intval = WIFINET_BINTVAL_DEFAULT;
    wnet_vif->vm_def_mgmt_txkey = WIFINET_KEYIX_NONE;
    myaddr[2] += vid<<4;

    rtnl_lock();
    in_dev = __in_dev_get_rtnl(wnet_vif->vm_ndev);
    rtnl_unlock();

    if (in_dev) {
      ifa_v4 = in_dev->ifa_list;
      if (ifa_v4 != NULL)
        ipv4 = ifa_v4->ifa_local;
    }

    if ( wifimac->drv_priv->drv_ops.add_interface(wifimac->drv_priv, vid, wnet_vif,
                wifi_mac_opmode_2_halmode(vm_opmode), myaddr, ipv4))
    {
        ERROR_DEBUG_OUT("add interface < fail > \n");
        wifi_mac_free_vmac(wnet_vif);
        return  -EIO;
    }

    wifi_mac_setup(wifimac, wnet_vif, vm_opmode);
    WIFINET_ADDR_COPY(wnet_vif->vm_myaddr, myaddr);
    vm_wlan_net_vif_register(wnet_vif, name);

    /*set mac operation mode(11GN, 11GNAC) for sta or p2p dev. */
    wifi_mac_mode_vattach_ex(wnet_vif, vmparam.vm_param_opmode);

    wnet_vif->vif_ops.write_word = wifi_mac_write_word;
    wnet_vif->vif_ops.read_word = wifi_mac_read_word;
    wnet_vif->vif_ops.bt_write_word = wifi_mac_bt_write_word;
    wnet_vif->vif_ops.bt_read_word = wifi_mac_bt_read_word;
    wnet_vif->vif_ops.pt_rx_start = wifi_mac_pt_rx_start;
    wnet_vif->vif_ops.pt_rx_stop = wifi_mac_pt_rx_stop;

    AML_OUTPUT("wnet_vif->vm_mac_mode=%d, wnet_vif:%p\n", wnet_vif->vm_mac_mode, wnet_vif);
    return 0;
}


enum wifi_mac_macmode p2p_phy_mode_filter (enum wifi_mac_macmode in)
{
    enum wifi_mac_macmode out = in;
    int change = false;
    switch (in)
    {
        case WIFINET_MODE_11B:
            DPRINTF(AML_DEBUG_WARNING,"%s should not support 11b only\n", __func__);
            break;
        case WIFINET_MODE_11BG:
            out = WIFINET_MODE_11G;
            change = true;
            break;
        case WIFINET_MODE_11BGN:
            out = WIFINET_MODE_11GN;
            change = true;
            break;
        case WIFINET_MODE_11GNAC:
            out = WIFINET_MODE_11GNAC;
            change = true;
            break;
        default:
            // need to check twice
            break;
    }
    if (change == true)
    {
        AML_PRINT(AML_DBG_MODULES_P2P,"rm 11b for p2p, macmode=%d\n", out);
    }
    return out;
}

static int wifi_mac_get_ap_mode(unsigned char rates[])
{
    int i = 0;
    int j = 0;
    int flag = 0;
    const struct wifi_mac_rateset basic_rate[] =
    {
        { 4, { 0x82, 0x84, 0x8b, 0x96} },
        { 8, { 0x8c, 0x92, 0x98, 0xa4, 0xb0, 0xc8, 0xe0, 0xec} },
    };

    /*check if rates has 80211_B rate */
    for (i = 0; i < rates[1]; i++) {
        for (j = 0; j < basic_rate[0].dot11_rate_num; j++) {
            if (WIFINET_GET_RATE_VAL(rates[i+2]) == WIFINET_GET_RATE_VAL(basic_rate[0].dot11_rate[j])) {
                flag |= BIT(WIFINET_MODE_11B);
                break;
            }
        }
    }

    /*check if rates has 80211_G rate */
    for (i = 0; i < rates[1]; i++) {
        for (j = 0; j < basic_rate[1].dot11_rate_num; j++) {
            if (WIFINET_GET_RATE_VAL(rates[i+2]) == WIFINET_GET_RATE_VAL(basic_rate[1].dot11_rate[j])) {
                flag |= BIT(WIFINET_MODE_11G);
                break;
            }
        }
    }

    return flag;
}

enum wifi_mac_macmode wifi_mac_get_sta_mode(struct wifi_scan_info *se)
{
    enum wifi_mac_macmode macmode = WIFINET_MODE_AUTO;
    unsigned int flag = 0;
    unsigned char is_2g_channel;

    if (se == NULL)
    {
        ERROR_DEBUG_OUT("se == NULL\n");
        return WIFINET_MODE_AUTO;
    }

    is_2g_channel = WIFINET_IS_CHAN_2GHZ(se->SI_chan);

    /*check if se->SI_rates has 80211_B/G rate */
    flag = wifi_mac_get_ap_mode(se->SI_rates);

    /*check if se->SI_exrates has 80211_B/G rate */
    flag |= wifi_mac_get_ap_mode(se->SI_exrates);

    if ((flag & (BIT(WIFINET_MODE_11G)|BIT(WIFINET_MODE_11B)))
            == (BIT(WIFINET_MODE_11G)|BIT(WIFINET_MODE_11B)))
    {
        macmode = WIFINET_MODE_11BG;
    }
    else if ((flag & (BIT(WIFINET_MODE_11G)|BIT(WIFINET_MODE_11B))) == (BIT(WIFINET_MODE_11G)))
    {
        macmode = WIFINET_MODE_11G;
    }
    else if ((flag & (BIT(WIFINET_MODE_11G)|BIT(WIFINET_MODE_11B))) == (BIT(WIFINET_MODE_11B)))
    {
        macmode = WIFINET_MODE_11B;
    }

    if ((se->SI_htcap_ie[1] != 0) && !wifi_mac_is_ht_forbidden(se))
    {
        if(macmode == WIFINET_MODE_11BG)
        {
            macmode = WIFINET_MODE_11BGN;

        } else if(macmode == WIFINET_MODE_11G) {
            macmode = WIFINET_MODE_11GN;

        } else {
            macmode = WIFINET_MODE_11N;
        }

        if(!is_2g_channel && (se->ie_vht_cap[0] == WIFINET_ELEMID_VHTCAP)) {
            macmode = WIFINET_MODE_11GNAC;
        }

    } else {
        if(!is_2g_channel && (se->ie_vht_cap[0] == WIFINET_ELEMID_VHTCAP)) {
            macmode = WIFINET_MODE_11G;
        }
    }//wifi03-5G, add vht under TKIP, but use 11g rate.

    DPRINTF(AML_DEBUG_INFO,"WARNING:: %s %d macmode=%d, chan_flags=0x%x\n",
            __func__,__LINE__,macmode, se->SI_chan->chan_flags );

    if(macmode == WIFINET_MODE_AUTO)
    {
        if(WIFINET_IS_CHAN_5GHZ (se->SI_chan))
        {
            macmode = WIFINET_MODE_11GNAC;
         }
        else
        {
            macmode = WIFINET_MODE_11BGN;
        }

    }
    DPRINTF(AML_DEBUG_INFO,"WARNING:: %s %d macmode=%d, need to check twice\n",
            __func__, __LINE__, macmode);
    return macmode;
}

int wifi_mac_mode_vattach_ex (struct wlan_net_vif *wnet_vif,
        enum wifi_mac_opmode opmode)
{
    if( opmode ==WIFINET_M_P2P_DEV) {
        wnet_vif->vm_flags_ext2 |= (WIFINET_FEXT2_PUREG | WIFINET_FEXT2_PUREN);
        wnet_vif->vm_mac_mode = WIFINET_MODE_11GN;

    } else {
        wnet_vif->vm_flags_ext2 |= (WIFINET_FEXT2_PUREB | WIFINET_FEXT2_PUREG | WIFINET_FEXT2_PUREN);
        wnet_vif->vm_mac_mode = WIFINET_MODE_11GNAC;
    }

    AML_OUTPUT("wnet_vif->vm_mac_mode=%d\n", wnet_vif->vm_mac_mode);
    return 1;
}

void wifi_mac_write_word(unsigned int addr,unsigned int data)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    wifimac->drv_priv->drv_ops.drv_write_word(addr, data);
}

unsigned int wifi_mac_read_word(unsigned int addr)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    return wifimac->drv_priv->drv_ops.drv_read_word(addr);
}

void wifi_mac_bt_write_word(unsigned int addr,unsigned int data)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    wifimac->drv_priv->drv_ops.drv_bt_write_word(addr, data);
}

unsigned int wifi_mac_bt_read_word(unsigned int addr)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    return wifimac->drv_priv->drv_ops.drv_bt_read_word(addr);
}


void wifi_mac_pt_rx_start(unsigned int qos)
{
     struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
     wifimac->drv_priv->drv_ops.drv_pt_rx_start(qos);
}

void wifi_mac_pt_rx_stop(void)
{
     struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
     wifimac->drv_priv->drv_ops.drv_pt_rx_stop();
}

int wifi_mac_connect_repair(struct wifi_mac *wifimac)
{
    struct wlan_net_vif *wnet_vif = NULL;
    int ret = 0;

#ifdef SDIO_BUILD_IN
    wifi_mac_set_wifi_insmod_status();
#endif

    list_for_each_entry(wnet_vif,&wifimac->wm_wnet_vifs, vm_next) {
        if (wnet_vif->vm_opmode == WIFINET_M_STA || wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            break;
        }
    }

    switch (wnet_vif->vm_opmode)
    {
        case WIFINET_M_STA:
            AML_OUTPUT("sta mode\n");
            WIFINET_FW_STAT_LOCK(wifimac);
            if (wifimac->recovery_stat != WIFINET_RECOVERY_END) {
                WIFINET_FW_STAT_UNLOCK(wifimac);
                return 0;
            }
            wifimac->recovery_stat = WIFINET_RECOVERY_START;
            WIFINET_FW_STAT_UNLOCK(wifimac);

            if (wifimac->wm_flags & WIFINET_F_SCAN) {
                wifimac->wm_scan->scan_StateFlags |= SCANSTATE_F_CANCEL;
            }
            if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
                wifimac->recovery_stat |= WIFINET_RECOVERY_UNDER_CONNECT;
            }
            //hi_get_irq_status();
            //pr_debug("get irq status done\n");
            wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
            wifi_mac_fw_recovery(wnet_vif);
            hi_get_irq_status();
            hi_clear_irq_status(SDIO_FW2HOST_EN);
            break;
        case WIFINET_M_HOSTAP:
            break;

        default:
            AML_OUTPUT("wnet_vif->vm_opmode=%d not support\n", wnet_vif->vm_opmode);
            ret = -EINVAL;
    }

    return 0;
}

static void wifi_mac_connect_repair_task(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *) param1;
    static unsigned int free_page = 0;
    static unsigned int send = 0;
    static unsigned int done = 0;
    static unsigned int free = 0;
    static unsigned int count = 0;
    static unsigned int tx_ok_num = 0;
    static unsigned int tx_fail_num = 0;
    struct hal_private* hal_priv = hal_get_priv();
    struct hw_interface* hif = hif_get_hw_interface();

#if 0 //this is for test
    wifimac->recovery_stat = WIFINET_RECOVERY_START;
    wifi_mac_connect_repair(wifimac);
    return 0;
#endif

    /*free_page/send/done/free all same as last value, go into else branch*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
    if (((free_page == hal_priv->txPageFreeNum) && (done == hif->HiStatus.Tx_Done_num) && (free == hif->HiStatus.Tx_Free_num) && (done > free))
#else
    if (((free_page == hal_priv->txPageFreeNum) && (done == atomic_read(&hif->HiStatus.Tx_Done_num))
        && (free == atomic_read(&hif->HiStatus.Tx_Free_num)) && (done > free))
#endif
        || ((hif->HiStatus.tx_ok_num == tx_ok_num) && (hif->HiStatus.tx_fail_num - tx_fail_num > 200))
        || (wifi_mac_get_host_wake_status() > 0)
        || (hal_priv->txPageFreeNum > 224)) {

        if ((hal_priv->txPageFreeNum > 224) || (wifi_mac_get_host_wake_status() > 0)) {
            count = 1;
        }

        count++;
        if (count == 2) {

            pr_emerg("last send %d, current send %d, last done %d, current done %d, last free %d, current free %d, count %d\n",
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
                send, hif->HiStatus.Tx_Send_num, done, hif->HiStatus.Tx_Done_num, free, hif->HiStatus.Tx_Free_num, count);
#else
                send, atomic_read(&hif->HiStatus.Tx_Send_num),
                done, atomic_read(&hif->HiStatus.Tx_Done_num),
                free, atomic_read(&hif->HiStatus.Tx_Free_num), count);
#endif

            pr_emerg("last free page %d, current free page %d, tx_ok %d, last tx_ok %d, tx_fail:%d, last_tx_fail:%d host_wake %hhu\n",
                    free_page, hal_priv->txPageFreeNum, tx_ok_num, hif->HiStatus.tx_ok_num, tx_fail_num, hif->HiStatus.tx_fail_num);

            count = 0;
            pr_emerg("will repair connection!\n");
            wifi_mac_connect_repair(wifimac);
        }

    } else {
        free_page = hal_priv->txPageFreeNum;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
        send = hif->HiStatus.Tx_Send_num;
        done = hif->HiStatus.Tx_Done_num;
        free = hif->HiStatus.Tx_Free_num;
#else
        send = atomic_read(&hif->HiStatus.Tx_Send_num);
        done = atomic_read(&hif->HiStatus.Tx_Done_num);
        free = atomic_read(&hif->HiStatus.Tx_Free_num);
#endif
        tx_ok_num = hif->HiStatus.tx_ok_num;
        tx_fail_num = hif->HiStatus.tx_fail_num;
        count = 0;
        return;
    }

    return;
}


int wifi_mac_trigger_recovery(void * arg)
{
    wifi_mac_add_work_task(arg, wifi_mac_connect_repair_task, NULL, (SYS_TYPE)arg, 0, 0, 0, 0);
    return 0;
}

void wifi_mac_fw_recovery(struct wlan_net_vif *wnet_vif)
{
    /*todo fw recovery*/
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct hal_private* hal_priv = hal_get_priv();

    wifimac->drv_priv->drv_ops.fw_repair(wifimac->drv_priv);
    hal_priv->powersave_init_flag = 0;
    wifimac->drv_priv->drv_ops.drv_interface_enable(ENABLE, wnet_vif->wnet_vif_id);

    wifi_mac_clear_host_wake_status();

    if (wifimac->recovery_stat & WIFINET_RECOVERY_UNDER_CONNECT) {
        wifimac->recovery_stat = WIFINET_RECOVERY_VIF_UP;

        os_timer_ex_start(&pwdev_priv->connect_timeout);
        wifi_mac_chk_scan(wnet_vif, WIFINET_SCANCFG_ACTIVE| WIFINET_SCANCFG_FLUSH, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);

    } else {
        wifimac->recovery_stat = WIFINET_RECOVERY_END;
    }

}

static unsigned char is_in_recovery(struct wifi_mac *wifimac)
{
    if (wifimac->recovery_stat & WIFINET_RECOVERY_START) {
        ;
    }

    return 0;
}

