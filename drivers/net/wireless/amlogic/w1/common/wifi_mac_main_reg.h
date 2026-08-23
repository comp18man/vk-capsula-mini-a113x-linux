#ifdef WIFI_MAC_MAIN_REG
#else
#define WIFI_MAC_MAIN_REG


#define WIFI_MAC_MAIN_REG_BASE                    (0xa00000)

#define MAC_CONTROL                               (WIFI_MAC_MAIN_REG_BASE + 0x0)
// Bit 0           reg_a1mis_abort_en             U     RW        default = 'h0
// Bit 1           reg_swirq                      U     RW        default = 'h0
// Bit 2           reg_sniffer_enable             U     RW        default = 'h0
// Bit 3           reg_smpdu_bitmap_update        U     RW        default = 'h0
// Bit 4           reg_txstartit_earlymode        U     RW        default = 'h0
// Bit 5           reg_ofdm_padding               U     RW        default = 'h1
// Bit 6           reg_status_count_enable        U     RW        default = 'h1
// Bit 11          reg_frequency_band             U     RW        default = 'h0
// Bit 12          reg_tximmstop                  U     RW        default = 'h0
// Bit 13          reg_endian                     U     RW        default = 'h1
// Bit 14          reg_cca_rst_req                U     RW        default = 'h0
// Bit 15          reg_fiqenable_rdrst            U     RW        default = 'h0
// Bit 17          reg_tcpip_offload_on           U     RW        default = 'h1
// Bit 18          reg_bws_ta_disable             U     RW        default = 'h1
// Bit 19          reg_vht_txtime_sel             U     RW        default = 'h0
// Bit 20          reg_qosnull_notupd_bitmap      U     RW        default = 'h0
typedef union MAC_CONTROL_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_a1mis_abort_en : 1;
    unsigned int reg_swirq : 1;
    unsigned int reg_sniffer_enable : 1;
    unsigned int reg_smpdu_bitmap_update : 1;
    unsigned int reg_txstartit_earlymode : 1;
    unsigned int reg_ofdm_padding : 1;
    unsigned int reg_status_count_enable : 1;
    unsigned int rsvd_0 : 4;
    unsigned int reg_frequency_band : 1;
    unsigned int reg_tximmstop : 1;
    unsigned int reg_endian : 1;
    unsigned int reg_cca_rst_req : 1;
    unsigned int reg_fiqenable_rdrst : 1;
    unsigned int rsvd_1 : 1;
    unsigned int reg_tcpip_offload_on : 1;
    unsigned int reg_bws_ta_disable : 1;
    unsigned int reg_vht_txtime_sel : 1;
    unsigned int reg_qosnull_notupd_bitmap : 1;
    unsigned int rsvd_2 : 11;
  } b;
} MAC_CONTROL_FIELD_T;

#define MAC_IRQ_MASK                              (WIFI_MAC_MAIN_REG_BASE + 0x4)
// Bit 0           reg_txstart_mask               U     RW        default = 'h0
// Bit 1           reg_txend_mask                 U     RW        default = 'h0
// Bit 9           reg_ackto_mask                 U     RW        default = 'h0
// Bit 11          reg_rxstart_mask               U     RW        default = 'h0
// Bit 12          reg_rxend_mask                 U     RW        default = 'h0
// Bit 31  :16     reg_abscount_mask              U     RW        default = 'h0
typedef union MAC_IRQ_MASK_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txstart_mask : 1;
    unsigned int reg_txend_mask : 1;
    unsigned int rsvd_0 : 7;
    unsigned int reg_ackto_mask : 1;
    unsigned int rsvd_1 : 1;
    unsigned int reg_rxstart_mask : 1;
    unsigned int reg_rxend_mask : 1;
#ifdef PROJECT_T9026
    unsigned int rsvd_2 : 7;
    unsigned int reg_abscount_mask : 12;
#else
    unsigned int rsvd_2 : 3;
    unsigned int reg_abscount_mask : 16;
#endif
  } b;
} MAC_IRQ_MASK_FIELD_T;

#define MAC_IRQ_STATUS                            (WIFI_MAC_MAIN_REG_BASE + 0xc)
// Bit 0           reg_txstart_it                 U     W1C       default = 'h0
// Bit 1           reg_txend_it                   U     W1C       default = 'h0
// Bit 3   :2      ro_txvmac_id                   U     RO        default = 'h0
// Bit 7   :4      ro_txstart_queue               U     RO        default = 'h0
// Bit 8           ro_txenderr                    U     RO        default = 'h0
// Bit 9           reg_ackto_it                   U     W1C       default = 'h0
// Bit 10          ro_txcol_flag                  U     RO        default = 'h0
// Bit 11          reg_rxstart_it                 U     W1C       default = 'h0
// Bit 12          reg_rxend_it                   U     W1C       default = 'h0
// Bit 13          ro_rxmpdu_ok                   U     RO        default = 'h0
// Bit 14          ro_unicast                     U     RO        default = 'h0
// Bit 15          ro_a1match                     U     RO        default = 'h0
// Bit 16          ro_bssidmatch                  U     RO        default = 'h0
// Bit 18          ro_srambuf_full1               U     RO        default = 'h0
// Bit 19          reg_swirq_it                   U     W1C       default = 'h0
// Bit 25  :20     reg_tx_frame_type              U     RW        default = 'h0
// Bit 30          reg_abscount_intr              U     W1C       default = 'h0
// Bit 31          reg_fiq_enable                 U     RW        default = 'h0
typedef union MAC_IRQ_STATUS_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txstart_it : 1;
    unsigned int reg_txend_it : 1;
    unsigned int ro_txvmac_id : 2;
    unsigned int ro_txstart_queue : 4;
    unsigned int ro_txenderr : 1;
    unsigned int reg_ackto_it : 1;
    unsigned int ro_txcol_flag : 1;
    unsigned int reg_rxstart_it : 1;
    unsigned int reg_rxend_it : 1;
    unsigned int ro_rxmpdu_ok : 1;
    unsigned int ro_unicast : 1;
    unsigned int ro_a1match : 1;
    unsigned int ro_bssidmatch : 1;
    unsigned int rsvd_0 : 1;
    unsigned int ro_srambuf_full1 : 1;
    unsigned int reg_swirq_it : 1;
    unsigned int reg_tx_frame_type : 6;
    unsigned int rsvd_1 : 4;
    unsigned int reg_abscount_intr : 1;
    unsigned int reg_fiq_enable : 1;
  } b;
} MAC_IRQ_STATUS_FIELD_T;

#define MAC_ERR_STATUS                            (WIFI_MAC_MAIN_REG_BASE + 0x10)
// Bit 0           ro_fiac_fw_timeout_err         U     RO        default = 'h0
// Bit 1           ro_hw_timeout_err              U     RO        default = 'h0
// Bit 2           ro_tx_wd_timeout_err           U     RO        default = 'h0
// Bit 3           ro_txinvalidlen_err            U     RO        default = 'h0
// Bit 4           ro_tx_keymiss                  U     RO        default = 'h0
// Bit 5           ro_tximmstop_err               U     RO        default = 'h0
// Bit 9   :6      ro_tx_phyerror                 U     RO        default = 'h0
// Bit 18  :10     ro_txcol_queue                 U     RO        default = 'h0
// Bit 19          ro_tx_not_grant                U     RO        default = 'h0
// Bit 22          ro_srambuf_full2               U     RO        default = 'h0
// Bit 23          ro_fcserr                      U     RO        default = 'h0
// Bit 24          ro_rxfifoov                    U     RO        default = 'h0
// Bit 25          ro_rx_wd_timeout_err           U     RO        default = 'h0
// Bit 26          ro_short_length                U     RO        default = 'h0
// Bit 27          ro_length_error                U     RO        default = 'h0
// Bit 31  :28     ro_rx_phyerror                 U     RO        default = 'h0
typedef union MAC_ERR_STATUS_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_fiac_fw_timeout_err : 1;
    unsigned int ro_hw_timeout_err : 1;
    unsigned int ro_tx_wd_timeout_err : 1;
    unsigned int ro_txinvalidlen_err : 1;
    unsigned int ro_tx_keymiss : 1;
    unsigned int ro_tximmstop_err : 1;
    unsigned int ro_tx_phyerror : 4;
    unsigned int ro_txcol_queue : 9;
    unsigned int ro_tx_not_grant : 1;
    unsigned int rsvd_0 : 2;
    unsigned int ro_srambuf_full2 : 1;
    unsigned int ro_fcserr : 1;
    unsigned int ro_rxfifoov : 1;
    unsigned int ro_rx_wd_timeout_err : 1;
    unsigned int ro_short_length : 1;
    unsigned int ro_length_error : 1;
    unsigned int ro_rx_phyerror : 4;
  } b;
} MAC_ERR_STATUS_FIELD_T;

#define ABS_STATUS                                (WIFI_MAC_MAIN_REG_BASE + 0x14)
// Bit 16          reg_mac_absstatus0             U     W1C       default = 'h0
// Bit 17          reg_mac_absstatus1             U     W1C       default = 'h0
// Bit 18          reg_mac_absstatus2             U     W1C       default = 'h0
// Bit 19          reg_mac_absstatus3             U     W1C       default = 'h0
// Bit 20          reg_mac_absstatus4             U     W1C       default = 'h0
// Bit 21          reg_mac_absstatus5             U     W1C       default = 'h0
// Bit 22          reg_mac_absstatus6             U     W1C       default = 'h0
// Bit 23          reg_mac_absstatus7             U     W1C       default = 'h0
// Bit 24          reg_mac_absstatus8             U     W1C       default = 'h0
// Bit 25          reg_mac_absstatus9             U     W1C       default = 'h0
// Bit 26          reg_mac_absstatus10            U     W1C       default = 'h0
// Bit 27          reg_mac_absstatus11            U     W1C       default = 'h0
// Bit 28          reg_mac_absstatus12            U     W1C       default = 'h0
// Bit 29          reg_mac_absstatus13            U     W1C       default = 'h0
// Bit 30          reg_mac_absstatus14            U     W1C       default = 'h0
// Bit 31          reg_mac_absstatus15            U     W1C       default = 'h0
typedef union ABS_STATUS_FIELD
{
  unsigned int data;
#ifdef PROJECT_T9026
  struct
  {
    unsigned int rsvd_0 : 20;
    unsigned int reg_mac_absstatus0 : 1;
    unsigned int reg_mac_absstatus1 : 1;
    unsigned int reg_mac_absstatus2 : 1;
    unsigned int reg_mac_absstatus3 : 1;
    unsigned int reg_mac_absstatus4 : 1;
    unsigned int reg_mac_absstatus5 : 1;
    unsigned int reg_mac_absstatus6 : 1;
    unsigned int reg_mac_absstatus7 : 1;
    unsigned int reg_mac_absstatus8 : 1;
    unsigned int reg_mac_absstatus9 : 1;
    unsigned int reg_mac_absstatus10 : 1;
    unsigned int reg_mac_absstatus11 : 1;
  } b;
#else
  struct
  {
    unsigned int rsvd_0 : 16;
    unsigned int reg_mac_absstatus0 : 1;
    unsigned int reg_mac_absstatus1 : 1;
    unsigned int reg_mac_absstatus2 : 1;
    unsigned int reg_mac_absstatus3 : 1;
    unsigned int reg_mac_absstatus4 : 1;
    unsigned int reg_mac_absstatus5 : 1;
    unsigned int reg_mac_absstatus6 : 1;
    unsigned int reg_mac_absstatus7 : 1;
    unsigned int reg_mac_absstatus8 : 1;
    unsigned int reg_mac_absstatus9 : 1;
    unsigned int reg_mac_absstatus10 : 1;
    unsigned int reg_mac_absstatus11 : 1;
    unsigned int reg_mac_absstatus12 : 1;
    unsigned int reg_mac_absstatus13 : 1;
    unsigned int reg_mac_absstatus14 : 1;
    unsigned int reg_mac_absstatus15 : 1;
  } b;
#endif
} ABS_STATUS_FIELD_T;

#define MAC_LOCAL_ADDRLO0                         (WIFI_MAC_MAIN_REG_BASE + 0x1c)
// Bit 31  :0      reg_mac_local_addrlo0          U     RW        default = 'h1279ffff
typedef union MAC_LOCAL_ADDRLO0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrlo0 : 32;
  } b;
} MAC_LOCAL_ADDRLO0_FIELD_T;

#define MAC_LOCAL_ADDRHI0                         (WIFI_MAC_MAIN_REG_BASE + 0x20)
// Bit 15  :0      reg_mac_local_addrhi0          U     RW        default = 'h5634
typedef union MAC_LOCAL_ADDRHI0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrhi0 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_ADDRHI0_FIELD_T;

#define MAC_LOCAL_ADDRLO1                         (WIFI_MAC_MAIN_REG_BASE + 0x24)
// Bit 31  :0      reg_mac_local_addrlo1          U     RW        default = 'h1279ffff
typedef union MAC_LOCAL_ADDRLO1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrlo1 : 32;
  } b;
} MAC_LOCAL_ADDRLO1_FIELD_T;

#define MAC_LOCAL_ADDRHI1                         (WIFI_MAC_MAIN_REG_BASE + 0x28)
// Bit 15  :0      reg_mac_local_addrhi1          U     RW        default = 'h5634
typedef union MAC_LOCAL_ADDRHI1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrhi1 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_ADDRHI1_FIELD_T;

#define MAC_LOCAL_ADDRLO2                         (WIFI_MAC_MAIN_REG_BASE + 0x2c)
// Bit 31  :0      reg_mac_local_addrlo2          U     RW        default = 'h1279ffff
typedef union MAC_LOCAL_ADDRLO2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrlo2 : 32;
  } b;
} MAC_LOCAL_ADDRLO2_FIELD_T;

#define MAC_LOCAL_ADDRHI2                         (WIFI_MAC_MAIN_REG_BASE + 0x30)
// Bit 15  :0      reg_mac_local_addrhi2          U     RW        default = 'h5634
typedef union MAC_LOCAL_ADDRHI2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrhi2 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_ADDRHI2_FIELD_T;

#define MAC_LOCAL_ADDRLO3                         (WIFI_MAC_MAIN_REG_BASE + 0x34)
// Bit 31  :0      reg_mac_local_addrlo3          U     RW        default = 'h1279ffff
typedef union MAC_LOCAL_ADDRLO3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrlo3 : 32;
  } b;
} MAC_LOCAL_ADDRLO3_FIELD_T;

#define MAC_LOCAL_ADDRHI3                         (WIFI_MAC_MAIN_REG_BASE + 0x38)
// Bit 15  :0      reg_mac_local_addrhi3          U     RW        default = 'h5634
typedef union MAC_LOCAL_ADDRHI3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_addrhi3 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_ADDRHI3_FIELD_T;

#define MAC_LOCAL_BSSIDLO0                        (WIFI_MAC_MAIN_REG_BASE + 0x3c)
// Bit 31  :0      reg_mac_local_bssidlo0         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDLO0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidlo0 : 32;
  } b;
} MAC_LOCAL_BSSIDLO0_FIELD_T;

#define MAC_LOCAL_BSSIDHI0                        (WIFI_MAC_MAIN_REG_BASE + 0x40)
// Bit 15  :0      reg_mac_local_bssidhi0         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDHI0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidhi0 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_BSSIDHI0_FIELD_T;

#define MAC_LOCAL_BSSIDLO1                        (WIFI_MAC_MAIN_REG_BASE + 0x44)
// Bit 31  :0      reg_mac_local_bssidlo1         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDLO1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidlo1 : 32;
  } b;
} MAC_LOCAL_BSSIDLO1_FIELD_T;

#define MAC_LOCAL_BSSIDHI1                        (WIFI_MAC_MAIN_REG_BASE + 0x48)
// Bit 15  :0      reg_mac_local_bssidhi1         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDHI1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidhi1 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_BSSIDHI1_FIELD_T;

#define MAC_LOCAL_BSSIDLO2                        (WIFI_MAC_MAIN_REG_BASE + 0x4c)
// Bit 31  :0      reg_mac_local_bssidlo2         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDLO2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidlo2 : 32;
  } b;
} MAC_LOCAL_BSSIDLO2_FIELD_T;

#define MAC_LOCAL_BSSIDHI2                        (WIFI_MAC_MAIN_REG_BASE + 0x50)
// Bit 15  :0      reg_mac_local_bssidhi2         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDHI2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidhi2 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_BSSIDHI2_FIELD_T;

#define MAC_LOCAL_BSSIDLO3                        (WIFI_MAC_MAIN_REG_BASE + 0x54)
// Bit 31  :0      reg_mac_local_bssidlo3         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDLO3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidlo3 : 32;
  } b;
} MAC_LOCAL_BSSIDLO3_FIELD_T;

#define MAC_LOCAL_BSSIDHI3                        (WIFI_MAC_MAIN_REG_BASE + 0x58)
// Bit 15  :0      reg_mac_local_bssidhi3         U     RW        default = 'h0
typedef union MAC_LOCAL_BSSIDHI3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_local_bssidhi3 : 16;
    unsigned int rsvd_0 : 16;
  } b;
} MAC_LOCAL_BSSIDHI3_FIELD_T;

#define STATUS_CNT_CTRL_IRQ                       (WIFI_MAC_MAIN_REG_BASE + 0x5c)
// Bit 5   :0      reg_fiq_status_cnt_idx0        U     RW        default = 'h0
// Bit 11  :6      reg_fiq_status_cnt_idx1        U     RW        default = 'h1
// Bit 17  :12     reg_fiq_status_cnt_idx2        U     RW        default = 'h9
// Bit 23  :18     reg_fiq_status_cnt_idx3        U     RW        default = 'h13
// Bit 24          reg_fiq_status_cnt_en0         U     RW        default = 'h1
// Bit 25          reg_fiq_status_cnt_en1         U     RW        default = 'h1
// Bit 26          reg_fiq_status_cnt_en2         U     RW        default = 'h1
// Bit 27          reg_fiq_status_cnt_en3         U     RW        default = 'h0
// Bit 28          reg_fiq_status_cnt_clr0        U     RW        default = 'h0
// Bit 29          reg_fiq_status_cnt_clr1        U     RW        default = 'h0
// Bit 30          reg_fiq_status_cnt_clr2        U     RW        default = 'h0
// Bit 31          reg_fiq_status_cnt_clr3        U     RW        default = 'h0
typedef union STATUS_CNT_CTRL_IRQ_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_fiq_status_cnt_idx0 : 6;
    unsigned int reg_fiq_status_cnt_idx1 : 6;
    unsigned int reg_fiq_status_cnt_idx2 : 6;
    unsigned int reg_fiq_status_cnt_idx3 : 6;
    unsigned int reg_fiq_status_cnt_en0 : 1;
    unsigned int reg_fiq_status_cnt_en1 : 1;
    unsigned int reg_fiq_status_cnt_en2 : 1;
    unsigned int reg_fiq_status_cnt_en3 : 1;
    unsigned int reg_fiq_status_cnt_clr0 : 1;
    unsigned int reg_fiq_status_cnt_clr1 : 1;
    unsigned int reg_fiq_status_cnt_clr2 : 1;
    unsigned int reg_fiq_status_cnt_clr3 : 1;
  } b;
} STATUS_CNT_CTRL_IRQ_FIELD_T;

#define IRQ_STATUS_CNT0                           (WIFI_MAC_MAIN_REG_BASE + 0x60)
// Bit 31  :0      ro_fiq_status_cnt0             U     RO        default = 'h0
typedef union IRQ_STATUS_CNT0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_fiq_status_cnt0 : 32;
  } b;
} IRQ_STATUS_CNT0_FIELD_T;

#define IRQ_STATUS_CNT1                           (WIFI_MAC_MAIN_REG_BASE + 0x64)
// Bit 31  :0      ro_fiq_status_cnt1             U     RO        default = 'h0
typedef union IRQ_STATUS_CNT1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_fiq_status_cnt1 : 32;
  } b;
} IRQ_STATUS_CNT1_FIELD_T;

#define IRQ_STATUS_CNT2                           (WIFI_MAC_MAIN_REG_BASE + 0x68)
// Bit 31  :0      ro_fiq_status_cnt2             U     RO        default = 'h0
typedef union IRQ_STATUS_CNT2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_fiq_status_cnt2 : 32;
  } b;
} IRQ_STATUS_CNT2_FIELD_T;

#define MAC_CLKJIC0                               (WIFI_MAC_MAIN_REG_BASE + 0x6c)
// Bit 7   :0      reg_rx_clk_jic                 U     RW        default = 'hff
// Bit 15  :8      reg_tx_clk_jic                 U     RW        default = 'hff
// Bit 23  :16     reg_timer_clk_jic              U     RW        default = 'hff
// Bit 31  :24     reg_reg_clk_jic                U     RW        default = 'hff
typedef union MAC_CLKJIC0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rx_clk_jic : 8;
    unsigned int reg_tx_clk_jic : 8;
    unsigned int reg_timer_clk_jic : 8;
    unsigned int reg_reg_clk_jic : 8;
  } b;
} MAC_CLKJIC0_FIELD_T;

#define MAC_CLKJIC1                               (WIFI_MAC_MAIN_REG_BASE + 0x70)
// Bit 11  :0      reg_dp_clk_jic                 U     RW        default = 'hfff
// Bit 21  :16     reg_ahb_clk_jic                U     RW        default = 'h3f
typedef union MAC_CLKJIC1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_dp_clk_jic : 12;
    unsigned int rsvd_0 : 4;
    unsigned int reg_ahb_clk_jic : 6;
    unsigned int rsvd_1 : 10;
  } b;
} MAC_CLKJIC1_FIELD_T;

#define IRQ_STATUS_CNT3                           (WIFI_MAC_MAIN_REG_BASE + 0x74)
// Bit 31  :0      ro_fiq_status_cnt3             U     RO        default = 'h0
typedef union IRQ_STATUS_CNT3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_fiq_status_cnt3 : 32;
  } b;
} IRQ_STATUS_CNT3_FIELD_T;

#define MAC_DEBUG0                                (WIFI_MAC_MAIN_REG_BASE + 0x78)
// Bit 31  :0      reg_mac_debug0                 U     RW        default = 'h0
typedef union MAC_DEBUG0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_debug0 : 32;
  } b;
} MAC_DEBUG0_FIELD_T;

#define MAC_DEBUG1                                (WIFI_MAC_MAIN_REG_BASE + 0x7c)
// Bit 31  :0      reg_mac_debug1                 U     RW        default = 'h0
typedef union MAC_DEBUG1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_debug1 : 32;
  } b;
} MAC_DEBUG1_FIELD_T;

#define MAC_DEBUG2                                (WIFI_MAC_MAIN_REG_BASE + 0x80)
// Bit 31  :0      reg_mac_debug2                 U     RW        default = 'h0
typedef union MAC_DEBUG2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_debug2 : 32;
  } b;
} MAC_DEBUG2_FIELD_T;

#define MAC_DEBUG3                                (WIFI_MAC_MAIN_REG_BASE + 0x84)
// Bit 31  :0      reg_mac_debug3                 U     RW        default = 'h0
typedef union MAC_DEBUG3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_debug3 : 32;
  } b;
} MAC_DEBUG3_FIELD_T;

#define MAC_FIACMAXDEL                            (WIFI_MAC_MAIN_REG_BASE + 0x88)
// Bit 6   :0      reg_txvector_hw_maxdel         U     RW        default = 'h3e
// Bit 12  :8      reg_fiac_maxdela               U     RW        default = 'h18
// Bit 20  :16     reg_fiac_maxdelb               U     RW        default = 'h10
typedef union MAC_FIACMAXDEL_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txvector_hw_maxdel : 7;
    unsigned int rsvd_0 : 1;
    unsigned int reg_fiac_maxdela : 5;
    unsigned int rsvd_1 : 3;
    unsigned int reg_fiac_maxdelb : 5;
    unsigned int rsvd_2 : 11;
  } b;
} MAC_FIACMAXDEL_FIELD_T;

#define DEVICE0_MODE                              (WIFI_MAC_MAIN_REG_BASE + 0x90)
// Bit 31  :0      reg_device0_mode               U     RW        default = 'h0
typedef union DEVICE0_MODE_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_device0_mode : 32;
  } b;
} DEVICE0_MODE_FIELD_T;

#define DEVICE1_MODE                              (WIFI_MAC_MAIN_REG_BASE + 0x94)
// Bit 31  :0      reg_device1_mode               U     RW        default = 'h0
typedef union DEVICE1_MODE_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_device1_mode : 32;
  } b;
} DEVICE1_MODE_FIELD_T;

#define DEVICE2_MODE                              (WIFI_MAC_MAIN_REG_BASE + 0x98)
// Bit 31  :0      reg_device2_mode               U     RW        default = 'h0
typedef union DEVICE2_MODE_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_device2_mode : 32;
  } b;
} DEVICE2_MODE_FIELD_T;

#define DEVICE3_MODE                              (WIFI_MAC_MAIN_REG_BASE + 0x9c)
// Bit 31  :0      reg_device3_mode               U     RW        default = 'h0
typedef union DEVICE3_MODE_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_device3_mode : 32;
  } b;
} DEVICE3_MODE_FIELD_T;

#define MAC_INTNUM                                (WIFI_MAC_MAIN_REG_BASE + 0xa0)
// Bit 15  :0      reg_rxend_cnt                  U     RW        default = 'h0
// Bit 31  :16     reg_rxstart_cnt                U     RW        default = 'h0
typedef union MAC_INTNUM_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_rxend_cnt : 16;
    unsigned int reg_rxstart_cnt : 16;
  } b;
} MAC_INTNUM_FIELD_T;

#define FRAME_TYPE_CNT_CTRL                       (WIFI_MAC_MAIN_REG_BASE + 0xa4)
// Bit 5   :0      reg_frame_type_idx0            U     RW        default = 'h1b
// Bit 11  :6      reg_frame_type_idx1            U     RW        default = 'h1c
// Bit 17  :12     reg_frame_type_idx2            U     RW        default = 'h1d
// Bit 23  :18     reg_frame_type_idx3            U     RW        default = 'h28
// Bit 24          reg_frame_cnt_en0              U     RW        default = 'h1
// Bit 25          reg_frame_cnt_en1              U     RW        default = 'h1
// Bit 26          reg_frame_cnt_en2              U     RW        default = 'h1
// Bit 27          reg_frame_cnt_en3              U     RW        default = 'h1
// Bit 28          reg_frame_cnt_clr0             U     RW        default = 'h0
// Bit 29          reg_frame_cnt_clr1             U     RW        default = 'h0
// Bit 30          reg_frame_cnt_clr2             U     RW        default = 'h0
// Bit 31          reg_frame_cnt_clr3             U     RW        default = 'h0
typedef union FRAME_TYPE_CNT_CTRL_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_frame_type_idx0 : 6;
    unsigned int reg_frame_type_idx1 : 6;
    unsigned int reg_frame_type_idx2 : 6;
    unsigned int reg_frame_type_idx3 : 6;
    unsigned int reg_frame_cnt_en0 : 1;
    unsigned int reg_frame_cnt_en1 : 1;
    unsigned int reg_frame_cnt_en2 : 1;
    unsigned int reg_frame_cnt_en3 : 1;
    unsigned int reg_frame_cnt_clr0 : 1;
    unsigned int reg_frame_cnt_clr1 : 1;
    unsigned int reg_frame_cnt_clr2 : 1;
    unsigned int reg_frame_cnt_clr3 : 1;
  } b;
} FRAME_TYPE_CNT_CTRL_FIELD_T;

#define FRAM_TYPE_CNT0                            (WIFI_MAC_MAIN_REG_BASE + 0xa8)
// Bit 31  :0      ro_frame_cnt0                  U     RO        default = 'h0
typedef union FRAM_TYPE_CNT0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_frame_cnt0 : 32;
  } b;
} FRAM_TYPE_CNT0_FIELD_T;

#define FRAM_TYPE_CNT1                            (WIFI_MAC_MAIN_REG_BASE + 0xac)
// Bit 31  :0      ro_frame_cnt1                  U     RO        default = 'h0
typedef union FRAM_TYPE_CNT1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_frame_cnt1 : 32;
  } b;
} FRAM_TYPE_CNT1_FIELD_T;

#define FRAM_TYPE_CNT2                            (WIFI_MAC_MAIN_REG_BASE + 0xf4)
// Bit 31  :0      ro_frame_cnt2                  U     RO        default = 'h0
typedef union FRAM_TYPE_CNT2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_frame_cnt2 : 32;
  } b;
} FRAM_TYPE_CNT2_FIELD_T;

#define FRAM_TYPE_CNT3                            (WIFI_MAC_MAIN_REG_BASE + 0xf8)
// Bit 31  :0      ro_frame_cnt3                  U     RO        default = 'h0
typedef union FRAM_TYPE_CNT3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_frame_cnt3 : 32;
  } b;
} FRAM_TYPE_CNT3_FIELD_T;

#define PACKET_IFS_US                             (WIFI_MAC_MAIN_REG_BASE + 0x124)
// Bit 7   :0      reg_dot11n_sifs_us             U     RW        default = 'ha
// Bit 15  :8      reg_dot11b_sifs_us             U     RW        default = 'ha
// Bit 19  :16     reg_rifs_us                    U     RW        default = 'h2
// Bit 23  :20     reg_signal_extension_us        U     RW        default = 'h6
// Bit 31  :24     reg_packet_ifs_rsvd            U     RW        default = 'h0
typedef union PACKET_IFS_US_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_dot11n_sifs_us : 8;
    unsigned int reg_dot11b_sifs_us : 8;
    unsigned int reg_rifs_us : 4;
    unsigned int reg_signal_extension_us : 4;
    unsigned int reg_packet_ifs_rsvd : 8;
  } b;
} PACKET_IFS_US_FIELD_T;

#define CTRL_FRAME_MASK                           (WIFI_MAC_MAIN_REG_BASE + 0x138)
// Bit 3   :0      reg_tx_control_frame_mask      U     RW        default = 'h0
typedef union CTRL_FRAME_MASK_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_tx_control_frame_mask : 4;
    unsigned int rsvd_0 : 28;
  } b;
} CTRL_FRAME_MASK_FIELD_T;

#define MAC_KEYINFO0                              (WIFI_MAC_MAIN_REG_BASE + 0x140)
// Bit 3   :0      reg_unicast_cipher_type0       U     RW        default = 'h0
// Bit 7   :4      reg_multicast_cipher_type0     U     RW        default = 'h0
// Bit 8           reg_multicast_key_lock0        U     RW        default = 'h0
// Bit 19  :16     reg_unicast_cipher_type1       U     RW        default = 'h0
// Bit 23  :20     reg_multicast_cipher_type1     U     RW        default = 'h0
// Bit 24          reg_multicast_key_lock1        U     RW        default = 'h0
typedef union MAC_KEYINFO0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_unicast_cipher_type0 : 4;
    unsigned int reg_multicast_cipher_type0 : 4;
    unsigned int reg_multicast_key_lock0 : 1;
    unsigned int rsvd_0 : 7;
    unsigned int reg_unicast_cipher_type1 : 4;
    unsigned int reg_multicast_cipher_type1 : 4;
    unsigned int reg_multicast_key_lock1 : 1;
    unsigned int rsvd_1 : 7;
  } b;
} MAC_KEYINFO0_FIELD_T;

#define MAC_KEYINFO1                              (WIFI_MAC_MAIN_REG_BASE + 0x144)
// Bit 3   :0      reg_unicast_cipher_type2       U     RW        default = 'h0
// Bit 7   :4      reg_multicast_cipher_type2     U     RW        default = 'h0
// Bit 8           reg_multicast_key_lock2        U     RW        default = 'h0
// Bit 19  :16     reg_unicast_cipher_type3       U     RW        default = 'h0
// Bit 23  :20     reg_multicast_cipher_type3     U     RW        default = 'h0
// Bit 24          reg_multicast_key_lock3        U     RW        default = 'h0
typedef union MAC_KEYINFO1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_unicast_cipher_type2 : 4;
    unsigned int reg_multicast_cipher_type2 : 4;
    unsigned int reg_multicast_key_lock2 : 1;
    unsigned int rsvd_0 : 7;
    unsigned int reg_unicast_cipher_type3 : 4;
    unsigned int reg_multicast_cipher_type3 : 4;
    unsigned int reg_multicast_key_lock3 : 1;
    unsigned int rsvd_1 : 7;
  } b;
} MAC_KEYINFO1_FIELD_T;

#define MAC_KEYTABLE_CMD                          (WIFI_MAC_MAIN_REG_BASE + 0x148)
// Bit 11  :0      reg_keytable_cmd_addr          U     RW        default = 'h0
// Bit 15  :12     reg_keytable_cmd_cipher_type     U     RW        default = 'h0
// Bit 18  :16     reg_keytable_cmd_type          U     RW        default = 'h0
// Bit 19          reg_keytable_cmd_multikey_sel     U     RW        default = 'h0
// Bit 27  :20     reg_keytable_cmd_stationid     U     RW        default = 'h0
// Bit 29  :28     ro_keytable_cmd_fail           U     RO        default = 'h0
// Bit 30          reg_keytable_cmd_en            U     RW        default = 'h0
// Bit 31          reg_keytable_cmd_ready         U     RW        default = 'h1
typedef union MAC_KEYTABLE_CMD_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_keytable_cmd_addr : 12;
    unsigned int reg_keytable_cmd_cipher_type : 4;
    unsigned int reg_keytable_cmd_type : 3;
    unsigned int reg_keytable_cmd_multikey_sel : 1;
    unsigned int reg_keytable_cmd_stationid : 8;
    unsigned int ro_keytable_cmd_fail : 2;
    unsigned int reg_keytable_cmd_en : 1;
    unsigned int reg_keytable_cmd_ready : 1;
  } b;
} MAC_KEYTABLE_CMD_FIELD_T;

#define MAC_KEYTABLE_DATA                         (WIFI_MAC_MAIN_REG_BASE + 0x14c)
// Bit 31  :0      reg_mac_keytable_data          U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data : 32;
  } b;
} MAC_KEYTABLE_DATA_FIELD_T;

#define MAC_KEYTABLE_DATA2                        (WIFI_MAC_MAIN_REG_BASE + 0x150)
// Bit 31  :0      reg_mac_keytable_data2         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data2 : 32;
  } b;
} MAC_KEYTABLE_DATA2_FIELD_T;

#define MAC_KEYTABLE_DATA3                        (WIFI_MAC_MAIN_REG_BASE + 0x154)
// Bit 31  :0      reg_mac_keytable_data3         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data3 : 32;
  } b;
} MAC_KEYTABLE_DATA3_FIELD_T;

#define MAC_KEYTABLE_DATA4                        (WIFI_MAC_MAIN_REG_BASE + 0x158)
// Bit 31  :0      reg_mac_keytable_data4         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data4 : 32;
  } b;
} MAC_KEYTABLE_DATA4_FIELD_T;

#define MAC_KEYTABLE_DATA5                        (WIFI_MAC_MAIN_REG_BASE + 0x15c)
// Bit 31  :0      reg_mac_keytable_data5         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data5 : 32;
  } b;
} MAC_KEYTABLE_DATA5_FIELD_T;

#define MAC_KEYTABLE_DATA6                        (WIFI_MAC_MAIN_REG_BASE + 0x160)
// Bit 31  :0      reg_mac_keytable_data6         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data6 : 32;
  } b;
} MAC_KEYTABLE_DATA6_FIELD_T;

#define MAC_KEYTABLE_DATA7                        (WIFI_MAC_MAIN_REG_BASE + 0x164)
// Bit 31  :0      reg_mac_keytable_data7         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data7 : 32;
  } b;
} MAC_KEYTABLE_DATA7_FIELD_T;

#define MAC_KEYTABLE_DATA8                        (WIFI_MAC_MAIN_REG_BASE + 0x168)
// Bit 31  :0      reg_mac_keytable_data8         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data8 : 32;
  } b;
} MAC_KEYTABLE_DATA8_FIELD_T;

#define MAC_KEYTABLE_DATA9                        (WIFI_MAC_MAIN_REG_BASE + 0x16c)
// Bit 31  :0      reg_mac_keytable_data9         U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data9 : 32;
  } b;
} MAC_KEYTABLE_DATA9_FIELD_T;

#define MAC_KEYTABLE_DATA10                       (WIFI_MAC_MAIN_REG_BASE + 0x170)
// Bit 31  :0      reg_mac_keytable_data10        U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data10 : 32;
  } b;
} MAC_KEYTABLE_DATA10_FIELD_T;

#define MAC_KEYTABLE_DATA11                       (WIFI_MAC_MAIN_REG_BASE + 0x174)
// Bit 31  :0      reg_mac_keytable_data11        U     RW        default = 'h0
typedef union MAC_KEYTABLE_DATA11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_keytable_data11 : 32;
  } b;
} MAC_KEYTABLE_DATA11_FIELD_T;

#define MAC_KEYMAP                                (WIFI_MAC_MAIN_REG_BASE + 0x178)
// Bit 7   :0      reg_keytable_offset0           U     RW        default = 'h0
// Bit 15  :8      reg_keytable_offset1           U     RW        default = 'h0
// Bit 23  :16     reg_keytable_offset2           U     RW        default = 'h0
// Bit 31  :24     reg_keytable_offset3           U     RW        default = 'h0
typedef union MAC_KEYMAP_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_keytable_offset0 : 8;
    unsigned int reg_keytable_offset1 : 8;
    unsigned int reg_keytable_offset2 : 8;
    unsigned int reg_keytable_offset3 : 8;
  } b;
} MAC_KEYMAP_FIELD_T;

#define MAC_DBG_STATUS1                           (WIFI_MAC_MAIN_REG_BASE + 0x190)
// Bit 4   :0      ro_key_err_type                U     RO        default = 'h0
typedef union MAC_DBG_STATUS1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_key_err_type : 5;
    unsigned int rsvd_0 : 27;
  } b;
} MAC_DBG_STATUS1_FIELD_T;

#define TX_START_CHK                              (WIFI_MAC_MAIN_REG_BASE + 0x1a8)
// Bit 25  :0      ro_txstart_cnt                 U     RO        default = 'h0
// Bit 26          reg_txstart_cnt_en             U     RW        default = 'h0
// Bit 27          reg_txstart_cnt_clr            U     RW        default = 'h1
// Bit 31  :28     reg_check_tx_queue             U     RW        default = 'h9
typedef union TX_START_CHK_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txstart_cnt : 26;
    unsigned int reg_txstart_cnt_en : 1;
    unsigned int reg_txstart_cnt_clr : 1;
    unsigned int reg_check_tx_queue : 4;
  } b;
} TX_START_CHK_FIELD_T;

#define TX_END_CHK                                (WIFI_MAC_MAIN_REG_BASE + 0x1ac)
// Bit 25  :0      ro_txend_cnt                   U     RO        default = 'h0
// Bit 26          reg_txend_cnt_en               U     RW        default = 'h0
// Bit 27          reg_txend_cnt_clr              U     RW        default = 'h1
typedef union TX_END_CHK_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txend_cnt : 26;
    unsigned int reg_txend_cnt_en : 1;
    unsigned int reg_txend_cnt_clr : 1;
    unsigned int rsvd_0 : 4;
  } b;
} TX_END_CHK_FIELD_T;

#define TX_ENABLE_CHK                             (WIFI_MAC_MAIN_REG_BASE + 0x1b0)
// Bit 25  :0      ro_txenable_cnt                U     RO        default = 'h0
// Bit 26          reg_txenable_cnt_en            U     RW        default = 'h0
// Bit 27          reg_txenable_cnt_clr           U     RW        default = 'h1
typedef union TX_ENABLE_CHK_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txenable_cnt : 26;
    unsigned int reg_txenable_cnt_en : 1;
    unsigned int reg_txenable_cnt_clr : 1;
    unsigned int rsvd_0 : 4;
  } b;
} TX_ENABLE_CHK_FIELD_T;

#define TX_ENABLEWR_CHK                           (WIFI_MAC_MAIN_REG_BASE + 0x1b4)
// Bit 25  :0      ro_txenable_wr_cnt_en          U     RO        default = 'h0
// Bit 26          reg_txenable_wr_cnt_en         U     RW        default = 'h0
// Bit 27          reg_txenable_wr_cnt_clr        U     RW        default = 'h1
typedef union TX_ENABLEWR_CHK_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txenable_wr_cnt_en : 26;
    unsigned int reg_txenable_wr_cnt_en : 1;
    unsigned int reg_txenable_wr_cnt_clr : 1;
    unsigned int rsvd_0 : 4;
  } b;
} TX_ENABLEWR_CHK_FIELD_T;

#define MAC_DATA_LOCAL_CNT                        (WIFI_MAC_MAIN_REG_BASE + 0x1b8)
// Bit 28  :0      ro_mac_data_local_cnt          U     RO        default = 'h0
// Bit 29          reg_mac_data_local_cnt_qos     U     RW        default = 'h0
// Bit 30          reg_mac_data_local_cnt_clr     U     RW        default = 'h0
// Bit 31          reg_mac_data_local_cnt_en      U     RW        default = 'h0
typedef union MAC_DATA_LOCAL_CNT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_data_local_cnt : 29;
    unsigned int reg_mac_data_local_cnt_qos : 1;
    unsigned int reg_mac_data_local_cnt_clr : 1;
    unsigned int reg_mac_data_local_cnt_en : 1;
  } b;
} MAC_DATA_LOCAL_CNT_FIELD_T;

#define MAC_DATA_OTHER_CNT                        (WIFI_MAC_MAIN_REG_BASE + 0x1bc)
// Bit 29  :0      ro_mac_data_other_cnt          U     RO        default = 'h0
// Bit 30          reg_mac_data_other_cnt_clr     U     RW        default = 'h0
// Bit 31          reg_mac_data_other_cnt_en      U     RW        default = 'h0
typedef union MAC_DATA_OTHER_CNT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_data_other_cnt : 30;
    unsigned int reg_mac_data_other_cnt_clr : 1;
    unsigned int reg_mac_data_other_cnt_en : 1;
  } b;
} MAC_DATA_OTHER_CNT_FIELD_T;

#define MAC_DATA_MULTI_CNT                        (WIFI_MAC_MAIN_REG_BASE + 0x1c0)
// Bit 29  :0      ro_mac_data_multi_cnt          U     RO        default = 'h0
// Bit 30          reg_mac_data_multi_cnt_clr     U     RW        default = 'h0
// Bit 31          reg_mac_data_multi_cnt_en      U     RW        default = 'h0
typedef union MAC_DATA_MULTI_CNT_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_mac_data_multi_cnt : 30;
    unsigned int reg_mac_data_multi_cnt_clr : 1;
    unsigned int reg_mac_data_multi_cnt_en : 1;
  } b;
} MAC_DATA_MULTI_CNT_FIELD_T;

#endif

