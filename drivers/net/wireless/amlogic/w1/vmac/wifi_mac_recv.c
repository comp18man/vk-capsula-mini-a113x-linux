/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer receive frame function
 *
 *
 ****************************************************************************************
 */

#include "wifi_mac_com.h"
#include "wifi_mac_scan.h"
#include "wifi_hal_com.h"
#include "wifi_mac_action.h"
#include "wifi_mac_sae.h"
#include "wifi_cmd_func.h"
#include "wifi_mac_tx_reg.h"

static struct sk_buff *wifi_mac_defrag(struct wifi_station *, struct sk_buff *, int);
static void wifi_mac_deliver_data(struct wifi_station *, struct sk_buff *);
static struct sk_buff *wifi_mac_decap(struct wlan_net_vif *, struct sk_buff *, int, int);
static void wifi_mac_send_error(struct wifi_station *, const unsigned char *mac, int subtype, long long arg);
static void wifi_mac_amsdu_subframe_decap(struct sk_buff *skb);
static int wifi_mac_amsdu_input(struct wifi_station *sta, struct sk_buff *skb);

void wifi_mac_forward_data(struct wlan_net_vif *wnet_vif)
{
    struct sk_buff *forward_skb = NULL;
    struct sk_buff *skb = NULL;
    struct sk_buff_head *fwdtxqueue = &wnet_vif->vm_forward_buffer_queue;
    unsigned int qlen_real = WIFINET_SAVEQ_QLEN(fwdtxqueue);
    while (qlen_real)
    {
        WIFINET_SAVEQ_LOCK(fwdtxqueue);
        WIFINET_SAVEQ_DEQUEUE(fwdtxqueue, skb, qlen_real);
        WIFINET_SAVEQ_UNLOCK(fwdtxqueue);

        if (!skb)
        {
            break;
        }
        forward_skb = wifi_mac_decap(wnet_vif, skb, wnet_vif->vm_forward.hdrspace, 1);
        if (forward_skb == NULL)
        {
            ERROR_DEBUG_OUT("decap skb err\n");
            return;
        }
        wifi_mac_hardstart(forward_skb, wnet_vif->vm_ndev);
    }
}
void wifi_mac_forward_init(struct drv_private *drv_priv, struct sk_buff *skb, struct wlan_net_vif *wnet_vif, int hdrspace)
{
    wnet_vif->vm_forward.skb = NULL;
    if (wnet_vif->vm_sta_assoc > 1) {
        wnet_vif->vm_forward.skb = os_skb_copy(skb, GFP_ATOMIC, wnet_vif->vm_forward.skb);
        if (wnet_vif->vm_forward.skb == NULL) {
            ERROR_DEBUG_OUT("skb is NULL\n");
            return;
        }
        wnet_vif->vm_forward.fwd_flag = 1;
        wnet_vif->vm_forward.hdrspace = hdrspace;
        drv_priv->net_ops->wifi_mac_forward_txq_enqueue(&wnet_vif->vm_forward_buffer_queue, wnet_vif->vm_forward.skb);
   }
}
int wifi_mac_rx_get_wnet_vifid(struct wifi_mac *wifimac,struct wifi_frame *wh)
{
    struct wlan_net_vif *wnet_vif=NULL;
    struct wlan_net_vif *tmpwnet_vif;
    unsigned char *bssid = NULL;
    unsigned char from2ds =(wh->i_fc[1] & WIFINET_FC1_DIR_MASK);

    if (from2ds == WIFINET_FC1_DIR_TODS)
    {
        bssid =wh->i_addr1;
    }
    else if (from2ds == WIFINET_FC1_DIR_FROMDS)
    {
        bssid =wh->i_addr2;
    }
    else if (from2ds == WIFINET_FC1_DIR_NODS)
    {
        bssid =wh->i_addr3;
    }
    else if (from2ds == WIFINET_FC1_DIR_DSTODS)
    {
        bssid =wh->i_addr1;
    }
    else
    {
        return -1;
    }

    list_for_each_entry(tmpwnet_vif,&wifimac->wm_wnet_vifs,vm_next)
    {
        if (tmpwnet_vif->vm_mainsta == NULL)
        {
            continue;
        }
        if (WIFINET_ADDR_EQ(tmpwnet_vif->vm_mainsta->sta_bssid, bssid))
        {
            wnet_vif = tmpwnet_vif;
            break;
        }
    }
    if (wnet_vif == NULL)
    {
        return -1;
    }

    return wnet_vif->wnet_vif_id;
}

void drv_forward_tasklet(unsigned long arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)arg;
    wifi_mac_forward_data(wnet_vif);
}


int wifi_mac_input(void *station, struct sk_buff *skb, struct wifi_mac_rx_status *rs)
{
    struct wifi_station *sta = (struct wifi_station *)station;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac;
    struct drv_private *drv_priv = NULL;
    struct wifi_station *sta_wds = NULL;
    struct wifi_station *temp_nsta = NULL;
    struct net_device *dev;
    struct wifi_frame *wh;
    struct wifi_mac_key *key;
    struct ether_header *eh;
    int hdrspace;
    unsigned char dir, type, subtype;
    unsigned short rxseq = 0;
    int is_amsdu = 0;
    unsigned char *bssid = NULL;
    type = -1;

    KASSERT(sta != NULL, ("null nsta"));
    sta->sta_inact = sta->sta_inact_reload;
    sta->sta_inact_time = jiffies;
    //dump_memory_internal(os_skb_data(skb), os_skb_get_pktlen(skb));
    KASSERT(os_skb_get_pktlen(skb) >= sizeof(struct wifi_mac_frame_min),
            ("frame length too short: %u", os_skb_get_pktlen(skb)));

    if (wnet_vif == NULL) {
        ERROR_DEBUG_OUT("sta:%p\n", sta);
        goto out;
    }

    dev = wnet_vif->vm_ndev;
    wifimac = wnet_vif->vm_wmac;
    drv_priv = wifimac->drv_priv;
    if (wnet_vif->vm_opmode == WIFINET_M_MONITOR)
        goto out;

    if (os_skb_get_pktlen(skb) < sizeof(struct wifi_mac_frame_min))
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY, sta->sta_macaddr, "too short (1): len %u", os_skb_get_pktlen(skb));
        wnet_vif->vif_sts.sts_rx_too_short++;
        goto out;
    }

    wh = (struct wifi_frame *)os_skb_data(skb);

    if ((wh->i_fc[0] & WIFINET_FC0_VERSION_MASK) != WIFINET_FC0_VERSION_0)
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY, sta->sta_macaddr, "wrong version %x", wh->i_fc[0]);
        wnet_vif->vif_sts.sts_rx_bad_ver++;
        goto err;
    }

    dir = wh->i_fc[1] & WIFINET_FC1_DIR_MASK;
    type = wh->i_fc[0] & WIFINET_FC0_TYPE_MASK;
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    if (dir == WIFINET_FC1_DIR_DSTODS)
        sta->sta_flags |= WIFINET_NODE_WDS;

    if (((wifimac->wm_flags & WIFINET_F_SCAN) == 0)
#ifdef CONFIG_P2P
        &&(wnet_vif->vm_p2p->p2p_enable ==0)
#endif//CONFIG_P2P
        && (wnet_vif->remain_on_channel == 0)
       )
    {
        switch (wnet_vif->vm_opmode)
        {
            case WIFINET_M_STA:
                bssid = wh->i_addr2;
                if (!WIFINET_ADDR_EQ(bssid, sta->sta_bssid))
                {
                    WIFINET_DPRINTF( AML_DEBUG_RECV, "%s,%s,%x%x\n", "not to bss",ether_sprintf(bssid),wh->i_fc[0],wh->i_fc[1]);
                    wnet_vif->vif_sts.sts_rx_wrong_bss++;
                    goto out;
                }

                if ((wnet_vif->vm_state == WIFINET_S_CONNECTED) && (!WIFINET_IS_PROBERSP(wh))) {
                    if (wnet_vif->vm_bmiss_count) {
                        pr_debug("received pkt from ap, vid:%d\n", wnet_vif->wnet_vif_id);
                    }
                    wnet_vif->vm_bmiss_count = 0;
                }

                if ((wnet_vif->vm_state != WIFINET_S_CONNECTED) && WIFINET_IS_CLASS3(wh)) {
                    //WIFINET_DPRINTF( AML_DEBUG_WARNING, "received class 3 pkt before connected, %x %x", wh->i_fc[0],wh->i_fc[1]);
                    goto out;
                }
                break;

            case WIFINET_M_IBSS:
                if (type == WIFINET_FC0_TYPE_DATA && sta == wnet_vif->vm_mainsta)
                {
                    sta = wifi_mac_fake_adhos_sta(wnet_vif, wh->i_addr2);
                    if (sta == NULL)
                    {
                        goto err;
                    }
                }
                break;

            case WIFINET_M_HOSTAP:
                if (dir != WIFINET_FC1_DIR_NODS)
                    bssid = wh->i_addr1;
                else if (type == WIFINET_FC0_TYPE_CTL)
                    bssid = wh->i_addr1;
                else
                {
                    if (os_skb_get_pktlen(skb) < sizeof(struct wifi_frame))
                    {
                        WIFINET_DPRINTF(AML_DEBUG_ANY, "too short (2): len %u", os_skb_get_pktlen(skb));
                        wnet_vif->vif_sts.sts_rx_too_short++;
                        goto out;
                    }
                    bssid = wh->i_addr3;
                }

                if (!WIFINET_ADDR_EQ(bssid, wnet_vif->vm_mainsta->sta_bssid) &&
                    !WIFINET_ADDR_EQ(bssid, dev->broadcast) && (subtype != WIFINET_FC0_SUBTYPE_BEACON))
                {
                    WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, bssid, "%s", "not to bss");
                    wnet_vif->vif_sts.sts_rx_wrong_bss++;
                    goto out;
                }
                break;

            default:
                goto out;
        }

        sta->sta_rstamp = jiffies;
        if (HAS_SEQ(type))
        {
            unsigned char tid;
            if (WIFINET_QOS_HAS_SEQ(wh))
            {
                if (dir == WIFINET_FC1_DIR_DSTODS)
                {
                    tid = ((struct WIFINET_S_FRAME_QOS_ADDR4 *)wh)->i_qos[0] & WIFINET_QOS_TID;
                }
                else
                {
                    tid = ((struct wifi_qos_frame *)wh)->i_qos[0] & WIFINET_QOS_TID;
                }
                if (TID_TO_WME_AC(tid) >= WME_AC_VI)
                    wifimac->wm_wme[wnet_vif->wnet_vif_id].wme_hipri_traffic++;
                tid++;
            }
            else
            {
                /*for management or control frames, this tid just be used to
                stored sequence number to avoid uploading duplicate frames */
                tid = 0;
            }
            /*filtering the same retry management/data frames */
            rxseq = le16toh(*(unsigned short *)wh->i_seq);
            if ((wh->i_fc[1] & WIFINET_FC1_RETRY) && (rxseq == sta->sta_rxseqs[tid]))
            {
                WIFINET_DPRINTF( AML_DEBUG_RECV,"duplicate seqnum %x id %u drop", rxseq >> WIFINET_SEQ_SEQ_SHIFT, tid);
                wnet_vif->vif_sts.sts_rx_dup++;
                WIFINET_NODE_STAT(sta, rx_dup_cnt);
                goto out;
            }
            sta->sta_rxseqs[tid] = rxseq;
        }
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_state == WIFINET_S_CONNECTED)
        && (!WIFINET_IS_PROBERSP(wh)) && (!WIFINET_IS_BEACON(wh)) && (!WIFINET_IS_ACTION(wh))) {
        bssid = wh->i_addr2;
        if (!WIFINET_ADDR_EQ(bssid, sta->sta_bssid)) {
            WIFINET_DPRINTF(AML_DEBUG_RECV, "%s,%s,%x%x\n", "not to bss",ether_sprintf(bssid),wh->i_fc[0],wh->i_fc[1]);
            wnet_vif->vif_sts.sts_rx_wrong_bss++;
            goto out;
        }

        if (wnet_vif->vm_bmiss_count) {
            pr_warn("%s, vid:%d vm_bmiss_count set to 0\n", __func__, wnet_vif->wnet_vif_id);
        }
        wnet_vif->vm_bmiss_count = 0;
    }

    switch (type)
    {
        case WIFINET_FC0_TYPE_DATA:
            if ((wnet_vif->vm_opmode == WIFINET_M_STA) || (wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT)) {
                os_timer_ex_start(&wnet_vif->vm_pwrsave.ips_timer_presleep);
            }

            hdrspace = wifi_mac_hdrspace(wifimac, wh);
            if (os_skb_get_pktlen(skb) < hdrspace)
            {
                WIFINET_DPRINTF( AML_DEBUG_ANY, "data too short: len %u, expecting %u", os_skb_get_pktlen(skb), hdrspace);
                wnet_vif->vif_sts.sts_rx_too_short++;
                goto out;
            }

            switch (wnet_vif->vm_opmode)
            {
                case WIFINET_M_STA:
                    if ((dir != WIFINET_FC1_DIR_FROMDS) &&
                        (!((wnet_vif->vm_flags_ext & WIFINET_FEXT_WDS) && (dir == WIFINET_FC1_DIR_DSTODS))))
                    {
                        WIFINET_DPRINTF( AML_DEBUG_ANY, "data invalid dir 0x%x", dir);
                        wnet_vif->vif_sts.sts_rx_wrong_dir++;
                        goto out;
                    }

                    if ((dev->flags & IFF_MULTICAST) && WIFINET_IS_MULTICAST(wh->i_addr1))
                    {
                        if (WIFINET_ADDR_EQ(wh->i_addr3, wnet_vif->vm_myaddr))
                        {
                            WIFINET_DPRINTF( AML_DEBUG_RECV, "%s","multicast echo");
                            wnet_vif->vif_sts.sts_rx_mcast_echo++;
                            goto out;
                        }

                        if (wnet_vif->vm_flags_ext & WIFINET_FEXT_WDS)
                        {
                            struct wifi_station *sta_wds=NULL;
                            sta_wds = wifi_mac_find_wds_sta(&wnet_vif->vm_sta_tbl,wh->i_addr3);
                            if (sta_wds)
                            {
                                WIFINET_DPRINTF( AML_DEBUG_RECV,"%s","multicast echo originated from nsta behind me");
                                wnet_vif->vif_sts.sts_rx_mcast_echo++;
                                goto out;
                            }
                        }
                    }

                    if (dir == WIFINET_FC1_DIR_DSTODS)
                    {
                        struct WIFINET_S_FRAME_ADDR4 *wh4;
                        wh4 = (struct WIFINET_S_FRAME_ADDR4 *)os_skb_data(skb);
                        sta_wds = wifi_mac_find_wds_sta(&wnet_vif->vm_sta_tbl, wh4->i_addr4);

                        if (sta_wds == NULL)
                        {
                            if (!WIFINET_ADDR_EQ(wh4->i_addr2, wnet_vif->vm_mainsta->sta_bssid))
                            {
                                temp_nsta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl,wh4->i_addr4,wnet_vif->wnet_vif_id);
                                if (temp_nsta == NULL)
                                {
                                    wifi_mac_add_wds_addr(&wnet_vif->vm_sta_tbl, sta, wh4->i_addr4);
                                }
                                else if (!WIFINET_ADDR_EQ(temp_nsta->sta_macaddr, wnet_vif->vm_myaddr))
                                {
                                    pr_debug("<running> %s %d \n",__func__,__LINE__);
                                    wifi_mac_sta_disconnect_from_ap(temp_nsta);
                                    wifi_mac_add_wds_addr(&wnet_vif->vm_sta_tbl, sta, wh4->i_addr4);
                                }
                            }
                        }
                    }
                    break;

                case WIFINET_M_IBSS:
                    if (dir != WIFINET_FC1_DIR_NODS)
                    {
                        WIFINET_DPRINTF( AML_DEBUG_ANY,"data invalid dir 0x%x", dir);
                        wnet_vif->vif_sts.sts_rx_wrong_dir++;
                        goto out;
                    }
                    break;

                case WIFINET_M_HOSTAP:
                    if ((dir != WIFINET_FC1_DIR_TODS) && (dir != WIFINET_FC1_DIR_DSTODS))
                    {
                        WIFINET_DPRINTF( AML_DEBUG_ANY,"data invalid dir 0x%x", dir);
                        wnet_vif->vif_sts.sts_rx_wrong_dir++;
                        pr_debug("<running> %s %d \n",__func__,__LINE__);
                        goto out;
                    }

                    if (sta == wnet_vif->vm_mainsta)
                    {
                        WIFINET_DPRINTF( AML_DEBUG_RECV,"data %s", "unknown src");
                        if  (wnet_vif->vm_state == WIFINET_S_CONNECTED)
                        {
                            // pr_debug("<running> %s %d \n",__func__,__LINE__);
                            wifi_mac_send_error(sta, wh->i_addr2, WIFINET_FC0_SUBTYPE_DEAUTH, WIFINET_REASON_NOT_AUTHED);
                        }
                        wnet_vif->vif_sts.sts_rx_not_assoc++;
                        goto err;
                    }
                    if (sta->sta_associd == 0)
                    {
                        int arg = WIFINET_REASON_NOT_ASSOCED;
                        WIFINET_DPRINTF( AML_DEBUG_RECV,"data %s", "unassoc src");
                        wifi_mac_send_mgmt(sta,
                                          WIFINET_FC0_SUBTYPE_DISASSOC, (void *)&arg);
                        wnet_vif->vif_sts.sts_rx_not_assoc++;
                        goto err;
                    }

                    if  (dir == WIFINET_FC1_DIR_TODS && !WIFINET_ADDR_EQ(wh->i_addr3, wnet_vif->vm_myaddr)) {
                        wifi_mac_forward_init(drv_priv, skb, wnet_vif, hdrspace);
                        if (wnet_vif->vm_forward.fwd_flag) {
                            tasklet_schedule(&drv_priv->forward_tasklet);
                            wnet_vif->vm_forward.fwd_flag = 0;
                            if(!WIFINET_ADDR_EQ(wh->i_addr3, wnet_vif->vm_ndev->broadcast)) {
                                goto out;
                            }
                        }
                    }

                    if (dir == WIFINET_FC1_DIR_DSTODS)
                    {
                        struct wifi_station_tbl *nt;
                        struct WIFINET_S_FRAME_ADDR4 *wh4;
                        if (!(wnet_vif->vm_flags_ext & WIFINET_FEXT_WDS))
                        {
                            WIFINET_DPRINTF( AML_DEBUG_RECV,"data %s", "4 addr not allowed");
                            goto err;
                        }
                        wh4 = (struct WIFINET_S_FRAME_ADDR4 *)os_skb_data(skb);
                        nt = &wnet_vif->vm_sta_tbl;
                        sta_wds = wifi_mac_find_wds_sta(nt, wh4->i_addr4);
                        if ((sta_wds != NULL) && (sta_wds != sta))
                        {
                            wifi_mac_rm_sta(nt, wh4->i_addr4);
                            wifi_mac_add_wds_addr(nt, sta, wh4->i_addr4);
                        }
                        if (sta_wds == NULL)
                        {
                            temp_nsta = wifi_mac_get_sta(nt,wh4->i_addr4,wnet_vif->wnet_vif_id);
                            if (temp_nsta)
                            {
                                pr_debug("<running> %s %d \n",__func__,__LINE__);
                                wifi_mac_sta_disconnect_from_ap(temp_nsta);
                            }
                        }
                    }

                    if ((wh->i_fc[1] & WIFINET_FC1_PWR_MGT) ^
                        (sta->sta_flags & WIFINET_NODE_PWR_MGT))
                    {
                        wifi_mac_pwrsave_state_change(sta, wh->i_fc[1] & WIFINET_FC1_PWR_MGT);
                    }
                    break;

                default:
                    goto out;
            }

            if (wnet_vif->current_keytype != WIFINET_CIPHER_TKIP)
            {
                wh = (struct wifi_frame *)os_skb_data(skb);
                wh->i_fc[1] &= ~WIFINET_FC1_WEP;
            }

            if (wh->i_fc[1] & WIFINET_FC1_WEP)
            {
                if ((wnet_vif->vm_flags & WIFINET_F_PRIVACY) == 0)
                {
                    WIFINET_DPRINTF( AML_DEBUG_ANY,"WEP %s", "PRIVACY off");
                    wnet_vif->vif_sts.sts_no_priv_ac++;
                    WIFINET_NODE_STAT(sta, rx_noprivacy);
                    goto out;
                }
                key = wifi_mac_security_decap(sta, skb, hdrspace);
                if (key == NULL)
                {
                    WIFINET_DPRINTF( AML_DEBUG_ANY,"WEP 0x%p", key);
                    WIFINET_NODE_STAT(sta, rx_wepfail);
                    goto out;
                }
                wh = (struct wifi_frame *)os_skb_data(skb);
                wh->i_fc[1] &= ~WIFINET_FC1_WEP;
            }
            else
            {
                key = NULL;
            }
            M_FLAG_SET(skb, M_TKIPMICHW);  //means default hw calc tkip mic
            if (!WIFINET_IS_MULTICAST(wh->i_addr1))
            {
                skb = wifi_mac_defrag(sta, skb, hdrspace);
                if (skb == NULL)
                {
                    goto out;
                }
            }

            if (subtype == WIFINET_FC0_SUBTYPE_QOS)
            {
                is_amsdu = (dir != WIFINET_FC1_DIR_DSTODS)
                    ? (((struct wifi_qos_frame *)wh)->i_qos[0] & WIFINET_QOS_AMSDU)
                    : (((struct WIFINET_S_FRAME_QOS_ADDR4 *)wh)->i_qos[0] & WIFINET_QOS_AMSDU);
            }

            wh = NULL;
            if (key != NULL &&
                !wifi_mac_security_demic(wnet_vif, key, skb, hdrspace, !M_FLAG_GET(skb,M_TKIPMICHW)))
            {
                WIFINET_DPRINTF_MACADDR(AML_DEBUG_RECV, sta->sta_macaddr, "data %s", "demic error");
                WIFINET_NODE_STAT(sta, rx_demicfail);
                goto out;
            }

            /*TEMPORARY solution. shijie.chen add for prevent (qos) null datas are delivered to upper protocol. */
            if ((subtype == WIFINET_FC0_SUBTYPE_NODATA) || (subtype == WIFINET_FC0_SUBTYPE_QOS_NULL))
                goto out;

            skb = wifi_mac_decap(wnet_vif, skb, hdrspace, 0);
            if (skb == NULL)
            {
                WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV|AML_DEBUG_KEY,
                                         sta->sta_macaddr, "data %s", "decap error");
                wnet_vif->vif_sts.sts_rx_decap++;
                WIFINET_NODE_STAT(sta, rx_decap);

                goto err;
            }
            eh = (struct ether_header *) os_skb_data(skb);
            if (! (sta->sta_flags & WIFINET_NODE_AUTH) )
            {
                if (eh->ether_type != __constant_htons(ETHERTYPE_PAE))
                {
                    WIFINET_DPRINTF( AML_DEBUG_RECV|AML_DEBUG_KEY, "data unauthorized port: ether type 0x%x len %u",
                                     eh->ether_type, os_skb_get_pktlen(skb));
                    wnet_vif->vif_sts.sts_rx_unauth++;
                    WIFINET_NODE_STAT(sta, rx_unauth);

                    goto err;
                }
            }

            wnet_vif->vm_devstats.rx_packets++;
            wnet_vif->vm_devstats.rx_bytes += os_skb_get_pktlen(skb);
            WIFINET_NODE_STAT(sta, rx_data);
            WIFINET_NODE_STAT_ADD(sta, rx_bytes, os_skb_get_pktlen(skb));

            if (is_amsdu)
            {
                return wifi_mac_amsdu_input(sta, skb);
            }

           AML_PRINT(AML_DBG_MODULES_TX, "to deliver seq:%d\n", rxseq);
            wifi_mac_deliver_data(sta, skb);
            return WIFINET_FC0_TYPE_DATA;

        case WIFINET_FC0_TYPE_MGT:
            WIFINET_NODE_STAT(sta, rx_mgmt);
            if ((dir != WIFINET_FC1_DIR_NODS)
#ifdef CONFIG_P2P
                &&(wnet_vif->vm_p2p->p2p_enable==0)
#endif//CONFIG_P2P
               )
            {
                WIFINET_DPRINTF(AML_DEBUG_RECV,"receive mgmt frame dir = %d drop !\n ",dir);
                wnet_vif->vif_sts.sts_rx_wrong_dir++;
                goto err;
            }

            if (os_skb_get_pktlen(skb) < sizeof(struct wifi_frame)) {
                WIFINET_DPRINTF( AML_DEBUG_ANY, "mgt too short: len %u", os_skb_get_pktlen(skb));
                wnet_vif->vif_sts.sts_rx_too_short++;
                goto out;
            }

            if ((wnet_vif->vm_state == WIFINET_S_CONNECTED) && (sta->sta_flags_ext & WIFINET_NODE_MFP)) {
                if (!(wh->i_fc[1] & WIFINET_FC1_WEP)) {
                    if (!wifi_mac_unprotected_mgmt_pkt_handle(sta, skb, subtype)) {
                        goto out;
                    }

                } else {
                    if (wifi_mac_mgmt_validate(sta, skb, subtype)) {
                        goto out;
                    }
                }
            }

            wifi_mac_recv_mgmt(sta, skb, subtype, rs->rs_rssi, rs->rs_channel);
            goto out;

        case WIFINET_FC0_TYPE_CTL:
            WIFINET_NODE_STAT(sta, rx_ctrl);
            wnet_vif->vif_sts.sts_rx_ctrl++;
            if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
            {
                switch (subtype)
                {
                    case WIFINET_FC0_SUBTYPE_PS_POLL:
                        wifi_mac_pwrsave_recv_pspoll(sta, skb);
                        break;
                }
            }
            goto out;

        default:
            WIFINET_DPRINTF( AML_DEBUG_ANY, "bad frame type 0x%x", type);
            break;
    }
err:
    wnet_vif->vm_devstats.rx_errors++;
out:
    if (skb != NULL)
    {
        os_skb_free(skb);
    }
    return type;
}

int wifi_mac_input_all(struct wifi_mac *wifimac, struct sk_buff *skb, struct wifi_mac_rx_status *rs)
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct wlan_net_vif *wnet_vif_next = NULL;
    int type = -1;
    struct wifi_frame *wh = NULL;
    unsigned char *bssid = NULL;
    unsigned char multicast = 0;
    struct sk_buff *skb1 = NULL;

    if (!skb) {
        return type;
    }

    wh = (struct wifi_frame *)os_skb_data(skb);
    multicast = WIFINET_IS_MULTICAST(wh->i_addr1);
    bssid = &wh->i_addr2[0];

    list_for_each_entry_safe(wnet_vif, wnet_vif_next,  &wifimac->wm_wnet_vifs, vm_next) {
        if ((wnet_vif->vm_state == WIFINET_S_INIT) || (wnet_vif->vm_mainsta == NULL)) {
            continue;
        }

        if (multicast) {
            if (((wnet_vif->vm_opmode == WIFINET_M_STA) || (wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT))
                && (wnet_vif->vm_state == WIFINET_S_CONNECTED) && !(wifimac->wm_flags & WIFINET_F_SCAN)
                && memcmp(bssid, wnet_vif->vm_mainsta->sta_bssid, WIFINET_ADDR_LEN)) {
                continue;

            } else {
                /* When multiple interfaces use skb_buff at the same time */
                if ((wifimac->wm_nopened > 1) && !list_is_last(&wnet_vif->vm_next, &wifimac->wm_wnet_vifs)) {
                    skb1 = os_skb_copy(skb, GFP_ATOMIC,skb1);
                    if (skb1 == NULL) {
                        continue;
                    }

                    type = wifi_mac_input(wnet_vif->vm_mainsta, skb1, rs);
                    skb1 = NULL;
                    continue;
                }
            }

        } else {
            if (wnet_vif->wnet_vif_id != rs->rs_wnet_vif_id) {
                continue;
            }
        }

        type = wifi_mac_input(wnet_vif->vm_mainsta, skb, rs);
        skb = NULL;
        break;
    }

    if (skb) {
        os_skb_free(skb);
    }

    return type;
}

static struct sk_buff *
wifi_mac_defrag(struct wifi_station *sta, struct sk_buff *skb, int hdrlen)
{
    struct wifi_frame *wh = (struct wifi_frame *) os_skb_data(skb);
    unsigned short rxseq, last_rxseq;
    unsigned char fragno, last_fragno;
    unsigned char more_frag = wh->i_fc[1] & WIFINET_FC1_MORE_FRAG;
    struct wifi_station_tbl *nt = &(sta->sta_wnet_vif->vm_sta_tbl);

    rxseq = le16_to_cpu(*(unsigned short *)wh->i_seq) >> WIFINET_SEQ_SEQ_SHIFT;
    fragno = le16_to_cpu(*(unsigned short *)wh->i_seq) & WIFINET_SEQ_FRAG_MASK;

    if (!more_frag && fragno == 0 && sta->sta_rxfrag[0] == NULL)
        return skb;

    if (nt == NULL)
    {
        os_skb_free(skb);
        return NULL;
    }

    sta->sta_rxfragstamp = jiffies;
    if (sta->sta_rxfrag[0])
    {
        struct wifi_frame *lwh;

        lwh = (struct wifi_frame *) sta->sta_rxfrag[0]->data;
        last_rxseq = le16_to_cpu(*(unsigned short *)lwh->i_seq) >>
                     WIFINET_SEQ_SEQ_SHIFT;
        last_fragno = le16_to_cpu(*(unsigned short *)lwh->i_seq) &
                      WIFINET_SEQ_FRAG_MASK;
        if (rxseq != last_rxseq
            || fragno != last_fragno + 1
            || (!WIFINET_ADDR_EQ(wh->i_addr1, lwh->i_addr1))
            || (!WIFINET_ADDR_EQ(wh->i_addr2, lwh->i_addr2))
            || ((sta->sta_rxfrag[0]->end - sta->sta_rxfrag[0]->tail) <
                os_skb_get_pktlen(skb)))
        {

            os_skb_free(sta->sta_rxfrag[0]);
            sta->sta_rxfrag[0] = NULL;
        }
    }

    if (sta->sta_rxfrag[0] == NULL && fragno == 0)      //the 1st fragment
    {
        sta->sta_rxfrag[0] = skb;
        if (more_frag)
        {
            if (skb_is_nonlinear(skb))
            {
                sta->sta_rxfrag[0] = os_skb_copy(skb, GFP_ATOMIC, sta->sta_rxfrag[0]);
                os_skb_free(skb);

            }
        #if defined (SYSTEM64)
            else if (skb_end_offset(skb) < sta->sta_wnet_vif->vm_ndev->mtu + hdrlen)
        #else
            else if (skb->end - skb->head < sta->sta_wnet_vif->vm_ndev->mtu + hdrlen)
        #endif
            {
                sta->sta_rxfrag[0] = os_skb_copy_expand(skb, 32,
                    (sta->sta_wnet_vif->vm_ndev->mtu + hdrlen) + 32 - os_skb_get_pktlen(skb), GFP_ATOMIC, sta->sta_rxfrag[0]);
                os_skb_free(skb);
            }
        }
    }
    else
    {
        if (sta->sta_rxfrag[0])     //the continue fragment
        {
            struct wifi_frame *lwh = (struct wifi_frame *)
                                          sta->sta_rxfrag[0]->data;

            memcpy(skb_tail_pointer(sta->sta_rxfrag[0]),
                   os_skb_data(skb) + hdrlen, os_skb_get_pktlen(skb) - hdrlen);
            os_skb_put(sta->sta_rxfrag[0], os_skb_get_pktlen(skb) - hdrlen);
            *(unsigned short *) lwh->i_seq = *(unsigned short *) wh->i_seq;

        }

        os_skb_free(skb);
    }

    if (more_frag)
    {
        skb = NULL;
    }
    else
    {
        //the fragmentation ended
        skb = sta->sta_rxfrag[0];

        sta->sta_rxfrag[0] = NULL;

        if(skb != NULL)
        {
            M_FLAG_CLR(skb, M_TKIPMICHW);   //means sw cal tkip mic
        }
    }

    return skb;
}

unsigned short
wifi_mac_eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    struct ethhdr *eth;

    skb_reset_mac_header(skb);

    os_skb_pull(skb, ETH_HLEN);


    eth= (struct ethhdr *)skb_mac_header(skb);


    if (*eth->h_dest&1)
    {
        if (memcmp(eth->h_dest,dev->broadcast, ETH_ALEN)==0)
            skb->pkt_type=PACKET_BROADCAST;
        else
            skb->pkt_type=PACKET_MULTICAST;
    }
    else
    {
        if (memcmp(eth->h_dest,dev->dev_addr, ETH_ALEN))
            skb->pkt_type=PACKET_OTHERHOST;
    }

    return eth->h_proto;
}

static void wifi_mac_tcp_pkt_retransmit(struct wifi_station *sta, struct tcphdr *th)
{
    struct drv_txdesc *ptxdesc;
    struct list_head *txdesc_queue;
    struct wifi_mac_pkt_info *mac_pkt_info;
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct drv_private *drv_priv = wifimac->drv_priv;
    struct drv_txlist *txlist;
    unsigned int i;

    for (i=0; i<HAL_NUM_TX_QUEUES; i++)
    {
        txlist = &drv_priv->drv_txlist_table[i];
        txdesc_queue = &txlist->tx_tcp_queue;

        while (1)
        {
            if (list_empty(txdesc_queue))
            {
                break;
            }

            ptxdesc = list_first_entry(txdesc_queue, struct drv_txdesc, txdesc_queue);
            mac_pkt_info = &ptxdesc->txinfo->ptxdesc->drv_pkt_info.pkt_info[0];

            if (mac_pkt_info->b_tcp_free)
            {
                DRV_TXQ_LOCK(txlist);
                list_del_init(&ptxdesc->txdesc_queue);
                DRV_TXQ_UNLOCK(txlist);

                wifi_mac_complete_wbuf(ptxdesc->txdesc_mpdu, 0);

                continue;
            }
            else
            {
                txdesc_queue = txdesc_queue->next;

                if (list_is_last(txdesc_queue, &txlist->tx_tcp_queue))
                {
                    break;
                }
            }
        }
    }
}

static void wifi_mac_recv_pkt_parse(struct wifi_station *sta, struct sk_buff *skb) {
    struct ether_header *eh = (struct ether_header *)((unsigned char *)os_skb_data(skb));
    struct iphdr *iphdrp = (struct iphdr *)((unsigned char *)eh + sizeof(struct ether_header));
    struct tcphdr *th = (struct tcphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
    struct udphdr *uh = (struct udphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
    unsigned char *p_eap = NULL;
    unsigned char i = 0;
    unsigned char *dhcp_p;
    unsigned short offset;
    unsigned short offset_max;

    static char start_flag = 0;
    static unsigned long in_time = 0;
    static unsigned long tcp_rx_payload_total = 0;

    if (eh->ether_type == __constant_htons(ETHERTYPE_PAE)) {
        p_eap = (unsigned char *)(eh+1);
        if (p_eap[1] == 3 && p_eap[4] == 2) {
            AML_OUTPUT("recv eapol frame, len:%d, key_info:%04x\n",  (p_eap[2] << 8) | p_eap[3], (p_eap[5] << 8) | p_eap[6]);
        }

    } else if (eh->ether_type == __constant_htons(ETHERTYPE_IP)) {
        if (iphdrp->protocol == IPPROTO_TCP) {
            AML_PRINT(AML_DBG_MODULES_TX, "tcp src:%d, dst:%d, seq:%u, tcp ack:%u\n",  __constant_htons(th->source),  __constant_htons(th->dest),
                __constant_htonl(th->seq), __constant_htonl(th->ack_seq));
            if (!start_flag) {
                start_flag = 1;
                in_time = jiffies;
            }

            tcp_rx_payload_total += (unsigned long)(skb->data + skb->len) - (unsigned long)((unsigned char *)th + (th->doff << 2));

            if (time_after(jiffies, in_time + HZ)) {
                sta->sta_wnet_vif->vm_rx_speed = tcp_rx_payload_total >> 17;
                start_flag = 0;
                tcp_rx_payload_total = 0;
            }

#ifdef DRV_TCP_RETRANSMISSION
            wifi_mac_tcp_pkt_retransmit(sta, th);
#endif

        } else if (iphdrp->protocol == IPPROTO_UDP) {
         if (((uh->source == 0x4400) && (uh->dest == 0x4300))
                || ((uh->source == 0x4300) && (uh->dest == 0x4400))) {
                //pr_debug("%s source:%04x, dest:%04x\n", __func__, uh->source, uh->dest);
                if (sta->connect_status == CONNECT_DHCP_GET_ACK) {
                    return;
                }
                dhcp_p = (unsigned char *)((unsigned char *)uh + 8);
                offset = 240;
                offset_max = os_skb_get_pktlen(skb);

                while ((dhcp_p[offset] != 255) && (offset < offset_max))
                {
                    if (dhcp_p[offset] == 53) {
                        AML_OUTPUT("dhcp recv state : %d\n", dhcp_p[offset + 2]);
                        break;
                    }

                    offset += dhcp_p[offset + 1] + 2;
                }
            } else {
                for (i = 0; i < udp_cnt; i++) {
                    if ((uh->source == __constant_htons(aml_udp_info[i].src_port)) && (uh->dest == __constant_htons(aml_udp_info[i].dst_port)) && (!aml_udp_info[i].out)) {
                        if (aml_udp_info[i].dst_ip == __constant_htonl(iphdrp->daddr)) {
                            aml_udp_info[i].rx += 1;
                        }
                    }
                }
            }
        }
    }
}

static void
wifi_mac_deliver_data(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct net_device *dev = wnet_vif->vm_ndev;
    struct ether_header *eh;

    /* TODO this code will cause WARN, it is used to router function, but cause dump, need t.b.d*/
    if (skb != NULL)
    {
        eh = (struct ether_header *) os_skb_data(skb);

        if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_flags_ext & WIFINET_C_STA_FORWARD))
        {
            if (eh->ether_type == ETHERTYPE_PAE)
            {
                WIFINET_ADDR_COPY(eh->ether_dhost, dev->dev_addr);
            }
        }

        wifi_mac_recv_pkt_parse(sta, skb);
        skb->dev = dev;
        skb->protocol = wifi_mac_eth_type_trans(skb, dev);

        if (sta->sta_vlan != 0 && wnet_vif->vm_vlgrp != NULL)
        {
           // none
        }
        else
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
            local_bh_disable();
            netif_receive_skb(skb);
            local_bh_enable();
#else
            netif_rx(skb);
#endif

            os_skb_count_free(skb);
            wnet_vif->vif_sts.sts_rx_msdu++;
            wnet_vif->vif_sts.sts_rx_msdu_time_stamp = jiffies;
        }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
        dev->last_rx = jiffies;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0) */
    }
}

static struct sk_buff *
wifi_mac_decap(struct wlan_net_vif *wnet_vif, struct sk_buff *skb, int hdrlen, int is_fwd)
{
    struct WIFINET_S_FRAME_QOS_ADDR4 wh;
    struct ether_header *eh;
    struct llc *llc;
    u_short ether_type = 0;
    struct wifi_qos_frame qwh;

    memcpy(&wh, os_skb_data(skb), MIN(hdrlen, sizeof(wh)));
    memcpy(&qwh, os_skb_data(skb), MIN(hdrlen, sizeof(struct wifi_qos_frame)));
    if (is_fwd) {
        if (((qwh.i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_QOS)
            && (qwh.i_qos[0] & WIFINET_QOS_AMSDU)) {
            hdrlen += 14;
        }
    }

    llc = (struct llc *) os_skb_pull(skb, hdrlen);  //stripped 802.11 header
    if (llc == NULL)
    {
        return NULL;
    }
    if (os_skb_get_pktlen(skb) >= LLC_SNAPFRAMELEN &&
        llc->llc_dsap == LLC_SNAP_LSAP && llc->llc_ssap == LLC_SNAP_LSAP &&
        llc->llc_control == LLC_UI && llc->llc_snap.org_code[0] == 0 &&
        llc->llc_snap.org_code[1] == 0 && llc->llc_snap.org_code[2] == 0)
    {
        ether_type = llc->llc_un.type_snap.ether_type;

        os_skb_pull(skb, LLC_SNAPFRAMELEN); //stripped LLC SNAP
        llc = NULL;
    }
    //added ethernet/802.3MAC header
    eh = (struct ether_header *) os_skb_push(skb, sizeof(struct ether_header));
    switch (wh.i_fc[1] & WIFINET_FC1_DIR_MASK)
    {
        case WIFINET_FC1_DIR_NODS:
            WIFINET_ADDR_COPY(eh->ether_dhost, wh.i_addr1);
            WIFINET_ADDR_COPY(eh->ether_shost, wh.i_addr2);
            break;
        case WIFINET_FC1_DIR_TODS:
            WIFINET_ADDR_COPY(eh->ether_dhost, wh.i_addr3);
            WIFINET_ADDR_COPY(eh->ether_shost, wh.i_addr2);
            break;
        case WIFINET_FC1_DIR_FROMDS:
            WIFINET_ADDR_COPY(eh->ether_dhost, wh.i_addr1);
            WIFINET_ADDR_COPY(eh->ether_shost, wh.i_addr3);
            break;
        case WIFINET_FC1_DIR_DSTODS:
            WIFINET_ADDR_COPY(eh->ether_dhost, wh.i_addr3);
            WIFINET_ADDR_COPY(eh->ether_shost, wh.i_addr4);
            break;
    }
    if (!ALIGNED_POINTER(os_skb_data(skb) + sizeof(*eh), unsigned int))
    {
        struct sk_buff *n;

        n = os_skb_copy(skb, GFP_ATOMIC, n);
        os_skb_free(skb);
        if (n == NULL)
            return NULL;
        skb = n;
        eh = (struct ether_header *) os_skb_data(skb);
    }

    if (llc != NULL)
    {
        /* if rx packet is not LLC SNAP, then ether_type should be included in upper layer.
         * and here, ether_type of the header is filled with length according to 802.3 MAC header
         */
        eh->ether_type = __constant_htons(os_skb_get_pktlen(skb) - sizeof(*eh));
    }
    else
    {
        /* if rx packet is LLC, ether_type is filled from LLC according to LLC SNAP.
          */
        eh->ether_type = ether_type;
    }
    return skb;
}

static void
wifi_mac_auth_open(struct wifi_station *sta, struct wifi_frame *wh,
                  int rssi, unsigned int rstamp, unsigned short seq, unsigned short status)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int arg;

    if (sta->sta_authmode == WIFINET_AUTH_SHARED)
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                                 sta->sta_macaddr, "open auth bad sta auth mode %u", sta->sta_authmode);
        wnet_vif->vif_sts.sts_rx_auth_mismatch++;
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
        {
            if (sta == wnet_vif->vm_mainsta)
            {
                sta = wifi_mac_bup_bss(wnet_vif, wh->i_addr2);
                if (sta == NULL)
                    return;
            }
            arg = (seq + 1) | (WIFINET_STATUS_ALG<<16);

            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&arg);
            return;
        }
    }
    switch (wnet_vif->vm_opmode)
    {
        case WIFINET_M_IBSS:
            if (wnet_vif->vm_state != WIFINET_S_CONNECTED || seq != WIFINET_AUTH_OPEN_REQUEST)
            {
                wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                return;
            }
            wifi_mac_top_sm(wnet_vif, WIFINET_S_AUTH, wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK);
            break;

        case WIFINET_M_HOSTAP:
            if (wnet_vif->vm_state != WIFINET_S_CONNECTED || seq != WIFINET_AUTH_OPEN_REQUEST)
            {
                wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                return;
            }
            /*here, ap's vm_mainsta pointer to itself. */
            if (sta == wnet_vif->vm_mainsta)
            {
                WIFINET_DPRINTF_STA( AML_DEBUG_DEBUG | AML_DEBUG_CONNECT,  sta, "station authenticated (%s)", "drop");
                /*alloc a new sta buf for connecting sta. */
                sta = wifi_mac_bup_bss(wnet_vif, wh->i_addr2);
                if (sta == NULL)
                    return;
            }
            sta->sta_flags |= WIFINET_NODE_AREF;

            arg = seq + 1;
            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&arg);
            WIFINET_DPRINTF_STA( AML_DEBUG_DEBUG | AML_DEBUG_CONNECT, sta, "station authenticated (%s)", "open");

            wifi_mac_sta_auth(sta);
            break;

        case WIFINET_M_STA:
            if ((wnet_vif->vm_state != WIFINET_S_AUTH) || (seq != WIFINET_AUTH_OPEN_RESPONSE)) {
                wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                pr_debug("<running> %s %d seq0x%x \n",__func__,__LINE__, seq);
                return;
            }

            /*status in authentication frame is equal to 0, that meanings succeed */
            if (status != 0) {
                WIFINET_DPRINTF_STA(AML_DEBUG_DEBUG | AML_DEBUG_CONNECT, sta, "open auth failed (reason %d)", status);
                wnet_vif->vif_sts.sts_rx_auth_fail++;
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);

            } else {
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                /*authentication success and change SM to send assoc req. */
                wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);
            }
            break;

        case WIFINET_M_MONITOR:
            break;
        case WIFINET_M_WDS:
            break;
        case WIFINET_M_P2P_CLIENT:
            break;
        case WIFINET_M_P2P_GO:
            break;
        case WIFINET_M_P2P_DEV:
            break;
    }
}

static void wifi_mac_auth_sae(struct wifi_station *sta, struct sk_buff *skb,
    unsigned short seq, struct wifi_frame *wh, unsigned short status, unsigned int channel)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    pr_debug("%s WIFINET_AUTH_ALG_SAE\n", __func__);

    switch (wnet_vif->vm_opmode) {
        case WIFINET_M_HOSTAP:
            if (wnet_vif->vm_state != WIFINET_S_CONNECTED) {
                return;
            }

            if (sta == wnet_vif->vm_mainsta) {
                /*alloc a new sta buf for connecting sta. */
                sta = wifi_mac_bup_bss(wnet_vif, wh->i_addr2);
                if (sta == NULL)
                    return;

            }
            sta->sta_flags |= WIFINET_NODE_AREF;
            memcpy(sta->sta_essid, wnet_vif->vm_mainsta->sta_essid, wnet_vif->vm_mainsta->sta_esslen);
            sta->sta_authmode = WIFINET_AUTH_SAE;
            vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb),os_skb_get_pktlen(skb));
            break;

        case WIFINET_M_STA:
            if (wnet_vif->vm_state != WIFINET_S_AUTH) {
                ERROR_DEBUG_OUT("<running>seq0x%x \n", seq);
                return;
            }

            if (status != 0) {
                WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "sae auth failed (reason %d)", status);
            }

            vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb),os_skb_get_pktlen(skb));
            break;

        default:
            break;
    }
}

static void
wifi_mac_send_error(struct wifi_station *sta, const unsigned char *mac, int subtype, long long  arg)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int istmp = 0;

    if (sta == wnet_vif->vm_mainsta) {
        sta = wifi_mac_tmp_nsta(wnet_vif, mac);
        if (sta == NULL) {
            return;
        }

        istmp = 1;
    }

    wifi_mac_send_mgmt(sta, subtype, (void *)&arg);
    if (istmp) {
        wifi_mac_free_sta(sta);
    }
}

static int
alloc_challenge(struct wifi_station *sta)
{
    // struct WIFINET_VMAC *vmac = sta->sta_vmac;
    if (sta->sta_challenge == NULL)
        sta->sta_challenge = (unsigned int *)NET_MALLOC(WIFINET_CHALLENGE_LEN,
            GFP_ATOMIC, "alloc_challenge.sta->sta_challenge");
    if (sta->sta_challenge == NULL)
    {
        WIFINET_DPRINTF_STA(AML_DEBUG_DEBUG | AML_DEBUG_CONNECT, sta,
                            "%s", "shared key challenge alloc failed");
    }
    return (sta->sta_challenge != NULL);
}
static const unsigned char *
wifi_mac_getbssid(struct wlan_net_vif *wnet_vif, const struct wifi_frame *wh)
{
    if (wnet_vif->vm_opmode == WIFINET_M_STA)
        return wh->i_addr2;

    if ((wh->i_fc[1] & WIFINET_FC1_DIR_MASK) != WIFINET_FC1_DIR_NODS)
        return wh->i_addr1;

    if (WIFINET_IS_PSPOLL(wh))
        return wh->i_addr1;

    return wh->i_addr3;
}

static void
wifi_mac_auth_shared(struct wifi_station *sta, struct wifi_frame *wh,
                    unsigned char *frm, unsigned char *efrm, int rssi, unsigned int rstamp,
                    unsigned short seq, unsigned short status)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    unsigned char *challenge;
    int allocbs, estatus, arg;
    estatus = 0;
    if ((wnet_vif->vm_flags & WIFINET_F_PRIVACY) == 0)
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                                 sta->sta_macaddr, "shared key auth %s", " PRIVACY is disabled");
        estatus = WIFINET_STATUS_ALG;
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        goto bad;
    }
    if (sta->sta_authmode != WIFINET_AUTH_SHARED)
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                                 sta->sta_macaddr, "shared key auth bad sta auth mode %u", sta->sta_authmode);
        wnet_vif->vif_sts.sts_rx_auth_mismatch++;
        estatus = WIFINET_STATUS_ALG;
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        goto bad;
    }

    challenge = NULL;
    if (frm + 1 < efrm)
    {
        if ((frm[1] + 2) > (efrm - frm))
        {
        #if defined (SYSTEM64)
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                                     sta->sta_macaddr, "shared key auth ie %d/%ld too long",
                                     frm[0], (frm[1] + 2) - (efrm - frm));
		#else
			WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
								 sta->sta_macaddr, "shared key auth ie %d/%d too long",
								 frm[0], (frm[1] + 2) - (efrm - frm));
		#endif
            wnet_vif->vif_sts.sts_rx_auth_mismatch++;
            estatus = WIFINET_STATUS_CHALLENGE;
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            goto bad;
        }
        if (*frm == WIFINET_ELEMID_CHALLENGE)
            challenge = frm;
        frm += frm[1] + 2;
    }
    switch (seq)
    {
        case WIFINET_AUTH_SHARED_CHALLENGE:
        case WIFINET_AUTH_SHARED_RESPONSE:
            if (challenge == NULL)
            {
                WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                                         sta->sta_macaddr, "shared key auth %s", "no challenge");
                wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                estatus = WIFINET_STATUS_CHALLENGE;
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                goto bad;
            }
            if (challenge[1] != WIFINET_CHALLENGE_LEN)
            {
                WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                    sta->sta_macaddr, "shared key auth bad challenge len %d", challenge[1]);
                wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                estatus = WIFINET_STATUS_CHALLENGE;
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                goto bad;
            }
        default:
            break;
    }
    switch (wnet_vif->vm_opmode)
    {
        case WIFINET_M_MONITOR:
        case WIFINET_M_IBSS:
        case WIFINET_M_WDS:
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                sta->sta_macaddr, "shared key auth bad operating mode %u", wnet_vif->vm_opmode);
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            return;
        case WIFINET_M_HOSTAP:
            if (wnet_vif->vm_state != WIFINET_S_CONNECTED)
            {
                WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                    sta->sta_macaddr, "shared key auth bad state %u", wnet_vif->vm_state);
                estatus = WIFINET_STATUS_ALG;
                goto bad;
            }
            switch (seq)
            {
                case WIFINET_AUTH_SHARED_REQUEST:
                    if (sta == wnet_vif->vm_mainsta)
                    {
                        pr_debug("<running> %s %d \n",__func__,__LINE__);
                        sta = wifi_mac_bup_bss(wnet_vif, wh->i_addr2);
                        if (sta == NULL)
                        {
                            return;
                        }
                        allocbs = 1;
                    }
                    else
                    {
                        allocbs = 0;
                    }
                    sta->sta_flags |= WIFINET_NODE_AREF;
                    sta->sta_rstamp = jiffies;
                    if (!alloc_challenge(sta))
                    {
                        return;
                    }
                    get_random_bytes(sta->sta_challenge, WIFINET_CHALLENGE_LEN);
                    WIFINET_DPRINTF_STA(AML_DEBUG_DEBUG | AML_DEBUG_CONNECT, sta,
                        "shared key %s auth request", allocbs ? "" : "re");
                    break;

                case WIFINET_AUTH_SHARED_RESPONSE:
                    if (sta == wnet_vif->vm_mainsta)
                    {
                        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                            sta->sta_macaddr, "shared key response %s", "unknown station");
                        return;
                    }
                    if (sta->sta_challenge == NULL)
                    {
                        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                            sta->sta_macaddr, "shared key response %s", "no challenge recorded");
                        wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                        estatus = WIFINET_STATUS_CHALLENGE;
                        goto bad;
                    }
                    if (memcmp(sta->sta_challenge, &challenge[2],
                               challenge[1]) != 0)
                    {
                        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                            sta->sta_macaddr, "shared key response %s", "challenge mismatch");
                        wnet_vif->vif_sts.sts_rx_auth_fail++;
                        estatus = WIFINET_STATUS_CHALLENGE;
                        goto bad;
                    }
                    WIFINET_DPRINTF_STA(AML_DEBUG_DEBUG | AML_DEBUG_CONNECT, sta,
                        "station authenticated (%s)", "shared key");
                    wifi_mac_sta_auth(sta);
                    break;
                default:
                    WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT,
                        sta->sta_macaddr, "shared key auth bad seq %d", seq);
                    wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                    estatus = WIFINET_STATUS_SEQUENCE;
                    goto bad;
            }
            arg = seq + 1;
            pr_debug("%s(%d) tx_auth 8\n", __func__, __LINE__);
            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&arg);
            break;

        case WIFINET_M_STA:
            if (wnet_vif->vm_state != WIFINET_S_AUTH)
                return;
            switch (seq)
            {
                case WIFINET_AUTH_SHARED_PASS:
                    if (sta->sta_challenge != NULL)
                    {
                        FREE(sta->sta_challenge,"sta->sta_challenge");
                        sta->sta_challenge = NULL;
                    }
                    if (status != 0)
                    {
                        WIFINET_DPRINTF_MACADDR(
                            AML_DEBUG_DEBUG | AML_DEBUG_CONNECT,
                            wifi_mac_getbssid(wnet_vif, wh),
                            "shared key auth failed (reason %d)",
                            status);
                        wnet_vif->vif_sts.sts_rx_auth_fail++;
                        goto bad;
                    }
                    pr_debug("<running> %s %d \n",__func__,__LINE__);
                    wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);
                    break;
                case WIFINET_AUTH_SHARED_CHALLENGE:
                    if (!alloc_challenge(sta))
                        goto bad;
                    memcpy(sta->sta_challenge, &challenge[2], challenge[1]);
                    arg = seq + 1;
                    pr_debug("<running> %s %d \n",__func__,__LINE__);
pr_debug("%s(%d) tx_auth 9\n", __func__, __LINE__);
                    wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&arg);
                    break;
                default:
                    WIFINET_DPRINTF( AML_DEBUG_CONNECT,
                                     "shared key auth bad seq %d", seq);
                    wnet_vif->vif_sts.sts_rx_auth_mismatch++;
                    goto bad;
            }
            break;

        case WIFINET_M_P2P_GO:
        case WIFINET_M_P2P_DEV:
        case WIFINET_M_P2P_CLIENT:
            break;
    }
    return;
bad:
    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
    {
        wifi_mac_send_error(sta, wh->i_addr2, WIFINET_FC0_SUBTYPE_AUTH, (seq + 1) | (estatus<<16));
    }
    else if (wnet_vif->vm_opmode == WIFINET_M_STA)
    {
        if (wnet_vif->vm_state == WIFINET_S_AUTH)
        {
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
        }
    }
}

static int __inline
iswpaoui(const unsigned char *frm)
{
    return frm[1] > 3 && READ_32L((unsigned char *)(frm+2)) == ((WPA_OUI_TYPE<<24)|WPA_OUI);
}

static int __inline
iswmeoui(const unsigned char *frm)
{
    return frm[1] > 3 && READ_32L((unsigned char *)(frm+2)) == ((WME_OUI_TYPE<<24)|WME_OUI);
}

static int __inline
iswmeparam(const unsigned char *frm)
{
    return frm[1] > 5 && READ_32L((unsigned char *)(frm+2)) == ((WME_OUI_TYPE<<24)|WME_OUI) &&
           frm[6] == WME_PARAM_OUI_SUBTYPE;
}

static int __inline
iswmeinfo(const unsigned char *frm)
{
    return frm[1] > 5 && READ_32L((unsigned char *)(frm+2)) == ((WME_OUI_TYPE<<24)|WME_OUI) &&
           frm[6] == WME_INFO_OUI_SUBTYPE;
}



int iswpsoui(const unsigned char *frm)
{
    return frm[1] > 3 && READ_32B((unsigned char *)(frm+2)) == WSC_OUI_BE;
}

#ifdef CONFIG_P2P
int isp2poui(const unsigned char *frm)
{
    return frm[1] > 3 && READ_32B((unsigned char *)(frm+2)) == P2P_OUI_BE;
}
int is_only_11b_rates(const unsigned char *frm)
{
    return frm[1] == 4 && READ_32B((unsigned char *)(frm+2)) == 0x02040b16;
}

#ifdef CONFIG_WFD
int iswfdoui(const unsigned char *frm)
{
    return frm[1] > 3 && READ_32B((unsigned char *)(frm+2)) == WFD_OUI_BE;
}
#endif //CONFIG_WFD
#endif //CONFIG_P2P

#ifdef CONFIG_WAPI

static int  iswaioui(const unsigned char *frm)
{
    return frm[1] > 3 && READ_32B((unsigned char *)(frm+2)) == WAI_OUI_BE;
}
#endif

int wpa_cipher(unsigned char *sel, unsigned char *keylen)
{
    unsigned int w = READ_32L(sel);

    switch (w) {
        case WPA_SEL(WPA_CSE_NULL):
            return WIFINET_CIPHER_NONE;

        case WPA_SEL(WPA_CSE_WEP40):
            if (keylen)
                *keylen = 40 / NBBY;
            return WIFINET_CIPHER_WEP;

        case WPA_SEL(WPA_CSE_WEP104):
            if (keylen)
                *keylen = 104 / NBBY;
            return WIFINET_CIPHER_WEP;

        case WPA_SEL(WPA_CSE_TKIP):
            return WIFINET_CIPHER_TKIP;

        case WPA_SEL(WPA_CSE_CCMP):
            return WIFINET_CIPHER_AES_CCM;
    }
    return 32;
}

int wpa_keymgmt(unsigned char *sel)
{
    unsigned int w = READ_32L(sel);

    switch (w) {
        case WPA_SEL(WPA_ASE_8021X_UNSPEC):
            return WPA_ASE_8021X_UNSPEC;

        case WPA_SEL(WPA_ASE_8021X_PSK):
            return WPA_ASE_8021X_PSK;

        case WPA_SEL(WPA_ASE_NONE):
            return WPA_ASE_NONE;
    }
    return 0;
}

unsigned char is_need_to_print(unsigned char* frm) {
    struct wifi_mac_p2p_action_frame *p2p_act = NULL;
    struct wifi_mac_action *ia = NULL;
    char ret = 0;
    ia = (struct wifi_mac_action *)frm;
    switch (ia->ia_category) {
        case AML_CATEGORY_QOS:
        case AML_CATEGORY_SA_QUERY:
            ret = 1;
            break;
        case AML_CATEGORY_BACK:
            if (ia->ia_action == WIFINET_ACTION_BA_ADDBA_REQUEST
                || ia->ia_action == WIFINET_ACTION_BA_ADDBA_RESPONSE
                || ia->ia_action == WIFINET_ACTION_BA_DELBA) {
                ret = 1;
            }
            break;
        case AML_CATEGORY_PUBLIC:
            if (ia->ia_action == WIFINET_ACT_PUBLIC_BSSCOEXIST
                || ia->ia_action == WIFINET_ACT_PUBLIC_GAS_RSP
                || ia->ia_action == WIFINET_ACT_PUBLIC_P2P) {
                ret = 1;
            }
            break;
        case AML_CATEGORY_HT:
            if (ia->ia_action == WIFINET_ACTION_HT_TXCHWIDTH
                || ia->ia_action == WIFINET_ACTION_HT_SMPOWERSAVE) {
                ret = 1;
            }
            break;
        case AML_CATEGORY_VHT:
            if (ia->ia_action == WIFINET_ACTION_VHT_CMPRSSD_BMFRM
                || ia->ia_action == WIFINET_ACTION_VHT_GROUP_ID
                || ia->ia_action == WIFINET_ACTION_VHT_OPERATE_MODE_NOTIFICATION) {
                ret = 1;
            }
            break;
        case AML_CATEGORY_P2P:
            p2p_act = (struct wifi_mac_p2p_action_frame *)frm;
            if (p2p_act->subtype == P2P_PRESENCE_RESP
                || p2p_act->subtype == P2P_PRESENCE_REQ
                || p2p_act->subtype == P2P_GO_DISC_REQ) {
                ret = 1;
            }
            break;
        default:
            break;
    }
    return ret;
}


int wifi_mac_parse_wpa(struct wifi_mac_Rsnparms *rsn, unsigned char *frm, unsigned char own)
{
    unsigned char len = frm[1];
    unsigned int w;
    int n;

    if (len < 14) {
        ERROR_DEBUG_OUT("WPA too short, len %u\n", len);
        return WIFINET_REASON_IE_INVALID;
    }
    frm += 6, len -= 4;

    w = READ_16L(frm);
    if (w != WPA_VERSION) {
        ERROR_DEBUG_OUT("WPA bad version %u\n", w);
        return WIFINET_REASON_IE_INVALID;
    }
    frm += 2, len -= 2;

    w = wpa_cipher(frm, &rsn->rsn_mcastkeylen);
    if (own == 1) {
        rsn->rsn_mcastcipher = w;
    }

    if (w != rsn->rsn_mcastcipher) {
        pr_err("WPA mcast cipher mismatch; got %u, expected %u\n", w, rsn->rsn_mcastcipher);
        return WIFINET_REASON_IE_INVALID;
    }
    frm += 4, len -= 4;

    n = READ_16L(frm);
    frm += 2, len -= 2;
    if (len < n * 4 + 2) {
        ERROR_DEBUG_OUT("WPA ucast cipher data too short; len %u, n %u\n", len, n);
        return WIFINET_REASON_IE_INVALID;
    }

    w = 0;
    for (; n > 0; n--) {
        w |= (1 << wpa_cipher(frm, &rsn->rsn_ucastkeylen));
        frm += 4, len -= 4;
    }

    if (own == 1) {
        rsn->rsn_ucastcipherset = w;

    } else {
        w &= rsn->rsn_ucastcipherset;
    }

    if (w == 0) {
        ERROR_DEBUG_OUT("WPA ucast cipher set empty\n");
        return WIFINET_REASON_IE_INVALID;
    }

    if (own == 0) {
        if (w & (1 << WIFINET_CIPHER_AES_CCM))
            rsn->rsn_ucastcipher = WIFINET_CIPHER_AES_CCM;
        else
            rsn->rsn_ucastcipher = WIFINET_CIPHER_TKIP;
    }

    n = READ_16L(frm);
    frm += 2, len -= 2;
    if (len < n * 4) {
        ERROR_DEBUG_OUT("WPA key mgmt alg data too short; len %u, n %u\n", len, n);
        return WIFINET_REASON_IE_INVALID;
    }

    w = 0;
    for (; n > 0; n--) {
        w |= (1 << wpa_keymgmt(frm));
        frm += 4, len -= 4;
    }

    if (own == 1) {
        rsn->rsn_keymgmtset = w;

    } else {
        w &= rsn->rsn_keymgmtset;
    }

    if (w == 0) {
        ERROR_DEBUG_OUT("WPA no acceptable key mgmt alg\n");
        return WIFINET_REASON_IE_INVALID;
    }

    if (len >= 2) {
        rsn->sa_valid = 1;
        rsn->gtksa = frm[0];
        rsn->ptksa = frm[1];
        pr_debug("%s add gtk/ptk sa\n", __func__);
    }

    return 0;
}

int rsn_cipher(unsigned char *sel, unsigned char *keylen)
{
    unsigned int w = READ_32L(sel);

    switch (w) {
        case RSN_SEL(RSN_CSE_NULL):
            return WIFINET_CIPHER_NONE;

        case RSN_SEL(RSN_CSE_WEP40):
            if (keylen)
                *keylen = 40 / NBBY;
            return WIFINET_CIPHER_WEP;

        case RSN_SEL(RSN_CSE_WEP104):
            if (keylen)
                *keylen = 104 / NBBY;
            return WIFINET_CIPHER_WEP;

        case RSN_SEL(RSN_CSE_TKIP):
            return WIFINET_CIPHER_TKIP;

        case RSN_SEL(RSN_CSE_WRAP):
            return WIFINET_CIPHER_AES_OCB;

        case RSN_SEL(RSN_CSE_CCMP):
            return WIFINET_CIPHER_AES_CCM;

        case RSN_SEL(RSN_CSE_PSK_256):
            return WIFINET_CIPHER_AES_PSK_256;

        case RSN_SEL(RSN_CSE_SAE):
            return WIFINET_CIPHER_AES_SAE;
    }

    return 32;
}


int rsn_keymgmt(unsigned char *sel)
{
    unsigned int w = READ_32L(sel);

    switch (w) {
        case RSN_SEL(RSN_ASE_8021X_UNSPEC):
            return RSN_ASE_8021X_UNSPEC;

        case RSN_SEL(RSN_ASE_8021X_PSK):
            return RSN_ASE_8021X_PSK;

        case RSN_SEL(RSN_ASE_8021X_PSK_SHA256):
            return RSN_ASE_8021X_PSK_SHA256;

        case RSN_SEL(RSN_ASE_8021X_SAE):
            return RSN_ASE_8021X_SAE;

        case RSN_SEL(RSN_ASE_NONE):
            return RSN_ASE_NONE;

        case RSN_SEL(RSN_ASE_8021X_EAP_SHA256):
            return RSN_ASE_8021X_EAP_SHA256;
    }

    return 0;
}

int wifi_mac_parse_own_rsn(struct wifi_station *sta, unsigned char *frm)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_Rsnparms *rsn = &sta->sta_rsn;
    const unsigned char len = frm[IE_LEN_OFFSET];
    unsigned int w;
    unsigned char n;
    int index = 0;

    if (len < 10) {
        WIFINET_DPRINTF(AML_DEBUG_WARNING, "RSN too short, len %u", len);
        return WIFINET_REASON_IE_INVALID;
    }

    index += IE_HDR_LEN;

    w = READ_16L(frm + index);
    if (w != RSN_VERSION) {
        WIFINET_DPRINTF(AML_DEBUG_WARNING, "RSN bad version %u", w);
        return WIFINET_REASON_IE_INVALID;
    }
    index += 2;

    w = rsn_cipher(frm + index, &rsn->rsn_mcastkeylen);
    if ((w == 32) || (((wnet_vif->vm_caps & cipher2cap(w)) == 0) && !wifi_mac_security_available(w))) {
        DPRINTF(AML_DEBUG_ERROR, "%s %d wnet_vif->vm_caps=0x%x\n", __func__, __LINE__, wnet_vif->vm_caps);
        return WIFINET_REASON_IE_INVALID;
    }
    rsn->rsn_mcastcipher = w;
    index += OUI_LEN;

    n = READ_16L(frm + index);
    index += 2;

    if ((len + IE_HDR_LEN - index) < n * OUI_LEN + 2) {
        WIFINET_DPRINTF(AML_DEBUG_WARNING, "RSN ucast cipher data too short; len %u, n %u", (len + IE_HDR_LEN - index), n);
        return WIFINET_REASON_IE_INVALID;
    }

    w = 0;
    for (; n > 0; n--) {
        w = rsn_cipher(frm + index, &rsn->rsn_ucastkeylen);

        if ((w == 32) || (((wnet_vif->vm_caps & cipher2cap(w)) == 0) && !wifi_mac_security_available(w))) {
            DPRINTF(AML_DEBUG_ERROR, "%s %d wnet_vif->vm_caps=0x%x\n", __func__, __LINE__, wnet_vif->vm_caps);
            return WIFINET_REASON_IE_INVALID;
        }
        rsn->rsn_ucastcipherset |= (1 << w);
        index += OUI_LEN;
    }

    if (rsn->rsn_ucastcipherset & (1 << WIFINET_CIPHER_AES_CCM))
        rsn->rsn_ucastcipher = WIFINET_CIPHER_AES_CCM;
    else
        rsn->rsn_ucastcipher = WIFINET_CIPHER_TKIP;

    n = READ_16L(frm + index);
    index += 2;
    if ((len + IE_HDR_LEN - index) < n * OUI_LEN) {
        WIFINET_DPRINTF(AML_DEBUG_WARNING, "RSN key mgmt alg data too short; len %u, n %u", (len + IE_HDR_LEN - index), n);
        return WIFINET_REASON_IE_INVALID;
    }

    w = 0;
    for (; n > 0; n--) {
        w = rsn_keymgmt(frm + index);

        if (w == 0) {
            DPRINTF(AML_DEBUG_ERROR, "%s %d not support key mgmt;wnet_vif->vm_caps=0x%x\n", __func__, __LINE__, wnet_vif->vm_caps);
            return WIFINET_REASON_IE_INVALID;
        }
        rsn->rsn_keymgmtset |= (1 << w);
        index += OUI_LEN;
    }

    if ((len + IE_HDR_LEN - index) >= 2) {
        rsn->rsn_caps = READ_16L(frm + index);
        rsn->rsn_caps_offset = index;
        index += 2;

    }

    rsn->rsn_pmkid_count = 0;
    if ((len + IE_HDR_LEN - index) >= 2) {
        rsn->rsn_pmkid_count = READ_16L(frm + index);
        index += 2;
    }

    if (rsn->rsn_pmkid_count != 0) {
        if ((len + IE_HDR_LEN - index) >= rsn->rsn_pmkid_count * WPA_PMKID_LEN) {
            rsn->rsn_pmkid_offset = index;
            for (n = 0; n < rsn->rsn_pmkid_count; n++) {
                if (n == 0) {
                    memcpy(rsn->rsn_pmkid, frm + index, WPA_PMKID_LEN);
                }

                index += WPA_PMKID_LEN;
            }

        } else {
            ERROR_DEBUG_OUT("RSN rsn_pmkid_count wrong\n");
            return WIFINET_REASON_IE_INVALID;
        }

    } else {
        pr_err("not pmkid\n");
        rsn->rsn_pmkid_offset = 0;
    }

    if ((len + IE_HDR_LEN - index) >= OUI_LEN) {
        rsn->rsn_gmcipher = rsn_keymgmt(frm + index);
        index += OUI_LEN;
    }

    WIFINET_DPRINTF(AML_DEBUG_WARNING, "RSN_len:%d, pmkid_count:%d, pmkid_offset:%d", len, rsn->rsn_pmkid_count, rsn->rsn_pmkid_offset);
    return 0;
}

int wifi_mac_parse_counterpart_rsn(struct wifi_mac_Rsnparms *rsn, unsigned char *frm, unsigned char is_judge)
{
    unsigned char len = frm[1];
    unsigned int w;
    int n;
    unsigned char pmkid_offset = frm[1];

    if (len < 10) {
        pr_err("RSN too short, len %u\n", len);
        return WIFINET_REASON_IE_INVALID;
    }

    frm += 2;
    w = READ_16L(frm);
    if (w != RSN_VERSION) {
        ERROR_DEBUG_OUT("RSN bad version %u\n", w);
        return WIFINET_REASON_IE_INVALID;
    }
    frm += 2, len -= 2;

    w = rsn_cipher(frm, &rsn->rsn_mcastkeylen);
    if (w == 32) {
        ERROR_DEBUG_OUT("RSN mcast cipher wrong got %u\n", w);
        return WIFINET_REASON_IE_INVALID;
    }
    frm += 4, len -= 4;

    if (is_judge && (w != rsn->rsn_mcastcipher)) {
        ERROR_DEBUG_OUT("RSN mcast not matching\n");
        return WIFINET_REASON_IE_INVALID;
    }

    n = READ_16L(frm);
    frm += 2, len -= 2;
    if ((n <= 0) || (len < n * 4 + 2)) {
        ERROR_DEBUG_OUT("RSN ucast cipher data too short; len %u, n %u\n", len, n);
        if (is_judge) {
            return WIFINET_REASON_IE_INVALID;
        }
    }

    w = 0;
    for (; n > 0; n--) {
        w |= (1 << rsn_cipher(frm, &rsn->rsn_ucastkeylen));
        if (w == 32) {
            ERROR_DEBUG_OUT("RSN pair cipher wrong got %u\n", w);
            return WIFINET_REASON_IE_INVALID;
        }
        frm += 4, len -= 4;
    }

    if (is_judge && !(w & rsn->rsn_ucastcipherset)) {
        ERROR_DEBUG_OUT("RSN ucast cipherset not matching\n");
        return WIFINET_REASON_IE_INVALID;
    }

    n = READ_16L(frm);
    frm += 2, len -= 2;
    if ((n <= 0) || (len < n * 4)) {
        ERROR_DEBUG_OUT("RSN keymgmt data too short; len %u, n %u\n", len, n);
        if (is_judge) {
            return WIFINET_REASON_IE_INVALID;
        }
    }

    w = 0;
    for (; n > 0; n--) {
        w |= (1 << rsn_keymgmt(frm));
        if (w == 1) {
            ERROR_DEBUG_OUT("RSN akm cipher wrong got %u\n", w);
            return WIFINET_REASON_IE_INVALID;
        }
        frm += 4, len -= 4;
    }

    if (is_judge && !(w & rsn->rsn_keymgmtset)) {
        ERROR_DEBUG_OUT("RSN keymgmtset not matching\n");
        return WIFINET_REASON_IE_INVALID;
    }

    if (len >= 2) {
        rsn->rsn_caps = READ_16L(frm);
        rsn->rsn_caps_offset = pmkid_offset - len + 2;
        frm += 2;
        len -= 2;

    } else {
        if (!is_judge) {
            ERROR_DEBUG_OUT("no rsn cap\n");
            return WIFINET_REASON_IE_INVALID;
        }
    }

    rsn->rsn_pmkid_count = 0;
    if (len >= 2) {
        rsn->rsn_pmkid_count = READ_16L(frm);
        frm += 2;
        len -= 2;
    }

    if (rsn->rsn_pmkid_count != 0) {
        if (len >= rsn->rsn_pmkid_count * 16) {
            for (n = 0; n < rsn->rsn_pmkid_count; n++) {
                if (n == 0) {
                    memcpy(rsn->rsn_pmkid, frm, 16);
                }

                frm += 16;
                len -= 16;
            }

        } else {
            ERROR_DEBUG_OUT("RSN rsn_pmkid_count wrong\n");
            return WIFINET_REASON_IE_INVALID;
        }

    } else {
        //pr_err("not pmkid\n");
        rsn->rsn_pmkid_count = 0;
        rsn->rsn_pmkid_offset = 0;
    }

    if (len >= 4) {
        rsn->rsn_gmcipher = rsn_keymgmt(frm);
        frm += 4;
        len -= 4;

    } else {
        rsn->rsn_gmcipher = 0;
    }

    return 0;
}

void wifi_mac_savenie(unsigned char **iep, const unsigned char *ie, size_t ielen, char *name)
{
    if (*iep == NULL || (*iep)[1] != ie[1])
    {
        if (*iep != NULL)
            FREE(*iep, name);

        *iep = NET_MALLOC(ielen, GFP_ATOMIC, name);
    }
    if (*iep != NULL)
        memcpy(*iep, ie, ielen);
}

void
wifi_mac_saveie(unsigned char **iep, const unsigned char *ie, char *name)
{
    unsigned int ielen = ie[1]+2;
    wifi_mac_savenie(iep, ie, ielen, name);
}


static int
wifi_mac_parse_wmeie(unsigned char *frm, const struct wifi_frame *wh,
                    struct wifi_station *sta)
{
    unsigned int len = frm[1];
    unsigned char ac;

    if (len != 7)
    {
        //WIFINET_DPRINTF(AML_DEBUG_ELEMID | AML_DEBUG_WME, "WME IE too short, len %u", len);
        return -1;
    }
    /*get uapsd filed from wmm ie and set flag for local */
    sta->sta_uapsd = frm[WME_CAPINFO_IE_OFFSET];
    if (WME_STA_UAPSD_ENABLED(sta->sta_uapsd))
    {
        sta->sta_flags |= WIFINET_NODE_UAPSD;
        /*get max num of msdu or mmpdu in a uapsd transmission*/
        switch (WME_STA_UAPSD_MAXSP(sta->sta_uapsd))
        {
            case 1:
                sta->sta_uapsd_maxsp = 2;
                break;
            case 2:
                sta->sta_uapsd_maxsp = 4;
                break;
            case 3:
                sta->sta_uapsd_maxsp = 6;
                break;
            default:
                /*all buffered msdus and mmpdus */
                sta->sta_uapsd_maxsp = WME_UAPSD_NODE_MAXQDEPTH;
        }
        for (ac = 0; ac < WME_NUM_AC; ac++)
        {
            sta->sta_uapsd_ac_trigena[ac] = (WME_STA_UAPSD_AC_ENABLED(ac, sta->sta_uapsd)) ? 1:0;
            sta->sta_uapsd_ac_delivena[ac] = (WME_STA_UAPSD_AC_ENABLED(ac, sta->sta_uapsd)) ? 1:0;
        }
    }
    else
    {
        sta->sta_flags &= ~WIFINET_NODE_UAPSD;
    }
    WIFINET_DPRINTF_STA(AML_DEBUG_PWR_SAVE, sta,
                        "UAPSD bit settings from STA: %02x", sta->sta_uapsd);

    return 1;
}

static int
wifi_mac_parse_wmeparams(struct wlan_net_vif *wnet_vif, unsigned char *frm,
                        const struct wifi_frame *wh, unsigned char *qosinfo)
{
    struct wifi_mac_wme_state *wme = &wnet_vif->vm_wmac->wm_wme[wnet_vif->wnet_vif_id];
    unsigned int len = frm[1], qosinfo_count;
    int i;
    int ret=0;

    *qosinfo = 0;

    if (len < sizeof(struct wifi_mac_wme_param) - 2) {
        WIFINET_DPRINTF( AML_DEBUG_ELEMID | AML_DEBUG_WME, "WME too short, len %u", len);
        return -1;
    }

    *qosinfo = frm[__offsetof(struct wifi_mac_wme_param, param_qosInfo)];
    qosinfo_count = *qosinfo & WME_QOSINFO_COUNT;
    if (qosinfo_count == wme->wme_wmeChanParams.cap_info_count)
        return 0;

    frm += __offsetof(struct wifi_mac_wme_param, params_acParams);
    for (i = 0; i < WME_NUM_AC; i++) {
        struct wmeParams *wmep = &wme->wme_wmeChanParams.cap_wmeParams[i];

        if (wmep->wmep_acm != MS(frm[0], WME_PARAM_ACM)
            ||wmep->wmep_aifsn != MS(frm[0], WME_PARAM_AIFSN)
            ||wmep->wmep_logcwmin != MS(frm[1], WME_PARAM_LOGCWMIN)
            ||wmep->wmep_logcwmax != MS(frm[1], WME_PARAM_LOGCWMAX)
            ||wmep->wmep_txopLimit != READ_16L(frm+2)) {
            wmep->wmep_acm = MS(frm[0], WME_PARAM_ACM);
            wmep->wmep_aifsn = MS(frm[0], WME_PARAM_AIFSN);
            wmep->wmep_logcwmin = MS(frm[1], WME_PARAM_LOGCWMIN);
            wmep->wmep_logcwmax = MS(frm[1], WME_PARAM_LOGCWMAX);
            wmep->wmep_txopLimit = READ_16L(frm+2);
            frm += 4;
            ret ++;

        } else {
            frm += 4;
        }

        if (wmep->wmep_logcwmin > 2)
            wmep->wmep_logcwmin = 2;
    }
    wme->wme_wmeChanParams.cap_info_count = qosinfo_count;
    return ret;
}

static int
wifi_mac_parse_dothparams(struct wlan_net_vif *wnet_vif, unsigned char *frm,
                         struct wifi_mac_scan_param* sp, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned int len = frm[1];
    unsigned char chan, csa_cnt;

    if (len < 4-2)
    {
        WIFINET_DPRINTF(AML_DEBUG_ELEMID | AML_DEBUG_DOTH,
                        "channel switch too short, len %u", len);
        return -1;
    }
    wnet_vif->vm_mainsta->sta_channel_switch_mode = frm[2];
    chan = frm[3];
    csa_cnt = frm[4];

    if (wifi_mac_chan_num_avail(wifimac, chan)==false)
    {
        WIFINET_DPRINTF(AML_DEBUG_ELEMID | AML_DEBUG_DOTH,
                        "channel switch invalid channel %u", chan);
        return -1;
    }

    if (csa_cnt > 0)
    {
        int ret = -1;
        unsigned char ret_char = false;

        sta->sta_chan_switch_chan = chan;
        os_timer_ex_cancel(&sta->csa_timer, CANCEL_SLEEP);
        os_timer_ex_start_period(&sta->csa_timer, csa_cnt * wnet_vif->vm_bcn_intval);

        if (csa_cnt == 1) {
             AML_OUTPUT("start_csa count:%d target_chan:%d\n", csa_cnt, chan);
        }

        ret = ((ret_char==true)?0:-1);
        return ret;
    }

    return 0;
}

/*
Set to 0 for no restriction
Set to 1 for 1/4 us
Set to 2 for 1/2 us
Set to 3 for 1 us
Set to 4 for 2 us
Set to 5 for 4 us
Set to 6 for 8 us
Set to 7 for 16 us
 */
static unsigned int
wifi_mac_parse_mpdudensity(unsigned int mpdudensity)
{
    switch (mpdudensity)
    {
        case 0:
            return 0;
        case 1:
        case 2:
        case 3:
            return 1;
        case 4:
            return 2;
        case 5:
            return 4;
        case 6:
            return 8;
        case 7:
            return 16;
        default:
            return 0;
    }
}

void wifi_mac_parse_htcap(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_htcap *xhtcap =(struct wifi_mac_ie_htcap *)ie;
    struct wifi_mac_ie_htcap_cmn *htcap = &xhtcap->hc_ie;
    struct wifi_mac   *wifimac = sta->sta_wmac;
    int htcapval = le16toh(htcap->hc_cap);

    /*SMPS power save mode. */
    switch (htcapval & WIFINET_HTCAP_C_SM_MASK) {
        case WIFINET_HTCAP_DISABLE_SMPS:
            if (((sta->sta_htcap & WIFINET_HTCAP_C_SM_MASK) != WIFINET_HTCAP_DISABLE_SMPS)) {
                sta->sta_htcap &= (~WIFINET_HTCAP_C_SM_MASK);
                sta->sta_htcap |= WIFINET_HTCAP_DISABLE_SMPS;
                sta->sta_update_rate_flag = WIFINET_NODE_SM_EN;
            }
            break;

        case WIFINET_HTCAP_STATIC_SMPS:
            if (((sta->sta_htcap & WIFINET_HTCAP_C_SM_MASK) != WIFINET_HTCAP_STATIC_SMPS)) {
                sta->sta_htcap &= (~WIFINET_HTCAP_C_SM_MASK);
                sta->sta_htcap |= WIFINET_HTCAP_STATIC_SMPS;
                sta->sta_update_rate_flag = WIFINET_NODE_SM_PWRSAV_STAT;
            }
            break;

        case WIFINET_HTCAP_DYNAMIC_SMPS:
            if (((sta->sta_htcap & WIFINET_HTCAP_C_SM_MASK) != WIFINET_HTCAP_DYNAMIC_SMPS)) {
                sta->sta_htcap &= (~WIFINET_HTCAP_C_SM_MASK);
                sta->sta_htcap |= WIFINET_HTCAP_DYNAMIC_SMPS;
                sta->sta_update_rate_flag = WIFINET_NODE_SM_PWRSAV_DYN;
            }

        default:
            break;
    }

    /*check if support short GI */
    if ((htcapval & WIFINET_HTCAP_C_SHORTGI20) && (wifimac->wm_flags_ext & WIFINET_FEXT_SHORTGI_ENABLE)) {
        sta->sta_htcap |= WIFINET_HTCAP_C_SHORTGI20;
        wifimac->wm_flags_ext |= WIFINET_FEXT_SHORTGI20;

    } else {
        sta->sta_htcap &= ~WIFINET_HTCAP_C_SHORTGI20;
        wifimac->wm_flags_ext &= (~WIFINET_FEXT_SHORTGI20);
    }

    if ((htcapval & WIFINET_HTCAP_C_SHORTGI40) && (wifimac->wm_flags_ext & WIFINET_FEXT_SHORTGI_ENABLE)) {
        sta->sta_htcap |= WIFINET_HTCAP_C_SHORTGI40;
        wifimac->wm_flags_ext |= WIFINET_FEXT_SHORTGI40;

    } else {
        sta->sta_htcap &= ~WIFINET_HTCAP_C_SHORTGI40;
        wifimac->wm_flags_ext &= (~WIFINET_FEXT_SHORTGI40);
    }

    /*check coexistence for 20MHz and 40MHz */
    if (!(htcapval & WIFINET_HTCAP_SUPPORTCBW40)) {
        /*only support 20MHz */
        sta->sta_chbw = WIFINET_BWC_WIDTH20;
        sta->sta_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;

    } else {
        /*both support 20MHz and 40MHz*/
        sta->sta_chbw = WIFINET_BWC_WIDTH40;
        sta->sta_htcap |= WIFINET_HTCAP_SUPPORTCBW40;
    }

    if ((sta->sta_wnet_vif) && (sta->sta_chbw > sta->sta_wnet_vif->vm_bandwidth)) {
        /*vm_bandwidth keep the max bw vmac can support */
        sta->sta_chbw = sta->sta_wnet_vif->vm_bandwidth;
    }

    if ((sta->sta_wnet_vif) && (sta->sta_wnet_vif->vm_curchan != NULL) && (sta->sta_chbw != sta->sta_wnet_vif->vm_curchan->chan_bw)) {
        if (sta->sta_wnet_vif->vm_opmode != WIFINET_M_HOSTAP) {
            sta->sta_chbw = sta->sta_wnet_vif->vm_curchan->chan_bw;
        }
    }

    if (sta->sta_wnet_vif) {
        DPRINTF(AML_DEBUG_BWC, "%s(%d) sta_chbw %d, wnet_vif_bandwidth:%d\n", __func__, __LINE__,  sta->sta_chbw, sta->sta_wnet_vif->vm_bandwidth);
    }

    if ((htcapval & WIFINET_HTCAP_40M_INTOLERANT) && (WIFINET_IS_CHAN_11N_HT40(wifimac->wm_curchan))) {
        sta->sta_flags_ext |= WIFINET_NODE_40_INTOLERANT;
    }

     /*negotiate  LDPC capability*/
    if ((htcapval & WIFINET_HTCAP_LDPC) && (wifimac->wm_flags & WIFINET_F_LDPC)) {
        sta->sta_flags |= WIFINET_NODE_LDPC;
        sta->sta_flags |= WIFINET_F_LDPC;

    } else {
        sta->sta_flags &= ~WIFINET_NODE_LDPC;
        sta->sta_flags &= ~WIFINET_F_LDPC;
    }
     /*negotiate  amsdu capability*/
    if (htcapval & WIFINET_HTCAP_C_MAXAMSDUSIZE ) /*Indicates support for receiving 7935 octets capability */
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, AMSDU_SIZE_7935);
    }
    else /*Indicates support for receiving 3839 octets capability */
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, AMSDU_SIZE_3839);
    }
    /*get amdpu information */
    sta->sta_maxampdu = ((1u << (WIFINET_HTCAP_MAXRXAMPDU_FACTOR + htcap->hc_maxampdu)) - 1);
    sta->sta_mpdudensity = wifi_mac_parse_mpdudensity(htcap->hc_mpdudensity);
    wifi_mac_set_ampduparams(sta);

    sta->sta_flags |= WIFINET_NODE_HT;
    if (htcap->hc_extcap&(WIFINET_HTCAP_EXTC_RDG_RESPONDER)) {
        sta->sta_flags_ext |= WIFINET_NODE_RDG;

    } else {
        sta->sta_flags_ext &= ~WIFINET_NODE_RDG;
    }
}

void
wifi_mac_parse_htinfo(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_htinfo *xhtinfo = (struct wifi_mac_ie_htinfo *)ie;
    struct wifi_mac_ie_htinfo_cmn  *htinfo = &xhtinfo->hi_ie;
    int8_t extoffset;
    struct wlan_net_vif  *wnet_vif = sta->sta_wnet_vif;

    /*get the second channel offset.*/
    switch (htinfo->hi_extchoff)
    {
        case WIFINET_HTINFO_EXTOFFSET_ABOVE:
            DPRINTF(AML_DEBUG_BEACON, "%s(%d) above\n\n\n",__func__, __LINE__);
            extoffset = SW_COFF_L10M;
            break;
        case WIFINET_HTINFO_EXTOFFSET_BELOW:
            DPRINTF(AML_DEBUG_BEACON, "%s(%d) below\n\n\n",__func__, __LINE__);
            extoffset = SW_COFF_U10M;
            break;
        case WIFINET_HTINFO_EXTOFFSET_NA:
        default:
            extoffset = 0;
            break;
    }

    /*check coexistence for 20MHz and 40MHz */
    if ((sta->sta_chbw == WIFINET_BWC_WIDTH20) && !(sta->sta_htcap & WIFINET_HTCAP_SUPPORTCBW40)) {
        DPRINTF(AML_DEBUG_BWC, "htcap and htinfo information conflict\n");
        return ;
    }

    sta->sta_chbw = wnet_vif->vm_bandwidth > htinfo->hi_txchwidth ?
        htinfo->hi_txchwidth : wnet_vif->vm_bandwidth ;

    if ((sta->sta_wnet_vif->vm_curchan != NULL) && (sta->sta_chbw != sta->sta_wnet_vif->vm_curchan->chan_bw)) {
        sta->sta_chbw = sta->sta_wnet_vif->vm_curchan->chan_bw;
    }
}


void wifi_mac_parse_vht_cap(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_vht_cap *vhtcap = (struct wifi_mac_ie_vht_cap *)ie;
    struct wifi_mac *wifimac;
    struct wlan_net_vif *wnet_vif;
    unsigned int  ampdu_len = 0;
    unsigned char tx_streams;   /* Default to max 1 tx spatial stream */
    unsigned char rx_streams;   /* Default to max 1 rx spatial stream */
    unsigned int  vht_cap_info;//keep the Ap's capability value

     if (vhtcap == NULL || sta == NULL)
     {
	    return;
     }
     wnet_vif = sta->sta_wnet_vif;
     wifimac = sta->sta_wmac;
     tx_streams = wifimac->wm_tx_streams;
     rx_streams = wifimac->wm_rx_streams;

     if (vhtcap->elem_id != WIFINET_ELEMID_VHTCAP)
     {
	   return ;
     }

    /* Negotiated capability set */
    vht_cap_info = le32toh(vhtcap->vht_cap_info);
    if ((vht_cap_info & WIFINET_VHTCAP_SHORTGI_80) && (wifimac->wm_flags_ext & WIFINET_FEXT_SHORTGI_ENABLE))
    {
        sta->sta_vhtcap |= WIFINET_VHTCAP_SHORTGI_80;
    }
    else
    {
        sta->sta_vhtcap &= ~WIFINET_VHTCAP_SHORTGI_80;
    }

    /*negotiate  LDPC capability*/
    if (vht_cap_info & WIFINET_VHTCAP_RX_LDPC &&  /*Indicates support for receiving LDPC coded packets*/
        (wifimac->wm_flags & WIFINET_F_LDPC)  )  /*our support LDPC TX and RX*/
    {
        sta->sta_flags |= WIFINET_NODE_LDPC;
        sta->sta_flags |= WIFINET_F_LDPC;
    }
    else
    {
        sta->sta_flags &= ~WIFINET_F_LDPC;
    }

    if (vht_cap_info & WIFINET_VHTCAP_RX_LDPC)
    {
        sta->sta_vhtcap  = sta->sta_vhtcap & ((wnet_vif->vm_ldpc & WIFINET_HTCAP_C_LDPC_RX) ? sta->sta_vhtcap  : ~WIFINET_VHTCAP_RX_LDPC);
        sta->sta_flags |= WIFINET_F_LDPC;
    }
    else
    {
        sta->sta_flags &= ~WIFINET_NODE_LDPC;
        sta->sta_flags &= ~WIFINET_F_LDPC;
    }

    if (vht_cap_info & WIFINET_VHTCAP_TX_STBC)
    {
        sta->sta_vhtcap  = sta->sta_vhtcap & (((wnet_vif->vm_tx_stbc) && (tx_streams >= 1)) ? sta->sta_vhtcap : ~WIFINET_VHTCAP_TX_STBC);
    }

    /* Tx on our side and Rx on the remote side should be considered for STBC with rate control */
    if (vht_cap_info & WIFINET_VHTCAP_RX_STBC)
    {
        sta->sta_vhtcap  = sta->sta_vhtcap & (((wnet_vif->vm_rx_stbc) && (rx_streams >= 1)) ? sta->sta_vhtcap : ~WIFINET_VHTCAP_RX_STBC);
    }

    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
    {
#ifdef SU_BF
        if (vht_cap_info & WIFINET_VHTCAP_SU_BFMEE)
        {
            sta->sta_flags |= WIFINET_F_SU_BF;
        }
        else
        {
            sta->sta_flags &= ~WIFINET_F_SU_BF;
        }
#endif
#ifdef MU_BF
        if (vht_cap_info & WIFINET_VHTCAP_MU_BFMEE)
        {
            sta->sta_flags |= WIFINET_F_MU_BF;
        }
        else
        {
            sta->sta_flags &= ~WIFINET_F_MU_BF;
        }
#endif
    }
    else
    {
#ifdef SU_BF
        if (vht_cap_info & WIFINET_VHTCAP_SU_BFMER)
        {
            sta->sta_flags |= WIFINET_F_SU_BF;
        }
        else
        {
            sta->sta_flags &= ~WIFINET_F_SU_BF;
        }
#endif
#ifdef MU_BF
        if (vht_cap_info & WIFINET_VHTCAP_MU_BFMER)
        {
            sta->sta_flags |= WIFINET_F_MU_BF;
        }
        else
        {
            sta->sta_flags &= ~WIFINET_F_MU_BF;
        }
#endif
    }

    if ( wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
    {
        /*set bandwidth for per sta connected. */
        switch(wnet_vif->vm_bandwidth) {
            case  WIFINET_BWC_WIDTH20:
                sta->sta_chbw = WIFINET_BWC_WIDTH20;
                break;

            case  WIFINET_BWC_WIDTH40:
                /* HTCAP Channelwidth will be set to max for VHT as well ? */
                if (!(sta->sta_htcap & WIFINET_HTCAP_SUPPORTCBW40))
                {
                    sta->sta_chbw =  WIFINET_BWC_WIDTH20;
                }
                else
                {
                    sta->sta_chbw =  WIFINET_BWC_WIDTH40;
                }
               break;

            case WIFINET_BWC_WIDTH80:
                if (!(sta->sta_htcap & WIFINET_HTCAP_SUPPORTCBW40))
                {
                    sta->sta_chbw =  WIFINET_BWC_WIDTH20;
                }
                else
                {
                    sta->sta_chbw =  WIFINET_BWC_WIDTH80;
                }
                break;

            default:
                /* Do nothing */
                break;
        }
    }
    DPRINTF(AML_DEBUG_BWC, "%s:%d, vm_chbw:%d, sta_chbw:%d \n", __func__, __LINE__, wnet_vif->vm_bandwidth, sta->sta_chbw);
    /*
     * The Maximum Rx A-MPDU defined by this field is equal to
     *   (2^^(13 + Maximum Rx A-MPDU Factor)) - 1
     * octets.  Maximum Rx A-MPDU Factor is an integer in the
     * range 0 to 7.
     */

    ampdu_len = (vht_cap_info & WIFINET_VHTCAP_MAX_AMPDU_LEN_EXP) >> WIFINET_VHTCAP_MAX_AMPDU_LEN_EXP_S;
    sta->sta_maxampdu = (1u << (WIFINET_VHTCAP_MAX_AMPDU_LEN_FACTOR + ampdu_len)) -1;
    if(vht_cap_info & WIFINET_VHTCAP_MAX_MPDU_LEN_11454 )
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, VHT_MPDU_SIZE_11451);
    }else if(vht_cap_info & WIFINET_VHTCAP_MAX_MPDU_LEN_7991 )
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, VHT_MPDU_SIZE_7991);
    }else
    {
         sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, VHT_MPDU_SIZE_3895);
    }
    sta->sta_flags |= WIFINET_NODE_VHT;
    sta->sta_tx_vhtrates = le16toh(vhtcap->tx_mcs_map);
    sta->sta_tx_max_rate = le16toh(vhtcap->tx_high_data_rate);
    sta->sta_rx_vhtrates = le16toh(vhtcap->rx_mcs_map);
    sta->sta_rx_max_rate = le16toh(vhtcap->rx_high_data_rate);
}

void wifi_mac_parse_vht_opt(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_vht_opt *vhtop = (struct wifi_mac_ie_vht_opt *)ie;
    struct wlan_net_vif  *wnet_vif;
    unsigned int peer_bw = 0;

    if (vhtop == NULL ||sta == NULL )
    {
        return ;
    }
    wnet_vif = sta->sta_wnet_vif;

    if(vhtop->elem_id != WIFINET_ELEMID_VHTOP)
    {
        return;
    }

    switch (vhtop->vht_op_chwidth)
    {
        case WIFINET_VHTOP_CHWIDTH_2040:
            if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
                peer_bw = WIFINET_BWC_WIDTH40;
                sta->sta_chbw =  peer_bw > wnet_vif->vm_bandwidth ?
                    wnet_vif->vm_bandwidth : peer_bw;
            }
            break;

        case WIFINET_VHTOP_CHWIDTH_80:
            peer_bw = WIFINET_BWC_WIDTH80;
            /*set bw for sta buffer, not change bw for ap. */
            sta->sta_chbw  = peer_bw > wnet_vif->vm_bandwidth ?
                wnet_vif->vm_bandwidth : peer_bw;
            break;

        default:
            ERROR_DEBUG_OUT( "Unsupported Channel Width\n");
            break;
    }

    sta->sta_vht_basic_mcs = le16toh(vhtop->vhtop_basic_mcs_set);
}

void wifi_mac_parse_vht_txpw(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_vht_txpwr_env* vht_txpwr_env = (struct wifi_mac_ie_vht_txpwr_env*) ie;

    if (vht_txpwr_env ==  NULL || sta == NULL ) {
      return ;
    }

    if (vht_txpwr_env->elem_id  !=  WIFINET_ELEMID_VHT_TX_PWR_ENVLP) {
      return ;
    }

    sta->sta_lmax_tx_pwr_cnt = GET_LMAX_TXPW_CNT(vht_txpwr_env->txpwr_info);
    sta->sta_lmax_tx_pwr_unt_intrp = GET_LMAX_TXPW_UNIT_INIT_CNT(vht_txpwr_env->txpwr_info);

    sta->sta_lmax_tx_pwr_20M = vht_txpwr_env->local_max_txpwr[0];
    sta->sta_lmax_tx_pwr_40M = vht_txpwr_env->local_max_txpwr[1];
    sta->sta_lmax_tx_pwr_80M = vht_txpwr_env->local_max_txpwr[2];
    sta->sta_lmax_tx_pwr_80P80M = vht_txpwr_env->local_max_txpwr[3];
}

void wifi_mac_parse_vht_ch_sw_wrp(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_vht_ch_sw_wrp* vht_ch_sw_wrp = (struct wifi_mac_ie_vht_ch_sw_wrp*) ie;
    unsigned char sub_ie_id = 0;
    unsigned char sub_ie_len = 0;
    unsigned char sub_ie_ofst = 2;

    if ((vht_ch_sw_wrp == NULL) || (sta == NULL)) {
        return ;
    }

    if (vht_ch_sw_wrp->elem_id != WIFINET_ELEMID_CHAN_SWITCH_WRAP) {
        return;
    }

    memset(sta->sta_new_country_sub_ie, 0, SUB_IE_MAX_LEN);
    memset(sta->sta_wide_bw_ch_sw_sub_ie, 0, SUB_IE_MAX_LEN);
    memset(sta->sta_new_vht_tx_pw_sub_ie, 0, SUB_IE_MAX_LEN);

    while (sub_ie_ofst < ie[1] + 2) {
        sub_ie_id = ie[sub_ie_ofst];
        sub_ie_len = ie[sub_ie_ofst + 1];

        AML_OUTPUT("sub_ie_id = %d, sub_id_len = %d\n", sub_ie_id, sub_ie_len);

        if (!(sub_ie_len <= SUB_IE_MAX_LEN - 2)) {
            ERROR_DEBUG_OUT("sub_ie_id[%d] : sub_ie_len[%d] is error\n",sub_ie_id,sub_ie_len);
            break;
        }

        if (sub_ie_id == WIFINET_ELEMID_COUNTRY) {
            memcpy(sta->sta_new_country_sub_ie, &ie[sub_ie_ofst], sub_ie_len + 2);
        } else if (sub_ie_id == WIFINET_ELEMID_WIDE_BAND_CHAN_SWITCH) {
            memcpy(sta->sta_wide_bw_ch_sw_sub_ie, &ie[sub_ie_ofst], sub_ie_len + 2);
        } else if (sub_ie_id == WIFINET_ELEMID_VHT_TX_PWR_ENVLP) {
            memcpy(sta->sta_new_vht_tx_pw_sub_ie, &ie[sub_ie_ofst], sub_ie_len + 2);
        }

        sub_ie_ofst += sub_ie_len + 2;
    }
}

void wifi_mac_parse_vht_ext_bss_ld(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_vht_ext_bss_ld* vht_ext_bss_ld = (struct wifi_mac_ie_vht_ext_bss_ld*)ie;

    if (vht_ext_bss_ld == NULL || sta == NULL) {
      return;
    }

    if (vht_ext_bss_ld->elem_id != WIFINET_ELEMID_EXT_BSS_LOAD) {
      return;
    }

    sta->sta_mu_mimo_sta_cnt = vht_ext_bss_ld->mu_mimo_sta_cnt;
    sta->sta_ss_ult = vht_ext_bss_ld->ss_ult;
    sta->sta_obs_scnd_20mhz = vht_ext_bss_ld->obs_scnd_20mhz;
    sta->sta_obs_scnd_40mhz = vht_ext_bss_ld->obs_scnd_40mhz;
    sta->sta_obs_scnd_80mhz = vht_ext_bss_ld->obs_scnd_80mhz;
}

void    wifi_mac_parse_vht_quiet_ch(struct wifi_station *sta, unsigned char *ie)
{
    struct wifi_mac_ie_vht_quiet_chn* vht_quiet_chn = (struct wifi_mac_ie_vht_quiet_chn*)ie;
    if (vht_quiet_chn == NULL || sta ==  NULL) {
        return;
    }

    if (vht_quiet_chn->elem_id !=  WIFINET_ELEMID_QUIET_CHANNEL) {
        return;
    }

    sta->sta_ap_quiet_md = vht_quiet_chn->ap_quiet_md;
    sta->sta_quiet_cnt = vht_quiet_chn->quiet_cnt;
    sta->sta_quiet_prd = vht_quiet_chn->quiet_prd;
    sta->sta_quiet_drt = vht_quiet_chn->quiet_drt;
    sta->sta_quiet_offset = vht_quiet_chn->quiet_offset;
}

void wifi_mac_parse_vht_op_md_ntf(struct wifi_station *sta, unsigned char opt_mode)
{
    unsigned char chan_bw = 0;
    sta->sta_opt_mode =  opt_mode;
    chan_bw = opt_mode & 0x3; // bit 0,bit 1: Channel Width
    if (0 == chan_bw) {
        sta->sta_chbw = WIFINET_BWC_WIDTH20;

    } else if (1 == chan_bw) {
        sta->sta_chbw = WIFINET_BWC_WIDTH40;

    } else if (2 == chan_bw) {
        sta->sta_chbw = WIFINET_BWC_WIDTH80;
    }
}

static int wifi_mac_parse_timeout_ie(struct wifi_station *sta, unsigned char *timeout) {
    unsigned char ie;
    unsigned char ie_len;
    unsigned char timeout_type;
    int timeout_val;

    ie = timeout[0];
    if (ie != WIFINET_ELEMID_TIMEOUT) {
        return 0;
    }

    ie_len = timeout[1];
    if (ie_len != 5) {
        return 0;
    }

    timeout_type = timeout[2];
    if (timeout_type != 3) {
        return 0;
    }

    timeout_val = htobe32(*(unsigned int *)(&timeout[3]));
    return timeout_val;
}

void wifi_mac_vht_ie_parse_all(struct wifi_station *sta, struct wifi_mac_scan_param* sp)
{
    if ((sta ==  NULL) || (sp == NULL)) {
        return;
    }

    //pr_debug("parse vht cap ++\n");
    if (sp->vht_cap) {
        wifi_mac_parse_vht_cap(sta, sp->vht_cap);
    }

    if (sp->vht_opt) {
        wifi_mac_parse_vht_opt(sta, sp->vht_opt);
    }

    if (sp->vht_tx_pwr) {
        wifi_mac_parse_vht_txpw(sta, sp->vht_tx_pwr);
    }

    if (sp->vht_ch_sw) {
        wifi_mac_parse_vht_ch_sw_wrp(sta, sp->vht_ch_sw);
    }

    if (sp->vht_ext_bss_ld) {
        wifi_mac_parse_vht_ext_bss_ld(sta, sp->vht_ext_bss_ld);
    }

    if (sp->vht_quiet_ch) {
        wifi_mac_parse_vht_quiet_ch(sta, sp->vht_quiet_ch);
    }

    if (sp->vht_opt_md_ntf) {
        wifi_mac_parse_vht_op_md_ntf(sta, sp->vht_opt_md_ntf[2]);
    }
}

static void
wifi_mac_deliver_l2uf(struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct net_device *dev = wnet_vif->vm_ndev;
    struct sk_buff *skb;
    struct l2_update_frame *l2uf;
    struct ether_header *eh;

    skb = os_skb_alloc(sizeof(*l2uf));
    if (!skb)
    {
        ERROR_DEBUG_OUT("wifi_mac_deliver_l2uf: no buf available\n");
        return;
    }
    os_skb_put(skb, sizeof(*l2uf));
    l2uf = (struct l2_update_frame *)(os_skb_data(skb));
    eh = &l2uf->eh;
    WIFINET_ADDR_COPY(eh->ether_dhost, dev->broadcast);
    WIFINET_ADDR_COPY(eh->ether_shost, sta->sta_macaddr);
    eh->ether_type = __constant_htons(os_skb_get_pktlen(skb) - sizeof(*eh));

    l2uf->dsap = 0;
    l2uf->ssap = 0;
    l2uf->control = 0xf5;
    l2uf->xid[0] = 0x81;
    l2uf->xid[1] = 0x80;
    l2uf->xid[2] = 0x00;

    skb->dev = dev;
    skb_reset_mac_header(skb);

    wifi_mac_deliver_data(sta, skb);
    return;
}

static void wifi_mac_pkt_parse_element(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb, struct wifi_mac_scan_param *scan, unsigned char *invalid, unsigned char *drop)
{
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct wifi_mac_p2p *p2p = wnet_vif->vm_p2p;
    struct wifi_frame *wh;
    struct drv_private *drv_priv;
    unsigned char *frm, *efrm;
    int has_erp = 0;
    int has_ht = 0;
    int index = 0;
    unsigned short subtype = 0;
    unsigned char sa[MAX_MAC_BUF_LEN] = {0};

    drv_priv = drv_get_drv_priv();
    wh = (struct wifi_frame *)os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);
    scan->frame_len = os_skb_get_pktlen(skb);
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;
    snprintf(sa, MAX_MAC_BUF_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
         wh->i_addr2[0], wh->i_addr2[1], wh->i_addr2[2], wh->i_addr2[3], wh->i_addr2[4], wh->i_addr2[5]);

    *invalid = 1;
    if ((subtype == WIFINET_FC0_SUBTYPE_BEACON) || (subtype == WIFINET_FC0_SUBTYPE_PROBE_RESP)) {
        WIFINET_VERIFY_LENGTH(efrm - frm, 12);
        scan->tstamp = frm;
        frm += 8;
        scan->bintval = le16toh(*(unsigned short *)frm);
        frm += 2;
        scan->capinfo = le16toh(*(unsigned short *)frm);
        frm += 2;

    } else if (subtype == WIFINET_FC0_SUBTYPE_ASSOC_RESP) {
        WIFINET_VERIFY_LENGTH(efrm - frm, 6);
        sta = wnet_vif->vm_mainsta;
        scan->capinfo = le16toh(*(unsigned short *)frm);
        frm += 2;
        scan->status_code = le16toh(*(unsigned short *)frm);
        frm += 2;
        scan->assoc_id = le16toh(*(unsigned short *)frm);
        frm += 2;
    }

    while (((frm+1) < efrm ) && (frm + frm[1] + 1) < efrm) {
        switch (*frm) {
            case WIFINET_ELEMID_SSID:
                scan->ssid = frm;
                break;

            case WIFINET_ELEMID_RATES:
                scan->rates = frm;
                break;

            case WIFINET_ELEMID_COUNTRY:
                scan->country = frm;
                break;

            case WIFINET_ELEMID_FHPARMS:
                if ((frm + 6) >= efrm) {
                    return;
                }
                break;

            case WIFINET_ELEMID_DSPARMS:
                if ((frm + 2) >= efrm) {
                    return;
                }

                scan->chan = frm[2];
                if (p2p->peer_listen_channel != 0) {
                    if (!memcmp(sa, p2p->peer_dev_addr, MAX_MAC_BUF_LEN) && (p2p->peer_listen_channel != scan->chan)) {
                        *drop = 1;
                    }
                }
                break;

            case WIFINET_ELEMID_TIM:
                scan->tim = frm;
                scan->timoff = frm - os_skb_data(skb);
                break;

            case WIFINET_ELEMID_IBSSPARMS:
                break;

            case WIFINET_ELEMID_XRATES:
                scan->xrates = frm;
                break;

            case WIFINET_ELEMID_ERP:
                if ((frm + 2) >= efrm || frm[1] != 1) {
                    return;
                }

                if (frm[1] != 1) {
                    wnet_vif->vif_sts.sts_rx_elem_too_long++;
                    return;
                }
                has_erp = 1;
                scan->erp = frm[2];
                break;

            case WIFINET_ELEMID_RSN:
                scan->rsn = frm;
                if ((wnet_vif->vm_opmode == WIFINET_M_STA) && pwdev_priv->connect_request) {
                    sta->sta_encrypt_flag = 1;
                }
                break;

            case WIFINET_ELEMID_VENDOR:
                if (iswpaoui(frm)) {
                    scan->wpa = frm;

                } else if (iswmeparam(frm) || iswmeinfo(frm)) {
                    scan->wme = frm;

                } else if (iswpsoui(frm)) {
                    scan->wps = frm;
                }
#ifdef CONFIG_P2P
                else if (isp2poui(frm)) {
                    if (index < MAX_P2PIE_NUM) {
                        scan->p2p[index++] = frm;
                    }
                    scan->p2p_noa = vm_p2p_get_p2pie_attrib_with_id(frm, P2P_ATTR_NOTICE_OF_ABSENCE);
                }
#ifdef CONFIG_WFD
                else if (iswfdoui(frm)) {
                    scan->wfd = frm;
                }
#endif /* CONFIG_WFD */
#endif /* CONFIG_P2P */
                break;

            case WIFINET_ELEMID_PWRCNSTR:
                scan->tpc = frm;
                break;

            case WIFINET_ELEMID_CHANSWITCHANN:
                scan->doth = frm;
                break;

            case WIFINET_ELEMID_QUIET:
                scan->quiet = frm;
                break;

            case WIFINET_ELEMID_HTCAP:
                if (drv_priv->drv_config.cfg_mac_mode >= CFG_11N)
                    scan->htcap =frm;

                break;

            case WIFINET_ELEMID_HTINFO:
                if (drv_priv->drv_config.cfg_mac_mode >= CFG_11N) {
                    scan->htinfo =frm;
                    scan->chan = frm[2];
                    if (p2p->peer_listen_channel != 0) {
                        if (!memcmp(sa, p2p->peer_dev_addr, MAX_MAC_BUF_LEN) && (p2p->peer_listen_channel != scan->chan)) {
                            *drop = 1;
                        }
                    }
                }
                break;

#ifdef CONFIG_WAPI
            case WIFINET_ELEMID_WAI:
                scan->wai = frm;
                if (wnet_vif->vm_opmode == WIFINET_M_STA && pwdev_priv->connect_request) {
                    sta->sta_encrypt_flag = 1;
                }
                break;
#endif //#ifdef CONFIG_WAPI

            case WIFINET_ELEMID_EXTCAP:
                    scan->ext_cap = frm;

                break;

            case WIFINET_ELEMID_VHTCAP:
                if (drv_priv->drv_config.cfg_mac_mode >= CFG_11AC)
                    scan->vht_cap = frm;

                break;

            case WIFINET_ELEMID_VHTOP:
                if (drv_priv->drv_config.cfg_mac_mode >= CFG_11AC)
                    scan->vht_opt = frm;

                break;

            case WIFINET_ELEMID_VHT_TX_PWR_ENVLP:
                scan->vht_tx_pwr = frm;
                break;

            case WIFINET_ELEMID_CHAN_SWITCH_WRAP:
                scan->vht_ch_sw = frm;
                break;

            case WIFINET_ELEMID_EXT_BSS_LOAD:
                scan->vht_ext_bss_ld = frm;
                break;

            case WIFINET_ELEMID_QUIET_CHANNEL:
                scan->vht_quiet_ch =  frm;
                break;

            case WIFINET_ELEMID_OP_MODE_NOTIFY:
                scan->vht_opt_md_ntf = frm;
                break;

            case WIFINET_ELEMID_RSNX:
                scan->rsnxe = frm;
                break;
            case WIFINET_ELEMID_BSS_LOAD:
                scan->bss_load = frm;
                break;

            default:
                DPRINTF(AML_DEBUG_ELEMID, "unhandled id %u, len %u", *frm, frm[1]);
                break;
        }

        if (scan->htinfo || scan->htcap)
            has_ht = 1;

        frm += frm[1] + 2;
    }

    if (frm > efrm) {
        wnet_vif->vif_sts.sts_rx_elem_err++;
        DPRINTF(AML_DEBUG_SCAN, "%s ignore\n",__func__);
        return;
    }

    *invalid = 0;
    return;
}

void wifi_mac_recv_beacon(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb, int rssi, unsigned int channel)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_frame *wh = (struct wifi_frame *)os_skb_data(skb);
    unsigned char invalid = 0;
    unsigned char drop = 0;
    unsigned char qosinfo;
    struct wifi_mac_scan_param scan = {0};
    int has_erp = 0;
    int has_ht  = 0;
    int i = 0;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++) {
        if (wifimac->wm_channels[i].chan_pri_num == channel)
            wifimac->wm_channels[i].chan_flags |= WIFINET_CHAN_AWARE;
    }
    WIFI_CHANNEL_UNLOCK(wifimac);

    if (!((wifimac->wm_flags & WIFINET_F_SCAN) ||
        ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) &&
        (wifimac->wm_protmode != WIFINET_PROT_NONE)) ||
        ((wnet_vif->vm_opmode == WIFINET_M_STA) && (sta->sta_associd)) ||
        (wnet_vif->vm_opmode == WIFINET_M_IBSS))) {
        DPRINTF(AML_DEBUG_RECV, "%s %d ignore\n",__func__,__LINE__);
        wnet_vif->vif_sts.sts_mng_discard++;
        return;
    }

    WIFINET_NODE_STAT(sta, rx_beacons);
    wifi_mac_pkt_parse_element(wnet_vif, sta, skb, &scan, &invalid, &drop);
    if (invalid) {
        wnet_vif->vif_sts.sts_rx_elem_err++;
        DPRINTF(AML_DEBUG_SCAN, "%s %d ignore, invalid=%d\n",__func__,__LINE__, invalid);
        return;
    }

    if(scan.wai != NULL) {
        wnet_vif->vif_sts.sts_rx_elem_err++;
        return;
    }

    if (scan.chan == 0) {
        if (scan.vht_opt != NULL) {
            //no DSPARMS IE and HTINFO IE, but with VHTINFO IE
            scan.chan = scan.vht_opt[3];

        } else if (wifimac->wm_curchan != NULL) {
            //no channel info in beacon or probe rsp, use current channel.(wifi51_5G,tkip or wep)
            scan.chan = channel;
//            pr_warn("no channel info, use mac_chan:%d cur_chan:%d, rx channel:%d\n", wifimac->wm_curchan->chan_pri_num, scan.chan, channel);
        }
    }

    if (scan.chan > WIFINET_CHAN_MAX_NUM) {
        WIFINET_DPRINTF(AML_DEBUG_ANY,"invalid channel %u", scan.chan);
        wnet_vif->vif_sts.sts_rx_elem_err++;
        return;
    }

    WIFINET_VERIFY_ELEMENT(scan.rates, WIFINET_RATE_MAXSIZE);
    WIFINET_VERIFY_ELEMENT(scan.xrates, WIFINET_RATE_MAXSIZE);
    WIFINET_VERIFY_ELEMENT(scan.ssid, WIFINET_NWID_LEN);

    if (wifimac->wm_flags & WIFINET_F_SCAN) {
        struct scaninfo_entry *se = NULL;
        struct scaninfo_entry *si_next = NULL;
        struct scaninfo_table *st = wifimac->wm_scan->ScanTablePriv;
        int hash = STA_HASH(wh->i_addr2);

        /* check if the channel is an candidate channel*/
        if (scan.ssid) {
            wifi_mac_update_roaming_candidate_chan(wnet_vif, &scan, rssi);
        }

        WIFI_SCAN_SE_LIST_LOCK(st);
        list_for_each_entry_safe(se, si_next, &st->st_hash[hash], se_hash) {
            if (WIFINET_ADDR_EQ(se->scaninfo.SI_macaddr, wh->i_addr2)) {
                if (vm_p2p_is_state(wnet_vif->vm_p2p, NET80211_P2P_STATE_SCAN)
                    && (se->scaninfo.SI_frame_len < scan.frame_len)) {
                    list_del_init(&se->se_list);
                    list_del_init(&se->se_hash);
                    FREE(se,"sta_add.se");
                } else {
                    WIFI_SCAN_SE_LIST_UNLOCK(st);
                    return;
                }
            }
        }
        WIFI_SCAN_SE_LIST_UNLOCK(st);

        wifi_mac_scan_rx(wnet_vif, &scan, wh, rssi, NULL);
    }

    //pr_debug("%s %02x:%02x:%02x:%02x:%02x:%02x, assoc_id:%d\n", __func__, wh->i_addr2[0], wh->i_addr2[1],
    //    wh->i_addr2[2], wh->i_addr2[3], wh->i_addr2[4], wh->i_addr2[5], sta->sta_associd);

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (sta->sta_associd != 0)
        && WIFINET_ADDR_EQ(wh->i_addr2, sta->sta_bssid)) {
        memcpy(sta->sta_tstamp.data, scan.tstamp, sizeof(sta->sta_tstamp));
        sta->bcn_stamp = jiffies;

        if (sta->sta_erp != scan.erp) {
            WIFINET_DPRINTF_STA( AML_DEBUG_CONNECT, sta, "erp change: was 0x%x, now 0x%x",
                sta->sta_erp, scan.erp);

            if (scan.erp & WIFINET_ERP_USE_PROTECTION)
                wifimac->wm_flags |= WIFINET_F_USEPROT;
            else
                wifimac->wm_flags &= ~WIFINET_F_USEPROT;
            wifi_mac_update_protmode(wifimac);
            sta->sta_erp = scan.erp;
        }

        if ((sta->sta_capinfo ^ scan.capinfo) & WIFINET_CAPINFO_SHORT_SLOTTIME) {
            WIFINET_DPRINTF_STA( AML_DEBUG_CONNECT, sta, "capabilities change: was 0x%x, now 0x%x",
                sta->sta_capinfo, scan.capinfo);
            if (wnet_vif->vm_curchan != WIFINET_CHAN_ERR)
                wifi_mac_set_shortslottime(wifimac, WIFINET_IS_CHAN_5GHZ(wnet_vif->vm_curchan) ||
                    (sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME));
            else
                wifi_mac_set_shortslottime(wifimac, sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME);

            sta->sta_capinfo = scan.capinfo;
        }

        if ((scan.wme != NULL) && (sta->sta_flags & WIFINET_NODE_QOS)) {
            int _retval;

            if ((wnet_vif->vm_state == WIFINET_S_CONNECTED)
                && ((_retval = wifi_mac_parse_wmeparams(wnet_vif, scan.wme, wh, &qosinfo)) > 0)) {
                if (qosinfo & WME_CAPINFO_UAPSD_EN)
                    sta->sta_flags |= WIFINET_NODE_UAPSD;
                else
                    sta->sta_flags &= ~WIFINET_NODE_UAPSD;
                wifi_mac_wme_updateparams(wnet_vif);
            }

        } else {
            sta->sta_flags &= ~WIFINET_NODE_UAPSD;
        }

        if (scan.quiet)
            wifi_mac_set_quiet(sta, scan.quiet);

        if (sta->sta_flags & WIFINET_NODE_HT) {
            if (scan.htcap) {
                wifi_mac_parse_htcap(sta, scan.htcap);
            }

            if (scan.htinfo) {
                wifi_mac_parse_htinfo(sta, scan.htinfo);
            }
        }

        if (sta->sta_flags & WIFINET_NODE_VHT) {
            wifi_mac_vht_ie_parse_all(sta, &scan);
        }

        if (scan.doth != NULL) {
            wifi_mac_parse_dothparams(wnet_vif, scan.doth, &scan, sta);
        }
        wifi_mac_pwrsave_sleep_wait_cancel(wnet_vif);

        if ((scan.tim != NULL) && (wifi_mac_pwrsave_is_sta_sleeping(wnet_vif) == 0)
            && ((wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_PS4QUIET) == 0)) {
            int flag_tim_dtim_proc = 0;

            struct wifi_mac_tim_ie *tim = (struct wifi_mac_tim_ie *) scan.tim;
            int aid = WIFINET_AID(sta->sta_associd);
            int ix = aid / NBBY;
            int min = tim->tim_bitctl &~ 1;
            int max = tim->tim_len + min - 4;
            WIFINET_PWRSAVE_LOCK(wnet_vif);
            wnet_vif->vm_pwrsave.ips_sleep_wait_reason = SLEEP_AFTER_BEACON;
            WIFINET_PWRSAVE_UNLOCK(wnet_vif);

            if (tim->tim_bitctl & 1) {
                wifi_mac_pwrsave_proc_dtim(wnet_vif);
                flag_tim_dtim_proc = 1;
            }

            /*FIXME: when we need receive broadcast frames, if can we ingore the processing of TIM ?  */
            if ((min <= ix) && (ix <= max) && isset(tim->tim_bitmap - min, aid)) {
                wifi_mac_pwrsave_proc_tim(wnet_vif);
                flag_tim_dtim_proc = 2;
            }

            /*
            'flag_tim_dtim_proc != 0' means that sta had sent null+ps=0 to ap.
            And restore full sleeping by sending null+ps=1 in 'isp_timer_presleep'.
            */
            if ((!flag_tim_dtim_proc) && (wifi_mac_pwrsave_is_wnet_vif_networksleep(wnet_vif) == 0)) {
                wnet_vif->vm_pwrsave.ips_sleep_wait_reason = SLEEP_AFTER_BEACON;
                //wifi_mac_pwrsave_restore_sleep(wnet_vif);
                wifi_mac_add_work_task(wifimac,wifi_mac_pwrsave_sleep_wait_ex,NULL,
                    (SYS_TYPE)wnet_vif,0,0,(SYS_TYPE)wnet_vif,(SYS_TYPE)wnet_vif->wnet_vif_replaycounter);
            }
        }

        if ((wnet_vif->vm_state == WIFINET_S_CONNECTED) && (sta == wnet_vif->vm_mainsta)) {
            if (scan.tim != NULL) {
                struct wifi_mac_tim_ie *tim = (struct wifi_mac_tim_ie *) scan.tim;
                wnet_vif->vm_dtim_period = tim->tim_period;
            }
        }

#ifdef CONFIG_P2P
        if ((wnet_vif->vm_p2p_support == 1) && (wnet_vif->vm_state == WIFINET_S_CONNECTED)
            && vm_p2p_chk_p2p_role(wnet_vif->vm_p2p, NET80211_P2P_ROLE_CLIENT)) {
            vm_p2p_client_parse_noa_ie(wnet_vif->vm_p2p, scan.p2p_noa);
        }
#endif
        return;
    }

    if (scan.htcap) {
        has_ht = 1;
    }

    if (scan.erp) {
        has_erp = 1;
    }

    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
        if (WIFINET_INCLUDE_11G(wnet_vif->vm_mac_mode)) {
            if (!has_erp || (has_erp && (scan.erp & WIFINET_ERP_NON_ERP_PRESENT))) {
                if (!(wifimac->wm_flags & WIFINET_F_USEPROT)) {
                    wifimac->wm_flags |= WIFINET_F_USEPROT;
                    wifimac->wm_flags_ext |= WIFINET_FEXT_ERPUPDATE;
                    wifi_mac_update_protmode(wifimac);
                    wifi_mac_set_shortslottime(wifimac, 0);
                }
                wifimac->wm_time_nonerp_present = jiffies;
            }
        }

        if (!has_ht && WIFINET_INCLUDE_11N(wnet_vif->vm_mac_mode)) {
            if (!(wifimac->wm_flags_ext & WIFINET_F_NONHT_AP)) {
                wifimac->wm_flags_ext |= WIFINET_F_NONHT_AP;
                wifi_mac_ht_prot(wifimac, sta, WIFINET_BEACON_UPDATE);
            }
            wifimac->wm_time_noht_present = jiffies;
        }
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_IBSS) && (scan.capinfo & WIFINET_CAPINFO_IBSS)) {
        if (!WIFINET_ADDR_EQ(wh->i_addr2, sta->sta_macaddr)) {
            sta = wifi_mac_add_neighbor(wnet_vif, wh, &scan);

        } else {
            memcpy(sta->sta_tstamp.data, scan.tstamp, sizeof(sta->sta_tstamp));
        }

        if (sta == NULL)
            return;
        sta->sta_rstamp = jiffies;
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_IBSS)
        && (wnet_vif->vm_state == WIFINET_S_CONNECTED) && WIFINET_ADDR_EQ(wh->i_addr3, wnet_vif->vm_mainsta->sta_bssid)) {
        wifimac->wm_syncbeacon = 0;
    }
}

void wifi_mac_recv_probersp(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb, int rssi, unsigned int channel)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned char invalid = 0;
    unsigned char drop = 0;
    struct wifi_mac_scan_param scan = {0};
    struct scaninfo_entry *se = NULL;
    struct scaninfo_entry *oldse = NULL;
    struct scaninfo_entry *si_next = NULL;
    struct scaninfo_table *st = wifimac->wm_scan->ScanTablePriv;
    struct wifi_frame *wh = (struct wifi_frame *)os_skb_data(skb);
    int hash = STA_HASH(wh->i_addr2);

    if (!(wifimac->wm_flags & WIFINET_F_SCAN)) {
        wnet_vif->vif_sts.sts_mng_discard++;
        return;
    }

    wifi_mac_pkt_parse_element(wnet_vif, sta, skb, &scan, &invalid, &drop);
    if (invalid) {
        wnet_vif->vif_sts.sts_rx_elem_err++;
        DPRINTF(AML_DEBUG_SCAN, "%s %d ignore, invalid=%d\n",__func__,__LINE__, invalid);
        return;
    }

    if(scan.wai != NULL) {
        wnet_vif->vif_sts.sts_rx_elem_err++;
        return;
    }

    if (scan.chan == 0) {
        if (scan.vht_opt != NULL) {
            //no DSPARMS IE and HTINFO IE, but with VHTINFO IE
            scan.chan = scan.vht_opt[3];
        } else if (wifimac->wm_curchan != NULL) {
            //no channel info in beacon or probe rsp, use current channel.(wifi51_5G,tkip or wep)
            scan.chan = channel;
//            pr_debug("no channel info, use mac_chan:%d cur_chan:%d, rx channel:%d\n", wifimac->wm_curchan->chan_pri_num, scan.chan, channel);
        }
    }

    if (scan.chan > WIFINET_CHAN_MAX_NUM) {
        WIFINET_DPRINTF(AML_DEBUG_ANY,"invalid channel %u", scan.chan);
        wnet_vif->vif_sts.sts_rx_elem_err++;
        return;
    }

    WIFINET_VERIFY_ELEMENT(scan.rates, WIFINET_RATE_MAXSIZE);
    WIFINET_VERIFY_ELEMENT(scan.xrates, WIFINET_RATE_MAXSIZE);
    WIFINET_VERIFY_ELEMENT(scan.ssid, WIFINET_NWID_LEN);

    if (drop) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d ignore, drop=%d\n",__func__,__LINE__, drop);
        return;
    }

    /* check if the channel is an candidate channel*/
    if (scan.ssid) {
        wifi_mac_update_roaming_candidate_chan(wnet_vif, &scan, rssi);
    }

    WIFINET_NODE_STAT(sta, rx_proberesp);
    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry_safe(se, si_next, &st->st_hash[hash], se_hash) {
        if (WIFINET_ADDR_EQ(se->scaninfo.SI_macaddr, wh->i_addr2)) {
            if (scan.ssid && (se->scaninfo.SI_ssid[1] == scan.ssid[1]) && !memcmp(se->scaninfo.SI_ssid, scan.ssid, se->scaninfo.SI_ssid[1] + 2)) {
                if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
                    WIFI_SCAN_SE_LIST_UNLOCK(st);
                    return;
                }
                oldse = se;
            }
        }
    }

    /* Need to update existing entry. List is locked, and as
     * 'oldse' is not NULL, 'wifi_mac_scan_rx()' recognizes
     * it as updating entry and won't take lock on list.
     */
    if (oldse)
        wifi_mac_scan_rx(wnet_vif, &scan, wh, rssi, oldse);

    WIFI_SCAN_SE_LIST_UNLOCK(st);

    /* This will add new entry. 'oldse' is NULL, we release
     * lock on list, 'wifi_mac_scan_rx()' will allocate new
     * entry, will lock this list to insert new entry.
     */
    if (!oldse)
        wifi_mac_scan_rx(wnet_vif, &scan, wh, rssi, oldse);
}


void wifi_mac_recv_probe_req(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb, int rssi, unsigned int channel)
{
    struct wifi_frame *wh;
    unsigned char *frm, *efrm;
    unsigned char invalid = 0;
    unsigned char *ssid = NULL, *rates = NULL, *xrates = NULL;
    unsigned char *wps = NULL;
#ifdef CONFIG_P2P
    unsigned char *p2p[MAX_P2PIE_NUM] = {NULL};
    unsigned char *wfd = NULL;
    static int num = 0;
    int index = 0;
#endif//CONFIG_P2P
    unsigned short subtype = 0;
    int allocbs = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *temp_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];

    wh = (struct wifi_frame *)os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;
    DPRINTF(AML_DEBUG_INFO, "%s %d ++\n",__func__,__LINE__);
    {

        if (wnet_vif->vm_opmode == WIFINET_M_STA ||
            wnet_vif->vm_state != WIFINET_S_CONNECTED)
        {
#ifdef CONFIG_P2P
            //not ignore at listen for p2p
            if (!vm_p2p_is_state(wnet_vif->vm_p2p, NET80211_P2P_STATE_LISTEN) ||
                (wnet_vif->vm_p2p_support == false))
#endif//CONFIG_P2P
            {
                wnet_vif->vif_sts.sts_mng_discard++;
#ifdef CONFIG_P2P
                if (wnet_vif->vm_p2p_support==true)
                {
                    num++;
                    if(num % 100 == 0)
                        DPRINTF(AML_DEBUG_SCAN, "%s %d ignore, mac=%s, not p2p listen\n",
                                __func__,__LINE__, ether_sprintf(wh->i_addr2));
                }
#endif//CONFIG_P2P
                return;
            }
        }

        if (temp_wnet_vif != NULL && temp_wnet_vif->vm_curchan != NULL && temp_wnet_vif->vm_curchan->chan_pri_num != channel) {
            DPRINTF(AML_DEBUG_INFO, "%s %d ignore, probe req %d, %d\n",__func__,__LINE__, temp_wnet_vif->vm_curchan->chan_pri_num, channel);
        }
        if (WIFINET_IS_MULTICAST(wh->i_addr2))
        {
            wnet_vif->vif_sts.sts_mng_discard++;
            DPRINTF(AML_DEBUG_SCAN, "%s %d ignore, ta=multicast\n",__func__,__LINE__);
            return;
        }

        ssid = rates = xrates = NULL;
        wps = NULL;

        while (((frm+1) < efrm ) && (frm + frm[1] + 1) < efrm )
        {
            switch (*frm)
            {
                case WIFINET_ELEMID_SSID:
                    ssid = frm;
                    break;
                case WIFINET_ELEMID_RATES:
                    rates = frm;
                    break;
                case WIFINET_ELEMID_XRATES:
                    xrates = frm;
                    break;
                case WIFINET_ELEMID_VENDOR:
                    if (iswpsoui(frm))
                        wps = frm;
#ifdef CONFIG_P2P
                    else if (isp2poui(frm))
                    {
                        if (index < MAX_P2PIE_NUM)
                            p2p[index++] = frm;
                    }
#ifdef CONFIG_WFD
                    else if (iswfdoui(frm))
                    {
                        wfd = frm;
                    }
#endif

#endif /* CONFIG_P2P */
                    break;

            }
            frm += frm[1] + 2;
        }
        if (frm > efrm )
        {
            WIFINET_DPRINTF( AML_DEBUG_ELEMID, "%s status %d","reached the end of frame",invalid);
            wnet_vif->vif_sts.sts_rx_elem_err++;

            return;
        }

#ifdef CONFIG_P2P
        //for p2p and listen, for client or dev, only response to P2P_WILDCARD_SSID
        if (wnet_vif->vm_p2p_support && vm_p2p_is_state(wnet_vif->vm_p2p, NET80211_P2P_STATE_LISTEN)
            && ((wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_CLIENT) ||(wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_DEVICE)))
        {
            if ((!ssid) || (ssid[1] != P2P_WILDCARD_SSID_LEN)
                || memcmp(&ssid[2],P2P_WILDCARD_SSID,P2P_WILDCARD_SSID_LEN)
                || (rates == NULL) || is_only_11b_rates(rates))
            {
                AML_PRINT(AML_DBG_MODULES_P2P,"drop, mac=%s, not p2p probereq\n ",
                         ether_sprintf(wh->i_addr2));
                wnet_vif->vif_sts.sts_rx_ssid_mismatch++;
                return;
            }
        }
        //for p2p and go, only response to ssid == go
        else if (wnet_vif->vm_p2p_support && (wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_GO))
        {
            if ((ssid != NULL) &&  ((ssid[1] != P2P_WILDCARD_SSID_LEN)
                || memcmp(&ssid[2],P2P_WILDCARD_SSID,P2P_WILDCARD_SSID_LEN)))
            {
            //   if (memcmp(&ssid[2], wnet_vif->vm_mainsta->sta_essid, wnet_vif->vm_mainsta->sta_esslen) == 0)
                   wnet_vif->vm_p2p->p2p_flag |= P2P_REQUEST_SSID;

                WIFINET_VERIFY_SSID(wnet_vif->vm_mainsta, ssid);
            }
        }
        else
#endif
        {
            //CONFIG_P2P
            WIFINET_VERIFY_SSID(wnet_vif->vm_mainsta, ssid);
        }
        WIFINET_VERIFY_ELEMENT(rates, WIFINET_RATE_MAXSIZE);
        WIFINET_VERIFY_ELEMENT(ssid, WIFINET_NWID_LEN);
        if ((wnet_vif->vm_flags & WIFINET_F_HIDESSID) && ssid && ssid[1] == 0)
        {
            WIFINET_DPRINTF( AML_DEBUG_RECV,"%s", "no ssid with ssid suppression enabled");
            wnet_vif->vif_sts.sts_rx_ssid_mismatch++; /*XXX*/
            return;
        }

        WIFINET_VERIFY_ELEMENT(xrates, WIFINET_RATE_MAXSIZE);

        if (sta == wnet_vif->vm_mainsta)
        {
            if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
            {
                sta = wifi_mac_fake_adhos_sta(wnet_vif, wh->i_addr2);
            }

            if (sta == NULL)
            {
                return;
            }
            allocbs = 1;
        }
        else
            allocbs = 0;

        WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2, "%s", "recv probe req");
#ifdef CONFIG_P2P
        for (index = 0; index < MAX_P2PIE_NUM; index++) {
            if (p2p[index] != NULL) {
                AML_PRINT(AML_DBG_MODULES_P2P,"length=%d get p2p-ie in probe req\n", p2p[index][1]);
                wifi_mac_saveie(&sta->sta_p2p_ie[index], p2p[index], "sta->sta_p2p_ie");

            } else if (sta->sta_p2p_ie[index] != NULL) {
                FREE(sta->sta_p2p_ie[index], "sta->sta_p2p_ie");
                sta->sta_p2p_ie[index] = NULL;
            }
        }
#endif //CONFIG_P2P
#ifdef CONFIG_WFD
        if (wfd != NULL)
        {
            DPRINTF(AML_DEBUG_INFO, "%s  length=%d get wdf-ie in probe req", __func__,wfd[1]);

            wifi_mac_saveie(&sta->sta_wfd_ie, wfd, "sta->sta_wfd_ie");
        }
        else if (sta->sta_wfd_ie != NULL)
        {
            FREE(sta->sta_wfd_ie,"sta->sta_wfd_ie");
            sta->sta_wfd_ie = NULL;
        }
#endif //CONFIG_WFD

        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_PROBE_RESP, (void *)wh->i_addr2);

        if (wps != NULL)
        {
            DPRINTF(AML_DEBUG_INFO, "%s  length=%d get wps -ie in probe req", __func__,wps[1]);
            wifi_mac_saveie(&sta->sta_wps_ie, wps, "sta->sta_wps_ie");
        }
        else if (sta->sta_wps_ie != NULL)
        {
            FREE(sta->sta_wps_ie,"sta->sta_wps_ie");
            sta->sta_wps_ie = NULL;
        }
    }
}

static void wifi_mac_status_code_handle(struct wifi_station *sta, unsigned char subtype, struct wifi_mac_scan_param *scan)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    unsigned short timeout = 1000;

    if (scan == NULL) {
        ERROR_DEBUG_OUT("scan NULL\n");
        return;
    }

    if ((WIFINET_STATUS_INVALID_PMKID == scan->status_code)
        && (sta->sta_rsn.rsn_keymgmtset & (1 << RSN_ASE_8021X_SAE))
        && (sta->sta_authmode == WIFINET_AUTH_OPEN)) {
        os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);
        aml_del_pmksa_by_index(wnet_vif, sta->sta_bssid);
        sta->sta_authmode = WIFINET_AUTH_SAE;
        wifi_mac_top_sm(wnet_vif, WIFINET_S_AUTH, WIFINET_STATUS_INVALID_PMKID);
        return;
    }

    if (WIFINET_FC0_SUBTYPE_AUTH == subtype) {
        if (WIFINET_STATUS_ALG == scan->status_code) {
            wnet_vif->vm_mainsta->sta_authmode = WIFINET_AUTH_SHARED;
        }

    } else if (WIFINET_FC0_SUBTYPE_ASSOC_RESP == subtype) {
        os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);
        if (WIFINET_STATUS_REFUSED_TEMPORARILY == scan->status_code) {
            if (scan->timeout_ie != NULL) {
                timeout = wifi_mac_parse_timeout_ie(sta, scan->timeout_ie);
            }

            pr_warn("wait %d to reconnect\n", timeout);
            os_timer_ex_start_period(&wnet_vif->vm_sm_switch, timeout);
            return;
        }

        wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN,0);
    }

    return;
}

static void wifi_mac_recv_auth(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb, unsigned int channel, int rssi)
{
    struct wifi_frame *wh;
    unsigned char *frm, *efrm;
    unsigned long rstamp = jiffies;
    unsigned short subtype = 0;
    unsigned short auth_type, seq, status;
    struct wifi_mac_scan_param scan = {0};
    int arg = 0;

    wh = (struct wifi_frame *) os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    WIFINET_VERIFY_LENGTH(efrm - frm, 6);
    auth_type = le16toh(*(unsigned short *)frm);
    seq = le16toh(*(unsigned short *)(frm + 2));
    status = le16toh(*(unsigned short *)(frm + 4));
    scan.status_code = status;
    WIFINET_DPRINTF_MACADDR( AML_DEBUG_WARNING, wh->i_addr2,
        "recv auth frame with algorithm %d seq %d", auth_type, seq);

    if ((sta->sta_associd || sta->is_disconnecting) && (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)) {
        AML_OUTPUT("sta try connect, but associd:%d not zero\n", sta->sta_associd);
        if (sta->is_disconnecting == 1) {
            return;
        }
        wifi_softap_allsta_stopping(wnet_vif,1);
        wifi_mac_notify_nsta_disconnect(sta, 0);
        return;
    }

    if ((wnet_vif->vm_aclop != NULL) && !wnet_vif->vm_aclop->iac_check(wnet_vif, wh->i_addr2)) {
        WIFINET_DPRINTF( AML_DEBUG_ACL, "auth %s", "disallowed by ACL");
        wnet_vif->vif_sts.sts_rx_acl_mismatch++;
        return;
    }

    if (wnet_vif->vm_flags & WIFINET_F_COUNTERM) {
        WIFINET_DPRINTF(AML_DEBUG_CONNECT | AML_DEBUG_KEY, "auth %s", "TKIP countermeasures enabled");
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            wifi_mac_send_error(sta, wh->i_addr2, WIFINET_FC0_SUBTYPE_AUTH, WIFINET_REASON_MIC_FAILURE);
        }

        return;
    }
    if ((wnet_vif->vm_opmode == WIFINET_M_STA)
        && (wnet_vif->vm_flags & WIFINET_F_PRIVACY)
        && !(wnet_vif->vm_flags & WIFINET_F_WPA)) { //wep
        if (wnet_vif->vm_mainsta->sta_authmode == WIFINET_AUTH_SHARED
            && status == WIFINET_STATUS_ALG) { //auth algo is wrong
            wnet_vif->vm_mainsta->sta_authmode = WIFINET_AUTH_OPEN;
            arg = WIFINET_AUTH_SHARED_REQUEST;
            AML_OUTPUT("auth type mismatch, try OPEN\n");
            wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&arg);
            return;

        } else if (wnet_vif->vm_mainsta->sta_authmode == WIFINET_AUTH_OPEN
            && status != WIFINET_STATUS_SUCCESS) { //maybe the password is wrong, not the auth type
            AML_OUTPUT("auth reject:%d\n", status);
            os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_NO_SLEEP);
            vm_cfg80211_indicate_disconnect(wnet_vif);
            wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
            return;
        }
    }

    //upper layer config wrong auth algorithm, so if get this error, try to switch auth algorithm
    if ((auth_type != WIFINET_AUTH_SAE) && (status != WIFINET_STATUS_SUCCESS)) {
        wifi_mac_status_code_handle(sta, WIFINET_FC0_SUBTYPE_AUTH, &scan);
        return;
    }

    if (auth_type == WIFINET_AUTH_SHARED) {
        wifi_mac_auth_shared(sta, wh, frm + 6, efrm, rssi, rstamp, seq, status);

    } else if (auth_type == WIFINET_AUTH_OPEN) {
        wifi_mac_auth_open(sta, wh, rssi, rstamp, seq, status);

    } else if (auth_type == WIFINET_AUTH_SAE) {
        wifi_mac_auth_sae(sta, skb, seq, wh, status, channel);

    } else {
        WIFINET_DPRINTF( AML_DEBUG_ANY, "auth Challenge Failure. alg %d", auth_type);
        wnet_vif->vif_sts.sts_auth_unsprt++;
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            wifi_mac_send_error(sta, wh->i_addr2, WIFINET_FC0_SUBTYPE_AUTH,
                (seq+1) | (WIFINET_STATUS_CHALLENGE<<16));
        }

        return;
    }
}

void wifi_mac_recv_assoc_req(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb,unsigned int channel,int rssi)
{
    struct wifi_frame *wh;
    unsigned char *frm, *efrm, *htinfo;
    unsigned char *ssid = NULL, *rates = NULL, *xrates = NULL;
    unsigned char *wpa = NULL, *wme = NULL, *htcap = NULL, *wps = NULL;
    unsigned char *rsnie =  NULL, *wai = NULL;
    int rate = 0;
    unsigned char *vhtcap = NULL;
    struct wifi_station *remote_sta = NULL;
#ifdef CONFIG_P2P
    unsigned char *wfd = NULL;
    unsigned char *p2p[MAX_P2PIE_NUM] = {NULL};
    int index = 0;
#endif

    unsigned short subtype = 0;
    int arg = 0;
    unsigned short capinfo, listen_intval;
    unsigned char reason;
    int reassoc, resp;
    struct wifi_mac_ie_vht_opt_md_ntf *vht_opt_md_ntf = NULL;

    pr_debug("<running> %s %d \n",__func__,__LINE__);

    wh = (struct wifi_frame *) os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;
    {
        if ((wnet_vif->vm_opmode != WIFINET_M_HOSTAP) || (wnet_vif->vm_state != WIFINET_S_CONNECTED)) {
            wnet_vif->vif_sts.sts_mng_discard++;
            WIFINET_DPRINTF_STA(AML_DEBUG_CONNECT, sta, "%s: Discarding Assoc/Reassoc Req, Opmode %d State %d",
                __func__, wnet_vif->vm_opmode, wnet_vif->vm_state);
            return;
        }

        if (subtype == WIFINET_FC0_SUBTYPE_REASSOC_REQ)
        {
            reassoc = 1;
            resp = WIFINET_FC0_SUBTYPE_REASSOC_RESP;
            //arg = 2;
            //wifi_mac_send_mgmt(sta, resp, (void *)&arg);
            if (sta != wnet_vif->vm_mainsta) {
                //wifi_mac_sta_disconnect_from_ap(sta);
            }
           // return;
        }
        else
        {
            reassoc = 0;
            resp = WIFINET_FC0_SUBTYPE_ASSOC_RESP;
        }

        remote_sta = wifi_mac_find_tx_sta(wnet_vif, wh->i_addr2);
        if (remote_sta != NULL) {
            remote_sta->sta_flags_ext |= WIFINET_NODE_REASSOC;
        }

        WIFINET_VERIFY_LENGTH(efrm - frm, (reassoc ? 10 : 4));
        if (!WIFINET_ADDR_EQ(wh->i_addr3, wnet_vif->vm_mainsta->sta_bssid))
        {
            WIFINET_DPRINTF( AML_DEBUG_ANY, "%s", "wrong bssid");
            wnet_vif->vif_sts.sts_rx_assoc_err++;
            return;
        }
        capinfo = le16toh(*(unsigned short *)frm);
        frm += 2;
        /*listen interval */
        listen_intval = le16toh(*(unsigned short *)frm);
        frm += 2;
        if (reassoc)
            frm += 6;

        rsnie = NULL;
        /*frm[1] is length field */
        while (((frm+1) < efrm ) && (frm + frm[1] + 1) < efrm )
        {
            switch (*frm)
            {
                case WIFINET_ELEMID_SSID:
                    ssid = frm;
                    break;
                case WIFINET_ELEMID_RATES:
                    rates = frm;
                    break;
                case WIFINET_ELEMID_XRATES:
                    xrates = frm;
                    break;
                case WIFINET_ELEMID_RSN:
                    if (wnet_vif->vm_flags & WIFINET_F_WPA2)
                    {
                        rsnie = frm;
                    }
                    wpa = frm;
                    break;
                case WIFINET_ELEMID_HTCAP:
                    htcap = (unsigned char *)frm;
                    break;

                case WIFINET_ELEMID_TIM_BROADCAST_REQ:
                    pr_debug("ELEMID_TIM_BROADCAST_REQ, frm:%02x:%02x:%02x:%02x\n", frm[0], frm[1], frm[2], frm[3]);
                    break;

                case WIFINET_ELEMID_HTINFO:
                    htinfo = (unsigned char *)frm;
                    break;

                case WIFINET_ELEMID_VHTCAP:
                   vhtcap = (unsigned char *)frm;
                   break;

                case WIFINET_ELEMID_OP_MODE_NOTIFY:
                    vht_opt_md_ntf = (struct wifi_mac_ie_vht_opt_md_ntf *)frm;
                    break;

                case WIFINET_ELEMID_VENDOR:
                    if (iswpaoui(frm))
                    {
                        if (wnet_vif->vm_flags & WIFINET_F_WPA1)
                            wpa = frm;
                    }
                    else if (iswmeinfo(frm))
                        wme = frm;
                    else if (iswpsoui(frm))
                        wps = frm;
#ifdef CONFIG_P2P
                    else if (isp2poui(frm))
                    {
                        if (index < MAX_P2PIE_NUM) {
                            p2p[index++] = frm;
                        }
                    }
#ifdef CONFIG_WFD
                    else if (iswfdoui(frm))
                    {
                        wfd = frm;
                    }
#endif

#endif /* CONFIG_P2P */
#ifdef CONFIG_WAPI
                    else if (iswaioui(frm))
                    {
                        pr_debug("ASSOC_REQ is waioui \n");
                        wai = frm;
                        dump_memory_internal(wai, wai[1]+2);
                    }
#endif //#ifdef CONFIG_WAPI
                    break;


#ifdef CONFIG_WAPI

                case WIFINET_ELEMID_WAI:
                    wai = frm;
                    pr_debug("ASSOC_REQ is WIFINET_ELEMID_WAI \n");
                    dump_memory_internal(wai, wai[1]+2);
                    break;

#endif //#ifdef CONFIG_WAPI
            }
            /*go to next ie, +2 is ie and length field in byte. */
            frm += frm[1] + 2;
        }
        if (frm > efrm )
        {
            WIFINET_DPRINTF( AML_DEBUG_ELEMID,"%s", "reached the end of frame");
            wnet_vif->vif_sts.sts_rx_elem_err++;
            return;
        }

        WIFINET_VERIFY_ELEMENT(rates, WIFINET_RATE_MAXSIZE);
        WIFINET_VERIFY_ELEMENT(xrates, WIFINET_RATE_MAXSIZE);
        WIFINET_VERIFY_ELEMENT(ssid, WIFINET_NWID_LEN);
        WIFINET_VERIFY_SSID(wnet_vif->vm_mainsta, ssid);

        if (sta == wnet_vif->vm_mainsta)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY, wh->i_addr2,
                "deny %s request, sta not authenticated", reassoc ? "reassoc" : "assoc");
            wifi_mac_send_error(sta, wh->i_addr2, WIFINET_FC0_SUBTYPE_DEAUTH, WIFINET_REASON_ASSOC_NOT_AUTHED);
            wnet_vif->vif_sts.sts_rx_assoc_unauth++;
            return;
        }
        //p2p assocreq check
#ifdef CONFIG_P2P
        if (wnet_vif->vm_p2p->p2p_enable &&
            (vm_p2p_rx_assocreq(wnet_vif->vm_p2p,p2p[0],sta)!=P2P_SC_SUCCESS))
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY, wh->i_addr2,
                "deny %s request, sta is not p2p station ", reassoc ? "reassoc" : "assoc");
            return;
        }
#endif //CONFIG_P2P

        if (wpa != NULL) {
            if (wpa[0] != WIFINET_ELEMID_RSN) {
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                reason = wifi_mac_parse_wpa(&sta->sta_rsn, wpa, 0);

            } else {
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                reason = wifi_mac_parse_counterpart_rsn(&sta->sta_rsn, wpa, 1);

                if (wnet_vif->vm_mainsta->sta_flags_ext & WIFINET_NODE_MFP) {
                    if ((sta->sta_rsn.rsn_caps & MFP_MASK) && sta->sta_rsn.rsn_gmcipher) {
                        sta->sta_flags_ext |= WIFINET_NODE_MFP;
                    }
                }

                if ((sta->sta_authmode == WIFINET_AUTH_SAE) && !(sta->sta_rsn.rsn_caps & MFP_MASK)) {
                    pr_warn("sae auth, but no pmf\n");
                    reason = WIFINET_REASON_IE_INVALID;
                }
            }

            if (reason != 0) {
                arg = reason;
                pr_debug("<running> %s %d reason=%d\n",__func__,__LINE__, reason);
                wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&arg);
                wifi_mac_sta_disconnect_from_ap(sta);
                wnet_vif->vif_sts.sts_rx_assoc_wpa_mismatch++;
                return;
            }

            WIFINET_DPRINTF_MACADDR(AML_DEBUG_CONNECT | AML_DEBUG_KEY, wh->i_addr2,
                "%s ie: mc %u/%u uc %u/%u key %u caps 0x%x", wpa[0] != WIFINET_ELEMID_RSN ?  "WPA" : "RSN",
                sta->sta_rsn.rsn_mcastcipher, sta->sta_rsn.rsn_mcastkeylen, sta->sta_rsn.rsn_ucastcipher,
                sta->sta_rsn.rsn_ucastkeylen, sta->sta_rsn.rsn_keymgmtset, sta->sta_rsn.rsn_caps);
        }

        if (sta->sta_challenge != NULL)
        {
            FREE(sta->sta_challenge,"sta->sta_challenge");
            sta->sta_challenge = NULL;
        }

        if ((capinfo & WIFINET_CAPINFO_ESS) == 0)
        {
            arg = WIFINET_STATUS_CAPINFO;
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY, wh->i_addr2,
                "deny %s request, capability mismatch 0x%x", reassoc ? "reassoc" : "assoc", capinfo);
            wifi_mac_send_mgmt(sta, resp, (void *)&arg);
            wifi_mac_sta_disconnect_from_ap(sta);
            wnet_vif->vif_sts.sts_rx_assoc_cap_mismatch++;
            return;
        }

        /*basic rate or 11b,g rate. */
        if (!wifi_mac_setup_rates(sta, rates, xrates,
            WIFINET_F_DOSORT | WIFINET_F_DOXSECT | WIFINET_F_DOBRS  | WIFINET_F_DOFRATE)) {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY, wh->i_addr2,
                "deny %s request, rate set mismatch", reassoc ? "reassoc" : "assoc");
        }

        if ((sta->sta_associd != 0) && (WIFINET_INCLUDE_11G(wnet_vif->vm_mac_mode)))
        {
            if ((sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME)
                != (capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME))
            {
                arg = WIFINET_STATUS_CAPINFO;
                WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY,wh->i_addr2,
                    "deny %s request, short slot time capability mismatch 0x%x", reassoc ? "reassoc": "assoc", capinfo);
                wifi_mac_send_mgmt(sta, resp, (void *)&arg);
                wifi_mac_sta_disconnect_from_ap(sta);
                wnet_vif->vif_sts.sts_rx_assoc_cap_mismatch++;
                return;
            }
        }

        sta->sta_rstamp = jiffies;
        sta->sta_listen_intval = ((listen_intval > DEFAULT_LISTEN_INTERVAL) ? DEFAULT_LISTEN_INTERVAL : listen_intval);
        sta->sta_capinfo = capinfo;

#ifdef CONFIG_WAPI
        if (wai != NULL)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2, "%s length=%d", "get wai-ie in assoc req", wai[1]);
            wifi_mac_saveie(&sta->sta_wai_ie, wai, "sta->sta_wai_ie");
        }
        else if (sta->sta_wai_ie != NULL)
        {
            FREE(sta->sta_wai_ie,"sta->sta_wai_ie");
            sta->sta_wai_ie = NULL;
        }
#endif //#ifdef CONFIG_WAPI

        if (rsnie != NULL)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2, "%s length=%d", "get rsnie-ie in assoc req", rsnie[1]);
            wifi_mac_saveie(&sta->sta_rsn_ie, rsnie, "sta->sta_rsn_ie");
        }
        else if (sta->sta_rsn_ie != NULL)
        {
            FREE(sta->sta_rsn_ie,"sta->sta_rsn_ie");
            sta->sta_rsn_ie = NULL;
        }

        if (wpa != NULL)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2, "%s length=%d", "get wpa-ie in assoc req", rsnie[1]);
            wifi_mac_saveie(&sta->sta_wpa_ie, wpa, "sta->sta_wpa_ie");
        }
        else if (sta->sta_wpa_ie != NULL)
        {
            FREE(sta->sta_wpa_ie,"sta->sta_wpa_ie");
            sta->sta_wpa_ie = NULL;
        }

        if (wme != NULL)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2,
                                     "%s length=%d", "get wme-ie in assoc req", wme[1]);
            wifi_mac_saveie(&sta->sta_wme_ie, wme, "sta->sta_wme_ie");
            if (wifi_mac_parse_wmeie(wme, wh, sta) > 0)
            {
                sta->sta_flags |= WIFINET_NODE_QOS;
            }
        }
        else if (sta->sta_wme_ie != NULL)
        {
            FREE(sta->sta_wme_ie,"sta->sta_wme_ie");
            sta->sta_wme_ie = NULL;
            sta->sta_flags &= ~WIFINET_NODE_QOS;
        }
#ifdef CONFIG_P2P
    for (index = 0; index < MAX_P2PIE_NUM; index++)
    {
        if (p2p[index] != NULL)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2,
                                     "%s length=%d", "get p2p-ie in assoc req", p2p[index][1]);
            wifi_mac_saveie(&sta->sta_p2p_ie[index], p2p[index], "sta->sta_p2p_ie");
        }
        else if (sta->sta_p2p_ie[index] != NULL)
        {
            FREE(sta->sta_p2p_ie[index],"sta->sta_p2p_ie");
            sta->sta_p2p_ie[index] = NULL;
        }
    }
#ifdef CONFIG_WFD
        if (wfd != NULL)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV, wh->i_addr2,
                                     "%s length=%d", "get p2p-ie in assoc req", wfd[1]);
            wifi_mac_saveie(&sta->sta_wfd_ie, wfd, "sta->sta_wfd_ie");
        }
        else if (sta->sta_wfd_ie != NULL)
        {
            FREE(sta->sta_wfd_ie,"sta->sta_wfd_ie");
            sta->sta_wfd_ie = NULL;
        }
#endif //CONFIG_WFD
#endif//#ifdef CONFIG_P2P

        if (wifi_mac_is_ht_enable(sta)) {
            if (htcap != NULL) {
                wifi_mac_parse_htcap(sta, htcap);

            } else {
                if ((wnet_vif->vm_flags_ext2 & WIFINET_FEXT2_PUREBGN) == WIFINET_FEXT2_PUREN)
                {
                    arg = WIFINET_STATUS_CAPINFO;
                    WIFINET_DPRINTF_MACADDR(AML_DEBUG_ANY, wh->i_addr2,
                        "deny %s request, non-11n station (in N-only mode)", reassoc ? "reassoc" : "assoc");
                    wifi_mac_send_mgmt(sta, resp, (void *)&arg);
                    wifi_mac_sta_disconnect_from_ap(sta);
                    wnet_vif->vif_sts.sts_rx_assoc_cap_mismatch++;
                    return;
                }
                pr_warn("%s warning:WIFINET_NODE_HT clear\n", __func__);
                sta->sta_flags &= ~WIFINET_NODE_HT;
                sta->sta_htcap = 0;
            }

            /* ht rate, mcs0 -7 for one spatial.*/
            rate = wifi_mac_setup_ht_rates(sta, htcap, WIFINET_F_DOFRATE | WIFINET_F_DOXSECT | WIFINET_F_DOBRS);
            if (!rate) {
                WIFINET_DPRINTF_MACADDR(AML_DEBUG_ANY, wh->i_addr2,
                    "deny %s request, ht rate set mismatch", reassoc ? "reassoc" : "assoc");
            }
        }

        if (wifi_mac_is_vht_enable(wnet_vif))
        {
            if (vhtcap)
            {
                wifi_mac_parse_vht_cap( sta, vhtcap );
                /*
                * merge vht rate in assoc req with local init,
                * mcs0 -9 for one spatial.
                */
                wifi_mac_setup_vht_rates( sta, vhtcap,
                    WIFINET_F_DOFRATE | WIFINET_F_DOXSECT | WIFINET_F_DOBRS);
            }

            if (vht_opt_md_ntf) {
                wifi_mac_parse_vht_op_md_ntf(sta, vht_opt_md_ntf->opt_mode);
                pr_debug("****** %s--vm_bandwidth = %d, sta_chbw =%d\n", __func__, wnet_vif->vm_bandwidth, sta->sta_chbw);
            }

        }

        if (wps != NULL)
        {
            wifi_mac_saveie(&sta->sta_wps_ie, wps, "sta->sta_wps_ie");
        }
        else if (sta->sta_wps_ie != NULL)
        {
            FREE(sta->sta_wps_ie,"sta->sta_wps_ie");
            sta->sta_wps_ie = NULL;
        }
        vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb),os_skb_get_pktlen(skb));
        wifi_mac_deliver_l2uf(sta);
        /* allocate and register aid for new connection, send assoc resp frame.*/
        if (!sta->sta_associd)
            wifi_mac_sta_connect(sta, resp);
    }
}

void wifi_mac_recv_assoc_rsp(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb, unsigned int channel)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_frame *wh;
    unsigned char invalid = 0;
    unsigned char drop = 0;
    struct wifi_mac_scan_param scan = {0};
    unsigned char qosinfo;
    struct drv_private *drv_priv = drv_get_drv_priv();

    wh = (struct wifi_frame *) os_skb_data(skb);

    if ((wnet_vif->vm_opmode != WIFINET_M_STA) || (wnet_vif->vm_state != WIFINET_S_ASSOC)) {
        wnet_vif->vif_sts.sts_mng_discard++;
        return;
    }

    wifi_mac_pkt_parse_element(wnet_vif, sta, skb, &scan, &invalid, &drop);
    AML_OUTPUT("%s assoc status code:%d\n", __func__, scan.status_code);

    if (scan.status_code != 0) {
        wnet_vif->vif_sts.sts_rx_auth_fail++;
        wifi_mac_status_code_handle(sta, WIFINET_FC0_SUBTYPE_ASSOC_RESP, &scan);
        return;
    }

    WIFINET_VERIFY_ELEMENT(scan.rates, WIFINET_RATE_MAXSIZE);
    WIFINET_VERIFY_ELEMENT(scan.xrates, WIFINET_RATE_MAXSIZE);

    if (!wifi_mac_setup_rates(sta, scan.rates, scan.xrates,
        WIFINET_F_DOSORT | WIFINET_F_DOXSECT | WIFINET_F_DOBRS  | WIFINET_F_DOFRATE)) {
        ERROR_DEBUG_OUT("assoc failed (rate set mismatch)\n");
        wnet_vif->vif_sts.sts_rx_assoc_rate_mismatch++;
        wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN,0);
        return;
    }

    sta->sta_capinfo = scan.capinfo;
    sta->sta_associd = scan.assoc_id & 0x3FF;
    /*parse wmm for uapsd and access category edca parameters */
    if ((scan.wme != NULL) && (wifi_mac_parse_wmeparams(wnet_vif, scan.wme, wh, &qosinfo) >= 0)) {
        if (qosinfo & WME_CAPINFO_UAPSD_EN) {
            sta->sta_flags |= WIFINET_NODE_UAPSD;
            DPRINTF(AML_DEBUG_PWR_SAVE, "%s%d, ap support U-APSD\n", __func__, __LINE__);

        } else {
            sta->sta_flags &= ~WIFINET_NODE_UAPSD;
        }
        sta->sta_flags |= WIFINET_NODE_QOS;
        wifi_mac_wme_updateparams(wnet_vif);

    } else {
        sta->sta_flags &= ~WIFINET_NODE_QOS;
        sta->sta_flags &= ~WIFINET_NODE_UAPSD;
    }

    if (wifi_mac_is_ht_enable(sta)) {
        if (scan.htcap != NULL && scan.htinfo != NULL) {
            wifi_mac_parse_htcap(sta, scan.htcap);
            wifi_mac_parse_htinfo(sta, scan.htinfo);
            /*get rate from ht cap field. */
            wifi_mac_setup_ht_rates(sta, scan.htcap,WIFINET_F_DOBRS);
            wifi_mac_setup_basic_ht_rates(sta, scan.htinfo);
        }
    }

    if (scan.vht_cap) {
        wifi_mac_parse_vht_cap(sta, scan.vht_cap);
        /*filter the same rate as local and save. */
        wifi_mac_setup_vht_rates(sta, scan.vht_cap,
            WIFINET_F_DOFRATE | WIFINET_F_DOXSECT | WIFINET_F_DOBRS);

        if (scan.vht_opt) {
            wifi_mac_parse_vht_opt(sta, scan.vht_opt);
        }

        if (scan.vht_opt_md_ntf) {
            wifi_mac_parse_vht_op_md_ntf(sta, scan.vht_opt_md_ntf[2]);
        }
    }

    if (WIFINET_IS_CHAN_5GHZ(wifimac->wm_curchan) ||
        (sta->sta_capinfo & WIFINET_CAPINFO_SHORT_PREAMBLE)) {
        wifimac->wm_flags |= WIFINET_F_SHPREAMBLE;
        wifimac->wm_flags &= ~WIFINET_F_USEBARKER;

    } else {
        wifimac->wm_flags &= ~WIFINET_F_SHPREAMBLE;
        wifimac->wm_flags |= WIFINET_F_USEBARKER;
    }

    wifi_mac_set_shortslottime(wifimac,
        WIFINET_IS_CHAN_5GHZ(wifimac->wm_curchan) || (sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME));

    if ((WIFINET_INCLUDE_11G(wnet_vif->vm_mac_mode)) && (sta->sta_erp & WIFINET_ERP_USE_PROTECTION)) {
        wifimac->wm_flags |= WIFINET_F_USEPROT;

    } else {
        wifimac->wm_flags &= ~WIFINET_F_USEPROT;
    }
    /*set protect mode, no protect, rts/cts or cts only */
    wifi_mac_update_protmode(wifimac);

    if (wnet_vif->vm_hal_opmode == WIFI_M_IBSS) {
        wnet_vif->vm_mainsta->sta_associd = 1;
    }
    if (wnet_vif->vm_state == WIFINET_S_ASSOC) {
        drv_priv->drv_ops.drv_set_tx_livetime(999);
        drv_priv->drv_ops.drv_set_bssid(drv_priv, sta->wnet_vif_id, sta->sta_bssid);
        drv_priv->drv_ops.RegisterStationID(drv_priv, sta->wnet_vif_id, sta->sta_associd, sta->sta_bssid, sta->sta_encrypt_flag);
        wifi_mac_top_sm(wnet_vif, WIFINET_S_CONNECTED, WIFINET_FC0_SUBTYPE_ASSOC_RESP);
        wnet_vif->vm_phase_flags &= ~PHASE_CONNECTING;
        if (wnet_vif->vm_phase_flags & PHASE_DISCONNECT_DELAY) {
            int mgmt_arg = WIFINET_REASON_AUTH_LEAVE;

            AML_OUTPUT("disconnecting for last disconnect cmd\n");
            wnet_vif->vm_phase_flags &= ~PHASE_DISCONNECT_DELAY;
            wifi_mac_send_mgmt(wnet_vif->vm_mainsta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&mgmt_arg);
            return;
        }

    } else {
        return;
    }
    wifi_mac_save_app_ie(&wnet_vif->assocrsp_ie, os_skb_data(skb) + sizeof(struct wifi_frame) + 6/*sizeof(capinfo+status+associd)*/,os_skb_get_pktlen(skb) - sizeof(struct wifi_frame)- 6);
    wifi_mac_notify_nsta_connect(sta, 1);
    wifi_mac_new_assoc(sta, 0);
    vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb), os_skb_get_pktlen(skb));
    if ((sta->sta_flags & WIFINET_NODE_VHT) && (drv_priv->drv_config.cfg_dynamic_bw == 1)) {
        #ifdef DYNAMIC_BW
            wifi_mac_set_reg_val(MAC_REG_BASE, sta->sta_chbw);
            wifi_mac_set_reg_val(PHY_AGC_BUSY_FSM, 0);
            wifi_mac_set_reg_val(MAC_RXPKT_CONTROL45, sta->sta_chbw);
        #endif
    }
}

void wifi_mac_recv_deauth(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb,unsigned int channel)
{
    struct wifi_frame *wh;
    unsigned char *frm, *efrm;
    unsigned short subtype =0;
    unsigned short reason;

    wh = (struct wifi_frame *) os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    {
        if (((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_phase_flags & PHASE_DISCONNECTING))
            || ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) && (sta->is_disconnecting == 1))) {
            AML_OUTPUT("ignore rx deauth because we send first\n");
            return;
        }
        if (wnet_vif->vm_state == WIFINET_S_SCAN) {
            wnet_vif->vif_sts.sts_mng_discard++;
            return;
        }
        WIFINET_VERIFY_LENGTH(efrm - frm, 2);
        reason = le16toh(*(unsigned short *)frm);
        wnet_vif->vif_sts.sts_rx_deauth++;
        WIFINET_NODE_STAT(sta, rx_deauth);

        vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb),os_skb_get_pktlen(skb));

        WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "recv deauthenticate (reason %d)", reason);

        switch (wnet_vif->vm_opmode) {
            case WIFINET_M_STA:
                if (wnet_vif->vm_state != WIFINET_S_INIT) {
                    wnet_vif->vm_chan_roaming_scan_flag = 0;

                    wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
                }
                break;

            case WIFINET_M_HOSTAP:
                if (sta != wnet_vif->vm_mainsta) {
                    wifi_softap_allsta_stopping(wnet_vif,1);
                    wifi_mac_sta_disconnect_from_ap(sta);
                }
                break;

            default:
                wnet_vif->vif_sts.sts_mng_discard++;
                break;
        }
    }
}

void wifi_mac_recv_disassoc(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct sk_buff *skb,unsigned int channel)
{
    struct wifi_frame *wh;
    unsigned char *frm, *efrm;
    unsigned short subtype = 0;
    unsigned short reason;

    wh = (struct wifi_frame *)os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    {
        if ((wnet_vif->vm_state != WIFINET_S_CONNECTED) && (wnet_vif->vm_state != WIFINET_S_ASSOC)
            && (wnet_vif->vm_state != WIFINET_S_AUTH)) {
            wnet_vif->vif_sts.sts_mng_discard++;
            return;
        }

        if ((sta->sta_flags_ext & WIFINET_NODE_MFP) && (wnet_vif->vm_state == WIFINET_S_ASSOC)) {
            ERROR_DEBUG_OUT("pmf not this time, just return\n");
            return;
        }

        WIFINET_VERIFY_LENGTH(efrm - frm, 2);
        reason = le16toh(*(unsigned short *)frm);
        wnet_vif->vif_sts.sts_rx_dis_assoc++;
        WIFINET_NODE_STAT(sta, rx_disassoc);
        vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb),os_skb_get_pktlen(skb));

        WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "recv disassociate (reason %d)", reason);

        pr_debug("%s %d source bssid: %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, __LINE__,
            wh->i_addr2[0], wh->i_addr2[1],wh->i_addr2[2],
            wh->i_addr2[3], wh->i_addr2[4], wh->i_addr2[5]);

        pr_debug("%s %d connect AP bssid:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, __LINE__,
            wnet_vif->vm_des_bssid[0], wnet_vif->vm_des_bssid[1], wnet_vif->vm_des_bssid[2],
            wnet_vif->vm_des_bssid[3], wnet_vif->vm_des_bssid[4], wnet_vif->vm_des_bssid[5]);

        switch (wnet_vif->vm_opmode)
        {
            case WIFINET_M_STA:
                if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
                    wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);

                } else {
                    wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);
                }
                break;

            case WIFINET_M_HOSTAP:
                if (sta != wnet_vif->vm_mainsta) {
                     wifi_softap_allsta_stopping(wnet_vif,1);
                     wifi_mac_sta_disconnect_from_ap(sta);
                }
                break;

            default:
                wnet_vif->vif_sts.sts_mng_discard++;
                break;
        }
    }
}

void wifi_mac_recv_action(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,
    struct sk_buff *skb,unsigned int channel)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_frame *wh;
    unsigned char *frm, *efrm;
    unsigned short subtype = 0;
    struct wifi_mac_action *ia = NULL;
    struct wifi_mac_action_ht_txchwidth *iachwidth = NULL;
    struct wifi_mac_action_bss_coex_frame *iabsscoex = NULL;
    struct wifi_mac_action_ht_smpowersave *iasmpowersave = NULL;
    enum wifi_mac_bwc_width chwidth;
    struct wifi_mac_action_ba_addbarequest *addbarequest = NULL;
    struct wifi_mac_action_ba_addbaresponse *addbaresponse = NULL;
    struct wifi_mac_action_ba_delba *delba = NULL;
    struct wifi_mac_ba_parameterset baparamset;
    struct wifi_mac_ba_seqctrl basequencectrl;
    struct wifi_mac_delba_parameterset delbaparamset;
    struct wifi_mac_action_mgt_args actionargs;
    struct wifi_mac_action_sa_query *sa_query = NULL;
    struct wifi_station *remote_sta = NULL;
    struct wifi_mac_action_csa_frame *csa_frame = NULL;
    unsigned short statuscode;
    unsigned short batimeout;
    unsigned short reasoncode;
    unsigned short transaction_identifier;

    wh = (struct wifi_frame *)os_skb_data(skb);
    frm = (unsigned char *)os_skb_data(skb) + sizeof(struct wifi_frame);
    efrm = os_skb_data(skb) + os_skb_get_pktlen(skb);

    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    {
        /*if (wnet_vif->vm_state != WIFINET_S_CONNECTED &&
            wnet_vif->vm_state != WIFINET_S_ASSOC &&
            wnet_vif->vm_state != WIFINET_S_AUTH)
        {
#ifdef CONFIG_P2P
            if ((wnet_vif->vm_p2p_support == false) || (!wnet_vif->vm_p2p->p2p_enable))
#endif
            {
                wnet_vif->vif_sts.sts_mng_discard++;
                return;
            }
        }*/

        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action));
        ia = (struct wifi_mac_action *)frm;

        wnet_vif->vif_sts.sts_rx_action++;
        WIFINET_NODE_STAT(sta, rx_action);
        if (is_need_to_print((unsigned char *)frm)) {
            WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "action mgt frame (cat %d, act %d)",
                ia->ia_category, ia->ia_action);
        }
#ifdef CTS_VERIFIER_GAS
    if (ia->ia_category == AML_CATEGORY_PUBLIC && ia->ia_action == WIFINET_ACT_PUBLIC_GAS_RSP) {
        wnet_vif->vm_p2p->p2p_flag |= P2P_GAS_RSP;
        mdelay(50);
    }
#endif
    pr_debug("recv_action_frame subtype 0x%x, category %d\n", subtype, ia->ia_category);

        switch (ia->ia_category) {
            case AML_CATEGORY_QOS:
                pr_debug("<running> %s %d \n",__func__,__LINE__);
                WIFINET_DPRINTF_STA(AML_DEBUG_ACTION, sta, "%s: QoS action mgt frames not supported, vm_state %d \n",
                    __func__, wnet_vif->vm_state);
                wnet_vif->vif_sts.sts_mng_discard++;
                break;

            case AML_CATEGORY_BACK:
                switch (ia->ia_action) {
                    case WIFINET_ACTION_BA_ADDBA_REQUEST:
                        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_ba_addbarequest));

                        addbarequest = (struct wifi_mac_action_ba_addbarequest *)frm;
                        *(unsigned short *)&baparamset = le16toh(*(unsigned short *)&addbarequest->rq_baparamset);

                        batimeout = le16toh(addbarequest->rq_batimeout);
                        *(unsigned short *)&basequencectrl = le16toh(*(unsigned short *)&addbarequest->rq_basequencectrl);

                        DPRINTF(AML_DEBUG_WARNING, "%s: ADDBA request . TID %d, buffer size %d  vm_state %d\n",
                            __func__, baparamset.tid, baparamset.buffersize, wnet_vif->vm_state);

                        if ((!WIFINET_NODE_USEAMPDU(sta)) || (wnet_vif->vm_opmode == WIFINET_M_HOSTAP && softap_get_sta_num(wnet_vif) > 1)) {
                            wifi_mac_addba_set_rsp(sta, baparamset.tid, WIFINET_STATUS_REFUSED);

                        } else {
                            wifi_mac_addba_set_rsp(sta, baparamset.tid, WIFINET_STATUS_SUCCESS);
                        }
                        wifi_mac_addba_req(sta, addbarequest->rq_dialogtoken, &baparamset, batimeout, basequencectrl);

                        actionargs.category = AML_CATEGORY_BACK;
                        actionargs.action = WIFINET_ACTION_BA_ADDBA_RESPONSE;
                        actionargs.arg1 = baparamset.tid;
                        actionargs.arg2 = baparamset.amsdusupported;
                        actionargs.arg3 = 0;
                        wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_ACTION, (void *) &actionargs);
                        break;

                    case WIFINET_ACTION_BA_ADDBA_RESPONSE:
                    {
                        struct aml_driver_nsta *drv_sta = sta->drv_sta;
                        struct drv_tx_scoreboard *tid = NULL;

                        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_ba_addbaresponse));
                        addbaresponse = (struct wifi_mac_action_ba_addbaresponse *) frm;

                        DPRINTF(AML_DEBUG_ADDBA, "%s:%d vm_state %d \n",__func__,__LINE__, wnet_vif->vm_state);

                        statuscode = le16toh(addbaresponse->rs_statuscode);
                        *(unsigned short *)&baparamset = le16toh(*(unsigned short *)&addbaresponse->rs_baparamset);
                        tid = DRV_GET_TIDTXINFO(drv_sta, baparamset.tid);
                        if (tid->addba_exchangeinprogress == 0) {
                            AML_OUTPUT("discard addba response, tid %d\n", tid->tid_index);
                            break;
                        }
                        batimeout = le16toh(addbaresponse->rs_batimeout);
                        wifi_mac_addba_rsp(sta, statuscode, &baparamset, batimeout);

                        //check and triger tid queue

                        tid->tid_tx_buff_sending = 1;
                        wifi_mac_buffer_txq_send(&tid->tid_tx_buffer_queue);

                        DPRINTF(AML_DEBUG_ADDBA, "%s: ADDBA response . TID %d, buffer size %d,tid %d statuscode %d vm_state %d\n",
                            __func__, baparamset.tid, baparamset.buffersize,addbaresponse->rs_baparamset.tid,statuscode, wnet_vif->vm_state);
                        tid->tid_tx_buff_sending = 0;
                        break;
                    }
                    case WIFINET_ACTION_BA_DELBA:
                        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_ba_delba));

                        delba = (struct wifi_mac_action_ba_delba *)frm;
                        *(unsigned short *)&delbaparamset= le16toh(*(unsigned short *)&delba->dl_delbaparamset);
                        reasoncode = le16toh(delba->dl_reasoncode);
                        wifi_mac_delba(sta, &delbaparamset, reasoncode);
                        DPRINTF(AML_DEBUG_ADDBA, "%s: DELBA  . TID %d, initiator %d, reason code %d vm_state %d\n",
                            __func__, delbaparamset.tid, delbaparamset.initiator, reasoncode, wnet_vif->vm_state);
                        break;

                    default:
                        DPRINTF(AML_DEBUG_ADDBA, "%s: invalid BA action mgt frame", __func__);
                        wnet_vif->vif_sts.sts_mng_discard++;
                        break;
                }
                break;

            case AML_CATEGORY_PUBLIC:
                switch (ia->ia_action)
                {
                    case WIFINET_ACT_PUBLIC_BSSCOEXIST:
                        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_bss_coex_frame));

                        iabsscoex = (struct wifi_mac_action_bss_coex_frame *) frm;
                        WIFINET_DPRINTF_STA(AML_DEBUG_ACTION, sta, "%s: Element 0 inf request = %d, 40 intolerant = %d "
                            "20 width req = %d,  obss exempt req = %d, obss exempt grant = %d\n", __func__, iabsscoex->coex.inf_request,
                            iabsscoex->coex.ht40_intolerant, iabsscoex->coex.ht20_width_req, iabsscoex->coex.obss_exempt_req, iabsscoex->coex.obss_exempt_grant);

                        if (iabsscoex->coex.ht40_intolerant || iabsscoex->coex.ht20_width_req
                            || (((skb->data_len - sizeof(struct wifi_frame)) > (sizeof(struct wifi_mac_action_bss_coex_frame)))
                            && (wifi_mac_recv_bss_intol_channelCheck(wifimac, &iabsscoex->chan_report)==true))){

                            sta->sta_flags_ext |= WIFINET_NODE_40_INTOLERANT;
                            sta->sta_chbw = WIFINET_BWC_WIDTH20;
                            sta->sta_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;

                            sta->sta_wnet_vif->vm_curchan->chan_bw = WIFINET_BWC_WIDTH20;
                            sta->sta_wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH20;
                            sta->sta_wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_NA;
                            sta->sta_wnet_vif->vm_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;

                            wifi_mac_change_cbw(wifimac, 1);
                        }
                        break;

                    case WIFINET_ACT_PUBLIC_CSA:
                        if (wnet_vif->vm_opmode == WIFINET_M_STA && WIFINET_ADDR_EQ(sta->sta_bssid, wh->i_addr2)) {
                           csa_frame = (struct wifi_mac_action_csa_frame *) frm;
                           sta->sta_channel_switch_mode = csa_frame->csa.chan_switch_mode;
                           sta->sta_chan_switch_chan = csa_frame->csa.new_chan_num;
                           AML_OUTPUT("csa action frame:0x%x 0x%x 0x%x 0x%x\n", csa_frame->csa.chan_switch_mode, csa_frame->csa.new_chan_num,
                               csa_frame->csa.new_operation_class, csa_frame->csa.chan_switch_count);
                           os_timer_ex_start_period(&sta->csa_timer, csa_frame->csa.chan_switch_count * wnet_vif->vm_bcn_intval);
                        } else {
                            AML_OUTPUT("addr1:%s\n", ether_sprintf(wh->i_addr1));
                            AML_OUTPUT("addr2:%s\n", ether_sprintf(wh->i_addr2));
                            AML_OUTPUT("addr3:%s\n", ether_sprintf(wh->i_addr3));
                        }
                       break;

                    default:
                        vm_cfg80211_notify_mgmt_rx(wnet_vif, channel, os_skb_data(skb),os_skb_get_pktlen(skb));
                        break;
                }
                break;

            case AML_CATEGORY_HT:
                switch (ia->ia_action)
                {
                    case WIFINET_ACTION_HT_TXCHWIDTH:
                        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_ht_txchwidth));

                        iachwidth = (struct wifi_mac_action_ht_txchwidth *) frm;
                        chwidth = (iachwidth->at_chwidth == WIFINET_A_HT_TXCHWIDTH_2040) ?
                            WIFINET_BWC_WIDTH40 : WIFINET_BWC_WIDTH20;
                        sta->sta_chbw = chwidth;
                        DPRINTF(AML_DEBUG_BWC, "%s(%d) sta_chbw 0x%x \n", __func__, __LINE__,  sta->sta_chbw);

                        WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta,
                            "%s: HT txchwidth action mgt frame. Width %d ", __func__, chwidth);
                        break;

                    case WIFINET_ACTION_HT_SMPOWERSAVE:
                        pr_debug("<running> %s %d \n",__func__,__LINE__);
                        WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_ht_smpowersave));

                        iasmpowersave = (struct wifi_mac_action_ht_smpowersave *) frm;
                        if (iasmpowersave->as_control & WIFINET_A_HT_SMPOWERSAVE_ENABLED)
                        {
                            if (iasmpowersave->as_control & WIFINET_A_HT_SMPOWERSAVE_MODE)
                            {
                                if ((sta->sta_htcap & WIFINET_HTCAP_C_SM_MASK) != WIFINET_HTCAP_DYNAMIC_SMPS)
                                {
                                    sta->sta_htcap &= (~WIFINET_HTCAP_C_SM_MASK);
                                    sta->sta_htcap |= WIFINET_HTCAP_DYNAMIC_SMPS;
                                    sta->sta_update_rate_flag = WIFINET_NODE_SM_PWRSAV_DYN;
                                }
                            }
                            else
                            {
                                if ((sta->sta_htcap & WIFINET_HTCAP_C_SM_MASK) != WIFINET_HTCAP_STATIC_SMPS)
                                {
                                    sta->sta_htcap &= (~WIFINET_HTCAP_C_SM_MASK);
                                    sta->sta_htcap |= WIFINET_HTCAP_STATIC_SMPS;
                                    sta->sta_update_rate_flag = WIFINET_NODE_SM_PWRSAV_STAT;
                                }
                            }
                        }
                        else
                        {
                            if ((sta->sta_htcap & WIFINET_HTCAP_C_SM_MASK) != WIFINET_HTCAP_DISABLE_SMPS)
                            {
                                sta->sta_htcap &= (~WIFINET_HTCAP_C_SM_MASK);
                                sta->sta_htcap |= WIFINET_HTCAP_DISABLE_SMPS;
                                sta->sta_update_rate_flag = WIFINET_NODE_SM_EN;
                            }
                        }
                        if (sta->sta_update_rate_flag)
                        {
                            sta->sta_update_rate_flag |= WIFINET_NODE_RATECHG;
                        }
                        break;

                    default:
                        WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta, "%s: invalid HT action mgt frame", __func__);
                        wnet_vif->vif_sts.sts_mng_discard++;
                        break;
                }
                break;

            case AML_CATEGORY_SA_QUERY:
                WIFINET_VERIFY_LENGTH(efrm - frm, sizeof(struct wifi_mac_action_sa_query));

                sa_query = (struct wifi_mac_action_sa_query *)frm;
                transaction_identifier = sa_query->sa_transaction_identifier;
                if (sa_query->sa_header.ia_action == WIFINET_ACTION_SA_QUERY_REQ) {
                    wifi_mac_send_sa_query(sta, WIFINET_ACTION_SA_QUERY_RSP, transaction_identifier);

                } else if (sa_query->sa_header.ia_action == WIFINET_ACTION_SA_QUERY_RSP) {
                    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
                        remote_sta = wifi_mac_find_tx_sta(wnet_vif, wh->i_addr2);

                        if ((remote_sta != NULL) && (transaction_identifier == (remote_sta->sa_query_seq - 1))) {
                            remote_sta->sta_flags_ext &= ~WIFINET_NODE_MFP_CONFIRM_DEAUTH;
                            remote_sta->sa_query_try_count = 0;
                        }

                    } else {
                        if (transaction_identifier == (sta->sa_query_seq - 1)) {
                            sta->sta_flags_ext &= ~WIFINET_NODE_MFP_CONFIRM_DEAUTH;
                            sta->sa_query_try_count = 0;
                        }
                     }
                }

                DPRINTF(AML_DEBUG_WARNING, "%s: get sa query . action:%d, identifier:%d\n",
                    __func__, sa_query->sa_header.ia_action, transaction_identifier);
                break;

#ifdef CONFIG_P2P
            case  AML_CATEGORY_P2P:
                if (wnet_vif->vm_p2p_support) {
                    vm_cfg80211_notify_mgmt_rx(wnet_vif,channel, os_skb_data(skb),os_skb_get_pktlen(skb));
                }
                break;
#endif//CONFIG_P2P

            case AML_CATEGORY_VHT:
                switch (ia->ia_action)
                {
                    case WIFINET_ACTION_VHT_CMPRSSD_BMFRM:
                        break;
                    case WIFINET_ACTION_VHT_GROUP_ID:
                        wifi_mac_parse_group_id_mgmt(wnet_vif, sta, frm);
                        break;
                    case WIFINET_ACTION_VHT_OPERATE_MODE_NOTIFICATION:
                        wifi_mac_parse_operate_mode_notification_mgmt(wnet_vif, sta, frm);
                        break;
                    default:
                        break;
                }
                break;

            default:
                WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta,
                    "%s: action mgt frame has invalid category %d", __func__, ia->ia_category);
                wnet_vif->vif_sts.sts_mng_discard++;
                break;
        }
    }
}

void wifi_mac_parse_operate_mode_notification_mgmt(struct wlan_net_vif *wnet_vif,
        struct wifi_station *sta, unsigned char *frame)
{
    struct wifi_mac_action * ia = (struct wifi_mac_action *) frame;
    unsigned char *operating_mode = NULL;

    if (ia->ia_action != WIFINET_ACTION_VHT_OPERATE_MODE_NOTIFICATION) {
        return;
    }
    operating_mode = (unsigned char *)(frame + sizeof(struct wifi_mac_action));
    wifi_mac_parse_vht_op_md_ntf(sta, *operating_mode);
}


void wifi_mac_parse_group_id_mgmt(struct wlan_net_vif *wnet_vif,
        struct wifi_station *sta, unsigned char *frame)
{
#ifdef MU_BF
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_action * ia = (struct wifi_mac_action *) frame;
    /* 8bytes, 64bits */
    unsigned char *mem_ship = NULL;
    /* 16bytes, 128bits */
    unsigned char *user_position = NULL;
    unsigned char feedback_type = 0;

    if (ia->ia_action != WIFINET_ACTION_VHT_GROUP_ID)
        return;

    mem_ship = (unsigned char *)(frame + sizeof(struct wifi_mac_action));
    user_position = (unsigned char *)(mem_ship + 8);

    if ((sta->sta_flags & WIFINET_F_SU_BF) && (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_SU_BFMEE))
    {
        feedback_type = 0;
    }
    if ((sta->sta_flags & WIFINET_F_MU_BF) && (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MU_BFMEE))
    {
        feedback_type = 1;
    }
    wifimac->drv_priv->drv_ops.drv_set_bmfm_info(wifimac->drv_priv, wnet_vif->wnet_vif_id,
        mem_ship, user_position, feedback_type);
#endif
}

void wifi_mac_recv_mgmt(struct wifi_station *sta, struct sk_buff *skb,
    int subtype, int rssi, unsigned int channel)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_frame *wh;

    wh = (struct wifi_frame *)os_skb_data(skb);

    ASSERT(sta!= NULL);

    if ((sta != wnet_vif->vm_mainsta)
#ifdef CONFIG_P2P
        && (!wnet_vif->vm_p2p->p2p_enable)
#endif
       )
    {
        struct wifi_station_tbl *nt;
        struct wifi_station *sta_wds = NULL;
        nt = &wnet_vif->vm_sta_tbl;
        sta_wds = wifi_mac_find_wds_sta(nt, wh->i_addr2);

        if (sta_wds) {
            if ((subtype ==  WIFINET_FC0_SUBTYPE_AUTH)
                || (subtype ==  WIFINET_FC0_SUBTYPE_ASSOC_REQ)
                || (subtype ==  WIFINET_FC0_SUBTYPE_REASSOC_REQ)) {
                wifi_mac_rm_sta(nt,wh->i_addr2);
                sta = wifi_mac_bup_bss(wnet_vif,wh->i_addr2);
                if (sta == NULL) {
                    return;
                }
            }
        }
    }

    switch (subtype) {
        case WIFINET_FC0_SUBTYPE_BEACON:
            wifi_mac_recv_beacon(wnet_vif, sta, skb, rssi, channel);
            break;

        case WIFINET_FC0_SUBTYPE_PROBE_RESP:
            wifi_mac_recv_probersp(wnet_vif, sta, skb, rssi, channel);
            break;

        case WIFINET_FC0_SUBTYPE_PROBE_REQ:
            wifi_mac_recv_probe_req(wnet_vif, sta, skb, rssi, channel);
            break;

        case WIFINET_FC0_SUBTYPE_AUTH:
            wifi_mac_recv_auth(wnet_vif, sta, skb, channel, rssi);
            break;

        case WIFINET_FC0_SUBTYPE_ASSOC_REQ:
        case WIFINET_FC0_SUBTYPE_REASSOC_REQ:
            wifi_mac_recv_assoc_req(wnet_vif, sta, skb, channel,rssi);
            break;

        case WIFINET_FC0_SUBTYPE_ASSOC_RESP:
        case WIFINET_FC0_SUBTYPE_REASSOC_RESP:
            wifi_mac_recv_assoc_rsp(wnet_vif, sta, skb, channel);
            break;

        case WIFINET_FC0_SUBTYPE_DEAUTH:
            wifi_mac_recv_deauth(wnet_vif, sta, skb, channel);
            break;

        case WIFINET_FC0_SUBTYPE_DISASSOC:
            wifi_mac_recv_disassoc(wnet_vif, sta, skb, channel);
            break;
        case WIFINET_FC0_SUBTYPE_ACTION:
            wifi_mac_recv_action(wnet_vif, sta, skb, channel);
            break;
        default:
            WIFINET_DPRINTF(AML_DEBUG_ANY, "mgt subtype 0x%x not handled", subtype);
            break;
    }
}

static void
wifi_mac_amsdu_subframe_decap(struct sk_buff *skb)
{
    struct ether_header eh_src, *eh_dst;
    struct llc *llc;

    memcpy(&eh_src, os_skb_data(skb), sizeof(struct ether_header));
    llc = (struct llc *) os_skb_pull(skb, sizeof(struct ether_header));
    eh_src.ether_type = llc->llc_un.type_snap.ether_type;
    os_skb_pull(skb, LLC_SNAPFRAMELEN);

    eh_dst = (struct ether_header *) os_skb_push(skb, sizeof(struct ether_header));
    memcpy(eh_dst, &eh_src, sizeof(struct ether_header));
}


static int
wifi_mac_amsdu_input(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct ether_header *subfrmhdr;
    int subfrm_len;

    os_skb_pull(skb, sizeof(struct ether_header));

    while (os_skb_get_pktlen(skb) >= sizeof(struct ether_header))
    {
        struct sk_buff *skb_subfrm;

        subfrmhdr = (struct ether_header *)os_skb_data(skb);
        subfrm_len = __constant_ntohs(subfrmhdr->ether_type);

        if (subfrm_len < LLC_SNAPFRAMELEN)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV,
                                     subfrmhdr->ether_shost,
                                     "A-MSDU sub-frame too short: len %u",
                                     subfrm_len);

            goto err_amsdu;
        }

        subfrm_len += sizeof(struct ether_header);

        if (os_skb_get_pktlen(skb) == subfrm_len)
        {
            wifi_mac_amsdu_subframe_decap(skb);
            wifi_mac_deliver_data(sta, skb);

            return WIFINET_FC0_TYPE_DATA;
        }

        if (os_skb_get_pktlen(skb) < roundup(subfrm_len, 4))
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV,subfrmhdr->ether_shost,
                            "Short A-MSDU: len %u", os_skb_get_pktlen(skb));
            goto err_amsdu;
        }

        skb_subfrm = skb_clone(skb, GFP_ATOMIC);

        if (!skb_subfrm) {
            pr_warn("AMSDU: failed to clone sub frame\n");
            goto err_amsdu;
        }
        os_skb_pull(skb, roundup(subfrm_len, 4));

        wifi_mac_amsdu_subframe_decap(skb_subfrm);

        os_skb_trim(skb_subfrm, subfrm_len - LLC_SNAPFRAMELEN);

        //subfrm_cnt++;

        wifi_mac_deliver_data(sta, skb_subfrm);
    }
err_amsdu:
    os_skb_free(skb);

    return WIFINET_FC0_TYPE_DATA;
}

//not use now
void
wifi_mac_check_mic(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    struct wifi_frame *wh;
    struct wifi_mac_key *key=NULL;
    int hdrspace;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (os_skb_get_pktlen(skb) < sizeof(struct wifi_mac_frame_min))
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_ANY,
                                 sta->sta_macaddr,
                                 "too short (1): len %u", os_skb_get_pktlen(skb));
        wnet_vif->vif_sts.sts_rx_too_short++;
        return;
    }

    wh = (struct wifi_frame *)os_skb_data(skb);

    hdrspace = wifi_mac_hdrspace(wifimac, wh);
    key = wifi_mac_security_decap(sta, skb, hdrspace);
    if (key == NULL)
    {
        WIFINET_NODE_STAT(sta, rx_wepfail);
        return;
    }

    if (!wifi_mac_security_demic(wnet_vif, key, skb, hdrspace, 1))
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_RECV,
                                 sta->sta_macaddr, "data %s", "demic error");
        WIFINET_NODE_STAT(sta, rx_demicfail);
    }
    return;
}

void wifi_mac_csa_wait_task(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)param1;
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param2;
    unsigned char ret_char = (unsigned char)param3;
    struct wifi_channel *vmac_chan = (struct wifi_channel *)param4;

    /*
    unsigned int delay_ms = 0;
    while ((wifimac->wm_flags & WIFINET_F_DOTH) && (wifimac->wm_flags & WIFINET_F_CHANSWITCH) && delay_ms < 1000) {
        msleep(10);
        delay_ms += 10;
    }
    AML_OUTPUT("delay time [%d]\n", delay_ms);
    */

    if ((wnet_vif->vm_mainsta->sta_channel_switch_mode == 1) && (ret_char == true) && concurrent_check_is_vmac_same_pri_channel(wifimac)) {
        wnet_vif->vm_switchchan = vmac_chan;
        wnet_vif->vm_chan_switch_scan_flag = 1;
        wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_FLUSH |
            WIFINET_SCANCFG_CREATE, wnet_vif->vm_des_nssid, wnet_vif->vm_des_ssid);
    } else {
        wnet_vif->vm_mainsta->sta_channel_switch_mode = 0;
    }
}


int wifi_mac_handle_csa(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, int chan)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_ie_vht_wide_bw_switch *bw_chan_switch = (struct wifi_mac_ie_vht_wide_bw_switch *)sta->sta_wide_bw_ch_sw_sub_ie;
    struct wifi_channel *vmac_chan = NULL;
    int bw = 0, center_chan = 0;
    unsigned char ret_char = false;

    center_chan = chan;

    if (bw_chan_switch->elem_len != 0) {
        center_chan = bw_chan_switch->new_ch_freq_seg1;
        if (bw_chan_switch->new_ch_width == NEW_CHANNEL_BANDWIDTH40) {
            bw = WIFINET_BWC_WIDTH40;
        } else if (bw_chan_switch->new_ch_width == NEW_CHANNEL_BANDWIDTH80) {
            bw = WIFINET_BWC_WIDTH80;
        } else {
            ERROR_DEBUG_OUT("not support new_ch_width %d \n",bw_chan_switch->new_ch_width);
        }
        AML_OUTPUT("station channel switch chan:%d bw:%d center_chan:%d \n", chan, bw, center_chan);
    }

    sta->sta_chbw = bw;

    vmac_chan = wifi_mac_find_chan(wifimac, chan, bw, center_chan);
    if ((vmac_chan == WIFINET_CHAN_ERR) || (vmac_chan == wnet_vif->vm_curchan)) {
        if (vmac_chan == WIFINET_CHAN_ERR) {
            ERROR_DEBUG_OUT("null chan:%d bw:%d center_chan:%d\n", chan, bw, center_chan);
            return ret_char;
        } else {
            wnet_vif->vm_mainsta->sta_channel_switch_mode = 0;
        }
    } else {
        ret_char = wifi_mac_set_wnet_vif_channel(wnet_vif, chan, bw, center_chan,CHANNEL_CONNECT_FLAG | CHANNEL_RESTORE_FLAG);
        wifi_mac_add_work_task(wifimac, vm_cfg80211_chan_switch_notify_task, NULL, (SYS_TYPE)wifimac, (SYS_TYPE)wnet_vif, 0, (SYS_TYPE)vmac_chan, 0);
    }

    if ((wifimac->wm_nrunning > 1) && !concurrent_check_is_vmac_same_pri_channel(wifimac)) {
        if (wifimac->wm_vsdb_slot == CONCURRENT_SLOT_NONE) {
            if ((wnet_vif->vm_p2p_support == 1) || (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)) {
                wifimac->wm_vsdb_slot = CONCURRENT_SLOT_P2P;
            } else {
                wifimac->wm_vsdb_slot = CONCURRENT_SLOT_STA;
            }
            wifi_mac_set_vsdb_task(wifimac, wnet_vif, ENABLE);
        }

        if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode()) && concurrent_check_vmac_is_AP(wifimac)) {
            channel_switch_announce_trigger(wifimac, wnet_vif->vm_curchan);
        }
    }

    wifi_mac_add_work_task(wifimac, wifi_mac_csa_wait_task, NULL, (SYS_TYPE)wifimac, (SYS_TYPE)wnet_vif, (SYS_TYPE)ret_char, (SYS_TYPE)vmac_chan, 0);
    return ret_char;
}

int wifi_mac_csa_handle_timeout(void *arg)
{
    struct wifi_station *sta = (struct wifi_station *)arg;
    wifi_mac_handle_csa(sta->sta_wnet_vif, sta, sta->sta_chan_switch_chan);
    return OS_TIMER_NOT_REARMED;
}
