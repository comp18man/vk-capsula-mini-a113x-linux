#ifdef WIFI_MAC_RX_REG
#else
#define WIFI_MAC_RX_REG


#define WIFI_MAC_RX_REG_BASE                      (0xa00000)

#define MAC_RXWD_TIMER                            (WIFI_MAC_RX_REG_BASE + 0x1c4)
// Bit 19  :0      reg_rx_wd_timer_duration       U     RW        default = 'h4e20
// Bit 21          reg_rx_wd_timer_en             U     RW        default = 'h1
// Bit 27  :24     reg_rxend_wait_cnum            U     RW        default = 'h6
// Bit 31  :28     reg_rxstart_wait_cnum          U     RW        default = 'h8
typedef union MAC_RXWD_TIMER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_wd_timer_duration : 20;
    unsigned int rsvd_0 : 1;
    unsigned int reg_rx_wd_timer_en : 1;
    unsigned int rsvd_1 : 2;
    unsigned int reg_rxend_wait_cnum : 4;
    unsigned int reg_rxstart_wait_cnum : 4;
  } b;
} MAC_RXWD_TIMER_FIELD_T;

#define MAC_CHANNEL_NUM                           (WIFI_MAC_RX_REG_BASE + 0x1e0)
// Bit 7   :0      reg_rx_channel_num             U     RW        default = 'h0
typedef union MAC_CHANNEL_NUM_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_channel_num : 8;
    unsigned int rsvd_0 : 24;
  } b;
} MAC_CHANNEL_NUM_FIELD_T;

#define MAC_RX_BEACON_BASEADDR                    (WIFI_MAC_RX_REG_BASE + 0x1e4)
// Bit 31  :0      reg_rx_beacon_baseaddr         U     RW        default = 'h0
typedef union MAC_RX_BEACON_BASEADDR_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_beacon_baseaddr : 32;
  } b;
} MAC_RX_BEACON_BASEADDR_FIELD_T;

#define MAC_RXENDTIME                             (WIFI_MAC_RX_REG_BASE + 0x1e8)
// Bit 31  :0      ro_mac_rxendtime               U     RO        default = 'h0
typedef union MAC_RXENDTIME_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_rxendtime : 32;
  } b;
} MAC_RXENDTIME_FIELD_T;

#define MAC_RXSTARTTIME                           (WIFI_MAC_RX_REG_BASE + 0x1ec)
// Bit 31  :0      ro_mac_rxstarttime             U     RO        default = 'h0
typedef union MAC_RXSTARTTIME_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_rxstarttime : 32;
  } b;
} MAC_RXSTARTTIME_FIELD_T;

#define MAC_RX_BASEADDR                           (WIFI_MAC_RX_REG_BASE + 0x1f0)
// Bit 31  :0      reg_rx_baseaddr                U     RW        default = 'h0
typedef union MAC_RX_BASEADDR_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_baseaddr : 32;
  } b;
} MAC_RX_BASEADDR_FIELD_T;

#define MAC_RX_EPOINTER                           (WIFI_MAC_RX_REG_BASE + 0x1f4)
// Bit 15  :0      reg_rx_eptr                    U     RW        default = 'h0
typedef union MAC_RX_EPOINTER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_eptr : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_RX_EPOINTER_FIELD_T;

#define MAC_RX_COUNT                              (WIFI_MAC_RX_REG_BASE + 0x1f8)
// Bit 15  :0      reg_rx_count_o                 U     RW        default = 'h0
typedef union MAC_RX_COUNT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_count_o : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_RX_COUNT_FIELD_T;

#define MAC_RX_WPOINTER                           (WIFI_MAC_RX_REG_BASE + 0x1fc)
// Bit 15  :0      reg_rx_wptr_o                  U     RW        default = 'h0
typedef union MAC_RX_WPOINTER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_wptr_o : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_RX_WPOINTER_FIELD_T;

#define MAC_RX_FRPOINTER                          (WIFI_MAC_RX_REG_BASE + 0x200)
// Bit 15  :0      reg_rx_frptr                   U     RW        default = 'h0
typedef union MAC_RX_FRPOINTER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_frptr : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_RX_FRPOINTER_FIELD_T;

#define MAC_RX_HRPOINTER                          (WIFI_MAC_RX_REG_BASE + 0x204)
// Bit 15  :0      ro_rx_hrptr_o                  U     RO        default = 'h0
typedef union MAC_RX_HRPOINTER_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rx_hrptr_o : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_RX_HRPOINTER_FIELD_T;

#define MAC_BITMAPTABLE_CMD                       (WIFI_MAC_RX_REG_BASE + 0x20c)
// Bit 8   :0      reg_bitmaptable_cmd_addr       U     RW        default = 'h0
// Bit 17  :16     reg_bitmaptable_cmd_type       U     RW        default = 'h0
// Bit 29          ro_bitmaptable_cmd_fail        U     RO        default = 'h0
// Bit 30          reg_bitmaptable_cmd_en         U     RW        default = 'h0
// Bit 31          reg_bitmaptable_ready          U     RW        default = 'h1
typedef union MAC_BITMAPTABLE_CMD_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_bitmaptable_cmd_addr : 9;
    unsigned int rsvd_0 : 7;
    unsigned int reg_bitmaptable_cmd_type : 2;
    unsigned int rsvd_1 : 11;
    unsigned int ro_bitmaptable_cmd_fail : 1;
    unsigned int reg_bitmaptable_cmd_en : 1;
    unsigned int reg_bitmaptable_ready : 1;
  } b;
} MAC_BITMAPTABLE_CMD_FIELD_T;

#define MAC_BITMAPTABLE_DATA                      (WIFI_MAC_RX_REG_BASE + 0x210)
// Bit 31  :0      reg_mac_bitmaptable_data       U     RW        default = 'h0
typedef union MAC_BITMAPTABLE_DATA_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_bitmaptable_data : 32;
  } b;
} MAC_BITMAPTABLE_DATA_FIELD_T;

#define MAC_BITMAPTABLE_DATA2                     (WIFI_MAC_RX_REG_BASE + 0x214)
// Bit 31  :0      reg_mac_bitmaptable_data2      U     RW        default = 'h0
typedef union MAC_BITMAPTABLE_DATA2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_bitmaptable_data2 : 32;
  } b;
} MAC_BITMAPTABLE_DATA2_FIELD_T;

#define MAC_BITMAPTABLE_DATA3                     (WIFI_MAC_RX_REG_BASE + 0x218)
// Bit 23  :0      reg_mac_bitmaptable_data3      U     RW        default = 'h0
typedef union MAC_BITMAPTABLE_DATA3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_bitmaptable_data3 : 24;
    unsigned int rsvd_0 : 8;
  } b;
} MAC_BITMAPTABLE_DATA3_FIELD_T;

#define MAC_RXINFO                                (WIFI_MAC_RX_REG_BASE + 0x21c)
// Bit 7   :0      ro_rx_rate                     U     RO        default = 'h0
// Bit 15  :10     ro_rx_packet_type              U     RO        default = 'h0
// Bit 17  :16     ro_rx_a1match_id               U     RO        default = 'h0
// Bit 21  :20     ro_rx_bssidmatch_id            U     RO        default = 'h0
// Bit 31  :24     ro_rx_rssi_ant0                U     RO        default = 'h0
typedef union MAC_RXINFO_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rx_rate : 8;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rx_packet_type : 6;
    unsigned int ro_rx_a1match_id : 2;
    unsigned int rsvd_1 : 2;
    unsigned int ro_rx_bssidmatch_id : 2;
    unsigned int rsvd_2 : 2;
    unsigned int ro_rx_rssi_ant0 : 8;
  } b;
} MAC_RXINFO_FIELD_T;

#define MAC_RXINFO2                               (WIFI_MAC_RX_REG_BASE + 0x220)
// Bit 31  :0      ro_rx_a2_lo                    U     RO        default = 'h0
typedef union MAC_RXINFO2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rx_a2_lo : 32;
  } b;
} MAC_RXINFO2_FIELD_T;

#define MAC_BMFM_INFO                             (WIFI_MAC_RX_REG_BASE + 0x224)
// Bit 0           reg_cdbk_info                  U     RW        default = 'h0
// Bit 5   :4      reg_ng                         U     RW        default = 'h0
// Bit 19  :8      rxrat_ndp_vector               U     RW        default = 'h60b
// Bit 23  :20     reg_wfull_space                U     RW        default = 'h4
// Bit 28          reg_bmfm_disable               U     RW        default = 'h0
typedef union MAC_BMFM_INFO_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_cdbk_info : 1;
    unsigned int rsvd_0 : 3;
    unsigned int reg_ng : 2;
    unsigned int rsvd_1 : 2;
    unsigned int rxrat_ndp_vector : 12;
    unsigned int reg_wfull_space : 4;
    unsigned int rsvd_2 : 4;
    unsigned int reg_bmfm_disable : 1;
    unsigned int rsvd_3 : 3;
  } b;
} MAC_BMFM_INFO_FIELD_T;

#define MAC_AID                                   (WIFI_MAC_RX_REG_BASE + 0x228)
// Bit 15  :0      mac_aid                        U     RW        default = 'h0
typedef union MAC_AID_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int mac_aid : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_AID_FIELD_T;

#define MAC_BMFM_CNT                              (WIFI_MAC_RX_REG_BASE + 0x22c)
// Bit 15  :0      cnt_bmfm_pkt                   U     RO        default = 'h0
// Bit 31  :16     cnt_bmfm_pkt_err               U     RO        default = 'h0
typedef union MAC_BMFM_CNT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int cnt_bmfm_pkt : 16;
    unsigned int cnt_bmfm_pkt_err : 16;
  } b;
} MAC_BMFM_CNT_FIELD_T;

#define MAC_COMPRESSED_BMFM_CNT                   (WIFI_MAC_RX_REG_BASE + 0x230)
// Bit 15  :0      cnt_compressed_bmfm_frame      U     RO        default = 'h0
typedef union MAC_COMPRESSED_BMFM_CNT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int cnt_compressed_bmfm_frame : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_COMPRESSED_BMFM_CNT_FIELD_T;

#define MAC_RX_BEACON_CTRL                        (WIFI_MAC_RX_REG_BASE + 0x234)
// Bit 15  :0      reg_rx_beacon_buf_len          U     RW        default = 'h0
// Bit 16          reg_rx_beacon_path_sel         U     RW        default = 'h0
// Bit 19  :17     ro_rx_beacon_page_num          U     RO        default = 'h0
// Bit 29  :20     reg_rx_beacon_clr_timer        U     RW        default = 'h50
// Bit 30          reg_rx_beacon_page_mode        U     RW        default = 'h0
// Bit 31          reg_rx_beacon_flag             U     RW        default = 'h0
typedef union MAC_RX_BEACON_CTRL_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_beacon_buf_len : 16;
    unsigned int reg_rx_beacon_path_sel : 1;
    unsigned int ro_rx_beacon_page_num : 3;
    unsigned int reg_rx_beacon_clr_timer : 10;
    unsigned int reg_rx_beacon_page_mode : 1;
    unsigned int reg_rx_beacon_flag : 1;
  } b;
} MAC_RX_BEACON_CTRL_FIELD_T;

#endif

