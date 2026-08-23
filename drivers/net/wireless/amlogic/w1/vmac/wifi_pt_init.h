/*****************************************************************************************
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
#ifndef _DRV_HAL_THR_INIT_H
#define _DRV_HAL_THR_INIT_H

#if defined (HAL_FPGA_VER)
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>    /* udelay */
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/moduleparam.h>
#include <linux/irqreturn.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h> /* pr_debug() */
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/gpio.h> //mach
#include <linux/timer.h>
#include <linux/string.h>
#endif



#include "wifi_pt_conf.h"
#include "wifi_pt_network.h"

#define IP_ADDRESS( a, b, c, d )  ( ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d )
#define _STA1_DRV_ 0
#define _STA2_DRV_ 1
#define STA1_VMAC0_MULTICAST 0
#define WIFI_11B_11M                0x03
#define WIFI_MAX_TID                    8
#define MAX_TID                         10
//#define HI_TXPRIVD_NUM_PER_TID          (128)
//#define HI_TXPRIVD_NUM_ALL_TID          (MAX_TID*HI_TXPRIVD_NUM_PER_TID)

#ifndef STA1_VMAC1_SHORTGI
#define STA1_VMAC1_SHORTGI gB2BTestCasePacket.if_shortGI
#endif

#ifndef STA1_VMAC1_GREENFIELD
#define STA1_VMAC1_GREENFIELD 0
#endif

#define RATE_LEN 21
#define RESERVED_LEN 256

#ifndef BSS_BW_40M
#define BSS_BW_40M gB2BTestCasePacket.channel_bw
#endif

#ifndef BSS_BW_80M
#define BSS_BW_80M gB2BTestCasePacket.channel_bw
#endif

struct _Queue {
        void** start;                           ///< Start of the ring buffer.
        void** end;                             ///< End of the ring buffer.
        void** front;                           ///< Front of the queue.
        void** back;                            ///< Back of the queue.
} ;

struct _Pool {
        struct _Queue queue;                         ///< Internal queue to manage the blocks.
        unsigned char* buffer;                        ///< Starting address of pool buffer.
        unsigned short size;                          ///< Size of allocation blocks.
        unsigned short max_count;                     ///< Number of allocation blocks.
        unsigned short min_count;                     ///< Minimun block count.
        unsigned char   name[16];
};

enum {
        RX_BUFFER_COUNT = 4,

};

enum {
        TX_TASK_CONFIRM_QUEUE_SIZE = 32,
        RX_QUEUE_SIZE = RX_BUFFER_COUNT + 1,    // Make sure RX queue is never full
};

struct _TxControl {
        unsigned int  control_0;     ///< = ( service << 16 | length )
        unsigned int  control_1;     ///< = ( ack_to << 23 | ackto_en << 22 | rate << 16 | power )
        unsigned int  length_0;
        unsigned int  address_0;
        unsigned int  length_1;
        unsigned int  address_1;
        unsigned int  length_2;
        unsigned int  address_2;
} ;

struct _TxDescriptor {
        struct sk_buff *skb;
        unsigned short len;
        unsigned short Tid;
} ;

enum {
        TX_BUFFER_COUNT     = 48*MAX_TID,
        TX_BUFFER_SIZE      = 21000,
        TX_DESCRIPTOR_COUNT = TX_BUFFER_COUNT + 3,
        TX_DESCRIPTOR_SIZE  = sizeof( struct _TxDescriptor ),
        TX_CONTROL_SIZE     = sizeof( struct _TxControl ),
};


enum {
        PHY_RATE_RESERVED = 0x0,
        PHY_RATE_2_SHORT  = 0x1,
        PHY_RATE_5_SHORT  = 0x2,
        PHY_RATE_11_SHORT = 0x3,

        PHY_RATE_1_LONG   = 0x4,
        PHY_RATE_2_LONG   = 0x5,
        PHY_RATE_5_LONG   = 0x6,
        PHY_RATE_11_LONG  = 0x7,

        PHY_RATE_48       = 0x8,
        PHY_RATE_24       = 0x9,
        PHY_RATE_12       = 0xA,
        PHY_RATE_6        = 0xB,
        PHY_RATE_54       = 0xC,
        PHY_RATE_36       = 0xD,
        PHY_RATE_18       = 0xE,
        PHY_RATE_9        = 0xF,

        PHY_RATE_MAX,
};

struct _TRC_CONF_MIB {
        unsigned char           dot11LongRetryLimit;
        unsigned char           dot11ShortRetryLimit;
        unsigned char           dot11CamMode;
        unsigned int            dot11RtsThreshold;
        unsigned short          dot11CurrentTxPowerLevel;
        unsigned short          dot11FragmentationThreshold;
        unsigned short          ot11BeaconInterval;
        unsigned short          dot11SlotTime;
        unsigned short          TxPowerLevelOfdm;
        unsigned short          TxPowerLevelDsssCck;
        unsigned short          Eifs;
        unsigned short          BasicRateTbl;
        unsigned short          SupportRateMap;
        unsigned char           dot11PreambleType;
        unsigned char           dot11RDSupport;
        unsigned char           dot11EncryptType;
        unsigned short          dot11EncryptLen;
        unsigned char           WmmEnabled;
        unsigned char           dot11PsMode;
        unsigned char           dot11MacMode;
        unsigned char           the_bssid[6];
        unsigned char           the_mac_address[6]; //the vmac address
        unsigned char           the_desc_address[6];    //the destination address of tx frame
        unsigned char           the_connect_address[6];    //the connected peer address
        unsigned char           bGotoDeepSleep;
        unsigned char           tid;
        unsigned short          SN[9];
        unsigned short          tx_rate;
        unsigned short          testtype;
        unsigned short          testmpdunum;
        unsigned short          testflag;
} ;

struct _B2B_Test_Case_Packet{
    unsigned char packet_type;
    unsigned char if_burst;
    unsigned char if_ampdu;
    unsigned char encryp_type;
    unsigned char tkip_mic;
    unsigned char tcpip_csum;
    unsigned int  send_frame_num;
    unsigned char data_rate;
    unsigned char channel_bw;
    unsigned char if_shortGI;
    unsigned char ldpc_enable;
    unsigned int  channel;
    unsigned int pkt_length;
};

extern struct _TRC_CONF_MIB TrcConfMib;
extern struct _B2B_Test_Case_Packet gB2BTestCasePacket;

struct _TxTask {
        struct _Queue confirm_queue;
        void* confirm_queue_buffer[ TX_TASK_CONFIRM_QUEUE_SIZE ];
        unsigned int confirm_queue_num;
} ;



struct _B2B_Platform_Conf{
    unsigned char enable_debug;
};

struct B2B_Tx_Task_Struct{
    struct task_struct* b2b_tx_thread;
    int                 b2b_tx_quit;
    struct semaphore   b2b_quite_semph;
       struct completion b2b_thread_cmplt;
};

void mib_init(void);
void network_init(void);
void dst_mac_addr_update(void);
void local_mac_addr_update(void);
void TxTask_Initialize(void);
void b2b_tx_thread_remove(void);
void driver_open(void);
int b2b_compare_local_and_bssid(void);
void driver_close(void);
void Task_Schedule(int usrtesttype);
void Net_RequestTransmit(unsigned char* frame,unsigned short len,unsigned short Tid);
struct _TxDescriptor* TxDescriptor_Create(void);
unsigned char Queue_Enqueue(struct _Queue* my, void* object);
void* Queue_Dequeue(struct _Queue* my);
unsigned int Queue_GetCount(struct _Queue* my);
unsigned char Queue_IsEmpty(struct _Queue* my);
void Queue_Create(struct _Queue* my, void* buffer[], unsigned int size);
unsigned char* Pool_GetBlock(struct _Pool* my);
unsigned char Pool_PutBlock( struct _Pool* my, void* block );
void Pool_Create( struct _Pool* my, unsigned short size,
    unsigned short count, unsigned char buffer[],
    void* pool_buffer[],const char* name );
struct sk_buff* TxBuffer_Alloc(void);
#endif
