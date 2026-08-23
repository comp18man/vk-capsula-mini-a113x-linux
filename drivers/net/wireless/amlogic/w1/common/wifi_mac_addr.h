#ifndef __WIFI_MAC_ADDR_H__
#define __WIFI_MAC_ADDR_H__

#define WIFI_MAC (0x0000)
#define RG_MAC_COUNT1 (WIFI_MAC + 0xb0)
#define RG_MAC_LOCAL_ADDRL0 (WIFI_MAC+0X1c)
#define RG_MAC_LOCAL_ADDRH0 (WIFI_MAC+0X20)

#define RG_MAC_BSSID0LO_REG (WIFI_MAC+0x3c)
#define RG_MAC_BSSID0HI_REG (WIFI_MAC+0x40)

#define RG_MAC_COUNT2 (WIFI_MAC+0Xb4)
#define RG_MAC_CHANNEL_INDEX (WIFI_MAC+0X1e0)
#define RG_MAC_BSSIDLO_N(_vid)    (RG_MAC_BSSID0LO_REG+(_vid)*8)
#define RG_MAC_BSSIDHI_N(_vid)    (RG_MAC_BSSID0HI_REG+(_vid)*8)
#define RG_MAC_LOCAL_ADDRLN(_vid) (RG_MAC_LOCAL_ADDRL0+(_vid)*8)
#define RG_MAC_LOCAL_ADDRHN(_vid) (RG_MAC_LOCAL_ADDRH0+(_vid)*8)

#define RG_MAC_CNT_CTRL_FIQ (WIFI_MAC + 0x5c )
#define RG_MAC_FRM_TYPE_CNT_CTRL (WIFI_MAC + 0xa4)
#define RG_MAC_RX_DATA_LOCAL_CNT (WIFI_MAC + 0x1b8)
#define RG_MAC_RX_DATA_OTHER_CNT (WIFI_MAC + 0x1bc)
#define RG_MAC_RX_DATA_MUTIL_CNT (WIFI_MAC + 0x1c0)
#define RG_MAC_TX_STT_CHK (WIFI_MAC + 0x1a8)
#define RG_MAC_TX_END_CHK (WIFI_MAC + 0x1ac)
#define RG_MAC_TX_EN_CHK (WIFI_MAC + 0x1b0)
#define RG_MAC_TX_ENWR_CHK (WIFI_MAC + 0x1b4)

#define RG_MAC_IRQ_STATUS_CNT0 (WIFI_MAC + 0x60)
#define RG_MAC_IRQ_STATUS_CNT1 (WIFI_MAC + 0x64)
#define RG_MAC_IRQ_STATUS_CNT2 (WIFI_MAC + 0x68)
#define RG_MAC_IRQ_STATUS_CNT3 (WIFI_MAC + 0x74)
#define RG_MAC_FRM_TYPE_CNT0 (WIFI_MAC + 0xa8)
#define RG_MAC_FRM_TYPE_CNT1 (WIFI_MAC + 0xac)
#define RG_MAC_FRM_TYPE_CNT2 (WIFI_MAC + 0xf4)
#define RG_MAC_FRM_TYPE_CNT3 (WIFI_MAC + 0xf8)
#define RG_MAC_BCN_CTRL_REG (WIFI_MAC + 0x234)
#define RG_MAC_RX_WPTR (WIFI_MAC + 0x1fc)
#define RG_MAC_RX_FRPTR (WIFI_MAC + 0x200)
#define RG_MAC_RX_HRPTR (WIFI_MAC + 0x204)



struct data_rx_local_cnt_bits{
    unsigned int data_local_cnt: 29,
            data_local_cnt_qos:1,
            data_local_cnt_clr:1,
            data_local_cnt_en:1;
};

struct data_rx_cnt_bits{
    unsigned int cnt:30,
            clr:1,
            en:1;
};


struct  tx_cnt_ctrl_bits
{
    unsigned int cnt  :26,
           en:1,
           clr:1,
           rsv:4;
};

struct  cnt_ctrl_bits
{
    unsigned int type_idx0 :6,
           type_idx1 :6,
           type_idx2 :6,
           type_idx3 :6,
           en0 :1,
           en1 :1,
           en2 :1,
           en3 :1,
           clr0 :1,
           clr1 :1,
           clr2 :1,
           clr3 :1;
};


struct ctrl_idx{
    unsigned char idx0;
    unsigned char idx1;
    unsigned char idx2;
    unsigned char idx3;
    unsigned char qos_en;
};

union sts_mac_reg_u{
    unsigned int x;
    struct ctrl_idx op_idx;
    struct  tx_cnt_ctrl_bits tx_cnt_ctrl;
    struct  cnt_ctrl_bits cnt_ctrl;
    struct  data_rx_cnt_bits  rx_other_cnt;
    struct  data_rx_cnt_bits  rx_mutil_cnt;
    struct  data_rx_local_cnt_bits rx_local_cnt;
};

struct sts_irq_ctrl{
    unsigned  int cnt:29,func_code:3 ;
};

struct sts_sw_cnt_ctrl{
    unsigned  int cnt:29,func_code:3 ;
};

#endif
