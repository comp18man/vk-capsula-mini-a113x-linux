/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac amsdu module,bulid amsdu when send,
 *
 *
 ****************************************************************************************
 */
#include "wifi_mac_com.h"
#include "wifi_debug.h"
#include "wifi_drv_xmit.h"

struct tasklet_struct amsdu_tasklet;

/*avoid use kalloc to allocate mem. */
static struct wifi_mac_msdu_node  g_msdu_node [(MAX_MSDU_CNT + 4) * WME_NUM_TID];

static int wifi_mac_msdu_list_init(struct msdu_list * msdu_list, int num)
{
    int i = 0;
    int node_num = num;

    if (msdu_list == NULL || num <= 0)
        return -1;

    INIT_LIST_HEAD(&(msdu_list->free_list));

    if (node_num > MAX_MSDU_CNT * WME_NUM_TID)
    {
        node_num = MAX_MSDU_CNT * WME_NUM_TID;
    }

    while (i < node_num)
    {
        INIT_LIST_HEAD(&(g_msdu_node[i].msdu_node));
        list_add_tail(&(g_msdu_node[i].msdu_node), &(msdu_list->free_list));
        i++;
    }
    return node_num;
}

static struct wifi_mac_msdu_node *wifi_mac_msdu_node_alloc(struct msdu_list *msdu_list)
{
    struct wifi_mac_msdu_node *msdu_node = NULL;

    if (msdu_list == NULL)
        return NULL;

    if (!list_empty(&(msdu_list->free_list)))
    {
        msdu_node = list_first_entry(&(msdu_list->free_list), struct wifi_mac_msdu_node, msdu_node);
        list_del_init(&(msdu_node->msdu_node));
    }

    return msdu_node;
}

static void wifi_mac_msdu_node_free(struct wifi_mac_msdu_node *msdu_node, struct msdu_list *msdu_list)
{
    if (msdu_node == NULL || msdu_list == NULL)
        return;

    list_add_tail(&(msdu_node->msdu_node), &(msdu_list->free_list));
}

static void
wifi_mac_amsdu_tx2drvlay(struct wifi_mac *wifimac, struct wifi_mac_amsdu_tx *amsdutx)
{
    struct wifi_station *sta = NULL;
    struct sk_buff *skbbuf = NULL;
    struct wifi_mac_tx_info *txinfo = NULL;
    int tid = 0;
    struct wlan_net_vif *wnet_vif = NULL;

    skbbuf = amsdutx->amsdu_tx_buf;
    if (skbbuf == NULL)
    {
        return;
    }
    amsdutx->amsdu_tx_buf = NULL;

    sta =  os_skb_get_nsta(skbbuf);
    wifimac = sta->sta_wmac;
    skbbuf = wifi_mac_encap(sta, skbbuf);
    txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    txinfo->b_amsdu = os_skb_is_amsdu(skbbuf);
    tid = os_skb_get_tid(skbbuf);

    WIFINET_AMSDU_LOCK(wifimac);
    if (txinfo->b_amsdu == 1)
    {
        wifimac->msdu_cnt[tid] = (wifimac->msdu_cnt[tid] > txinfo->amsdunum) ?
                            (wifimac->msdu_cnt[tid] - txinfo->amsdunum) : 0;
    }
    else
    {
        wifimac->msdu_cnt[tid] = (wifimac->msdu_cnt[tid] > 0) ? (wifimac->msdu_cnt[tid] - 1) : 0;
    }
    WIFINET_AMSDU_UNLOCK(wifimac);

    wnet_vif = netdev_priv(skbbuf->dev);

    if (wifi_mac_build_txinfo(wifimac, skbbuf, txinfo) != 0)
    {
        wifi_mac_complete_wbuf(skbbuf, 0);
        wnet_vif->vif_sts.sts_tx_amsdu_drop_msdu[txinfo->tid_index]++;
        return;
    }

    wnet_vif->vif_sts.sts_tx_amsdu_out_msdu[txinfo->tid_index]++;
    wifimac->drv_priv->drv_ops.tx_start(wifimac->drv_priv, skbbuf);
}

void wifi_mac_txamsdu_free_all(struct wifi_mac *wifimac, unsigned char vid)
{
    struct wifi_mac_amsdu_tx *amsdutx = NULL;
    struct sk_buff *skbbuf = NULL;
    struct wifi_mac_msdu_node *msdu_node = NULL;
    struct list_head amsdu_not_free;
    struct wifi_mac_msdu_node *need_free_node = NULL;
    int tid = 0;
    struct wifi_mac_tx_info *txinfo = NULL;

    INIT_LIST_HEAD(&amsdu_not_free);
    WIFINET_AMSDU_TASKLET_LOCK(wifimac);
    WIFINET_AMSDU_LOCK(wifimac);
    while (!list_empty(&wifimac->wm_amsdu_txq))
    {
        amsdutx = list_first_entry(&wifimac->wm_amsdu_txq, struct wifi_mac_amsdu_tx, amsdu_qelem);
        list_del_init(&amsdutx->amsdu_qelem);

        if ((amsdutx->vid == vid) || (vid == 3)) {
            amsdutx->b_work = 0;
            skbbuf = amsdutx->amsdu_tx_buf;
            amsdutx->amsdu_tx_buf = NULL;

            if (skbbuf != NULL) {
                tid = os_skb_get_tid(skbbuf);
                txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
                txinfo->b_amsdu = os_skb_is_amsdu(skbbuf);
                need_free_node = wifi_mac_msdu_node_alloc(&(wifimac->msdu_node_list));
                if(need_free_node == NULL) {
                    ERROR_DEBUG_OUT("alloc msdu_node failed\n");
                    break;
                }
                need_free_node->skbbuf = skbbuf;
                list_add_tail(&(need_free_node->msdu_node), &(wifimac->free_amsdu_list));

                if (txinfo->b_amsdu == 1)
                {
                    wifimac->msdu_cnt[tid] = (wifimac->msdu_cnt[tid] > txinfo->amsdunum) ?
                            (wifimac->msdu_cnt[tid] - txinfo->amsdunum) : 0;

                } else {
                    wifimac->msdu_cnt[tid] = (wifimac->msdu_cnt[tid] > 0) ? (wifimac->msdu_cnt[tid] - 1) : 0;
                }
            }

            while (!list_empty(&amsdutx->msdu_list)) {
                msdu_node = list_first_entry(&amsdutx->msdu_list, struct wifi_mac_msdu_node, msdu_node);
                tid = os_skb_get_tid(msdu_node->skbbuf);
                list_del_init(&msdu_node->msdu_node);
                list_add_tail(&(msdu_node->msdu_node), &(wifimac->free_amsdu_list));
                wifimac->msdu_cnt[tid] = (wifimac->msdu_cnt[tid] > 0) ? (wifimac->msdu_cnt[tid] - 1) : 0;
            }

            while (amsdutx->amsdunum > 0) {
                amsdutx->amsdunum--;
                skbbuf = amsdutx->msdu_tmp_buf[amsdutx->amsdunum];
                amsdutx->msdu_tmp_buf[amsdutx->amsdunum] = NULL;
                if (skbbuf != NULL) {
                    tid = os_skb_get_tid(skbbuf);
                    need_free_node = wifi_mac_msdu_node_alloc(&(wifimac->msdu_node_list));
                    if(need_free_node == NULL) {
                        ERROR_DEBUG_OUT("alloc msdu_node failed\n");
                        break;
                    }
                    need_free_node->skbbuf = skbbuf;
                    list_add_tail(&(need_free_node->msdu_node), &(wifimac->free_amsdu_list));
                    wifimac->msdu_cnt[tid] = (wifimac->msdu_cnt[tid] > 0) ? (wifimac->msdu_cnt[tid] - 1) : 0;
                }
            }
        } else {
            list_add_tail(&amsdutx->amsdu_qelem, &amsdu_not_free);
        }
    }

    if (!list_empty(&amsdu_not_free)) {
        list_splice_tail_init(&amsdu_not_free, &wifimac->wm_amsdu_txq);
    }
    WIFINET_AMSDU_UNLOCK(wifimac);
    WIFINET_AMSDU_TASKLET_UNLOCK(wifimac);
}

/*shijie.chen add amsdu tasklet to aggregate amsdu and remove amsdu timer*/
static void wifi_mac_amsdu_tasklet(unsigned long arg)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)(SYS_TYPE)arg;
    struct wifi_mac_amsdu_tx *amsdutx = NULL;
    struct wifi_mac_msdu_node *msdu_node = NULL;
    unsigned long now_time;
    struct drv_private *drv_priv = NULL;

    struct wifi_mac_msdu_node *need_free_node = NULL;
    drv_priv = wifimac->drv_priv;

    WIFINET_AMSDU_LOCK(wifimac);
    while (!list_empty(&wifimac->wm_amsdu_txq))
    {
        amsdutx = list_first_entry(&wifimac->wm_amsdu_txq, struct wifi_mac_amsdu_tx, amsdu_qelem);
        /*get tid list and send all buffer data firstly and then do dequeue. */
        while (!list_empty(&amsdutx->msdu_list))
        {
            msdu_node = list_first_entry(&amsdutx->msdu_list, struct wifi_mac_msdu_node, msdu_node);
            /*dequeue from amsdutx->msdu_list and enqueue to msdu_node_list->free_list */
            list_del_init(&msdu_node->msdu_node);

            WIFINET_AMSDU_UNLOCK(wifimac);
            wifi_mac_amsdu_ex(msdu_node->skbbuf);
            WIFINET_AMSDU_LOCK(wifimac);

            wifi_mac_msdu_node_free(msdu_node, &(wifimac->msdu_node_list));
        }

        /*
        No remain msdu in tmp buf.
        FIXME: if there more than one tid txqs list on 'wm_amsdu_txq',
        the next tid txqs are held on.
        */
        if (amsdutx->amsdunum == 0)
        {
            list_del_init(&amsdutx->amsdu_qelem);
            amsdutx->b_work = 0;
            continue;
        }
        else
        {
            break;
        }
    }

    while (!list_empty(&wifimac->free_amsdu_list)) {
        need_free_node = list_first_entry(&wifimac->free_amsdu_list, struct wifi_mac_msdu_node, msdu_node);
        list_del_init(&need_free_node->msdu_node);
        wifi_mac_complete_wbuf(need_free_node->skbbuf, 0);
        wifi_mac_msdu_node_free(need_free_node, &(wifimac->msdu_node_list));
    }
    WIFINET_AMSDU_UNLOCK(wifimac);

    /*flush remain msdus if exist and live time is timeout*/
    if ((amsdutx != NULL) && (amsdutx->amsdunum > 0))
    {
        now_time = jiffies;
        if (!time_after(now_time, amsdutx->in_time + (AMSDU_MAX_LIVE_TIME*HZ/1000)))
        {
            return;
        }

        if (wifi_mac_amsdu_aggr(wifimac, amsdutx) != NULL)
        {
            wifi_mac_amsdu_tx2drvlay(wifimac, amsdutx);
        }
    }
}

static void wifi_mac_amsdu_tasklet_ex(unsigned long arg)
{
    WIFINET_AMSDU_TASKLET_LOCK((struct wifi_mac *)(SYS_TYPE)arg);
    wifi_mac_amsdu_tasklet(arg);
    WIFINET_AMSDU_TASKLET_UNLOCK((struct wifi_mac *)(SYS_TYPE)arg);
}

int wifi_mac_txamsdu_task(void *arg)
{
    tasklet_schedule(&amsdu_tasklet);
    return OS_TIMER_REARMED;
}

static int wifi_mac_amsdu_check(struct sk_buff *skb)
{
    struct ether_header *eh;
    if (M_FLAG_GET(skb, M_UAPSD)|| M_PWR_SAV_GET(skb))
    {
        return 1;
    }
    eh = (struct ether_header *)(os_skb_data(skb));

    if (eh->ether_type == __constant_htons(ETHERTYPE_IP))
    {
        return 0;
    }

    return 1;
}

static void wifi_mac_amsdu_encap(struct sk_buff *amsdu_buf, struct sk_buff *skb, int prepend_ether)
{
    struct ether_header *eh;
    struct llc *llc;
    struct ether_header *eh_inter;
    unsigned short payload, msdulen, padlen;
    int offset;
    unsigned char *dest, *src;

    if (prepend_ether)
    {
        skb_reserve(amsdu_buf, 44);
        eh_inter = (struct ether_header *)(os_skb_data(amsdu_buf));
        memcpy(eh_inter, os_skb_data(skb), sizeof(struct ether_header));
        skb_put(amsdu_buf, sizeof(struct ether_header));
    }

    if (!prepend_ether)
    {
        payload = roundup((os_skb_get_pktlen(amsdu_buf)-sizeof(struct ether_header)), 4);
        padlen = payload - (os_skb_get_pktlen(amsdu_buf)-sizeof(struct ether_header));
        skb_put(amsdu_buf, padlen);
    }

    offset = os_skb_get_pktlen(amsdu_buf);

    eh = (struct ether_header *)(os_skb_data(skb));

    eh_inter = (struct ether_header *)(os_skb_data(amsdu_buf)+offset);

    memcpy(eh_inter, eh, sizeof(struct ether_header) - sizeof(eh->ether_type)); //only copy DA,SA fields
    payload = os_skb_get_pktlen(skb) + LLC_SNAPFRAMELEN;
    msdulen = payload - sizeof(struct ether_header);
    eh_inter->ether_type = __constant_htons(msdulen);

    DPRINTF(AML_DEBUG_INFO,"%s(%d) pktlen %d msdulen %d payload %d\n",
            __func__,__LINE__, os_skb_get_pktlen(skb), payload, msdulen);

    llc = (struct llc *)((unsigned char *)eh_inter + sizeof(struct ether_header));
    llc->llc_dsap = llc->llc_ssap = LLC_SNAP_LSAP;
    llc->llc_control = LLC_UI;
    llc->llc_snap.org_code[0] = 0;
    llc->llc_snap.org_code[1] = 0;
    llc->llc_snap.org_code[2] = 0;
    llc->llc_snap.ether_type = eh->ether_type;

    dest = (unsigned char *)((unsigned char *)llc + sizeof(struct llc));
    src = (unsigned char *)(os_skb_data(skb) + sizeof(struct ether_header));
    memcpy(dest, src, (os_skb_get_pktlen(skb) - sizeof(struct ether_header)));

    os_skb_put(amsdu_buf, payload);
}

/* alloc skb with specific length dynamic to aggregate amsdu.*/
struct sk_buff *wifi_mac_amsdu_aggr(struct wifi_mac *wifimac, struct wifi_mac_amsdu_tx *amsdutx)
{
    struct wifi_station *sta = NULL;
    struct sk_buff *amsdu_wbuf = NULL;
    unsigned char tid_index = 0;
    int i = 0, skblen = 0;
    struct wifi_mac_tx_info *txinfo = NULL;
    struct drv_txdesc *msdu_ptxdesc = NULL;
    struct drv_txdesc *amsdu_ptxdesc = NULL;

    /*no msdu need to aggregate */
    if (amsdutx->amsdunum == 0)
    {
        return NULL;
    }

    if (amsdutx->amsdu_tx_buf != NULL)
    {
        wifi_mac_free_skb(amsdutx->amsdu_tx_buf);
    }

    sta = os_skb_get_nsta(amsdutx->msdu_tmp_buf[0]);
    tid_index = os_skb_get_tid(amsdutx->msdu_tmp_buf[0]);
    for (i = 0; i < amsdutx->amsdunum; i++)
    {
        if (i == 0)
        {
            skblen = amsdutx->framelen + AMSDU_BUFFER_HEADROOM + sizeof(struct ether_header) + 512;
            amsdutx->amsdu_tx_buf = wifi_mac_alloc_skb(wifimac, skblen);
            if (amsdutx->amsdu_tx_buf == NULL)
            {
                ERROR_DEBUG_OUT("alloc skb fail\n");
                return NULL;
            }

            amsdu_wbuf = amsdutx->amsdu_tx_buf;
            txinfo = (struct wifi_mac_tx_info *)os_skb_cb(amsdu_wbuf);

            wifi_mac_amsdu_encap(amsdu_wbuf, amsdutx->msdu_tmp_buf[i], 1);
            os_skb_set_priority(amsdu_wbuf, os_skb_get_priority(amsdutx->msdu_tmp_buf[i]));
            os_skb_set_tid(amsdu_wbuf, tid_index);
            os_skb_set_nsta(amsdu_wbuf, sta);
            os_skb_set_amsdu(amsdu_wbuf);
            amsdu_wbuf->dev = amsdutx->msdu_tmp_buf[i]->dev;

            msdu_ptxdesc = ((struct wifi_mac_tx_info *)os_skb_cb(amsdutx->msdu_tmp_buf[i]))->ptxdesc;
            amsdu_ptxdesc = txinfo->ptxdesc;
            memcpy(&amsdu_ptxdesc->drv_pkt_info.pkt_info[i], &msdu_ptxdesc->drv_pkt_info.pkt_info[0], sizeof(struct wifi_mac_pkt_info));

            amsdu_ptxdesc->drv_pkt_info.pkt_info_count++;

            if (M_FLAG_GET(amsdutx->msdu_tmp_buf[i], M_CHECKSUMHW))
            {
                DPRINTF(AML_DEBUG_INFO, "%s %d inherit M_CHECKSUMHW\n", __func__,__LINE__);
                M_FLAG_SET(amsdu_wbuf, M_CHECKSUMHW);
            }
        }
        else
        {
            wifi_mac_amsdu_encap(amsdu_wbuf, amsdutx->msdu_tmp_buf[i], 0);
            msdu_ptxdesc = ((struct wifi_mac_tx_info *)os_skb_cb(amsdutx->msdu_tmp_buf[i]))->ptxdesc;
            memcpy(&amsdu_ptxdesc->drv_pkt_info.pkt_info[i], &msdu_ptxdesc->drv_pkt_info.pkt_info[0], sizeof(struct wifi_mac_pkt_info));

            if (!((amsdu_ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_ack_del && amsdu_ptxdesc->drv_pkt_info.pkt_info[i].b_tcp_ack_del) &&
                ((amsdu_ptxdesc->drv_pkt_info.pkt_info[0].tcp_src_port == amsdu_ptxdesc->drv_pkt_info.pkt_info[i].tcp_src_port) &&
                (amsdu_ptxdesc->drv_pkt_info.pkt_info[0].tcp_dst_port == amsdu_ptxdesc->drv_pkt_info.pkt_info[i].tcp_dst_port)))) {
                amsdu_ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_ack_del = 0;
            }

            amsdu_ptxdesc->drv_pkt_info.pkt_info_count++;
        }

        wifi_mac_complete_wbuf(amsdutx->msdu_tmp_buf[i], 0);
        amsdutx->msdu_tmp_buf[i] = NULL;
    }

    /*get amsdu sub frame num */
    txinfo->amsdunum = amsdutx->amsdunum;

    amsdutx->amsdunum = 0;
    amsdutx->framelen = 0;
    return amsdu_wbuf;
}

struct sk_buff *wifi_mac_amsdu_ex( struct sk_buff *skbbuf)
{
    struct wifi_station *sta = os_skb_get_nsta(skbbuf);
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wifi_mac_amsdu_tx *amsdutx = NULL;
    struct drv_private *drv_priv = NULL;
    unsigned char tid_index = os_skb_get_tid(skbbuf);
    unsigned int framelen = 0, amsdu_max_buffer_size = 0, amsdu_max_len = 0;
    int amsdu_deny = 0, tx_amsdu_sub_max = 0, txcnt = 0;
    unsigned long now_time;
    static unsigned int ampdu_max_len = 3212;

    drv_priv = wifimac->drv_priv;
    amsdutx = &(sta->sta_amsdu->amsdutx[tid_index]);

    if((sta->sta_chbw == WIFINET_BWC_WIDTH80)
        && (drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH80))
    {
        amsdu_max_buffer_size = AMSDU_MAX_BUFFER_SIZE_BW80;
        amsdu_max_len = AMSDU_MAX_LEN_BW80;
        tx_amsdu_sub_max = DEFAULT_TXAMSDU_SUB_MAX_BW80;
    }
    else
    {
       amsdu_max_buffer_size = AMSDU_MAX_BUFFER_SIZE;
       amsdu_max_len = AMSDU_MAX_LEN;
       tx_amsdu_sub_max = DEFAULT_TXAMSDU_SUB_MAX;
    }

    amsdu_max_len = MIN(sta->sta_amsdu->amsdu_max_length, amsdu_max_len);
    amsdu_max_len = MIN(amsdu_max_len, ampdu_max_len);

    /*adjust max subframe num dynamicly to send to hal-layer ASAP if hal-layer no packets to send. */
    txcnt = wifimac->drv_priv->drv_ops.txlist_all_qcnt(wifimac->drv_priv, wifimac->wm_ac2q[os_skb_get_priority(skbbuf)]);
    if (txcnt <= 4)
    {
        tx_amsdu_sub_max = DEFAULT_TXAMSDU_SUB_MAX_BW80/4;
    }
    else
    {
       tx_amsdu_sub_max = DEFAULT_TXAMSDU_SUB_MAX_BW80/2;
    }
    amsdu_deny = wifi_mac_amsdu_check(skbbuf);
    framelen = AMSDU_SUBFRAME_TOTAL_LEN(os_skb_get_pktlen(skbbuf));

    /*if skbbuf is deny transmit bufferable skb previously and return current skbbuf */
    if ((framelen > AMSDU_MAX_SUBFRM_LEN) || amsdu_deny )
    {
        DPRINTF(AML_DEBUG_INFO, "%s %d tx amsdu firstly\n", __func__,__LINE__);
        if (amsdutx->amsdunum > 0)
        {
            if (wifi_mac_amsdu_aggr(wifimac, amsdutx) != NULL)
            {
                wifi_mac_amsdu_tx2drvlay(wifimac, amsdutx);
            }
        }
        /*send msdu frames via amsdu path */
        amsdutx->amsdu_tx_buf = skbbuf;
        wifi_mac_amsdu_tx2drvlay(wifimac, amsdutx);
        return NULL;
    }

    if (amsdutx->amsdunum == 0)
    {
        amsdutx->in_time = jiffies;
    }

    now_time = jiffies;
    if (((framelen + amsdutx->framelen + sizeof(struct ether_header)) <= amsdu_max_len)
        && (amsdutx->amsdunum + 1 <= tx_amsdu_sub_max)
        && (!time_after(now_time, amsdutx->in_time + (AMSDU_MAX_LIVE_TIME*HZ/1000))))
    {
        DPRINTF(AML_DEBUG_INFO, "%s %d just add to amsdu\n", __func__,__LINE__);
        amsdutx->msdu_tmp_buf[amsdutx->amsdunum] = skbbuf;
        amsdutx->framelen += framelen;
        amsdutx->amsdunum++;

        /*if meet aggregation condition after last aggregating, we shall send amsdu immediately. */
        if ((amsdutx->framelen + sizeof(struct ether_header) == amsdu_max_len)
          || (amsdutx->amsdunum == tx_amsdu_sub_max))
        {
            if (wifi_mac_amsdu_aggr(wifimac, amsdutx) != NULL)
            {
                wifi_mac_amsdu_tx2drvlay(wifimac, amsdutx);
            }
        }
    }
    else
    {
        if (wifi_mac_amsdu_aggr(wifimac, amsdutx) != NULL)
        {
            wifi_mac_amsdu_tx2drvlay(wifimac, amsdutx);
        }
        /*process exception */
        if (amsdutx->amsdunum >= DEFAULT_TXAMSDU_SUB_MAX_BW80 * 2)
        {
            ERROR_DEBUG_OUT("alloc skb fail, drop skb\n");
            while (--(amsdutx->amsdunum) >= 0)
            {
                wifi_mac_complete_wbuf(amsdutx->msdu_tmp_buf[amsdutx->amsdunum], 0);
            }
            amsdutx->amsdunum = 0;
            amsdutx->framelen = 0;
        }
        amsdutx->msdu_tmp_buf[amsdutx->amsdunum] = skbbuf;
        amsdutx->framelen += framelen;
        amsdutx->amsdunum++;
    }
    return NULL;
}

struct sk_buff *wifi_mac_amsdu_send( struct sk_buff * skbbuf)
{
    struct wifi_station *sta = os_skb_get_nsta(skbbuf);
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_msdu_node *msdu_node = NULL;
    struct wifi_mac_amsdu_tx *amsdutx = NULL;
    unsigned char tid_index = os_skb_get_tid(skbbuf);

    if (sta->sta_amsdu == NULL)
    {
        ERROR_DEBUG_OUT("ERROR: not called\n");
        return skbbuf;
    }

    if (wnet_vif->vm_fragthreshold < WIFINET_FRAGMT_THRESHOLD_MAX ||
        !(wnet_vif->vm_opmode == WIFINET_M_STA ||
          wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT ||
          wnet_vif->vm_opmode == WIFINET_M_HOSTAP||
          wnet_vif->vm_opmode == WIFINET_M_P2P_GO))
    {
        ERROR_DEBUG_OUT("not satisfy amsdu\n");
        return skbbuf;
    }

    /* shijie.chen add. FIXME: add spin_lock for every tid. */
    WIFINET_AMSDU_LOCK(wifimac);
    msdu_node = wifi_mac_msdu_node_alloc(&(wifimac->msdu_node_list));
    if (msdu_node == NULL)
    {
        WIFINET_AMSDU_UNLOCK(wifimac);
        ERROR_DEBUG_OUT("alloc failed\n");
        return skbbuf;
    }

    /* buffer all data frames from tcp/ip layer with specific tid of a sta */
    msdu_node->skbbuf = skbbuf;
    amsdutx = &(sta->sta_amsdu->amsdutx[tid_index]);
    list_add_tail(&msdu_node->msdu_node, &amsdutx->msdu_list);
    wifimac->msdu_cnt[tid_index] ++;
    wnet_vif->vif_sts.sts_tx_amsdu_in_msdu[tid_index]++;

    if (amsdutx->b_work == 0)
    {
        amsdutx->b_work = 1;
        list_add_tail(&amsdutx->amsdu_qelem, &wifimac->wm_amsdu_txq);
    }
    /*counter for one tid for all stations */

    amsdutx->tid = tid_index;
    WIFINET_AMSDU_UNLOCK(wifimac);

    if (wifimac->msdu_cnt[tid_index] > 1)
    {
        wifi_mac_amsdu_tasklet_ex((unsigned long)(SYS_TYPE)wifimac);
    } else {
        tasklet_schedule(&amsdu_tasklet);
    }
    return NULL;
}

int wifi_mac_amsdu_attach(struct wifi_mac *wifimac)
{
    int tid = 0;

    INIT_LIST_HEAD(&wifimac->wm_amsdu_txq);
    INIT_LIST_HEAD(&wifimac->free_amsdu_list);
    tasklet_init(&amsdu_tasklet, wifi_mac_amsdu_tasklet_ex, (unsigned long)(SYS_TYPE)wifimac);
    os_timer_ex_initialize(&wifimac->drv_priv->drv_txtimer, 0, wifi_mac_txamsdu_task, &wifimac->drv_priv);
    WIFINET_AMSDU_LOCK_INIT(wifimac);
    WIFINET_AMSDU_TASKLET_LOCK_INIT(wifimac);

    for (tid = 0; tid < WME_NUM_TID; tid++)
    {
        wifimac->msdu_cnt[tid] = 0;
    }
    wifi_mac_msdu_list_init(&(wifimac->msdu_node_list), MAX_MSDU_CNT * WME_NUM_TID);

    return 0;
}

void
wifi_mac_amsdu_detach(struct wifi_mac *wifimac)
{
    tasklet_kill(&amsdu_tasklet);
    WIFINET_AMSDU_LOCK_DESTROY(wifimac);
}

int wifi_mac_alloc_amsdu_node(struct wifi_mac *wifimac, unsigned char vid, struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int tid = 0;

    sta->sta_amsdu = (struct wifi_mac_amsdu *)NET_MALLOC(sizeof(struct wifi_mac_amsdu), GFP_ATOMIC, "sta_amsdu alloc");
    if (sta->sta_amsdu == NULL)
    {
        DPRINTF( AML_DEBUG_INFO,"<running> %s %d sta_amsdu= %p fail \n",__func__,__LINE__,sta->sta_amsdu);
        return ENOMEM;
    }

    sta->sta_amsdu->amsdu_max_length = AMSDU_MAX_BUFFER_SIZE_BW80;
    sta->sta_amsdu->amsdu_max_sub = DEFAULT_TXAMSDU_SUB_MAX_BW80;

    /*init amsdu max length capability */
    if (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MAX_MPDU_LEN_11454)
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, VHT_MPDU_SIZE_11451);

    } else if (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MAX_MPDU_LEN_7991) {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, VHT_MPDU_SIZE_7991);

    } else {
         sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, VHT_MPDU_SIZE_3895);
    }

    if (wnet_vif->vm_htcap & WIFINET_HTCAP_C_MAXAMSDUSIZE)
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, AMSDU_SIZE_7935);
    }
    else
    {
        sta->sta_amsdu->amsdu_max_length = MIN(sta->sta_amsdu->amsdu_max_length, AMSDU_SIZE_3839);
    }

    pr_debug("<running> %s (%d) amsdu_max_length=%d\n", __func__,__LINE__, sta->sta_amsdu->amsdu_max_length );

    for (tid = 0; tid < WME_NUM_TID; tid++)
    {
        INIT_LIST_HEAD(&(sta->sta_amsdu->amsdutx[tid].msdu_list));
        sta->sta_amsdu->amsdutx[tid].vid = vid;
    }
    return 0;
}

void wifi_mac_amsdu_nsta_free(struct wifi_mac *wifimac, struct wifi_station *sta)
{
    if (sta->sta_amsdu)
        NET_FREE(sta->sta_amsdu, "sta_amsdu alloc");
}

