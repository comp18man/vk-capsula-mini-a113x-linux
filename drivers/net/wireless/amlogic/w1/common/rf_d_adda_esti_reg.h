#ifdef RF_D_ADDA_ESTI_REG
#else
#define RF_D_ADDA_ESTI_REG


#define RF_D_ADDA_ESTI_REG_BASE                   (0xa0ec00)

#define RG_ESTI_A0                                (RF_D_ADDA_ESTI_REG_BASE + 0x0)
// Bit 31  :0      rg_esti_cfg0                   U     RW        default = 'h0
typedef union RG_ESTI_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_esti_cfg0 : 32;
  } b;
} RG_ESTI_A0_FIELD_T;

#define RG_ESTI_A1                                (RF_D_ADDA_ESTI_REG_BASE + 0x4)
// Bit 31  :0      rg_esti_cfg1                   U     RW        default = 'h0
typedef union RG_ESTI_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_esti_cfg1 : 32;
  } b;
} RG_ESTI_A1_FIELD_T;

#define RG_ESTI_A3                                (RF_D_ADDA_ESTI_REG_BASE + 0xc)
// Bit 14  :0      ro_rxirr_eup_0                 U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_eup_1                 U     RO        default = 'h0
typedef union RG_ESTI_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_eup_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_eup_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A3_FIELD_T;

#define RG_ESTI_A4                                (RF_D_ADDA_ESTI_REG_BASE + 0x10)
// Bit 14  :0      ro_rxirr_eup_2                 U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_eup_3                 U     RO        default = 'h0
typedef union RG_ESTI_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_eup_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_eup_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A4_FIELD_T;

#define RG_ESTI_A5                                (RF_D_ADDA_ESTI_REG_BASE + 0x14)
// Bit 14  :0      ro_rxirr_pup_0                 U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_pup_1                 U     RO        default = 'h0
typedef union RG_ESTI_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_pup_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_pup_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A5_FIELD_T;

#define RG_ESTI_A6                                (RF_D_ADDA_ESTI_REG_BASE + 0x18)
// Bit 14  :0      ro_rxirr_pup_2                 U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_pup_3                 U     RO        default = 'h0
typedef union RG_ESTI_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_pup_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_pup_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A6_FIELD_T;

#define RG_ESTI_A7                                (RF_D_ADDA_ESTI_REG_BASE + 0x1c)
// Bit 14  :0      ro_rxirr_elow_0                U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_elow_1                U     RO        default = 'h0
typedef union RG_ESTI_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_elow_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_elow_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A7_FIELD_T;

#define RG_ESTI_A8                                (RF_D_ADDA_ESTI_REG_BASE + 0x20)
// Bit 14  :0      ro_rxirr_elow_2                U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_elow_3                U     RO        default = 'h0
typedef union RG_ESTI_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_elow_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_elow_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A8_FIELD_T;

#define RG_ESTI_A9                                (RF_D_ADDA_ESTI_REG_BASE + 0x24)
// Bit 14  :0      ro_rxirr_plow_0                U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_plow_1                U     RO        default = 'h0
typedef union RG_ESTI_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_plow_0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_plow_1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A9_FIELD_T;

#define RG_ESTI_A10                               (RF_D_ADDA_ESTI_REG_BASE + 0x28)
// Bit 14  :0      ro_rxirr_plow_2                U     RO        default = 'h0
// Bit 30  :16     ro_rxirr_plow_3                U     RO        default = 'h0
typedef union RG_ESTI_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_plow_2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int ro_rxirr_plow_3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_ESTI_A10_FIELD_T;

#define RG_ESTI_A11                               (RF_D_ADDA_ESTI_REG_BASE + 0x2c)
// Bit 0           rg_iqib_enable                 U     RW        default = 'h0
// Bit 1           rg_iqib_hold                   U     RW        default = 'h0
// Bit 7   :4      rg_iqib_period                 U     RW        default = 'h0
// Bit 11  :8      rg_iqib_step_a                 U     RW        default = 'h1
// Bit 15  :12     rg_iqib_step_b                 U     RW        default = 'h1
// Bit 18  :16     rg_iqib_shift                  U     RW        default = 'h0
// Bit 20          rg_iqib_man_en                 U     RW        default = 'h0
// Bit 24          rg_rxirr_read_response         U     RW        default = 'h0
// Bit 28          rg_rxirr_read_response_bypass     U     RW        default = 'h0
// Bit 31          ro_rxirr_ready                 U     RO        default = 'h0
typedef union RG_ESTI_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_iqib_enable : 1;
    unsigned int rg_iqib_hold : 1;
    unsigned int rsvd_0 : 2;
    unsigned int rg_iqib_period : 4;
    unsigned int rg_iqib_step_a : 4;
    unsigned int rg_iqib_step_b : 4;
    unsigned int rg_iqib_shift : 3;
    unsigned int rsvd_1 : 1;
    unsigned int rg_iqib_man_en : 1;
    unsigned int rsvd_2 : 3;
    unsigned int rg_rxirr_read_response : 1;
    unsigned int rsvd_3 : 3;
    unsigned int rg_rxirr_read_response_bypass : 1;
    unsigned int rsvd_4 : 2;
    unsigned int ro_rxirr_ready : 1;
  } b;
} RG_ESTI_A11_FIELD_T;

#define RG_ESTI_A12                               (RF_D_ADDA_ESTI_REG_BASE + 0x30)
// Bit 21  :0      rg_set_b                       U     RW        default = 'h0
// Bit 31          rg_set_val                     U     RW        default = 'h0
typedef union RG_ESTI_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_set_b : 22;
    unsigned int rsvd_0 : 9;
    unsigned int rg_set_val : 1;
  } b;
} RG_ESTI_A12_FIELD_T;

#define RG_ESTI_A13                               (RF_D_ADDA_ESTI_REG_BASE + 0x34)
// Bit 18  :0      rg_set_a                       U     RW        default = 'h0
typedef union RG_ESTI_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_set_a : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_ESTI_A13_FIELD_T;

#define RG_ESTI_A14                               (RF_D_ADDA_ESTI_REG_BASE + 0x38)
// Bit 3   :0      rg_dc_rm_leaky_factor          U     RW        default = 'h0
// Bit 28  :4      rg_spectrum_ana_angle_man      U     RW        default = 'h0
// Bit 31          rg_spectrum_ana_angle_man_en     U     RW        default = 'h0
typedef union RG_ESTI_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dc_rm_leaky_factor : 4;
    unsigned int rg_spectrum_ana_angle_man : 25;
    unsigned int rsvd_0 : 2;
    unsigned int rg_spectrum_ana_angle_man_en : 1;
  } b;
} RG_ESTI_A14_FIELD_T;

#define RG_ESTI_A15                               (RF_D_ADDA_ESTI_REG_BASE + 0x3c)
// Bit 31  :0      rg_esti_cfg2                   U     RW        default = 'h0
typedef union RG_ESTI_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_esti_cfg2 : 32;
  } b;
} RG_ESTI_A15_FIELD_T;

#define RG_ESTI_A16                               (RF_D_ADDA_ESTI_REG_BASE + 0x40)
// Bit 31  :0      rg_esti_cfg3                   U     RW        default = 'h0
typedef union RG_ESTI_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_esti_cfg3 : 32;
  } b;
} RG_ESTI_A16_FIELD_T;

#define RG_ESTI_A17                               (RF_D_ADDA_ESTI_REG_BASE + 0x44)
// Bit 6   :0      ro_dcoc_code_i                 U     RO        default = 'h0
// Bit 14  :8      ro_dcoc_code_q                 U     RO        default = 'h0
// Bit 16          ro_dcoc_code_ready             U     RO        default = 'h0
// Bit 24          ro_dcoc_finish                 U     RO        default = 'h0
// Bit 28          rg_dcoc_read_response_bypass     U     RW        default = 'h0
// Bit 31          rg_dcoc_read_response          U     RW        default = 'h0
typedef union RG_ESTI_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_dcoc_code_i : 7;
    unsigned int rsvd_0 : 1;
    unsigned int ro_dcoc_code_q : 7;
    unsigned int rsvd_1 : 1;
    unsigned int ro_dcoc_code_ready : 1;
    unsigned int rsvd_2 : 7;
    unsigned int ro_dcoc_finish : 1;
    unsigned int rsvd_3 : 3;
    unsigned int rg_dcoc_read_response_bypass : 1;
    unsigned int rsvd_4 : 2;
    unsigned int rg_dcoc_read_response : 1;
  } b;
} RG_ESTI_A17_FIELD_T;

#define RG_ESTI_A18                               (RF_D_ADDA_ESTI_REG_BASE + 0x48)
// Bit 3   :0      ro_lnatank_esti_code           U     RO        default = 'h0
// Bit 8           ro_lnatank_esti_code_ready     U     RO        default = 'h0
// Bit 16          ro_lnatank_esti_finish         U     RO        default = 'h0
// Bit 28          rg_lnatank_read_response_bypass     U     RW        default = 'h0
// Bit 31          rg_lnatank_read_response       U     RW        default = 'h0
typedef union RG_ESTI_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_lnatank_esti_code : 4;
    unsigned int rsvd_0 : 4;
    unsigned int ro_lnatank_esti_code_ready : 1;
    unsigned int rsvd_1 : 7;
    unsigned int ro_lnatank_esti_finish : 1;
    unsigned int rsvd_2 : 11;
    unsigned int rg_lnatank_read_response_bypass : 1;
    unsigned int rsvd_3 : 2;
    unsigned int rg_lnatank_read_response : 1;
  } b;
} RG_ESTI_A18_FIELD_T;

#define RG_ESTI_A19                               (RF_D_ADDA_ESTI_REG_BASE + 0x4c)
// Bit 7   :0      ro_txirr_dc_i                  U     RO        default = 'h0
// Bit 19  :12     ro_txirr_dc_q                  U     RO        default = 'h0
// Bit 24          ro_txirr_dc_ready              U     RO        default = 'h0
// Bit 25          ro_txirr_code_ready            U     RO        default = 'h0
// Bit 26          ro_txirr_mode_ready            U     RO        default = 'h0
// Bit 28          rg_txirr_code_continue         U     RW        default = 'h0
// Bit 29          rg_txirr_code_continue_bypass     U     RW        default = 'h1
// Bit 30          rg_txirr_mode_continue         U     RW        default = 'h0
// Bit 31          rg_txirr_mode_continue_bypass     U     RW        default = 'h1
typedef union RG_ESTI_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txirr_dc_i : 8;
    unsigned int rsvd_0 : 4;
    unsigned int ro_txirr_dc_q : 8;
    unsigned int rsvd_1 : 4;
    unsigned int ro_txirr_dc_ready : 1;
    unsigned int ro_txirr_code_ready : 1;
    unsigned int ro_txirr_mode_ready : 1;
    unsigned int rsvd_2 : 1;
    unsigned int rg_txirr_code_continue : 1;
    unsigned int rg_txirr_code_continue_bypass : 1;
    unsigned int rg_txirr_mode_continue : 1;
    unsigned int rg_txirr_mode_continue_bypass : 1;
  } b;
} RG_ESTI_A19_FIELD_T;

#define RG_ESTI_A20                               (RF_D_ADDA_ESTI_REG_BASE + 0x50)
// Bit 7   :0      ro_txirr_alpha                 U     RO        default = 'h0
// Bit 19  :12     ro_txirr_theta                 U     RO        default = 'h0
// Bit 24          ro_txirr_irr_ready             U     RO        default = 'h0
// Bit 28          rg_txirr_read_response_bypass     U     RW        default = 'h0
// Bit 31          rg_txirr_read_response         U     RW        default = 'h0
typedef union RG_ESTI_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txirr_alpha : 8;
    unsigned int rsvd_0 : 4;
    unsigned int ro_txirr_theta : 8;
    unsigned int rsvd_1 : 4;
    unsigned int ro_txirr_irr_ready : 1;
    unsigned int rsvd_2 : 3;
    unsigned int rg_txirr_read_response_bypass : 1;
    unsigned int rsvd_3 : 2;
    unsigned int rg_txirr_read_response : 1;
  } b;
} RG_ESTI_A20_FIELD_T;

#define RG_ESTI_A21                               (RF_D_ADDA_ESTI_REG_BASE + 0x54)
// Bit 7   :0      rg_tx_dcmax_i                  U     RW        default = 'h33
// Bit 15  :8      rg_tx_dcmax_q                  U     RW        default = 'h33
// Bit 23  :16     rg_tx_alpha_max                U     RW        default = 'h7b
// Bit 31  :24     rg_tx_theta_max                U     RW        default = 'h6b
typedef union RG_ESTI_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_tx_dcmax_i : 8;
    unsigned int rg_tx_dcmax_q : 8;
    unsigned int rg_tx_alpha_max : 8;
    unsigned int rg_tx_theta_max : 8;
  } b;
} RG_ESTI_A21_FIELD_T;

#define RG_ESTI_A22                               (RF_D_ADDA_ESTI_REG_BASE + 0x58)
// Bit 7   :0      rg_tx_dcmin_i                  U     RW        default = 'hcd
// Bit 15  :8      rg_tx_dcmin_q                  U     RW        default = 'hcd
// Bit 23  :16     rg_tx_alpha_min                U     RW        default = 'h85
// Bit 31  :24     rg_tx_theta_min                U     RW        default = 'h95
typedef union RG_ESTI_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_tx_dcmin_i : 8;
    unsigned int rg_tx_dcmin_q : 8;
    unsigned int rg_tx_alpha_min : 8;
    unsigned int rg_tx_theta_min : 8;
  } b;
} RG_ESTI_A22_FIELD_T;

#define RG_ESTI_A23                               (RF_D_ADDA_ESTI_REG_BASE + 0x5c)
// Bit 3   :0      rg_tx_dc_i_step                U     RW        default = 'h5
// Bit 7   :4      rg_tx_dc_q_step                U     RW        default = 'h5
// Bit 11  :8      rg_tx_alpha_step               U     RW        default = 'h3
// Bit 15  :12     rg_tx_theta_step               U     RW        default = 'h4
// Bit 19  :16     rg_txirr_oper_bnd              U     RW        default = 'h1
typedef union RG_ESTI_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_tx_dc_i_step : 4;
    unsigned int rg_tx_dc_q_step : 4;
    unsigned int rg_tx_alpha_step : 4;
    unsigned int rg_tx_theta_step : 4;
    unsigned int rg_txirr_oper_bnd : 4;
    unsigned int rsvd_0 : 12;
  } b;
} RG_ESTI_A23_FIELD_T;

#define RG_ESTI_A24                               (RF_D_ADDA_ESTI_REG_BASE + 0x60)
// Bit 24  :0      rg_spectrum_ana_angle_dcoc     U     RW        default = 'h0
typedef union RG_ESTI_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_spectrum_ana_angle_dcoc : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A24_FIELD_T;

#define RG_ESTI_A25                               (RF_D_ADDA_ESTI_REG_BASE + 0x64)
// Bit 24  :0      rg_spectrum_ana_angle_lna      U     RW        default = 'h1f99999
typedef union RG_ESTI_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_spectrum_ana_angle_lna : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A25_FIELD_T;

#define RG_ESTI_A26                               (RF_D_ADDA_ESTI_REG_BASE + 0x68)
// Bit 24  :0      rg_spectrum_ana_angle_txirr     U     RW        default = 'h1c00000
typedef union RG_ESTI_A26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_spectrum_ana_angle_txirr : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A26_FIELD_T;

#define RG_ESTI_A27                               (RF_D_ADDA_ESTI_REG_BASE + 0x6c)
// Bit 24  :0      rg_spectrum_ana_angle_txdc     U     RW        default = 'h1e00000
typedef union RG_ESTI_A27_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_spectrum_ana_angle_txdc : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A27_FIELD_T;

#define RG_ESTI_A28                               (RF_D_ADDA_ESTI_REG_BASE + 0x70)
// Bit 24  :0      rg_spectrum_ana_angle_dpd      U     RW        default = 'h0
typedef union RG_ESTI_A28_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_spectrum_ana_angle_dpd : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A28_FIELD_T;

#define RG_ESTI_A29                               (RF_D_ADDA_ESTI_REG_BASE + 0x74)
// Bit 24  :0      rg_spectrum_ana_angle_pwc      U     RW        default = 'h0
typedef union RG_ESTI_A29_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_spectrum_ana_angle_pwc : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A29_FIELD_T;

#define RG_ESTI_A30                               (RF_D_ADDA_ESTI_REG_BASE + 0x78)
// Bit 24  :0      rg_irr_tone_freq_0             U     RW        default = 'h4ccccc
typedef union RG_ESTI_A30_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_0 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A30_FIELD_T;

#define RG_ESTI_A31                               (RF_D_ADDA_ESTI_REG_BASE + 0x7c)
// Bit 24  :0      rg_irr_tone_freq_1             U     RW        default = 'ha66666
typedef union RG_ESTI_A31_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_1 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A31_FIELD_T;

#define RG_ESTI_A32                               (RF_D_ADDA_ESTI_REG_BASE + 0x80)
// Bit 24  :0      rg_irr_tone_freq_2             U     RW        default = 'hd33333
typedef union RG_ESTI_A32_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_2 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A32_FIELD_T;

#define RG_ESTI_A33                               (RF_D_ADDA_ESTI_REG_BASE + 0x84)
// Bit 24  :0      rg_irr_tone_freq_3             U     RW        default = 'hf99999
typedef union RG_ESTI_A33_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_3 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A33_FIELD_T;

#define RG_ESTI_A34                               (RF_D_ADDA_ESTI_REG_BASE + 0x88)
// Bit 24  :0      rg_irr_tone_freq_4             U     RW        default = 'h1066666
typedef union RG_ESTI_A34_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_4 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A34_FIELD_T;

#define RG_ESTI_A35                               (RF_D_ADDA_ESTI_REG_BASE + 0x8c)
// Bit 24  :0      rg_irr_tone_freq_5             U     RW        default = 'h12ccccc
typedef union RG_ESTI_A35_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_5 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A35_FIELD_T;

#define RG_ESTI_A36                               (RF_D_ADDA_ESTI_REG_BASE + 0x90)
// Bit 24  :0      rg_irr_tone_freq_6             U     RW        default = 'h1599999
typedef union RG_ESTI_A36_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_6 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A36_FIELD_T;

#define RG_ESTI_A37                               (RF_D_ADDA_ESTI_REG_BASE + 0x94)
// Bit 24  :0      rg_irr_tone_freq_7             U     RW        default = 'h1b33333
typedef union RG_ESTI_A37_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_tone_freq_7 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_ESTI_A37_FIELD_T;

#define RG_ESTI_A38                               (RF_D_ADDA_ESTI_REG_BASE + 0x98)
// Bit 15  :0      rg_irr_interp_coef_up0         U     RW        default = 'hf751
// Bit 31  :16     rg_irr_interp_coef_up1         U     RW        default = 'h97e
typedef union RG_ESTI_A38_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_up0 : 16;
    unsigned int rg_irr_interp_coef_up1 : 16;
  } b;
} RG_ESTI_A38_FIELD_T;

#define RG_ESTI_A39                               (RF_D_ADDA_ESTI_REG_BASE + 0x9c)
// Bit 15  :0      rg_irr_interp_coef_up2         U     RW        default = 'hfbb7
// Bit 31  :16     rg_irr_interp_coef_up3         U     RW        default = 'h292
typedef union RG_ESTI_A39_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_up2 : 16;
    unsigned int rg_irr_interp_coef_up3 : 16;
  } b;
} RG_ESTI_A39_FIELD_T;

#define RG_ESTI_A40                               (RF_D_ADDA_ESTI_REG_BASE + 0xa0)
// Bit 15  :0      rg_irr_interp_coef_up4         U     RW        default = 'h369c
// Bit 31  :16     rg_irr_interp_coef_up5         U     RW        default = 'hd196
typedef union RG_ESTI_A40_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_up4 : 16;
    unsigned int rg_irr_interp_coef_up5 : 16;
  } b;
} RG_ESTI_A40_FIELD_T;

#define RG_ESTI_A41                               (RF_D_ADDA_ESTI_REG_BASE + 0xa4)
// Bit 15  :0      rg_irr_interp_coef_up6         U     RW        default = 'h1ae1
// Bit 31  :16     rg_irr_interp_coef_up7         U     RW        default = 'hf4d1
typedef union RG_ESTI_A41_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_up6 : 16;
    unsigned int rg_irr_interp_coef_up7 : 16;
  } b;
} RG_ESTI_A41_FIELD_T;

#define RG_ESTI_A42                               (RF_D_ADDA_ESTI_REG_BASE + 0xa8)
// Bit 15  :0      rg_irr_interp_coef_up8         U     RW        default = 'had86
// Bit 31  :16     rg_irr_interp_coef_up9         U     RW        default = 'h4663
typedef union RG_ESTI_A42_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_up8 : 16;
    unsigned int rg_irr_interp_coef_up9 : 16;
  } b;
} RG_ESTI_A42_FIELD_T;

#define RG_ESTI_A43                               (RF_D_ADDA_ESTI_REG_BASE + 0xac)
// Bit 15  :0      rg_irr_interp_coef_upa         U     RW        default = 'hd7c6
// Bit 31  :16     rg_irr_interp_coef_upb         U     RW        default = 'hdd8
typedef union RG_ESTI_A43_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_upa : 16;
    unsigned int rg_irr_interp_coef_upb : 16;
  } b;
} RG_ESTI_A43_FIELD_T;

#define RG_ESTI_A44                               (RF_D_ADDA_ESTI_REG_BASE + 0xb0)
// Bit 15  :0      rg_irr_interp_coef_upc         U     RW        default = 'h288d
// Bit 31  :16     rg_irr_interp_coef_upd         U     RW        default = 'hde88
typedef union RG_ESTI_A44_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_upc : 16;
    unsigned int rg_irr_interp_coef_upd : 16;
  } b;
} RG_ESTI_A44_FIELD_T;

#define RG_ESTI_A45                               (RF_D_ADDA_ESTI_REG_BASE + 0xb4)
// Bit 15  :0      rg_irr_interp_coef_upe         U     RW        default = 'h11a2
// Bit 31  :16     rg_irr_interp_coef_upf         U     RW        default = 'hfac5
typedef union RG_ESTI_A45_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_upe : 16;
    unsigned int rg_irr_interp_coef_upf : 16;
  } b;
} RG_ESTI_A45_FIELD_T;

#define RG_ESTI_A46                               (RF_D_ADDA_ESTI_REG_BASE + 0xb8)
// Bit 15  :0      rg_irr_interp_coef_low0        U     RW        default = 'h288d
// Bit 31  :16     rg_irr_interp_coef_low1        U     RW        default = 'hde88
typedef union RG_ESTI_A46_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_low0 : 16;
    unsigned int rg_irr_interp_coef_low1 : 16;
  } b;
} RG_ESTI_A46_FIELD_T;

#define RG_ESTI_A47                               (RF_D_ADDA_ESTI_REG_BASE + 0xbc)
// Bit 15  :0      rg_irr_interp_coef_low2        U     RW        default = 'h11a2
// Bit 31  :16     rg_irr_interp_coef_low3        U     RW        default = 'hfac5
typedef union RG_ESTI_A47_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_low2 : 16;
    unsigned int rg_irr_interp_coef_low3 : 16;
  } b;
} RG_ESTI_A47_FIELD_T;

#define RG_ESTI_A48                               (RF_D_ADDA_ESTI_REG_BASE + 0xc0)
// Bit 15  :0      rg_irr_interp_coef_low4        U     RW        default = 'had86
// Bit 31  :16     rg_irr_interp_coef_low5        U     RW        default = 'h4663
typedef union RG_ESTI_A48_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_low4 : 16;
    unsigned int rg_irr_interp_coef_low5 : 16;
  } b;
} RG_ESTI_A48_FIELD_T;

#define RG_ESTI_A49                               (RF_D_ADDA_ESTI_REG_BASE + 0xc4)
// Bit 15  :0      rg_irr_interp_coef_low6        U     RW        default = 'hd7c6
// Bit 31  :16     rg_irr_interp_coef_low7        U     RW        default = 'hdd8
typedef union RG_ESTI_A49_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_low6 : 16;
    unsigned int rg_irr_interp_coef_low7 : 16;
  } b;
} RG_ESTI_A49_FIELD_T;

#define RG_ESTI_A50                               (RF_D_ADDA_ESTI_REG_BASE + 0xc8)
// Bit 15  :0      rg_irr_interp_coef_low8        U     RW        default = 'h369c
// Bit 31  :16     rg_irr_interp_coef_low9        U     RW        default = 'hd196
typedef union RG_ESTI_A50_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_low8 : 16;
    unsigned int rg_irr_interp_coef_low9 : 16;
  } b;
} RG_ESTI_A50_FIELD_T;

#define RG_ESTI_A51                               (RF_D_ADDA_ESTI_REG_BASE + 0xcc)
// Bit 15  :0      rg_irr_interp_coef_lowa        U     RW        default = 'h1ae1
// Bit 31  :16     rg_irr_interp_coef_lowb        U     RW        default = 'hf4d1
typedef union RG_ESTI_A51_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_lowa : 16;
    unsigned int rg_irr_interp_coef_lowb : 16;
  } b;
} RG_ESTI_A51_FIELD_T;

#define RG_ESTI_A52                               (RF_D_ADDA_ESTI_REG_BASE + 0xd0)
// Bit 15  :0      rg_irr_interp_coef_lowc        U     RW        default = 'hf751
// Bit 31  :16     rg_irr_interp_coef_lowd        U     RW        default = 'h97e
typedef union RG_ESTI_A52_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_lowc : 16;
    unsigned int rg_irr_interp_coef_lowd : 16;
  } b;
} RG_ESTI_A52_FIELD_T;

#define RG_ESTI_A53                               (RF_D_ADDA_ESTI_REG_BASE + 0xd4)
// Bit 15  :0      rg_irr_interp_coef_lowe        U     RW        default = 'hfbb7
// Bit 31  :16     rg_irr_interp_coef_lowf        U     RW        default = 'h292
typedef union RG_ESTI_A53_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_irr_interp_coef_lowe : 16;
    unsigned int rg_irr_interp_coef_lowf : 16;
  } b;
} RG_ESTI_A53_FIELD_T;

#define RG_ESTI_A54                               (RF_D_ADDA_ESTI_REG_BASE + 0xd8)
// Bit 3   :0      rg_rxirr_esti_len_avg_0        U     RW        default = 'h2
// Bit 7   :4      rg_rxirr_esti_miu_0            U     RW        default = 'h0
// Bit 15  :8      rg_rxirr_esti_navg_0           U     RW        default = 'h3
// Bit 19  :16     rg_rxirr_esti_len_avg_1        U     RW        default = 'h3
// Bit 23  :20     rg_rxirr_esti_miu_1            U     RW        default = 'h1
// Bit 31  :24     rg_rxirr_esti_navg_1           U     RW        default = 'h7
typedef union RG_ESTI_A54_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_esti_len_avg_0 : 4;
    unsigned int rg_rxirr_esti_miu_0 : 4;
    unsigned int rg_rxirr_esti_navg_0 : 8;
    unsigned int rg_rxirr_esti_len_avg_1 : 4;
    unsigned int rg_rxirr_esti_miu_1 : 4;
    unsigned int rg_rxirr_esti_navg_1 : 8;
  } b;
} RG_ESTI_A54_FIELD_T;

#define RG_ESTI_A55                               (RF_D_ADDA_ESTI_REG_BASE + 0xdc)
// Bit 3   :0      rg_rxirr_esti_len_avg_2        U     RW        default = 'h4
// Bit 7   :4      rg_rxirr_esti_miu_2            U     RW        default = 'h2
// Bit 15  :8      rg_rxirr_esti_navg_2           U     RW        default = 'hb
// Bit 19  :16     rg_rxirr_esti_len_avg_3        U     RW        default = 'h4
// Bit 23  :20     rg_rxirr_esti_miu_3            U     RW        default = 'h3
// Bit 31  :24     rg_rxirr_esti_navg_3           U     RW        default = 'h13
typedef union RG_ESTI_A55_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_esti_len_avg_2 : 4;
    unsigned int rg_rxirr_esti_miu_2 : 4;
    unsigned int rg_rxirr_esti_navg_2 : 8;
    unsigned int rg_rxirr_esti_len_avg_3 : 4;
    unsigned int rg_rxirr_esti_miu_3 : 4;
    unsigned int rg_rxirr_esti_navg_3 : 8;
  } b;
} RG_ESTI_A55_FIELD_T;

#define RG_ESTI_A56                               (RF_D_ADDA_ESTI_REG_BASE + 0xe0)
// Bit 13  :0      ro_rxirr_interp_e_0            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_e_1            U     RO        default = 'h0
typedef union RG_ESTI_A56_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_e_0 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_e_1 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A56_FIELD_T;

#define RG_ESTI_A57                               (RF_D_ADDA_ESTI_REG_BASE + 0xe4)
// Bit 13  :0      ro_rxirr_interp_e_2            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_e_3            U     RO        default = 'h0
typedef union RG_ESTI_A57_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_e_2 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_e_3 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A57_FIELD_T;

#define RG_ESTI_A58                               (RF_D_ADDA_ESTI_REG_BASE + 0xe8)
// Bit 13  :0      ro_rxirr_interp_e_4            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_e_5            U     RO        default = 'h0
typedef union RG_ESTI_A58_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_e_4 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_e_5 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A58_FIELD_T;

#define RG_ESTI_A59                               (RF_D_ADDA_ESTI_REG_BASE + 0xec)
// Bit 13  :0      ro_rxirr_interp_e_6            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_e_7            U     RO        default = 'h0
typedef union RG_ESTI_A59_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_e_6 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_e_7 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A59_FIELD_T;

#define RG_ESTI_A60                               (RF_D_ADDA_ESTI_REG_BASE + 0xf0)
// Bit 13  :0      ro_rxirr_interp_p_0            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_p_1            U     RO        default = 'h0
typedef union RG_ESTI_A60_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_p_0 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_p_1 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A60_FIELD_T;

#define RG_ESTI_A61                               (RF_D_ADDA_ESTI_REG_BASE + 0xf4)
// Bit 13  :0      ro_rxirr_interp_p_2            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_p_3            U     RO        default = 'h0
typedef union RG_ESTI_A61_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_p_2 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_p_3 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A61_FIELD_T;

#define RG_ESTI_A62                               (RF_D_ADDA_ESTI_REG_BASE + 0xf8)
// Bit 13  :0      ro_rxirr_interp_p_4            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_p_5            U     RO        default = 'h0
typedef union RG_ESTI_A62_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_p_4 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_p_5 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A62_FIELD_T;

#define RG_ESTI_A63                               (RF_D_ADDA_ESTI_REG_BASE + 0xfc)
// Bit 13  :0      ro_rxirr_interp_p_6            U     RO        default = 'h0
// Bit 29  :16     ro_rxirr_interp_p_7            U     RO        default = 'h0
typedef union RG_ESTI_A63_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rxirr_interp_p_6 : 14;
    unsigned int rsvd_0 : 2;
    unsigned int ro_rxirr_interp_p_7 : 14;
    unsigned int rsvd_1 : 2;
  } b;
} RG_ESTI_A63_FIELD_T;

#define RG_ESTI_A64                               (RF_D_ADDA_ESTI_REG_BASE + 0x100)
// Bit 23  :0      rg_adda_wait_count             U     RW        default = 'h50
typedef union RG_ESTI_A64_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_adda_wait_count : 24;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A64_FIELD_T;

#define RG_ESTI_A65                               (RF_D_ADDA_ESTI_REG_BASE + 0x104)
// Bit 14  :0      rg_adda_calc_count             U     RW        default = 'h50
typedef union RG_ESTI_A65_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_adda_calc_count : 15;
    unsigned int rsvd_0 : 17;
  } b;
} RG_ESTI_A65_FIELD_T;

#define RG_ESTI_A66                               (RF_D_ADDA_ESTI_REG_BASE + 0x108)
// Bit 27  :0      rg_adda_esti_count             U     RW        default = 'h40000
typedef union RG_ESTI_A66_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_adda_esti_count : 28;
    unsigned int rsvd_0 : 4;
  } b;
} RG_ESTI_A66_FIELD_T;

#define RG_ESTI_A67                               (RF_D_ADDA_ESTI_REG_BASE + 0x10c)
// Bit 0           rg_txpwc_read_response         U     RW        default = 'h0
// Bit 4           rg_txpwc_read_response_bypass     U     RW        default = 'h1
// Bit 16  :8      rg_txpwc_ph                    U     RW        default = 'h0
// Bit 28  :20     rg_txpwc_pl                    U     RW        default = 'h0
typedef union RG_ESTI_A67_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_txpwc_read_response : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_txpwc_read_response_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_txpwc_ph : 9;
    unsigned int rsvd_2 : 3;
    unsigned int rg_txpwc_pl : 9;
    unsigned int rsvd_3 : 3;
  } b;
} RG_ESTI_A67_FIELD_T;

#define RG_ESTI_A68                               (RF_D_ADDA_ESTI_REG_BASE + 0x110)
// Bit 7   :0      rg_txpwc_vh_amp                U     RW        default = 'h0
// Bit 15  :8      rg_txpwc_vl_amp                U     RW        default = 'h0
// Bit 25  :24     rg_txpwc_dc_cnt                U     RW        default = 'h0
typedef union RG_ESTI_A68_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_txpwc_vh_amp : 8;
    unsigned int rg_txpwc_vl_amp : 8;
    unsigned int rsvd_0 : 8;
    unsigned int rg_txpwc_dc_cnt : 2;
    unsigned int rsvd_1 : 6;
  } b;
} RG_ESTI_A68_FIELD_T;

#define RG_ESTI_A69                               (RF_D_ADDA_ESTI_REG_BASE + 0x114)
// Bit 8   :0      ro_txpwc_slope                 U     RO        default = 'h0
// Bit 26  :12     ro_txpwc_offset                U     RO        default = 'h0
// Bit 31          ro_txpwc_ready                 U     RO        default = 'h0
typedef union RG_ESTI_A69_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txpwc_slope : 9;
    unsigned int rsvd_0 : 3;
    unsigned int ro_txpwc_offset : 15;
    unsigned int rsvd_1 : 4;
    unsigned int ro_txpwc_ready : 1;
  } b;
} RG_ESTI_A69_FIELD_T;

#define RG_ESTI_A70                               (RF_D_ADDA_ESTI_REG_BASE + 0x118)
// Bit 0           rg_txdpd_esti_read_response     U     RW        default = 'h0
// Bit 2           rg_txdpd_esti_read_response_bypass     U     RW        default = 'h0
// Bit 11  :4      rg_txdpd_esti_alpha            U     RW        default = 'h33
// Bit 18  :12     rg_txdpd_esti_sync_gap         U     RW        default = 'h20
// Bit 22          ro_txdpd_esti_ready            U     RO        default = 'h0
// Bit 31  :24     rg_txdpd_esti_mp_pow_thresh     U     RW        default = 'h6
typedef union RG_ESTI_A70_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_txdpd_esti_read_response : 1;
    unsigned int rsvd_0 : 1;
    unsigned int rg_txdpd_esti_read_response_bypass : 1;
    unsigned int rsvd_1 : 1;
    unsigned int rg_txdpd_esti_alpha : 8;
    unsigned int rg_txdpd_esti_sync_gap : 7;
    unsigned int rsvd_2 : 3;
    unsigned int ro_txdpd_esti_ready : 1;
    unsigned int rsvd_3 : 1;
    unsigned int rg_txdpd_esti_mp_pow_thresh : 8;
  } b;
} RG_ESTI_A70_FIELD_T;

#define RG_ESTI_A72                               (RF_D_ADDA_ESTI_REG_BASE + 0x11c)
// Bit 11  :0      ro_txdpd_esti_coef_i_0         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_0         U     RO        default = 'h0
typedef union RG_ESTI_A72_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_0 : 12;
    unsigned int ro_txdpd_esti_coef_q_0 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A72_FIELD_T;

#define RG_ESTI_A73                               (RF_D_ADDA_ESTI_REG_BASE + 0x120)
// Bit 11  :0      ro_txdpd_esti_coef_i_1         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_1         U     RO        default = 'h0
typedef union RG_ESTI_A73_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_1 : 12;
    unsigned int ro_txdpd_esti_coef_q_1 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A73_FIELD_T;

#define RG_ESTI_A74                               (RF_D_ADDA_ESTI_REG_BASE + 0x124)
// Bit 11  :0      ro_txdpd_esti_coef_i_2         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_2         U     RO        default = 'h0
typedef union RG_ESTI_A74_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_2 : 12;
    unsigned int ro_txdpd_esti_coef_q_2 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A74_FIELD_T;

#define RG_ESTI_A75                               (RF_D_ADDA_ESTI_REG_BASE + 0x128)
// Bit 11  :0      ro_txdpd_esti_coef_i_3         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_3         U     RO        default = 'h0
typedef union RG_ESTI_A75_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_3 : 12;
    unsigned int ro_txdpd_esti_coef_q_3 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A75_FIELD_T;

#define RG_ESTI_A76                               (RF_D_ADDA_ESTI_REG_BASE + 0x12c)
// Bit 11  :0      ro_txdpd_esti_coef_i_4         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_4         U     RO        default = 'h0
typedef union RG_ESTI_A76_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_4 : 12;
    unsigned int ro_txdpd_esti_coef_q_4 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A76_FIELD_T;

#define RG_ESTI_A77                               (RF_D_ADDA_ESTI_REG_BASE + 0x130)
// Bit 11  :0      ro_txdpd_esti_coef_i_5         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_5         U     RO        default = 'h0
typedef union RG_ESTI_A77_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_5 : 12;
    unsigned int ro_txdpd_esti_coef_q_5 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A77_FIELD_T;

#define RG_ESTI_A78                               (RF_D_ADDA_ESTI_REG_BASE + 0x134)
// Bit 11  :0      ro_txdpd_esti_coef_i_6         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_6         U     RO        default = 'h0
typedef union RG_ESTI_A78_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_6 : 12;
    unsigned int ro_txdpd_esti_coef_q_6 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A78_FIELD_T;

#define RG_ESTI_A79                               (RF_D_ADDA_ESTI_REG_BASE + 0x138)
// Bit 11  :0      ro_txdpd_esti_coef_i_7         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_7         U     RO        default = 'h0
typedef union RG_ESTI_A79_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_7 : 12;
    unsigned int ro_txdpd_esti_coef_q_7 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A79_FIELD_T;

#define RG_ESTI_A80                               (RF_D_ADDA_ESTI_REG_BASE + 0x13c)
// Bit 11  :0      ro_txdpd_esti_coef_i_8         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_8         U     RO        default = 'h0
typedef union RG_ESTI_A80_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_8 : 12;
    unsigned int ro_txdpd_esti_coef_q_8 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A80_FIELD_T;

#define RG_ESTI_A81                               (RF_D_ADDA_ESTI_REG_BASE + 0x140)
// Bit 11  :0      ro_txdpd_esti_coef_i_9         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_9         U     RO        default = 'h0
typedef union RG_ESTI_A81_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_9 : 12;
    unsigned int ro_txdpd_esti_coef_q_9 : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A81_FIELD_T;

#define RG_ESTI_A82                               (RF_D_ADDA_ESTI_REG_BASE + 0x144)
// Bit 11  :0      ro_txdpd_esti_coef_i_a         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_a         U     RO        default = 'h0
typedef union RG_ESTI_A82_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_a : 12;
    unsigned int ro_txdpd_esti_coef_q_a : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A82_FIELD_T;

#define RG_ESTI_A83                               (RF_D_ADDA_ESTI_REG_BASE + 0x148)
// Bit 11  :0      ro_txdpd_esti_coef_i_b         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_b         U     RO        default = 'h0
typedef union RG_ESTI_A83_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_b : 12;
    unsigned int ro_txdpd_esti_coef_q_b : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A83_FIELD_T;

#define RG_ESTI_A84                               (RF_D_ADDA_ESTI_REG_BASE + 0x14c)
// Bit 11  :0      ro_txdpd_esti_coef_i_c         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_c         U     RO        default = 'h0
typedef union RG_ESTI_A84_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_c : 12;
    unsigned int ro_txdpd_esti_coef_q_c : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A84_FIELD_T;

#define RG_ESTI_A85                               (RF_D_ADDA_ESTI_REG_BASE + 0x150)
// Bit 11  :0      ro_txdpd_esti_coef_i_d         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_d         U     RO        default = 'h0
typedef union RG_ESTI_A85_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_d : 12;
    unsigned int ro_txdpd_esti_coef_q_d : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A85_FIELD_T;

#define RG_ESTI_A86                               (RF_D_ADDA_ESTI_REG_BASE + 0x154)
// Bit 11  :0      ro_txdpd_esti_coef_i_e         U     RO        default = 'h0
// Bit 23  :12     ro_txdpd_esti_coef_q_e         U     RO        default = 'h0
typedef union RG_ESTI_A86_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_coef_i_e : 12;
    unsigned int ro_txdpd_esti_coef_q_e : 12;
    unsigned int rsvd_0 : 8;
  } b;
} RG_ESTI_A86_FIELD_T;

#define RG_ESTI_A87                               (RF_D_ADDA_ESTI_REG_BASE + 0x158)
// Bit 0           rg_snr_esti_en                 U     RW        default = 'h1
// Bit 6   :4      rg_snr_esti_calctime           U     RW        default = 'h0
// Bit 10  :8      rg_snr_alpha                   U     RW        default = 'h0
// Bit 18  :16     rg_txdpd_snr_esti_alpha        U     RW        default = 'h0
typedef union RG_ESTI_A87_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_snr_esti_en : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_snr_esti_calctime : 3;
    unsigned int rsvd_1 : 1;
    unsigned int rg_snr_alpha : 3;
    unsigned int rsvd_2 : 5;
    unsigned int rg_txdpd_snr_esti_alpha : 3;
    unsigned int rsvd_3 : 13;
  } b;
} RG_ESTI_A87_FIELD_T;

#define RG_ESTI_A88                               (RF_D_ADDA_ESTI_REG_BASE + 0x15c)
// Bit 8   :0      ro_adda_snr_db                 U     RO        default = 'h0
// Bit 12          ro_adda_db_vld                 U     RO        default = 'h0
// Bit 24  :16     ro_adda_pwr_db                 U     RO        default = 'h0
typedef union RG_ESTI_A88_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_adda_snr_db : 9;
    unsigned int rsvd_0 : 3;
    unsigned int ro_adda_db_vld : 1;
    unsigned int rsvd_1 : 3;
    unsigned int ro_adda_pwr_db : 9;
    unsigned int rsvd_2 : 7;
  } b;
} RG_ESTI_A88_FIELD_T;

#define RG_ESTI_A89                               (RF_D_ADDA_ESTI_REG_BASE + 0x160)
// Bit 8   :0      ro_txdpd_esti_snr_db           U     RO        default = 'h0
typedef union RG_ESTI_A89_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_esti_snr_db : 9;
    unsigned int rsvd_0 : 23;
  } b;
} RG_ESTI_A89_FIELD_T;

#define RG_ESTI_A90                               (RF_D_ADDA_ESTI_REG_BASE + 0x164)
// Bit 29  :0      ro_dcoc_dc_i                   U     RO        default = 'h0
typedef union RG_ESTI_A90_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_dcoc_dc_i : 30;
    unsigned int rsvd_0 : 2;
  } b;
} RG_ESTI_A90_FIELD_T;

#define RG_ESTI_A91                               (RF_D_ADDA_ESTI_REG_BASE + 0x168)
// Bit 29  :0      ro_dcoc_dc_q                   U     RO        default = 'h0
typedef union RG_ESTI_A91_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_dcoc_dc_q : 30;
    unsigned int rsvd_0 : 2;
  } b;
} RG_ESTI_A91_FIELD_T;

#define RG_ESTI_A92                               (RF_D_ADDA_ESTI_REG_BASE + 0x16c)
// Bit 31  :0      ro_txirr_dc_pow_low            U     RO        default = 'h0
typedef union RG_ESTI_A92_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txirr_dc_pow_low : 32;
  } b;
} RG_ESTI_A92_FIELD_T;

#define RG_ESTI_A93                               (RF_D_ADDA_ESTI_REG_BASE + 0x170)
// Bit 22  :0      ro_txirr_dc_pow_high           U     RO        default = 'h0
typedef union RG_ESTI_A93_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txirr_dc_pow_high : 23;
    unsigned int rsvd_0 : 9;
  } b;
} RG_ESTI_A93_FIELD_T;

#define RG_ESTI_A94                               (RF_D_ADDA_ESTI_REG_BASE + 0x174)
// Bit 31  :0      ro_lnatank_esti_pow            U     RO        default = 'h0
typedef union RG_ESTI_A94_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_lnatank_esti_pow : 32;
  } b;
} RG_ESTI_A94_FIELD_T;

#define RG_ESTI_A96                               (RF_D_ADDA_ESTI_REG_BASE + 0x17c)
// Bit 11  :0      ro_txdpd_gain                  U     RO        default = 'h0
typedef union RG_ESTI_A96_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_txdpd_gain : 12;
    unsigned int rsvd_0 : 20;
  } b;
} RG_ESTI_A96_FIELD_T;

#endif

