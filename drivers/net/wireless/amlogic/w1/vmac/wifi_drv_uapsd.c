/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     driver layer u-APSU power save module
 *
 *
 ****************************************************************************************
 */

#include "wifi_drv_uapsd.h"
#include "wifi_mac_com.h"
#include "wifi_drv_xmit.h"
#include "wifi_pkt_desc.h"

static void drv_txdesc_list_init(struct list_head *txdesc_list, int *qcnt)
{
    INIT_LIST_HEAD(txdesc_list);
    (*qcnt) = 0;
}

static void drv_txdesc_list_cleanup(struct drv_private *drv_priv, int *qcnt, struct drv_txdesc *ptxdesc, struct drv_txlist *txlist)
{
    if (ptxdesc == NULL)
    {
        return;
    }

    list_del_init(&ptxdesc->txdesc_queue);
    (*qcnt) --;

    DRV_TXQ_UNLOCK(txlist);
    drv_tx_complete(drv_priv, ptxdesc, 0);
    DRV_TXQ_LOCK(txlist);
}

/*
 * Reclaim all UAPSD nsta resources.
 * Context: Tasklet
 */
static void drv_txdesc_list_drain(struct drv_private *drv_priv, struct list_head *txdesc_list, int *qcnt, unsigned char vid)
{
    struct drv_txdesc *ptxdesc = NULL;
    struct list_head txdesc_list_head_not_free;

    if (*qcnt == 0) {
        return;
    }

    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d qcnt=%d\n",__func__,__LINE__, *qcnt);

    INIT_LIST_HEAD(&txdesc_list_head_not_free);

    DRV_TX_QUEUE_LOCK(drv_priv);
    while (!list_empty(txdesc_list)) {
        ptxdesc = list_first_entry(txdesc_list, struct drv_txdesc, txdesc_queue);
        list_del_init(&ptxdesc->txdesc_queue);

        if ((ptxdesc->txinfo->wnet_vif_id == vid) || (vid == 3)) {
            (*qcnt) --;
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            drv_tx_complete(drv_priv, ptxdesc, 0);
            DRV_TX_QUEUE_LOCK(drv_priv);

        } else {
            list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head_not_free);
        }
    }

    if (!list_empty(&txdesc_list_head_not_free)) {
        list_splice_tail_init(&txdesc_list_head_not_free, txdesc_list);
    }
    DRV_TX_QUEUE_UNLOCK(drv_priv);
}

/*
 * Add frames to per nsta UAPSD queue.
 * Frames will be transmitted on receiving trigger.
 * Context: Tasklet
 */
static void drv_txdesc_list_queue (struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct list_head *txdesc_list, int *qcnt)
{
    struct drv_txdesc *ptxdesc, *bf_prev;
    struct list_head *bf_qnode = NULL;

    if (list_empty(txdesc_list_head))
    {
        DPRINTF(AML_DEBUG_ERROR, "<ERROR>%s %d buffer head qcnt=%d !\n",__func__,__LINE__, (*qcnt));
        return;
    }
    ptxdesc = list_first_entry(txdesc_list_head, struct drv_txdesc, txdesc_queue);

    /*
     * Lock out interrupts since this queue shall be accessed
     * in interrupt context.
     */
    DRV_TX_QUEUE_LOCK(drv_priv);
    if (!list_empty(txdesc_list))
    {
        bf_qnode = txdesc_list->prev;
        bf_prev = list_entry(bf_qnode, struct drv_txdesc, txdesc_queue);
        bf_prev->txdesc_queue_next = ptxdesc;
    }

    list_splice_tail_init(txdesc_list_head,txdesc_list);
    (*qcnt) ++;
    DRV_TX_QUEUE_UNLOCK(drv_priv);

    //DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d bufffer head qcnt=%d\n",__func__,__LINE__, (*qcnt));
}

#ifdef CONFIG_P2P
static int drv_if_noa_started (struct drv_txdesc *ptxdesc)
{
    int ret = -1;
    struct sk_buff *skbbuf;
    struct wifi_skb_callback *cb;
    struct wifi_station *sta;

    do
    {
        ASSERT((SYS_TYPE)ptxdesc > 0x500);
        if (ptxdesc==NULL)
        {
            break;
        }
        skbbuf = ptxdesc->txdesc_mpdu;
        ASSERT((SYS_TYPE)skbbuf > 0x500);
        if (skbbuf==NULL)
        {
            break;
        }
        cb = (struct wifi_skb_callback *)skbbuf->cb;
        ASSERT((SYS_TYPE)cb > 0x500);
        if (cb==NULL)
        {
            break;
        }
        sta = cb->sta;
        ASSERT((SYS_TYPE)sta > 0x500);
        /* nsta may disconnect during tx */

        if (sta)
        {
            struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
            ASSERT((SYS_TYPE)wnet_vif > 0x500);
            if (wnet_vif && wnet_vif->vm_p2p)
            {
                if (P2P_NoA_START_FLAG(wnet_vif->vm_p2p->HiP2pNoaCountNow))
                {
                    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d canceled by noa start\n",__func__,__LINE__);
                    ret = 1;
                }
            }
        }
    }
    while(0);
    return ret;
}
#endif

static void drv_txdesc_list_send (struct drv_private *drv_priv, struct list_head *txdesc_list, int *qcnt)
{
    struct list_head tx_queue, txdesc_list_head;
    struct drv_txdesc *ptxdesc = NULL, *lastbf = NULL;
    struct drv_txlist *txlist = NULL;

    if (*qcnt == 0)
    {
        return;
    }

    INIT_LIST_HEAD(&txdesc_list_head);
    INIT_LIST_HEAD(&tx_queue);

    do
    {
        DRV_TX_QUEUE_LOCK(drv_priv);
        if (list_empty(txdesc_list))
        {
            DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_PWR_SAVE, "%s %d\n",__func__,__LINE__);
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            break;
        }
        ptxdesc = list_first_entry(txdesc_list, struct drv_txdesc, txdesc_queue);

#ifdef CONFIG_P2P
        if (drv_if_noa_started(ptxdesc) >= 0)
        {
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            break;
        }
#endif

        txlist = &drv_priv->drv_txlist_table[ptxdesc->txinfo->queue_id];
        DRV_TXQ_LOCK(txlist);
        ASSERT(txlist->txlist_qnum == ptxdesc->txinfo->queue_id);

        if (aml_tx_hal_buffer_full(drv_priv, txlist->txlist_qnum, 1, 1)==1)
        {
            DRV_TXQ_UNLOCK(txlist);
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            break;
        }

        lastbf = ptxdesc->txdesc_queue_last;
        KASSERT(ptxdesc->txdesc_queue_last, ("ptxdesc->txdesc_queue_last is NULL"));

        //cut lastbf as head of av_mcastq_bhead, and add to head of txdesc_list_head
        list_cut_position(&txdesc_list_head,txdesc_list,&lastbf->txdesc_queue);

        //add head of txdesc_list_head to last of tx_queue, and reinit txdesc_list_head
        list_splice_tail_init(&txdesc_list_head,&tx_queue);
        (*qcnt) --;

        drv_to_hal(drv_priv, txlist, &tx_queue);
        DRV_TXQ_UNLOCK(txlist);
        DRV_TX_QUEUE_UNLOCK(drv_priv);
    }
    while ((*qcnt > 0) && !list_empty(txdesc_list));
}


static void
drv_uapsd_send_qos_null(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta, unsigned char ac)
{
    struct wifi_station * sta = (struct wifi_station *)drv_sta->net_nsta;
    sta->sta_flags &= ~WIFINET_NODE_UAPSD_SP;

    drv_priv->net_ops->wifi_mac_send_qosnulldata(sta, ac);
}

void drv_tx_uapsd_nsta_init (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    drv_txdesc_list_init(&(drv_sta->sta_uapsd_queue), &(drv_sta->sta_uapsd_queuecnt));
}

void drv_tx_uapsd_nsta_cleanup(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    drv_txdesc_list_drain(drv_priv, &(drv_sta->sta_uapsd_queue), &(drv_sta->sta_uapsd_queuecnt), 3);
}

unsigned int drv_tx_uapsd_nsta_qcnt(void * nsta)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);

#ifdef DRV_SUPPORT_TX_WITHDRAW
    return drv_sta->sta_uapsd_queuecnt + drv_sta->sta_txwd_uapsd_qqcnt;
#else
    return drv_sta->sta_uapsd_queuecnt;
#endif
}

void drv_tx_queue_uapsd_nsta(struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct aml_driver_nsta *drv_sta)
{
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d drv_sta->sta_uapsd_queuecnt=%d\n", __func__,__LINE__, drv_sta->sta_uapsd_queuecnt);
    drv_txdesc_list_queue(drv_priv, txdesc_list_head, &(drv_sta->sta_uapsd_queue), &(drv_sta->sta_uapsd_queuecnt));
}

int drv_process_uapsd_nsta_trigger( struct drv_private *drv_priv, void * nsta, unsigned char maxsp, unsigned char ac, unsigned char flush)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_txdesc *ptxdesc = NULL, *lastbf=NULL;
    struct drv_txlist *txlist = drv_priv->drv_uapsdqueue;
    unsigned char count = 0, txn_real = 0, eosp_flag = 0;
    struct list_head tx_queue, txdesc_list_head;
    int uapsd_qqcnt = drv_tx_uapsd_nsta_qcnt(nsta);
    int *qqcnt = NULL;
    struct list_head *list_head = NULL;

    struct wifi_qos_frame *whqos = NULL;


    if (!drv_sta->sta_isuapsd)
    {
        return 0;
    }

    if ((uapsd_qqcnt == 0)
        ||(aml_tx_hal_buffer_full(drv_priv, txlist->txlist_qnum, 1, 1)))
    {
        if (!flush)
            drv_uapsd_send_qos_null(drv_priv, drv_sta, ac);
        return 0;
    }

    INIT_LIST_HEAD(&tx_queue);
    INIT_LIST_HEAD(&txdesc_list_head);
    /*
     * Send all frames
     */
    if (maxsp == WME_UAPSD_NODE_MAXQDEPTH)
        txn_real = uapsd_qqcnt;
    else
    {
        if (maxsp > uapsd_qqcnt)
            txn_real = uapsd_qqcnt;
        else
            txn_real = maxsp;
    }

    pr_debug("before tx uapsd\n");
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d flush=%d uapsd_qqcnt=%d\n", __func__,__LINE__, flush, uapsd_qqcnt);
    for (count = txn_real; ((count > 0) && (eosp_flag==0)); count--)
    {

        pr_debug("tx=%d\n", count);

#ifdef DRV_SUPPORT_TX_WITHDRAW
        if (drv_sta->sta_txwd_uapsd_qqcnt)
        {
            list_head  = &drv_sta->sta_txwd_uapsd_q;
            qqcnt = &drv_sta->sta_txwd_uapsd_qqcnt;
        }
        else
        {
            list_head  = &drv_sta->sta_uapsd_queue;
            qqcnt = &drv_sta->sta_uapsd_queuecnt;
        }
#else
        list_head  = &drv_sta->sta_uapsd_queue;
        qqcnt = &drv_sta->sta_uapsd_queuecnt;
#endif

        DRV_TX_QUEUE_LOCK(drv_priv);
        if (list_empty(list_head))
        {
            DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_PWR_SAVE, "%s %d\n",__func__,__LINE__);
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            break;
        }
        ptxdesc = list_first_entry(list_head, struct drv_txdesc, txdesc_queue);

#ifdef CONFIG_P2P
        if (drv_if_noa_started(ptxdesc) >= 0)
        {
            struct wifi_station *sta = (struct wifi_station *)(drv_sta->net_nsta);
            sta->sta_flags_ext |= WIFINET_NODE_UAPSD_WAIT_NOA_END;
            if (flush)
            {
                sta->sta_flags_ext |= WIFINET_NODE_UAPSD_FLUSH_WAIT_NOA_END;
            }
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            break;
        }
#endif

        //test by wp
        DRV_TXQ_LOCK(txlist);
        ASSERT(ptxdesc->txinfo->queue_id== txlist->txlist_qnum);
        if (aml_tx_hal_buffer_full(drv_priv,txlist->txlist_qnum, 1, 1))
        {
            DRV_TXQ_UNLOCK(txlist);
            DRV_TX_QUEUE_UNLOCK(drv_priv);
            DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_PWR_SAVE, "%s %d\n",__func__,__LINE__);
            break;
        }

        lastbf = ptxdesc->txdesc_queue_last;
        KASSERT(ptxdesc->txdesc_queue_last, ("ptxdesc->txdesc_queue_last is NULL"));
        whqos = (struct wifi_qos_frame *)os_skb_data(lastbf->txdesc_mpdu);

        list_cut_position(&txdesc_list_head,list_head,&lastbf->txdesc_queue);
        list_splice_tail_init(&txdesc_list_head,&tx_queue);
        (*qqcnt)--;
        /*
         * Mark EOSP flag at the last frame in this SP
         */

        if (!flush)
        {
         #define UAPSD_BRN   (2)
            if ((count == 1) ||
                ((count > 1) &&
                 aml_tx_hal_buffer_full(drv_priv, txlist->txlist_qnum, (UAPSD_BRN+1), (UAPSD_BRN+1))))
            {
                whqos->i_qos[0] |= WIFINET_QOS_EOSP;
                eosp_flag = 1;
            }
        }

        /*
         * Clear More data bit for EOSP frame if we have
         * no pending frames
         */
        if (drv_tx_uapsd_nsta_qcnt(nsta) == 0)
        {
            whqos->i_fc[1] &= ~WIFINET_FC1_MORE_DATA;
        }

        lastbf->txdesc_framectrl = *(unsigned short*)&whqos->i_fc[0];
        drv_to_hal(drv_priv, txlist, &tx_queue);
        DRV_TXQ_UNLOCK(txlist);
        DRV_TX_QUEUE_UNLOCK(drv_priv);
    }

    if (flush && (drv_tx_uapsd_nsta_qcnt(nsta) > 0))
    {
        DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d sta_uapsd_queuecnt=%d, need to flush another time\n",
                __func__,__LINE__, drv_tx_uapsd_nsta_qcnt(nsta));
    }
    return drv_tx_uapsd_nsta_qcnt(nsta);
}


#ifdef AML_MCAST_QUEUE

/* setup multicast queue */
void drv_tx_mcastq_init (struct drv_private *drv_priv, int wnet_vif_id)
{
    struct wlan_net_vif *wnet_vif = drv_priv->drv_wnet_vif_table[wnet_vif_id];
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[HAL_WME_MCAST];
    drv_txdesc_list_init(&txlist->txlist_backup, &txlist->txlist_backup_qcnt);
    wnet_vif->vm_mqueue_flag_send = 0;
}

void drv_tx_mcastq_cleanup (struct drv_private *drv_priv, int wnet_vif_id)
{
    struct wlan_net_vif *wnet_vif = drv_priv->drv_wnet_vif_table[wnet_vif_id];
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[HAL_WME_MCAST];
    drv_txdesc_list_drain(drv_priv, &txlist->txlist_backup, &txlist->txlist_backup_qcnt, wnet_vif_id);
    wnet_vif->vm_mqueue_flag_send = 0;
}

//when multicast, put it into Tid8
void drv_tx_mcastq_addbuf(struct drv_private *drv_priv, struct list_head *head)
{
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[HAL_WME_MCAST];
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d txlist->txlist_backup_qcnt=%d\n", __func__,__LINE__, txlist->txlist_backup_qcnt);
    drv_txdesc_list_queue(drv_priv, head, &txlist->txlist_backup, &txlist->txlist_backup_qcnt);
}

int drv_tx_mcastq_send (struct drv_private *drv_priv)
{
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[HAL_WME_MCAST];
    int *qcnt = &txlist->txlist_backup_qcnt;
    struct list_head tx_queue, *wnet_vif_mcast_q = &txlist->txlist_backup, *bf_qnode = NULL;
    struct drv_txdesc *ptxdesc = NULL;
    struct sk_buff * skbbuf = NULL;
    struct wifi_skb_callback *cb = NULL;
    struct wifi_station *sta = NULL;
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d txlist->txlist_backup_qcnt=%d\n", __func__,__LINE__, txlist->txlist_backup_qcnt);

    DRV_TX_QUEUE_LOCK(drv_priv);

    if (*qcnt == 0) {
        DRV_TX_QUEUE_UNLOCK(drv_priv);
        return 0;
    }

    if (list_empty(wnet_vif_mcast_q))
    {
        DRV_TX_QUEUE_UNLOCK(drv_priv);
        DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_PWR_SAVE, "%s %d\n",__func__,__LINE__);
        return 0;
    }

    bf_qnode = wnet_vif_mcast_q->next;
    INIT_LIST_HEAD(&tx_queue);

    do
    {
        struct list_head *curr_next = NULL;

        ptxdesc = list_entry(bf_qnode, struct drv_txdesc, txdesc_queue);
        skbbuf = ptxdesc->txdesc_mpdu;
        cb = (struct wifi_skb_callback *)skbbuf->cb;
        sta = cb->sta;
        if (sta)
        {
            struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
            if (txlist->txlist_backup_qcnt && !wnet_vif->vm_ps_sta) //as ap, if has no ps sta, then send bufferred mcast packets
            {
#ifdef CONFIG_P2P
                if (drv_if_noa_started(ptxdesc) >= 0) {
                    break;
                }
#endif
                DRV_TXQ_LOCK(txlist);

                ASSERT(txlist->txlist_qnum == ptxdesc->txinfo->queue_id);
                if (aml_tx_hal_buffer_full(drv_priv, txlist->txlist_qnum, 1, 1)==1)
                {
                    DRV_TXQ_UNLOCK(txlist);
                    break;
                }

                curr_next = ptxdesc->txdesc_queue.next;
                list_del_init(&ptxdesc->txdesc_queue);
                list_add_tail(&ptxdesc->txdesc_queue,&tx_queue);
                (*qcnt) --;

                drv_to_hal(drv_priv, txlist, &tx_queue);
                DRV_TXQ_UNLOCK(txlist);
            }
        }

        /* If element is still in list, go to next. */
        if (!curr_next)
            bf_qnode = bf_qnode->next;
        else
            bf_qnode = curr_next;

        if (bf_qnode == wnet_vif_mcast_q)
            break;
    }
    while ((*qcnt > 0) && !list_empty(wnet_vif_mcast_q));

    DRV_TX_QUEUE_UNLOCK(drv_priv);

    return 0;
}

#endif

#ifdef DRV_SUPPORT_TX_WITHDRAW

void drv_tx_withdraw_init (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    drv_txdesc_list_init(&(drv_sta->sta_txwd_uapsd_q), &(drv_sta->sta_txwd_uapsd_qqcnt));
    drv_txdesc_list_init(&(drv_sta->sta_txwd_legacyps_q), &(drv_sta->sta_txwd_legacyps_qqcnt));
}

void drv_tx_withdraw_cleanup (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    drv_txdesc_list_drain(drv_priv, &(drv_sta->sta_txwd_uapsd_q), &(drv_sta->sta_txwd_uapsd_qqcnt), 3);
    drv_txdesc_list_drain(drv_priv, &(drv_sta->sta_txwd_legacyps_q), &(drv_sta->sta_txwd_legacyps_qqcnt), 3);
}

void drv_tx_withdraw_uapsd_addbuf(struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct aml_driver_nsta *drv_sta)
{
    drv_txdesc_list_queue(drv_priv, txdesc_list_head, &(drv_sta->sta_txwd_uapsd_q), &(drv_sta->sta_txwd_uapsd_qqcnt));
}

void drv_tx_withdraw_legacyps_addbuf(struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct aml_driver_nsta *drv_sta)
{
    drv_txdesc_list_queue(drv_priv, txdesc_list_head, &(drv_sta->sta_txwd_legacyps_q), &(drv_sta->sta_txwd_legacyps_qqcnt));
}

int drv_tx_withdraw_legacyps_send (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    int legacyps_qqcnt_init, legacyps_qqcnt_now;
    int tx_frames = 0;

    if (!drv_sta->sta_txwd_legacyps_qqcnt)
    {
        return tx_frames;
    }

    legacyps_qqcnt_init = legacyps_qqcnt_now = 1;

    /* tx one frame */
    drv_txdesc_list_send(drv_priv, &(drv_sta->sta_txwd_legacyps_q), &legacyps_qqcnt_now);
    tx_frames = legacyps_qqcnt_init - legacyps_qqcnt_now;
    drv_sta->sta_txwd_legacyps_qqcnt -= tx_frames;
    return tx_frames;
}

#endif

void drv_tx_bk_list_init (struct drv_txlist *txlist)
{
    drv_txdesc_list_init(&txlist->txlist_backup, &txlist->txlist_backup_qcnt);
}

/* release all backup buf for the queue */
void drv_txq_backup_drain (struct drv_private *drv_priv, struct drv_txlist *txlist, unsigned char vid)
{
    drv_txdesc_list_drain(drv_priv, &txlist->txlist_backup, &txlist->txlist_backup_qcnt, vid);
}

/* release backup buf for the drv_sta */
void drv_txq_backup_cleanup (struct drv_private *drv_priv, struct drv_txlist *txlist, struct aml_driver_nsta *drv_sta)
{
    struct drv_txdesc *ptxdesc = NULL, *txdesc_queue_next = NULL;
    struct aml_driver_nsta *sta;

    DRV_TX_QUEUE_LOCK(drv_priv);
    DRV_TXQ_LOCK(txlist);
    list_for_each_entry_safe(ptxdesc,txdesc_queue_next,&txlist->txlist_backup,txdesc_queue) {
        sta = (struct aml_driver_nsta *)ptxdesc->txdesc_sta;
        if (sta != drv_sta) {
            continue;
        }

        drv_txdesc_list_cleanup(drv_priv, &txlist->txlist_backup_qcnt, ptxdesc, txlist);
    }
    DRV_TXQ_UNLOCK(txlist);
    DRV_TX_QUEUE_UNLOCK(drv_priv);
}

int drv_txq_backup_qcnt(struct drv_txlist *txlist)
{
    return txlist->txlist_backup_qcnt;
}

void drv_txq_backup_addbuf(struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct drv_txlist *txlist)
{
    drv_txdesc_list_queue(drv_priv, txdesc_list_head, &txlist->txlist_backup, &txlist->txlist_backup_qcnt);
}

int drv_txq_backup_send (struct drv_private *drv_priv, struct drv_txlist *txlist)
{
    drv_txdesc_list_send(drv_priv, &txlist->txlist_backup, &txlist->txlist_backup_qcnt);
    return 0;
}

