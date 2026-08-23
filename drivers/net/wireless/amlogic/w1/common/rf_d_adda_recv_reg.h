#ifdef RF_D_ADDA_RECV_REG
#else
#define RF_D_ADDA_RECV_REG


#define RF_D_ADDA_RECV_REG_BASE                   (0xa0e800)

#define RG_RECV_A0                                (RF_D_ADDA_RECV_REG_BASE + 0x0)
// Bit 31  :0      rg_recv_cfg0                   U     RW        default = 'h0
typedef union RG_RECV_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_recv_cfg0 : 32;
  } b;
} RG_RECV_A0_FIELD_T;

#define RG_RECV_A1                                (RF_D_ADDA_RECV_REG_BASE + 0x4)
// Bit 31  :0      rg_recv_cfg1                   U     RW        default = 'h0
typedef union RG_RECV_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_recv_cfg1 : 32;
  } b;
} RG_RECV_A1_FIELD_T;

#define RG_RECV_A2                                (RF_D_ADDA_RECV_REG_BASE + 0x8)
// Bit 0           rg_rxneg_msb                   U     RW        default = 'h0
// Bit 1           rg_rxiq_swap                   U     RW        default = 'h0
// Bit 11  :4      rg_rxi_dcoft                   U     RW        default = 'h0
// Bit 19  :12     rg_rxq_dcoft                   U     RW        default = 'h0
// Bit 20          rg_dcloop_bypass               U     RW        default = 'h1
// Bit 24          rg_rx_adc_fs                   U     RW        default = 'h1
// Bit 28          rg_to_agc_en                   U     RW        default = 'h0
typedef union RG_RECV_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxneg_msb : 1;
    unsigned int rg_rxiq_swap : 1;
    unsigned int rsvd_0 : 2;
    unsigned int rg_rxi_dcoft : 8;
    unsigned int rg_rxq_dcoft : 8;
    unsigned int rg_dcloop_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_adc_fs : 1;
    unsigned int rsvd_2 : 3;
    unsigned int rg_to_agc_en : 1;
    unsigned int rsvd_3 : 3;
  } b;
} RG_RECV_A2_FIELD_T;

#define RG_RECV_A3                                (RF_D_ADDA_RECV_REG_BASE + 0xc)
// Bit 14  :0      rg_rxirr_eup_0                 U     RW        default = 'h7f51
// Bit 30  :16     rg_rxirr_eup_1                 U     RW        default = 'h46
typedef union RG_RECV_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_eup_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_eup_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A3_FIELD_T;

#define RG_RECV_A4                                (RF_D_ADDA_RECV_REG_BASE + 0x10)
// Bit 14  :0      rg_rxirr_eup_2                 U     RW        default = 'h7fb7
// Bit 30  :16     rg_rxirr_eup_3                 U     RW        default = 'h17
typedef union RG_RECV_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_eup_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_eup_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A4_FIELD_T;

#define RG_RECV_A5                                (RF_D_ADDA_RECV_REG_BASE + 0x14)
// Bit 14  :0      rg_rxirr_pup_0                 U     RW        default = 'h5a4
// Bit 30  :16     rg_rxirr_pup_1                 U     RW        default = 'h7ff9
typedef union RG_RECV_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_pup_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_pup_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A5_FIELD_T;

#define RG_RECV_A6                                (RF_D_ADDA_RECV_REG_BASE + 0x18)
// Bit 14  :0      rg_rxirr_pup_2                 U     RW        default = 'h7ff6
// Bit 30  :16     rg_rxirr_pup_3                 U     RW        default = 'h7
typedef union RG_RECV_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_pup_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_pup_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A6_FIELD_T;

#define RG_RECV_A7                                (RF_D_ADDA_RECV_REG_BASE + 0x1c)
// Bit 14  :0      rg_rxirr_elow_0                U     RW        default = 'h7f7d
// Bit 30  :16     rg_rxirr_elow_1                U     RW        default = 'h7fba
typedef union RG_RECV_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_elow_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_elow_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A7_FIELD_T;

#define RG_RECV_A8                                (RF_D_ADDA_RECV_REG_BASE + 0x20)
// Bit 14  :0      rg_rxirr_elow_2                U     RW        default = 'h46
// Bit 30  :16     rg_rxirr_elow_3                U     RW        default = 'h7feb
typedef union RG_RECV_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_elow_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_elow_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A8_FIELD_T;

#define RG_RECV_A9                                (RF_D_ADDA_RECV_REG_BASE + 0x24)
// Bit 14  :0      rg_rxirr_plow_0                U     RW        default = 'h5a5
// Bit 30  :16     rg_rxirr_plow_1                U     RW        default = 'h7ffa
typedef union RG_RECV_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_plow_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_plow_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A9_FIELD_T;

#define RG_RECV_A10                               (RF_D_ADDA_RECV_REG_BASE + 0x28)
// Bit 14  :0      rg_rxirr_plow_2                U     RW        default = 'h7ff3
// Bit 30  :16     rg_rxirr_plow_3                U     RW        default = 'h8
typedef union RG_RECV_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_plow_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_plow_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A10_FIELD_T;

#define RG_RECV_A11                               (RF_D_ADDA_RECV_REG_BASE + 0x2c)
// Bit 0           rg_rxirr_man_mode              U     RW        default = 'h1
// Bit 4           rg_rxirr_bypass                U     RW        default = 'h1
// Bit 11  :8      rg_rx_async_wgap               U     RW        default = 'ha
typedef union RG_RECV_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_man_mode : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_rxirr_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_async_wgap : 4;
    unsigned int rsvd_2 : 20;
  } b;
} RG_RECV_A11_FIELD_T;

#define RG_RECV_A12                               (RF_D_ADDA_RECV_REG_BASE + 0x30)
// Bit 24  :0      rg_rxddc_angle                 U     RW        default = 'h0
// Bit 28          rg_rxddc_phs_reset             U     RW        default = 'h0
// Bit 31          rg_rxddc_bypass                U     RW        default = 'h1
typedef union RG_RECV_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxddc_angle : 25;
    unsigned int rsvd_0 : 3;
    unsigned int rg_rxddc_phs_reset : 1;
    unsigned int rsvd_1 : 2;
    unsigned int rg_rxddc_bypass : 1;
  } b;
} RG_RECV_A12_FIELD_T;

#define RG_RECV_A13                               (RF_D_ADDA_RECV_REG_BASE + 0x34)
// Bit 8   :0      rg_dc_sub_thr_pow              U     RW        default = 'h74
typedef union RG_RECV_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dc_sub_thr_pow : 9;
    unsigned int rsvd_0 : 23;
  } b;
} RG_RECV_A13_FIELD_T;

#define RG_RECV_A14                               (RF_D_ADDA_RECV_REG_BASE + 0x38)
// Bit 8   :0      rg_hdmi_sub_thr_pow1           U     RW        default = 'h70
// Bit 20  :12     rg_hdmi_sub_thr_pow2           U     RW        default = 'h68
typedef union RG_RECV_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_hdmi_sub_thr_pow1 : 9;
    unsigned int rsvd_0 : 3;
    unsigned int rg_hdmi_sub_thr_pow2 : 9;
    unsigned int rsvd_1 : 11;
  } b;
} RG_RECV_A14_FIELD_T;

#define RG_RECV_A15                               (RF_D_ADDA_RECV_REG_BASE + 0x3c)
// Bit 8   :0      rg_cw_sub_thr_pow1             U     RW        default = 'h78
// Bit 20  :12     rg_cw_sub_thr_pow2             U     RW        default = 'h66
typedef union RG_RECV_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_sub_thr_pow1 : 9;
    unsigned int rsvd_0 : 3;
    unsigned int rg_cw_sub_thr_pow2 : 9;
    unsigned int rsvd_1 : 11;
  } b;
} RG_RECV_A15_FIELD_T;

#define RG_RECV_A16                               (RF_D_ADDA_RECV_REG_BASE + 0x40)
// Bit 0           rg_cw_remove_sub_man_bypass0     U     RW        default = 'h0
// Bit 1           rg_cw_remove_sub_bypass_man_mode0     U     RW        default = 'h0
// Bit 4           rg_cw_remove_sub_man_bypass1     U     RW        default = 'h0
// Bit 5           rg_cw_remove_sub_bypass_man_mode1     U     RW        default = 'h0
// Bit 8           rg_cw_remove_sub_man_bypass2     U     RW        default = 'h0
// Bit 9           rg_cw_remove_sub_bypass_man_mode2     U     RW        default = 'h0
// Bit 12          rg_cw_remove_sub_man_bypass3     U     RW        default = 'h0
// Bit 13          rg_cw_remove_sub_bypass_man_mode3     U     RW        default = 'h0
// Bit 16          rg_cw_remove_sub_man_bypass4     U     RW        default = 'h0
// Bit 17          rg_cw_remove_sub_bypass_man_mode4     U     RW        default = 'h0
// Bit 20          rg_dc_bypass                   U     RW        default = 'h0
// Bit 24          rg_hdmi_bypass                 U     RW        default = 'h0
// Bit 28          rg_cw_bypass                   U     RW        default = 'h0
// Bit 30          rg_cw_remove_bypass_val        U     RW        default = 'h1
// Bit 31          rg_cw_remove_bypass_man        U     RW        default = 'h1
typedef union RG_RECV_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_man_bypass0 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode0 : 1;
    unsigned int rsvd_0 : 2;
    unsigned int rg_cw_remove_sub_man_bypass1 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode1 : 1;
    unsigned int rsvd_1 : 2;
    unsigned int rg_cw_remove_sub_man_bypass2 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode2 : 1;
    unsigned int rsvd_2 : 2;
    unsigned int rg_cw_remove_sub_man_bypass3 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode3 : 1;
    unsigned int rsvd_3 : 2;
    unsigned int rg_cw_remove_sub_man_bypass4 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode4 : 1;
    unsigned int rsvd_4 : 2;
    unsigned int rg_dc_bypass : 1;
    unsigned int rsvd_5 : 3;
    unsigned int rg_hdmi_bypass : 1;
    unsigned int rsvd_6 : 3;
    unsigned int rg_cw_bypass : 1;
    unsigned int rsvd_7 : 1;
    unsigned int rg_cw_remove_bypass_val : 1;
    unsigned int rg_cw_remove_bypass_man : 1;
  } b;
} RG_RECV_A16_FIELD_T;

#define RG_RECV_A17                               (RF_D_ADDA_RECV_REG_BASE + 0x44)
// Bit 24  :0      rg_cw_remove_sub_angle_base1     U     RW        default = 'h1d33333
typedef union RG_RECV_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base1 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A17_FIELD_T;

#define RG_RECV_A18                               (RF_D_ADDA_RECV_REG_BASE + 0x48)
// Bit 24  :0      rg_cw_remove_sub_angle_base2     U     RW        default = 'h800000
typedef union RG_RECV_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base2 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A18_FIELD_T;

#define RG_RECV_A19                               (RF_D_ADDA_RECV_REG_BASE + 0x4c)
// Bit 24  :0      rg_cw_remove_sub_angle_base3     U     RW        default = 'h1e00000
typedef union RG_RECV_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base3 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A19_FIELD_T;

#define RG_RECV_A20                               (RF_D_ADDA_RECV_REG_BASE + 0x50)
// Bit 24  :0      rg_cw_remove_sub_angle_base4     U     RW        default = 'he00000
typedef union RG_RECV_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base4 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A20_FIELD_T;

#define RG_RECV_A21                               (RF_D_ADDA_RECV_REG_BASE + 0x54)
// Bit 9   :0      rg_dc_dc_remove_cnt            U     RW        default = 'h3ff
// Bit 25  :16     rg_hdmi_dc_remove_cnt          U     RW        default = 'h3f
typedef union RG_RECV_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dc_dc_remove_cnt : 10;
    unsigned int rsvd_0 : 6;
    unsigned int rg_hdmi_dc_remove_cnt : 10;
    unsigned int rsvd_1 : 6;
  } b;
} RG_RECV_A21_FIELD_T;

#define RG_RECV_A22                               (RF_D_ADDA_RECV_REG_BASE + 0x58)
// Bit 9   :0      rg_cw_dc_remove_cnt            U     RW        default = 'h3f
typedef union RG_RECV_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_dc_remove_cnt : 10;
    unsigned int rsvd_0 : 22;
  } b;
} RG_RECV_A22_FIELD_T;

#define RG_RECV_A23                               (RF_D_ADDA_RECV_REG_BASE + 0x5c)
// Bit 0           rg_cali_cw_rmv_bypass          U     RW        default = 'h1
// Bit 4           rg_agc_cw_rmv_bypass           U     RW        default = 'h1
typedef union RG_RECV_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cali_cw_rmv_bypass : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_agc_cw_rmv_bypass : 1;
    unsigned int rsvd_1 : 27;
  } b;
} RG_RECV_A23_FIELD_T;

#define RG_RECV_A24                               (RF_D_ADDA_RECV_REG_BASE + 0x60)
// Bit 8   :0      rg_cw_remove_gaindb_val        U     RW        default = 'h84
// Bit 12          rg_cw_remove_gaindb_man        U     RW        default = 'h0
typedef union RG_RECV_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_gaindb_val : 9;
    unsigned int rsvd_0 : 3;
    unsigned int rg_cw_remove_gaindb_man : 1;
    unsigned int rsvd_1 : 19;
  } b;
} RG_RECV_A24_FIELD_T;

#define RG_RECV_A25                               (RF_D_ADDA_RECV_REG_BASE + 0x64)
// Bit 0           rg_rssi_snr_est                U     RW        default = 'h0
typedef union RG_RECV_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rssi_snr_est : 1;
    unsigned int rsvd_0 : 31;
  } b;
} RG_RECV_A25_FIELD_T;

#endif

