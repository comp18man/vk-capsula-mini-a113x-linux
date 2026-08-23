/*****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     Driver interface source file
 * Author : Boatman Yang(xuexing.yang@amlogic.com)
 *
 * Date:    20160901
 ****************************************************************************************
 */
#include "wifi_mac_com.h"
#include "wifi_hal_com.h"
#include "wifi_pt_if.h"
#include "wifi_pt_init.h"

static struct _Pool our_txpriv_pool;
static void *our_txpriv_pool_buffer[HI_TXPRIVD_NUM_ALL_TID + 1];
unsigned char our_txpriv_descriptors_buffer[DRIVER_TXPRIV_DESC_LEN];

int send_frame_num = 0;
int recv_frame_num = 0;

static void Do_make_amsdu_header(struct _WIFI_htframe *htframe,unsigned char *buffer, int len)
{
    unsigned char tmp[1000];
    unsigned short msdulen = len;

    /* buffer is body, len is bodylen */

    htframe->QosCtrl[0] |= BIT(7);
    memcpy(tmp,buffer,len);
    memcpy(buffer,htframe->Address1,6);
    memcpy(buffer+6,htframe->Address2,6);
    WRITE_16B(buffer+12,msdulen);
    /* memcpy(buffer+12,&msdulen,2); */
    memcpy(buffer+14,tmp,len);
}

void Driver_intr_tx_handle(void *drv_priv, struct txdonestatus *tx_done_status,
                           SYS_TYPE callback, unsigned char queue_id)
{
    unsigned char *buffer;

    buffer = (unsigned char*)(unsigned int *)OS_DMAA2DDRddress((void *)(unsigned long)callback);
    /* Here,do not need destroy it now. */
    if (buffer != NULL)
        //TxBuffer_Destroy(buffer);

    send_frame_num++;

    if ((send_frame_num > STA2_VMAC1_SEND_FRAME_NUM) && (STA2_VMAC1_SEND_FRAME_NUM != 99999)) {
        /* pr_debug("send complete> %d status %d \n",status, STA2_VMAC1_SEND_FRAME_NUM); */
        /* Test_Done(1); */
    }
    /*
     * if one frame send fail ,end test
     */
    if (tx_done_status->txstatus != TX_DESCRIPTOR_STATUS_SUCCESS) {
        /* pr_debug("send complete status %d \n",status); */
        /* Test_Done(0); */
    }
}

void Driver_mic_error_event(void *drv_priv, const void *wh, unsigned char *sa,
                            unsigned char vmac_id)
{
    DBG_HAL_THR_ENTER();
    DBG_HAL_THR_EXIT();
}

void Driver_intr_rx_handle(void *drv_prv, struct sk_buff *skb, unsigned char Rssi,
                           unsigned char RxRate, unsigned char bw, unsigned char channel,
                           unsigned char aggr, unsigned char wnet_vif_id, unsigned char keyid)
{
    recv_frame_num++;

    Net_Receive((unsigned char*)os_skb_data(skb), os_skb_get_pktlen(skb), Rssi);
    os_skb_free(skb);
    return;
}

void Driver_intr_dtim_send(void *dev, unsigned char vid)
{
    DBG_HAL_THR_ENTER();
    DBG_HAL_THR_EXIT();
}

void Driver_intr_bcn_send(void *dev, unsigned char vid)
{
    DBG_HAL_THR_ENTER();
    DBG_HAL_THR_EXIT();
}

void Driver_intr_ba_recv(void *dev, unsigned char vid)
{
    DBG_HAL_THR_ENTER();
    DBG_HAL_THR_EXIT();
}

void drv_intr_bt_info_change(void* dpriv, unsigned char wnet_vif_id, unsigned char bt_lk_change)
{
    DBG_HAL_THR_ENTER();
    DBG_HAL_THR_EXIT();
}

int Do_HI_AGG_TxPriv_TYPE_AMSDU(struct _HI_TxPrivDescripter_chain* HI_TxPriv[],
                                unsigned char* skbptr[], int length[], int packetNum)
{
    int i;
    int headerlength, total;
    struct _WIFI_htframe * htframe;
    int msdulen = 0;
    unsigned char **buffer;

    htframe = (struct _WIFI_htframe *)os_skb_data((struct sk_buff *)skbptr[0]);//? not right. please change it
    headerlength = MacFrame_GetHeaderSize( htframe->FrameCtrl );
    total = headerlength;

    for (i = 0; i < packetNum; i ++)
    {
#if STA2_VMAC1_TKIPMIC_HW
        if(i != packetNum-1)
        {
            length[i] -= 8;/* tkip mic */
        }
#endif /* STA2_VMAC1_TKIPMIC_HW */
        /* pr_debug("length[%d]= %d \n ",i,length[i]); */

        buffer = &os_skb_data((struct sk_buff *)skbptr[i]);

        Do_make_amsdu_header(htframe, *(unsigned char **)((unsigned long)buffer + i) + headerlength,
                             length[i] - headerlength);
        msdulen = ALIGN(length[i] - headerlength + 14, 4);
        total += msdulen;
        memcpy(os_skb_data((struct sk_buff *)skbptr[0]) + total,
                           *(unsigned char **)((unsigned long)buffer + i) + headerlength,
                           msdulen);
    }

    length[0] = total;
    Do_HI_AGG_TxPriv_TYPE_AMPDU(HI_TxPriv, skbptr, length, 1);

    return 1;
}

void Do_HI_AGG_TxPriv_TYPE_AMPDU(struct _HI_TxPrivDescripter_chain* HI_TxPriv[],
                                 unsigned char* skbptr[], int length[], int packetNum)
{
    int i;

    for (i = 0; i < packetNum; i ++)
    {
        unsigned int datalen;
        unsigned short headerlength;
        unsigned char FLAG;
        struct _WIFI_htframe *htframe;
        unsigned char *buffer;
        unsigned int frame_control;

        buffer = os_skb_data((struct sk_buff *)skbptr[i]);
        frame_control = *(unsigned int *)buffer;
        headerlength = MacFrame_GetHeaderSize(frame_control);
        htframe = (struct _WIFI_htframe *)buffer;
        datalen = length[i];
        FLAG = 0;
        FLAG |=WIFI_MORE_AGG;
        FLAG |=WIFI_FIRST_BUFFER;
        FLAG &= ~WIFI_MORE_BUFFER;
        if(packetNum == 1)
        {
            FLAG &= ~WIFI_MORE_AGG;
        }
        else if( i==(packetNum - 1) )
        {
            FLAG &= ~WIFI_MORE_AGG;
        }
        HI_TxPriv[i]->HI_TxPriv.DMAADDR = (SYS_TYPE)skbptr[i];
        HI_TxPriv[i]->HI_TxPriv.DDRADDR = (unsigned long)buffer;
        /* pr_debug("HI_TxPriv.DDRADDR %p length=0x%x\n",buffer[i], length[i]); */
        HI_TxPriv[i]->HI_TxPriv.DMALEN = length[i];
        HI_TxPriv[i]->HI_TxPriv.MPDULEN = length[i];
        HI_TxPriv[i]->HI_TxPriv.Delimiter = 0;
        ASSERT(HI_TxPriv[i]->HI_TxPriv.DMALEN > 0);
        HI_TxPriv[i]->HI_TxPriv.Flag = FLAG;
        HI_TxPriv[i]->HI_TxPriv.Seqnum = (*(unsigned short *)htframe->SeqCtrl) >> 4;
        HI_TxPriv[i]->HI_TxPriv.HTC = *((unsigned short *)htframe->HtCtrl);
        HI_TxPriv[i]->HI_TxPriv.FrameControl = htframe->FrameCtrl;
        /* PRINT("htframe->FrameCtrl %x,fc %x \n",htframe->FrameCtrl,frame_control); */
        HI_TxPriv[i]->HI_TxPriv.TX_STATUS = TX_DESCRIPTOR_STATUS_NEW;
        HI_TxPriv[i]->HI_TxPriv.ShortRetryNum = 0;
        HI_TxPriv[i]->HI_TxPriv.LongRetryNum = 0;
        HI_TxPriv[i]->HI_TxPriv.ACKRSSI = 0;
        HI_TxPriv[i]->HI_TxPriv.TimeStmp = 0;
        HI_TxPriv[i]->HI_TxPriv.hostreserve = (unsigned long)OS_DDR2DMAAddress((unsigned int*)buffer);
        #ifdef UNO_B2B_SUPPORT
        HI_TxPriv[i]->HI_TxPriv.PrivDmaAddr = (unsigned long)OS_DDR2DMAAddress(&HI_TxPriv[i]->HI_TxPriv);
        #endif
        HI_TxPriv[i]->buffer = buffer;
        /* OS_DCACHE_WRITE_TO_DEV((unsigned int*)buffer[i],length[i]); */
    }
}


void Do_HI_AGG_TxPriv_TYPE_COMMO(struct _HI_TxPrivDescripter_chain* HI_TxPriv,
                                 unsigned char* skbptr, int length,int packetNum)
{
    unsigned char FLAG;
    unsigned short headerlength;
    struct _WIFI_htframe *htframe;
    unsigned int frame_control;
    unsigned int datalen = 0;
    unsigned char *buffer = os_skb_data((struct sk_buff *)skbptr);

    DBG_HAL_THR_ENTER();

    frame_control = *(unsigned int*)buffer;
    headerlength = MacFrame_GetHeaderSize(frame_control);
    htframe = (struct _WIFI_htframe *)buffer;
    datalen = length;
    FLAG = THR_WIFI_MORE_AGG;
    FLAG |= THR_WIFI_FIRST_BUFFER;
    FLAG &= ~THR_WIFI_MORE_BUFFER;
    FLAG &= ~THR_WIFI_MORE_AGG;

    HI_TxPriv->HI_TxPriv.DMAADDR = (SYS_TYPE)skbptr;
    HI_TxPriv->HI_TxPriv.DDRADDR = (unsigned long)buffer;
    HI_TxPriv->HI_TxPriv.DMALEN = length;
    HI_TxPriv->HI_TxPriv.MPDULEN = length;
    HI_TxPriv->HI_TxPriv.Delimiter = 0;
    HI_TxPriv->HI_TxPriv.Flag = FLAG;
    HI_TxPriv->HI_TxPriv.Seqnum = (*(unsigned short *)htframe->SeqCtrl) >> 4;
    HI_TxPriv->HI_TxPriv.HTC = *((unsigned short *)htframe->HtCtrl);
    HI_TxPriv->HI_TxPriv.FrameControl = htframe->FrameCtrl;
    /* PRINT("htframe->FrameCtrl %x,fc %x \n",htframe->FrameCtrl,frame_control); */
    HI_TxPriv->HI_TxPriv.TX_STATUS = 0; /*TX_DESCRIPTOR_STATUS_NEW */
    HI_TxPriv->HI_TxPriv.ShortRetryNum = 0;
    HI_TxPriv->HI_TxPriv.LongRetryNum = 0;
    HI_TxPriv->HI_TxPriv.ACKRSSI = 0;
    HI_TxPriv->HI_TxPriv.TimeStmp = 0;
    HI_TxPriv->HI_TxPriv.hostreserve = (unsigned long)THR_OS_DDR2DMAAddress((unsigned int*)buffer);
    HI_TxPriv->buffer = buffer;

    DBG_HAL_THR_EXIT();
}

struct _HI_TxPrivDescripter_chain * Driver_GetTxPriv(void)
{
    struct _HI_TxPrivDescripter_chain *HI_TxPrivDp;

    HI_TxPrivDp = (struct _HI_TxPrivDescripter_chain*)Pool_GetBlock(&our_txpriv_pool);
    if (!HI_TxPrivDp)
        return 0;

    memset(HI_TxPrivDp, 0, sizeof(struct _HI_TxPrivDescripter_chain));

    return HI_TxPrivDp;
}

static void new_list_del(struct thr_list_head * prev, struct thr_list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

void Driver_CreatTxPriv(void)
{
    unsigned char *buffer = (unsigned char*)(((unsigned long)our_txpriv_descriptors_buffer + 7) & ~0x7);

    Pool_Create(&our_txpriv_pool,
                sizeof(struct _HI_TxPrivDescripter_chain),
                HI_TXPRIVD_NUM_ALL_TID,
                buffer,
                our_txpriv_pool_buffer,
                "driverTxPriv");
}

void Driver_FreeAGG(struct _HI_AGG_TxDescripter_chain *HI_AGG_TxDp)
{
    DBG_HAL_THR_ENTER();
    /* OS_DEL_HEAD(HI_AGG_TxDp->workList); */
    new_list_del(&HI_AGG_TxDp->workList,&HI_AGG_TxDp->workList);
    FREE(HI_AGG_TxDp,"getagg");
    HI_AGG_TxDp = NULL;
    DBG_HAL_THR_EXIT();
}

extern struct hal_layer_ops *FiOpt2Driver;
unsigned char Driver_FillAgg(struct _HI_AGG_TxDescripter_chain *HI_AGG_TxDp)
{
    int i = 0;
    struct thr_list_head *head;
    struct _HI_TxPrivDescripter_chain *HI_TxPrivDp;
    struct sk_buff *Hi_Agg_Txd_Is_Over = NULL;
    struct hal_private *hal_priv = hal_get_priv();

    DBG_HAL_THR_ENTER();

    AML_TXLOCK_LOCK();
    list_for_each(head, &HI_AGG_TxDp->workList)
    {
        HI_TxPrivDp = (struct _HI_TxPrivDescripter_chain *)list_entry(head, struct _HI_AGG_TxDescripter_chain, workList);

        if (i == 0) {
            Hi_Agg_Txd_Is_Over = FiOpt2Driver->hal_fill_agg_start(&HI_AGG_TxDp->HI_AGG_TxD,&HI_TxPrivDp->HI_TxPriv);
            if (Hi_Agg_Txd_Is_Over == NULL) {
                AML_TXLOCK_UNLOCK();
                goto AGGFREE;
            }

            Pool_PutBlock(&our_txpriv_pool, (void *)HI_TxPrivDp);
        } else {
            Hi_Agg_Txd_Is_Over = FiOpt2Driver->hal_fill_priv(&HI_TxPrivDp->HI_TxPriv,HI_AGG_TxDp->HI_AGG_TxD.TID);
            if (Hi_Agg_Txd_Is_Over == NULL){
                AML_TXLOCK_UNLOCK();
                goto AGGFREE;
            }

            Pool_PutBlock(&our_txpriv_pool, (void *)HI_TxPrivDp);
        }

        i++;
    }
    AML_TXLOCK_UNLOCK();

    up(&hal_priv->hi_irq_thread_sem);
AGGFREE:
    Driver_FreeAGG(HI_AGG_TxDp);
    DBG_HAL_THR_EXIT();

    return 0;
}

struct _HI_AGG_TxDescripter_chain *Driver_GetAGG(void)
{
    struct _HI_AGG_TxDescripter_chain *HI_AGG_TxDp;

    HI_AGG_TxDp = (struct _HI_AGG_TxDescripter_chain *)ZMALLOC(sizeof(struct _HI_AGG_TxDescripter_chain),
                                                               "getagg", GFP_ATOMIC);

    if (!HI_AGG_TxDp)
        return NULL;

    return HI_AGG_TxDp;
}

void Driver_ListInit(struct _HI_AGG_TxDescripter_chain *HI_AGG_TxDp)
{
        OS_LIST_INIT(&HI_AGG_TxDp->workList);
}

int Driver_IsTxPrivEnough(int num)
{
    if (our_txpriv_pool.min_count > num)
    {
        /* pr_debug("---xman debug: the min_count is :%d.\n",our_txpriv_pool.min_count); */
        return 1;
    }
    else
    {
        pr_debug("---xman debug: the min_count is :%d.\n",our_txpriv_pool.min_count);
        return 0;
    }
}

