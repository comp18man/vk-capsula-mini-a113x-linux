/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     driver layer send frame module
 *
 *
 ****************************************************************************************
 */
#include "wifi_mac_com.h"
#include "wifi_drv_uapsd.h"
#include "wifi_hal.h"
#include "wifi_drv_power.h"
#include "wifi_drv_xmit.h"

static int drv_tx_normal(struct drv_private *drv_priv, struct drv_txlist *txlist, struct list_head *txdesc_list_head, unsigned char vid);
static  int drv_tx_send_ampdu(struct drv_private *drv_priv, struct drv_txlist *txlist, struct drv_tx_scoreboard *tid, struct list_head *txdesc_list_head);
static void drv_tx_update_ba_win(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid, int seqnum, struct drv_txlist *txlist);
static void drv_txlist_push_pending(struct drv_private *drv_priv, struct drv_txlist *txlist, unsigned char vid);
static  void drv_tx_queue_tid(struct drv_txlist *txlist, struct drv_tx_scoreboard *tid);
static void drv_txlist_pause_for_sta_tid(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid);
static void drv_txlist_resume_for_sta_tid(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid);
static void drv_txlist_flush_for_sta_tid(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid);
static void drv_txampdu_tasklet(unsigned long arg);
static void drv_tx_sched_aggr(struct drv_private *drv_priv, struct drv_txlist *txlist, struct drv_tx_scoreboard *tid);
extern struct tasklet_struct amsdu_tasklet;
extern void wifi_mac_tx_lock_timer_attach(void);
extern void wifi_mac_tx_lock_timer_cancel(void);

static unsigned int
drv_tx_ack_optimize(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid, struct drv_txdesc *ptxdesc_last)
{
    struct list_head *pkt_pending_queue = &tid->txds_queue;
    struct drv_txdesc *ptxdesc;
    unsigned int counts = 0;
    struct wifi_mac_pkt_info *mac_pkt_info;
    struct wifi_mac_pkt_info *mac_pkt_info_last = &ptxdesc_last->drv_pkt_info.pkt_info[0];

    if (!mac_pkt_info_last->b_tcp_ack_del)
        return counts;

    list_for_each_entry(ptxdesc, pkt_pending_queue, txdesc_queue)
    {
        mac_pkt_info = &ptxdesc->drv_pkt_info.pkt_info[0];

        if (mac_pkt_info->b_tcp_ack_del && (mac_pkt_info->tcp_src_port == mac_pkt_info_last->tcp_src_port)
            && (mac_pkt_info->tcp_dst_port == mac_pkt_info_last->tcp_dst_port)) {

            if (mac_pkt_info->eat_count < drv_priv->drv_config.cfg_eat_count_max) {
                struct wifi_frame *wh = (struct wifi_frame *)os_skb_data(ptxdesc_last->txdesc_mpdu);

                ptxdesc_last->txinfo->seqnum = ptxdesc->txinfo->seqnum;
                CIRCLE_Sub_One(tid->seq_next, WIFINET_SEQ_MAX);
                *(unsigned short*)wh->i_seq = htole16(ptxdesc_last->txinfo->seqnum << WIFINET_SEQ_SEQ_SHIFT);

                mac_pkt_info_last->eat_count = mac_pkt_info->eat_count + 1;
                list_move(&ptxdesc_last->txdesc_queue, &ptxdesc->txdesc_queue);
                list_del_init(&ptxdesc->txdesc_queue);
                wifi_mac_free_skb(ptxdesc->txdesc_mpdu);
                counts++;
                break;
            }
            else {
                break;
            }
        }
    }
    return counts;
}

static void drv_retransmit_tasklet(unsigned long arg)
{
    struct drv_private *drv_priv = (struct drv_private *)arg;

    wifi_mac_buffer_txq_send(&drv_priv->retransmit_queue);
}

int drv_txlist_cleanup( struct drv_private *drv_priv)
{
    driv_ps_wakeup(drv_priv);
    drv_hal_tx_frm_pause(drv_priv, 0);
    os_timer_ex_del(&drv_priv->drv_txtimer, CANCEL_SLEEP);

    drv_txlist_flushfree(drv_priv, 3);
    tasklet_kill(&drv_priv->ampdu_tasklet);
    tasklet_kill(&drv_priv->retransmit_tasklet);
    WIFINET_SAVEQ_DESTROY(&drv_priv->retransmit_queue);
    driv_ps_sleep(drv_priv);

    return 0;
}

int drv_tx_init( struct drv_private *drv_priv, int nbufs)
{
    int error = 0;

    DRV_TX_NORMAL_BUF_LOCK_INIT(drv_priv);
    DRV_TX_QUEUE_LOCK_INIT(drv_priv);
    DRV_TX_TIMEOUT_LOCK_INIT(drv_priv);
    DRV_HRTIMER_LOCK_INIT(drv_priv);

    INIT_LIST_HEAD(&drv_priv->drv_normal_buffer_queue[0]);
    INIT_LIST_HEAD(&drv_priv->drv_normal_buffer_queue[1]);
    drv_priv->drv_normal_buffer_count[0] = 0;
    drv_priv->drv_normal_buffer_count[1] = 0;

    drv_priv->drv_ampdu_buffer_count[0] = 0;
    drv_priv->drv_ampdu_buffer_count[1] = 0;

    tasklet_init(&drv_priv->ampdu_tasklet, drv_txampdu_tasklet, (unsigned long)(SYS_TYPE)drv_priv);
    tasklet_init(&drv_priv->retransmit_tasklet, drv_retransmit_tasklet, (unsigned long)(SYS_TYPE)drv_priv);
    WIFINET_SAVEQ_INIT(&(drv_priv->retransmit_queue), "retransmit_queue");
    return error;
}

struct drv_txlist *drv_txlist_initial(struct drv_private *drv_priv, int queue_id)
{
    if (!DRV_TXQUEUE_VALUE(drv_priv, queue_id))
    {
        struct drv_txlist *txlist = &drv_priv->drv_txlist_table[queue_id];

        txlist->txlist_qnum = queue_id;
        INIT_LIST_HEAD(&txlist->txlist_q);
        INIT_LIST_HEAD(&txlist->txlist_acq);
        DRV_TXQ_LOCK_INIT(txlist);
        INIT_LIST_HEAD(&txlist->tx_tcp_queue);
        DRV_TX_BACKUPQ_LOCK_INIT(txlist);
        txlist->txlist_qcnt = 0;

        drv_priv->drv_txqueue_map |= 1<<queue_id;
        drv_tx_bk_list_init(txlist);
    }
    return &drv_priv->drv_txlist_table[queue_id];
}

void drv_txlist_destroy(struct drv_private *drv_priv, struct drv_txlist *txlist)
{
    DRV_TXQ_LOCK_DESTROY(txlist);
    drv_priv->drv_txqueue_map &= ~(1<<txlist->txlist_qnum);
}

int drv_txlist_setup(struct drv_private *drv_priv)
{
    int i;
    for (i=HAL_WME_MIN; i<HAL_WME_MAX; i++)
    {
        if (i == HAL_WME_MGT)
        {
            drv_priv->drv_mgmtxqueue = drv_txlist_initial(drv_priv,i);
        }
        else if (i == HAL_WME_NOQOS)
        {
            drv_priv->drv_noqosqueue = drv_txlist_initial(drv_priv,i);
        }
        else if (i == HAL_WME_UAPSD)
        {
            drv_priv->drv_uapsdqueue = drv_txlist_initial(drv_priv,i);
        }

        else if (i == HAL_WME_MCAST)
        {
            drv_priv->drv_mcasequeue = drv_txlist_initial(drv_priv,i);
        }
        else
        {
            drv_txlist_initial(drv_priv,i);
        }
    }
    return 1;
}



int drv_update_wmmq_param(struct drv_private *drv_priv,
    unsigned char wnet_vif_id,int ac, int aifs,
    int cwmin,int cwmax,int txoplimit)
{

    int error = 0;

    driv_ps_wakeup(drv_priv);
    drv_hal_settxqueueprops( wnet_vif_id,ac,  aifs,(cwmin|(cwmax<<4)),txoplimit);

    driv_ps_sleep(drv_priv);
    return error;
}

void drv_set_protmode( struct drv_private *drv_priv, enum prot_mode mode)
{
    drv_priv->drv_protect_mode = mode;
}

static unsigned char aml_convert_tid(struct drv_txdesc *ptxdesc)
{
    unsigned char TID;
    if (ptxdesc->txinfo->b_mgmt )
        TID = QUEUE_MANAGE;
    else if (ptxdesc->txinfo->b_PsPoll)
        TID = QUEUE_MANAGE;
    else if (ptxdesc->txinfo->b_uapsd )
        TID = QUEUE_MANAGE;
    else if (ptxdesc->txinfo->b_qosdata)
    {
        TID = ptxdesc->txinfo->tid_index;
    }
    else
        TID = QUEUE_NO_QOS;
    return TID;
}



// if there is free buf in TxShareFifoBuf
int aml_tx_hal_buffer_full(struct drv_private *drv_priv,
    unsigned char queue_id,int txaggrneed,int txprivneed)
{
    int txpriv_qspace = 0;
    static unsigned char print_cnt = 0;

    txpriv_qspace = drv_priv->hal_priv->hal_ops.hal_get_priv_cnt(queue_id);
    txpriv_qspace = txpriv_qspace >= 2 ? txpriv_qspace - 1 : 0;

    //not full
    if (txpriv_qspace >= txprivneed) {
        return 0;

    } else {
        if (print_cnt++ == 200) {
            pr_debug("%s, queue_id:%d full\n", __func__, queue_id);
        }
    }

    //full
    return 1;
}

static void aml_prepare_agg_tx_priv_param(struct drv_private *drv_priv,
    struct hi_agg_tx_desc *agg_content, struct drv_txdesc *ptxdesc, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = drv_priv->wmac;
    static unsigned int cnt = 0;

    cnt++;

    agg_content->CurrentRate = ptxdesc->txdesc_rateinfo[0].vendor_rate_code ;//& RATE_MASK;//WIFI_11N_MCS3

    agg_content->TxTryRate1 =  ptxdesc->txdesc_rateinfo[1].vendor_rate_code ;//& RATE_MASK;
    agg_content->TxTryRate2 =  ptxdesc->txdesc_rateinfo[2].vendor_rate_code ;//& RATE_MASK;
    agg_content->TxTryRate3 =  ptxdesc->txdesc_rateinfo[3].vendor_rate_code ;//& RATE_MASK;

    agg_content->TxTryNum0 = ptxdesc->txdesc_rateinfo[0].trynum;
    agg_content->TxTryNum1 = ptxdesc->txdesc_rateinfo[1].trynum;
    agg_content->TxTryNum2 = ptxdesc->txdesc_rateinfo[2].trynum;
    agg_content->TxTryNum3 = ptxdesc->txdesc_rateinfo[3].trynum;

    agg_content->EncryptType = (ptxdesc->txinfo->key_type == HAL_KEY_TYPE_WEP )&& \
               ((sta->sta_ucastkey.wk_keylen == 13 )|| \
                (sta->sta_ucastkey.wk_keylen == 26))? HAL_KEY_TYPE_WEP128 :ptxdesc->txinfo->key_type;

    agg_content->StaId = ( sta->sta_wnet_vif->vm_opmode == WIFINET_M_STA )? 1:sta->sta_associd;
    agg_content->KeyId = ptxdesc->txinfo->key_index;
    agg_content->TID = aml_convert_tid(ptxdesc);
    agg_content->queue_id = ptxdesc->txinfo->queue_id;

    if (ptxdesc->txinfo->b_noack) {
        agg_content->FLAG |= WIFI_IS_NOACK;
    }

    if (ptxdesc->txinfo->b_pmf) {
	agg_content->FLAG |= WIFI_IS_PMF;
    }

    if (ptxdesc->txinfo->b_mcast) {
        agg_content->FLAG |= WIFI_IS_Group;

    } else {
        if (ptxdesc->txinfo->b_Ampdu) {
            agg_content->FLAG |= WIFI_IS_BLOCKACK ;
            agg_content->FLAG |= WIFI_IS_AGGR;
            if (drv_priv->drv_config.cfg_burst_ack) {
                agg_content->FLAG |= WIFI_IS_BURST;
            }

        } else {
            if (!IS_11B_RATE(agg_content->CurrentRate) && (drv_priv->drv_config.cfg_burst_ack) &&(ptxdesc->txinfo->b_datapkt)
                && !(M_PWR_SAV_GET((ptxdesc->txdesc_mpdu)) || M_FLAG_GET((ptxdesc->txdesc_mpdu), M_UAPSD))
                && !(ptxdesc->txinfo->b_mcast)) {
                agg_content->FLAG |= WIFI_IS_BURST;
            }
        }
    }
    //protect

    if (WIFI_MCS_RATE(agg_content->CurrentRate))
    {
        if ((sta->sta_wnet_vif->vm_flags & WIFINET_F_RDG) && (ptxdesc->txinfo->b_datapkt))
        {
            agg_content->FLAG2 |= TX_DESCRIPTER_HTC;
        }
        if (sta->sta_flags_ext&WIFINET_NODE_RDG)
        {
            agg_content->FLAG2 |=  TX_DESCRIPTER_RD_SUPPORT;
        }
        if ( ( wifimac->wm_flags_ext &WIFINET_FEXT_SHORTGI20 ) && ( ptxdesc->txdesc_chanbw == CHAN_BW_20M) )
        {
            agg_content->FLAG |= WIFI_IS_SHORTGI;
        }

        if ( ( wifimac->wm_flags_ext &WIFINET_FEXT_SHORTGI40 ) && ( ptxdesc->txdesc_chanbw == CHAN_BW_40M) )
        {
            agg_content->FLAG |= WIFI_IS_SHORTGI;
        }

        if ((sta->sta_vhtcap & WIFINET_VHTCAP_SHORTGI_80) && (ptxdesc->txdesc_chanbw == CHAN_BW_80M))
        {
            if (ptxdesc->txdesc_rateinfo[0].shortgi_en)
                agg_content->FLAG |= WIFI_IS_SHORTGI;

            if (ptxdesc->txdesc_rateinfo[1].shortgi_en)
                agg_content->FLAG |= WIFI_IS_SHORTGI1;

            if (ptxdesc->txdesc_rateinfo[2].shortgi_en)
                agg_content->FLAG |= WIFI_IS_SHORTGI2;
        }
    }

    if ((ptxdesc->txinfo->b_rsten)
        || ((agg_content->FLAG & WIFI_IS_BURST) && (wifimac->wm_protmode == WIFINET_PROT_RTSCTS)))
    {
        agg_content->FLAG |=  WIFI_IS_RTSEN;
    }
    if ((ptxdesc->txinfo->b_csten)
        || ((agg_content->FLAG & WIFI_IS_BURST) && (wifimac->wm_protmode == WIFINET_PROT_CTSONLY)))
    {
        agg_content->FLAG |=  WIFI_IS_CTSEN;
    }

    agg_content->MpduNum = ptxdesc->txdesc_pktnum;
    agg_content->AGGR_len = ptxdesc->txdesc_agglen;
    agg_content->aggr_page_num =  ptxdesc->txdesc_aggr_page_num;
    DPRINTF(AML_DEBUG_XMIT,"b_mcast %d b_mgmt %d\n",ptxdesc->txinfo->b_mcast,ptxdesc->txinfo->b_mgmt);
    if (ptxdesc->txinfo->b_mcast || ptxdesc->txinfo->b_mgmt)
    {
        agg_content->FLAG |= ptxdesc->txdesc_flag<<WIFI_CHANNEL_BW_OFFSET ;
    }
    else
    {
        agg_content->FLAG |= ptxdesc->txdesc_chanbw<<WIFI_CHANNEL_BW_OFFSET ;
    }

     if( drv_priv->drv_config.cfg_dynamic_bw &&  IS_VHT_RATE(agg_content->CurrentRate ) )
    {
        agg_content->FLAG |= WIFI_IS_DYNAMIC_BW;
    }
    if (ptxdesc->txinfo->b_hwchecksum)
    {
        agg_content->FLAG2 |= TX_DESCRIPTER_CHECKSUM;
    }

    if (ptxdesc->txinfo->b_mgmt
        &&( (( ptxdesc->txdesc_framectrl &WIFINET_FC0_SUBTYPE_MASK)==WIFINET_FC0_SUBTYPE_BEACON)
            ||(( ptxdesc->txdesc_framectrl &WIFINET_FC0_SUBTYPE_MASK)==WIFINET_FC0_SUBTYPE_PROBE_RESP)))
    {
        agg_content->FLAG2 |= TX_DESCRIPTER_BEACON;
    }

#ifdef CONFIG_P2P
    /* legacy ps and uapsd both are unicast, mcast here is for code safe */
    if ((M_PWR_SAV_GET((struct sk_buff *)(ptxdesc->txdesc_mpdu)) 
        || M_FLAG_GET((struct sk_buff *)(ptxdesc->txdesc_mpdu), M_UAPSD))
        && !(ptxdesc->txinfo->b_mcast))
    {
        struct wifi_mac_p2p *p2p = sta->sta_wnet_vif->vm_p2p;
        if (p2p->p2p_flag & P2P_NOA_START_FLAG_HI)
        {
            agg_content->FLAG2 |= TX_DESCRIPTER_P2P_PS_NOA_TRIGRSP;
            agg_content->HiP2pNoaCountNow = p2p->HiP2pNoaCountNow;
        }
    }
#endif
    if (ptxdesc->txinfo->b_hwtkipmic)
    {
        agg_content->FLAG2 |= TX_DESCRIPTER_MIC;
    }

    if( (sta->sta_flags&WIFINET_NODE_LDPC)  &&  IS_MCS_RATE(agg_content->CurrentRate )  )
    {
        agg_content->FLAG2 |= TX_DESCRIPTER_ENABLE_TX_LDPC;
    }

    agg_content->vid= ptxdesc->txinfo->wnet_vif_id;
}

static void drv_init_txpriv (struct hi_tx_priv_hdr * txpriv_content, struct drv_txdesc *ptxdesc)
{
    txpriv_content->DMAADDR = (SYS_TYPE)ptxdesc->txdesc_mpdu;
    txpriv_content->DDRADDR = ptxdesc->txdesc_ddraddr;
    txpriv_content->DMALEN =ptxdesc->txinfo->mpdulen;
    txpriv_content->MPDULEN = ptxdesc->txinfo->mpdulen;
    txpriv_content->Delimiter = ptxdesc->txdesc_delimit;
    txpriv_content->Seqnum = ptxdesc->txinfo->seqnum;


    txpriv_content->hostreserve = (SYS_TYPE)ptxdesc;
    txpriv_content->FrameControl = ptxdesc->txdesc_framectrl;
    txpriv_content->Flag = WIFI_MORE_AGG|WIFI_FIRST_BUFFER;
}

 //in the three-demo fifo and txlist_q which indexed by id from drv_list[0..7]
int drv_to_hal(struct drv_private *drv_priv, struct drv_txlist *txlist, struct list_head *head)
{
    struct drv_txdesc *ptxdesc, *first;
    struct wifi_qos_frame *qh;
    struct aml_ratecontrol *rateinfo;
    struct aml_driver_nsta *drv_sta;
    struct wifi_station *sta;
    struct hi_agg_tx_desc agg_content;
    struct hi_tx_priv_hdr txpriv_content;
    int mpdunum = 1;
    struct list_head *txdesc_queue = NULL;
    struct hal_private *hal_priv = NULL;

    hal_priv = hal_get_priv();
    if (list_empty(head)) {
        return -1;
    }
    ptxdesc = list_first_entry(head, struct drv_txdesc, txdesc_queue);
    first = ptxdesc;
    drv_sta = (struct aml_driver_nsta *)ptxdesc->txdesc_sta;
    sta = (struct wifi_station *)drv_sta->net_nsta;
    ASSERT(txlist->txlist_qnum == ptxdesc->txinfo->queue_id);

    if (aml_tx_hal_buffer_full(drv_priv, txlist->txlist_qnum, 1, (ptxdesc->txdesc_pktnum )) !=0 ) {
        return -1;
    }
    qh = (struct wifi_qos_frame *)ptxdesc->txdesc_ddraddr;

    //rate & retry
    rateinfo = &ptxdesc->txdesc_rateinfo[0];
    ptxdesc->txdesc_chanbw = rateinfo->bw;
    memset(&agg_content,0,sizeof(agg_content));
    memset(&txpriv_content,0,sizeof(txpriv_content));

    aml_prepare_agg_tx_priv_param(drv_priv, &agg_content, ptxdesc, sta);

    if (ptxdesc->txinfo->b_Ampdu)
    {
        agg_content.FLAG |= WIFI_IS_BLOCKACK ;
        agg_content.FLAG |= WIFI_IS_AGGR;
        ASSERT(WIFINET_QOS_HAS_SEQ(qh));
    }

    drv_init_txpriv(&txpriv_content, ptxdesc);
    txpriv_content.EncryptType = agg_content.EncryptType;
    if ((ptxdesc->txinfo->b_Ampdu == 0)||(ptxdesc->txdesc_pktnum == 1))
    {
        txpriv_content.Flag &= ~WIFI_MORE_AGG;
    }

    drv_priv->hal_priv->hal_ops.hal_fill_agg_start(&agg_content, &txpriv_content);

   //loop the residue mpdus for ampdu
    do
    {
        if (list_is_last(&ptxdesc->txdesc_queue,head))
        {
            break;
        }
        txdesc_queue = ptxdesc->txdesc_queue.next;

#ifdef DRV_TCP_RETRANSMISSION
        if ((ptxdesc->drv_pkt_info.pkt_info[0].b_tcp) && (!ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_saved_flag)) {
            list_del_init(&ptxdesc->txdesc_queue);
            list_add_tail(&ptxdesc->txdesc_queue, &txlist->tx_tcp_queue);
            ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_saved_flag=1;
        }
#endif
        ptxdesc = list_entry(txdesc_queue, struct drv_txdesc, txdesc_queue);
        mpdunum++;
        qh = (struct wifi_qos_frame *) ptxdesc->txdesc_ddraddr;
        //rate & retry
        memcpy(&ptxdesc->txdesc_rateinfo[0], rateinfo, 4*sizeof(struct aml_ratecontrol));
        ptxdesc->txdesc_chanbw = rateinfo->bw;
        drv_init_txpriv(&txpriv_content, ptxdesc);

        if (list_is_last(&ptxdesc->txdesc_queue,head)) {
            txpriv_content.Flag &= ~WIFI_MORE_AGG;
            ASSERT(mpdunum == agg_content.MpduNum);
        }

        drv_priv->hal_priv->hal_ops.hal_fill_priv(&txpriv_content, agg_content.queue_id);

    }
    while (1);

#ifdef DRV_TCP_RETRANSMISSION
    if ((ptxdesc->drv_pkt_info.pkt_info[0].b_tcp) && (!ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_saved_flag)) {
        list_del_init(&ptxdesc->txdesc_queue);
        list_add_tail(&ptxdesc->txdesc_queue, &txlist->tx_tcp_queue);
        ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_saved_flag=1;
    }
#endif

    up(&hal_priv->hi_irq_thread_sem);

    /*Saving txdesc which are sent to hal layer.  And will be freed when txcomplete,
    so we need to protect txlist_q.*/
    if (!list_empty(head))
    {
        txlist->txlist_qcnt += list_first_entry(head, struct drv_txdesc, txdesc_queue)->txdesc_pktnum;
        list_splice_tail_init((head), &txlist->txlist_q);
    }

    if (hal_priv->bhaltxdrop || hal_priv->bhalPowerSave) {
        pr_debug("%s hal_priv->bhaltxdrop:%d\n", __func__, hal_priv->bhaltxdrop);
    }
    return 0;
}

int drv_rate_findindex_from_ratecode(const struct drv_rate_table *rt, int vendor_rate_code)
{
    int i;

    for (i = 0; i < rt->rateCount; i++) {
        if (rt->info[i].vendor_rate_code == vendor_rate_code) {
            break;
        }
    }

    return i;
}

static  int
drv_aggr_query(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta, struct sk_buff * skb)
{
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);
    struct drv_tx_scoreboard *tid;
    tid = DRV_GET_TIDTXINFO(drv_sta, txinfo->tid_index);

    //if ps, disable agg
    if (M_PWR_SAV_GET(skb) || M_FLAG_GET(skb, M_UAPSD)) {
        return 0;
    }

    if (tid->cleanup_inprogress) {
        return 0;
    }

    if (tid->addba_exchangecomplete) {
        return 1;
    }

    return 0;
}

int drv_tx_get_mgmt_frm_rate(struct drv_private *drv_priv,
                    struct wlan_net_vif *wnet_vif,
                    unsigned char fc_type,unsigned char *rate,unsigned short *flag );

static void drv_tx_get_spec_frm_rate(struct drv_private *drv_priv, struct sk_buff *skbbuf,
    struct drv_txdesc *ptxdesc, struct aml_ratecontrol *ratectrl, unsigned char retry_times)
{
    struct wifi_frame *wh = (struct wifi_frame *)os_skb_data(skbbuf);
    struct wifi_mac_tx_info *txinfo = ptxdesc->txinfo;
    const struct drv_rate_table *rt = drv_priv->drv_currratetable;
    struct wifi_station *sta = (struct wifi_station *)txinfo->cb.sta;

    drv_tx_get_mgmt_frm_rate(drv_priv, sta->sta_wnet_vif, wh->i_fc[0], &ratectrl[0].vendor_rate_code, &ptxdesc->txdesc_flag);
    ratectrl[0].rate_index = drv_rate_findindex_from_ratecode(rt, ratectrl[0].vendor_rate_code);
    ratectrl[1].vendor_rate_code = ratectrl[2].vendor_rate_code = ratectrl[3].vendor_rate_code = ratectrl[0].vendor_rate_code;
    ratectrl[1].bw = ratectrl[2].bw = ratectrl[3].bw = ratectrl[0].bw = sta->sta_chbw;
    ratectrl[0].trynum = retry_times;
}

static void drv_tx_lower_rate_when_signal_weak(struct wlan_net_vif *wnet_vif, struct drv_txdesc *ptxdesc)
{
    struct wifi_station *sta = wnet_vif->vm_mainsta;
    unsigned int sta_chbw = sta->sta_chbw;
    unsigned short rx_speed = wnet_vif->vm_rx_speed;
    unsigned int rate = 0, tcp_rx = ptxdesc->drv_pkt_info.pkt_info[0].b_tcp_ack;
    int i, power, power_weak_thresh = wnet_vif->vm_wmac->wm_signal_power_weak_thresh_narrow;
    unsigned char mcs_bw = 0;

    if (!IS_MCS_RATE(ptxdesc->txdesc_rateinfo[0].vendor_rate_code))
        return;

    if ((wnet_vif->vm_curchan != WIFINET_CHAN_ERR) && (WIFINET_IS_CHAN_5GHZ(wnet_vif->vm_curchan)))
        power_weak_thresh = wnet_vif->vm_wmac->wm_signal_power_weak_thresh_wide;

    power = wnet_vif->vm_mainsta->sta_avg_bcn_rssi;
    if (power < power_weak_thresh) {
        if ((wnet_vif->vm_curchan != WIFINET_CHAN_ERR) && (WIFINET_IS_CHAN_2GHZ(wnet_vif->vm_curchan))) {
            if (sta_chbw == WIFINET_BWC_WIDTH20) {
                if (tcp_rx) {
                    rate = (power + 71 > 0) ? WIFI_11N_MCS4 : (power + 78 > 0) ? WIFI_11N_MCS3 : (power + 80 > 0) ? WIFI_11N_MCS2 : (power + 83 >0) ? WIFI_11N_MCS1 : WIFI_11N_MCS0;
                } else {
                    rate = (power + 72 > 0) ? WIFI_11N_MCS6 : (power + 74 > 0) ? WIFI_11N_MCS5 :  (power + 79 > 0) ? WIFI_11N_MCS4 : (power + 81 > 0) ? WIFI_11N_MCS3 : (power + 84 > 0) ? WIFI_11N_MCS2 : (power + 86 > 0) ? WIFI_11N_MCS1 : WIFI_11N_MCS0;
                }
            }

            else if (sta_chbw == WIFINET_BWC_WIDTH40) {
                if (tcp_rx) {
                    rate = (power + 70 > 0) ? WIFI_11N_MCS3 : (power + 75 > 0) ? WIFI_11N_MCS2 : (power + 80 >0) ? WIFI_11N_MCS1 : WIFI_11N_MCS0;
                } else {
                    rate = (power + 72 > 0) ? WIFI_11N_MCS6 : (power + 74 > 0) ? WIFI_11N_MCS5 : (power + 77 > 0) ? WIFI_11N_MCS4 : (power + 79 > 0) ? WIFI_11N_MCS3 : (power + 82 > 0) ? WIFI_11N_MCS2 : (power + 84 > 0) ? WIFI_11N_MCS1 : WIFI_11N_MCS0;
                }
            }
            else
                ERROR_DEBUG_OUT("bw-%d not found!\n", sta_chbw);

        } else {
            if (sta_chbw == WIFINET_BWC_WIDTH20  || power < sta->sta_wmac->wm_signal_power_bw_change_thresh_narrow) {
                if (tcp_rx) {
                    rate = (power + 73 > 0) ? WIFI_11AC_MCS3 : (power + 78 > 0) ? WIFI_11AC_MCS2 : (power + 81 >0) ? WIFI_11AC_MCS1 : WIFI_11AC_MCS0;

                    if (rx_speed < 20)
                        rate = WIFI_11AC_MCS0;
                } else {
                    rate = (power + 65 > 0) ? WIFI_11AC_MCS6 : (power + 68 > 0) ? WIFI_11AC_MCS5 : (power + 70 > 0) ? WIFI_11AC_MCS4 : \
                    (power + 73 > 0) ? WIFI_11AC_MCS3 : (power + 78 > 0) ? WIFI_11AC_MCS2 :  (power + 81 > 0) ? WIFI_11AC_MCS1 : WIFI_11AC_MCS0;
                }

                if (!(sta->sta_flags & WIFINET_NODE_VHT))
                    rate &= 0x8f;
            }

            else if (sta_chbw == WIFINET_BWC_WIDTH40  || power < sta->sta_wmac->wm_signal_power_bw_change_thresh_wide) {
                if (tcp_rx) {
                    rate = (power + 56 > 0) ? WIFI_11AC_MCS6 : (power + 62 > 0) ? WIFI_11AC_MCS5 : (power + 68 > 0) ? WIFI_11AC_MCS4 : (power + 73 > 0) ? WIFI_11AC_MCS3 : (power + 75 > 0) ? WIFI_11AC_MCS2 : (power + 77 >0) ? WIFI_11AC_MCS1 : WIFI_11AC_MCS0;

                    if (rx_speed < 20)
                        rate = WIFI_11AC_MCS0;
                } else {
                    rate = (power + 54 > 0) ? WIFI_11AC_MCS8 : (power + 60 > 0) ? WIFI_11AC_MCS7 : (power + 62 > 0) ? WIFI_11AC_MCS6 : (power + 64 > 0) ? WIFI_11AC_MCS5 : (power + 67 > 0) ? WIFI_11AC_MCS4 : (power + 69 > 0) ? WIFI_11AC_MCS3 : \
                    (power + 74 > 0) ? WIFI_11AC_MCS2 : (power + 76 > 0) ? WIFI_11AC_MCS1 : WIFI_11AC_MCS0;
                }

                if (!(sta->sta_flags & WIFINET_NODE_VHT))
                    rate &= 0x8f;
            }

            else if (sta_chbw == WIFINET_BWC_WIDTH80) {
                if (tcp_rx) {
                    rate = (power + 60 > 0) ? WIFI_11AC_MCS4 : (power + 63 > 0) ? WIFI_11AC_MCS3 : (power + 66 >0) ? WIFI_11AC_MCS2 : \
                    (power + 70 > 0) ? WIFI_11AC_MCS1 : WIFI_11AC_MCS0;

                    if (rx_speed < 10)
                        rate = WIFI_11AC_MCS0;
                } else {
                    rate = (power + 54 > 0) ? WIFI_11AC_MCS6 : (power + 57 > 0) ? WIFI_11AC_MCS5 : (power + 62 >0) ? WIFI_11AC_MCS4 : \
                    (power + 64 > 0) ? WIFI_11AC_MCS3 : (power + 67 >0) ? WIFI_11AC_MCS2 : (power + 71 >0) ? WIFI_11AC_MCS1 : WIFI_11AC_MCS0;
                }
            }
            else
                ERROR_DEBUG_OUT("BW-%d not found!\n", sta_chbw);

        }

        if (rate > ptxdesc->txdesc_rateinfo[0].vendor_rate_code)
            rate = ptxdesc->txdesc_rateinfo[0].vendor_rate_code;

        mcs_bw = sta_chbw;
        if ((sta->sta_avg_bcn_rssi < sta->sta_wmac->wm_signal_power_bw_change_thresh_narrow) && !(rate & 0xf)) {
            mcs_bw = CHAN_BW_20M;

        } else if (sta->sta_avg_bcn_rssi < sta->sta_wmac->wm_signal_power_bw_change_thresh_wide) {
            if (sta_chbw >= CHAN_BW_40M)
                mcs_bw = CHAN_BW_40M;
        }

        for (i = 0 ; i < DRV_TXDESC_RATE_NUM - 1; i++) {
            if (IS_VHT_RATE(rate)) {
                if ((rate - i) > 0xc0) {
                    ptxdesc->txdesc_rateinfo[i].vendor_rate_code = (rate - i);
                } else {
                    ptxdesc->txdesc_rateinfo[i].vendor_rate_code = 0xc0;
                }
                ptxdesc->txdesc_rateinfo[i].flags |= IEEE80211_TX_RC_VHT_MCS;
            } else if (IS_HT_RATE(rate)) {
                if ((rate - i) > 0x80) {
                    ptxdesc->txdesc_rateinfo[i].vendor_rate_code = (rate - i);
                } else {
                    ptxdesc->txdesc_rateinfo[i].vendor_rate_code = 0x80;
                }
                ptxdesc->txdesc_rateinfo[i].flags |= IEEE80211_TX_RC_MCS;
            } else {
                ERROR_DEBUG_OUT("power: %d; bw: %d; rate: 0x%x\n", power, sta_chbw, rate);
            }
            ptxdesc->txdesc_rateinfo[i].shortgi_en = 0;
            ptxdesc->txdesc_rateinfo[i].bw = mcs_bw;
            ptxdesc->txdesc_rateinfo[i].flags &= ~(IEEE80211_TX_RC_40_MHZ_WIDTH | IEEE80211_TX_RC_80_MHZ_WIDTH | IEEE80211_TX_RC_SHORT_GI);
            ptxdesc->txdesc_rateinfo[i].flags |= (mcs_bw == CHAN_BW_80M) ? IEEE80211_TX_RC_80_MHZ_WIDTH : (mcs_bw == CHAN_BW_40M) ? IEEE80211_TX_RC_40_MHZ_WIDTH : CHAN_BW_20M;
            ptxdesc->txdesc_rateinfo[i].rate_index = ptxdesc->txdesc_rateinfo[i].vendor_rate_code & 0x0f;

            if (IS_MCS_RATE(ptxdesc->txdesc_rateinfo[i].vendor_rate_code)) {
                ptxdesc->txdesc_rateinfo[i].maxampdulen = max_4ms_framelen[0][HT_RC_2_MCS(ptxdesc->txdesc_rateinfo[i].vendor_rate_code)];
            }
        }
    }
}

static int drv_tx_prepare(struct drv_private *drv_priv, struct sk_buff *skbbuf,struct drv_txdesc *ptxdesc)
{
    struct wifi_frame *wh;
    struct aml_ratecontrol *ratectrl;
    struct wlan_net_vif *wnet_vif;
    struct wifi_station *sta;
    struct sk_buff *skb = (struct sk_buff *)skbbuf;
    struct wifi_mac_tx_info *txinfo = ptxdesc->txinfo;
    char seq_assign_flag = 0;
    struct wifi_mac_pkt_info *mac_pkt_info = &txinfo->ptxdesc->drv_pkt_info.pkt_info[0];

    wh = (struct wifi_frame *)os_skb_data(skbbuf);
    ptxdesc->txdesc_framectrl = *(unsigned short*)&wh->i_fc[0];
    ptxdesc->txdesc_ddraddr = (SYS_TYPE)wh;
    sta = (struct wifi_station *)txinfo->cb.sta;
    wnet_vif = sta->sta_wnet_vif;

    ratectrl = &ptxdesc->txdesc_rateinfo[0];

    if (txinfo->b_datapkt) {
        if (txinfo->b_mcast) {
            drv_tx_get_spec_frm_rate(drv_priv, skbbuf, ptxdesc, ratectrl, 1);

        } else if (mac_pkt_info->b_eap || mac_pkt_info->b_dhcp || mac_pkt_info->b_arp) {
            drv_tx_get_spec_frm_rate(drv_priv, skbbuf, ptxdesc, ratectrl, DRV_MGT_TXMAXTRY);

        } else if (txinfo->b_nulldata) {
            drv_tx_get_spec_frm_rate(drv_priv, skbbuf, ptxdesc, ratectrl, DRV_MGT_TXMAXTRY);

        } else {// unicast packet
            struct drv_tx_scoreboard *tid;
            tid = DRV_GET_TIDTXINFO((struct aml_driver_nsta *)(sta->drv_sta), txinfo->tid_index);

            if (ptxdesc->rate_valid == 0) {
                if (!drv_lookup_rate(drv_priv, sta->drv_sta, ratectrl)) {
                    return 0;
                }
            }

            /* no need to do aggr in legacy/drv_cfg
             * not support/bar-ba session fail: need to add vht case , t.b.d/zqh
            */
            if (IS_MCS_RATE(ratectrl[0].vendor_rate_code) && txinfo->ht
                && drv_priv->drv_config.cfg_txaggr && drv_aggr_query(drv_priv, sta->drv_sta, skbbuf)) {
                /*assign a new seq num with specify tid */
                if (likely(!(txinfo->b_txfrag))) {
                    *(unsigned short*)wh->i_seq = htole16(tid->seq_next << WIFINET_SEQ_SEQ_SHIFT);
                    /* the flag of assign new seq,
                     * for qos data which can be aggregated.
                     */
                    txinfo->b_Ampdu = 1;
                    CIRCLE_Add_One(tid->seq_next, WIFINET_SEQ_MAX);
                    seq_assign_flag = 1;
                }

            } else {
                if ((wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_NODATA) {
                    if (IS_HT_RATE(ratectrl[0].vendor_rate_code)) {
                        os_skb_put(skb, 2);
                        wh->i_fc[0] |= WIFINET_FC0_SUBTYPE_QOS_NULL;
                        txinfo->packetlen += 2;
                        txinfo->mpdulen += 2;
                        txinfo->cb.hdrsize += 2;
                    }
                }

                if (IS_VHT_RATE(ratectrl[0].vendor_rate_code)) {
                    *(unsigned short*)wh->i_seq = htole16(tid->seq_next << WIFINET_SEQ_SEQ_SHIFT);
                    CIRCLE_Add_One(tid->seq_next, WIFINET_SEQ_MAX);
                    seq_assign_flag = 1;
                }
            }

            if ((wnet_vif->vm_opmode == WIFINET_M_STA) && wnet_vif->vm_change_rate_enable)
                drv_tx_lower_rate_when_signal_weak(wnet_vif, ptxdesc);
        }

    } else {
        drv_tx_get_spec_frm_rate(drv_priv, skbbuf, ptxdesc, ratectrl, DRV_MGT_TXMAXTRY);
    }

    //seq not assign
    if ((!seq_assign_flag) && (!txinfo->b_PsPoll) && !txinfo->b_pmf) {
        if (!txinfo->b_txfrag) {
            *(unsigned short *)&wh->i_seq[0] = htole16(sta->sta_txseqs[txinfo->tid_index] << WIFINET_SEQ_SEQ_SHIFT);
            sta->sta_txseqs[txinfo->tid_index]++;
        }
    }

    ptxdesc->rate_valid = 1;
    txinfo->seqnum = *(unsigned short *)wh->i_seq >> WIFINET_SEQ_SEQ_SHIFT;//seqnum
    //pr_debug("%s ptxdesc:%p, skbbuf:%p, vid:%d, sta:%p, tid:%d, sn:%04x, ampdu:%d, qos:%d, rate_code:%02x, frame_control:%04x, mcast:%d, data:%d, dhcp:%d, eap:%d\n",
    //    __func__, ptxdesc, skbbuf, wnet_vif->wnet_vif_id, sta, txinfo->tid_index, txinfo->seqnum, txinfo->b_Ampdu, txinfo->b_qosdata,
    //    ratectrl->vendor_rate_code, *((unsigned short *)&(wh->i_fc[0])), txinfo->b_mcast, txinfo->b_datapkt, mac_pkt_info->b_dhcp, mac_pkt_info->b_eap);

    if (txinfo->b_pmf) {
        pr_debug("%s vid:%d, sta:%p, sn:%04x, frame_control:%04x, mcast:%d\n",
            __func__, wnet_vif->wnet_vif_id, sta, txinfo->seqnum, *((unsigned short *)&(wh->i_fc[0])), txinfo->b_mcast);
    }

    return 1;
}

int drv_tx_start( struct drv_private *drv_priv, struct sk_buff *skbbuf)
{
    int error = 0;
    struct wlan_net_vif *wnet_vif = NULL;
    struct sk_buff *skb_new = NULL;
    struct aml_driver_nsta *drv_sta = NULL;
    struct drv_tx_scoreboard *tid = NULL;
    unsigned char tid_index = os_skb_get_tid(skbbuf);
    struct wifi_station *sta = drv_priv->net_ops->os_skb_get_nsta(skbbuf);
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    unsigned short header_room = os_skb_hdrspace(skbbuf);
    unsigned short tail_room = os_skb_get_tailroom(skbbuf);

    if (sta)
        wnet_vif = sta->sta_wnet_vif;

    if (!sta  || wnet_vif == NULL || wnet_vif->vm_wmac == NULL ||
        (wnet_vif->vm_opmode == WIFINET_M_HOSTAP && sta->is_disconnecting && txinfo->b_datapkt)) {
        ERROR_DEBUG_OUT("sta is NULL or softap disconnect\n");
        return -1;
    }

    if (wnet_vif->vm_opmode != WIFINET_M_HOSTAP && wnet_vif->vm_wmac->recovery_stat < WIFINET_RECOVERY_VIF_UP) {
        ERROR_DEBUG_OUT("fw recovery not finish\n");
        return -2;
    }

    drv_sta = sta->drv_sta;
    if (!drv_sta) {
        ERROR_DEBUG_OUT("drv_sta is NULL\n");
        return -3;
    }
    tid = DRV_GET_TIDTXINFO(drv_sta, txinfo->tid_index);

    if ((txinfo->b_datapkt && !txinfo->b_nulldata && !txinfo->b_mcast && !txinfo->ptxdesc->drv_pkt_info.pkt_info[0].b_eap) &&
        !drv_priv->drv_ops.check_aggr_allow_to_send(drv_priv, sta->drv_sta, tid_index) && (!tid->tid_tx_buff_sending)) {
        drv_priv->net_ops->wifi_mac_buffer_txq_enqueue(&tid->tid_tx_buffer_queue, skbbuf);
        return 0;
    }

    do
    {
        struct wifi_frame *wh = (struct wifi_frame *)os_skb_data(skbbuf);
        ASSERT(skbbuf != NULL);

        if (drv_priv->drv_pkt_drop[wnet_vif->wnet_vif_id]) {
            ERROR_DEBUG_OUT("vid:%d, disconnect not allow to send pkt\n", wnet_vif->wnet_vif_id);
            error = -4;
            break;
        }

        if ((wnet_vif->vm_opmode == WIFINET_M_STA)
            || (wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT)) {
            os_timer_ex_start(&wnet_vif->vm_pwrsave.ips_timer_presleep);
        }

    #ifdef  CONFIG_CONCURRENT_MODE
        if (((wnet_vif->vm_wmac->wm_vsdb_slot != CONCURRENT_SLOT_NONE)
            && (wnet_vif->wnet_vif_id != wnet_vif->vm_wmac->wm_vsdb_slot))
            || (wnet_vif->vm_wmac->wm_vsdb_flags & CONCURRENT_CHANNEL_SWITCH)) {
            DPRINTF(AML_DEBUG_WARNING, "backup due to vid:%d, slot:%d, flag:%04x\n",
                wnet_vif->wnet_vif_id, wnet_vif->vm_wmac->wm_vsdb_slot, wnet_vif->vm_wmac->wm_vsdb_flags);
            drv_priv->net_ops->wifi_mac_buffer_txq_enqueue(&wnet_vif->vm_tx_buffer_queue, skbbuf);
            return 0;
        }
    #endif

        //if no need backup for powersave (M_PWR_SAV_BYPASS), bypass here. such as nulldata
        //if not triggered uapsd frame to peer sta, also bypass here.
        if (!M_FLAG_GET(skbbuf, M_PWR_SAV_BYPASS) && !M_FLAG_GET(skbbuf, M_UAPSD))
        {
            //if p2p ps frame, backup
            //if ps4quiet frame, backup; but if probereq, just send out
            if ((wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_MASK) &&
                !(((wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_MASK) == WIFINET_PSQUEUE_PS4QUIET) && (WIFINET_IS_PROBEREQ(wh))))
            {
                if (wnet_vif->vm_state != WIFINET_S_CONNECTED)
                {
                    DPRINTF(AML_DEBUG_CONNECT, "%s %d state=%s, disconnecting, drop\n",
                            __func__,__LINE__, wifi_mac_state_name[wnet_vif->vm_state]);
                    error = -5;
                    break;
                }

                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d iv_pstxqueue_flags=0x%x fc[0]=0x%x\n",
                        __func__,__LINE__, wnet_vif->vm_pstxqueue_flags & WIFINET_PSQUEUE_MASK, wh->i_fc[0]);

                drv_priv->net_ops->wifi_mac_buffer_txq_enqueue(&wnet_vif->vm_tx_buffer_queue, skbbuf);
                error = 0;
                break;
            }

            //for ap/go, if peer sta is in powersave, backup
            if (sta->sta_flags & WIFINET_NODE_PWR_MGT)
            {
                //pr_debug("ap buffer queue\n");
                drv_priv->net_ops->wifi_mac_pwrsave_psqueue_enqueue(sta, skbbuf);
                error = 0;
                break;
            }
        }

        //if vmac is in sleep, but ps=0 in frame, so need to wakeup first
        if (drv_priv->net_ops->wifi_mac_pwrsave_is_wnet_vif_sleeping(wnet_vif) == 0) {
            DRV_PS_LOCK(drv_priv);
            if (!(wh->i_fc[1] & WIFINET_FC1_PWR_MGT) && !drv_priv->add_wakeup_work) {
                drv_priv->add_wakeup_work = 1;
                DRV_PS_UNLOCK(drv_priv);
                drv_priv->net_ops->wifi_mac_pwrsave_wakeup_for_tx(wnet_vif);
                DRV_PS_LOCK(drv_priv);
            }
            DRV_PS_UNLOCK(drv_priv);

        } else {
            DRV_PS_LOCK(drv_priv);
            drv_priv->add_wakeup_work = 0;
            DRV_PS_UNLOCK(drv_priv);
        }

        if (M_PWR_SAV_GET(skbbuf)) {
            //bufferd pkt under ps-poll scheme should send in wm_mng_qid
            txinfo->queue_id = sta->sta_wmac->wm_mng_qid;
        }

        if ((header_room < HI_TXDESC_DATAOFFSET + 32) || (tail_room < 32) || (((unsigned long)skbbuf->data) % 8)) {
            skb_new = os_skb_copy_expand(skbbuf, HI_TXDESC_DATAOFFSET + 32, 32, GFP_ATOMIC, skb_new);
            if (skb_new == NULL) {
                error = -5;
                break;
            }

            os_skb_free(skbbuf);
            skbbuf = skb_new;
        }
        error = drv_send(skbbuf,  drv_priv);
    }
    while (0);

    if (error < 0)
    {
        struct wifi_mac_tx_status ts;
        ts.ts_flags = WIFINET_TX_ERROR;
        ts.ts_shortretry = 0;
        ts.ts_longretry = 0;

        drv_priv->net_ops->wifi_mac_tx_complete(wnet_vif, skbbuf, &ts);

        ERROR_DEBUG_OUT("drop skb in drv_send due to err:%d\n", error);
    }

    return error;
}

enum tx_frame_flag drv_set_tx_frame_flag(struct sk_buff *skbbuf)
{
    struct wifi_mac_p2p_pub_act_frame *p2p_pub_act = NULL;
    struct wifi_mac_p2p_action_frame *p2p_act = NULL;
    unsigned char ret = TX_OTHER_FRAME;
    struct wifi_frame *wh = (struct wifi_frame *)(skbbuf->data);
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    struct wifi_mac_pkt_info *mac_pkt_info = &txinfo->ptxdesc->drv_pkt_info.pkt_info[0];

    p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)(skbbuf->data + sizeof(struct wifi_frame));
    p2p_act = (struct wifi_mac_p2p_action_frame *)(skbbuf->data + sizeof(struct wifi_frame));

    if (p2p_pub_act && (p2p_pub_act->category == AML_CATEGORY_PUBLIC)
        && (p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P)) {
        switch (p2p_pub_act->subtype) {
            case P2P_GO_NEGO_REQ:
            case P2P_GO_NEGO_RESP:
            case P2P_PROVISION_DISC_REQ:
            case P2P_PROVISION_DISC_RESP:
            case P2P_INVITE_REQ:
            case P2P_INVITE_RESP:
                ret = TX_P2P_OTHER_GO_NEGO_FRAME;
                break;
            case P2P_GO_NEGO_CONF:
                ret = TX_P2P_GO_NEGO_CONF;
                break;
            default:
                break;
        }
    } else if (p2p_act && (p2p_act->category == AML_CATEGORY_P2P) && (p2p_act->subtype == P2P_PRESENCE_REQ)) {
        ret = TX_P2P_PRESENCE_REQ;
    }
#ifdef CTS_VERIFIER_GAS
    else if (p2p_pub_act && (p2p_pub_act->category == AML_CATEGORY_PUBLIC)
        && ((p2p_pub_act->action == WIFINET_ACT_PUBLIC_GAS_REQ) || (p2p_pub_act->action == WIFINET_ACT_PUBLIC_GAS_RSP))) {
        ret = TX_P2P_GAS;
    }
#endif

    if (WIFINET_IS_PROBEREQ(wh)) {
        ret = TX_MGMT_PROBE_REQ;

    } else if (WIFINET_IS_AUTH(wh)) {
        ret = TX_MGMT_AUTH;

    } else if (WIFINET_IS_DEAUTH(wh)) {
        ret = TX_MGMT_DEAUTH;

    } else if (WIFINET_IS_ASSOC_REQ(wh)) {
        ret = TX_MGMT_ASSOC_REQ;

    } else if (WIFINET_IS_DISASSOC(wh)) {
        ret = TX_MGMT_DISASSOC;

    } else if (WIFINET_IS_ACTION(wh)) {
        if ((p2p_pub_act->category == AML_CATEGORY_BACK) && (p2p_pub_act->action == WIFINET_ACTION_BA_ADDBA_REQUEST)) {
            ret = TX_MGMT_ADDBA_RSP;
        } else if (p2p_pub_act->action == WIFINET_ACT_PUBLIC_CSA) {
            ret = TX_MGMT_CSA_ACTION;
        }
    } else if (mac_pkt_info->b_eap) {
        ret = TX_MGMT_EAPOL;
    } else if (mac_pkt_info->b_dhcp) {
        ret = TX_MGMT_DHCP;
    }

    return ret;
}


int drv_send(struct sk_buff *skbbuf, struct drv_private *drv_priv)
{
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    struct wifi_station *sta = (struct wifi_station *)(txinfo->cb.sta);
    struct aml_driver_nsta *drv_sta = sta->drv_sta;
    struct drv_txdesc *ptxdesc = NULL;
    struct list_head txdesc_list_head;
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, txinfo->tid_index);
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[txinfo->queue_id];
    int ret = -1;
    struct wifi_frame *wh = (struct wifi_frame *)(skbbuf->data);

    sts_sw_probe(sts_drv_send_skb_idx , 1);

    INIT_LIST_HEAD(&txdesc_list_head);

    ptxdesc = txinfo->ptxdesc;
    ptxdesc->txinfo = txinfo;
    list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head);

    //build separated tx desc: set tx_info/calc pages/set skbbuf/next ptr/last_ptr/subframe for agg/
    ptxdesc->txdesc_queue_next = NULL;
    ptxdesc->txdesc_frame_flag = drv_set_tx_frame_flag(skbbuf);

    /*protect from get txdesc to enqueue tid_queue by DRV_TXQ_LOCK */
    DRV_TXQ_LOCK(txlist);
    //get a fitable rate according tx_info
    if (!drv_tx_prepare(drv_priv, skbbuf, ptxdesc)) {
        ERROR_DEBUG_OUT("drv_tx_prepare not ok\n");
        goto bad;
    }

    ptxdesc->txdesc_sta = drv_sta;
    ptxdesc->txdesc_mpdu = skbbuf;
    ptxdesc->txdesc_agglen = txinfo->packetlen;
    ptxdesc->txdesc_aggr_page_num = drv_priv->hal_priv->hal_ops.hal_calc_mpdu_page(ptxdesc->txinfo->mpdulen);
    ptxdesc->txdesc_queue_lastframe = ptxdesc;  // pointer to itself
    ptxdesc->txdesc_queue_last = ptxdesc;          //pointer to itself
    ptxdesc->txdesc_pktnum = 1;
    drv_txdesc_set_rts_cts(drv_priv, ptxdesc);

    if (txinfo->b_uapsd && (wh->i_fc[0] & (FRAME_TYPE_MASK | FRAME_SUBTYPE_MASK)) == MAC_FCTRL_QOS_DATA) {
        drv_tx_queue_uapsd_nsta(drv_priv, &txdesc_list_head, drv_sta);
        DRV_TXQ_UNLOCK(txlist);
        pr_debug("%s uapsd\n", __func__);
        return 0;
    }

    /*send qos data frames : not frag and set agg flag*/
    if (txinfo->b_Ampdu) {
        if (drv_tx_send_ampdu(drv_priv, txlist, tid, &txdesc_list_head)) {
            ERROR_DEBUG_OUT("<running> drv_tx_send_ampdu fail\n");
            goto bad;
        }

    } else {
        DRV_TXQ_UNLOCK(txlist);
        /*no aggregated: data frames,  mgmt frames and multicast/broad frames*/
        if (txinfo->b_mcast) {
#ifdef  AML_MCAST_QUEUE
            //mcast backup also use txlist_backup_qcnt
            if (txinfo->ps) {
                drv_tx_mcastq_addbuf(drv_priv, &txdesc_list_head);
                pr_debug("%s mcastq\n", __func__);
                return 0;
            }
#endif
        }

        ret = drv_tx_normal(drv_priv, txlist, &txdesc_list_head, sta->wnet_vif_id);
        DRV_TXQ_LOCK(txlist);

        if (ret != 0) {
            ERROR_DEBUG_OUT("<running> fail\n");
            goto bad;
        }
    }

    DRV_TXQ_UNLOCK(txlist);
    return 0;

// recycle tx_ds resource
bad:
    DRV_TXQ_UNLOCK(txlist);
    DPRINTF(AML_DEBUG_INFO,"<running> %s ret:%d\n",__func__, ret);
    return -ENOMEM;
}

void drv_tx_complete(struct drv_private *drv_priv, struct drv_txdesc *ptxdesc, int txok)
{
    struct sk_buff *skbbuf = ptxdesc->txdesc_mpdu;
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac_tx_status  ts;
    unsigned char ampdu = ptxdesc->txinfo->b_Ampdu;
    int b_aggr;

#ifdef DRV_TCP_RETRANSMISSION
    struct wifi_mac_pkt_info *mac_pkt_info = &ptxdesc->drv_pkt_info.pkt_info[0];
#endif

    ts.ts_flags = 0;

    if (skbbuf == NULL) {
        ERROR_DEBUG_OUT("(skbbuf== NULL) ERROR!!!!!!!\n");
        return;
    }

    if (ampdu) {
        b_aggr = ptxdesc->txinfo->b_aggr;

        if (b_aggr) {
            drv_priv->drv_stats.tx_end_ampdu_cnt++;
        }

        if ((!b_aggr ) && txok) {
            drv_priv->drv_stats.tx_end_normal_cnt++;

        } else {
            drv_priv->drv_stats.tx_end_fail_cnt++;
        }
    }

    if (txok) {
        drv_priv->drv_stats.tx_total_bytes += ptxdesc->txinfo->packetlen;

    } else {
        ts.ts_flags = WIFINET_TX_ERROR;
    }

    {
        struct wifi_frame *wh = (struct wifi_frame *)os_skb_data(skbbuf);
        struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skbbuf->cb;
        struct wifi_mac_tx_info *txinfo = ptxdesc->txinfo;
        unsigned short queue_id = txinfo->queue_id;
        struct wifi_station *sta = cb->sta;
        unsigned char vid = txinfo->wnet_vif_id;
        /* nsta may disconnect during tx */

        if (sta)
        {
            wnet_vif = drv_priv->drv_wnet_vif_table[vid];

            if (M_NULL_PWR_SAV_GET(skbbuf))
            {
                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d null with ps=%d ok=%d\n",
                        __func__,__LINE__, !!(wh->i_fc[1] & WIFINET_FC1_PWR_MGT), !ts.ts_flags);
                if (wh->i_fc[1] & WIFINET_FC1_PWR_MGT)
                {
                    //if send nulldata with ps=1 successfully, enter sleep
                    //else wakeup
                    if (!ts.ts_flags)
                    {
                         drv_priv->net_ops->wifi_mac_pwrsave_fullsleep(wnet_vif, SLEEP_AFTER_TX_NULL_WITH_PS);
#ifdef USER_UAPSD_TRIGGER
                        os_timer_ex_start(&(wnet_vif->vm_pwrsave.ips_timer_uapsd_trigger));
#endif
                    }
                    else
                    {
                         drv_priv->net_ops->wifi_mac_pwrsave_wkup_and_NtfyAp(wnet_vif, WKUP_FROM_PSNULL_FAIL);
                    }
                }
                M_NULL_PWR_SAV_CLR(skbbuf);
            }

            if (M_PWR_SAV_GET(skbbuf))
            {
                WIFINET_PWRSAVE_LOCK(wnet_vif);
                if (wnet_vif->vm_legacyps_txframes > 0)
                    wnet_vif->vm_legacyps_txframes--;
                WIFINET_PWRSAVE_UNLOCK(wnet_vif);
                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d legacyps_txframes=%d\n", __func__,__LINE__, wnet_vif->vm_legacyps_txframes);
            }

            if ((queue_id == HAL_WME_MCAST) || (queue_id == HAL_WME_UAPSD) || M_PWR_SAV_GET(skbbuf))
            {
#ifdef CONFIG_P2P
                if (wnet_vif->vm_p2p->p2p_flag & P2P_OPPPS_CWEND_FLAG_HI)
                {
                    drv_p2p_go_opps_cwend_may_sleep(wnet_vif);
                }
#endif
            }
        }
    }

    //skb allocated by tcp/ip layer and released by driver layer,
    //no matter any reasons: ok or aged or resource full or net fail...

#ifdef DRV_TCP_RETRANSMISSION
    if (!mac_pkt_info->b_tcp)
    {
        drv_priv->net_ops->wifi_mac_tx_complete(wnet_vif, skbbuf, &ts);
    }
    else
    {
        mac_pkt_info->b_tcp_free = 1;
    }
#else
    drv_priv->net_ops->wifi_mac_tx_complete(wnet_vif, skbbuf, &ts);
#endif
}

static void reset_connected_sta_keepalive_time(struct wifi_station *sta)
{
    sta->sta_inact = sta->sta_inact_reload;
    sta->sta_inact_time = jiffies;
}

static void drv_tx_complete_mgmt_handle(struct drv_private *drv_priv,struct drv_txdesc *ptxdesc,
    unsigned char status, struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = NULL;
    int txok = (status == TX_DESCRIPTOR_STATUS_SUCCESS);
    int mgmt_arg;
    static int deauth_fail_time = 0;

    wnet_vif = sta->sta_wnet_vif;

    if ((ptxdesc->txdesc_frame_flag == TX_P2P_OTHER_GO_NEGO_FRAME)
        || (ptxdesc->txdesc_frame_flag == TX_P2P_GO_NEGO_CONF)
        || (ptxdesc->txdesc_frame_flag == TX_P2P_PRESENCE_REQ)) {

        pr_debug("%s, txdesc_frame_flag=%d, status=%d\n", __func__, ptxdesc->txdesc_frame_flag, status);
        if (txok) {
            sta->sta_wnet_vif->vm_p2p->tx_status_flag = WIFINET_TX_STATUS_SUCC;
            sta->sta_wnet_vif->vm_p2p->send_tx_status_flag = 1;
            cfg80211_mgmt_tx_status(sta->sta_wnet_vif->vm_wdev, sta->sta_wnet_vif->vm_p2p->cookie,
                sta->sta_wnet_vif->vm_p2p->raw_action_pkt, sta->sta_wnet_vif->vm_p2p->raw_action_pkt_len, txok, GFP_KERNEL);

        } else {
            if (ptxdesc->txdesc_frame_flag == TX_P2P_GO_NEGO_CONF || ptxdesc->txdesc_frame_flag == TX_P2P_PRESENCE_REQ) {
                sta->sta_wnet_vif->vm_p2p->action_retry_time = DEFAULT_MGMT_RETRY_INTERVAL;

            } else {
                sta->sta_wnet_vif->vm_p2p->action_retry_time = DEFAULT_P2P_ACTION_RETRY_INTERVAL;
            }
            sta->sta_wnet_vif->vm_p2p->tx_status_flag = WIFINET_TX_STATUS_FAIL;
            os_timer_ex_start_period(&sta->sta_wnet_vif->vm_actsend, sta->sta_wnet_vif->vm_p2p->action_retry_time);
        }
    }

#ifdef CTS_VERIFIER_GAS
    if (ptxdesc->txdesc_frame_flag == TX_P2P_GAS) {
        pr_debug("%s, txdesc_frame_flag=%d, status=%d\n", __func__, ptxdesc->txdesc_frame_flag, status);

        if ((sta->sta_wnet_vif->vm_p2p->action_code == WIFINET_ACT_PUBLIC_GAS_REQ && sta->sta_wnet_vif->vm_p2p->p2p_flag & P2P_GAS_RSP) ||
            (sta->sta_wnet_vif->vm_p2p->action_code == WIFINET_ACT_PUBLIC_GAS_RSP && txok) ||
            txok) {

            sta->sta_wnet_vif->vm_p2p->tx_status_flag = WIFINET_TX_STATUS_SUCC;
            sta->sta_wnet_vif->vm_p2p->send_tx_status_flag = 1;
            cfg80211_mgmt_tx_status(sta->sta_wnet_vif->vm_wdev, sta->sta_wnet_vif->vm_p2p->cookie,
            sta->sta_wnet_vif->vm_p2p->raw_action_pkt, sta->sta_wnet_vif->vm_p2p->raw_action_pkt_len, txok, GFP_KERNEL);

        } else {
            sta->sta_wnet_vif->vm_p2p->action_retry_time = 150;// DEFAULT_MGMT_RETRY_INTERVAL;
            sta->sta_wnet_vif->vm_p2p->tx_status_flag = WIFINET_TX_STATUS_FAIL;
            os_timer_ex_start_period(&sta->sta_wnet_vif->vm_actsend, sta->sta_wnet_vif->vm_p2p->action_retry_time);

        }
    }
#endif

    if (ptxdesc->txdesc_frame_flag == TX_MGMT_CSA_ACTION) {
        AML_OUTPUT("channel switch announce action frm status=%d, channel:%d\n",status, sta->sta_wmac->wm_curchan->chan_pri_num);
    }

    if ((ptxdesc->txdesc_frame_flag >= TX_MGMT_PROBE_REQ) && !txok) {
        drv_priv->drv_ops.cca_busy_check();
        pr_debug("%s, txdesc_frame_flag:%d, status=%d, rate:%02x\n",
            __func__, ptxdesc->txdesc_frame_flag, status, ptxdesc->txdesc_rateinfo[0].vendor_rate_code);

    } else if (ptxdesc->txdesc_frame_flag == TX_MGMT_ADDBA_RSP) {
        ;
    }

    if (ptxdesc->txdesc_frame_flag == TX_MGMT_DEAUTH && txok
        && sta->sta_wnet_vif->vm_opmode == WIFINET_M_STA) {
        wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
        deauth_fail_time = 0;
    }

    if (ptxdesc->txdesc_frame_flag == TX_MGMT_DEAUTH && !txok
        && sta->sta_wnet_vif->vm_opmode == WIFINET_M_STA) {
        deauth_fail_time ++;
    }

    if(deauth_fail_time == 1) {
        mgmt_arg = WIFINET_REASON_AUTH_LEAVE;
        wifi_mac_send_mgmt(wnet_vif->vm_mainsta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&mgmt_arg);
    }

    if(deauth_fail_time == 2) {
        wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_sm_switch, NULL, (SYS_TYPE)wnet_vif, WIFINET_S_SCAN, 0, 0, 0);
        deauth_fail_time = 0;
    }

}

static void drv_tx_complete_task(struct drv_private *drv_priv, struct drv_txlist *txlist,
    struct drv_txdesc *ptxdesc, unsigned char status,unsigned char RetryCnt,int8_t AckRssi)
{
    struct list_head txdesc_list_head;
    int sr, lr;
    int uapsdq = 0, mcastq = 0;
    unsigned char vendor_rate_code;
    struct aml_driver_nsta *drv_sta = ptxdesc->txdesc_sta;
    int txok = (status == TX_DESCRIPTOR_STATUS_SUCCESS);
    struct wifi_mac_tx_status ts;
    struct wifi_station *sta;
    unsigned char rt_rate_index;
    unsigned char left_try_num;
    struct wifi_mac_pkt_info *mac_pkt_info = &ptxdesc->drv_pkt_info.pkt_info[0];
    struct wlan_net_vif *wnet_vif = NULL;
    struct hw_interface* hif = hif_get_hw_interface();

    if (drv_sta == NULL) {
        return;
    }
    sta = (struct wifi_station *)drv_sta->net_nsta;

    wnet_vif = sta->sta_wnet_vif;

    ts.ts_ackrssi = AckRssi - 256;

    drv_tx_complete_mgmt_handle(drv_priv,ptxdesc,status,sta);

    if (txok) {
        if ((sta->sta_wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
            && ((ptxdesc->txdesc_framectrl & WIFINET_FC0_SUBTYPE_MASK) == WIFINET_FC0_SUBTYPE_NODATA)
            && ((ptxdesc->txdesc_framectrl & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_DATA)) {
            reset_connected_sta_keepalive_time(sta);
        }
        hif->HiStatus.tx_ok_num++;

    } else {
        hif->HiStatus.tx_fail_num++;
    }

    if (txlist == drv_priv->drv_uapsdqueue) {
        uapsdq = 1;
    }

    if (txlist == drv_priv->drv_mcasequeue) {
        mcastq = 1;
    }

    INIT_LIST_HEAD(&txdesc_list_head);

#ifdef DRV_TCP_RETRANSMISSION
    if (!mac_pkt_info->b_tcp)
    {
        list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head);
    }
#else
    list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head);
#endif

    if (drv_sta != NULL) {
        int noratectrl;

        noratectrl = drv_sta->sta_cleanup || drv_sta->sta_powesave;
        if (txok) {
            ts.ts_flags=0;
            vendor_rate_code = ptxdesc->txdesc_rateinfo[0].vendor_rate_code;
            rt_rate_index = drv_rate_findindex_from_ratecode(drv_priv->drv_currratetable, vendor_rate_code);

            if (ptxdesc->txinfo->b_datapkt) {
                ts.ts_ratekbps= drv_priv->drv_currratetable->info[rt_rate_index].rateKbps;
                if (IS_HT_RATE(vendor_rate_code)) {
                    unsigned char rateFlags = ptxdesc->txdesc_rateinfo[0].flags;

                    if (rateFlags & HAL_RATECTRL_CW40_FLAG) {
                        ts.ts_ratekbps = (ts.ts_ratekbps * 27) / 13;
                    }

                    if (rateFlags & HAL_RATECTRL_SGI_FLAG) {
                        ts.ts_ratekbps = (ts.ts_ratekbps * 10) / 9;
                    }
                }

            } else {
                ts.ts_ratekbps = 0;
            }
            drv_priv->net_ops->wifi_mac_tx_status(drv_sta->net_nsta,&ts);
        }

        sr = (RetryCnt & 0xf0) >> 4;
        lr = RetryCnt & 0x0f;

        if (ptxdesc->txinfo->b_noack == 0) {
            /* Only data packets take participate in rate control. */
            if (ptxdesc->txinfo->b_datapkt && !noratectrl) {
                ts.ts_longretry = lr;
                ts.ts_shortretry = sr;
                ptxdesc->txdesc_rateinfo[3].trynum = 0;

                if (status == TX_DESCRIPTOR_STATUS_SUCCESS) {
                    if ((ts.ts_longretry + ts.ts_shortretry) < ptxdesc->txdesc_rateinfo[0].trynum) {
                        ts.ts_finaltsi = 0;
                        ts.ts_rateindex = ptxdesc->txdesc_rateinfo[0].vendor_rate_code;
                        left_try_num = ts.ts_longretry+ts.ts_shortretry;
                        ptxdesc->txdesc_rateinfo[0].trynum = left_try_num + 1;
                        ptxdesc->txdesc_rateinfo[1].trynum = 0;
                        ptxdesc->txdesc_rateinfo[2].trynum = 0;

                    } else if ((ts.ts_longretry + ts.ts_shortretry) < (ptxdesc->txdesc_rateinfo[0].trynum + ptxdesc->txdesc_rateinfo[1].trynum)) {
                        ts.ts_finaltsi = 1;
                        ts.ts_rateindex = ptxdesc->txdesc_rateinfo[1].vendor_rate_code;
                        left_try_num = ts.ts_longretry + ts.ts_shortretry - ptxdesc->txdesc_rateinfo[0].trynum;
                        ptxdesc->txdesc_rateinfo[1].trynum = left_try_num + 1;
                        ptxdesc->txdesc_rateinfo[2].trynum = 0;

                    } else {
                        ts.ts_finaltsi = 2;
                        ts.ts_rateindex = ptxdesc->txdesc_rateinfo[2].vendor_rate_code;
                        left_try_num = ts.ts_longretry + ts.ts_shortretry - ptxdesc->txdesc_rateinfo[0].trynum - ptxdesc->txdesc_rateinfo[1].trynum;
                        ptxdesc->txdesc_rateinfo[2].trynum = left_try_num + 1;
                    }

                    if (status == TX_DESCRIPTOR_STATUS_SUCCESS) {
                        ptxdesc->txdesc_rateinfo[0].flags |= HAL_RATECTRL_TX_SEND_SUCCESS;
                    }

                } else {
                    ptxdesc->txdesc_rateinfo[1].trynum = 0;
                    ptxdesc->txdesc_rateinfo[2].trynum = 0;
                }

                if (ptxdesc->txinfo->b_Ampdu) {
                    if (ptxdesc->txdesc_queue_last != ptxdesc) {
                        AML_PRINT(AML_DBG_MODULES_RATE_CTR, "first status:%d, ts.ts_longretry:%d, ts.ts_shortretry:%d, rate:%02x:%02x:%02x, bw:%d, seqnum:%04x, vid:%d, rssi:%d, snr:%d\n", status,
                            ts.ts_longretry, ts.ts_shortretry, ptxdesc->txdesc_rateinfo[0].vendor_rate_code, ptxdesc->txdesc_rateinfo[1].vendor_rate_code,
                            ptxdesc->txdesc_rateinfo[2].vendor_rate_code, ptxdesc->txdesc_rateinfo[0].bw, ptxdesc->txinfo->seqnum, ptxdesc->txinfo->wnet_vif_id, sta->sta_avg_bcn_rssi, sta->sta_avg_snr);

                    } else {
                        AML_PRINT(AML_DBG_MODULES_RATE_CTR, "status:%d, ts.ts_longretry:%d, ts.ts_shortretry:%d, rate:%02x:%02x:%02x, bw:%d, seqnum:%04x, vid:%d, rssi:%d, snr:%d\n", status,
                            ts.ts_longretry, ts.ts_shortretry, ptxdesc->txdesc_rateinfo[0].vendor_rate_code, ptxdesc->txdesc_rateinfo[1].vendor_rate_code,
                            ptxdesc->txdesc_rateinfo[2].vendor_rate_code, ptxdesc->txdesc_rateinfo[0].bw, ptxdesc->txinfo->seqnum, ptxdesc->txinfo->wnet_vif_id, sta->sta_avg_bcn_rssi, sta->sta_avg_snr);
                    }
                } else {
                    AML_PRINT(AML_DBG_MODULES_RATE_CTR, "status:%d, ts.ts_longretry:%d, ts.ts_shortretry:%d, rate:%02x:%02x:%02x, bw:%d, seqnum:%04x, vid:%d, rssi:%d, snr:%d\n", status,
                        ts.ts_longretry, ts.ts_shortretry, ptxdesc->txdesc_rateinfo[0].vendor_rate_code, ptxdesc->txdesc_rateinfo[1].vendor_rate_code,
                        ptxdesc->txdesc_rateinfo[2].vendor_rate_code, ptxdesc->txdesc_rateinfo[0].bw, ptxdesc->txinfo->seqnum, ptxdesc->txinfo->wnet_vif_id, sta->sta_avg_bcn_rssi, sta->sta_avg_snr);
                }
                DRV_MINSTREL_LOCK(drv_priv);
                drv_priv->ratctrl_ops->rate_tx_complete(ptxdesc->txdesc_rateinfo, sta);
                DRV_MINSTREL_UNLOCK(drv_priv);
            }
        }

        if ((ptxdesc->txinfo->b_nulldata == 0) && ptxdesc->txinfo->b_Ampdu) {
            struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, ptxdesc->txinfo->tid_index);

            DRV_TXQ_LOCK(txlist);
            drv_tx_update_ba_win(drv_priv, tid, ptxdesc->txinfo->seqnum, txlist);
            if  (tid->cleanup_inprogress) {
                if (tid->baw_head == tid->baw_tail) {
                    tid->addba_exchangecomplete = 0;
                    tid->addba_exchangeattempts = 0;
                    tid->addba_exchangestatuscode = WIFINET_STATUS_UNSPECIFIED;
                    tid->cleanup_inprogress = 0;
                    DPRINTF(AML_DEBUG_ADDBA, "%s head:%d, tail:%d, seq_start:%04x, seq_next:%04x, paused:%d\n",
                        __func__, tid->baw_head, tid->baw_tail, tid->seq_start, tid->seq_next, tid->paused);
                }
            }
            DRV_TXQ_UNLOCK(txlist);
        }
    }

    do
    {
        if (uapsdq)
        {
            if (drv_sta)
            {
                DRV_TXQ_LOCK(txlist);
                drv_priv->net_ops->wifi_mac_uapsd_eosp_indicate(drv_sta->net_nsta, ptxdesc->txdesc_mpdu, txok);
                DRV_TXQ_UNLOCK(txlist);
            }
#ifdef DRV_SUPPORT_TX_WITHDRAW
            if (status == TX_DESCRIPTOR_STATUS_P2P_NOA_WITHDRAW)
            {
                //if withdraw by fw, such as p2p noa, backup to uapsd_withdraw_queue
                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d uapsd withdraw\n",__func__,__LINE__);
                drv_tx_withdraw_uapsd_addbuf(drv_priv, txdesc_list_head, drv_sta);
                break;
            }
#endif
            drv_tx_complete(drv_priv, ptxdesc, txok);
            break;
        }

        if (M_PWR_SAV_GET((struct sk_buff *)(ptxdesc->txdesc_mpdu)))
        {
#ifdef DRV_SUPPORT_TX_WITHDRAW
            if (status == TX_DESCRIPTOR_STATUS_P2P_NOA_WITHDRAW)
            {
                //if withdraw by fw, such as p2p noa, backup to uapsd_withdraw_queue
                DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d legacy ps withdraw\n",__func__,__LINE__);
                drv_tx_withdraw_legacyps_addbuf(drv_priv, txdesc_list_head, drv_sta);
                break;
            }
#endif
            drv_tx_complete(drv_priv, ptxdesc, txok);
            break;
        }

        if (ptxdesc->txinfo->b_datapkt && mac_pkt_info->b_tcp) {
            AML_PRINT(AML_DBG_MODULES_TX, "seqnum:%d(0x%04x), scr_port:%u, dst_port:%u; seq_num:%u; ack_num:%u, txok:%d, rate:%02x\n", ptxdesc->txinfo->seqnum,
                ptxdesc->txinfo->seqnum, __constant_htons(mac_pkt_info->tcp_src_port), __constant_htons(mac_pkt_info->tcp_dst_port),
                __constant_htonl(mac_pkt_info->tcp_seqnum), __constant_htonl(mac_pkt_info->tcp_ack_seqnum), txok, ptxdesc->txdesc_rateinfo[0].vendor_rate_code);

            if (!txok) {
                ptxdesc->rate_valid = 0;
                skb_reserve(ptxdesc->txdesc_mpdu, HI_TXDESC_DATAOFFSET);
                drv_priv->net_ops->wifi_mac_buffer_txq_enqueue(&drv_priv->retransmit_queue, ptxdesc->txdesc_mpdu);
                break;
            }
        }

        drv_tx_complete(drv_priv, ptxdesc, txok);
    }
    while(0);

    tasklet_schedule(&drv_priv->ampdu_tasklet);
    tasklet_schedule(&drv_priv->retransmit_tasklet);

    if (drv_txq_backup_qcnt(txlist) > 0)
    {
        if (mcastq)
        {
            //if in ps, cannot send here, so use this
            drv_tx_mcastq_send(drv_priv);
        }
        else
        {
            DPRINTF(AML_DEBUG_XMIT, "%s %d normal_backup qcnt=%d\n", __func__, __LINE__, drv_txq_backup_qcnt(txlist));
            drv_txq_backup_send(drv_priv, txlist);
        }
    }

    return;
}

void drv_tx_irq_tasklet(void *drv_priv_s, struct txdonestatus *tx_done_status,
    SYS_TYPE callback, unsigned char queue_id)
{
    int qqcnt = 0;
    struct drv_private *drv_priv = (struct drv_private *)drv_priv_s;
    struct drv_txdesc *ptxdesc = (struct drv_txdesc *)callback;
    struct drv_txlist *txlist;

#ifdef DRV_TCP_RETRANSMISSION
    struct wifi_mac_pkt_info *mac_pkt_info = &ptxdesc->drv_pkt_info.pkt_info[0];
#endif

    if (ptxdesc == NULL) {
        //pr_err("%s: callback txds is null\n", __func__);
        return;
    }

    if (DRV_TXQUEUE_VALUE(drv_priv, queue_id))
    {
        txlist = &drv_priv->drv_txlist_table[queue_id];

        DRV_TXQ_LOCK(txlist);
        if (ptxdesc->txdesc_sta == NULL) {
            pr_warn("%s pkt has already freed\n", __func__);
            DRV_TXQ_UNLOCK(txlist);
            return;
        }

#ifdef DRV_TCP_RETRANSMISSION
        if (!mac_pkt_info->b_tcp)
        {
            list_del_init(&ptxdesc->txdesc_queue);
        }
#else
        list_del_init(&ptxdesc->txdesc_queue);
#endif

        if (txlist->txlist_qcnt > 0)
             txlist->txlist_qcnt--;
        DRV_TXQ_UNLOCK(txlist);

        drv_tx_complete_task(drv_priv, txlist, ptxdesc, tx_done_status->txstatus, tx_done_status->txretrynum, tx_done_status->ack_rssi);
        qqcnt += drv_txlist_all_qcnt(drv_priv, queue_id);
    }

    if (drv_priv->net_ops->wifi_mac_notify_qstatus)
    {
        drv_priv->net_ops->wifi_mac_notify_qstatus(drv_priv->wmac, qqcnt);
    }

    drv_priv->drv_stats.rx_ack_rssi += tx_done_status->ack_rssi;
    drv_priv->drv_stats.tx_retry_cnt += tx_done_status->txretrynum;
    drv_priv->drv_stats.tx_short_retry_cnt += tx_done_status->txsretrynum;
}

static void drv_txlist_free_all_by_vid(struct drv_private *drv_priv, struct drv_txlist *txlist, unsigned char vid)
{
    struct drv_txdesc *ptxdesc;
    struct list_head txdesc_list_head_not_free;
    struct drv_tx_scoreboard *tid = NULL;
    struct hi_tx_desc *pTxDPape = NULL;
    void *desc_sta = NULL;

    INIT_LIST_HEAD(&txdesc_list_head_not_free);

    DRV_TXQ_LOCK(txlist);
    for (;;)
    {
        if (list_empty(&txlist->txlist_q)) {
            break;
        }
        ptxdesc = list_first_entry(&txlist->txlist_q, struct drv_txdesc, txdesc_queue);
        list_del_init(&ptxdesc->txdesc_queue);
        desc_sta = ptxdesc->txdesc_sta;
        if ((ptxdesc->txinfo->wnet_vif_id == vid) || (vid == 3)) {
            ptxdesc->txdesc_sta = NULL;

            DRV_TXQ_UNLOCK(txlist);
            pTxDPape = (struct hi_tx_desc *)os_skb_data(ptxdesc->txdesc_mpdu);
            if (pTxDPape && !pTxDPape->TxOption.pkt_position) {
                tid = DRV_GET_TIDTXINFO((struct aml_driver_nsta *)desc_sta, ptxdesc->txinfo->tid_index);
                if (tid != NULL && ptxdesc->txinfo->b_Ampdu) {
                    drv_tx_update_ba_win(drv_priv, tid, ptxdesc->txinfo->seqnum, txlist);
                }

                drv_tx_complete(drv_priv, ptxdesc, 0);
                txlist->txlist_qcnt--;
            }
            DRV_TXQ_LOCK(txlist);

        } else {
            list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head_not_free);
        }
    }

    if (!list_empty(&txdesc_list_head_not_free)) {
        list_splice_tail_init(&txdesc_list_head_not_free, &txlist->txlist_q);
    }

    if (drv_priv->drv_config.cfg_txaggr)
    {
        drv_txlist_push_pending(drv_priv, txlist, vid);
        drv_priv->net_ops->wifi_mac_free_txamsdu_queue(drv_priv->wmac, vid);
    }
    DRV_TXQ_UNLOCK(txlist);

    drv_txq_backup_drain(drv_priv, txlist, vid);
}

static void drv_txlist_free_all_by_drv_sta(struct drv_private *drv_priv, struct drv_txlist *txlist, struct aml_driver_nsta *drv_sta)
{
    struct drv_txdesc *ptxdesc;
    struct list_head txdesc_list_head_not_free;

    INIT_LIST_HEAD(&txdesc_list_head_not_free);

    DRV_TXQ_LOCK(txlist);
    for (;;)
    {
        if (list_empty(&txlist->txlist_q)) {
            break;
        }

        ptxdesc = list_first_entry(&txlist->txlist_q, struct drv_txdesc, txdesc_queue);
        list_del_init(&ptxdesc->txdesc_queue);

        if (ptxdesc->txdesc_sta == drv_sta) {
            txlist->txlist_qcnt--;
            ptxdesc->txdesc_sta = NULL;

            pr_debug("free qid:%d, drv_sta:%p\n", ptxdesc->txinfo->queue_id, drv_sta);
            DRV_TXQ_UNLOCK(txlist);
            drv_tx_complete(drv_priv, ptxdesc, 0);
            DRV_TXQ_LOCK(txlist);

        } else {
            list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head_not_free);
        }
    }

    if (!list_empty(&txdesc_list_head_not_free)) {
        list_splice_tail_init(&txdesc_list_head_not_free, &txlist->txlist_q);
    }
    DRV_TXQ_UNLOCK(txlist);
}


void drv_txlist_flushfree(struct drv_private *drv_priv, unsigned char vid)
{
    int i;

    for (i = 0; i < HAL_NUM_TX_QUEUES; i++) {
        if (DRV_TXQUEUE_VALUE(drv_priv, i)) {
            drv_txlist_free_all_by_vid(drv_priv, &drv_priv->drv_txlist_table[i], vid);
        }
    }
    driv_ps_wakeup(drv_priv);
    drv_hal_settxqueueflush(vid);
    driv_ps_sleep(drv_priv);
}

void free_txlist_when_free_sta(struct drv_private *drv_priv, void *nsta)
{
    int i;
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);

    for (i = 0; i < HAL_NUM_TX_QUEUES; i++) {
        if (DRV_TXQUEUE_VALUE(drv_priv, i)) {
            drv_txlist_free_all_by_drv_sta(drv_priv, &drv_priv->drv_txlist_table[i], drv_sta);
        }
    }
}

void drv_set_pkt_drop(struct drv_private *drv_priv, unsigned char vid, unsigned char enable)
{
    drv_priv->drv_pkt_drop[vid] = enable;
}

void drv_set_is_mother_channel(struct drv_private *drv_priv, unsigned char vid, unsigned char enable)
{
    //pr_debug("vid:%d mother channel is:%d\n", vid, enable);
    drv_priv->is_mother_channel[vid] = enable;
}

void drv_free_normal_buffer_queue(struct drv_private *drv_priv, unsigned char vid)
{
    struct drv_txdesc *ptxdesc = NULL;

    DRV_TX_NORMAL_BUF_LOCK(drv_priv);
    while (!list_empty(&drv_priv->drv_normal_buffer_queue[vid]))
    {
        ptxdesc = list_first_entry(&drv_priv->drv_normal_buffer_queue[vid], struct drv_txdesc, txdesc_queue);
        list_del_init(&ptxdesc->txdesc_queue);
        drv_tx_complete(drv_priv, ptxdesc, 0);
        //pr_debug("%s\n", __func__);
    }

    drv_priv->drv_normal_buffer_count[vid] = 0;
    DRV_TX_NORMAL_BUF_UNLOCK(drv_priv);
}


void drv_flush_normal_buffer_queue(struct drv_private *drv_priv, unsigned char vid)
{
    struct drv_txlist *txlist;
    struct drv_txdesc *ptxdesc = NULL;
    struct list_head txdesc_list_head;
    struct aml_driver_nsta *drv_sta;
    struct wlan_net_vif *wnet_vif = NULL;

    INIT_LIST_HEAD(&txdesc_list_head);

    DRV_TX_NORMAL_BUF_LOCK(drv_priv);
    while (!list_empty(&drv_priv->drv_normal_buffer_queue[vid]))
    {
        ptxdesc = list_first_entry(&drv_priv->drv_normal_buffer_queue[vid], struct drv_txdesc, txdesc_queue);
        list_del_init(&ptxdesc->txdesc_queue);
        list_add_tail(&ptxdesc->txdesc_queue, &txdesc_list_head);

        drv_sta = ptxdesc->txdesc_sta;
        txlist = &drv_priv->drv_txlist_table[ptxdesc->txinfo->queue_id];

        ASSERT(txlist->txlist_qnum == ptxdesc->txinfo->queue_id);
        if (aml_tx_hal_buffer_full(drv_priv,txlist->txlist_qnum, 1, 1) == 1)
        {
            list_add_tail(&ptxdesc->txdesc_queue, &drv_priv->drv_normal_buffer_queue[vid]);
            DRV_TX_NORMAL_BUF_UNLOCK(drv_priv);
            return;
        }
        DRV_TX_NORMAL_BUF_UNLOCK(drv_priv);

        if (drv_sta == NULL) {
            continue;
        }

        if (drv_sta->net_nsta != NULL) {
            wnet_vif = ((struct wifi_station *)(drv_sta->net_nsta))->sta_wnet_vif;
        }

        if ((wnet_vif != NULL) && (wnet_vif->vm_state == WIFINET_S_CONNECTED)) {
            ptxdesc->txinfo->b_Ampdu = 0;
            if (drv_tx_normal(drv_priv, txlist, &txdesc_list_head, vid))
            {
                ASSERT(0);
                return;
            }
        } else {
            drv_tx_complete(drv_priv, ptxdesc, 0);
        }
        DRV_TX_NORMAL_BUF_LOCK(drv_priv);
        drv_priv->drv_normal_buffer_count[vid]--;
    }
    DRV_TX_NORMAL_BUF_UNLOCK(drv_priv);
}

unsigned short drv_tx_pending_pkt(struct drv_private *drv_priv)
{
    unsigned short pending_cnt = 0;
    unsigned char queue_id = 0;

    for (queue_id = 0; queue_id < HAL_NUM_TX_QUEUES; queue_id++)
    {
        if (DRV_TXQUEUE_VALUE(drv_priv, queue_id)) {
            pending_cnt += drv_priv->drv_txlist_table[queue_id].txlist_qcnt
                + drv_priv->drv_txlist_table[queue_id].txds_pending_cnt;// all frame in amlmain
        }
    }

    return pending_cnt;
}

unsigned int
drv_txlist_qcnt( struct drv_private *drv_priv, int queue_id)
{
    return drv_priv->drv_txlist_table[queue_id].txlist_qcnt ;
}

unsigned int drv_txlist_all_qcnt ( struct drv_private *drv_priv, int queue_id)
{
    unsigned int cnt = drv_priv->drv_txlist_table[queue_id].txlist_qcnt + drv_priv->drv_txlist_table[queue_id].txds_pending_cnt;
    cnt += drv_priv->drv_txlist_table[queue_id].txlist_backup_qcnt;
    return cnt;
}

int drv_txlist_isfull(struct drv_private *drv_priv, int queue_id, struct sk_buff *skbbuf, void *nsta)
{
    int tid_idx = 0;
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    int fifo_freecnt = 0;

    struct wifi_station *sta = (struct wifi_station *)drv_sta->net_nsta;

    if ((sta->sta_flags & WIFINET_NODE_UAPSD) &&
        drv_tx_uapsd_nsta_qcnt(sta->drv_sta) >= WME_UAPSD_NODE_MAXQDEPTH)
    {
        ERROR_DEBUG_OUT("<running> \n");
        return -1;
    }

    /*get fifo free num */
    fifo_freecnt = drv_priv->hal_priv->hal_ops.hal_get_priv_cnt(queue_id);

    tid_idx = drv_priv->net_ops->os_skb_get_tid(skbbuf);

    /*check if txlist is full by mpdu pending cnt is greater than
        (get_fifo free num + WIFI_MAX_TXFRAME/WIFI_MAX_TID))*/
    if(drv_priv->drv_txlist_table[queue_id].txds_pending_cnt
        > (fifo_freecnt + WIFI_MAX_TXFRAME/WIFI_MAX_TID + 48)
        || (fifo_freecnt < 2))
    {
        return 1;
    }
    return 0;
}

void drv_txdesc_set_rts_cts(struct drv_private *drv_priv, struct drv_txdesc *ptxdesc)
{
    unsigned char i;

    for (i = 0; i <= 2; ++i) {
        if (ptxdesc->txdesc_rateinfo[i].trynum) {
            break;
        }
    }

    if ((ptxdesc->txinfo->b_mgmt == 0) && (ptxdesc->txinfo->b_rsten == 0)
        && (drv_priv->drv_protect_mode != PROT_M_NONE) && (!IS_CCK_RATE(ptxdesc->txdesc_rateinfo[i].vendor_rate_code))
        && (ptxdesc->txinfo->b_noack == 0) && !(M_PWR_SAV_GET((struct sk_buff *)(ptxdesc->txdesc_mpdu))
        || M_FLAG_GET((struct sk_buff *)(ptxdesc->txdesc_mpdu), M_UAPSD)))
    {
        if (drv_priv->drv_protect_mode == PROT_M_RTSCTS)
            ptxdesc->txinfo->b_rsten = 1;
        else if (drv_priv->drv_protect_mode == PROT_M_CTSONLY)
            ptxdesc->txinfo->b_csten = 1;
    }

    if (drv_priv->drv_config.cfg_aggr_prot && ptxdesc->txinfo->b_aggr)
    {
        ptxdesc->txinfo->b_rsten = 1;
    }
}

int drv_aggr_check( struct drv_private *drv_priv, void * nsta, unsigned char tid_index)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_tx_scoreboard *tid;

    if (!drv_priv->drv_config.cfg_txaggr) {
        return 0;
    }
    tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);

    if (tid->cleanup_inprogress) {
        pr_debug("<running> %s %d \n", __func__, __LINE__);
        return 0;
    }

    if (!tid->addba_exchangecomplete) {
        if (!tid->addba_exchangeinprogress && (tid->addba_exchangeattempts < ADDBA_EXCHANGE_ATTEMPTS)) {
            tid->addba_exchangeattempts++;
            DPRINTF(AML_DEBUG_ADDBA,"<running> %s %d \n",__func__,__LINE__);
            return 1;
        }
    }

    return 0;
}

int drv_aggr_tid( struct drv_private *drv_priv, void * nsta, unsigned char tid_index)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_tx_scoreboard *tid;

    if (!drv_priv->drv_config.cfg_txaggr) {
        return 0;
    }
    tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);

    if (tid->cleanup_inprogress) {
        return 0;
    }

    if (!tid->addba_exchangecomplete) {
        if (tid->addba_exchangeinprogress) {
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}

int drv_aggr_allow_to_send(struct drv_private *drv_priv, void * nsta, unsigned char tid_index)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_tx_scoreboard *tid;

    tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);

    if (!drv_priv->drv_config.cfg_txaggr || tid->addba_exchangecomplete) {
        return 1;
    }

    if (tid->cleanup_inprogress || tid->addba_exchangeinprogress) {
        return 0;
    }

    return 1;
}

int drv_get_amsdu_supported(struct drv_private *drv_priv, void *nsta, int tid_index)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);

    if (drv_priv->drv_config.cfg_txaggr && drv_priv->drv_config.cfg_txamsdu && tid->addba_exchangecomplete) {
        return tid->addba_amsdusupported;
    }

    return 0;
}


static void
drv_addba_timer_ex(unsigned long param1,unsigned long param2,
    unsigned long param3,unsigned long param4,unsigned long param5)
{
    struct drv_tx_scoreboard *tid = (struct drv_tx_scoreboard *)param1;
    struct drv_private *drv_priv = (struct drv_private *)param2;

    DPRINTF(AML_DEBUG_ADDBA,"<running> %s %d  \n",__func__,__LINE__);

    driv_ps_wakeup(drv_priv);
    if (cmpxchg(&tid->addba_exchangeinprogress, 1, 0) == 1)
    {
        drv_txlist_flush_for_sta_tid(drv_priv, tid);
        tid->tid_tx_buff_sending = 1;
        wifi_mac_buffer_txq_send(&tid->tid_tx_buffer_queue);
        tid->tid_tx_buff_sending = 0;
    }

    driv_ps_sleep(drv_priv);

    os_timer_ex_cancel(&tid->addba_requesttimer, CANCEL_NO_SLEEP);

    return ;   /* don't re-arm itself */
}

// static unsigned int my_drvaddbatimer_taskid,my_drvaddbatimer_once=0;
static int
drv_addba_timer(void *arg)
{
    struct drv_tx_scoreboard *tid = (struct drv_tx_scoreboard *)arg;
    struct drv_private *drv_priv = tid->drv_sta->sta_drv_priv;

    drv_hal_add_workitem((WorkHandler)drv_addba_timer_ex,NULL, (SYS_TYPE)arg,(SYS_TYPE)drv_priv,0,0, 0);
    return OS_TIMER_NOT_REARMED;
}

void drv_addba_req_setup(struct drv_private *drv_priv, void * nsta, unsigned char tid_index,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short *batimeout, struct wifi_mac_ba_seqctrl *basequencectrl, unsigned short buffersize)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);

    baparamset->amsdusupported = (drv_priv->drv_config.cfg_txamsdu);
    baparamset->bapolicy = WIFINET_BA_POLICY_IMMEDIATE;
    baparamset->tid = tid_index;
    baparamset->buffersize = buffersize;
    *batimeout = 0;
    basequencectrl->fragnum = 0;
    basequencectrl->startseqnum = tid->seq_next;
    tid->seq_start = tid->seq_next;

    /* addba,addba_exchangeinprogress=1 */
    /* Start ADDBA request timer */
    if (cmpxchg(&tid->addba_exchangeinprogress, 0, 1) == 0) {
        if (!os_timer_ex_active(&tid->addba_requesttimer)) {
            DPRINTF(AML_DEBUG_ADDBA,"<running> %s %d \n",__func__,__LINE__);
            os_timer_ex_start(&tid->addba_requesttimer);
        }
    }
    DPRINTF(AML_DEBUG_ADDBA,"%s %d addba_exchangecomplete=%d ,addba_exchangeinprogress=%d\n",
            __func__,__LINE__,tid->addba_exchangecomplete , tid->addba_exchangeinprogress);
    drv_txlist_pause_for_sta_tid(drv_priv, tid);
}


void
drv_addba_rsp_process(
    struct drv_private *drv_priv, void * nsta,
    unsigned short statuscode,
    struct wifi_mac_ba_parameterset *baparamset,
    unsigned short batimeout)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct wifi_station *sta = (struct wifi_station *)drv_sta->net_nsta;
    unsigned short tid_index = baparamset->tid;
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);
    int resume = 1;

    if (baparamset->tid>= WME_NUM_TID)
    {
        DPRINTF(AML_DEBUG_ADDBA|AML_DEBUG_ERROR,"<running> %s %d TID too big %d \n",
                __func__,__LINE__,baparamset->tid);
        return;
    }

    if (cmpxchg(&tid->addba_exchangeinprogress, 1, 0) == 1)
    {
        os_timer_ex_cancel(&tid->addba_requesttimer, CANCEL_NO_SLEEP);
    }
    else
    {
        resume = 0;
    }

    tid->addba_exchangestatuscode = statuscode;

    if (statuscode == WIFINET_STATUS_SUCCESS)
    {
        DPRINTF(AML_DEBUG_ADDBA,"%s amsdusupported %d start:%04x, next:%04x\n",
            __func__, baparamset->amsdusupported, tid->seq_start, tid->seq_next);

        tid->addba_exchangecomplete = 1;
        tid->baw_size = baparamset->buffersize;//MIN(baparamset->buffersize, tid->baw_size);

        if (drv_priv->drv_config.cfg_txamsdu && baparamset->amsdusupported) {
            tid->addba_amsdusupported = 1;
            drv_priv->drv_config.cfg_ampdu_subframes = 16;

        } else {
            tid->addba_amsdusupported = 0;
            drv_priv->drv_config.cfg_ampdu_subframes = DEFAULT_TXAMPDU_SUB_MAX;
        }

        if (resume)
        {
            hal_phy_addba_ok(sta->sta_wnet_vif->wnet_vif_id, drv_hal_nsta_staid((struct wifi_station *)sta),
                tid_index, drv_sta->tx_agg_st.tid[tid_index].seq_start, tid->baw_size, BA_INITIATOR, BA_IMMIDIATE);
            drv_txlist_resume_for_sta_tid(drv_priv, tid);
        }
    }
    else
    {
        DPRINTF(AML_DEBUG_ADDBA,"<running> %s %d fail \n",__func__,__LINE__);

        if (resume)
        {
            drv_txlist_flush_for_sta_tid(drv_priv, tid);
        }
    }
}


unsigned short
drv_addba_status( struct drv_private *drv_priv, void * nsta, unsigned char tid_index)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);
    unsigned short status;


    if ((tid->addba_exchangestatuscode == WIFINET_STATUS_SUCCESS) &&
        !tid->addba_exchangecomplete)
    {
        status = 0xFFFF;
    }
    else
    {
        status = tid->addba_exchangestatuscode;
    }

    return status;
}


void
drv_addba_clear( struct drv_private *drv_priv, void * nsta)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);
    int i;
    struct drv_tx_scoreboard *tid;
    struct drv_rx_scoreboard *RxTidState;

    for (i = 0; i < ARRAY_LENGTH(drv_sta->tx_agg_st.tid); i++)
    {
        tid = &drv_sta->tx_agg_st.tid[i];
        if (tid->addba_exchangecomplete)
        {
            tid->addba_exchangecomplete = 0;
            tid->addba_exchangeattempts = 0;
            tid->addba_exchangestatuscode = WIFINET_STATUS_UNSPECIFIED;
            tid->paused = 0;
        }
    }

    for (i = 0; i < ARRAY_LENGTH(drv_sta->rx_scb); i++)
    {
        RxTidState = &drv_sta->rx_scb[i];
        if (RxTidState->rx_addba_exchangecomplete)
            RxTidState->rx_addba_exchangecomplete = 0;
    }
}

static void drv_tx_add2baw(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid, struct drv_txdesc *ptxdesc)
{
    int index, desc_id;

    index  = DRV_BA_INDEX(tid->seq_start, ptxdesc->txinfo->seqnum);
    desc_id = (tid->baw_head + index) & (DRV_TID_MAX_BUFS - 1);

    if (tid->tx_desc[desc_id] != NULL) {
        pr_debug("desc_id:%d, sta:%p\n", desc_id, tid->drv_sta->net_nsta);
        pr_debug("new seqnum=%d new tid_index=%d\n",  ptxdesc->txinfo->seqnum,  ptxdesc->txinfo->tid_index);
        pr_debug("tid->seq_start=%d", tid->seq_start);
    }

    tid->tx_desc[desc_id] = ptxdesc;
    tid->baw_sn_last = ptxdesc->txinfo->seqnum;

    if (index >= ((tid->baw_tail - tid->baw_head) & (DRV_TID_MAX_BUFS - 1))) {
        tid->baw_tail = desc_id;
        CIRCLE_Add_One(tid->baw_tail, DRV_TID_MAX_BUFS);
    }
}

static void drv_tx_update_ba_win(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid, int seqnum, struct drv_txlist *txlist)
{
    int index, desc_id;

    index  = DRV_BA_INDEX(tid->seq_start, seqnum);
    if (index > tid->baw_size)
    {
        return;
    }
    desc_id = (tid->baw_head + index) & (DRV_TID_MAX_BUFS - 1);

    tid->tx_desc[desc_id] = NULL;

    if (((tid->baw_tail - tid->baw_head) & (DRV_TID_MAX_BUFS - 1)) >= tid->baw_size - 1 - MIN(tid->baw_size/2, drv_priv->drv_config.cfg_ampdu_subframes))
    {
        if (tid->tx_desc[tid->baw_head] != NULL)
        {
            tid->tx_desc[tid->baw_head] = NULL;
            if (txlist->txlist_qcnt > 0)
                txlist->txlist_qcnt--;
        }
    }

    while ((tid->baw_head != tid->baw_tail) && (tid->tx_desc[tid->baw_head] == NULL))
    {
        CIRCLE_Add_One(tid->seq_start, WIFINET_SEQ_MAX);
        CIRCLE_Add_One(tid->baw_head, DRV_TID_MAX_BUFS);
    }
}

static void
drv_txlist_pause_for_sta_tid(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid)
{
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[tid->ac->queue_id];

    DPRINTF(AML_DEBUG_XMIT,"<running> %s %d    tid->paused++ \n",__func__,__LINE__);

    DRV_TXQ_LOCK(txlist);
    tid->paused++;
    DRV_TXQ_UNLOCK(txlist);
}


static void
drv_txlist_resume_for_sta_tid(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid)
{
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[tid->ac->queue_id];

    DRV_TXQ_LOCK(txlist);

    ASSERT(tid->paused > 0);
    DPRINTF(AML_DEBUG_XMIT,"<running> %s %d tid->paused--\n",__func__,__LINE__);
    tid->paused--;

    DPRINTF(AML_DEBUG_ADDBA, "%s tid->paused:%d\n",__func__, tid->paused);
    if (tid->paused > 0)
    {
        DRV_TXQ_UNLOCK(txlist);
        return;
    }


    if (list_empty(&tid->txds_queue))
    {
        DRV_TXQ_UNLOCK(txlist);
        return;
    }

    //connect tid to ac and ac to list
    drv_tx_queue_tid(txlist, tid);
    drv_txlist_task(drv_priv, txlist);
    DRV_TXQ_UNLOCK(txlist);
}

static void drv_txlist_flush_for_sta_tid(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid)
{
    struct drv_txlist *txlist = &drv_priv->drv_txlist_table[tid->ac->queue_id];
    struct drv_txdesc *ptxdesc;
    struct list_head txdesc_list_head;
    struct aml_driver_nsta *drv_sta = tid->drv_sta;
    struct wlan_net_vif *wnet_vif = NULL;

    if ((drv_sta != NULL) && (drv_sta->net_nsta != NULL)) {
        wnet_vif = ((struct wifi_station *)(drv_sta->net_nsta))->sta_wnet_vif;
    }

    INIT_LIST_HEAD(&txdesc_list_head);

    DRV_TXQ_LOCK(txlist);
    ASSERT(tid->paused > 0);
    tid->paused--;
    DPRINTF(AML_DEBUG_ADDBA, "%s queue_id:%d, pending:%d, paused:%d\n", __func__, tid->ac->queue_id, txlist->txds_pending_cnt, tid->paused);

    if (tid->paused > 0)
    {
        DRV_TXQ_UNLOCK(txlist);
        return;
    }

    while (!list_empty(&tid->txds_queue))
    {
        ptxdesc = list_first_entry(&tid->txds_queue, struct drv_txdesc, txdesc_queue);

        ASSERT(txlist->txlist_qnum == ptxdesc->txinfo->queue_id);

        txlist->txds_pending_cnt--;
        list_cut_position(&txdesc_list_head, &tid->txds_queue, &ptxdesc->txdesc_queue_lastframe->txdesc_queue);

        DRV_TXQ_UNLOCK(txlist);
        if ((wnet_vif != NULL) && (wnet_vif->vm_state == WIFINET_S_CONNECTED)) {
            ptxdesc->txinfo->b_Ampdu = 0;
            DPRINTF(AML_DEBUG_XMIT,"<running> %s %d \n",__func__,__LINE__);

            if (drv_tx_normal(drv_priv, txlist, &txdesc_list_head, wnet_vif->wnet_vif_id))
            {
                ASSERT(0);
                return;
            }

        } else {
            drv_tx_complete(drv_priv, ptxdesc, 0);
        }

        DPRINTF(AML_DEBUG_XMIT,"<running> %s %d \n",__func__,__LINE__);
        DRV_TXQ_LOCK(txlist);
    }

    DRV_TXQ_UNLOCK(txlist);
}

void
drv_txampdu_del(struct drv_private *drv_priv, 
    struct aml_driver_nsta *drv_sta, unsigned char tid_index)
{
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);
    struct drv_txlist *txlist = NULL;
    struct list_head txdesc_list_head;

    INIT_LIST_HEAD(&txdesc_list_head);

    if (tid->ac == NULL)
        return;
    txlist =&drv_priv->drv_txlist_table[tid->ac->queue_id];

    DRV_TXQ_LOCK(txlist);
    if (tid->cleanup_inprogress) {
        DRV_TXQ_UNLOCK(txlist);
        return;
    }

    if (!tid->addba_exchangecomplete) {
        tid->addba_exchangeattempts = 0;
        tid->addba_exchangestatuscode = WIFINET_STATUS_UNSPECIFIED;
        DRV_TXQ_UNLOCK(txlist);
        return;
    }

    DRV_TXQ_UNLOCK(txlist);
    if (cmpxchg(&tid->addba_exchangeinprogress, 1, 0) == 1)
        os_timer_ex_cancel(&tid->addba_requesttimer, CANCEL_NO_SLEEP);

    drv_txlist_pause_for_sta_tid(drv_priv, tid);
    DRV_TXQ_LOCK(txlist);

    if (tid->baw_head != tid->baw_tail) {
        tid->cleanup_inprogress = 1;

    } else {
        tid->addba_exchangecomplete = 0;
        tid->addba_exchangeattempts = 0;
        tid->addba_exchangestatuscode = WIFINET_STATUS_UNSPECIFIED;
    }
    DRV_TXQ_UNLOCK(txlist);
    drv_txlist_flush_for_sta_tid(drv_priv, tid);

    DPRINTF(AML_DEBUG_ADDBA, "%s baw_head:%d, baw_tail:%d, seq start:%04x, next:%04x, paused:%d, cleanup_inprogress:%d\n",
        __func__, tid->baw_head, tid->baw_tail, tid->seq_start, tid->seq_next, tid->paused, tid->cleanup_inprogress);
}


void drv_txrxampdu_del( struct drv_private *drv_priv, 
    void * nsta, unsigned char tid_index, unsigned char initiator)
{
    struct aml_driver_nsta *drv_sta = DRIVER_NODE(nsta);

    if (initiator)
        drv_txampdu_del(drv_priv, drv_sta, tid_index);
    else
        drv_rxampdu_del(drv_priv, drv_sta, tid_index);
}


static  void
drv_tx_queue_tid(struct drv_txlist *txlist, struct drv_tx_scoreboard *tid)
{
    struct drv_queue_ac *ac = tid->ac;
    if (tid->paused)
    {
        return;
    }

    if (tid->b_work)
    {
        return;
    }

    // connect tid 2 ac: ac->tid_queue->tid1->tid2....
    tid->b_work = 1;
    list_add_tail(&tid->tid_queue_elem, &ac->tid_queue);

    if (ac->b_work)
    {
        return;
    }

    // connect ac node 2 txlist: tx->txlist_acq->ac1->ac2...
    ac->b_work = 1;
    list_add_tail(&ac->ac_queue, &txlist->txlist_acq);
}

static int
drv_tx_normal(struct drv_private *drv_priv, struct drv_txlist *txlist, struct list_head *txdesc_list_head, unsigned char vid)
{
    struct drv_txdesc *ptxdesc;
    struct wifi_frame *wh;

    if (list_empty(txdesc_list_head))
    {
        ERROR_DEBUG_OUT("----------list empty -------------\n");
        return -1;
    }
    ptxdesc = list_first_entry(txdesc_list_head, struct drv_txdesc, txdesc_queue);
    wh = (struct wifi_frame *)os_skb_data(ptxdesc->txdesc_mpdu);

    if (!drv_priv->is_mother_channel[vid] && !WIFINET_IS_PROBEREQ(wh)) {
        pr_debug("%s vid:%d not mother channel, buffer\n", __func__, vid);

        DRV_TX_NORMAL_BUF_LOCK(drv_priv);
        list_add_tail(&ptxdesc->txdesc_queue, &drv_priv->drv_normal_buffer_queue[vid]);
        drv_priv->drv_normal_buffer_count[vid]++;
        DRV_TX_NORMAL_BUF_UNLOCK(drv_priv);

        return 0;
    }

    if (aml_tx_hal_buffer_full(drv_priv,txlist->txlist_qnum,1,(ptxdesc->txdesc_pktnum )) != 0)
    {
        DPRINTF(AML_DEBUG_XMIT, "%s %d hal_buffer full\n", __func__, __LINE__);
        drv_txq_backup_addbuf(drv_priv, txdesc_list_head, txlist);

    } else {
        if (drv_txq_backup_qcnt(txlist) > 0)
        {
            DPRINTF(AML_DEBUG_XMIT, "%s %d hal_buffer not full, normal_backup qcnt=%d\n",
                __func__, __LINE__, drv_txq_backup_qcnt(txlist));
            drv_txq_backup_addbuf(drv_priv, txdesc_list_head, txlist);
            drv_txq_backup_send(drv_priv, txlist);

        } else {
            DPRINTF(AML_DEBUG_XMIT,"%s rate_index:%d\n", __func__, ptxdesc->txdesc_rateinfo[0].rate_index);
            drv_priv->drv_stats.tx_normal_cnt++;

            ptxdesc->txinfo->b_Ampdu = 0;
            ptxdesc->txdesc_pktnum = 1;
            ptxdesc->txdesc_queue_last = ptxdesc->txdesc_queue_lastframe; /* one single frame */

            DRV_TXQ_LOCK(txlist);
            drv_to_hal(drv_priv, txlist, txdesc_list_head);
            DRV_TXQ_UNLOCK(txlist);
        }
    }

    return 0;
}

static int drv_tx_send_ampdu(struct drv_private *drv_priv, struct drv_txlist *txlist,
    struct drv_tx_scoreboard *tid, struct list_head *txdesc_list_head)
{
    struct drv_txdesc *ptxdesc;

    if (list_empty(txdesc_list_head))
    {
        ERROR_DEBUG_OUT("no data to send\n");
        return 0;
    }


    ptxdesc = list_first_entry(txdesc_list_head, struct drv_txdesc, txdesc_queue);
    drv_priv->drv_stats.tx_pkts_cnt++;

    /*
    counts = drv_tx_ack_optimize(drv_priv, tid, ptxdesc);
    txlist->txds_pending_cnt -= counts;*/

    //connect txdesc to tid->txds_queue
    list_splice_tail_init(txdesc_list_head, &tid->txds_queue);

    ASSERT(txlist->txlist_qnum == ptxdesc->txinfo->queue_id);

    drv_tx_queue_tid(txlist, tid);
    AML_PRINT(AML_DBG_MODULES_TX, "mpdu pending cnt:%d, seq:%d, len:%d, qid:%d, tid:%d, sn:0x%04x, timeout:%d rate:0x%02x\n", txlist->txds_pending_cnt, ptxdesc->txinfo->seqnum,
        ptxdesc->txdesc_agglen, ptxdesc->txinfo->queue_id, ptxdesc->txinfo->tid_index, ptxdesc->txinfo->seqnum, drv_priv->wait_mpdu_timeout,ptxdesc->txdesc_rateinfo[0].vendor_rate_code);

    DRV_HRTIMER_LOCK(drv_priv);

    if ((++txlist->txds_pending_cnt < drv_priv->drv_config.cfg_ampdu_subframes) && (!drv_priv->wait_mpdu_timeout)) {
        //pending pkts not enough, wait more pkts or scheduled by TX_OK.
        wifi_mac_tx_lock_timer_cancel();
        wifi_mac_tx_lock_timer_attach();

        DRV_HRTIMER_UNLOCK(drv_priv);
        return 0;
    }
    wifi_mac_tx_lock_timer_cancel();
    DRV_HRTIMER_UNLOCK(drv_priv);

#if 0// 1 schedule; 0 direct call
    tasklet_schedule(&drv_priv->ampdu_tasklet);
#else
    drv_txlist_task(drv_priv, txlist);
#endif
    return 0;
}

unsigned int drv_lookup_rate(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta,   struct aml_ratecontrol *txdesc_rateinfo)
{
    unsigned char i;
    unsigned short maxampdu;
    unsigned short aggr_limit;
    unsigned int maxampdulen, frame_length;
    struct wifi_station *sta = (struct wifi_station *)drv_sta->net_nsta;
    struct wlan_net_vif *wnet_vif = NULL;

    if (!sta) {
        DPRINTF(AML_DEBUG_WARNING, "%s sta is NULL\n", __func__);
        return 0;
    }
    wnet_vif = sta->sta_wnet_vif;

    if (wnet_vif != NULL) {
        if (wnet_vif->vm_state != WIFINET_S_CONNECTED) {
            DPRINTF(AML_DEBUG_WARNING, "%s state:%d\n", __func__, wnet_vif->vm_state);
            return 0;
        }

    } else {
        ERROR_DEBUG_OUT("wnet_vif is NULL\n");
        return 0;
    }
    DRV_MINSTREL_LOCK(drv_priv);
    if (!drv_priv->ratctrl_ops->rate_findrate(txdesc_rateinfo, sta)) {
        ERROR_DEBUG_OUT("get rate error, drop\n");
        DRV_MINSTREL_UNLOCK(drv_priv);
        return 0;
    }
    DRV_MINSTREL_UNLOCK(drv_priv);
    maxampdulen = WIFINET_AMPDU_LIMIT_MAX;

    for (i = 0; i < 3; i++)
    {
        if (IS_MCS_RATE(txdesc_rateinfo[i].vendor_rate_code) && txdesc_rateinfo[i].trynum)
        {
            //frame_length = txdesc_rateinfo[i].maxampdulen;
            frame_length = max_send_packet_len(txdesc_rateinfo[i].vendor_rate_code, 0,  0, 1);
            maxampdulen = MIN(maxampdulen, frame_length);
            DPRINTF(AML_DEBUG_XMIT,"function %s , line %d,rate =%02x, frame_length=%d, maxampdulen %d\n",\
             __func__, __LINE__,  txdesc_rateinfo[i].vendor_rate_code, frame_length,maxampdulen );
        }
    }

    aggr_limit = MIN(maxampdulen, drv_priv->drv_config.cfg_ampdu_limit);
    DPRINTF(AML_DEBUG_XMIT,"function %s, line %d, cfg_ampdu_limit=%d, maxampdulen =%d, aggr_limit=%d \n",
                    __func__, __LINE__, drv_priv->drv_config.cfg_ampdu_limit, maxampdulen, aggr_limit);

    maxampdu = drv_sta->tx_agg_st.maxampdu;
    if (maxampdu)
        aggr_limit = MIN(aggr_limit, maxampdu);

    DPRINTF(AML_DEBUG_XMIT, "%s vendor_rate_code=%x, trynum:%d:%d:%d\n",  __func__, txdesc_rateinfo[0].vendor_rate_code,
        txdesc_rateinfo[0].trynum, txdesc_rateinfo[1].trynum, txdesc_rateinfo[2].trynum);
    return aggr_limit;
}

static const unsigned int bits_per_symbol[][2] =
{
    /* 20MHz 40MHz */
    {    26,   54 },     //  0: BPSK
    {    52,  108 },     //  1: QPSK 1/2
    {    78,  162 },     //  2: QPSK 3/4
    {   104,  216 },     //  3: 16-QAM 1/2
    {   156,  324 },     //  4: 16-QAM 3/4
    {   208,  432 },     //  5: 64-QAM 2/3
    {   234,  486 },     //  6: 64-QAM 3/4
    {   260,  540 },     //  7: 64-QAM 5/6
    {    52,  108 },     //  8: BPSK
    {   104,  216 },     //  9: QPSK 1/2
    {   156,  324 },     // 10: QPSK 3/4
    {   208,  432 },     // 11: 16-QAM 1/2
    {   312,  648 },     // 12: 16-QAM 3/4
    {   416,  864 },     // 13: 64-QAM 2/3
    {   468,  972 },     // 14: 64-QAM 3/4
    {   520, 1080 },     // 15: 64-QAM 5/6
};

static  int
drv_compute_num_delims(struct drv_private *drv_priv, struct drv_txdesc *ptxdesc, unsigned short packetlen)
{
    unsigned int nsymbits, nsymbols;
    int width, half_gi;
    int ndelim, mindelim;
    unsigned short minlen;
    unsigned char flags;
    unsigned int mpdudensity;

    ndelim = DRV_AGGR_GET_NDELIM(packetlen);
    mpdudensity = DRIVER_NODE(ptxdesc->txdesc_sta)->tx_agg_st.mpdudensity;

    if (mpdudensity == 0)
    {
        return ndelim;
    }

    flags = ptxdesc->txdesc_rateinfo[0].flags;
    width = (flags & HAL_RATECTRL_CW40_FLAG) ? 1 : 0;
    half_gi = (flags & HAL_RATECTRL_SGI_FLAG) ? 1 : 0;

    if (half_gi)
    {
        nsymbols=NUM_SYMBOLS_PER_USEC_HALFGI(mpdudensity);
    }
    else
    {
        nsymbols=NUM_SYMBOLS_PER_USEC(mpdudensity);
    }

    if (nsymbols == 0)
    {
        nsymbols = 1;
    }

    nsymbits = bits_per_symbol[HT_RC_2_MCS(ptxdesc->txdesc_rateinfo[0].vendor_rate_code)][width];
    minlen = (nsymbols * nsymbits) / BITS_PER_BYTE;

    if (packetlen < minlen)
    {
        mindelim = (minlen - packetlen) / DRV_AGGR_DELIM_SZ;
        ndelim = MAX(mindelim, ndelim);
    }

    return ndelim;
}

static int
drv_txampdu_build(struct drv_private *drv_priv, struct drv_tx_scoreboard *tid,
    struct list_head *tx_queue, struct drv_txdesc **bf_last, struct drv_txlist *txlist)
{
    struct drv_txdesc *ptxdesc, *bf_first, *bf_prev = NULL;
    struct list_head txdesc_list_head;
    int nframes = 0, ndelim;
    unsigned short aggr_limit = 4096, al = 0, bpad = 0, al_delta, h_baw = tid->baw_size/2;
    int status = 0;
    int prev_al = 0;
    unsigned short txdesc_aggr_page_num = 0;
    unsigned char use_sample_rate = 0;

    INIT_LIST_HEAD(&txdesc_list_head);

    bf_first = list_first_entry(&tid->txds_queue, struct drv_txdesc, txdesc_queue);
    if (!BAW_WITHIN(tid->seq_start, tid->baw_size, bf_first->txinfo->seqnum + MIN(h_baw, drv_priv->drv_config.cfg_ampdu_subframes))) {
        return status;
    }

    use_sample_rate = ((bf_first->txdesc_rateinfo[0].flags & HAL_RATECTRL_USE_SAMPLE_RATE) != 0) ? 1 : 0;
    if (!use_sample_rate) {
        if (drv_priv->minstrel_sample_rate[0].trynum != 0) {
            memcpy(&bf_first->txdesc_rateinfo, &drv_priv->minstrel_sample_rate, DRV_TXDESC_RATE_NUM * sizeof(struct aml_ratecontrol));
            memset(&drv_priv->minstrel_sample_rate, 0, DRV_TXDESC_RATE_NUM * sizeof(struct aml_ratecontrol));
        }
    }

    if (bf_first->rate_valid == 0) {
        aggr_limit = drv_lookup_rate(drv_priv,  tid->drv_sta, (bf_first->txdesc_rateinfo));

    } else if(bf_first->rate_valid == 1) {
        aggr_limit = MIN(bf_first->txdesc_rateinfo[0].maxampdulen, bf_first->txdesc_rateinfo[1].maxampdulen);
        aggr_limit = MIN(bf_first->txdesc_rateinfo[2].maxampdulen, aggr_limit);
    }

    AML_PRINT(AML_DBG_MODULES_TX, "seq_start %d, cur_sn %d last_sn = %d, pending:%d, wait_mpdu_timeout:%d, aggr_limit:%d\n",
        tid->seq_start, bf_first->txinfo->seqnum, tid->baw_sn_last, txlist->txds_pending_cnt, drv_priv->wait_mpdu_timeout, aggr_limit);

    do {
        ptxdesc = list_first_entry(&tid->txds_queue, struct drv_txdesc, txdesc_queue);

        /*backup sample rate*/
        if ((nframes != 0) && (ptxdesc->txdesc_rateinfo[0].flags & HAL_RATECTRL_USE_SAMPLE_RATE)) {
            memcpy(&drv_priv->minstrel_sample_rate, &ptxdesc->txdesc_rateinfo, DRV_TXDESC_RATE_NUM * sizeof(struct aml_ratecontrol));
        }

        if (!BAW_WITHIN(tid->seq_start, tid->baw_size, ptxdesc->txinfo->seqnum)) {
            status = 1;
            break;
        }

        al_delta = DRV_AGGR_DELIM_SZ + ptxdesc->txinfo->packetlen;
        if (nframes && (aggr_limit < (al + bpad + al_delta + prev_al))) {
            status = 2;
            break;
        }

        if (nframes >= MIN(h_baw, drv_priv->drv_config.cfg_ampdu_subframes)) {
            status = 2;
            break;
        }

        txdesc_aggr_page_num += drv_priv->hal_priv->hal_ops. hal_calc_mpdu_page(ptxdesc->txinfo->mpdulen); //fixed!1011
        al += bpad + al_delta;
        ndelim = drv_compute_num_delims(drv_priv, bf_first, ptxdesc->txinfo->packetlen);
        bpad = PADBYTES(al_delta) + (ndelim << 2);
        ptxdesc->txdesc_delimit = ndelim;

        if (txlist->txds_pending_cnt > 0)
            txlist->txds_pending_cnt--;

        list_cut_position(&txdesc_list_head,&tid->txds_queue, &ptxdesc->txdesc_queue_lastframe->txdesc_queue);
        drv_tx_add2baw(drv_priv, tid, ptxdesc);

        if ((drv_priv->drv_config.cfg_ampduackpolicy & BIT(tid->tid_index)) == 0) {
            DRV_SET_TX_SET_BLOCKACK_POLICY(drv_priv, (struct wifi_qos_frame *) os_skb_data(ptxdesc->txdesc_mpdu));
            ptxdesc->txinfo->b_noack = 1;
        }

        list_splice_tail_init(&txdesc_list_head,tx_queue);
        nframes ++;
        if (bf_prev) {
            bf_prev->txdesc_queue_next = ptxdesc;
        }
        bf_prev = ptxdesc;
    }
    while (!list_empty(&tid->txds_queue));

    bf_first->txdesc_agglen = al;
    bf_first->txdesc_pktnum = nframes;
    bf_first->txdesc_aggr_page_num = txdesc_aggr_page_num;
    *bf_last = bf_prev;
    return status;
}

static void drv_txampdu_tasklet(unsigned long arg)
{
    struct drv_private *drv_priv = (struct drv_private *)arg;
    struct drv_txlist *txlist = NULL;
    unsigned char i = 0;
    unsigned char wait_mpdu_timeout = drv_priv->wait_mpdu_timeout;
    unsigned char wait_mpdu_timeout_status_change = 0;
    static int s_txampdu_send_flag = 0;

    if (drv_priv->drv_config.cfg_txaggr == 0 || s_txampdu_send_flag)
    {
        return;
    }

    s_txampdu_send_flag = 1;

    DRV_TX_TIMEOUT_LOCK(drv_priv);
    for (i = 0; i < HAL_WME_NOQOS; i++)
    {
        txlist = &drv_priv->drv_txlist_table[i];

        DRV_TXQ_LOCK(txlist);
        drv_txlist_task(drv_priv, txlist);

        if (wait_mpdu_timeout) {
            if (!drv_priv->wait_mpdu_timeout) {
                wait_mpdu_timeout_status_change = 1;
            }
            drv_priv->wait_mpdu_timeout = 1;
        }

        if ((i == HAL_WME_AC_VO) && wait_mpdu_timeout_status_change) {
            /* BYPASS */
            drv_priv->wait_mpdu_timeout = 0;
        }
        DRV_TXQ_UNLOCK(txlist);
    }
    DRV_TX_TIMEOUT_UNLOCK(drv_priv);

    s_txampdu_send_flag = 0;
}

static void drv_tx_sched_aggr(struct drv_private *drv_priv, struct drv_txlist *txlist,
    struct drv_tx_scoreboard *tid)
{
    struct drv_txdesc *ptxdesc, *bf_last, *bf_lastaggr = NULL;
    int status, aggr_num;
    struct list_head tx_queue;
    int ret = 0;
    struct list_head *txdesc_queue = NULL;
    unsigned char hal_co_get_cnt;

    if (!drv_priv->is_mother_channel[tid->vid]) {
        //pr_debug("%s not mother channel vid:%d\n", __func__, tid->vid);
        return;
    }

    INIT_LIST_HEAD(&tx_queue);
    while (1)
    {
        if (txlist->txds_pending_cnt < drv_priv->drv_config.cfg_ampdu_subframes)
        {
            if (!drv_priv->wait_mpdu_timeout) {
                //not allowed to form ampdu when pending mpdu less than cfg_ampdu_subframes
                break;

            } else {
                //compel to form ampdu when pending mpdu less than cfg_ampdu_subframes except hal pending too much data
                hal_co_get_cnt = drv_priv->hal_priv->hal_ops.hal_get_priv_cnt(txlist->txlist_qnum);
                if (HI_AGG_TXD_NUM_PER_QUEUE - hal_co_get_cnt >= drv_priv->drv_config.cfg_ampdu_subframes) {
                    AML_PRINT(AML_DBG_MODULES_TX, "wait more mpdu to form ampdu due to hal_co_get_cnt:%d\n", hal_co_get_cnt);
                    drv_priv->wait_mpdu_timeout = 0;//wait another tx_ok or pending enough mpdu
                    break;
                }
            }
        }

        if (list_empty(&tid->txds_queue))
        {
            break;
        }

        if (aml_tx_hal_buffer_full(drv_priv, txlist->txlist_qnum,1,drv_priv->drv_config.cfg_ampdu_subframes))
        {
            break;
        }

        //get tx_ds from txlist  to local tx_queue
        status = drv_txampdu_build(drv_priv, tid, &tx_queue, &bf_lastaggr, txlist);

        if (list_empty(&tx_queue))
        {
            break;
        }

        ptxdesc = list_first_entry(&tx_queue, struct drv_txdesc, txdesc_queue);

        aggr_num = ptxdesc->txdesc_pktnum;
        txlist->tx_aggr_status[aggr_num-1]++;

        txdesc_queue = (&tx_queue)->prev;
        bf_last = list_entry(txdesc_queue, struct drv_txdesc, txdesc_queue);

        ptxdesc->txdesc_queue_last = bf_last;
        ptxdesc->txinfo->b_11n = 1;
        drv_priv->wait_mpdu_timeout = 0;

        if (ptxdesc->txdesc_pktnum == 1)
        {
            drv_priv->drv_stats.tx_ampdu_one_frame_cnt++;
            ASSERT(ptxdesc->txdesc_queue_lastframe == bf_last);

            ptxdesc->txinfo->b_aggr = 0;

            if (drv_priv->drv_config.cfg_ampdu_oneframe==0)
            {
                ptxdesc->txdesc_agglen = ptxdesc->txinfo->packetlen;
                ptxdesc->txdesc_aggr_page_num = drv_priv->hal_priv->hal_ops. hal_calc_mpdu_page(ptxdesc->txinfo->mpdulen);
                ptxdesc->txinfo->b_Ampdu = 0;
            }

            DPRINTF(AML_DEBUG_INFO,"<running> %s %d tid->seq_start %x, ptxdesc->txinfo->seqnum %x \n",__func__,__LINE__,tid->seq_start, ptxdesc->txinfo->seqnum);
            DPRINTF(AML_DEBUG_INFO,"<running> %s %d ptxdesc->txdesc_pktnum=1,tidq_qcnt %d, txlist_qcnt %d \n",__func__,__LINE__,txlist->txds_pending_cnt,txlist->txlist_qcnt);
            ret = drv_to_hal(drv_priv, txlist, &tx_queue);
            KASSERT(ret>=0, ("drv_to_hal"));
            break;
        }

        ptxdesc->txinfo->b_aggr  = 1;
        drv_priv->drv_stats.tx_ampdu_cnt++;

        ASSERT(bf_lastaggr);
        ASSERT(bf_lastaggr->txdesc_queue_lastframe == bf_last);

        ret=drv_to_hal(drv_priv, txlist, &tx_queue);
        KASSERT(ret>=0, ("drv_to_hal"));
    }
}

//txlist->acq->ac1->ac2...
//ac->tid1->tid2..
//
void drv_txlist_task(struct drv_private *drv_priv, struct drv_txlist *txlist)
{
    struct drv_queue_ac *ac;
    struct drv_tx_scoreboard *tid;

    if (list_empty(&txlist->txlist_acq))
    {
        return;
    }

    // get ac from txlist by data type (index)
    ac = list_first_entry(&txlist->txlist_acq, struct drv_queue_ac, ac_queue);
    list_del_init(&ac->ac_queue);
    ac->b_work = 0;

  //get one tid node  and process the txdesc which linked on the AC
    do
    {
        if (list_empty(&ac->tid_queue))
        {
            return ;
        }

        tid = list_first_entry(&ac->tid_queue, struct drv_tx_scoreboard, tid_queue_elem);
        list_del_init(&tid->tid_queue_elem);
        tid->b_work = 0;

        if (tid->paused)
        {
            continue;
        }

        //process one tid get from ac->tid_queue, agg only happens by tid unit
        drv_tx_sched_aggr(drv_priv, txlist, tid);

        if (!list_empty(&tid->txds_queue))
        {
            drv_tx_queue_tid(txlist, tid);
        }
        break;
    }
    while (!list_empty(&ac->tid_queue));

    if (!list_empty(&ac->tid_queue))
    {
        if (ac->b_work == 0)
        {
            ac->b_work = 1;
            list_add_tail(&ac->ac_queue, &txlist->txlist_acq);
        }
    }
}

static void
drv_tid_swq_cleanup(struct drv_private *drv_priv,
    struct drv_txlist *txlist, struct drv_tx_scoreboard *tid)
{
    struct drv_txdesc *ptxdesc;

    for (;;)
    {
        if (list_empty(&tid->txds_queue))
        {
            break;
        }
        ptxdesc = list_first_entry(&tid->txds_queue, struct drv_txdesc, txdesc_queue);
        list_del_init(&ptxdesc->txdesc_queue);

        txlist->txds_pending_cnt--;

        DRV_TXQ_UNLOCK(txlist);
        drv_tx_complete(drv_priv, ptxdesc, 0);
        DRV_TXQ_LOCK(txlist);
    }
}

static void
drv_tid_drain(struct drv_private *drv_priv,
    struct drv_txlist *txlist,
    struct drv_tx_scoreboard *tid)
{
    drv_tid_swq_cleanup(drv_priv, txlist, tid);

    tid->seq_next = tid->seq_start;
    tid->baw_tail = tid->baw_head;
    pr_warn("%s baw_head %x\n", __func__, tid->baw_head);
}

static void
drv_tid_cleanup(struct drv_private *drv_priv,
    struct drv_txlist *txlist,
    struct drv_tx_scoreboard *tid)
{
    drv_tid_swq_cleanup(drv_priv, txlist, tid);

    if (tid->cleanup_inprogress)
        return;

    if (tid->baw_head != tid->baw_tail)
    {
        tid->paused++;
        tid->cleanup_inprogress = 1;
    }
    else
    {
        tid->addba_exchangecomplete = 0;
        tid->addba_exchangeattempts = 0;
        tid->addba_exchangestatuscode = WIFINET_STATUS_UNSPECIFIED;
    }
}

static void drv_txlist_push_pending(struct drv_private *drv_priv, struct drv_txlist *txlist, unsigned char vid)
{
    struct drv_queue_ac *ac = NULL;
    struct drv_tx_scoreboard *tid;
    struct list_head ac_not_release;
    unsigned char ac_need_flush = 0;

    INIT_LIST_HEAD(&ac_not_release);
    while (!list_empty(&txlist->txlist_acq)) {
        ac = list_first_entry(&txlist->txlist_acq, struct drv_queue_ac, ac_queue);
        list_del_init(&ac->ac_queue);
        ac->b_work = 0;

        while (!list_empty(&ac->tid_queue))
        {
            tid = list_first_entry(&ac->tid_queue, struct drv_tx_scoreboard, tid_queue_elem);
            if ((tid->vid == vid) || (vid == 3)) {
                 list_del_init(&tid->tid_queue_elem);
                 tid->b_work = 0;
                 drv_tid_drain(drv_priv, txlist, tid);
                 ac_need_flush = 1;

             } else {
                 ac_need_flush = 0;
                 break;
             }
         }

         if (!ac_need_flush) {
             list_add_tail(&ac->ac_queue, &ac_not_release);
             ac->b_work = 1;
         }
     }

     if (!list_empty(&ac_not_release)) {
         list_splice_tail_init(&ac_not_release, &txlist->txlist_acq);
     }
}

void drv_txlist_init_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta, unsigned char vid)
{
    struct drv_tx_scoreboard *tid;
    struct drv_queue_ac *ac;
    int tid_index, acindex;

    drv_sta->tx_agg_st.maxampdu = drv_priv->drv_config.cfg_ampdu_limit;

    for (tid_index = 0, tid = &drv_sta->tx_agg_st.tid[tid_index]; tid_index < WME_NUM_TID; tid_index++, tid++)
    {
        tid->drv_sta = drv_sta;
        tid->vid = vid;
        tid->tid_index = tid_index;
        tid->baw_size = WME_MAX_BA;
        INIT_LIST_HEAD(&tid->txds_queue);

        acindex = TID_TO_WME_AC(tid_index);
        tid->ac = &drv_sta->tx_agg_st.ac[acindex];
        os_timer_ex_initialize(&tid->addba_requesttimer, ADDBA_TIMEOUT, drv_addba_timer, tid);
        WIFINET_SAVEQ_INIT(&tid->tid_tx_buffer_queue, "tid_tx_buffer_queue");
        tid->addba_exchangestatuscode = WIFINET_STATUS_UNSPECIFIED;
    }

    for (acindex = 0, ac = &drv_sta->tx_agg_st.ac[acindex]; acindex < WME_NUM_AC; acindex++, ac++)
    {
        INIT_LIST_HEAD(&ac->tid_queue);

        switch (acindex)
        {
            case WME_AC_BE:
                ac->queue_id = HAL_WME_AC_BE;
                break;
            case WME_AC_BK:
                ac->queue_id = HAL_WME_AC_BK;
                break;
            case WME_AC_VI:
                ac->queue_id = HAL_WME_AC_VI;
                break;
            case WME_AC_VO:
                ac->queue_id = HAL_WME_AC_VO;
                break;
        }
    }

}


void drv_txlist_cleanup_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    int i;
    struct drv_queue_ac *ac = NULL, *ac_next = NULL;
    struct drv_tx_scoreboard *tid;
    struct drv_txlist *txlist;

    for (i = 0; i < HAL_NUM_TX_QUEUES; i++)
    {
        if (DRV_TXQUEUE_VALUE(drv_priv, i))
        {
            txlist = &drv_priv->drv_txlist_table[i];
            DRV_TXQ_LOCK(txlist);
            if (list_empty(&txlist->txlist_acq))
            {
                DRV_TXQ_UNLOCK(txlist);
                continue;
            }

            list_for_each_entry_safe(ac,ac_next,&txlist->txlist_acq,ac_queue)
            {
                if (list_empty(&ac->tid_queue))
                {
                    continue;
                }
                tid = list_first_entry(&ac->tid_queue, struct drv_tx_scoreboard, tid_queue_elem);
                if (tid && tid->drv_sta != drv_sta)
                {
                    continue;
                }
                list_del_init(&ac->ac_queue);
                ac->b_work = 0;

                while (!list_empty(&ac->tid_queue))
                {
                    tid = list_first_entry(&ac->tid_queue, struct drv_tx_scoreboard, tid_queue_elem);
                    list_del_init(&tid->tid_queue_elem);
                    tid->b_work = 0;
                    if (cmpxchg(&tid->addba_exchangeinprogress, 1, 0) == 1)
                    {
                        os_timer_ex_cancel(&tid->addba_requesttimer, CANCEL_NO_SLEEP);
                        tid->paused--;
                    }
                    drv_tid_cleanup(drv_priv, txlist, tid);
                }
            }
            DRV_TXQ_UNLOCK(txlist);
        }
    }

    for (i = 0; i < HAL_NUM_TX_QUEUES; i++)
    {
        if (DRV_TXQUEUE_VALUE(drv_priv, i))
        {
            txlist = &drv_priv->drv_txlist_table[i];
            drv_txq_backup_cleanup(drv_priv, txlist, drv_sta);
        }
    }
}


void drv_txlist_free_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    if (drv_priv->drv_config.cfg_txaggr) {
        struct drv_tx_scoreboard *tid;
        int tid_index;

        for (tid_index = 0, tid = &drv_sta->tx_agg_st.tid[tid_index]; tid_index < WME_NUM_TID; tid_index++, tid++) {
            WIFINET_SAVEQ_DESTROY(&tid->tid_tx_buffer_queue);
            os_timer_ex_del(&tid->addba_requesttimer, CANCEL_NO_SLEEP);
        }
    }

    free_txlist_when_free_sta(drv_priv, drv_sta);
}

void drv_txlist_pause_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta)
{
    int tid_index;
    struct drv_tx_scoreboard *tid;

    if (!drv_priv->drv_config.cfg_txaggr )

        return;

    for (tid_index = 0, tid = &drv_sta->tx_agg_st.tid[tid_index]; tid_index < WME_NUM_TID;
         tid_index++, tid++)
    {
        drv_txlist_pause_for_sta_tid(drv_priv,tid);
    }
}

void drv_txlist_resume_for_sta(struct drv_private *drv_priv,
                                    struct aml_driver_nsta *drv_sta)
{
    int tid_index;
    struct drv_tx_scoreboard *tid;

    if (!drv_priv->drv_config.cfg_txaggr)
        return;

    for (tid_index = 0, tid = &drv_sta->tx_agg_st.tid[tid_index]; tid_index < WME_NUM_TID;
         tid_index++, tid++)
    {
        if (tid->addba_exchangecomplete)
        {
            drv_txlist_resume_for_sta_tid(drv_priv,tid);
        }
        else
        {
            drv_txlist_flush_for_sta_tid(drv_priv,tid);
        }
    }
}

void drv_set_ampduparams( struct drv_private *drv_priv,
                void * nsta, unsigned short maxampdu,unsigned int mpdudensity)
{
    DRIVER_NODE(nsta)->tx_agg_st.maxampdu = maxampdu;
    DRIVER_NODE(nsta)->tx_agg_st.mpdudensity = mpdudensity;
}

int drv_tx_get_mgmt_frm_rate(struct drv_private *drv_priv,
    struct wlan_net_vif *wnet_vif, unsigned char fc_type,unsigned char *rate,unsigned short *flag)
{
    switch (wnet_vif->vm_mac_mode)
    {
        case WIFINET_MODE_11B:
        case WIFINET_MODE_11BG:
        case WIFINET_MODE_11BGN:
            *rate = WIFI_11B_1M;
            *flag = CHAN_BW_20M;
            break;

        case WIFINET_MODE_11N:
        case WIFINET_MODE_11AC:
        case WIFINET_MODE_11NAC:
            *rate = WIFI_11N_MCS0;
            *flag = CHAN_BW_20M;
            break;

        case WIFINET_MODE_AUTO:
        case WIFINET_MODE_11G:
        case WIFINET_MODE_11GN:
        case WIFINET_MODE_11GNAC:
            *rate = WIFI_11G_6M;
            *flag = CHAN_BW_20M;
            break;

        default:
            pr_err("<running> %s %d  vm_mac_mode =%d ERROR\n",__func__,__LINE__, wnet_vif->vm_mac_mode);
            break;
    }
    return 0;
}
