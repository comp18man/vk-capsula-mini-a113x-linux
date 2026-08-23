#ifdef WIFI_MAC_TX_REG
#else
#define WIFI_MAC_TX_REG


#define WIFI_MAC_TX_REG_BASE                      (0xa00000)

#define MAC_TXSTARTTIME_HIGH                      (WIFI_MAC_TX_REG_BASE + 0x238)
// Bit 31  :0      ro_mac_txstarttime_high        U     RO        default = 'h0
typedef union MAC_TXSTARTTIME_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_txstarttime_high : 32;
  } b;
} MAC_TXSTARTTIME_HIGH_FIELD_T;

#define MAC_TXSTARTTIME_LOW                       (WIFI_MAC_TX_REG_BASE + 0x23c)
// Bit 31  :0      ro_mac_txstarttime_low         U     RO        default = 'h0
typedef union MAC_TXSTARTTIME_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_txstarttime_low : 32;
  } b;
} MAC_TXSTARTTIME_LOW_FIELD_T;

#define MAC_TXENDTIME                             (WIFI_MAC_TX_REG_BASE + 0x240)
// Bit 31  :0      ro_mac_txendtime               U     RO        default = 'h0
typedef union MAC_TXENDTIME_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_txendtime : 32;
  } b;
} MAC_TXENDTIME_FIELD_T;

#define MAC_TXPOINTER                             (WIFI_MAC_TX_REG_BASE + 0x244)
// Bit 31  :0      reg_tx_table_ptr               U     RW        default = 'h0
typedef union MAC_TXPOINTER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_tx_table_ptr : 32;
  } b;
} MAC_TXPOINTER_FIELD_T;

#define MAC_TXCONTROL_FIAC                        (WIFI_MAC_TX_REG_BASE + 0x248)
// Bit 15  :10     reg_tx_duration_fiac           U     RW        default = 'h1
// Bit 24          reg_dcpready_fiac              U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_fiac           U     RW        default = 'h0
typedef union MAC_TXCONTROL_FIAC_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 10;
    unsigned int reg_tx_duration_fiac : 6;
    unsigned int rsvd_1 : 8;
    unsigned int reg_dcpready_fiac : 1;
    unsigned int rsvd_2 : 3;
    unsigned int reg_tx_priority_fiac : 4;
  } b;
} MAC_TXCONTROL_FIAC_FIELD_T;

#define MAC_TXWD_TIMER                            (WIFI_MAC_TX_REG_BASE + 0x24c)
// Bit 23  :0      reg_tx_wd_timer_duration       U     RW        default = 'h4e20
// Bit 31          reg_tx_wd_timer_en             U     RW        default = 'h1
typedef union MAC_TXWD_TIMER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_tx_wd_timer_duration : 24;
    unsigned int rsvd_0 : 7;
    unsigned int reg_tx_wd_timer_en : 1;
  } b;
} MAC_TXWD_TIMER_FIELD_T;

#define MAC_TXINFO                                (WIFI_MAC_TX_REG_BASE + 0x250)
// Bit 0           reg_txend_queue_bitinfo0       U     W1C       default = 'h0
// Bit 1           reg_txend_queue_bitinfo1       U     W1C       default = 'h0
// Bit 2           reg_txend_queue_bitinfo2       U     W1C       default = 'h0
// Bit 3           reg_txend_queue_bitinfo3       U     W1C       default = 'h0
// Bit 4           reg_txend_queue_bitinfo4       U     W1C       default = 'h0
// Bit 5           reg_txend_queue_bitinfo5       U     W1C       default = 'h0
// Bit 6           reg_txend_queue_bitinfo6       U     W1C       default = 'h0
// Bit 7           reg_txend_queue_bitinfo7       U     W1C       default = 'h0
// Bit 8           reg_txend_queue_bitinfo8       U     W1C       default = 'h0
// Bit 9           reg_txend_queue_bitinfo9       U     W1C       default = 'h0
// Bit 25  :16     ro_txend_queue_errinfo         U     RO        default = 'h0
typedef union MAC_TXINFO_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txend_queue_bitinfo0 : 1;
    unsigned int reg_txend_queue_bitinfo1 : 1;
    unsigned int reg_txend_queue_bitinfo2 : 1;
    unsigned int reg_txend_queue_bitinfo3 : 1;
    unsigned int reg_txend_queue_bitinfo4 : 1;
    unsigned int reg_txend_queue_bitinfo5 : 1;
    unsigned int reg_txend_queue_bitinfo6 : 1;
    unsigned int reg_txend_queue_bitinfo7 : 1;
    unsigned int reg_txend_queue_bitinfo8 : 1;
    unsigned int reg_txend_queue_bitinfo9 : 1;
    unsigned int rsvd_0 : 6;
    unsigned int ro_txend_queue_errinfo : 10;
    unsigned int rsvd_1 : 6;
  } b;
} MAC_TXINFO_FIELD_T;

#define TX_CTRL_FRAME_SEND                        (WIFI_MAC_TX_REG_BASE + 0x258)
// Bit 7   :0      reg_tx_other_info_len          U     RW        default = 'h34
// Bit 10  :8      reg_dynamic_mcs                U     RW        default = 'h1
// Bit 12          reg_dynamic_mcs_en             U     RW        default = 'h1
// Bit 25  :16     reg_txout_time_thr             U     RW        default = 'h320
// Bit 28          reg_txout_time_thr_en          U     RW        default = 'h1
typedef union TX_CTRL_FRAME_SEND_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_tx_other_info_len : 8;
    unsigned int reg_dynamic_mcs : 3;
    unsigned int rsvd_0 : 1;
    unsigned int reg_dynamic_mcs_en : 1;
    unsigned int rsvd_1 : 3;
    unsigned int reg_txout_time_thr : 10;
    unsigned int rsvd_2 : 2;
    unsigned int reg_txout_time_thr_en : 1;
    unsigned int rsvd_3 : 3;
  } b;
} TX_CTRL_FRAME_SEND_FIELD_T;

#define TX_IAC_SEND_CTRL                          (WIFI_MAC_TX_REG_BASE + 0x25c)
// Bit 0           reg_iac_frame_send             U     RW        default = 'h0
// Bit 6   :4      reg_iac_frame_type             U     RW        default = 'h0
// Bit 10  :8      reg_iac_tid_num                U     RW        default = 'h0
// Bit 11          reg_iac_power_mode             U     RW        default = 'h0
typedef union TX_IAC_SEND_CTRL_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_iac_frame_send : 1;
    unsigned int rsvd_0 : 3;
    unsigned int reg_iac_frame_type : 3;
    unsigned int rsvd_1 : 1;
    unsigned int reg_iac_tid_num : 3;
    unsigned int reg_iac_power_mode : 1;
    unsigned int rsvd_2 : 20;
  } b;
} TX_IAC_SEND_CTRL_FIELD_T;

#define TX_IAC_BAR_INFO                           (WIFI_MAC_TX_REG_BASE + 0x260)
// Bit 3   :0      reg_iac_tid_0                  U     RW        default = 'h0
// Bit 15  :4      reg_iac_bitmap_index_0         U     RW        default = 'h0
// Bit 19  :16     reg_iac_tid_1                  U     RW        default = 'h0
// Bit 31  :20     reg_iac_bitmap_index_1         U     RW        default = 'h0
typedef union TX_IAC_BAR_INFO_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_iac_tid_0 : 4;
    unsigned int reg_iac_bitmap_index_0 : 12;
    unsigned int reg_iac_tid_1 : 4;
    unsigned int reg_iac_bitmap_index_1 : 12;
  } b;
} TX_IAC_BAR_INFO_FIELD_T;

#define TXIAC_ADDR1_LOW                           (WIFI_MAC_TX_REG_BASE + 0x264)
// Bit 31  :0      reg_txiac_addr1_low            U     RW        default = 'h0
typedef union TXIAC_ADDR1_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txiac_addr1_low : 32;
  } b;
} TXIAC_ADDR1_LOW_FIELD_T;

#define TXIAC_ADDR1_HIGH                          (WIFI_MAC_TX_REG_BASE + 0x268)
// Bit 15  :0      reg_txiac_addr1_high           U     RW        default = 'h0
typedef union TXIAC_ADDR1_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txiac_addr1_high : 16;
    unsigned int rsvd_0 : 16;
  } b;
} TXIAC_ADDR1_HIGH_FIELD_T;

#define TXIAC_ADDR2_LOW                           (WIFI_MAC_TX_REG_BASE + 0x26c)
// Bit 31  :0      reg_txiac_addr2_low            U     RW        default = 'h0
typedef union TXIAC_ADDR2_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txiac_addr2_low : 32;
  } b;
} TXIAC_ADDR2_LOW_FIELD_T;

#define TXIAC_ADDR2_HIGH                          (WIFI_MAC_TX_REG_BASE + 0x270)
// Bit 15  :0      reg_txiac_addr2_high           U     RW        default = 'h0
typedef union TXIAC_ADDR2_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txiac_addr2_high : 16;
    unsigned int rsvd_0 : 16;
  } b;
} TXIAC_ADDR2_HIGH_FIELD_T;

#define TX_IAC_VECTOR                             (WIFI_MAC_TX_REG_BASE + 0x274)
// Bit 31  :0      reg_txiac_vector               U     RW        default = 'h0
typedef union TX_IAC_VECTOR_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txiac_vector : 32;
  } b;
} TX_IAC_VECTOR_FIELD_T;

#define MAC_TXVECTOR_CONF                         (WIFI_MAC_TX_REG_BASE + 0x278)
// Bit 15  :0      reg_mac_tx_service             U     RW        default = 'h0
// Bit 18  :16     reg_mac_tx_htdltf_num          U     RW        default = 'h1
// Bit 22  :20     reg_mac_tx_hteltf_num          U     RW        default = 'h0
// Bit 26  :24     reg_mac_tx_vhtltf_num          U     RW        default = 'h0
typedef union MAC_TXVECTOR_CONF_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_tx_service : 16;
    unsigned int reg_mac_tx_htdltf_num : 3;
    unsigned int rsvd_0 : 1;
    unsigned int reg_mac_tx_hteltf_num : 3;
    unsigned int rsvd_1 : 1;
    unsigned int reg_mac_tx_vhtltf_num : 3;
    unsigned int rsvd_2 : 5;
  } b;
} MAC_TXVECTOR_CONF_FIELD_T;

#define TXOP_REMAIN                               (WIFI_MAC_TX_REG_BASE + 0x27c)
// Bit 14  :0      reg_txop_remain                U     RW        default = 'h0
// Bit 25  :16     reg_rts_time                   U     RW        default = 'h0
// Bit 30  :28     reg_dyn_bwd_ctrl               U     RW        default = 'h0
typedef union TXOP_REMAIN_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txop_remain : 15;
    unsigned int rsvd_0 : 1;
    unsigned int reg_rts_time : 10;
    unsigned int rsvd_1 : 2;
    unsigned int reg_dyn_bwd_ctrl : 3;
    unsigned int rsvd_2 : 1;
  } b;
} TXOP_REMAIN_FIELD_T;

#define FW_PKT_DURA_CALC_IN                       (WIFI_MAC_TX_REG_BASE + 0x280)
// Bit 30  :0      reg_fw_pkt_dura_calc_in        U     RW        default = 'h0
// Bit 31          reg_fw_pkt_dura_calc_req       U     RW        default = 'h0
typedef union FW_PKT_DURA_CALC_IN_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_fw_pkt_dura_calc_in : 31;
    unsigned int reg_fw_pkt_dura_calc_req : 1;
  } b;
} FW_PKT_DURA_CALC_IN_FIELD_T;

#define FW_PKT_DURA_CALC_OUT                      (WIFI_MAC_TX_REG_BASE + 0x284)
// Bit 15  :0      ro_fw_pkt_dura_calc_out        U     RO        default = 'h0
typedef union FW_PKT_DURA_CALC_OUT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_fw_pkt_dura_calc_out : 16;
    unsigned int rsvd_0 : 16;
  } b;
} FW_PKT_DURA_CALC_OUT_FIELD_T;

#define TXOUT_ENDCNT_MAX                          (WIFI_MAC_TX_REG_BASE + 0x288)
// Bit 3   :0      reg_txout_endcnt_max           U     RW        default = 'h4
// Bit 15  :8      reg_tx_rts_rate                U     RW        default = 'h4
// Bit 23  :16     reg_tx_time_gap                U     RW        default = 'h10
typedef union TXOUT_ENDCNT_MAX_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txout_endcnt_max : 4;
    unsigned int rsvd_0 : 4;
    unsigned int reg_tx_rts_rate : 8;
    unsigned int reg_tx_time_gap : 8;
    unsigned int rsvd_1 : 8;
  } b;
} TXOUT_ENDCNT_MAX_FIELD_T;

#define MAC_TXVECTOR_P1_NONQOS                    (WIFI_MAC_TX_REG_BASE + 0x28c)
// Bit 29  :0      reg_mac_txvector_p1_nonqos     U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_nonqos_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_nonqos_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_NONQOS_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_nonqos : 30;
    unsigned int reg_mac_txvector_p1_nonqos_rts : 1;
    unsigned int reg_mac_txvector_p1_nonqos_cts : 1;
  } b;
} MAC_TXVECTOR_P1_NONQOS_FIELD_T;

#define MAC_TXVECTOR_P1_ACP0                      (WIFI_MAC_TX_REG_BASE + 0x290)
// Bit 29  :0      reg_mac_txvector_p1_acp0       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp0_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp0_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp0 : 30;
    unsigned int reg_mac_txvector_p1_acp0_rts : 1;
    unsigned int reg_mac_txvector_p1_acp0_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP0_FIELD_T;

#define MAC_TXVECTOR_P1_ACP1                      (WIFI_MAC_TX_REG_BASE + 0x294)
// Bit 29  :0      reg_mac_txvector_p1_acp1       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp1_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp1_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp1 : 30;
    unsigned int reg_mac_txvector_p1_acp1_rts : 1;
    unsigned int reg_mac_txvector_p1_acp1_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP1_FIELD_T;

#define MAC_TXVECTOR_P1_ACP2                      (WIFI_MAC_TX_REG_BASE + 0x298)
// Bit 29  :0      reg_mac_txvector_p1_acp2       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp2_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp2_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp2 : 30;
    unsigned int reg_mac_txvector_p1_acp2_rts : 1;
    unsigned int reg_mac_txvector_p1_acp2_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP2_FIELD_T;

#define MAC_TXVECTOR_P1_ACP3                      (WIFI_MAC_TX_REG_BASE + 0x29c)
// Bit 29  :0      reg_mac_txvector_p1_acp3       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp3_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp3_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp3 : 30;
    unsigned int reg_mac_txvector_p1_acp3_rts : 1;
    unsigned int reg_mac_txvector_p1_acp3_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP3_FIELD_T;

#define MAC_TXVECTOR_P1_ACP4                      (WIFI_MAC_TX_REG_BASE + 0x2a0)
// Bit 29  :0      reg_mac_txvector_p1_acp4       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp4_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp4_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp4 : 30;
    unsigned int reg_mac_txvector_p1_acp4_rts : 1;
    unsigned int reg_mac_txvector_p1_acp4_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP4_FIELD_T;

#define MAC_TXVECTOR_P1_ACP5                      (WIFI_MAC_TX_REG_BASE + 0x2a4)
// Bit 29  :0      reg_mac_txvector_p1_acp5       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp5_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp5_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp5 : 30;
    unsigned int reg_mac_txvector_p1_acp5_rts : 1;
    unsigned int reg_mac_txvector_p1_acp5_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP5_FIELD_T;

#define MAC_TXVECTOR_P1_ACP6                      (WIFI_MAC_TX_REG_BASE + 0x2a8)
// Bit 29  :0      reg_mac_txvector_p1_acp6       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp6_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp6_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp6 : 30;
    unsigned int reg_mac_txvector_p1_acp6_rts : 1;
    unsigned int reg_mac_txvector_p1_acp6_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP6_FIELD_T;

#define MAC_TXVECTOR_P1_ACP7                      (WIFI_MAC_TX_REG_BASE + 0x2ac)
// Bit 29  :0      reg_mac_txvector_p1_acp7       U     RW        default = 'h0
// Bit 30          reg_mac_txvector_p1_acp7_rts     U     RW        default = 'h0
// Bit 31          reg_mac_txvector_p1_acp7_cts     U     RW        default = 'h0
typedef union MAC_TXVECTOR_P1_ACP7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_txvector_p1_acp7 : 30;
    unsigned int reg_mac_txvector_p1_acp7_rts : 1;
    unsigned int reg_mac_txvector_p1_acp7_cts : 1;
  } b;
} MAC_TXVECTOR_P1_ACP7_FIELD_T;

#define MAC_RXPKT_CONTROL0                        (WIFI_MAC_TX_REG_BASE + 0x2b4)
// Bit 31  :0      reg_mac_rxpkt_control0         U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control0 : 32;
  } b;
} MAC_RXPKT_CONTROL0_FIELD_T;

#define MAC_RXPKT_CONTROL2                        (WIFI_MAC_TX_REG_BASE + 0x2b8)
// Bit 31  :0      reg_mac_rxpkt_control2         U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control2 : 32;
  } b;
} MAC_RXPKT_CONTROL2_FIELD_T;

#define MAC_RXPKT_CONTROL3                        (WIFI_MAC_TX_REG_BASE + 0x2bc)
// Bit 31  :0      reg_mac_rxpkt_control3         U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control3 : 32;
  } b;
} MAC_RXPKT_CONTROL3_FIELD_T;

#define MAC_RXPKT_CONTROL4                        (WIFI_MAC_TX_REG_BASE + 0x2c0)
// Bit 31  :0      reg_mac_rxpkt_control4         U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control4 : 32;
  } b;
} MAC_RXPKT_CONTROL4_FIELD_T;

#define MAC_RXPKT_CONTROL6                        (WIFI_MAC_TX_REG_BASE + 0x2c4)
// Bit 31  :0      reg_mac_rxpkt_control6         U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control6 : 32;
  } b;
} MAC_RXPKT_CONTROL6_FIELD_T;

#define MAC_RXPKT_CONTROL7                        (WIFI_MAC_TX_REG_BASE + 0x2c8)
// Bit 31  :0      reg_mac_rxpkt_control7         U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control7 : 32;
  } b;
} MAC_RXPKT_CONTROL7_FIELD_T;

#define MAC_RXPKT_CONTROL8                        (WIFI_MAC_TX_REG_BASE + 0x2cc)
// Bit 31  :0      reg_mac_rxpkt_control8         U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control8 : 32;
  } b;
} MAC_RXPKT_CONTROL8_FIELD_T;

#define MAC_RXPKT_CONTROL10                       (WIFI_MAC_TX_REG_BASE + 0x2d0)
// Bit 31  :0      reg_mac_rxpkt_control10        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control10 : 32;
  } b;
} MAC_RXPKT_CONTROL10_FIELD_T;

#define MAC_RXPKT_CONTROL11                       (WIFI_MAC_TX_REG_BASE + 0x2d4)
// Bit 31  :0      reg_mac_rxpkt_control11        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control11 : 32;
  } b;
} MAC_RXPKT_CONTROL11_FIELD_T;

#define MAC_RXPKT_CONTROL12                       (WIFI_MAC_TX_REG_BASE + 0x2d8)
// Bit 31  :0      reg_mac_rxpkt_control12        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control12 : 32;
  } b;
} MAC_RXPKT_CONTROL12_FIELD_T;

#define MAC_RXPKT_CONTROL14                       (WIFI_MAC_TX_REG_BASE + 0x2dc)
// Bit 31  :0      reg_mac_rxpkt_control14        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control14 : 32;
  } b;
} MAC_RXPKT_CONTROL14_FIELD_T;

#define MAC_RXPKT_CONTROL16                       (WIFI_MAC_TX_REG_BASE + 0x2e0)
// Bit 31  :0      reg_mac_rxpkt_control16        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control16 : 32;
  } b;
} MAC_RXPKT_CONTROL16_FIELD_T;

#define MAC_RXPKT_CONTROL18                       (WIFI_MAC_TX_REG_BASE + 0x2e4)
// Bit 31  :0      reg_mac_rxpkt_control18        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control18 : 32;
  } b;
} MAC_RXPKT_CONTROL18_FIELD_T;

#define MAC_RXPKT_CONTROL20                       (WIFI_MAC_TX_REG_BASE + 0x2e8)
// Bit 31  :0      reg_mac_rxpkt_control20        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control20 : 32;
  } b;
} MAC_RXPKT_CONTROL20_FIELD_T;

#define MAC_RXPKT_CONTROL22                       (WIFI_MAC_TX_REG_BASE + 0x2ec)
// Bit 31  :0      reg_mac_rxpkt_control22        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control22 : 32;
  } b;
} MAC_RXPKT_CONTROL22_FIELD_T;

#define MAC_RXPKT_CONTROL26                       (WIFI_MAC_TX_REG_BASE + 0x2f0)
// Bit 31  :0      reg_mac_rxpkt_control26        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control26 : 32;
  } b;
} MAC_RXPKT_CONTROL26_FIELD_T;

#define MAC_RXPKT_CONTROL29                       (WIFI_MAC_TX_REG_BASE + 0x2f4)
// Bit 31  :0      reg_mac_rxpkt_control29        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL29_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control29 : 32;
  } b;
} MAC_RXPKT_CONTROL29_FIELD_T;

#define MAC_RXPKT_CONTROL30                       (WIFI_MAC_TX_REG_BASE + 0x2f8)
// Bit 31  :0      reg_mac_rxpkt_control30        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL30_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control30 : 32;
  } b;
} MAC_RXPKT_CONTROL30_FIELD_T;

#define MAC_RXPKT_CONTROL32                       (WIFI_MAC_TX_REG_BASE + 0x2fc)
// Bit 31  :0      reg_mac_rxpkt_control32        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL32_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control32 : 32;
  } b;
} MAC_RXPKT_CONTROL32_FIELD_T;

#define MAC_RXPKT_CONTROL33                       (WIFI_MAC_TX_REG_BASE + 0x300)
// Bit 31  :0      reg_mac_rxpkt_control33        U     RW        default = 'hc0000000
typedef union MAC_RXPKT_CONTROL33_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control33 : 32;
  } b;
} MAC_RXPKT_CONTROL33_FIELD_T;

#define MAC_RXPKT_CONTROL34                       (WIFI_MAC_TX_REG_BASE + 0x304)
// Bit 31  :0      reg_mac_rxpkt_control34        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL34_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control34 : 32;
  } b;
} MAC_RXPKT_CONTROL34_FIELD_T;

#define MAC_RXPKT_CONTROL36                       (WIFI_MAC_TX_REG_BASE + 0x308)
// Bit 31  :0      reg_mac_rxpkt_control36        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL36_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control36 : 32;
  } b;
} MAC_RXPKT_CONTROL36_FIELD_T;

#define MAC_RXPKT_CONTROL37                       (WIFI_MAC_TX_REG_BASE + 0x30c)
// Bit 31  :0      reg_mac_rxpkt_control37        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL37_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control37 : 32;
  } b;
} MAC_RXPKT_CONTROL37_FIELD_T;

#define MAC_RXPKT_CONTROL38                       (WIFI_MAC_TX_REG_BASE + 0x310)
// Bit 31  :0      reg_mac_rxpkt_control38        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL38_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control38 : 32;
  } b;
} MAC_RXPKT_CONTROL38_FIELD_T;

#define MAC_RXPKT_CONTROL40                       (WIFI_MAC_TX_REG_BASE + 0x314)
// Bit 31  :0      reg_mac_rxpkt_control40        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL40_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control40 : 32;
  } b;
} MAC_RXPKT_CONTROL40_FIELD_T;

#define MAC_RXPKT_CONTROL41                       (WIFI_MAC_TX_REG_BASE + 0x318)
// Bit 31  :0      reg_mac_rxpkt_control41        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL41_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control41 : 32;
  } b;
} MAC_RXPKT_CONTROL41_FIELD_T;

#define MAC_RXPKT_CONTROL42                       (WIFI_MAC_TX_REG_BASE + 0x31c)
// Bit 31  :0      reg_mac_rxpkt_control42        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL42_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control42 : 32;
  } b;
} MAC_RXPKT_CONTROL42_FIELD_T;

#define MAC_RXPKT_CONTROL44                       (WIFI_MAC_TX_REG_BASE + 0x320)
// Bit 31  :0      reg_mac_rxpkt_control44        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL44_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control44 : 32;
  } b;
} MAC_RXPKT_CONTROL44_FIELD_T;

#define MAC_RXPKT_CONTROL45                       (WIFI_MAC_TX_REG_BASE + 0x324)
// Bit 31  :0      reg_mac_rxpkt_control45        U     RW        default = 'hc0000000
typedef union MAC_RXPKT_CONTROL45_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control45 : 32;
  } b;
} MAC_RXPKT_CONTROL45_FIELD_T;

#define MAC_RXPKT_CONTROL46                       (WIFI_MAC_TX_REG_BASE + 0x328)
// Bit 31  :0      reg_mac_rxpkt_control46        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL46_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control46 : 32;
  } b;
} MAC_RXPKT_CONTROL46_FIELD_T;

#define MAC_RXPKT_CONTROL48                       (WIFI_MAC_TX_REG_BASE + 0x32c)
// Bit 31  :0      reg_mac_rxpkt_control48        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL48_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control48 : 32;
  } b;
} MAC_RXPKT_CONTROL48_FIELD_T;

#define MAC_RXPKT_CONTROL49                       (WIFI_MAC_TX_REG_BASE + 0x330)
// Bit 31  :0      reg_mac_rxpkt_control49        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL49_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control49 : 32;
  } b;
} MAC_RXPKT_CONTROL49_FIELD_T;

#define MAC_RXPKT_CONTROL50                       (WIFI_MAC_TX_REG_BASE + 0x334)
// Bit 31  :0      reg_mac_rxpkt_control50        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL50_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control50 : 32;
  } b;
} MAC_RXPKT_CONTROL50_FIELD_T;

#define MAC_RXPKT_CONTROL52                       (WIFI_MAC_TX_REG_BASE + 0x338)
// Bit 31  :0      reg_mac_rxpkt_control52        U     RW        default = 'he0000000
typedef union MAC_RXPKT_CONTROL52_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control52 : 32;
  } b;
} MAC_RXPKT_CONTROL52_FIELD_T;

#define MAC_RXPKT_CONTROL53                       (WIFI_MAC_TX_REG_BASE + 0x33c)
// Bit 31  :0      reg_mac_rxpkt_control53        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL53_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control53 : 32;
  } b;
} MAC_RXPKT_CONTROL53_FIELD_T;

#define MAC_RXPKT_CONTROL56                       (WIFI_MAC_TX_REG_BASE + 0x340)
// Bit 31  :0      reg_mac_rxpkt_control56        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL56_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control56 : 32;
  } b;
} MAC_RXPKT_CONTROL56_FIELD_T;

#define MAC_RXPKT_CONTROL57                       (WIFI_MAC_TX_REG_BASE + 0x344)
// Bit 31  :0      reg_mac_rxpkt_control57        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL57_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control57 : 32;
  } b;
} MAC_RXPKT_CONTROL57_FIELD_T;

#define MAC_RXPKT_CONTROL58                       (WIFI_MAC_TX_REG_BASE + 0x348)
// Bit 31  :0      reg_mac_rxpkt_control58        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL58_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control58 : 32;
  } b;
} MAC_RXPKT_CONTROL58_FIELD_T;

#define MAC_RXPKT_CONTROL61                       (WIFI_MAC_TX_REG_BASE + 0x34c)
// Bit 31  :0      reg_mac_rxpkt_control61        U     RW        default = 'h0
typedef union MAC_RXPKT_CONTROL61_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control61 : 32;
  } b;
} MAC_RXPKT_CONTROL61_FIELD_T;

#define MAC_RXPKT_CONTROL62                       (WIFI_MAC_TX_REG_BASE + 0x350)
// Bit 31  :0      reg_mac_rxpkt_control62        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL62_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control62 : 32;
  } b;
} MAC_RXPKT_CONTROL62_FIELD_T;

#define MAC_RXPKT_CONTROL24                       (WIFI_MAC_TX_REG_BASE + 0x354)
// Bit 31  :0      reg_mac_rxpkt_control24        U     RW        default = 'h20000000
typedef union MAC_RXPKT_CONTROL24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control24 : 32;
  } b;
} MAC_RXPKT_CONTROL24_FIELD_T;

#define MAC_11B_LUT_1M                            (WIFI_MAC_TX_REG_BASE + 0x358)
// Bit 31  :0      reg_mac_11b_lut_1m             U     RW        default = 'h0
typedef union MAC_11B_LUT_1M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11b_lut_1m : 32;
  } b;
} MAC_11B_LUT_1M_FIELD_T;

#define MAC_11B_LUT_2M                            (WIFI_MAC_TX_REG_BASE + 0x35c)
// Bit 31  :0      reg_mac_11b_lut_2m             U     RW        default = 'h11010000
typedef union MAC_11B_LUT_2M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11b_lut_2m : 32;
  } b;
} MAC_11B_LUT_2M_FIELD_T;

#define MAC_11B_LUT_5M                            (WIFI_MAC_TX_REG_BASE + 0x360)
// Bit 31  :0      reg_mac_11b_lut_5m             U     RW        default = 'h22020202
typedef union MAC_11B_LUT_5M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11b_lut_5m : 32;
  } b;
} MAC_11B_LUT_5M_FIELD_T;

#define MAC_11B_LUT_11M                           (WIFI_MAC_TX_REG_BASE + 0x364)
// Bit 31  :0      reg_mac_11b_lut_11m            U     RW        default = 'h33030303
typedef union MAC_11B_LUT_11M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11b_lut_11m : 32;
  } b;
} MAC_11B_LUT_11M_FIELD_T;

#define MAC_11G_LUT_6M                            (WIFI_MAC_TX_REG_BASE + 0x368)
// Bit 31  :0      reg_mac_11g_lut_6m             U     RW        default = 'h44040404
typedef union MAC_11G_LUT_6M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_6m : 32;
  } b;
} MAC_11G_LUT_6M_FIELD_T;

#define MAC_11G_LUT_9M                            (WIFI_MAC_TX_REG_BASE + 0x36c)
// Bit 31  :0      reg_mac_11g_lut_9m             U     RW        default = 'h44050505
typedef union MAC_11G_LUT_9M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_9m : 32;
  } b;
} MAC_11G_LUT_9M_FIELD_T;

#define MAC_11G_LUT_12M                           (WIFI_MAC_TX_REG_BASE + 0x370)
// Bit 31  :0      reg_mac_11g_lut_12m            U     RW        default = 'h55060606
typedef union MAC_11G_LUT_12M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_12m : 32;
  } b;
} MAC_11G_LUT_12M_FIELD_T;

#define MAC_11G_LUT_18M                           (WIFI_MAC_TX_REG_BASE + 0x374)
// Bit 31  :0      reg_mac_11g_lut_18m            U     RW        default = 'h55060606
typedef union MAC_11G_LUT_18M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_18m : 32;
  } b;
} MAC_11G_LUT_18M_FIELD_T;

#define MAC_11G_LUT_24M                           (WIFI_MAC_TX_REG_BASE + 0x378)
// Bit 31  :0      reg_mac_11g_lut_24m            U     RW        default = 'h56060606
typedef union MAC_11G_LUT_24M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_24m : 32;
  } b;
} MAC_11G_LUT_24M_FIELD_T;

#define MAC_11G_LUT_36M                           (WIFI_MAC_TX_REG_BASE + 0x37c)
// Bit 31  :0      reg_mac_11g_lut_36m            U     RW        default = 'h66080608
typedef union MAC_11G_LUT_36M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_36m : 32;
  } b;
} MAC_11G_LUT_36M_FIELD_T;

#define MAC_11G_LUT_48M                           (WIFI_MAC_TX_REG_BASE + 0x380)
// Bit 31  :0      reg_mac_11g_lut_48m            U     RW        default = 'h67080608
typedef union MAC_11G_LUT_48M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_48m : 32;
  } b;
} MAC_11G_LUT_48M_FIELD_T;

#define MAC_11G_LUT_54M                           (WIFI_MAC_TX_REG_BASE + 0x384)
// Bit 31  :0      reg_mac_11g_lut_54m            U     RW        default = 'h67080608
typedef union MAC_11G_LUT_54M_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11g_lut_54m : 32;
  } b;
} MAC_11G_LUT_54M_FIELD_T;

#define MAC_11N_LUT_MCS0                          (WIFI_MAC_TX_REG_BASE + 0x388)
// Bit 31  :0      reg_mac_11n_lut_mcs0           U     RW        default = 'h88800404
typedef union MAC_11N_LUT_MCS0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs0 : 32;
  } b;
} MAC_11N_LUT_MCS0_FIELD_T;

#define MAC_11N_LUT_MCS1                          (WIFI_MAC_TX_REG_BASE + 0x38c)
// Bit 31  :0      reg_mac_11n_lut_mcs1           U     RW        default = 'h99810606
typedef union MAC_11N_LUT_MCS1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs1 : 32;
  } b;
} MAC_11N_LUT_MCS1_FIELD_T;

#define MAC_11N_LUT_MCS2                          (WIFI_MAC_TX_REG_BASE + 0x390)
// Bit 31  :0      reg_mac_11n_lut_mcs2           U     RW        default = 'h9a810606
typedef union MAC_11N_LUT_MCS2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs2 : 32;
  } b;
} MAC_11N_LUT_MCS2_FIELD_T;

#define MAC_11N_LUT_MCS3                          (WIFI_MAC_TX_REG_BASE + 0x394)
// Bit 31  :0      reg_mac_11n_lut_mcs3           U     RW        default = 'hbb830608
typedef union MAC_11N_LUT_MCS3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs3 : 32;
  } b;
} MAC_11N_LUT_MCS3_FIELD_T;

#define MAC_11N_LUT_MCS4                          (WIFI_MAC_TX_REG_BASE + 0x398)
// Bit 31  :0      reg_mac_11n_lut_mcs4           U     RW        default = 'hbc830608
typedef union MAC_11N_LUT_MCS4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs4 : 32;
  } b;
} MAC_11N_LUT_MCS4_FIELD_T;

#define MAC_11N_LUT_MCS5                          (WIFI_MAC_TX_REG_BASE + 0x39c)
// Bit 31  :0      reg_mac_11n_lut_mcs5           U     RW        default = 'hbd830608
typedef union MAC_11N_LUT_MCS5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs5 : 32;
  } b;
} MAC_11N_LUT_MCS5_FIELD_T;

#define MAC_11N_LUT_MCS6                          (WIFI_MAC_TX_REG_BASE + 0x3a0)
// Bit 31  :0      reg_mac_11n_lut_mcs6           U     RW        default = 'hbe830608
typedef union MAC_11N_LUT_MCS6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs6 : 32;
  } b;
} MAC_11N_LUT_MCS6_FIELD_T;

#define MAC_11N_LUT_MCS7                          (WIFI_MAC_TX_REG_BASE + 0x3a4)
// Bit 31  :0      reg_mac_11n_lut_mcs7           U     RW        default = 'hbe830606
typedef union MAC_11N_LUT_MCS7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11n_lut_mcs7 : 32;
  } b;
} MAC_11N_LUT_MCS7_FIELD_T;

#define MAC_11AC_LUT_MCS0                         (WIFI_MAC_TX_REG_BASE + 0x3a8)
// Bit 31  :0      reg_mac_11ac_lut_mcs0          U     RW        default = 'h48040404
typedef union MAC_11AC_LUT_MCS0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs0 : 32;
  } b;
} MAC_11AC_LUT_MCS0_FIELD_T;

#define MAC_11AC_LUT_MCS1                         (WIFI_MAC_TX_REG_BASE + 0x3ac)
// Bit 31  :0      reg_mac_11ac_lut_mcs1          U     RW        default = 'h59060606
typedef union MAC_11AC_LUT_MCS1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs1 : 32;
  } b;
} MAC_11AC_LUT_MCS1_FIELD_T;

#define MAC_11AC_LUT_MCS2                         (WIFI_MAC_TX_REG_BASE + 0x3b0)
// Bit 31  :0      reg_mac_11ac_lut_mcs2          U     RW        default = 'h5a060606
typedef union MAC_11AC_LUT_MCS2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs2 : 32;
  } b;
} MAC_11AC_LUT_MCS2_FIELD_T;

#define MAC_11AC_LUT_MCS3                         (WIFI_MAC_TX_REG_BASE + 0x3b4)
// Bit 31  :0      reg_mac_11ac_lut_mcs3          U     RW        default = 'h6b080608
typedef union MAC_11AC_LUT_MCS3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs3 : 32;
  } b;
} MAC_11AC_LUT_MCS3_FIELD_T;

#define MAC_11AC_LUT_MCS4                         (WIFI_MAC_TX_REG_BASE + 0x3b8)
// Bit 31  :0      reg_mac_11ac_lut_mcs4          U     RW        default = 'h6c080608
typedef union MAC_11AC_LUT_MCS4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs4 : 32;
  } b;
} MAC_11AC_LUT_MCS4_FIELD_T;

#define MAC_11AC_LUT_MCS5                         (WIFI_MAC_TX_REG_BASE + 0x3bc)
// Bit 31  :0      reg_mac_11ac_lut_mcs5          U     RW        default = 'h6d080608
typedef union MAC_11AC_LUT_MCS5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs5 : 32;
  } b;
} MAC_11AC_LUT_MCS5_FIELD_T;

#define MAC_11AC_LUT_MCS6                         (WIFI_MAC_TX_REG_BASE + 0x3c0)
// Bit 31  :0      reg_mac_11ac_lut_mcs6          U     RW        default = 'h6e080608
typedef union MAC_11AC_LUT_MCS6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs6 : 32;
  } b;
} MAC_11AC_LUT_MCS6_FIELD_T;

#define MAC_11AC_LUT_MCS7                         (WIFI_MAC_TX_REG_BASE + 0x3c4)
// Bit 31  :0      reg_mac_11ac_lut_mcs7          U     RW        default = 'h6e080608
typedef union MAC_11AC_LUT_MCS7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs7 : 32;
  } b;
} MAC_11AC_LUT_MCS7_FIELD_T;

#define MAC_11AC_LUT_MCS8                         (WIFI_MAC_TX_REG_BASE + 0x3c8)
// Bit 31  :0      reg_mac_11ac_lut_mcs8          U     RW        default = 'h6f080608
typedef union MAC_11AC_LUT_MCS8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs8 : 32;
  } b;
} MAC_11AC_LUT_MCS8_FIELD_T;

#define MAC_11AC_LUT_MCS9                         (WIFI_MAC_TX_REG_BASE + 0x3cc)
// Bit 31  :0      reg_mac_11ac_lut_mcs9          U     RW        default = 'h6f0b080b
typedef union MAC_11AC_LUT_MCS9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_11ac_lut_mcs9 : 32;
  } b;
} MAC_11AC_LUT_MCS9_FIELD_T;

#define VID0_TIMESTAMP_HIGH                       (WIFI_MAC_TX_REG_BASE + 0x3d0)
// Bit 31  :0      reg_vid0_timestamp_high        U     RW        default = 'h0
typedef union VID0_TIMESTAMP_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid0_timestamp_high : 32;
  } b;
} VID0_TIMESTAMP_HIGH_FIELD_T;

#define VID0_TIMESTAMP_LOW                        (WIFI_MAC_TX_REG_BASE + 0x3d4)
// Bit 31  :0      reg_vid0_timestamp_low         U     RW        default = 'h0
typedef union VID0_TIMESTAMP_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid0_timestamp_low : 32;
  } b;
} VID0_TIMESTAMP_LOW_FIELD_T;

#define VID1_TIMESTAMP_HIGH                       (WIFI_MAC_TX_REG_BASE + 0x3d8)
// Bit 31  :0      reg_vid1_timestamp_high        U     RW        default = 'h0
typedef union VID1_TIMESTAMP_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid1_timestamp_high : 32;
  } b;
} VID1_TIMESTAMP_HIGH_FIELD_T;

#define VID1_TIMESTAMP_LOW                        (WIFI_MAC_TX_REG_BASE + 0x3dc)
// Bit 31  :0      reg_vid1_timestamp_low         U     RW        default = 'h0
typedef union VID1_TIMESTAMP_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid1_timestamp_low : 32;
  } b;
} VID1_TIMESTAMP_LOW_FIELD_T;

#define VID2_TIMESTAMP_HIGH                       (WIFI_MAC_TX_REG_BASE + 0x3e0)
// Bit 31  :0      reg_vid2_timestamp_high        U     RW        default = 'h0
typedef union VID2_TIMESTAMP_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid2_timestamp_high : 32;
  } b;
} VID2_TIMESTAMP_HIGH_FIELD_T;

#define VID2_TIMESTAMP_LOW                        (WIFI_MAC_TX_REG_BASE + 0x3e4)
// Bit 31  :0      reg_vid2_timestamp_low         U     RW        default = 'h0
typedef union VID2_TIMESTAMP_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid2_timestamp_low : 32;
  } b;
} VID2_TIMESTAMP_LOW_FIELD_T;

#define VID3_TIMESTAMP_HIGH                       (WIFI_MAC_TX_REG_BASE + 0x3e8)
// Bit 31  :0      reg_vid3_timestamp_high        U     RW        default = 'h0
typedef union VID3_TIMESTAMP_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid3_timestamp_high : 32;
  } b;
} VID3_TIMESTAMP_HIGH_FIELD_T;

#define VID3_TIMESTAMP_LOW                        (WIFI_MAC_TX_REG_BASE + 0x3ec)
// Bit 31  :0      reg_vid3_timestamp_low         U     RW        default = 'h0
typedef union VID3_TIMESTAMP_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vid3_timestamp_low : 32;
  } b;
} VID3_TIMESTAMP_LOW_FIELD_T;

#define MAC_RXPKT_CONTROL17                       (WIFI_MAC_TX_REG_BASE + 0x3f0)
// Bit 31  :0      reg_mac_rxpkt_control17        U     RW        default = 'hc0000000
typedef union MAC_RXPKT_CONTROL17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control17 : 32;
  } b;
} MAC_RXPKT_CONTROL17_FIELD_T;

#define MAC_RXPKT_CONTROL21                       (WIFI_MAC_TX_REG_BASE + 0x3f4)
// Bit 31  :0      reg_mac_rxpkt_control21        U     RW        default = 'hc0000000
typedef union MAC_RXPKT_CONTROL21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_rxpkt_control21 : 32;
  } b;
} MAC_RXPKT_CONTROL21_FIELD_T;

#endif

