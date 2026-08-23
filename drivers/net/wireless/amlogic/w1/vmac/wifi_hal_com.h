#ifndef _TOTAL_H_
#define _TOTAL_H_
#define __INLINE static inline

#ifndef _STA2_DRV_
#define _STA2_DRV_   1
#endif

#ifndef _STA1_DRV_
#define _STA1_DRV_   0
#endif


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
#ifdef CONFIG_HAS_WAKELOCK
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,14,29)
#include <linux/wakelock.h>
#endif
extern unsigned long long g_dbg_info_enable;
#endif

#elif defined (HAL_SIM_VER)
#include "os.h"
#include "utils.h"
#if defined (SYSTEM32)
typedef unsigned int  SYS_TYPE;
#elif defined (SYSTEM64)
typedef unsigned long SYS_TYPE;
#else
#err "SYSTEM TYPE ERR"
#endif

#define MALLOC(a) malloc(a)
#define FREE     free
#define FRINT io_printf
#define OS_LIST CO_LIST_HDR

struct _HI_TxPrivDescripter_chain;
struct _hi_agg_tx_desc_chain;
#endif

#include "wifi_share_fifo.h"
#include "fi_sdio.h"
#include "opt_all.h"
#include "fi_cmd.h"
#include "patch_fi_cmd.h"

struct hw_interface;

#include "wifi_sys_addr.h"
#if defined (HAL_FPGA_VER)
#include "wifi_drv_config.h"
#include "wifi_drv_statistic.h"
#include "wifi_phy.h"
#include "osdep.h"
#include "wifi_rate_ctrl.h"
#include "wifi_drv_reg_ops.h"
#include "wifi_pt_init.h"

#elif defined (HAL_SIM_VER)
#include "wifi_phy.h"
#include "host.h"
#include "network.h"
#include "ieee80211.h"
#endif

#include "wifi_hal_platform.h"
#include "wifi_hif.h"


//////////////////value ////////////////////////
#define CPU_RUN	BIT(24)
#define CPU_HALT BIT(25)
#define SDIO_FW2HOST_GPIO_EN (BIT(20))
#define SDIO_FW2HOST_EN (BIT(31))
#define UART_WORK_CLK (80000000)
#define HI_FI_SYNC_DELAY_MS (1000)
#define HI_FI_SYNC_DELAY_MS_STEP (200)
#define SRAM_MAX_LEN (1024*4)
#define MAX_OFFSET (1024*100)
#define SLOT_MASK (0x3f<<16)
// 1:level sensitive interrupt; 0:edge sensitive interrupt
#define SDIO_GPIO_IRQ_TRIG_MODE_LEVEL  (BIT(30))
//bit0~bit11, if edge sensitive interrupt mode enabled, interrupt
//will generate pulse, this is the pulse width, unit is ahb clk
#define SDIO_GPIO_IRQ_EDGE_TIMEUP_MASK  (0xfff)
//sdio manufacturer code, usually vendor ID, 'a'=0x61, 'm'=0x6d
#define VENDOR_AMLOGIC_EFUSE ('a'|('m'<<8))
//sdio manufacturer info, usually product ID
#define PRODUCT_AMLOGIC_EFUSE (0x9007)

#define SWITCH_CLK_WAIT_US (10)
#define UART_BAUD_RATE (115200)
/***aml hw cmd***/
#define FCS_LEN     4

enum {
        hirq_tx_err_idx,
        hirq_rx_err_idx,
        hirq_rx_ok_idx,
        hirq_tx_ok_idx,
        hirq_bcn_send_ok_vid0_idx	,
        hirq_bcn_send_ok_vid1_idx	,
        hirq_goto_wkp_vid0_idx		,
        hirq_goto_wkp_vid1_idx		,
        hirq_p2p_oppps_cwend_vid0_idx,
        hirq_p2p_oppps_cwend_vid1_idx,
        hirq_p2p_noa_start_vid0_idx	,
        hirq_p2p_noa_start_vid1_idx	,
        hirq_bcn_recv_ok_vid0_idx	,
        hirq_bcn_recv_ok_vid1_idx	,
        hirq_bcn_miss_vid0_idx		,
        hirq_bcn_miss_vid1_idx		,
        hirq_max_idx		,
    };

enum wifi_module_sn {
      MODULE_ITON = 0X1,
      MODULE_AMPAK,
      MODULE_FN_LINK,
};

#define MAC_FCTRL_PROTOCOLVERSION_MASK 0x0003
#define MAC_FCTRL_TYPE_MASK            0x000C
#define MAC_FCTRL_MGT_T            0x0000
#define MAC_FCTRL_CTRL_T           0x0004
#define MAC_FCTRL_DATA_T           0x0008

#define MAC_FCTRL_SUBT_MASK            0x00F0
/* Management SubType                                                               */
#define MAC_FCTRL_ASSOCREQ_ST      0x0000
#define MAC_FCTRL_ASSOCRSP_ST      0x0010
#define MAC_FCTRL_REASSOCREQ_ST    0x0020
#define MAC_FCTRL_REASSOCRSP_ST    0x0030
#define MAC_FCTRL_PROBEREQ_ST      0x0040
#define MAC_FCTRL_PROBERSP_ST      0x0050
#define MAC_FCTRL_BEACON_ST        0x0080
#define MAC_FCTRL_ATIM_ST          0x0090
#define MAC_FCTRL_DISASSOC_ST      0x00A0
#define MAC_FCTRL_AUTHENT_ST       0x00B0
#define MAC_FCTRL_DEAUTHENT_ST     0x00C0
#define MAC_FCTRL_ACTION_ST        0x00D0
/* Control SubTypes                                                                 */
#define MAC_FCTRL_PSPOLL_ST        0x00A0
#define MAC_FCTRL_RTS_ST           0x00B0
#define MAC_FCTRL_CTS_ST           0x00C0
#define MAC_FCTRL_ACK_ST           0x00D0
#define MAC_FCTRL_CFEND_ST         0x00E0

#define MAC_DATA_ST             0x0000
#define MAC_CFACK_ST_BIT           BIT(4)
#define MAC_CFPOLL_ST_BIT          BIT(5)
#define MAC_NODATA_ST_BIT          BIT(6)
#define MAC_QOS_ST_BIT             BIT(7)

#define MAC_FCTRL_DATACFACKPOLL_ST (MAC_CFACK_ST_BIT | MAC_CFPOLL_ST_BIT)

#define MAC_FCTRL_TODS                 0x0100
#define MAC_FCTRL_FROMDS               0x0200
#define MAC_FCTRL_PWRMGT               0x1000
#define MAC_FCTRL_PROTECTEDFRAME       0x4000


/* FRAME CONTROL : Type information including Type and SubType                          */
#define MAC_FCTRL_ASSOCREQ              (MAC_FCTRL_MGT_T     | MAC_FCTRL_ASSOCREQ_ST)
#define MAC_FCTRL_ASSOCRSP              (MAC_FCTRL_MGT_T     | MAC_FCTRL_ASSOCRSP_ST)
#define MAC_FCTRL_REASSOCREQ            (MAC_FCTRL_MGT_T     | MAC_FCTRL_REASSOCREQ_ST)
#define MAC_FCTRL_REASSOCRSP            (MAC_FCTRL_MGT_T     | MAC_FCTRL_REASSOCRSP_ST)
#define MAC_FCTRL_PROBEREQ              (MAC_FCTRL_MGT_T     | MAC_FCTRL_PROBEREQ_ST)
#define MAC_FCTRL_PROBERSP              (MAC_FCTRL_MGT_T     | MAC_FCTRL_PROBERSP_ST)
#define MAC_FCTRL_BEACON                (MAC_FCTRL_MGT_T     | MAC_FCTRL_BEACON_ST)
#define MAC_FCTRL_ATIM                  (MAC_FCTRL_MGT_T     | MAC_FCTRL_ATIM_ST)
#define MAC_FCTRL_DISASSOC              (MAC_FCTRL_MGT_T     | MAC_FCTRL_DISASSOC_ST)
#define MAC_FCTRL_AUTHENT               (MAC_FCTRL_MGT_T     | MAC_FCTRL_AUTHENT_ST)
#define MAC_FCTRL_DEAUTHENT             (MAC_FCTRL_MGT_T     | MAC_FCTRL_DEAUTHENT_ST)
#define MAC_FCTRL_ACTION                (MAC_FCTRL_MGT_T     | MAC_FCTRL_ACTION_ST)

#define MAC_FCTRL_PSPOLL                (MAC_FCTRL_CTRL_T    | MAC_FCTRL_PSPOLL_ST)
#define MAC_FCTRL_RTS                   (MAC_FCTRL_CTRL_T    | MAC_FCTRL_RTS_ST)
#define MAC_FCTRL_CTS                   (MAC_FCTRL_CTRL_T    | MAC_FCTRL_CTS_ST)
#define MAC_FCTRL_ACK                   (MAC_FCTRL_CTRL_T    | MAC_FCTRL_ACK_ST)
#define MAC_FCTRL_CFEND                 (MAC_FCTRL_CTRL_T    | MAC_FCTRL_CFEND_ST)
#define MAC_FCTRL_CFEND_CFACK           (MAC_FCTRL_CFEND     | MAC_CFACK_ST_BIT)

#define MAC_FCTRL_DATA              (MAC_FCTRL_DATA_T    | MAC_DATA_ST)
#define MAC_FCTRL_DATA_CFACK            (MAC_FCTRL_DATA_T    | MAC_CFACK_ST_BIT)
#define MAC_FCTRL_NULL_FUNCTION         (MAC_FCTRL_DATA_T    | MAC_NODATA_ST_BIT)
#define MAC_FCTRL_NULL_CFACK       (MAC_FCTRL_NULL_FUNCTION  | MAC_CFACK_ST_BIT)
#define MAC_FCTRL_NULL_CFPOLL      (MAC_FCTRL_NULL_FUNCTION  | MAC_CFPOLL_ST_BIT)
#define MAC_FCTRL_NULL_CFACKPOLL   (MAC_FCTRL_NULL_FUNCTION  | MAC_FCTRL_DATACFACKPOLL_ST)

#define MAC_FCTRL_QOS_DATA              (MAC_FCTRL_DATA_T    | MAC_QOS_ST_BIT)
#define MAC_FCTRL_QOS_DATA_CFACK        (MAC_FCTRL_QOS_DATA  | MAC_CFACK_ST_BIT)
#define MAC_FCTRL_QOS_NULL              (MAC_FCTRL_QOS_DATA  | MAC_NODATA_ST_BIT)
#define MAC_FCTRL_QOS_NULL_CFACK        (MAC_FCTRL_QOS_DATA  | MAC_FCTRL_NULL_CFACK)


///
/// The MacFrame class encapsulates all operations on MAC header fields.
///
enum
{
    FRAME_TYPE_MASK                         = 0x0000000F,        // Include the version field for error checking
    FRAME_SUBTYPE_MASK                      = 0x000000F0,
    FRAME_CONTROL_MASK                      = 0x0000FF00,
    DURATION_ID_MASK                        = 0xFFFF0000,
    DURATION_ID_SHIFT                       = 16,
    //
    // Management frame types
    //
    FRAME_TYPE_MANAGEMENT                           = 0x00000000,
    FRAME_SUBTYPE_ASSOCIATION_REQUEST       = 0x00000000,
    FRAME_SUBTYPE_ASSOCIATION_RESPONSE      = 0x00000010,
    FRAME_SUBTYPE_REASSOCIATION_REQUEST     = 0x00000020,
    FRAME_SUBTYPE_REASSOCIATION_RESPONSE    = 0x00000030,
    FRAME_SUBTYPE_PROBE_REQUEST                 = 0x00000040,
    FRAME_SUBTYPE_PROBE_RESPONSE                = 0x00000050,
    FRAME_SUBTYPE_BEACON                            = 0x00000080,
    FRAME_SUBTYPE_ATIM                              = 0x00000090,
    FRAME_SUBTYPE_DISASSOCIATION                = 0x000000A0,
    FRAME_SUBTYPE_AUTHENTICATION                = 0x000000B0,
    FRAME_SUBTYPE_DEAUTHENTICATION          = 0x000000C0,
    FRAME_SUBTYPE_ACTION                            = 0x000000D0,
    FRAME_SUBTYPE_ACTION_NO_ACK                 = 0x000000E0,
    //
    // Control frame types
    //
    FRAME_TYPE_CONTROL                              = 0x00000004,
    FRAME_SUBTYPE_CONTROL_WRAPPER               = 0x00000070,
    FRAME_SUBTYPE_BLOCK_ACK_REQUEST         = 0x00000080,
    FRAME_SUBTYPE_BLOCK_ACK                         = 0x00000090,
    FRAME_SUBTYPE_PS_POLL                           = 0x000000A0,
    FRAME_SUBTYPE_RTS                               = 0x000000B0,
    FRAME_SUBTYPE_CTS                               = 0x000000C0,
    FRAME_SUBTYPE_ACK                               = 0x000000D0,
    FRAME_SUBTYPE_CF_END                            = 0x000000E0,
    FRAME_SUBTYPE_CFEND_CFACK                   = 0x000000F0,
    //
    // Data frame types (various combinations are possible)
    //
    FRAME_TYPE_DATA                                     = 0x00000008,
    FRAME_SUBTYPE_DATA                              = 0x00000000,
    FRAME_SUBTYPE_CF_ACK                            = 0x00000010,
    FRAME_SUBTYPE_CF_POLL                           = 0x00000020,
    FRAME_SUBTYPE_NULL                              = 0x00000040,
    FRAME_SUBTYPE_QOS                               = 0x00000080,
    FRAME_SUBTYPE_CFACK_POLL                        = 0x00000030,
    //
    // Frame control flags (various combinations are possible)
    //
    FRAME_CONTROL_TO_DS                             = 0x00000100,
    FRAME_CONTROL_FROM_DS                           = 0x00000200,
    FRAME_CONTROL_MORE_FRAGMENT                 = 0x00000400,
    FRAME_CONTROL_RETRY                             = 0x00000800,
    FRAME_CONTROL_POWER_MANAGEMENT          = 0x00001000,
    FRAME_CONTROL_MORE_DATA                     = 0x00002000,
    FRAME_CONTROL_PROTECTED                         = 0x00004000,
    FRAME_CONTROL_ORDERED                           = 0x00008000,
    //
    // Sequence Control
    //
    SEQUENCE_CONTROL_FRAGMENT_MASK          = 0x000F,
    SEQUENCE_CONTROL_FRAGMENT_SHIFT         = 0,
    SEQUENCE_CONTROL_SEQUENCE_MASK          = 0xFFF0,
    SEQUENCE_CONTROL_SEQUENCE_SHIFT         = 4,
    //
    // QoS Control
    //
    QOS_CONTROL_TID_MASK                        = 0x000F,
    QOS_CONTROL_TID_SHIFT                       = 0,
    QOS_CONTROL_EOSP_MASK                       = 0x0010,
    QOS_CONTROL_EOSP_SHIFT                      = 4,
    QOS_CONTROL_ACK_POLICY_MASK             = 0x0060,
    QOS_CONTROL_ACK_POLICY_SHIFT            = 5,
    QOS_CONTROL_ACK_POLICY_NORMAL           = 0x0000,
    QOS_CONTROL_ACK_POLICY_NO_ACK           = 0x0040,
    QOS_CONTROL_AMSDU_MASK                  = 0x0080,
    QOS_CONTROL_AMSDU_SHIFT                 = 7,
    QOS_CONTROL_TXOP_QUEUE_MASK             = 0xFF00,
    QOS_CONTROL_TXOP_QUEUE_SHIFT            = 8,
};

/*
 * Channels are specified by frequency.
 */
 struct hal_channel
{
    /*center frequency, setting in Mhz */
    unsigned short   channel;
    /* primary channel number */
    unsigned char  pchan_num;
    /*shijie.chen add,  center channel number of center frequency,
    set cchan_num to rf */
    unsigned char  cchan_num;
    int  chan_bw;
} ;

struct reg_table
{
    unsigned int regaddr;
    unsigned int regdata;
};

struct  PHY_PRIMARY_CHANNEL_BIT
{
    unsigned int    primary_channel         :8,
             central_frequency              :8,
             start_frequency                :8,
             def_pri_ch                     :3,
             pri_ch_cfg_sel                 :1,
             ap_bw                          :2,
             rf_fs                          :2;
};

///< Common signature of task handlers.
typedef void (*WorkHandler)(SYS_TYPE param1,SYS_TYPE param2,
    SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5); 
typedef void (*TaskHandler)(SYS_TYPE param1);

struct  hal_work_task
{
    WorkHandler task;
    WorkHandler taskcallback;
    SYS_TYPE param1;
    SYS_TYPE param2;
    SYS_TYPE param3;
    SYS_TYPE param4;
    SYS_TYPE param5;
} ;


/****************************** aml fwdownload**************************************************/
#ifdef PROJECT_T9026

// for check
#define ICCM_BUFFER_RD_LEN (96*1024)
#define ICCM_CHECK_LEN (96*1024)
#define DCCM_CHECK_LEN (48*1024)

#define ICCM_ALL_LEN (96*1024)
#define ICCM_ROM_LEN (0)
#define ICCM_RAM_LEN (96*1024)
#define DCCM_LEN (48*1024)
#define SRAM_LEN (8*1024)

#define ICCM_CHECK

#elif defined(PROJECT_W1)
#define ICCM_ROM_LEN    (96 * 1024)
#define ICCM_RAM_LEN    (64 * 1024)
#define DCCM_LEN        (48 * 1024)
#define SRAM_LEN        (8 * 1024)
#define ICCM_ALL_LEN    (ICCM_ROM_LEN + ICCM_RAM_LEN)

// for check
#define ICCM_BUFFER_RD_LEN  (ICCM_RAM_LEN)
#define ICCM_CHECK_LEN      (ICCM_RAM_LEN)
#define DCCM_CHECK_LEN      (DCCM_LEN)

#ifdef HAL_FPGA_VER
#define ICCM_CHECK
#define ICCM_ROM
#endif

#endif

/********************************** aml hal**********************************************/

#ifndef BIT
#define BIT(n)    (1UL << (n))
#endif //BIT

#define WIFI_ADDR2(wh)          (((unsigned char *)wh)+4+6)
#define MAC_FCTRL_PROTECTEDFRAME       0x4000
#define MAC_FCTRL_TODS                 0x0100
#define MAC_FCTRL_FROMDS               0x0200
#define MAC_HEAD_QOS_UP_MASK            0x07
#define MAC_SHORT_MAC_HDR_LEN          24              /* size without FCS              */
#define MAC_FCTRL_TYPE_MASK            0x000C
#define MAC_QOS_ST_BIT             BIT(7)
#define MAC_FCTRL_DATA_T           0x0008

#define IEEE80211_FC1_WEP                       0x40
#define MAC_FCTRL_PWRMGT               0x1000

#define TXID_INVALID 0xff

/******************************* aml hal test*************************************************/
#define HAL_TEST_ENABLE    BIT(0)
#define HAL_TEST_HELPINFO    BIT(1)

#define TEST_WIFI_SRAM_32              BIT(4)
#define TEST_WIFI_ICCM_32              BIT(5)
#define TEST_WIFI_DCCM_32              BIT(6)
#define TEST_WIFI_ICCM_ROM_32              BIT(7)
#define TEST_WIFI_SRAM              BIT(8)
#define TEST_WIFI_ICCM              BIT(9)
#define TEST_WIFI_DCCM              BIT(10)
#define TEST_WIFI_ICCM_ROM              BIT(11)
#define DUMP_WIFI_REGISTER          BIT(12)
#define MEASURE_SDIO_CMD_RW            BIT(13)
#define MEASURE_SDIO_THROUGHPUT    BIT(14)
#define MEASURE_WIFI_THROUGHPUT    BIT(15)
#define TEST_WIFI_EFUSE             BIT(16)
//#define TEST_SDIO_MODULE          BIT(17)

#define HAL_TEST_DEFAULT    (0)


/****************************** aml hi**************************************************/

/* tx+rx page <= 448 pagenum, the other pages 512-(tx+rx)
    (at least 32KB) are for capture or dpd training*/
#define DEFAULT_TXPAGENUM 224

#ifdef SRAM_FULL_TEST
#define DEFAULT_RXPAGENUM 20
#else
#define DEFAULT_RXPAGENUM 224
#endif
#define PT_MODE 1
#define DEFAULT_BCN_NUM             4
#define DEFAULT_SRAM_LEN        (100*1024)
#define PAGE_DELT                                 0   /**/
#define TOTAL_SRAM_NUM                     8     /*system have SRAM number ,every SRAM is 32K*/
#define TXRX_SRAM_NUM                      7    /*TX RX used SRAM number, and TX RX use low address sram*/
#define TBC_ADDR_BEGIN_OFFSET         (0x08000* TXRX_SRAM_NUM)
#define TBC_ADDR_END_OFFSET            (0x40000)
/*ramshare_and_ahb_ctrl register config value*/
#define TBC_RAM_SHARE_MASK            ( 0x0000007f | ( (0xFF& (~((1<<TXRX_SRAM_NUM)-1) ) ) <<24 )  )


#if defined (HAL_FPGA_VER)
#define RX_FIFO_SIZE  ( 4*DEFAULT_RXPAGENUM*(PAGE_LEN))
#define MAX_SEM_CNT 2
#endif

#define DATA_PACKET_LENGTH_MIN 1000
#define THROUGHPUT_GET_INTERVAL 2000 //ms

//for tx mpdus, state machine: GET -> MAKE -> SET
//each mpdu occupy one unit
#define CO_TX_BUFFER_GET           0
#define CO_TX_BUFFER_MAKE          1
#define CO_TX_BUFFER_SET           2
#define CO_SF_BLOCK_TX_NBR         3

/*
 * Transmit queue subtype.  These map directly to
 * WME Access Categories (except for UPSD).  Refer
 * to Table 5 of the WME spec.
 */

 enum
{
    HAL_WME_MIN = 0,
    HAL_WME_AC_BE = 0,            /* best effort access category*/
    HAL_WME_AC_BK = 1,            /* background access category */
    HAL_WME_AC_VI = 2,            /* video access category */
    HAL_WME_AC_VO = 3,            /* voice access category */
    HAL_WME_NOQOS = 4,
    HAL_WME_MGT = 5,
    HAL_WME_UAPSD = 6,
    HAL_WME_MCAST = 7,
    HAL_WME_MAX = 8,
} ;

#define HAL_NUM_TX_QUEUES HAL_WME_MAX        /* max possible # of queues */


 struct hi_status
{
    unsigned int TX_REQ_EVENT_num[HAL_NUM_TX_QUEUES];
    unsigned int TX_DOWNLOAD_OK_EVENT_num[HAL_NUM_TX_QUEUES];
    unsigned int TX_SEND_OK_EVENT_num[HAL_NUM_TX_QUEUES];
    unsigned int RX_No_buffer_err_num;
    unsigned int RX_No_buffer_err2_num;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
    unsigned int Tx_Send_num;
    unsigned int Tx_Done_num;
    unsigned int Tx_Free_num;    //num of tx frames have been freed after tx completed
#else
    atomic_t Tx_Send_num;
    atomic_t Tx_Done_num;
    atomic_t Tx_Free_num;
#endif
    unsigned int tx_ok_num;
    unsigned int tx_fail_num;
    unsigned int Rx_Rcv_num;
    unsigned int PushDownEvent_Err_num;
    unsigned int PushDownCmd_Err_num;
    unsigned int UnKownEvent_Err_num;
    unsigned int HiTask_enter_num;
    unsigned int HiTask_exit_num;
    unsigned int rxlen_Err_num;
} ;


//save some info for ampdu tx
//not necessary, already deleted in 11ac, to be deleted in x111

 struct fw_txdesc_fifo
{
    unsigned long callback;
    struct hi_tx_desc *pTxDPape;
    struct sk_buff *ampduskb;
    unsigned char txqueueid;
} ;

 struct fw_rx_desc
{
    struct sk_buff * skb;
    unsigned int len;
} ;

#define MAX_WORK_TASK 32

 struct  aml_work_task_t
{
    TaskHandler task;
    SYS_TYPE    param1;
} ;

enum dhd_bus_wake_state
{
    WAKE_LOCK_OFF,
    WAKE_LOCK_PRIV,
    WAKE_LOCK_IOCTL,
    WAKE_LOCK_SDIO,
    WAKE_LOCK_WORK,
    WAKE_LOCK_RX,
    WAKE_LOCK_IRQ,
    WAKE_LOCK_TIMEOUT,
    WAKE_LOCK_HI_IRQ_THREAD,
    WAKE_LOCK_TXOK,
    WAKE_LOCK_MAX
};

/* note: not really a mode; there are really multiple PHY's */
enum wifi_mac_chanbw
{
    WIFINET_BW_20               = 0,    /* 2Ghz, HT20 */
    WIFINET_BW_40PLUS           = 1,    /* 2Ghz, HT40 (ext ch +1) */
    WIFINET_BW_40MINUS          = 2,    /* 2Ghz, HT40 (ext ch -1) */

    WIFINET_BW_80_U10             =3 ,    /*primary channel = center channel +10*/
    WIFINET_BW_80_U30             =4 ,     /*primary channel = center channel +30*/
    WIFINET_BW_80_L10             =5 ,     /*primary channel = center channel -10*/
    WIFINET_BW_80_L30             =6 ,     /*primary channel = center channel -30*/

    WIFINET_BW_MAX,
};


#define BW_20			0
#define BW_40			1
#define BW_80			2

#define HI_AGG_TXD_NUM_PER_QUEUE        (80)
#define HI_AGG_TXD_NUM_ALL_QUEUE            (HAL_NUM_TX_QUEUES*HI_AGG_TXD_NUM_PER_QUEUE)

#define WIFI_MCS_RATE(a)        ((a)&WIFI_11N_MASK)

#define BUS_DMA_TODEVICE        1

#if defined(HAL_FPGA_VER)
//struct use to set rx packet date address to host driver
 struct rx_share_fifo_ctrl
{
    unsigned char   *FDB;    // 8 byte  fifo start address
    unsigned long     FDN;    // 8 Byte  fifo size
    unsigned long     FDH;    // 4 Byte fifo  head
    unsigned long     FDT;    // 4 Byte fifo  tail
} ;

#endif


struct hw_sts_snr
{
    unsigned long snr_avr;
    unsigned long snr_num;
    unsigned int snr_run;
};

struct  hw_interface
{
    struct amlw_hif_ops   hif_ops;// exported

#if defined (HAL_FPGA_VER)
    struct rx_share_fifo_ctrl  rx_fifo;
    struct tx_status_list  tx_status_list;
    struct sk_buff_head bcn_list_head;
#endif

    struct hi_status HiStatus;
    struct _FIFO_SHARE_CTRL CmdDownFifo;

    unsigned int  Wifi_DeviceID;
    unsigned int  Wifi_VendorID;

    HW_CONFIG hw_config;
    struct hw_sts_snr sts_snr;
} ;
#define     howmanypage(x,y) (((x - 12) + ((y - 12) -1) )/ (y - 12))

 struct Tx_FrameDesc
{
    struct sk_buff *skb;
    unsigned long callback;
    unsigned char valid;
    unsigned char txqueueid;
} ;

#define MAX_PN_LEN      16
#define MAX_PN_WINDOW   62
 struct unicastReplayCnt
{
#define MAX_RX_QUEUE    4   //rx PN of each tid need to be increase, so rx need 4 ACs
#define MAX_TX_QUEUE    1   //tx PN of all tids are merged into one ac, we can keep the increase
#define TX_UNICAST_REPCNT_ID 0


    unsigned char txPN[MAX_TX_QUEUE/*macx ac queue num*/][MAX_PN_LEN]; //see HW_TxOption PN
    unsigned char rxPN[MAX_RX_QUEUE/*macx ac queue num*/][MAX_PN_LEN]; //see HW_RxDescripter_bit PN

} ;
 struct multicastReplayCnt
{

    unsigned char txPN[MAX_PN_LEN];    //see HW_TxOption PN
    unsigned char rxPN[MAX_PN_LEN];    //see HW_RxDescripter_bit PN

} ;

#define AC_MAX_INDEX 4 // BE:0,BK:1,VI:2,VO:3
 struct AcParam
{
    unsigned char ac;
    unsigned char aifsn;
    unsigned char cwminmax;
    unsigned short txop;
} ;

// since amlogic's self-definition reset ,we need store ap's bssid,sta's mac ,and others.
 struct sta_connect_msg
{
    RegStaIdParam CmdParam;
    Add_BA_Struct AddBA;
    unsigned char PreambleType;
    unsigned short channel;
    enum wifi_mac_chanbw  chan_bw;
    struct UniCastKeyCmd unitikey;
    struct MultiCastKeyCmd multikey;
    unsigned int lr_limit;
    unsigned int sr_limit;
    unsigned char scan;
    unsigned int slot;
    struct AcParam wme_acparam[AC_MAX_INDEX];
} ;

#ifndef MAC_LEN
#define MAC_LEN 6
#endif


 struct hi_tx_priv_hdr
{
    unsigned char          TX_STATUS;
    unsigned char          ShortRetryNum;
    unsigned char          LongRetryNum;
    unsigned char          ACKRSSI;
    unsigned int           TimeStmp;
    unsigned long         DDRADDR;    //struct WIFINET_S_FRAME *, point to frame body
    SYS_TYPE               DMAADDR;    //mapped to txdesc_ddraddr, no use now /*shijie.chen change to pointer skb */
    unsigned short         DMALEN; //data len for hal_fill_agg_start or hal_fill_priv
    unsigned short         MPDULEN;    // frame length
    unsigned long         hostreserve;    //pointer of ptxdesc
    unsigned char          Delimiter;///Delimiter padding
    unsigned char          Flag;
    unsigned short         Seqnum;
    unsigned short         HTC;
    unsigned short         FrameControl;
    unsigned short     	PrivDmaAddr;
    unsigned char          EncryptType;
} ;

 struct hi_agg_tx_desc
{
    unsigned char              RateTableMode;
    unsigned char              TxPower;
    unsigned char              StaId;  //assoc id
    unsigned char              KeyId;
    unsigned char              EncryptType;
    unsigned char              TID;
    unsigned char              CurrentRate;
    unsigned char              TxTryRate1;
    unsigned char              TxTryRate2;
    unsigned char              TxTryRate3;
    unsigned char              TxTryNum0;
    unsigned char              TxTryNum1;
    unsigned char              TxTryNum2;
    unsigned char              TxTryNum3;
    unsigned short             FLAG;   //wifi flag, such as short gi, rts en
    unsigned short             FLAG2;  //tx descriptor flag, such as checksum, mic
    unsigned short             AGGR_len;   //total len of mpdus
    unsigned char              TxPrivNum;  //only mac1.0 use
    unsigned short             MpduNum;    //number of mpdus
    int                              vid;
    int                             queue_id;
    unsigned short            HiP2pNoaCountNow;
    unsigned char             aggr_page_num;
    unsigned char             reserve[3];
} ;

#if defined (HAL_FPGA_VER)
#define READ_LEN_PER_ONCE               (96 * 1024)
#define READ_BUFFERABLE_LEN             8 * 1024
#elif defined (HAL_SIM_VER)
#define READ_LEN_PER_ONCE               (32 * 1024)
#endif

// last 1024 word memory of sram for dpd training
#define DPD_MEMORY_ADDR (0x00b00000 + (512 * PAGE_LEN) - (8 * PAGE_LEN))
#define DPD_MEMORY_LEN (1024 * 4)

//(0x00b00000 + (512 * PAGE_LEN) - (64 * PAGE_LEN))
#define SRAM_FWLOG_BUFFER (0x00b00000 + (512 * PAGE_LEN) - (16 * PAGE_LEN))
#define SRAM_FWLOG_BUFFER_LEN (1024 * 4) //4K

struct hal_layer_ops
{
    struct sk_buff *(*hal_fill_agg_start)(struct hi_agg_tx_desc* HI_AGG,struct hi_tx_priv_hdr* HI_TxPriv);
    struct sk_buff *(*hal_fill_priv)(struct hi_tx_priv_hdr* HI_TxPriv,unsigned char TID);
    int (*hal_calc_mpdu_page)(int mpdulen);
    int (*hal_get_agg_cnt)(unsigned char TID);
    int (*hal_get_priv_cnt)(unsigned char TID);
    int (*hal_init)(void * drv_prv);
    int (*hal_exit)(void);
    int (*hal_probe)(void);
    int (*hal_open)(void * drv_prv);
    int (*hal_close)(void * drv_prv);

#if defined (HAL_FPGA_VER)
    void (*hal_exit_priv)(void);
    void (*hal_init_priv)(void);
    void (*hal_reinit_priv)(void);
    struct hw_interface* (*hif_get_hw_interface)(void);
    struct aml_hal_call_backs* (*get_hal_call_back_table)(void);
#endif

    unsigned int (* phy_enable_bcn)(unsigned char wnet_vif_id,unsigned short BeaconInterval, unsigned short DtimPeriod, unsigned char BssType);
    unsigned int (* phy_set_bcn_buf)(unsigned char wnet_vif_id,unsigned char *pBeacon, unsigned short len,unsigned short Rate,unsigned short Flag);
    unsigned int (* phy_switch_chan)(unsigned short channel, unsigned char bw, unsigned char restore);
    unsigned int (* phy_set_rf_chan)(struct hal_channel *hchan, unsigned char flag, unsigned char vid, unsigned char opmode);
    unsigned int (* phy_set_mac_bssid)(unsigned char wnet_vif_id,unsigned char * Bssid);
    unsigned int (* phy_set_mac_addr)(unsigned char wnet_vif_id,unsigned char * MacAddr);
    unsigned int (* phy_vmac_disconnect)(unsigned char wnet_vif_id);
    unsigned int (* phy_init_hmac)(unsigned char wnet_vif_id);
    unsigned int (* phy_disable_net_traffic)(void);
    unsigned int (* phy_update_bcn_intvl)(unsigned char wnet_vif_id,unsigned short BcnInterval);
    unsigned int (* phy_rst_mcast_key)(unsigned char wnet_vif_id);
    unsigned int (* phy_rst_ucast_key)(unsigned char wnet_vif_id);
    unsigned int (* phy_rst_all_key)(unsigned char wnet_vif_id);
    unsigned int (* phy_clr_key)(unsigned char wnet_vif_id,unsigned short StaAid);
    unsigned int (* phy_set_ucast_key)(unsigned char wnet_vif_id,unsigned short StaAid, unsigned char *pMac, unsigned char *pKey, unsigned char keyLen, unsigned char encryType, unsigned char keyId);
    unsigned int (* phy_set_mcast_key)(unsigned char wnet_vif_id,unsigned char *pKey, unsigned char keyLen, unsigned int keyId,unsigned char encryType,unsigned char AuthRole);
    unsigned int (* phy_set_rekey_data)(unsigned char wnet_vif_id, void *rekey_data, unsigned short StaAid);
    unsigned int (* phy_set_preamble_type)( unsigned char PreambleType);
    unsigned int (* phy_register_sta_id)(unsigned char vid,unsigned short StaAid,unsigned char *pMac, unsigned char encrypt);
    unsigned int (* phy_unregister_sta_id)(unsigned char wnet_vif_id,unsigned short StaAid);
    unsigned int (* phy_unregister_all_sta_id)(unsigned char wnet_vif_id);

    unsigned int (*AmlhF_ShowStatus)(void);
    unsigned int (*phy_addba_ok)(unsigned char wnet_vif_id,unsigned short StaAid,unsigned char TID,unsigned short SeqNumStart,unsigned char BA_Size,unsigned char AuthRole,unsigned char BA_TYPE);
    unsigned int (*phy_delt_ba_ok)(unsigned char wnet_vif_id,unsigned short StaAid,unsigned char TID,unsigned char AuthRole);
    unsigned int (*phy_get_extern_chan_status) (void);

    unsigned int (*phy_set_lretry_limit)(unsigned int data);
    unsigned int (*phy_set_sretry_limit)(unsigned int data);
    
    int (*hal_reg_task)(TaskHandler task);
    int (*hal_call_task)(SYS_TYPE taskid, SYS_TYPE param1);
    int (*hal_get_agg_pend_cnt)(void);
    unsigned int (*phy_set_cam_mode)(unsigned char wnet_vif_id,unsigned char OpMode);
    unsigned int (*phy_set_dhcp)(unsigned char wnet_vif_id,unsigned int ip);
    unsigned int (*phy_set_chan_support_type)(struct hal_channel *chan);
    unsigned int (*phy_set_chan_phy_type)(int);
    unsigned char  (*hal_tx_empty)(void);
    unsigned char *  (*hal_get_config)(void);
    void  (*phy_scan_cmd)(unsigned int data);// 1 start,0 end
    void (*phy_set_tx_power_accord_rssi)(int bw, unsigned short channel, unsigned char rssi, unsigned char power_mode);
    void (*phy_set_channel_rssi)(unsigned char rssi);
    unsigned int (*phy_pwr_save_mode)(unsigned char wnet_vif_id,unsigned int data);// 1 start,0 end
    unsigned int (*phy_set_rd_support)(unsigned char wnet_vif_id, unsigned int data);
    unsigned int (*phy_set_txlive_time)(unsigned int  txlivetime);//ms
    unsigned int (*phy_set_slot_time)( unsigned int  slot);//us
    unsigned int (*phy_set_bcn_intvl)(unsigned char wnet_vif_id,unsigned int  bcninterval);//us
    unsigned int (*phy_set_ac_param)(unsigned char wnet_vif_id,unsigned char ac,unsigned char aifsn,unsigned char cwminmax,unsigned short txop);
    int (*hal_tx_flush)(unsigned char vid);
    int (*phy_set_p2p_opps_cwend_enable)(unsigned char vid, unsigned char p2p_oppps_cw);
    int (*phy_set_p2p_noa_enable)(unsigned char vid, unsigned int duration, unsigned int interval, unsigned int starttime, unsigned char count, unsigned char flag);
    unsigned long long (*phy_get_tsf)(unsigned char vid);
    int (*hal_txframe_pause)(int pause);
    void (*Hal_RxerrRst_trigger)(int dbg_rxreset);
    unsigned int (*phy_send_null_data)(struct NullDataCmd null_data, int len);

    unsigned int (*phy_keep_alive)(struct NullDataCmd null_data_param, int null_data_len, int enable, int period);
    unsigned int (*phy_set_beacon_miss)(unsigned char vid, unsigned char enable, int period);
    unsigned int (*phy_set_vsdb)(unsigned char vid, unsigned char enable);
    unsigned int (*phy_set_arp_agent)(unsigned char vid, unsigned char enable, unsigned int ipv4,
        unsigned char * ipv6);
    unsigned int (*phy_set_pattern)(unsigned char vid, unsigned char offset, unsigned char len,
        unsigned char id, unsigned char *mask, unsigned char *pattern);
    int (*phy_set_suspend)(unsigned char vid, unsigned char enable, unsigned char mode, unsigned int filters);
    unsigned int (*phy_get_rw_ptr)(unsigned char vid);
    void (*hal_get_sts)(unsigned int op_code, unsigned int ctrl_code);

    int (*hal_set_suspend)(unsigned char enable);

    void (*hal_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*hal_read_word)(unsigned int addr);

    void (*hal_bt_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*hal_bt_read_word)(unsigned int addr);

    void (*hal_pt_rx_start)(unsigned int qos);
    void (*hal_pt_rx_stop)(void);

    // sdio pmu
    unsigned char (*hal_get_fw_ps_status)(void);
    unsigned char (*hal_check_fw_wake)(void);

    // beamforming
    unsigned int (*phy_send_ndp_announcement)(struct NDPAnncmntCmd ndp_annc);
    void (*phy_set_bmfm_info)(int wnet_vif_id, unsigned char *group_id, unsigned char * user_position, unsigned char feedback_type);
    unsigned int (*phy_set_coexist_en)( unsigned char enable);
    unsigned int (*phy_set_coexist_max_miss_bcn)( unsigned int miss_bcn_cnt);
    unsigned int (*phy_set_coexist_req_timeslice_timeout_value)( unsigned int timeout_value);
    unsigned int (*phy_set_coexist_not_grant_weight)( unsigned int not_grant_weight);
    unsigned int (*phy_set_coexist_max_not_grant_cnt)( unsigned int coexist_max_not_grant_cnt);
    unsigned int (*phy_set_coexist_scan_priority_range)( unsigned int coexist_scan_priority_range);
    unsigned int (*phy_set_coexist_be_bk_noqos_priority_range)( unsigned int coexist_scan_priority_range);
    unsigned int (*phy_coexist_config)(const void *data, int data_len);

    unsigned int (*phy_interface_enable)(unsigned char enable, unsigned char vid);
    unsigned int (*hal_set_fwlog_cmd)(unsigned char mode);
    unsigned int (*hal_cfg_cali_param)(void);
    unsigned int (*hal_cfg_txpwr_cffc_param)(void * chan,void * txpwr_plan);
};

#define HAL_FW_IN_ACTIVE  BIT(0)
#define HAL_FW_IN_AWAKE  BIT(1)
#define HAL_FW_IN_SLEEP  BIT(2)
#define HAL_FW_STS_MASK  (HAL_FW_IN_SLEEP | HAL_FW_IN_AWAKE | HAL_FW_IN_ACTIVE)

#define HAL_DRV_IN_SLEEPING  BIT(6) // driver set sleep cmd
#define HAL_DRV_IN_ACTIVE  BIT(7) // driver set cmds or tx frames
#define HAL_DRV_STS_MASK  (HAL_DRV_IN_ACTIVE | HAL_DRV_IN_SLEEPING)

 struct hal_private
{
#if defined (HAL_FPGA_VER)
#define WORK_ITEM_NUM 100
    struct  hal_work_task WorkFifoBuf[WORK_ITEM_NUM];
    struct _CO_SHARED_FIFO WorkFifo;
    SYS_TYPE WorkFifo_task_id;
    unsigned char ah_macaddr[6];
    unsigned char hi_task_stop;
    unsigned char bRfInit;
    unsigned char need_scheduling_tx;
#endif

    struct task_struct * work_thread;
    struct semaphore work_thread_sem;
    /* thread completion */
    struct completion  work_thread_completion;
    int work_thread_quit;

    unsigned int  ctrl_wait_flag;
    unsigned int  ctrl_register_flag;
    struct  aml_work_task_t ctrl_task[MAX_WORK_TASK];

#if defined (HAL_FPGA_VER)
    struct task_struct* rx_thread;
    struct semaphore rx_thread_sem;
    struct completion rx_thread_completion;
    int rx_thread_quit;

    struct task_struct* txok_thread;
    struct semaphore txok_thread_sem;
    int txok_thread_quit;
    struct completion txok_thread_completion;

    struct task_struct* hi_irq_thread;
    struct semaphore hi_irq_thread_sem;
    struct completion hi_irq_thread_completion;
    int hi_irq_thread_quit;

    struct task_struct* hi_dpd_tx_thread;
    struct semaphore hi_dpd_tx_thread_sem;
    struct completion hi_dpd_tx_thread_completion;
    int hi_dpd_tx_thread_quit;

#if defined(CONFIG_HAS_WAKELOCK)
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,14,29)
    struct wake_lock  wakelock[WAKE_LOCK_MAX];
#endif
#endif

#endif

    unsigned char dhcp_offload;
    struct fw_event_to_driver *fw_event;
    unsigned char hal_fw_event_counter;

    unsigned int tx_page_offset;
    unsigned int rx_host_offset;

    unsigned char hal_suspend_mode;
    atomic_t sdio_suspend_cnt;
    atomic_t drv_suspend_cnt;

    struct _CO_SHARED_FIFO txds_trista_fifo[HAL_NUM_TX_QUEUES];
    struct fw_txdesc_fifo  txds_trista_fifo_buf[HI_AGG_TXD_NUM_ALL_QUEUE];

    struct hal_layer_ops hal_ops; // exported
    struct hw_interface* hif;        // call next layer

    void *drv_priv;
    struct aml_hal_call_backs * hal_call_back;
    void *Hi_TxAgg[WIFI_MAX_TXQUEUE_ID];  //save the pointer of TxPageInfo
    struct tx_complete_status *txcompletestatus;
    struct Tx_FrameDesc tx_frames[WIFI_MAX_TXFRAME];
    unsigned long tx_frames_map[BITS_TO_LONGS(WIFI_MAX_TXFRAME)];
    unsigned int txPageFreeNum;  //the num of free tx pages
    struct unicastReplayCnt  uRepCnt[WIFI_MAX_VID][WIFI_MAX_STA];
    struct multicastReplayCnt mRepCnt[WIFI_MAX_VID];

    struct OS_TQ_STRUCT hi_tasktq;      /* hi intr tasklet */
    OS_MUTEX hal_phy_mutex;
    OS_MUTEX power_mutex;

    OS_LOCK tx_spinlock;
    OS_LOCK com_spinlock;
    OS_LOCK pn_spinlock;

    unsigned long tx_spinlock_flag;
    unsigned long com_spinlock_flag;
    unsigned long pn_spinlock_flag;

    unsigned char hst_if_init_ok; // shared for DMA & SDIO
    unsigned char hst_if_irq_en; //shared for DMA & SDIO
    unsigned char bhalOpen;
    unsigned char bhalProbelok;
    unsigned char bhalMKeySet[4]; //for all vmac_id, now for mkey reset
    unsigned char bhalUKeySet[4]; //for all vmac_id, now for ukey reset
    unsigned char bWpaMicMeasureEnable;

    unsigned int int_status_copy; //0x70  is read-clean REG,save for debug
    unsigned char HalTxFrameDoneCounter;  //old counter of frames have been tx completed
    unsigned int HalRxFrameDoneCounter;  //old counter of frames have been rxed from firmware
    unsigned short HalTxPageDoneCounter;   //old counter of pages have been tx completed

    unsigned char use_irq;
    int tx_queueid_downloadok; //the id of queue which has been downloaded ok, init as 0
    unsigned int beaconaddr[WIFI_MAX_VID];
    int bhaltxdrop;
    //when driver insmod and remod, the value is -1 as a flag.
    unsigned char bhalPowerSave;
    unsigned char powersave_init_flag;
    unsigned char hal_fw_ps_status;
    unsigned char hal_drv_ps_status;
    int rssi;   //counted after rxed acked frames responsed txed frames
    unsigned int rtc_func_enable;
    unsigned int ps_host_state;
    unsigned int fec_coding;
    struct sta_connect_msg sta_con_msg;

#if defined (HAL_FPGA_VER)
    unsigned int sts_hirq[hirq_max_idx +1];
    unsigned int gpio_irq_cnt;
    /* for host sw statistic*/
    struct sts_sw_cnt_ctrl sts_hst_sw[sts_hst_sw_max_idx];
    unsigned int sts_en_bcn[2];
    unsigned int sts_dis_bcn[2];
#endif
};

/*** aml platform***/
struct platform_wifi_gpio
{
    unsigned int gpio_wakeup;
    unsigned int gpio_reset;
    unsigned int gpio_irq;
    unsigned int   gpio_irq_mode;
    unsigned int   irq_num;
    unsigned int gpio_rtc;
    unsigned int clk_sel; // 0: FPGA CLK 1: SSV RF
    unsigned int filter_num;
};

enum
{
    WIFI_GPIO_IRQ_HIGH,
    WIFI_GPIO_IRQ_LOW,
    WIFI_GPIO_IRQ_RISING,
    WIFI_GPIO_IRQ_FALLING
};

extern struct platform_wifi_gpio amlhal_gpio;

/*** aml sdio***/

 enum cust_gpio_modes
{
    WLAN_RESET_ON,
    WLAN_RESET_OFF,
    WLAN_POWER_ON,
    WLAN_POWER_OFF
} ;

 struct sdio_rw_desc
{
    unsigned int     addr;               /* request data */
    unsigned char *    buf;
    unsigned int     len;
    short   status;
    unsigned char      func;
    unsigned char      flag;
    unsigned char      rw_flag;
} ;

 struct sdio_syn_desc
{
    struct sdio_rw_desc desc;
    struct semaphore syn_rw_sem;
    char  b_inused;
} ;

#define AMLSD_DBG_REQ       (1<<1)
#define AMLSD_DBG_RESP      (1<<2)
#define AMLSD_DBG_IOS       (1<<9)
#define AMLSD_DBG_CLKC      (1<<11)


/*for status cnt control*/

/*** aml test***/
/********************************* aml test***********************************************/

#define THREAD_SWITCH_INTERVAL_ONCE      	(5000)  //ms

#define HAL_TEST_ENABLE    			BIT(0)
#define HAL_TEST_HELPINFO    			BIT(1)

#define TEST_WIFI_SRAM_32              		BIT(4)
#define TEST_WIFI_ICCM_32              		BIT(5)
#define TEST_WIFI_DCCM_32              		BIT(6)
#define TEST_WIFI_ICCM_ROM_32              	BIT(7)
#define TEST_WIFI_SRAM              		BIT(8)
#define TEST_WIFI_ICCM              		BIT(9)
#define TEST_WIFI_DCCM              		BIT(10)
#define TEST_WIFI_ICCM_ROM              	BIT(11)
#define DUMP_WIFI_REGISTER          		BIT(12)
#define MEASURE_SDIO_CMD_RW            		BIT(13)
#define MEASURE_SDIO_THROUGHPUT    		BIT(14)
#define MEASURE_WIFI_THROUGHPUT    		BIT(15)
#define TEST_WIFI_EFUSE             		BIT(16)

#define HAL_TEST_DEFAULT    			(0)
#define TEST_PRINT  				pr_debug

#define INBUFFER_LEN_VALID_BIT 			(256-2)
#define INBUFFER_LEN 				(256/8)
#define ADC_WIDTH               		24
#define BLOCK_REAL_LEN       			(INBUFFER_LEN_VALID_BIT/ADC_WIDTH*4)

#define INBUFFER_LEN 				(256/8)
#define INBUFFER_LEN_VALID_BIT 			(256-2)
#define INBUFFER_LEN_VALID 			((INBUFFER_LEN_VALID_BIT)/8)

#define DA 					0	//LG

#define SW					9
#define FCS_ERR					3
#define AGC_FSM_TRIG				0



#if defined (HAL_SIM_VER)
/*********************************  aml txdesc***********************************************/
#define BITS_PER_BYTE			8
#define OFDM_PLCP_BITS                  22
#define L_STF                           8
#define L_LTF                           8
#define L_SIG                           4
#define HT_SIG                          8
#define HT_GF_STF                       8
#define HT_LTF1                         8
#define HT_STF                          4

#define HT_LTF_STREAM1                  4
#define VHT_SIG_A                       8
#define VHT_STF                         4
#define VHT_LTF                         4
#define VHT_SIG_B                       4

#define MCS_2_STREAMS(_mcs)             ((((_mcs)&0x78)>>3)+1)
#define SYMBOL_TIME(_ns)                ((_ns) << 2)            // ns * 4 us
#define SYMBOL_TIME_HALFGI(_ns)         (((_ns) * 18 + 4) / 5)  // ns * 3.6 us
#define NUM_SYMBOLS_PER_USEC(_usec)     (_usec >> 2)
#define NUM_SYMBOLS_PER_USEC_HALFGI(_usec) (((_usec*5)-4)/18)
#define SIGNAL_EXTENSION_VALUE          6              /* Value of the signal extension          */
#endif

#define DESC_PREMBLETYPE  tmp_dot11_preamble_type
#define DESC_RIFS  wifi_conf_mib.dot11Rifs
#define DESC_RATE  HiTxDesc->CurrentRate
#define DESC_FLAG  HiTxDesc->FLAG
#define DESC_VID  HiTxDesc->vid
#define DESC_BANDWIDTH ( ((HiTxDesc->FLAG&WIFI_CHANNEL_BW_MASK) >> WIFI_CHANNEL_BW_OFFSET)&0x3)
#define DESC_IS_AMPDU !!(HiTxDesc->FLAG&WIFI_IS_AGGR)
#define DESC_IS_SHORTGI  !!(HiTxDesc->FLAG&WIFI_IS_SHORTGI)
#define DESC_LEN HiTxDesc->AGGR_len
#define DESC_IS_GREEN 0

#define MAX_HI_CMD 0xff

#define PHY_TEST 20

#define IS_OFMD_RATE(_r)                ((_r)>WIFI_11B_11M)


/* Mpdu expand length */
/* WEP  */
#define DP_SEC_WEP_IV_LEN          	4    /* IV field Length  */
#define DP_SEC_WEP_ICV_LEN         	4    /* ICV field Length */
#define DP_SEC_WEP_EXP_LEN (DP_SEC_WEP_IV_LEN + DP_SEC_WEP_ICV_LEN)/* WEP expand length */

/* TKIP  */
#define DP_SEC_TKIP_IV_LEN         	8    /* IV field Length */
#define DP_SEC_TKIP_MIC_LEN        	8    /* MIC length  */
#define DP_SEC_TKIP_ICV_LEN        	4    /* ICV field Length */
#define DP_SEC_TKIP_MPDU_EXP_LEN (DP_SEC_TKIP_IV_LEN + DP_SEC_TKIP_ICV_LEN)

/* CCMP   */
#define DP_SEC_CCMP_IV_LEN         	8    /* IV field Length */
#define DP_SEC_CCMP_MIC_LEN        	8    /* MIC length */
#define DP_SEC_CCMP_EXP_LEN (DP_SEC_CCMP_IV_LEN + DP_SEC_CCMP_MIC_LEN )

/* WAPI  */
#define DP_SEC_WAPI_PN_LEN         	16    /* IV field Length */
#define DP_SEC_WAPI_MIC_LEN        	16    /* MIC length */
#define DP_SEC_WAPI_EXP_LEN (DP_SEC_WAPI_PN_LEN + DP_SEC_WAPI_MIC_LEN +2)

 struct wifi_cfg_mib
{
    unsigned char          dot11LongRetryLimit;    //not use now
    unsigned char          dot11ShortRetryLimit;   //not use now
    /*  MODE_IBSS = 0,MODE_AP = 1,MODE_STA = 2, MODE_WDS = 3*/
    unsigned char          dot11CamMode;
    unsigned int            dot11RtsThreshold;  //not use now
    unsigned short         dot11CurrentTxPowerLevel;   //not use now
    unsigned short         dot11FragmentationThreshold;    //now not use
    unsigned short         dot11BeaconInterval;    //not use now
    unsigned short         dot11SlotTime;  //not use now
    unsigned short         BasicRateTbl;   //not use now
    unsigned short         SupportRateMap; //not use now
    unsigned char          dot11PreambleType;
    unsigned char          dot11Rifs;  //not use now
    unsigned int            cmddata[MAX_HI_CMD];    //save param for each cmd
};
///
/// Rate Control class. All members are private.
///
struct rate_control
{
    unsigned int bit_time;                      ///< Shifted time for each bit, = 2^18 / rate.
    //unsigned short ack_time;                      ///< ACK and CTS-to-self time in us including SIFS.
    unsigned char  rts_rate;                      ///< ACK and CTS-to-self time in us including SIFS.
    unsigned char  ack_rate;                      ///< ACK and CTS-to-self rates from the BasicRateSet.
    unsigned char  tx_power;                      ///< Transmit power for the given rate.
    unsigned char  reserve;                      ///< Reservations
} ;

 struct ht_mcs_control
{
    unsigned char  rts_rate;                      ///< ACK and CTS-to-self time in us including SIFS.
    unsigned char  ack_rate;                      ///< ACK and CTS-to-self rates from the BasicRateSet.
    unsigned char  tx_power;                      ///< Transmit power for the given rate.
    unsigned char  reserve;                      ///< Reservations
};

 struct vht_mcs_control
{
    unsigned char  rts_rate;                      ///< ACK and CTS-to-self time in us including SIFS.
    unsigned char  ack_rate;                      ///< ACK and CTS-to-self rates from the BasicRateSet.
    unsigned char  tx_power;                      ///< Transmit power for the given rate.
    unsigned char  reserve;                      ///< Reservations
} ;

extern struct wifi_cfg_mib  wifi_conf_mib;

/*** aml  uno***/
//for TxPrivDescripter->Flag
#define WIFI_MORE_BUFFER            	BIT(0)
#define WIFI_FIRST_BUFFER           	BIT(1)
#define WIFI_MORE_AGG                   	BIT(2)

enum
{
    TX_DESCRIPTOR_STATUS_NEW,
    TX_DESCRIPTOR_STATUS_SUCCESS,
    TX_DESCRIPTOR_STATUS_RTS_FAIL,
    TX_DESCRIPTOR_STATUS_ACK_FAIL,
    TX_DESCRIPTOR_STATUS_AGED,
    TX_DESCRIPTOR_STATUS_DISCARD,
    TX_DESCRIPTOR_STATUS_DELBA,
    TX_DESCRIPTOR_STATUS_P2P_NOA_WITHDRAW,
    TX_DESCRIPTOR_STATUS_NULL_DATA_OK,
    TX_DESCRIPTOR_STATUS_NULL_DATA_FAIL,
    TX_DESCRIPTOR_STATUS_PHY_ERROR,
    TX_DESCRIPTOR_STATUS_LENGTH_ERROR,
    TX_DESCRIPTOR_STATUS_DELIMIT_ERROR,
};

//RateTableMode
enum
{
    PHY_MODE_11A2 = 0,
    PHY_MODE_11B2 = 1,
    PHY_MODE_11G = 2,
    PHY_MODE_11NA_HT20 = 3,
    PHY_MODE_11NG_HT20 = 4,
    PHY_MODE_11NA_HT40 = 5,
    PHY_MODE_11NG_HT40 = 6
};

#define CHAN_BW_20M 0 // 0 bit8 and bit9 are both 0
#define CHAN_BW_40M 1
#define CHAN_BW_80M 2

//FLAG
#define WIFI_IS_AGGR                BIT(0)
#define WIFI_IS_NOACK               BIT(1)
#define WIFI_IS_RTSEN               BIT(2)
#define WIFI_IS_CTSEN               BIT(3)
#define WIFI_IS_SHORTGI             BIT(4)
#define WIFI_IS_PMF                 BIT(5)
#define WIFI_IS_BURST               BIT(6)
#define WIFI_IS_Group               BIT(7)
#define WIFI_CHANNEL_BW_MASK        (BIT(8)|BIT(9))
#define WIFI_IS_SHORTGI1            BIT(10)
#define WIFI_IS_SHORTGI2            BIT(11)
#define WIFI_CHANNEL_BW_OFFSET      8
#define WIFI_IS_FRAGMENT            BIT(13)
#define WIFI_IS_BLOCKACK            BIT(14)
#define WIFI_IS_DYNAMIC_BW       BIT(15)

//
///Flag2
//
#define TX_DESCRIPTER_NOT_FREE      BIT(0)
#define TX_DESCRIPTER_BEACON        BIT(2)
#define TX_DESCRIPTER_RD_SUPPORT    BIT(3)
#define TX_DESCRIPTER_HTC           BIT(6)
#define TX_DESCRIPTER_CHECKSUM      BIT(7)
#define TX_DESCRIPTER_MIC           BIT(8)
#define TX_DESCRIPTER_P2P_PS_NOA_TRIGRSP  BIT(9)
#define TX_DESCRIPTER_ENABLE_TX_LDPC  BIT(10)


#define RX_PHY_NOWEP    0
#define RX_PHY_WEP64    1
#define RX_PHY_TKIP     2
#define RX_PHY_WAPI     3
#define RX_PHY_CCMP     4
#define RX_PHY_WEP128   5

enum hal_op_mode
{
    WIFI_M_IBSS     =0x0,/* IBSS (adhoc) station */
    WIFI_M_STA      =0x1,/* infrastructure station */
    WIFI_M_HOSTAP   =0x2,/* Software Access Point */
    WIFI_M_WDS      =0x3,/* WDS link */
    WIFI_M_MONITOR   =0x4/* Monitor mode */
} ;

/***calibration config struct***/
 struct product_cal_mib_v3
{
    unsigned int calibration_flag;
    ///rx frequency calibration
    unsigned int RxFreqReg[14];
    ///rx IQ mismatch calibration
    unsigned int RxIqMismatchReg;
    ///rx DC calibration
    unsigned int RxDcReg;
    int  Ant_loss_db;
    int  noisefloor;
    unsigned char TxScaleID[8][14];
    ///Tx IqMismatch
    unsigned int TxIqMismatchReg;
    ///tx dc
    //txdci(5: 0) and txdcq(11: 6)
    unsigned int TxDcReg;
    ///MAC address
    unsigned char MacAddr[6];
    unsigned char reserve1[2];
    int  Ant_loss_gen;
    unsigned char reserve2[2];
    unsigned short sum;
} ;

struct aml_hal_call_backs
{
    /* callbacks regist to hal layer for interrupt behind handle */
    int  (*get_defaultcfg)(void *  drv_prv, void* para);
    void (*mic_error_event)(void *  drv_prv,const void * wh,unsigned char * sa,unsigned char wnet_vif_id);
    void (*intr_tx_handle)(void *drv_prv, struct txdonestatus *tx_done_status, SYS_TYPE callback, unsigned char queue_id);
    void (*intr_tx_ok_timeout)(void *drv_prv);
    void (*intr_tx_pkt_clear)(void *drv_prv);
    void (*intr_rx_handle)(void *drv_prv,struct sk_buff *skb,unsigned char Rssi,unsigned char RxRate, unsigned char bw,
        unsigned char channel,  unsigned char aggr, unsigned char wnet_vif_id,unsigned char keyid);
    int (*pmf_encrypt_pkt_handle)(void *drv_prv, struct sk_buff *skb, unsigned char rssi, unsigned char RxRate,
        unsigned char channel,  unsigned char aggr, unsigned char wnet_vif_id,unsigned char keyid);
    void (*intr_bcn_send)(void *  drv_prv,unsigned char wnet_vif_id);
    void (*intr_dtim_send)(void *  drv_prv,unsigned char wnet_vif_id);
    void (*intr_ba_recv)(void *  drv_prv,unsigned char wnet_vif_id);
    void (*event_indicate)(void *  drv_prv,unsigned char wnet_vif_id,unsigned char eventid,unsigned int param);
    int  (*get_stationid)(void *  drv_prv,unsigned char *mac,unsigned char  wnet_vif_id,int * staid);
    void (*intr_gotodeepsleep)(void *  drv_prv);
    void (*intr_gotowakeup)(void *  drv_prv,unsigned char wnet_vif_id);
    int (*dev_probe)(void);
    int  (*dev_remove)(void);
    void (*intr_p2p_opps_cwend)(void *  drv_prv,unsigned char wnet_vif_id);
    void (*intr_p2p_noa_start)(void *  drv_prv,unsigned char wnet_vif_id);
    void (*intr_fw_event)(void *  drv_prv, void *event);
    void (*intr_tx_nulldata_handle)(void* drv_prv, struct tx_nulldata_status * tx_null_status);
    void (*intr_beacon_miss_handle)(void * drv_prv, unsigned char wnet_vif_id);
    void (*drv_intr_bt_info_change)(void * dpriv, unsigned char wnet_vif_id,unsigned char bt_lk_info );
    void (*drv_pwrsave_wake_req)(void * dpriv, int power_save);
};

struct hal_private*  hal_get_priv(void);
struct hw_interface* hif_get_hw_interface(void);

void print_driver_version(void);

#endif
