
#ifdef CONFIG_SDIO_IF
/*
 *****************************************************************************************
 *
 * Copyright (C) amlogic 2018
 *
 * Project:  wifi
 *
 * Description:
 *      This file interface with host driver used sdio bus/AHB bus
 *
 *
 *
 ****************************************************************************************
 */
#ifndef _FI_SDIO_H
#define _FI_SDIO_H

#define PRODUCT_AMLOGIC 0x8888
#define VENDOR_AMLOGIC  0x8888
#define  HOST_VERSION   1234
#define FW_VERSION_W1   0x1
#define FW_VERSION_W1U  0x2

 /* memory mapping for wifi space */
#define MAC_ICCM_AHB_BASE    0x00000000
#define MAC_SRAM_BASE        0x00a10000
#define MAC_REG_BASE         0x00a00000
#define MAC_DCCM_AHB_BASE    0x00d00000
#define PHY_REG_AGC_BASE     0x00a08000
#define PHY_AGC_BUSY_FSM          (PHY_REG_AGC_BASE+0x34)

#define DF_AGC_REG_A12 (PHY_REG_AGC_BASE + 0x30)
#define DF_AGC_REG_A27 (PHY_REG_AGC_BASE + 0x6c)
#define REG_STF_AC_Q_THR (PHY_REG_AGC_BASE + 0x5c)
#define REG_ED_THR_DB (PHY_REG_AGC_BASE + 0x74)


/*
* BT device baseAddr seen from wifi system side
* is different from BT device actual baseAddr at BT system side.
* For example, actual ICCM baseAddr at BT system may be 0x00900000,
* but ICCM baseAddr seen from wifi system side may be 0x00300000
*/
#define WF2BT_APB_BASE      (0x00200000)  //BT APB baseAddr seen from wifi system side
#define WF2BT_ICCM_RAM      (0x00300000)  //BT DCCM RAM baseAddr seen from wifi system side
#define WF2BT_DCCM_RAM      (0x00400000)  //BT DCCM RAM baseAddr seen from wifi system side
#define WF2BT_MAC_ARC       (WF2BT_APB_BASE + 0x0c)
#define RW_OPERTION_SIZE    (4)

//base address
#define   PRODUCT_AMLOGIC_ADDR  (MAC_SRAM_BASE+0x000000)
#define   VENDOR_AMLOGIC_ADDR  (MAC_SRAM_BASE+0x000004)
#define   FIRMWARE_VERSION_ADDR  (MAC_SRAM_BASE+0x000008)
#define   HOST_VERSION_ADDR  (MAC_SRAM_BASE+0x00000C)
#define   HW_CONFIG_ADDR  (MAC_SRAM_BASE+0x000010)
#define   FW_IRQ_TIME_ADDR  (MAC_SRAM_BASE+0x000028)

#define   APP_CMD_PERFIFO_LEN  64
#define   APP_CMD_FIFO_NUM  2
#define   CMD_DOWN_FIFO_CTRL_ADDR  (MAC_SRAM_BASE+0x000030)
#define   CMD_DOWN_FIFO_FDB_ADDR  (CMD_DOWN_FIFO_CTRL_ADDR)
#define   CMD_DOWN_FIFO_FDN_ADDR  (CMD_DOWN_FIFO_CTRL_ADDR+4)
#define   CMD_DOWN_FIFO_FDH_ADDR  (CMD_DOWN_FIFO_CTRL_ADDR+8)
#define   CMD_DOWN_FIFO_FDT_ADDR  (CMD_DOWN_FIFO_CTRL_ADDR+12)
#define   CMD_DOWN_FIFO_BASE_ADDR  (MAC_SRAM_BASE+0x000040)

#define   CMD_UP_FIFO_CTRL_ADDR  (MAC_SRAM_BASE+0x0000c0)
#define   CMD_UP_FIFO_FDB_ADDR  (CMD_UP_FIFO_CTRL_ADDR)
#define   CMD_UP_FIFO_FDN_ADDR  (CMD_UP_FIFO_CTRL_ADDR+4)
#define   CMD_UP_FIFO_FDH_ADDR  (CMD_UP_FIFO_CTRL_ADDR+8)
#define   CMD_UP_FIFO_FDT_ADDR  (CMD_UP_FIFO_CTRL_ADDR+12)
#define   CMD_UP_FIFO_BASE_ADDR  (MAC_SRAM_BASE+0x0000d0)

/*wifi operate mode */
#define MODE_IBSS 0
#define MODE_STA 1
#define MODE_AP 2
#define MODE_WDS 3


#define PAGE_LEN  512//256
#define WIFI_MAX_VID  2
#define WIFI_MAX_TID  8
#define WIFI_MAX_STA  16
#define WIFI_MIN_STA_ID  1
#define WIFI_MAX_TXQUEUE_ID  10
#define WIFI_MAX_TXFRAME  128//64//48
#define WIFI_MAX_FW_EVENT 4
/* shijie.chen add,
 * for expanding buffer of rxcomplete status to avoid overlapping or overwriting. */
#define WIFI_MAX_RXFRAME  255 //127
#define PHY_VMAC_ID  (0x20)
#define TX_ADDRESSTABLE_NUM  256
#ifdef DHCP_OFFLOAD
#define RX_TMP_MAX_LEN  512
#else
#define RX_TMP_MAX_LEN  256
#endif
#define MAX_PAGE_NUM  20 /*max number page in a mpdu*/

#define DISABLE_BEACON 0
#define ENABLE_BEACON 1
#define PREAMBLE_LONG 0
#define PREAMBLE_SHORT 1

///BA_TYPE
#define immidiate_BA_TYPE 1

///AuthRole
#define BA_INITIATOR 0
#define BA_RESPONDER 1

///encrypt type
#define WIFI_NO_WEP  0x0
#define WIFI_WEP64  0x1
#define WIFI_TKIP  0x2
#define WIFI_AES  0x4
#define WIFI_WEP128  0x5
#define WIFI_WEP256  0x6
#define WIFI_WPI  0x7

#define TX_ERROR_IRQ  BIT(0)
#define RX_ERROR_IRQ  BIT(1)
#define RX_OK  BIT(2)
#define TX_OK  BIT(3)
#define BEACON_SEND_OK_VID0  BIT(4)
#define BEACON_SEND_OK_VID1  BIT(5)
#define GOTO_WAKEUP_VID0  BIT(6)
#define GOTO_WAKEUP_VID1  BIT(7)
#define P2P_OPPPS_CWEND_VID0  BIT(8)
#define P2P_OPPPS_CWEND_VID1  BIT(9)
#define FW_EVENT  BIT(10)
#define P2P_NoA_START_VID1  BIT(11)
#define BEACON_RECV_OK_VID0  BIT(12)
#define BEACON_RECV_OK_VID1  BIT(13)
#define BEACON_MISS_VID0 BIT(14)
#define BEACON_MISS_VID1 BIT(15)
#define IRQMASK  0xffff
#define FW_RX_PTR_MASK  0xffff0000
#define FW_RX_PTR_OFFSET  16

/*Rate definition*/
#define WIFI_11N_MASK  0x80
#define WIFI_11AC_MASK  0xc0
#define WIFI_RATE_MASK  0xf0
#define WIFI_RATE_INVALID  0xff

#define IS_CCK_RATE(_r)  (((_r) == WIFI_11B_1M) | ((_r) == WIFI_11B_2M) | ((_r) == WIFI_11B_5M) | ((_r) == WIFI_11B_11M))
#define IS_HT_RATE(_r)  (((_r)&(WIFI_RATE_MASK))==WIFI_11N_MASK)
#define IS_VHT_RATE(_r) (((_r)&(WIFI_RATE_MASK))==WIFI_11AC_MASK)
#define IS_MCS_RATE(_r) ((_r)&(WIFI_RATE_MASK))
#define GET_HT_MCS(_r)  ((_r)&(~WIFI_RATE_MASK))
#define GET_VHT_MCS(_r) ((_r)&(~WIFI_RATE_MASK))
#define GET_MCS(_r)     ((_r)&(~WIFI_RATE_MASK))

#define WIFI_11B_1M  0x00
#define WIFI_11B_2M  0x01
#define WIFI_11B_5M  0x02
#define WIFI_11B_11M 0x03

#define WIFI_11G_6M   0x04
#define WIFI_11G_9M   0x05
#define WIFI_11G_12M  0x06
#define WIFI_11G_18M  0x07
#define WIFI_11G_24M  0x08
#define WIFI_11G_36M  0x09
#define WIFI_11G_48M  0x0a
#define WIFI_11G_54M  0x0b
#define WIFI_11BG_MAX (WIFI_11G_54M)

#define WIFI_11N_MCS0  0x80
#define WIFI_11N_MCS1  0x81
#define WIFI_11N_MCS2  0x82
#define WIFI_11N_MCS3  0x83
#define WIFI_11N_MCS4  0x84
#define WIFI_11N_MCS5  0x85
#define WIFI_11N_MCS6  0x86
#define WIFI_11N_MCS7  0x87
#define WIFI_11N_MAX   0x07

#define WIFI_11AC_MCS0  0xc0
#define WIFI_11AC_MCS1  0xc1
#define WIFI_11AC_MCS2  0xc2
#define WIFI_11AC_MCS3  0xc3
#define WIFI_11AC_MCS4  0xc4
#define WIFI_11AC_MCS5  0xc5
#define WIFI_11AC_MCS6  0xc6
#define WIFI_11AC_MCS7  0xc7
#define WIFI_11AC_MCS8  0xc8
#define WIFI_11AC_MCS9  0xc9
#define WIFI_11AC_MAX   0x09

#define WIFI_RATE_MAX  (WIFI_11G_54M)


/*WME stream classes*/
/* best effort */
#define WME_AC_BE   0
/* background */
#define WME_AC_BK   1
/* video */
#define WME_AC_VI   2
/* voice */
#define WME_AC_VO   3

/**
 * Access category value are mapped on the BuP ACP: (Highest priority => Lowest AC)
 */
enum
{
        QUEUE_TID_BK =1,
        QUEUE_TID_BK2=2,
        QUEUE_TID_BE =0 ,
        QUEUE_TID_EE =3,
        QUEUE_TID_CL =4 ,
        QUEUE_TID_VI =5,
        QUEUE_TID_VO =6 ,
        QUEUE_TID_NC =7,
        QUEUE_NO_QOS= 8,
        QUEUE_MANAGE= 9,
        QUEUE_BEACON= 10,
        ACCESS_MAX_NO_ACTIVE =   11
};

/*WME priorities mapped to the corresponding access categories*/
//bup queue=0~9, queue 9 = fastiqc, others can be sw defined, now only use 1~7
//iac use the father's queue
//ctrl frame use the father's queue

enum
{
        QUEUE_AC_MIN = 0,
        /* manage */
        QUEUE_AC_MANAGE = 0,
        /* no qos */
        QUEUE_AC_NO_QOS = 1,
        /*  Spare BK */
        QUEUE_AC_0 = 2,
        /* Excellent Effort BE */
        QUEUE_AC_1 = 3,
        /* Controlled Load  VI */
        QUEUE_AC_2 = 4,
        /* VO */
        QUEUE_AC_3 = 5,
        /* beacon */
        QUEUE_AC_BEACON = 6,
        QUEUE_AC_MAX = 7,
        QUEUE_AC_FIAC = 9
};

//firmware version
#define FW_UNUSED_IRQ_VERSION   0x0000
#define FW_IRQ_VERSION          FW_VERSION_W1

/* Ack policy */
/*Explicit BA*/
#define BLOCKACK  0x3
#define NOACK  0x1
/*Normal ack or Implicit BA */
#define ACK  0x0

/* [1:0]: MPDU flag: indicates the current buffer location of the MPDU data.*/
#define HW_FIRST_MPDUBUF_FLAG  BIT(0)
#define HW_LAST_MPDUBUF_FLAG  BIT(1)
#define HW_FIRST_AGG_FLAG  BIT(2)
#define HW_LAST_AGG_FLAG  BIT(3)

/* [5:2]: reserved */
/* [7]: Beacon and Probe Response Flag: indicates whether the current
frame is Beacon or Probe Response frame.*/

//mpdu len = frame len, not include fcs and encrypt ext
#define HW_MPDU_LEN_SET(_len)  ((_len)<<6)
#define HW_MPDU_LEN_GET(_len)  (((_len)>>6)&0x3fff)
#define HW_BUFFER_LEN_SET(_len)  ((_len)<<20)
#define HW_BUFFER_LEN_GET(_len)  (((_len)>>20)&0xfff)
#define HW_BUFFER_LEN_CLEAR(_len)  ((_len)&0xfffff)

#pragma pack(1)
typedef struct HW_TxBufferInfo
{
        unsigned int MPDUBufFlag;
        /* indicate to next buffer address */
        unsigned int MPDUBufNext;
        /* indicate to current data buffer address */
        unsigned int MPDUBufAddr;
} HW_TxBufferInfo;

#pragma pack()
//policy
/*
   TX buffer number, TX Descriptor Flag and TXVECTOR
   [9:0]: TX buffer number: indicates the total number of TX buffer in the current
   TX frame.
   [10]: TX Descriptor more Flag: indicates whether the current TX Descriptor is followed by another one
   [11]: encryption flag, indicates whether the transmission frame should be encrypted.
   1'b0: not be encrypted; 1: should be encrypted.
   [12]: WEP flag, indicates whether the encryption type is WEP64 or WEP128.
   [14:13]: fragment flag: indicates the current fragment location of the MSDU
   2'b00: the middle fragment of the MSDU;
   2'b01: the first fragment of the MSDU;
 */


//HT_SIGN1
/*
   TXVECTOR
   [1:0]: format
   [4:2]: nonht_mod
   [8:5]: tx_pwr
   [16:9]: tx_rate:
   [18:17]: Channel_BW
   [20:19]: Channel_OFFSET
   [21]: preamble_type
   [22]: GI_type
   [23]: channel_offset25, it's valid only in 11b frame.
0:Donnot rotate 25M away from current channel
1:rotate 25M away from current channel
[31:24]: antenna_set
 */
#define NON_HT      0
#define HT_MF       1
#define VHT         3


enum
{
        SMOOHING_NOT_REC = 0,
        SMOOHING_REC =1
};
enum
{
        NOT_SOUNDING = 0,
        SOUNDING =1
};
enum
{
        COMPRESSED_SV = 0,
        NON_COMPRESSED_SV = 1,
        CSI_MATRICES = 2
};

// Channel_BW
#define  SW_CBW20         0
#define  SW_CBW40         1
#define  SW_CBW80         2


/* channel offset, primary channel relative to central channel */
/* pri = cent */
#define  SW_COFF_0   0
/* pri = cent + 10M */
#define  SW_COFF_U10M   1
/* pri = cent - 10M */
#define  SW_COFF_L10M   2
/* pri = cent + 30M */
#define  SW_COFF_U30M   3
/* pri = cent - 30M */
#define  SW_COFF_L30M   4

// define rf sample rate 
#define RF_SMP_160 (0x2)


//tv_nonht_mod
#define ERP_DSSS            0
#define ERP_CCK             1
#define ERP_OFDM            2
#define ERP_PBCC            3
#define DSSS_OFDM           4
#define OFDM                    5
#define NON_HT_DUP_OFDM 6

#pragma pack(1)

typedef struct hw_tx_vector_bits
{
        /*
           [0:3]: reserved
           [4]:encrypted_disable: indicates the frame encrypted disable,1'b1: disable;1'b0: enable (so default to be enabled).
           [5]:htc_modify: indicates the frame contain htc field,1'b1: contain;1'b0:no contain. ????
           [6]:sq_valid: indicates the sequence information that replaces Sequence field of the frame is valid.
           [7]:fc_valid: indicates the frame control information that replaces FC field of the frame is valid.
           [8]:du_valid: indicates the duration information that replaces duration field of the frame is valid.??????
           [9]:Beacon and Probe Resp: indicates whether the current frame is Beacon or Probe Response frame.
           [10]:tx_mic_enable: indicates this frame is do mic operation
           [11]:llength_hw_calc: l_length calculated by HW
           [12]:duration_hw_calc: duration calculated by HW
           [13:14]:reserved
           [15]:control_wrapper: The format of the Control Wrapper frame. ????
           [16:19]:tid_num: Any of the identifiers usable by higher layer entities to distinguish medium access
           control (MAC) service data units (MSDUs) to MAC entities that support quality of service (QoS) within the MAC data service.
           There are 16 possible TID values; eight identify TCs, and the other eight identify parameterized TSs.
           The TID is assigned to an MSDU in the layers above the MAC.
           [20:21]:virtual device id: indicates the ID number of virtual device which send the TX frame.
           [22]:ackto_en: Ack timeout enable signal
           [23:31]:ackto_value: Ack timeout value
         */
        unsigned int tv_reserved0:1,
                     tv_fw_duration_valid:1,
                     tv_rty_flag:1,
                     /*ip/tcp/udp checksum hw calculate enable, hw will check if ip/tcp/udp */
                     tv_txcsum_enbale:1,
                     /*the final encrypted function is decided by tv_encrypted_disable and "encrypt type" */
                     tv_encrypted_disable:1,
                     /*not use, must be 0 */
                     tv_htc_modify:1,
                     /*default 0, only beacon use 1 */
                     tv_sq_valid:1,
                     /*now 1, always use tv_FrameControl of txvector */
                     tv_fc_valid:1,
                     /*be 1 for frame from host (not include beacon, beacon duration=0), be 0 for ctrl frame from fw */
                     tv_du_valid:1,
                     /*be 1 for Beacon and Probe Resp */
                     tv_beacon_flag:1,
                     /*only for tkip mic */
                     tv_tkip_mic_enable:1,
                     /*now 0 */
                     tv_llength_hw_calc:1,
                     tv_ack_policy:2,
                     tv_single_ampdu:1,
                     /*now 0 */
                     tv_control_wrapper:1,
                     /*now 0 */
                     tv_tid_num:4,
                     /*wnet_vif_id */
                     tv_wnet_vif_id:2,
                     /*Ack timeout enable, for no WIFI_IS_NOACK, such as data/rts/bar */
                     tv_ack_to_en:1,
                     /*unit us */
                     tv_ack_timeout:9;

        /*
           [0:11]: l_length
           FORMAT is NON_HT:
           Indicates the length of the PSDU in octets in the range of 1 to 4095. This
           value is used by the PHY to determine the number of octet transfers that
           occur between the MAC and the PHY.
           FORMAT is HT_MF:
           Indicates the value in the Length field of the L-SIG in the range of 1 to
           4095. This use is defined in 9.23.4. This parameter may be used for the
           protection of more than one PPDU as described in 9.23.5.

           [12:31]:length/apep_length:
           Indicates the length of an HT PSDU in the range of 0 to 65 535 octets.
           A value of 0 indicates a NDP that contains no data symbols after the HT preamble (see 20.3.9).
           If equal to 0, indicates a VHT NDP PPDU for both RXVECTOR and TXVECTOR.
           If greater than 0 in the TXVECTOR, indicates the number of  octets in the range 1 to 1 048 575 in the A-MPDU pre-EOF
           padding (see 9.12.2) carried in the PSDU. This parameter is used to determine the number of
           OFDM symbols in the Data field that do not appear after a subframe with 1 in the EOF subfield and,
           after being rounded up to a 4 octet boundary with the two LSBs removed, is placed in the VHT-SIG-B Length field.
           NOTE?The rounding up of the APEP_LENGTH parameter to a 4-octet word boundary could result in a value that is larger than
           the PSDU_LENGTH calculated using the equations in 22.4.3.
           If greater than 0 in the RXVECTOR, this parameter is the value obtained from the VHT-SIG-B Length field multiplied by 4.
        */
        /* ieee80211 ht txvector member, non ht length */
        unsigned int tv_L_length:12,
                     /* ieee80211 ht txvector member, ht length, equal to Fw_TxPriv.AggrLen */
                     tv_length:20;

        /*
           [0:15]:frame control info:it replaces FC field of the frame when fc_valid bit is set.
           [16:31]:duration information: it replaces duration field of the frame when du_valid bit is set.
         */
        unsigned short tv_FrameControl;
        unsigned short tv_txop_time:15,
                       tv_burst_en:1;

        /*
           [0:15]:sequence control info:it replaces sequence control field of the frame when sq_valid bit is set.
           [16]:aggregation :
           Indicates whether the PSDU contains an A-MPDU.
           Enumerated type:
           AGGREGATED indicates this packet has A-MPDU aggregation.
           NOT_AGGREGATED indicates this packet does not have A-MPDU aggregation.
           [17]:no_sig_extn:
           Indicates whether signal extension needs to be applied at the end of transmission.
           Boolean values:
           true indicates no signal extension is present.
           false indicates signal extension may be present depending on other
           TXVECTOR parameters (see 20.2.2).

           [18:19]:stbc:
           Indicates the difference between the number of space-time streams (Nsts)
           and the number of spatial streams (Nss) indicated by the MCS as follows:
           0 indicates no STBC (Nsts==Nss ).
           1 indicates Nsts-Nss=1.    2 indicates Nsts-Nss=2.    Value of 3 is reserved.

           [20]:DYN_BANDWIDTH_IN_NON_HT:
           "0" : static;"1" : dynamic

           [21]:txop_ps_not:
           Indicates whether a VHT AP allows non-AP VHT STAs in TXOP power save mode to enter Doze state during the TXOP.
           0 indicates that the VHT AP allows non-AP VHT STAs to enter doze mode during a TXOP.
           1 indicates that the VHT AP does not allow non-AP VHT STAs to enter doze mode during a TXOP.
         */

        unsigned int tv_SequenceNum:16,
                     /*ieee80211 ht txvector member*/
                     tv_ampdu_flag:1,
                     /*ieee80211 ht txvector member, related to 11Mbps*/
                     tv_no_sig_extn:1,
                     /*ieee80211 ht txvector member, now 0*/
                     tv_STBC:2,
                     tv_dyn_bw_nonht:1,
                     tv_txop_ps_not:1,
                     tv_reserved2:10;

        /*
           [0:1]: user position: user position in mu
           [2:7]: Group id: Group id in mu
           [8:16]: partial aid: partial aid
           [17:18]: num_users: Indicates the number of users with non-zero space-time streams.Integer: range 1 to 4.
           [19]: sounding:
           Indicates whether this packet is a sounding packet.
           Enumerated type:
           SOUNDING indicates this is a sounding packet.
           NOT_SOUNDING indicates this is not a sounding packet.

           [20]: fec_coding:
           Indicates which FEC encoding is used.
           Enumerated type:
           BCC_CODING indicates binary code.
           LDPC_CODING indicates low-density parity check code.

           [21:22]: num_exten_ss
           Indicates the number of extension spatial streams that are sounded during
           the extension part of the HT training in the range of 0 to 3.

           [23:24]: num_tx:The N_TX parameter indicates the number of transmit chains.
           [25:27]: num_sts:
           Indicates the number of space-time streams. Integer: range 1-8 for SU, 1-4 per user in the TXVECTOR and 0-
           4 in the RXVECTOR for MU. NUM_STS summed over all users is in the range 1 to 8.

           [28:29]: CH_BANDWIDTH_IN_NON_HT:
           Indicates the channel width of the transmitted PPDU when non ht

           [30:31]: expansion_mat_type:
           Enumerated type:
           COMPRESSED_SV indicates that EXPANSION_MAT is a set of
           compressed beamforming feedback matrices.
           NON_COMPRESSED_SV indicates that EXPANSION_MAT is a set of
           noncompressed beamforming feedback matrices.
           CSI_MATRICES indicates that EXPANSION_MAT is a set of channel
           state matrices.
         */
        unsigned int tv_usr_postion: 2,
                     tv_group_id:6,
                     tv_partial_id:9,
                     tv_num_users:2,
                     /* ieee80211 ht txvector member, now 0 */
                     tv_sounding:1,
                     /* ieee80211 ht txvector member, now 0 */
                     tv_FEC_coding:1,
                     /* ieee80211 ht txvector member, now 0 */
                     tv_num_exten_ss:2,
                     /* ieee80211 ht txvector member, now 1 */
                     tv_num_tx:2,
                     tv_num_sts:3,
                     tv_ch_bw_nonht:2,
                     /* ieee80211 ht txvector member, now 1 */
                     tv_expansion_mat_type:2;
        /*
           [0:1]: format: TXVECTOR_PHASE1 format
           [2:4]: nonht_mod: TXVECTOR_PHASE1 nonht_mod
           [5:8]:tx_pwr: TXVECTOR_PHASE1 tx_pwr
           [9:16]:tx_rate: TXVECTOR_PHASE1 tx_rate
           [17:18]:channel_bw: TXVECTOR_PHASE1 Channel_BW,  0: HT_CBW20;      1: HT_CBW40;  2: NON_HT_CBW20; 3: NON_HT_CBW40
           [19:20]:channel_offset: TXVECTOR_PHASE1 Channel_OFFSET   0:CH_OFF_20;   1:CH_OFF_40;  2:CH_OFF_20U;  3:CH_OFF_20L
           [21]:preamble_type: TXVECTOR_PHASE1 preamble_type
           [22]:gi_type: TXVECTOR_PHASE1 GI_type
           [23]:beamformed: Set to 1 if a beamforming steering matrix is applied to the waveform in an SU transmission
           [24:31]:antenna_set: TXVECTOR_PHASE1 antenna_set
         */

        union
        {
                struct HT_SIGN
                {
                        /*ieee80211 ht txvector member, now HT_MF or NON_HT */
                        unsigned int tv_format:2,
                                     /*ieee80211 ht txvector member, now ERP_DSSS, ERP_CCK, ERP_OFDM, NON_HT_DUP_OFDM*/
                                     tv_nonht_mod:3,
                                     /*ieee80211 ht txvector member, but org 1~8, here 0~15, mapped to different regs*/
                                     tv_tx_pwr:4,
                                     /*ieee80211 ht txvector member use L_rate, use what ?? now means real rate for non-ht/ht*/
                                     tv_tx_rate:8,
                                     /*ieee80211 ht txvector member, HT_CBW20 or HT_CBW40 for ht,NON_HT_CBW20 or
                                        NON_HT_CBW40 for non ht*/
                                     tv_Channel_BW:2,
                                     tv_cfend_flag:1,
                                     tv_bar_flag:1,
                                     /*ieee80211 ht txvector member, PREAMBLE_LONG or PREAMBLE_SHORT, 1Mbps always
                                        use PREAMBLE_LONG*/
                                     tv_preamble_type:1,
                                     /*ieee80211 ht txvector member*/
                                     tv_GI_type:1,
                                     tv_beamformed:1,
                                     /*ieee80211 ht txvector member, now 0*/
                                     tv_antenna_set:4,
                                     tv_check_cca_bw:1,
                                     tv_reserved:1,
                                     tv_rts_flag:1,
                                     tv_cts_flag:1;
                } htbit ;
                unsigned int    tv_HT_SIGN1;
        } tv_ht;

        /*
           [0:31]: head index: indicates the address of the first TX buffer entry.
         */

        unsigned int tv_MPDUInfoAddr;
} hw_tx_vector_bits,V_hw_tx_vector_bits;

typedef union
{
        struct
        {
                /*ieee80211 ht txvector member, now HT_MF or NON_HT */
                unsigned int tv_format:2,
                             /*ieee80211 ht txvector member, now ERP_DSSS, ERP_CCK, ERP_OFDM, NON_HT_DUP_OFDM*/
                             tv_nonht_mod:3,
                             /*ieee80211 ht txvector member, but org 1~8, here 0~15, mapped to different regs*/
                             tv_tx_pwr:4,
                             /*ieee80211 ht txvector member use L_rate, use what ?? now means real rate for non-ht/ht*/
                             tv_tx_rate:8,
                             /*ieee80211 ht txvector member, HT_CBW20 or HT_CBW40 for ht, NON_HT_CBW20 or
                                NON_HT_CBW40 for non ht*/
                             tv_Channel_BW:2,
                             tv_cfend_flag:1,
                             tv_bar_flag:1,
                             /*ieee80211 ht txvector member, PREAMBLE_LONG or PREAMBLE_SHORT, 1Mbps always
                                use PREAMBLE_LONG*/
                             tv_preamble_type:1,
                             /*ieee80211 ht txvector member*/
                             tv_GI_type:1,
                             tv_beamformed:1,
                             /*ieee80211 ht txvector member, now 0*/
                             tv_antenna_set:4,
                             tv_check_cca_bw:1,
                             tv_reserved:1,
                             tv_rts_flag:1,
                             tv_cts_flag:1;
        } htbit ;
        unsigned int tv_HT_SIGN1;
} FwPriv_tv_ht;

typedef struct HW_CtrlTxDescripter
{
        struct hw_tx_vector_bits TxVector;
        unsigned int MPDUBufFlag;
        /* indicate to next buffer address */
        unsigned int NextMPDUInfoAddr;
        unsigned int MPDUBufAddr;
        unsigned char FrameBuffer[32];
} HW_CtrlTxDescripter,*PV_HW_CtrlTxDescripter;

//copied from Fw_TxPriv
typedef struct txdonestatus
{
        unsigned char txstatus;
        /* txid for driver call back and free skb */
        unsigned char callbackid;
        /* for rate control */
        unsigned char txretrynum:4,
                            txsretrynum:4;
        unsigned char ack_rssi;
} txdonestatus;

struct tx_nulldata_status
{
    unsigned char txstatus;
    /* frame type, if just a event, it is null */
    unsigned char frame_type;
    /* vmac id */
    /* tid number */
    unsigned char wnet_vif_id:4,
                        tid:4;
    /* qos null data */
    /* power save */
    /* power save flag */
    unsigned char qos:1,
                        ps:1,
                        pwr_flag:1;
};

typedef struct tx_complete_status
{
    /* counter of frames have been tx completed */
    unsigned char txdoneframecounter;
    /* have been send manage frame sequence num, now 0, not use */
    unsigned char txmanageframecounter;
    /* counter of pages have been tx completed */
    unsigned short txpagecounter;
    unsigned char reserve[4];
    union
    {
        struct txdonestatus txstatus;
        struct tx_nulldata_status tx_null_status;
    }tx_status[WIFI_MAX_TXFRAME];
} tx_complete_status;

enum fw_event_type
{
  DHCP_OFFLOAD_EVENT,
  VSDB_SWITCH_PREPARE_EVENT,
  VSDB_CHANNEL_SWITCH_EVENT,
  TBTT_EVENT,
};

struct fw_event_basic_info
{
    unsigned char event;
    unsigned char vid;
    unsigned char len;//event_data length
    unsigned char reserved;
};

struct fw_event_no_data
{
    struct fw_event_basic_info basic_info;
    unsigned char reserved[28];
};

struct dhcp_offload_event
{
    struct fw_event_basic_info basic_info;
    unsigned char local_ip[4];
    unsigned char server_ip[4];
    unsigned char subnet_mask[4];
    unsigned char gw[4];
    unsigned char dns1[4];
    unsigned char dns2[4];
};

typedef struct fw_event_to_driver
{
    unsigned char event_counter;
    unsigned char reserved[3];
    union {
        unsigned char data[32];
        struct fw_event_no_data normal_event;
        struct dhcp_offload_event dhcp_event;
    } event_data[WIFI_MAX_FW_EVENT];
} fw_event_to_driver;

typedef struct Fw_TxPriv
{
        /* hash from vid/staid/tid */
        unsigned int pTxInfo;
        /* frame exchange duration */
        unsigned int FrameExchDur;
        unsigned int TxLiveTime;
        /* such as WIFI_IS_RTSEN */
        unsigned short Flag;
        /* such as TX_DESCRIPTER_BEACON */
        unsigned short Flag2;
        unsigned short HiP2pNoaCountNow;
        /* ampdu len. packetlen(frame len(mpdu len) + fcs + encrypt ext len) + delimiter(for ampdu) */
        unsigned short AggrLen;
        /* sequence number */
        unsigned short SN;
        unsigned char TxCurrentRate;
        /* mpdu num in ampdu */
        unsigned char AggrNum;
        /* retry num, for tx data/manage */
        unsigned char txretrynum;
        /* short retry num, for tx ctrl frame */
        unsigned char txsretrynum;
        /* QUEUE_TID_BK~QUEUE_BEACON */
        unsigned char TID;
        /* for sta, 1; for ap, sta_associd as unicast, 1 as broadcast (beacon) */
        unsigned char StaId;
        /* pages num of (a)mpdu */
        unsigned char PageNum;
        /* such as TX_DESCRIPTOR_STATUS_SUCCESS ... */
        unsigned char txstatus;
        /* rssi of ack responsed to the txed frame, for tx rate control */
        unsigned char ackrssi;
        /* wnet_vif_id */
        unsigned char vid;
        /* save the txid for single mpdu, the txids map for ampdu, see Hal_alloc_tx_id */
        unsigned char hostcallbackid;
        unsigned char reserve;
        /* index 1/2/3 mapped to TxRetry index 0/1/2. index 0 as the initial value of TxCurrentRate */
        unsigned char TxRate[4];
        /*  index 0/1/2 used. index 3 not use */
        unsigned char TxRetry[4];
        /* one ampdu consumed total tx page */
        unsigned char aggr_page_num;
        unsigned char Reserve2;
        unsigned int next_mpdu_addr;
        unsigned int mpdu_last_page;
} Fw_TxPriv;

/*use in HW_TxVector option address, for key */
typedef struct HW_TxOption
{
    unsigned char is_bc;
    unsigned char key_type;
    unsigned char pkt_position;
    unsigned char KeyIdex;
    //tkip/ccmp use 8B, wpi use 16B
    unsigned char PN[16];
} HW_TxOption;

#define HI_TXDESC_DATAOFFSET  ((size_t) &(((struct hi_tx_desc *)0)->txdata))
#define HI_FIRST_PAGE_DATA_LEN  (PAGE_LEN-HI_TXDESC_DATAOFFSET)

typedef struct hi_tx_desc
{
        unsigned int MPDUBufFlag;// 4
        struct hw_tx_vector_bits TxVector; //28
        struct HW_TxOption TxOption;//20
        struct Fw_TxPriv TxPriv; // 44
        unsigned char txdata[1];//36
} hi_tx_desc;


//8 -32 -20-44-152
#define FW_TXDESC_DATAOFFSET  ((size_t) &(((struct TxDescPage *)0)->txdata))
#define FW_FIRST_PAGE_DATA_LEN  (PAGE_LEN-FW_TXDESC_DATAOFFSET)
#define FW_TXDESC_X_LEN  ((size_t) sizeof(struct Fw_TxPriv)+sizeof(struct HW_TxOption)+sizeof(struct hw_tx_vector_bits))

typedef struct TxDescPage
{
        struct HW_TxBufferInfo BufferInfo;  // 4*3=12
        struct hw_tx_vector_bits TxVector; // 4*7=28
        struct HW_TxOption TxOption;// 1*20=20
        struct Fw_TxPriv TxPriv;       //44
        unsigned char txdata[1]; //
} TxDescPage;

//8 -198
#define PAGE_DATA_LEN   (PAGE_LEN-sizeof( struct HW_TxBufferInfo))
typedef struct OtherTxPage
{
        struct HW_TxBufferInfo BufferInfo;  //12
        unsigned char txdata[PAGE_DATA_LEN];
} OtherTxPage;

#pragma pack()
#define HI_RXPRIV_STATUS_OFFSET (0x20-4)
/*RxDecryptType
  8'h00: No cipher;  8'h01: WEP64;  8'h02: TKIP;  8'h03: reserved;
  8'h04: CCMP;  8'h05: WEP128;  8'h06: reserved;  8'h07: WAPI;
 */
#define RX_PHY_NOWEP   0
#define RX_PHY_WEP64   1
#define RX_PHY_TKIP    2
#define RX_PHY_WAPI    3
#define RX_PHY_CCMP    4
#define RX_PHY_WEP128  5
#pragma pack(1)

#define RX_PRIV_HDR_LEN  sizeof(HW_RxDescripter_bit)
typedef struct HW_RxDescripter_bit
{
        unsigned char PN[16];
        /* rx rssi */
        unsigned char RxRSSI_ant0;
        unsigned char RxRSSI_ant1;
        /* now not read, but filled with dot11Channel */
        unsigned char RxChannel;
        /* real rate */
        unsigned char RxRate;
        unsigned int RxAntenna:2,
                     ch_bwd_innonht:2,
                     dyn_bwd_innonht:1,
                     RxA1match:1,
                     /* net_vif_id for the matched A1 */
                     RxA1match_id:2,
                     RxShortGI:1,
                     Channel_BW:2,
                     RxPreambleType:1,
                     aggregation:1,
                     /* for key mic check */
                     mic_err:1,
                     /* for key miss check */
                     keymiss_err:1,
                     /* for key icv check */
                     icv_err:1,
                     /* rx mpdu len = frame len, not include fcs or encrypt ext */
                     RxLength:14,
                     reserved1:2 ;
        unsigned int RxTime:26,
                     bssidmatch_id:2,
                     partial_aid:1,
                     beamformed:1,
                     Channel_OFFSET:2;
        /* next free page address for rx */
        unsigned short Next_RxPage;
        /* such as RX_PHY_TKIP ...*/
        unsigned char RxDecryptType:3,
                      reserved2:1,
                      /* defined by UPLOAD_HOST_ENABLE in register */
                      upload2host_flag:1,
                      /* key id */
                      key_id:2,
                      reserved1e:1;
        /* rtl return error flag : 0: no error 1: error; */
        unsigned char RxTcpCSUM_err:1,
                      RxIPCSUM_err:1,
                      /* hw calculate tcp checksum */
                      RxTcpCSUMCalculated:1,
                      /* F offset, hw calculate ip checksum */
                      RxIPCSUMCalculated:1,
                      channel_bw_rts:2;
        unsigned char data[0];
} HW_RxDescripter_bit, V_HW_RxDescripter_bit;


typedef struct _Irq_Time
{
        unsigned short MaxTimerOut;
        unsigned short MinInterval;
} Irq_Time;

/* struct use to set rx packet date address to host driver */
typedef struct _FIFO_SHARE_CTRL
{
        /* 4 byte Base DMA address of Rx descripter */
        unsigned int    FDB;
        /* 4 Byte fifo descripter num */
        unsigned int    FDN;
        /* 4 Byte fifo descripter head */
        unsigned int    FDH;
        /* 4 Byte fifo descripter tail */
        unsigned int    FDT;
        /* 4 Byte fifo ctrl  base address */
        unsigned int    FCB;
        /* 4 Byte fifo per descripter len */
        unsigned int    FDL;
} FIFO_SHARE_CTRL;

typedef struct _FIFO_SHARE_CTRL_PACKET
{
        /* 4 byte Base DMA address of Rx descripter */
        unsigned int    FDB;
        /* 4 Byte fifo descripter num */
        unsigned int    FDN;
        /* 4 Byte fifo descripter head, save hw mac write pointer */
        unsigned int    FDH1;
        /* 4 Byte fifo descripter head, save fw sw read pointer */
        unsigned int    FDH2;
        /* 4 Byte fifo descripter tail */
        unsigned int    FDT;
        /* 4 Byte fifo ctrl  base address */
        unsigned int    FCB;
        /* 4 Byte fifo per descripter len */
        unsigned int    FDL;
        /* pointer to a valid data that need upload to driver.
         driver will get it value when leave suspend.*/
        unsigned int    FDH3;
} FIFO_SHARE_CTRL_PACKET;

typedef struct HW_CONFIG
{
        unsigned short txpagenum;
        unsigned short rxpagenum;
        unsigned int beaconframeaddress;
        unsigned int rxframeaddress;
        unsigned int txcompleteaddress;
        unsigned short bcn_page_num;
        unsigned short flags;
        unsigned int fweventaddress;
} HW_CONFIG;

#pragma pack()
typedef struct _FI_STATUS
{
        unsigned int TX_SEND_REQ_mpdu_num;
        unsigned int TX_SEND_REQ_PAGE_num;
        unsigned int TX_SEND_OK_mpdu_num;
        unsigned int TX_SEND_OK_PAGE_num;
} FI_STATUS;


///9.get rssi
typedef struct
{
        unsigned char len;
        unsigned char rssi[7];
        int padc;
        int icgain;
        unsigned char len_err;
        unsigned char rssi_err[7];
        int padc_err;
        int icgain_err;
} rssi_s;

typedef struct
{
        unsigned int len_agc;
        unsigned int agc_learn_d[20];
} agc_learn_s;


typedef struct UniCastKeyCmd
{
        unsigned char Cmd;
        unsigned char vid;
        unsigned char StaId;
        unsigned char KeyType;
        unsigned char KeyLen;
        unsigned char KeyId;
        unsigned char Rsc[16];
        unsigned char Key[32];
        unsigned char PNStep;
} UniCastKeyCmd;

/*Multicast_Key_Set_Cmd*/
typedef struct MultiCastKeyCmd
{
        unsigned char Cmd;
        unsigned char vid;
        unsigned char KeyId;
        unsigned char KeyType;
        unsigned char KeyLen;
        unsigned char AuthRole; //not use now
        unsigned char PN[16];
        unsigned char Key[32];
        unsigned char PNStep;
} MultiCastKeyCmd;

#define KEY_KEK_LENGTH (16)
#define KEY_KCK_LENGTH (16)
#define KEY_REPLAY_CTR_LENGTH (8)

typedef struct RekeyDataCmd
{
        unsigned char Cmd;
        unsigned char vid;
        unsigned char StaId;
        unsigned char kek[KEY_KEK_LENGTH];
        unsigned char kck[KEY_KCK_LENGTH];
        unsigned char replay_counter[KEY_REPLAY_CTR_LENGTH];
        //unsigned int akmp;
} RekeyDataCmd;

/* Power_Save_Cmd */
/* active, no power save*/
#define PS_ACTIVE   0x0
/* awake, in power save */
#define PS_AWAKE   0x1
/* doze/sleep, in power save */
#define PS_DOZE   0x2
/* one vmac sleep, but keep in awake */
#define PS_NETWORK_SLEEP   0x3

#define P2P_NOA_START_MATCH_BEACON  BIT(5)
#define P2P_NOA_END_MATCH_BEACON    BIT(6)

#define P2P_NoA_START_FLAG(m)       ((m)&1)
#define P2P_NoA_START_COUNT(m)      ((m)>>1)

/* wake up filter in wow mode */
#define WOW_FILTER_OPTION_ANY BIT(1)
#define WOW_FILTER_OPTION_MAGIC_PACKET BIT(2)
#define WOW_FILTER_OPTION_EAP_REQ BIT(3)
#define WOW_FILTER_OPTION_4WAYHS BIT(4)
#define WOW_FILTER_OPTION_DISCONNECT BIT(5)
#define WOW_FILTER_OPTION_GTK_ERROR BIT(6)
#endif// _FI_AHB_H
#endif// #ifdef CONFIG_SDIO_IF

