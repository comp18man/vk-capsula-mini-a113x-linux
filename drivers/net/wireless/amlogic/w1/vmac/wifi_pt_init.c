/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     Initialization work before testing HAL layer throughput performance
 * Author : Boatman Yang(xuexing.yang@amlogic.com)
 *
 * Date:    20160901
 ****************************************************************************************
 */
#include <linux/string.h>
#include "wifi_mac_com.h"
#include "wifi_pt_init.h"
#include "wifi_pt_network.h"
#include "wifi_pt_if.h"
#include "wifi_hal.h"
#include "wifi_drv_main.h"

extern int send_frame_num;
extern int recv_frame_num;

struct aml_hal_call_backs callback;



static struct B2B_Tx_Task_Struct b2b_tx_struct;

//Added for B2B test case packet information init
/////////////////////////////////////////type,burst,ampdu,encry, tkip, tcpip, pkts_num, rate, bw, shortGI,ldpc_enable, channel,pkt_len;
struct _B2B_Test_Case_Packet gB2BTestCasePacket={1,    0,    0,    0,    0,     0,     1500,   0x04, 0,  0,         1,           6,   1200};
struct _B2B_Platform_Conf gB2BPlatformConf={0};

void Queue_Create( struct _Queue* my, void* buffer[], unsigned int size )
{
        ASSERT( size > 1 );

        my->start = buffer;
        my->end   = buffer + size - 1;
        my->front = buffer;
        my->back  = buffer;
}

enum {
        ACCESS_TX_QUEUE_SIZE = 128,
};


enum {
        NET_STATE_IDLE,
        NET_STATE_SEND_PING,
        NET_STATE_SEND_UDP,
        NET_STATE_SEND_SIM_TEST,
        NET_STATE_SEND_TCP,
};

struct _TRC_CONF_MIB    TrcConfMib;
extern struct _Network     my_network[WIFI_MAX_VID];
static struct _TxTask        myTxTask;
static struct _Pool           our_tx_descriptor_pool;
static void*                    our_tx_descriptor_pool_buffer[ TX_DESCRIPTOR_COUNT + 1 ];
static unsigned char         our_tx_descriptors[ TX_DESCRIPTOR_SIZE * TX_DESCRIPTOR_COUNT ];

static char *pKey="12345678123456781234567812345678123456781234567857657343545334343534567765466344378592345723875698";

struct hal_layer_ops* FiOpt2Driver;

unsigned int Queue_GetCount( struct _Queue* my )
{
        void** front = my->front;
        void** back  = my->back;

        return front <= back ? back - front : ( my->end - my->start ) + 1 - ( front - back );
}


void* Queue_Dequeue( struct _Queue* my )
{
        void** front = my->front;

        if ( front != my->back ) {
                void* object = *front;

                front = ( front == my->end ) ? my->start : front + 1;

                my->front = front;                  // Change of my->front must be the last!
                return object;
        }

        return 0;
}



static unsigned char Queue_HasObject( struct _Queue* my, void* object )
{
        void** start = my->start;
        void** end   = my->end;
        void** front = my->front;
        void** back  = my->back;

        while ( front != back ) {
                if ( *front == object ) return true;

                front = ( front == end ) ? start : front + 1;
        }
        return false;
}



static unsigned char Queue_IsFull( struct _Queue* my )
{
        void** front = my->front;
        void** back  = my->back;

        return (back + 1 == front )|| (back == my->end && front == my->start);
}

unsigned char Queue_Enqueue( struct _Queue* my, void* object )
{
        //
        // Prohibit enqueuing the same object twice
        //
        void** back ;
        void** next;
        ASSERT( !Queue_HasObject( my, object ) );

        back = my->back;
        next = ( back == my->end ) ? my->start : back + 1;

        if ( next != my->front ) {
                *back = object;

                my->back = next;                    // Change of my->back must be the last!
                return true;
        }

        return false;
}


unsigned char Pool_PutBlock( struct _Pool* my, void* block )
{
        unsigned char* p = ( unsigned char* )block;
        if ( my->buffer <= p && p < my->buffer + my->size * my->max_count ) {
                //
                // Check that p is aligned to the start of a block
                //
                ASSERT( !Queue_IsFull( &my->queue ) && ( p - my->buffer ) % my->size == 0 );

                //
                // This function will ASSERT if block is already in the queue!
                //
                Queue_Enqueue( &my->queue, block );
                return true;
        }
        return false;
}

#if 1

static void TxDescriptor_Destroy( struct _TxDescriptor* my )
{

        DBG_HAL_THR_ENTER();

        Pool_PutBlock( &our_tx_descriptor_pool, my );

        DBG_HAL_THR_EXIT();
}

unsigned char* Pool_GetBlock( struct _Pool* my )
{
        unsigned char* block = (unsigned char*)Queue_Dequeue( &my->queue );
        unsigned int count = Queue_GetCount( &my->queue );

        if ( count < my->min_count ) {
                my->min_count = count;
        }

        return block;
}

struct _TxDescriptor* TxDescriptor_Create(void)
{
    struct _TxDescriptor *descriptor = ( struct _TxDescriptor* )Pool_GetBlock( &our_tx_descriptor_pool );
    ASSERT( descriptor != NULL );
    //
    // FIXME: initialize other fields as well?
    //

    return descriptor;
}

static unsigned char uno_rate[RATE_LEN + 1] =
{
    WIFI_11B_1M      ,
    WIFI_11N_MCS0    ,
    WIFI_11B_5M      ,
    WIFI_11N_MCS1    ,
    WIFI_11G_6M      ,
    WIFI_11N_MCS0    ,
    WIFI_11G_12M     ,
    WIFI_11N_MCS7    ,
    WIFI_11G_24M     ,
    WIFI_11N_MCS0    ,
    WIFI_11B_1M      ,
    WIFI_11B_1M      ,
    WIFI_11N_MCS0    ,
    WIFI_11B_1M      ,
    WIFI_11N_MCS2    ,
    WIFI_11B_1M      ,
    WIFI_11N_MCS4    ,
    WIFI_11B_1M      ,
    WIFI_11N_MCS6    ,
    WIFI_11B_1M      ,
    WIFI_11N_MCS0
};

static void thr_list_add(struct thr_list_head *new_head,
                                struct thr_list_head *prev,
                                struct thr_list_head *next)
{
        next->prev = new_head;
        new_head->next = next;
        new_head->prev = prev;
        prev->next = new_head;
}

static struct _HI_TxPrivDescripter_chain*
Do_HI_AGG_TxDP(struct _HI_AGG_TxDescripter_chain  *HI_AGG_chain,
                    struct _HI_TxPrivDescripter_chain* HI_TxPriv[],
                    int length[],unsigned char TID,
                    unsigned short FLAG,int HI_TxPrivNum,int MpduNum)
{
    static unsigned int rate_index = 0;
    int exlen=0;
    int i;
    struct hi_agg_tx_desc* HI_AGG_TxDP = &HI_AGG_chain->HI_AGG_TxD;

    DBG_HAL_THR_ENTER();

    HI_AGG_TxDP->RateTableMode = 0;
    HI_AGG_TxDP->TxPower = 0x64;
    HI_AGG_TxDP->StaId = 0x1;
    HI_AGG_TxDP->KeyId = 0;
    HI_AGG_TxDP->EncryptType = TrcConfMib.dot11EncryptType;
    HI_AGG_TxDP->TID = TID;
    HI_AGG_TxDP->queue_id = TID;

    if(++rate_index >= RATE_LEN)
        rate_index = 0;

    if(gB2BTestCasePacket.data_rate != 0xff)
        HI_AGG_TxDP->CurrentRate = gB2BTestCasePacket.data_rate;
    else
        HI_AGG_TxDP->CurrentRate = uno_rate[rate_index];

    HI_AGG_TxDP->TxTryRate1 = 0;
    HI_AGG_TxDP->TxTryRate2 = 0;
    HI_AGG_TxDP->TxTryRate3 = 0;
    HI_AGG_TxDP->TxTryNum0 = 3;
    HI_AGG_TxDP->TxTryNum1 = 2;
    HI_AGG_TxDP->TxTryNum2 = 1;
    HI_AGG_TxDP->TxTryNum3 = 0;
    HI_AGG_TxDP->FLAG = FLAG;
    //PRINT("HI_AGG_TxDP->FLAG 0x%x b40M %d rate 0x%x\n",HI_AGG_TxDP->FLAG,STA1_VMAC0_SEND_40M, STA1_VMAC0_SEND_RATE);

    //clear flag field before setting!
    HI_AGG_TxDP->FLAG &= (~WIFI_CHANNEL_BW_MASK);

     if (gB2BTestCasePacket.channel_bw == SW_CBW80)
    {
          HI_AGG_TxDP->FLAG |= (SW_CBW80 << WIFI_CHANNEL_BW_OFFSET);
    }
    else if(gB2BTestCasePacket.channel_bw == SW_CBW40)
    {
         HI_AGG_TxDP->FLAG  |=(SW_CBW40 << WIFI_CHANNEL_BW_OFFSET);
    }
    else if(gB2BTestCasePacket.channel_bw == SW_CBW20)
    {
         HI_AGG_TxDP->FLAG |=(SW_CBW20 << WIFI_CHANNEL_BW_OFFSET);
    }
    else
    {
            ERROR_DEBUG_OUT("not support, bw in flag 0x%x\n", (HI_AGG_TxDP->FLAG >> 8) & 0x3);
     }

    //pr_debug("%s (%d) bw in flag 0x%x\n",__func__, __LINE__, (HI_AGG_TxDP->FLAG >> 8) & 0x3);

    HI_AGG_TxDP->FLAG2  = TrcConfMib.dot11RDSupport ? TX_DESCRIPTER_RD_SUPPORT : 0;
    HI_AGG_TxDP->FLAG2 |= TrcConfMib.dot11RDSupport ? TX_DESCRIPTER_HTC : 0;
    if(STA2_VMAC1_SEND_TID_CTRL==2) {
        //HI_AGG_TxDP->FLAG2 |= TX_DESCRIPTER_MBA;
    }

    if(gB2BTestCasePacket.tkip_mic){
        HI_AGG_TxDP->FLAG2 |= (TrcConfMib.dot11EncryptType==WIFI_TKIP)?TX_DESCRIPTER_MIC:0;
    }

    if(gB2BTestCasePacket.ldpc_enable)
        HI_AGG_TxDP->FLAG2 |= TX_DESCRIPTER_ENABLE_TX_LDPC;
    else
        HI_AGG_TxDP->FLAG2 &= ~TX_DESCRIPTER_ENABLE_TX_LDPC;

#ifdef STA2_TCPIP_CHECKSUM
    if(gB2BTestCasePacket.tcpip_csum){
        HI_AGG_TxDP->FLAG2 |= TX_DESCRIPTER_CHECKSUM;
    }
#endif //STA2_TCPIP_CHECKSUM
    HI_AGG_TxDP->MpduNum = MpduNum;

    HI_AGG_TxDP->TxTryNum3 = 0;
    HI_AGG_TxDP->AGGR_len = 0;
    HI_AGG_TxDP->aggr_page_num = 0;
    HI_AGG_TxDP->vid=0;

    switch(TrcConfMib.dot11EncryptType){
    case WIFI_WEP64:
        exlen = (DP_SEC_WEP_EXP_LEN);
        break;
    case WIFI_TKIP:
        exlen = (DP_SEC_TKIP_MPDU_EXP_LEN);
        break;
    case WIFI_AES:
        exlen=(DP_SEC_CCMP_EXP_LEN);
        break;
    case WIFI_WEP128:
        exlen=(DP_SEC_WEP_EXP_LEN);
        break;
    case WIFI_WPI:
        exlen=(DP_SEC_WAPI_EXP_LEN);
        break;
    }

    if (FLAG & WIFI_IS_AGGR) {
        for (i = 0; i< MpduNum-1; i++) {
            HI_AGG_TxDP->AGGR_len += ALIGN(length[i]+exlen,4)+4+4+0;
            HI_AGG_TxDP->aggr_page_num += howmanypage(length[i]+FW_TXDESC_DATAOFFSET, PAGE_LEN);
            //pr_debug("HI_AGG_TxDP->AGGR_len 0x%x ,length[%d] 0x%x\n",HI_AGG_TxDP->AGGR_len,i,length[i]);
        }

        if (IS_VHT_RATE(HI_AGG_TxDP->CurrentRate)) {
            //for vht, the last subframe also need padding
            // HI_AGG_TxDP->AGGR_len += ALIGN(length[i]+exlen,4)+4+4+0;
            HI_AGG_TxDP->AGGR_len += ((unsigned long)length[i]+exlen+4+4);//delimiter +fcslength
        }else {
            HI_AGG_TxDP->AGGR_len += ((unsigned long)length[i]+exlen+4+4);//delimiter +fcslength
        }

        HI_AGG_TxDP->aggr_page_num += howmanypage(length[i]+FW_TXDESC_DATAOFFSET, PAGE_LEN);
        //pr_debug("current vht rate 0x%x\n",HI_AGG_TxDP->CurrentRate);
    }
    else {
        HI_AGG_TxDP->AGGR_len += exlen;
        HI_AGG_TxDP->AGGR_len += HI_TxPriv[0]->HI_TxPriv.MPDULEN;
        HI_AGG_TxDP->AGGR_len +=4/*fcs*/;
        HI_AGG_TxDP->aggr_page_num = howmanypage(HI_TxPriv[0]->HI_TxPriv.MPDULEN+FW_TXDESC_DATAOFFSET, PAGE_LEN);
    }

    for (i = 0; i< MpduNum; i++){
        if(gB2BTestCasePacket.tkip_mic == 1){ //new
            HI_TxPriv[i]->HI_TxPriv.MPDULEN -= DP_SEC_TKIP_MIC_LEN;
        }
    }
    // pr_debug("HI_AGG_TxDP->AGGR_len %x ,MPDULEN %x\n",HI_AGG_TxDP->AGGR_len,HI_TxPriv[0]->HI_TxPriv.MPDULEN);
    // pr_debug("---xman debug---: Before OS_ADD_TAIL, HI_TxPrivNum: %d.\n",HI_TxPrivNum);
    for (i = 0; i < HI_TxPrivNum ; i ++) {
        // pr_debug(" Do_HI_AGG_TxDP +HI_TxPriv =%p DMAADDR =%x\n",HI_TxPriv[i],HI_TxPriv[i]->HI_TxPriv.DMAADDR);
        //OS_ADD_TAIL(&HI_TxPriv[i]->workList,&HI_AGG_chain->workList);
        thr_list_add(&HI_TxPriv[i]->workList,(&HI_AGG_chain->workList)->prev,&HI_AGG_chain->workList);
    }

    DBG_HAL_THR_EXIT();
    return HI_TxPriv[0];
}

static int HostSendTYPE_AMSDU(unsigned char* buffer[],int length[],
    int packetNum,unsigned char TID,unsigned short FLAG)
{
        //int i;
        int prinum =1;
        struct _HI_AGG_TxDescripter_chain* Hi_TxAgg;
        struct _HI_TxPrivDescripter_chain* HI_TxPriv[2];
        if (!Driver_IsTxPrivEnough(1)) {
                return 0;
        }
        Hi_TxAgg = Driver_GetAGG();
        if (Hi_TxAgg==NULL)
                return false;

        Driver_ListInit(Hi_TxAgg);

        HI_TxPriv[0] = Driver_GetTxPriv();

        //Do_HI_AGG_TxDP(Hi_TxAgg,HI_TxPriv,*buffer,length,TID,FLAG,packetNum  +  1,1, vmac_id);
        Do_HI_AGG_TxPriv_TYPE_AMSDU( HI_TxPriv,buffer,length,packetNum);
        Do_HI_AGG_TxDP(Hi_TxAgg,HI_TxPriv,length,TID,FLAG,prinum,1);
        Driver_FillAgg(Hi_TxAgg);
        return 0;

}

static int HostSendTYPE_AMPDU(unsigned char* buffer[],int length[],
    int packetNum,unsigned char TID,unsigned short FLAG)
{
        int i;
        struct _HI_AGG_TxDescripter_chain *Hi_TxAgg = NULL;
        struct _HI_TxPrivDescripter_chain **HI_TxPriv = NULL;
        int privnum = packetNum;
        if (!Driver_IsTxPrivEnough(packetNum)) {
                pr_warn("warning: no tx desc\n");
                return 0;
        }
        Hi_TxAgg = Driver_GetAGG();
        //NULL will be returned
        if (Hi_TxAgg==NULL)
                return false;

        Driver_ListInit(Hi_TxAgg);
        HI_TxPriv = (struct _HI_TxPrivDescripter_chain**)ZMALLOC(sizeof(struct _HI_TxPrivDescripter_chain*)*(packetNum),"hst_ampdu", GFP_ATOMIC);//xman modified
        for ( i = 0; i < packetNum ; i ++) {
                HI_TxPriv[i] =  Driver_GetTxPriv();
                ASSERT(HI_TxPriv[i]!= NULL);
        }

        Do_HI_AGG_TxPriv_TYPE_AMPDU(HI_TxPriv,buffer, length, packetNum);
        Do_HI_AGG_TxDP(Hi_TxAgg,HI_TxPriv,length,TID,FLAG,privnum,packetNum);
        Driver_FillAgg(Hi_TxAgg);
        FREE(HI_TxPriv,"hst_ampdu");
        HI_TxPriv = NULL;
        return 0;
}

static int  HostSendTYPE_COMMO(unsigned char* buffer,int length,unsigned char TID,unsigned short FLAG)
{

    int privnum =1;
    struct _HI_AGG_TxDescripter_chain* Hi_TxAgg;
    struct _HI_TxPrivDescripter_chain* HI_TxPriv[2];

    DBG_HAL_THR_ENTER();

    if (!Driver_IsTxPrivEnough(1)) {
        return false;
    }

    Hi_TxAgg = Driver_GetAGG();
    if (Hi_TxAgg==NULL)
            return false;

    Driver_ListInit(Hi_TxAgg);
    HI_TxPriv[0] =  Driver_GetTxPriv();
    ASSERT(HI_TxPriv[0]!= NULL);
    Do_HI_AGG_TxPriv_TYPE_COMMO(HI_TxPriv[0],buffer,length,1);
    Do_HI_AGG_TxDP(Hi_TxAgg,HI_TxPriv,&length,TID,FLAG,privnum,1);
    Driver_FillAgg(Hi_TxAgg);
    DBG_HAL_THR_EXIT();

    return true;
}
#endif

static void HostSendDataPacket(int type, int packetNum, unsigned short FLAG)
{
        int i;
        struct _TxDescriptor* descriptor = NULL;
        struct _TxDescriptor* descriptor1 = NULL;
        unsigned char **buffer = (unsigned char **)ZMALLOC(sizeof(unsigned char *)*32,"hsdpbuf", GFP_ATOMIC);
        int* length = (int*)ZMALLOC(sizeof(int)*32,"hsdplen", GFP_ATOMIC);
        unsigned char   TID = 0;

        DBG_HAL_THR_ENTER();

        if ( ( type!=TYPE_COMMON ) && ( type!=TYPE_COMMON_NOACK ) && ( type!=TYPE_BURST_ACK) )
        {
            if (packetNum>=32)
                packetNum = 32;
            for (i = 0; i < packetNum; i++ )
            {
                descriptor =  (struct _TxDescriptor*)Net_GetNextPacketDes();
                ASSERT(descriptor != NULL);
                if (descriptor==NULL)
                {
                    break;
                }
                buffer[i] = (unsigned char *)descriptor->skb;
                length[i] = descriptor->len;
                TID = descriptor->Tid;
                TxDescriptor_Destroy(descriptor);
            }
            packetNum = i;
            if (packetNum ==0) {
                pr_debug(" HostSendDataPacket packetNum=0\n");
                goto end;
            }
        }

        switch ( type ) {
        case TYPE_COMMON_NOACK:
                FLAG |= WIFI_IS_NOACK;
                FLAG &= (~WIFI_IS_AGGR);
                descriptor1 = (struct _TxDescriptor*)Net_GetNextPacketDes();
                ASSERT(descriptor1 != NULL);
                if (descriptor1 == NULL)
                    goto end;
                //if (TrcConfMib.dot11FragmentationThreshold < descriptor1->len )
                 //       FLAG = WIFI_IS_FRAGMENT;
                //pr_debug("---b2b debug ---Common type before HostSendTYPE_COMMO, FLAG is : %d\n", FLAG);
                HostSendTYPE_COMMO((unsigned char *)descriptor1->skb,descriptor1->len, descriptor1->Tid,FLAG);
                TxDescriptor_Destroy(descriptor1);
                break;
        case TYPE_COMMON:
                FLAG &= (~WIFI_IS_AGGR);
                descriptor1 = (struct _TxDescriptor*)Net_GetNextPacketDes();
                ASSERT(descriptor1 != NULL);
                if (descriptor1 == NULL)
                    goto end;
                //if (TrcConfMib.dot11FragmentationThreshold < descriptor1->len )
                 //       FLAG = WIFI_IS_FRAGMENT;
                //pr_debug("---b2b debug ---Common type before HostSendTYPE_COMMO, FLAG is : %d\n", FLAG);
                HostSendTYPE_COMMO((unsigned char *)descriptor1->skb,descriptor1->len, descriptor1->Tid,FLAG);
                TxDescriptor_Destroy(descriptor1);
                break;
        case TYPE_AMPDU:
                //pr_debug("----length: %d, packetNum: %d, TID: %d, FLAG: %d.\n",*length,packetNum, TID, FLAG);
                FLAG |= WIFI_IS_AGGR;
                HostSendTYPE_AMPDU( buffer,length,packetNum, TID, FLAG);
                break;
        case TYPE_AMSDU:
                pr_debug("\n---xman debug---,length:%d, packetNum:%d, TID: %d, FLAG:%x.\n",*length,packetNum, TID, FLAG);
                HostSendTYPE_AMSDU(buffer,length,packetNum, TID, FLAG);
                break;
        case  TYPE_AMSDU_AMPDU:
                FLAG |= WIFI_IS_BLOCKACK|WIFI_IS_AGGR;
                ERROR_DEBUG_OUT("TYPE_AMSDU_AMPDU, Not supported yet!!!\n");
                break;
        default:
                ERROR_DEBUG_OUT( "Unknown NET_MESSAGE type\n" );
                break;
        }

end:
        for(i = 0; i < packetNum; i++){
            if(buffer[i] != NULL){
                buffer[i] = NULL;
            }
        }
        FREE(buffer,"hsdpbuf");
        FREE(length,"hsdplen");
        buffer = NULL;
        length = NULL;
        DBG_HAL_THR_EXIT();

}

static int b2b_tx_thread_function(void *param)
{
    unsigned int loop = 0;
    struct B2B_Tx_Task_Struct * b2b_tx_struct_internal = (struct B2B_Tx_Task_Struct  *)param;
    int hal_full = 0;

    while (!b2b_tx_struct_internal->b2b_tx_quit)
    {
        if (down_interruptible(&b2b_tx_struct.b2b_quite_semph) != 0)
        {
            /* interrupted, exit */
            PRINT_ERR("hal_work_thread down_interruptible interrupted\n");
            break;
        }

        hal_full = 0;
        pr_debug("pt max tx: %d\n", gB2BTestCasePacket.send_frame_num);

        TrcConfMib.tid = STA2_VMAC1_SEND_TID;
        loop = 0;
        while(TrcConfMib.testtype != TYPE_STOP_TX)
        {
            if(gB2BTestCasePacket.send_frame_num > loop)
            {
                if (FiOpt2Driver->hal_get_priv_cnt == NULL) {
                    pr_err("===>>> %s ==>> hal_get_priv_cnt is NULL\n", __func__);
                    break;
                }
                if ((FiOpt2Driver->hal_get_priv_cnt(TrcConfMib.tid) < TrcConfMib.testmpdunum ))//+1
                {
                    if(hal_full == 0)
                    {
                        pr_warn("%s %d: hal buffer full.\n", __func__, __LINE__);
                        hal_full = 1;
                    }
                    msleep(10);
                    continue;
                }

                HostSendDataPacket(TrcConfMib.testtype,TrcConfMib.testmpdunum,TrcConfMib.testflag);
                //pr_debug("test flag = 0x%x bw =0x%x\n", TrcConfMib.testflag, (TrcConfMib.testflag >>WIFI_CHANNEL_BW_OFFSET) & 0x3 );
                loop++;
            }//tx max pkt
            else
            {
                pr_debug("**** pt send pkt %d done  ***", loop);
                break; // quite into to thread to sleep
            }
        }// start/stop loop
        pr_debug("**** stop : when pt send pkt %d done ***", loop);
    }// thread loop

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
    complete_and_exit(&b2b_tx_struct.b2b_thread_cmplt, 0);
#else
    kthread_complete_and_exit(&b2b_tx_struct.b2b_thread_cmplt, 0);
#endif
    pr_debug("**** exit b2b_tx_thread_function ***");
    return 0;
}


void Task_Schedule(int usrtesttype)
{
    //unsigned char vmac_id =0;

    DBG_HAL_THR_ENTER();

    TrcConfMib.testflag = 0;
    switch (usrtesttype)
    {
        // 1
        case TYPE_COMMON:
            TrcConfMib.tid = STA2_VMAC1_SEND_TID;
            TrcConfMib.testtype = TYPE_COMMON;
            TrcConfMib.testflag = 0;//WIFI_IS_RTSEN;
            TrcConfMib.testmpdunum = 1;
            if( STA1_VMAC0_MULTICAST == 1 )
            {
               TrcConfMib.testflag = WIFI_IS_Group | WIFI_IS_NOACK;
            }

            up(&b2b_tx_struct.b2b_quite_semph);
            break;
        // 2
        case TYPE_AMPDU:
            TrcConfMib.testflag &= ~WIFI_IS_NOACK;
            TrcConfMib.testflag |= WIFI_IS_BLOCKACK|WIFI_IS_AGGR;
            TrcConfMib.testtype = TYPE_AMPDU;
            TrcConfMib.testmpdunum = 1;//STA1_VMAC0_AGG_NUM;
            up(&b2b_tx_struct.b2b_quite_semph);
            break;
        // 3
        case TYPE_AMSDU:
            TrcConfMib.testflag = 5;//WIFI_IS_RTSEN;
            TrcConfMib.testtype = TYPE_AMSDU;
            TrcConfMib.testmpdunum = 2;
            if(STA1_VMAC0_MULTICAST==1){
               TrcConfMib.testflag=WIFI_IS_Group|WIFI_IS_NOACK;
            }
            up(&b2b_tx_struct.b2b_quite_semph);
            break;
        // 4
        case TYPE_AMSDU_AMPDU:
            ERROR_DEBUG_OUT("---Not supported yet---!!!\n");
            up(&b2b_tx_struct.b2b_quite_semph);
            return;
            //break;
        // 5,
        case TYPE_COMMON_NOACK: //5
            TrcConfMib.tid = STA2_VMAC1_SEND_TID;//STA2_VMAC1_SEND_TID;
            TrcConfMib.testflag |= WIFI_IS_NOACK;
            TrcConfMib.testflag &= ~WIFI_IS_BLOCKACK;
            TrcConfMib.testtype = TYPE_COMMON;
            TrcConfMib.testmpdunum = 1;
            up(&b2b_tx_struct.b2b_quite_semph);
            break;
        // 6
        case TYPE_BURST_ACK:
            TrcConfMib.testflag |= WIFI_IS_BURST;
            TrcConfMib.testtype = TYPE_COMMON;
            TrcConfMib.testmpdunum = 3;
            up(&b2b_tx_struct.b2b_quite_semph);
            break;
        // 7
        case TYPE_BURST_BA:
            TrcConfMib.testflag |= WIFI_IS_BLOCKACK|WIFI_IS_BURST
                                   |WIFI_IS_NOACK|WIFI_IS_AGGR|WIFI_IS_RTSEN;
            TrcConfMib.testtype = TYPE_AMPDU;
            TrcConfMib.testmpdunum = 3;
            up(&b2b_tx_struct.b2b_quite_semph);
            break;
        // 8
        case TYPE_STOP_TX:
            TrcConfMib.testtype=TYPE_STOP_TX;
            pr_debug("send stop!\n");
            break;
        default:
            break;
    }

     pr_debug("%s(%d) test flag =0x%x, test type = 0x%x\n",
                    __func__, __LINE__, TrcConfMib.testflag, usrtesttype);

    DBG_HAL_THR_EXIT();

}

void b2b_tx_thread_remove(void)
{
    if ( b2b_tx_struct.b2b_tx_thread)
        {
             init_completion(&b2b_tx_struct.b2b_thread_cmplt);
            b2b_tx_struct.b2b_tx_quit = 1;
            up(&b2b_tx_struct.b2b_quite_semph);
            kthread_stop(b2b_tx_struct.b2b_tx_thread);
            wait_for_completion(&b2b_tx_struct.b2b_thread_cmplt);
            b2b_tx_struct.b2b_tx_thread =NULL;
    /*
        pr_debug("======> Remove b2b tx thread\n");
        b2b_tx_struct.b2b_tx_quit = 1;
        kthread_stop(b2b_tx_struct.b2b_tx_thread);
        b2b_tx_struct.b2b_tx_thread = NULL;
     */

    }
}

void driver_close(void)
{
    FiOpt2Driver->hal_close((struct net_device *)NULL);
    FiOpt2Driver->hal_exit();
}

/*Check local mac address is the same as bssid
**Return: 1: Not the same;0:the same
*/

int b2b_compare_local_and_bssid(void)
{
    int ret = 0;
    int i = 0;

    for (i=0;i<6;i++)
    {
        if ( TrcConfMib.the_bssid[i] != TrcConfMib.the_mac_address[i] )
        {
            pr_err("\n\nb2b_compare_local_and_bssid: NOT the same\n");
            ret = 1;
            break;
        }
    }
    return ret;
}

void driver_open(void)
{
    unsigned char vmac_id =0;
    struct hal_private *hal_priv = hal_get_priv();
    DBG_HAL_THR_ENTER();


    pr_debug("********* b2b debug enter driver_open ***************\n");

    callback.mic_error_event = Driver_mic_error_event;
    callback.intr_tx_handle = Driver_intr_tx_handle;
    callback.intr_rx_handle = Driver_intr_rx_handle;
    callback.intr_bcn_send = Driver_intr_bcn_send;
    callback.intr_dtim_send = Driver_intr_dtim_send;
    callback.intr_ba_recv = Driver_intr_ba_recv;
    callback.dev_remove = drv_dev_remove;
    callback.drv_intr_bt_info_change = drv_intr_bt_info_change;
    FiOpt2Driver = &hal_priv->hal_ops;

    hal_priv->hal_call_back = &callback;


    FiOpt2Driver->hal_init((struct net_device *)NULL);
    TrcConfMib.dot11RDSupport = 0;

    FiOpt2Driver->phy_register_sta_id(vmac_id,1,TrcConfMib.the_connect_address, 0);

    FiOpt2Driver->phy_set_mac_addr(vmac_id,TrcConfMib.the_mac_address);
    FiOpt2Driver->phy_set_mac_bssid(vmac_id,TrcConfMib.the_bssid);
    FiOpt2Driver->phy_set_lretry_limit(0);
    FiOpt2Driver->phy_set_sretry_limit(0);

    if (STA1_VMAC1_SHORTGI == 1) {
        TrcConfMib.testflag |= WIFI_IS_SHORTGI;
    }else{
        TrcConfMib.testflag |= ~WIFI_IS_SHORTGI;
    }

    if (STA1_VMAC1_GREENFIELD == 1) {
        TrcConfMib.testflag |= WIFI_IS_GREENFIELD;
    }

    TrcConfMib.tid = STA2_VMAC1_SEND_TID;

    if ( 0 != b2b_compare_local_and_bssid() )
    {
        FiOpt2Driver->phy_addba_ok(vmac_id,1,TrcConfMib.tid,
            TrcConfMib.SN[TrcConfMib.tid],64,
            BA_INITIATOR,immidiate_BA_TYPE);
    }
    else
    {
        FiOpt2Driver->phy_addba_ok(vmac_id,1,TrcConfMib.tid,
            TrcConfMib.SN[TrcConfMib.tid],64,
            BA_RESPONDER,immidiate_BA_TYPE);
    }

    if ( 0 != b2b_compare_local_and_bssid() )//Tx side
    {
        pr_debug("\n---B2B debug---: Tx side\n");
        TrcConfMib.dot11EncryptType = gB2BTestCasePacket.encryp_type;
        TrcConfMib.dot11EncryptLen = STA1_VMAC0_KEY_LEN;
        if(STA1_VMAC0_IBSS)
        {
            FiOpt2Driver->phy_set_cam_mode(vmac_id, MODE_IBSS);
        }
        else
        {
            FiOpt2Driver->phy_set_cam_mode(vmac_id, MODE_STA);
        }

        if(TrcConfMib.dot11EncryptType != WIFI_NO_WEP)
         {
            FiOpt2Driver->phy_set_ucast_key(vmac_id,1,
                my_network->dst_mac_address,(unsigned char *)pKey,
                TrcConfMib.dot11EncryptLen,TrcConfMib.dot11EncryptType, 0);
            FiOpt2Driver->phy_set_mcast_key(vmac_id,(unsigned char *)pKey,
                TrcConfMib.dot11EncryptLen, 0,
                TrcConfMib.dot11EncryptType,DP_SEC_KEY_AUTH);
         }
    }
    else
    {
        pr_debug("\n---B2B debug---: Rx side\n");
        TrcConfMib.dot11EncryptType = gB2BTestCasePacket.encryp_type;
        TrcConfMib.dot11EncryptLen = STA1_VMAC0_KEY_LEN;
        if(STA1_VMAC0_IBSS)
        {
            FiOpt2Driver->phy_set_cam_mode(vmac_id, MODE_IBSS);
        }
        else
        {
            FiOpt2Driver->phy_set_cam_mode(vmac_id, MODE_AP);
        }

        if(TrcConfMib.dot11EncryptType != WIFI_NO_WEP)
        {
            FiOpt2Driver->phy_set_ucast_key(vmac_id, 1,
                my_network->src_mac_address,(unsigned char *)pKey,
                TrcConfMib.dot11EncryptLen, TrcConfMib.dot11EncryptType, 0);
            FiOpt2Driver->phy_set_mcast_key(vmac_id,(unsigned char *)pKey,
                TrcConfMib.dot11EncryptLen, 0,
                TrcConfMib.dot11EncryptType,DP_SEC_KEY_SUPP);
        }
    }





    if(BSS_BW_80M == 2)
    {
        pr_debug("\n---B2B debug---: 80M bw\n");
        //amlhal_SetChannel_BW(SW_CBW80,SW_COFF_U30M);
    }
    else if(BSS_BW_40M == 1)
    {
        //amlhal_SetChannel_BW(SW_CBW40,SW_HI_CH_OFF_20U);
        pr_debug("\n---B2B debug---: 40M bw\n");
        //PhySetChanSupportType(WIFINET_BW_40PLUS);
    }
    else
    {
        pr_debug("\n---B2B debug---: 20M bw\n");
        //amlhal_SetChannel_BW(SW_CBW20,SW_HI_CH_OFF_20);
        //PhySetChanSupportType(WIFINET_BW_20);
    }


    sema_init(&b2b_tx_struct.b2b_quite_semph,0);


    //Create b2b tx send thread
    b2b_tx_struct.b2b_tx_quit = 0;
    b2b_tx_struct.b2b_tx_thread = kthread_run(b2b_tx_thread_function,
                                    &b2b_tx_struct, "b2b_tx_thread");

    if (IS_ERR(b2b_tx_struct.b2b_tx_thread)) {
        b2b_tx_struct.b2b_tx_thread = NULL;
        pr_info("B2B: Create b2b tx task failed!\n");
        goto b2b_tx_thread_create_fail;
    }
    else {
        pr_info("B2B: Create b2b tx task success!\n");
        return;
    }

b2b_tx_thread_create_fail:
    ERROR_DEBUG_OUT("B2B: Create b2b tx task failed ++\n");
    if ( b2b_tx_struct.b2b_tx_thread )
    {
        b2b_tx_struct.b2b_tx_quit = 1;
        KILL_PROC((pid_t)(SYS_TYPE)b2b_tx_struct.b2b_tx_thread, SIGTERM);
        b2b_tx_struct.b2b_tx_thread = NULL;
    }
    return;

    DBG_HAL_THR_EXIT();
}

unsigned char Queue_IsEmpty( struct _Queue* my )
{
        return my->front == my->back;
}


void Pool_Create( struct _Pool* my, unsigned short size,
    unsigned short count, unsigned char buffer[],
    void* pool_buffer[],const char* name )
{
        //
        // Enforce (at least) word alignment of all the blocks
        //
        ASSERT( ( unsigned int )(unsigned long)buffer % 4 == 0
            && size % 4 == 0 && count != 0 );

        //
        // These fields are used for debugging only, and may be removed to save space.
        //
        my->buffer = buffer;
        my->size = size;
        my->max_count = count;
        my->min_count = count;
        strscpy(my->name, name, sizeof(my->name));
        //
        // Important: queue_buffer[] size must be at least count + 1
        //
        Queue_Create( &my->queue, pool_buffer, count + 1 );

        while ( count-- ) {
                Queue_Enqueue( &my->queue, buffer );
                buffer += size;
        }
}

struct sk_buff* TxBuffer_Alloc(void)
{
    struct sk_buff *skb = os_skb_alloc(gB2BTestCasePacket.pkt_length + RESERVED_LEN + HI_TXDESC_DATAOFFSET);

    while (!skb) {
        skb = os_skb_alloc(gB2BTestCasePacket.pkt_length + RESERVED_LEN + HI_TXDESC_DATAOFFSET);
    }

    ASSERT(skb);
    skb_reserve(skb, HI_TXDESC_DATAOFFSET);

    return skb;
}

static void TxDescriptor_Initialize(void)
{
        Pool_Create(&our_tx_descriptor_pool,
                    TX_DESCRIPTOR_SIZE,
                    TX_DESCRIPTOR_COUNT,
                    our_tx_descriptors,
                    our_tx_descriptor_pool_buffer,
                    "tx_descriptor");
}

void TxTask_Initialize(void)
{
    Driver_CreatTxPriv();
    TxDescriptor_Initialize();
    Queue_Create(&myTxTask.confirm_queue,
        myTxTask.confirm_queue_buffer,
        TX_TASK_CONFIRM_QUEUE_SIZE );
}

void mib_init(void)
{
    DBG_HAL_THR_ENTER();

#if 1
    //The local macaddr
    TrcConfMib.the_bssid[0] = 0x00;
    TrcConfMib.the_bssid[1] = 0x12;
    TrcConfMib.the_bssid[2] = 0x34;
    TrcConfMib.the_bssid[3] = 0x58;
    TrcConfMib.the_bssid[4] = 0x00;
    TrcConfMib.the_bssid[5] = 0x12;

    //The peer mac address
    TrcConfMib.the_mac_address[0] = 0x00;
    TrcConfMib.the_mac_address[1] = 0x12;
    TrcConfMib.the_mac_address[2] = 0x34;
    TrcConfMib.the_mac_address[3] = 0x58;
    TrcConfMib.the_mac_address[4] = 0x00;
    TrcConfMib.the_mac_address[5] = 0x12;

    TrcConfMib.the_desc_address[0] = 0x00;
    TrcConfMib.the_desc_address[1] = 0x12;
    TrcConfMib.the_desc_address[2] = 0x34;
    TrcConfMib.the_desc_address[3] = 0x58;
    TrcConfMib.the_desc_address[4] = 0x00;
    TrcConfMib.the_desc_address[5] = 0x12;

    TrcConfMib.the_connect_address[0] = 0x00;
    TrcConfMib.the_connect_address[1] = 0x12;
    TrcConfMib.the_connect_address[2] = 0x34;
    TrcConfMib.the_connect_address[3] = 0x58;
    TrcConfMib.the_connect_address[4] = 0x00;
    TrcConfMib.the_connect_address[5] = 0x12;

#if STA1_VMAC0_MULTICAST
    TrcConfMib.the_desc_address[0] = 0xff;
    TrcConfMib.the_desc_address[1] = 0xff;
    TrcConfMib.the_desc_address[2] = 0xff;
    TrcConfMib.the_desc_address[3] = 0xff;
    TrcConfMib.the_desc_address[4] = 0xff;
    TrcConfMib.the_desc_address[5] = 0xff;
#endif //STA1_VMAC0_MULTICASE

    TrcConfMib.tx_rate= WIFI_11B_11M;
    TrcConfMib.dot11FragmentationThreshold = 2000;
    TrcConfMib.tid = STA2_VMAC1_SEND_TID;
#endif

    TrcConfMib.tx_rate= WIFI_11B_11M;
    TrcConfMib.dot11FragmentationThreshold = 2000;
    TrcConfMib.tid = STA2_VMAC1_SEND_TID;

    DBG_HAL_THR_EXIT();

 }

