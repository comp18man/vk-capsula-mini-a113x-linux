#ifdef RF_D_RX_REG
#else
#define RF_D_RX_REG


#define RF_D_RX_REG_BASE                          (0xc00)

#define RG_RX_A0                                  (RF_D_RX_REG_BASE + 0x0)
// Bit 31  :0      rg_wf_rx_dig_rsv0              U     RW        default = 'hffff
typedef union RG_RX_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_DIG_RSV0 : 32;
  } b;
} RG_RX_A0_FIELD_T;

#define RG_RX_A1                                  (RF_D_RX_REG_BASE + 0x4)
// Bit 31  :0      rg_wf_rx_dig_rsv1              U     RW        default = 'hffff
typedef union RG_RX_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_DIG_RSV1 : 32;
  } b;
} RG_RX_A1_FIELD_T;

#define RG_RX_A2                                  (RF_D_RX_REG_BASE + 0x8)
// Bit 0           rg_wf_rx_bbbm_man_mode         U     RW        default = 'h0
// Bit 3   :2      rg_wf_rx_bw_sel                U     RW        default = 'h0
// Bit 4           rg_wf_rx_adcfs_sel             U     RW        default = 'h0
// Bit 15  :8      rg_wf_rx_gain_man              U     RW        default = 'h9c
// Bit 16          rg_wf_rx_gain_man_mode         U     RW        default = 'h0
// Bit 23  :20     rg_wf_rx_osc_freq              U     RW        default = 'h5
typedef union RG_RX_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_BBBM_MAN_MODE : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_BW_SEL : 2;
    unsigned int RG_WF_RX_ADCFS_SEL : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WF_RX_GAIN_MAN : 8;
    unsigned int RG_WF_RX_GAIN_MAN_MODE : 1;
    unsigned int rsvd_2 : 3;
    unsigned int RG_WF_RX_OSC_FREQ : 4;
    unsigned int rsvd_3 : 8;
  } b;
} RG_RX_A2_FIELD_T;

#define RG_RX_A3                                  (RF_D_RX_REG_BASE + 0xc)
// Bit 0           rg_m0_wf_rx_fe_topbias_en      U     RW        default = 'h0
// Bit 1           rg_m0_wf_rx_lna_bias_en        U     RW        default = 'h0
// Bit 2           rg_m0_wf_rx_gm_bias_en         U     RW        default = 'h0
// Bit 3           rg_m0_wf_rx_mxr_bias_en        U     RW        default = 'h0
// Bit 4           rg_m0_wf5g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 5           rg_m0_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m0_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m0_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m0_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m0_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m0_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m0_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m0_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m0_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m0_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m0_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m0_wf_rtx_abb_topbias_en     U     RW        default = 'h0
// Bit 18          rg_m0_wf_rtx_abb_rx_en         U     RW        default = 'h0
// Bit 22          rg_m0_wf_rx_tia_bias_en        U     RW        default = 'h0
// Bit 23          rg_m0_wf_rx_rssipga_bias_en     U     RW        default = 'h0
// Bit 24          rg_m0_wf_rx_lpf_bias_en        U     RW        default = 'h0
// Bit 25          rg_m0_wf_rx_tia_en             U     RW        default = 'h0
// Bit 26          rg_m0_wf_rx_rssipga_en         U     RW        default = 'h0
// Bit 27          rg_m0_wf_rx_lpf_en             U     RW        default = 'h0
// Bit 28          rg_m0_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h0
// Bit 29          rg_m0_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h0
typedef union RG_RX_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M0_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M0_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M0_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M0_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M0_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M0_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M0_WF5G_RX_GM_EN : 1;
    unsigned int RG_M0_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M0_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M0_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M0_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M0_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M0_WF2G_RX_GM_EN : 1;
    unsigned int RG_M0_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M0_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M0_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M0_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M0_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M0_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M0_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M0_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M0_WF_RX_TIA_EN : 1;
    unsigned int RG_M0_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M0_WF_RX_LPF_EN : 1;
    unsigned int RG_M0_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M0_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A3_FIELD_T;

#define RG_RX_A4                                  (RF_D_RX_REG_BASE + 0x10)
// Bit 0           rg_m1_wf_rx_fe_topbias_en      U     RW        default = 'h0
// Bit 1           rg_m1_wf_rx_lna_bias_en        U     RW        default = 'h0
// Bit 2           rg_m1_wf_rx_gm_bias_en         U     RW        default = 'h0
// Bit 3           rg_m1_wf_rx_mxr_bias_en        U     RW        default = 'h0
// Bit 4           rg_m1_wf5g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 5           rg_m1_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m1_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m1_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m1_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m1_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m1_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m1_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m1_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m1_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m1_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m1_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m1_wf_rtx_abb_topbias_en     U     RW        default = 'h0
// Bit 18          rg_m1_wf_rtx_abb_rx_en         U     RW        default = 'h0
// Bit 22          rg_m1_wf_rx_tia_bias_en        U     RW        default = 'h0
// Bit 23          rg_m1_wf_rx_rssipga_bias_en     U     RW        default = 'h0
// Bit 24          rg_m1_wf_rx_lpf_bias_en        U     RW        default = 'h0
// Bit 25          rg_m1_wf_rx_tia_en             U     RW        default = 'h0
// Bit 26          rg_m1_wf_rx_rssipga_en         U     RW        default = 'h0
// Bit 27          rg_m1_wf_rx_lpf_en             U     RW        default = 'h0
// Bit 28          rg_m1_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h0
// Bit 29          rg_m1_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h0
typedef union RG_RX_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M1_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M1_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M1_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M1_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M1_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M1_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M1_WF5G_RX_GM_EN : 1;
    unsigned int RG_M1_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M1_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M1_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M1_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M1_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M1_WF2G_RX_GM_EN : 1;
    unsigned int RG_M1_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M1_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M1_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M1_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M1_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M1_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M1_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M1_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M1_WF_RX_TIA_EN : 1;
    unsigned int RG_M1_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M1_WF_RX_LPF_EN : 1;
    unsigned int RG_M1_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M1_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A4_FIELD_T;

#define RG_RX_A5                                  (RF_D_RX_REG_BASE + 0x14)
// Bit 0           rg_m2_wf_rx_fe_topbias_en      U     RW        default = 'h1
// Bit 1           rg_m2_wf_rx_lna_bias_en        U     RW        default = 'h0
// Bit 2           rg_m2_wf_rx_gm_bias_en         U     RW        default = 'h0
// Bit 3           rg_m2_wf_rx_mxr_bias_en        U     RW        default = 'h0
// Bit 4           rg_m2_wf5g_rx_lna_trsw_en      U     RW        default = 'h1
// Bit 5           rg_m2_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m2_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m2_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m2_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m2_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m2_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m2_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m2_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m2_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m2_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m2_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m2_wf_rtx_abb_topbias_en     U     RW        default = 'h1
// Bit 18          rg_m2_wf_rtx_abb_rx_en         U     RW        default = 'h0
// Bit 22          rg_m2_wf_rx_tia_bias_en        U     RW        default = 'h0
// Bit 23          rg_m2_wf_rx_rssipga_bias_en     U     RW        default = 'h0
// Bit 24          rg_m2_wf_rx_lpf_bias_en        U     RW        default = 'h0
// Bit 25          rg_m2_wf_rx_tia_en             U     RW        default = 'h0
// Bit 26          rg_m2_wf_rx_rssipga_en         U     RW        default = 'h0
// Bit 27          rg_m2_wf_rx_lpf_en             U     RW        default = 'h0
// Bit 28          rg_m2_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h0
// Bit 29          rg_m2_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h0
typedef union RG_RX_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M2_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M2_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M2_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M2_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M2_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M2_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M2_WF5G_RX_GM_EN : 1;
    unsigned int RG_M2_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M2_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M2_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M2_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M2_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M2_WF2G_RX_GM_EN : 1;
    unsigned int RG_M2_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M2_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M2_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M2_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M2_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M2_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M2_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M2_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M2_WF_RX_TIA_EN : 1;
    unsigned int RG_M2_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M2_WF_RX_LPF_EN : 1;
    unsigned int RG_M2_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M2_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A5_FIELD_T;

#define RG_RX_A6                                  (RF_D_RX_REG_BASE + 0x18)
// Bit 0           rg_m3_wf_rx_fe_topbias_en      U     RW        default = 'h1
// Bit 1           rg_m3_wf_rx_lna_bias_en        U     RW        default = 'h1
// Bit 2           rg_m3_wf_rx_gm_bias_en         U     RW        default = 'h1
// Bit 3           rg_m3_wf_rx_mxr_bias_en        U     RW        default = 'h1
// Bit 4           rg_m3_wf5g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 5           rg_m3_wf5g_rx_lna_en           U     RW        default = 'h1
// Bit 6           rg_m3_wf5g_rx_gm_en            U     RW        default = 'h1
// Bit 7           rg_m3_wf5g_rx_mxr_en           U     RW        default = 'h1
// Bit 8           rg_m3_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m3_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m3_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m3_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m3_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m3_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m3_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m3_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m3_wf_rtx_abb_topbias_en     U     RW        default = 'h1
// Bit 18          rg_m3_wf_rtx_abb_rx_en         U     RW        default = 'h1
// Bit 22          rg_m3_wf_rx_tia_bias_en        U     RW        default = 'h1
// Bit 23          rg_m3_wf_rx_rssipga_bias_en     U     RW        default = 'h1
// Bit 24          rg_m3_wf_rx_lpf_bias_en        U     RW        default = 'h1
// Bit 25          rg_m3_wf_rx_tia_en             U     RW        default = 'h1
// Bit 26          rg_m3_wf_rx_rssipga_en         U     RW        default = 'h1
// Bit 27          rg_m3_wf_rx_lpf_en             U     RW        default = 'h1
// Bit 28          rg_m3_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h1
// Bit 29          rg_m3_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h1
typedef union RG_RX_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M3_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M3_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M3_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M3_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M3_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M3_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M3_WF5G_RX_GM_EN : 1;
    unsigned int RG_M3_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M3_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M3_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M3_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M3_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M3_WF2G_RX_GM_EN : 1;
    unsigned int RG_M3_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M3_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M3_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M3_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M3_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M3_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M3_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M3_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M3_WF_RX_TIA_EN : 1;
    unsigned int RG_M3_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M3_WF_RX_LPF_EN : 1;
    unsigned int RG_M3_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M3_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A6_FIELD_T;

#define RG_RX_A7                                  (RF_D_RX_REG_BASE + 0x1c)
// Bit 3   :0      rg_wf5g_rx_lna_tank_sel_band0     U     RW        default = 'h9
// Bit 7   :4      rg_wf5g_rx_lna_tank_sel_band1     U     RW        default = 'h9
// Bit 11  :8      rg_wf5g_rx_lna_tank_sel_band2     U     RW        default = 'h8
// Bit 15  :12     rg_wf5g_rx_lna_tank_sel_band3     U     RW        default = 'h7
// Bit 19  :16     rg_wf5g_rx_lna_tank_sel_band4     U     RW        default = 'h6
// Bit 23  :20     rg_wf5g_rx_lna_tank_sel_band5     U     RW        default = 'h5
// Bit 27  :24     rg_wf5g_rx_lna_tank_sel_band6     U     RW        default = 'h4
// Bit 31  :28     rg_wf5g_rx_lna_tank_sel_band7     U     RW        default = 'h4
typedef union RG_RX_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND0 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND1 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND2 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND3 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND4 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND5 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND6 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND7 : 4;
  } b;
} RG_RX_A7_FIELD_T;

#define RG_RX_A8                                  (RF_D_RX_REG_BASE + 0x20)
// Bit 3   :0      rg_wf5g_rx_lna_tank_sel_band8     U     RW        default = 'h3
// Bit 7   :4      rg_wf5g_rx_lna_tank_sel_band9     U     RW        default = 'h2
// Bit 11  :8      rg_wf5g_rx_lna_tank_sel_man     U     RW        default = 'h2
// Bit 12          rg_wf5g_rx_lna_tank_sel_man_mode     U     RW        default = 'h0
// Bit 19  :16     rg_wf2g_rx_lna_tank_sel        U     RW        default = 'h5
// Bit 23  :20     rg_wf2g_rx_lna_tank_sel_man     U     RW        default = 'h5
// Bit 24          rg_wf2g_rx_lna_tank_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND8 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_BAND9 : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_MAN : 4;
    unsigned int RG_WF5G_RX_LNA_TANK_SEL_MAN_MODE : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF2G_RX_LNA_TANK_SEL : 4;
    unsigned int RG_WF2G_RX_LNA_TANK_SEL_MAN : 4;
    unsigned int RG_WF2G_RX_LNA_TANK_SEL_MAN_MODE : 1;
    unsigned int rsvd_1 : 7;
  } b;
} RG_RX_A8_FIELD_T;

#define RG_RX_A9                                  (RF_D_RX_REG_BASE + 0x24)
// Bit 2   :0      rg_wf5g_rx_lna_tank_offset_g1     U     RW        default = 'h0
// Bit 6   :4      rg_wf5g_rx_lna_tank_offset_g2     U     RW        default = 'h0
// Bit 10  :8      rg_wf5g_rx_lna_tank_offset_g3     U     RW        default = 'h0
// Bit 14  :12     rg_wf5g_rx_lna_tank_offset_g4     U     RW        default = 'h2
// Bit 18  :16     rg_wf5g_rx_lna_tank_offset_g5     U     RW        default = 'h2
// Bit 26  :24     rg_wf5g_rx_lna_tank_offset_g6     U     RW        default = 'h2
// Bit 30  :28     rg_wf5g_rx_lna_tank_offset_g7     U     RW        default = 'h2
typedef union RG_RX_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G1 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G2 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G3 : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G4 : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G5 : 3;
    unsigned int rsvd_4 : 5;
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G6 : 3;
    unsigned int rsvd_5 : 1;
    unsigned int RG_WF5G_RX_LNA_TANK_OFFSET_G7 : 3;
    unsigned int rsvd_6 : 1;
  } b;
} RG_RX_A9_FIELD_T;

#define RG_RX_A10                                 (RF_D_RX_REG_BASE + 0x28)
// Bit 2   :0      rg_wf2g_rx_lna_tank_offset_g1     U     RW        default = 'h0
// Bit 6   :4      rg_wf2g_rx_lna_tank_offset_g2     U     RW        default = 'h0
// Bit 10  :8      rg_wf2g_rx_lna_tank_offset_g3     U     RW        default = 'h0
// Bit 14  :12     rg_wf2g_rx_lna_tank_offset_g4     U     RW        default = 'h0
// Bit 18  :16     rg_wf2g_rx_lna_tank_offset_g5     U     RW        default = 'h0
// Bit 26  :24     rg_wf2g_rx_lna_tank_offset_g6     U     RW        default = 'h2
// Bit 30  :28     rg_wf2g_rx_lna_tank_offset_g7     U     RW        default = 'h3
typedef union RG_RX_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G1 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G2 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G3 : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G4 : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G5 : 3;
    unsigned int rsvd_4 : 5;
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G6 : 3;
    unsigned int rsvd_5 : 1;
    unsigned int RG_WF2G_RX_LNA_TANK_OFFSET_G7 : 3;
    unsigned int rsvd_6 : 1;
  } b;
} RG_RX_A10_FIELD_T;

#define RG_RX_A11                                 (RF_D_RX_REG_BASE + 0x2c)
// Bit 6   :0      rg_wf_rxfe_gain_g1             U     RW        default = 'h12
// Bit 14  :8      rg_wf_rxfe_gain_g2             U     RW        default = 'h1a
// Bit 22  :16     rg_wf_rxfe_gain_g3             U     RW        default = 'h32
// Bit 30  :24     rg_wf_rxfe_gain_g4             U     RW        default = 'h3a
typedef union RG_RX_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RXFE_GAIN_G1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RXFE_GAIN_G2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RXFE_GAIN_G3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_RXFE_GAIN_G4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A11_FIELD_T;

#define RG_RX_A12                                 (RF_D_RX_REG_BASE + 0x30)
// Bit 6   :0      rg_wf_rxfe_gain_g5             U     RW        default = 'h52
// Bit 14  :8      rg_wf_rxfe_gain_g6             U     RW        default = 'h5a
// Bit 22  :16     rg_wf_rxfe_gain_g7             U     RW        default = 'h72
// Bit 30  :24     rg_wf_rxfe_gain_g8             U     RW        default = 'h7a
typedef union RG_RX_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RXFE_GAIN_G5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RXFE_GAIN_G6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RXFE_GAIN_G7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_RXFE_GAIN_G8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A12_FIELD_T;

#define RG_RX_A13                                 (RF_D_RX_REG_BASE + 0x34)
// Bit 6   :0      rg_wf_rxfe_gain_g9             U     RW        default = 'h7e
// Bit 22  :8      rg_wf_rxfe_lpf_gain_man        U     RW        default = 'h7ec
// Bit 24          rg_wf_rxfe_lpf_gain_man_mode     U     RW        default = 'h0
typedef union RG_RX_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RXFE_GAIN_G9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RXFE_LPF_GAIN_MAN : 15;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RXFE_LPF_GAIN_MAN_MODE : 1;
    unsigned int rsvd_2 : 7;
  } b;
} RG_RX_A13_FIELD_T;

#define RG_RX_A14                                 (RF_D_RX_REG_BASE + 0x38)
// Bit 2   :0      rg_wf5g_rx_lna_ictrl_g1        U     RW        default = 'h3
// Bit 6   :4      rg_wf5g_rx_lna_ictrl_g2        U     RW        default = 'h3
// Bit 10  :8      rg_wf5g_rx_lna_ictrl_g3        U     RW        default = 'h3
// Bit 14  :12     rg_wf5g_rx_lna_ictrl_g4        U     RW        default = 'h2
// Bit 18  :16     rg_wf5g_rx_lna_ictrl_g5        U     RW        default = 'h2
// Bit 22  :20     rg_wf5g_rx_lna_ictrl_g6        U     RW        default = 'h3
// Bit 26  :24     rg_wf5g_rx_lna_ictrl_g7        U     RW        default = 'h3
// Bit 30  :28     rg_wf5g_rx_lna_ictrl_man       U     RW        default = 'h0
// Bit 31          rg_wf5g_rx_lna_ictrl_man_mode     U     RW        default = 'h0
typedef union RG_RX_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_ICTRL_G1 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_G2 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_G3 : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_G4 : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_G5 : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_G6 : 3;
    unsigned int rsvd_5 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_G7 : 3;
    unsigned int rsvd_6 : 1;
    unsigned int RG_WF5G_RX_LNA_ICTRL_MAN : 3;
    unsigned int RG_WF5G_RX_LNA_ICTRL_MAN_MODE : 1;
  } b;
} RG_RX_A14_FIELD_T;

#define RG_RX_A15                                 (RF_D_RX_REG_BASE + 0x3c)
// Bit 5   :0      rg_wf5g_rx_lna_slice_g1        U     RW        default = 'h1
// Bit 13  :8      rg_wf5g_rx_lna_slice_g2        U     RW        default = 'h1
// Bit 21  :16     rg_wf5g_rx_lna_slice_g3        U     RW        default = 'h2
// Bit 29  :24     rg_wf5g_rx_lna_slice_g4        U     RW        default = 'h4
typedef union RG_RX_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_SLICE_G1 : 6;
    unsigned int rsvd_0 : 2;
    unsigned int RG_WF5G_RX_LNA_SLICE_G2 : 6;
    unsigned int rsvd_1 : 2;
    unsigned int RG_WF5G_RX_LNA_SLICE_G3 : 6;
    unsigned int rsvd_2 : 2;
    unsigned int RG_WF5G_RX_LNA_SLICE_G4 : 6;
    unsigned int rsvd_3 : 2;
  } b;
} RG_RX_A15_FIELD_T;

#define RG_RX_A16                                 (RF_D_RX_REG_BASE + 0x40)
// Bit 5   :0      rg_wf5g_rx_lna_slice_g5        U     RW        default = 'hc
// Bit 13  :8      rg_wf5g_rx_lna_slice_g6        U     RW        default = 'h1c
// Bit 21  :16     rg_wf5g_rx_lna_slice_g7        U     RW        default = 'h3c
// Bit 29  :24     rg_wf5g_rx_lna_slice_man       U     RW        default = 'h0
// Bit 31          rg_wf5g_rx_lna_slice_man_mode     U     RW        default = 'h0
typedef union RG_RX_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_SLICE_G5 : 6;
    unsigned int rsvd_0 : 2;
    unsigned int RG_WF5G_RX_LNA_SLICE_G6 : 6;
    unsigned int rsvd_1 : 2;
    unsigned int RG_WF5G_RX_LNA_SLICE_G7 : 6;
    unsigned int rsvd_2 : 2;
    unsigned int RG_WF5G_RX_LNA_SLICE_MAN : 6;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF5G_RX_LNA_SLICE_MAN_MODE : 1;
  } b;
} RG_RX_A16_FIELD_T;

#define RG_RX_A17                                 (RF_D_RX_REG_BASE + 0x44)
// Bit 3   :0      rg_wf5g_rx_lna_rfb_lb_g1       U     RW        default = 'h5
// Bit 7   :4      rg_wf5g_rx_lna_rfb_lb_g2       U     RW        default = 'h2
// Bit 11  :8      rg_wf5g_rx_lna_rfb_lb_g3       U     RW        default = 'h4
// Bit 15  :12     rg_wf5g_rx_lna_rfb_lb_g4       U     RW        default = 'h7
// Bit 19  :16     rg_wf5g_rx_lna_rfb_lb_g5       U     RW        default = 'h7
// Bit 23  :20     rg_wf5g_rx_lna_rfb_lb_g6       U     RW        default = 'h7
// Bit 27  :24     rg_wf5g_rx_lna_rfb_lb_g7       U     RW        default = 'h0
// Bit 31  :28     rg_wf5g_rx_lna_rfb_hb_g1       U     RW        default = 'h2
typedef union RG_RX_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G1 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G2 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G3 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G4 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G5 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G6 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_LB_G7 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G1 : 4;
  } b;
} RG_RX_A17_FIELD_T;

#define RG_RX_A18                                 (RF_D_RX_REG_BASE + 0x48)
// Bit 3   :0      rg_wf5g_rx_lna_rfb_hb_g2       U     RW        default = 'h0
// Bit 7   :4      rg_wf5g_rx_lna_rfb_hb_g3       U     RW        default = 'h2
// Bit 11  :8      rg_wf5g_rx_lna_rfb_hb_g4       U     RW        default = 'h5
// Bit 15  :12     rg_wf5g_rx_lna_rfb_hb_g5       U     RW        default = 'h4
// Bit 19  :16     rg_wf5g_rx_lna_rfb_hb_g6       U     RW        default = 'h5
// Bit 23  :20     rg_wf5g_rx_lna_rfb_hb_g7       U     RW        default = 'h0
// Bit 27  :24     rg_wf5g_rx_lna_rfb_man         U     RW        default = 'h0
// Bit 31          rg_wf5g_rx_lna_rfb_man_mode     U     RW        default = 'h0
typedef union RG_RX_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G2 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G3 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G4 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G5 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G6 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_HB_G7 : 4;
    unsigned int RG_WF5G_RX_LNA_RFB_MAN : 4;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF5G_RX_LNA_RFB_MAN_MODE : 1;
  } b;
} RG_RX_A18_FIELD_T;

#define RG_RX_A19                                 (RF_D_RX_REG_BASE + 0x4c)
// Bit 2   :0      rg_wf5g_rx_lna_rt_g1           U     RW        default = 'h5
// Bit 5   :3      rg_wf5g_rx_lna_rt_g2           U     RW        default = 'h7
// Bit 8   :6      rg_wf5g_rx_lna_rt_g3           U     RW        default = 'h5
// Bit 11  :9      rg_wf5g_rx_lna_rt_g4           U     RW        default = 'h5
// Bit 14  :12     rg_wf5g_rx_lna_rt_g5           U     RW        default = 'h3
// Bit 17  :15     rg_wf5g_rx_lna_rt_g6           U     RW        default = 'h1
// Bit 26  :24     rg_wf5g_rx_lna_rt_man          U     RW        default = 'h0
// Bit 31          rg_wf5g_rx_lna_rt_man_mode     U     RW        default = 'h0
typedef union RG_RX_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_RT_G1 : 3;
    unsigned int RG_WF5G_RX_LNA_RT_G2 : 3;
    unsigned int RG_WF5G_RX_LNA_RT_G3 : 3;
    unsigned int RG_WF5G_RX_LNA_RT_G4 : 3;
    unsigned int RG_WF5G_RX_LNA_RT_G5 : 3;
    unsigned int RG_WF5G_RX_LNA_RT_G6 : 3;
    unsigned int rsvd_0 : 6;
    unsigned int RG_WF5G_RX_LNA_RT_MAN : 3;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF5G_RX_LNA_RT_MAN_MODE : 1;
  } b;
} RG_RX_A19_FIELD_T;

#define RG_RX_A20                                 (RF_D_RX_REG_BASE + 0x50)
// Bit 2   :0      rg_wf5g_rx_lna_att_lb_g1       U     RW        default = 'h3
// Bit 5   :3      rg_wf5g_rx_lna_att_lb_g2       U     RW        default = 'h1
// Bit 8   :6      rg_wf5g_rx_lna_att_lb_g3       U     RW        default = 'h0
// Bit 11  :9      rg_wf5g_rx_lna_att_hb_g1       U     RW        default = 'h6
// Bit 14  :12     rg_wf5g_rx_lna_att_hb_g2       U     RW        default = 'h2
// Bit 17  :15     rg_wf5g_rx_lna_att_hb_g3       U     RW        default = 'h1
// Bit 26  :24     rg_wf5g_rx_lna_att_man         U     RW        default = 'h0
// Bit 31          rg_wf5g_rx_lna_att_man_mode     U     RW        default = 'h0
typedef union RG_RX_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LNA_ATT_LB_G1 : 3;
    unsigned int RG_WF5G_RX_LNA_ATT_LB_G2 : 3;
    unsigned int RG_WF5G_RX_LNA_ATT_LB_G3 : 3;
    unsigned int RG_WF5G_RX_LNA_ATT_HB_G1 : 3;
    unsigned int RG_WF5G_RX_LNA_ATT_HB_G2 : 3;
    unsigned int RG_WF5G_RX_LNA_ATT_HB_G3 : 3;
    unsigned int rsvd_0 : 6;
    unsigned int RG_WF5G_RX_LNA_ATT_MAN : 3;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF5G_RX_LNA_ATT_MAN_MODE : 1;
  } b;
} RG_RX_A20_FIELD_T;

#define RG_RX_A21                                 (RF_D_RX_REG_BASE + 0x54)
// Bit 4   :0      rg_wf_rx_tia_cfix_sel_g1       U     RW        default = 'h1f
// Bit 12  :8      rg_wf_rx_tia_cfix_sel_g2       U     RW        default = 'h1a
// Bit 20  :16     rg_wf_rx_tia_cfix_sel_g3       U     RW        default = 'h14
// Bit 28  :24     rg_wf_rx_tia_cfix_sel_g4       U     RW        default = 'h1f
typedef union RG_RX_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_TIA_CFIX_SEL_G1 : 5;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_RX_TIA_CFIX_SEL_G2 : 5;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WF_RX_TIA_CFIX_SEL_G3 : 5;
    unsigned int rsvd_2 : 3;
    unsigned int RG_WF_RX_TIA_CFIX_SEL_G4 : 5;
    unsigned int rsvd_3 : 3;
  } b;
} RG_RX_A21_FIELD_T;

#define RG_RX_A22                                 (RF_D_RX_REG_BASE + 0x58)
// Bit 4   :0      rg_wf_rx_tia_cfix_sel_g5       U     RW        default = 'h1a
// Bit 12  :8      rg_wf_rx_tia_cfix_sel_g6       U     RW        default = 'h14
// Bit 28  :24     rg_wf_rx_tia_cfix_sel_man      U     RW        default = 'h0
// Bit 31          rg_wf_rx_tia_cfix_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_TIA_CFIX_SEL_G5 : 5;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_RX_TIA_CFIX_SEL_G6 : 5;
    unsigned int rsvd_1 : 11;
    unsigned int RG_WF_RX_TIA_CFIX_SEL_MAN : 5;
    unsigned int rsvd_2 : 2;
    unsigned int RG_WF_RX_TIA_CFIX_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A22_FIELD_T;

#define RG_RX_A23                                 (RF_D_RX_REG_BASE + 0x5c)
// Bit 3   :0      rg_wf_rx_tia_cunit_sel_g1      U     RW        default = 'h5
// Bit 7   :4      rg_wf_rx_tia_cunit_sel_g2      U     RW        default = 'h3
// Bit 11  :8      rg_wf_rx_tia_cunit_sel_g3      U     RW        default = 'h3
// Bit 15  :12     rg_wf_rx_tia_cunit_sel_g4      U     RW        default = 'h2
// Bit 19  :16     rg_wf_rx_tia_cunit_sel_g5      U     RW        default = 'h1
// Bit 23  :20     rg_wf_rx_tia_cunit_sel_g6      U     RW        default = 'h1
// Bit 27  :24     rg_wf_rx_tia_cunit_sel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_tia_cunit_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_G1 : 4;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_G2 : 4;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_G3 : 4;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_G4 : 4;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_G5 : 4;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_G6 : 4;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_MAN : 4;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_RX_TIA_CUNIT_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A23_FIELD_T;

#define RG_RX_A24                                 (RF_D_RX_REG_BASE + 0x60)
// Bit 3   :0      rg_wf_rx_lpf_sra_sel_bw20m     U     RW        default = 'h1
// Bit 7   :4      rg_wf_rx_lpf_sra_sel_bw40m     U     RW        default = 'h1
// Bit 11  :8      rg_wf_rx_lpf_sra_sel_bw80m     U     RW        default = 'h3
// Bit 15  :12     rg_wf_rx_lpf_sra_sel_bw160m     U     RW        default = 'h3
// Bit 27  :24     rg_wf_rx_lpf_sra_sel_man       U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_sra_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_SRA_SEL_BW20M : 4;
    unsigned int RG_WF_RX_LPF_SRA_SEL_BW40M : 4;
    unsigned int RG_WF_RX_LPF_SRA_SEL_BW80M : 4;
    unsigned int RG_WF_RX_LPF_SRA_SEL_BW160M : 4;
    unsigned int rsvd_0 : 8;
    unsigned int RG_WF_RX_LPF_SRA_SEL_MAN : 4;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WF_RX_LPF_SRA_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A24_FIELD_T;

#define RG_RX_A25                                 (RF_D_RX_REG_BASE + 0x64)
// Bit 3   :0      rg_wf_rx_lpf_sraq_sel_bw20m     U     RW        default = 'h1
// Bit 7   :4      rg_wf_rx_lpf_sraq_sel_bw40m     U     RW        default = 'h1
// Bit 11  :8      rg_wf_rx_lpf_sraq_sel_bw80m     U     RW        default = 'h3
// Bit 15  :12     rg_wf_rx_lpf_sraq_sel_bw160m     U     RW        default = 'h3
// Bit 27  :24     rg_wf_rx_lpf_sraq_sel_man      U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_sraq_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_SRAQ_SEL_BW20M : 4;
    unsigned int RG_WF_RX_LPF_SRAQ_SEL_BW40M : 4;
    unsigned int RG_WF_RX_LPF_SRAQ_SEL_BW80M : 4;
    unsigned int RG_WF_RX_LPF_SRAQ_SEL_BW160M : 4;
    unsigned int rsvd_0 : 8;
    unsigned int RG_WF_RX_LPF_SRAQ_SEL_MAN : 4;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WF_RX_LPF_SRAQ_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A25_FIELD_T;

#define RG_RX_A26                                 (RF_D_RX_REG_BASE + 0x68)
// Bit 4   :0      rg_wf_rx_lpf_cfix_sel_bw20m     U     RW        default = 'h1a
// Bit 9   :5      rg_wf_rx_lpf_cfix_sel_bw40m     U     RW        default = 'h1a
// Bit 14  :10     rg_wf_rx_lpf_cfix_sel_bw80m     U     RW        default = 'h1a
// Bit 19  :15     rg_wf_rx_lpf_cfix_sel_bw160m     U     RW        default = 'h1a
// Bit 24  :20     rg_wf_rx_lpf_cfix_sel_man      U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_cfix_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_CFIX_SEL_BW20M : 5;
    unsigned int RG_WF_RX_LPF_CFIX_SEL_BW40M : 5;
    unsigned int RG_WF_RX_LPF_CFIX_SEL_BW80M : 5;
    unsigned int RG_WF_RX_LPF_CFIX_SEL_BW160M : 5;
    unsigned int RG_WF_RX_LPF_CFIX_SEL_MAN : 5;
    unsigned int rsvd_0 : 6;
    unsigned int RG_WF_RX_LPF_CFIX_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A26_FIELD_T;

#define RG_RX_A27                                 (RF_D_RX_REG_BASE + 0x6c)
// Bit 2   :0      rg_wf_rx_lpf_c1unit_sel_bw20m     U     RW        default = 'h5
// Bit 6   :4      rg_wf_rx_lpf_c1unit_sel_bw40m     U     RW        default = 'h2
// Bit 10  :8      rg_wf_rx_lpf_c1unit_sel_bw80m     U     RW        default = 'h2
// Bit 14  :12     rg_wf_rx_lpf_c1unit_sel_bw160m     U     RW        default = 'h0
// Bit 18  :16     rg_wf_rx_lpf_c1unit_sel_man     U     RW        default = 'h0
// Bit 20          rg_wf_rx_lpf_c1unit_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A27_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_C1UNIT_SEL_BW20M : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_LPF_C1UNIT_SEL_BW40M : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RX_LPF_C1UNIT_SEL_BW80M : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_RX_LPF_C1UNIT_SEL_BW160M : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF_RX_LPF_C1UNIT_SEL_MAN : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF_RX_LPF_C1UNIT_SEL_MAN_MODE : 1;
    unsigned int rsvd_5 : 11;
  } b;
} RG_RX_A27_FIELD_T;

#define RG_RX_A28                                 (RF_D_RX_REG_BASE + 0x70)
// Bit 2   :0      rg_wf_rx_lpf_c2unit_sel_bw20m     U     RW        default = 'h5
// Bit 6   :4      rg_wf_rx_lpf_c2unit_sel_bw40m     U     RW        default = 'h2
// Bit 10  :8      rg_wf_rx_lpf_c2unit_sel_bw80m     U     RW        default = 'h2
// Bit 14  :12     rg_wf_rx_lpf_c2unit_sel_bw160m     U     RW        default = 'h0
// Bit 18  :16     rg_wf_rx_lpf_c2unit_sel_man     U     RW        default = 'h0
// Bit 20          rg_wf_rx_lpf_c2unit_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A28_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_C2UNIT_SEL_BW20M : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_LPF_C2UNIT_SEL_BW40M : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RX_LPF_C2UNIT_SEL_BW80M : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_RX_LPF_C2UNIT_SEL_BW160M : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF_RX_LPF_C2UNIT_SEL_MAN : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF_RX_LPF_C2UNIT_SEL_MAN_MODE : 1;
    unsigned int rsvd_5 : 11;
  } b;
} RG_RX_A28_FIELD_T;

#define RG_RX_A29                                 (RF_D_RX_REG_BASE + 0x74)
// Bit 3   :0      rg_wf_rx_lpf_rn_sel_bw20madc160m     U     RW        default = 'h1
// Bit 7   :4      rg_wf_rx_lpf_rn_sel_bw40madc160m     U     RW        default = 'h1
// Bit 11  :8      rg_wf_rx_lpf_rn_sel_bw80madc160m     U     RW        default = 'h3
// Bit 15  :12     rg_wf_rx_lpf_rn_sel_bw80madc320m     U     RW        default = 'h3
// Bit 19  :16     rg_wf_rx_lpf_rn_sel_bw160madc320m     U     RW        default = 'h3
// Bit 23  :20     rg_wf_rx_lpf_rn_sel_man        U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_rn_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A29_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_RN_SEL_BW20MADC160M : 4;
    unsigned int RG_WF_RX_LPF_RN_SEL_BW40MADC160M : 4;
    unsigned int RG_WF_RX_LPF_RN_SEL_BW80MADC160M : 4;
    unsigned int RG_WF_RX_LPF_RN_SEL_BW80MADC320M : 4;
    unsigned int RG_WF_RX_LPF_RN_SEL_BW160MADC320M : 4;
    unsigned int RG_WF_RX_LPF_RN_SEL_MAN : 4;
    unsigned int rsvd_0 : 7;
    unsigned int RG_WF_RX_LPF_RN_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A29_FIELD_T;

#define RG_RX_A30                                 (RF_D_RX_REG_BASE + 0x78)
// Bit 2   :0      rg_wf_rx_lpf_cn_sel_bw20madc160m     U     RW        default = 'h3
// Bit 6   :4      rg_wf_rx_lpf_cn_sel_bw40madc160m     U     RW        default = 'h3
// Bit 10  :8      rg_wf_rx_lpf_cn_sel_bw80madc160m     U     RW        default = 'h3
// Bit 14  :12     rg_wf_rx_lpf_cn_sel_bw80madc320m     U     RW        default = 'h1
// Bit 18  :16     rg_wf_rx_lpf_cn_sel_bw160madc320m     U     RW        default = 'h0
// Bit 22  :20     rg_wf_rx_lpf_cn_sel_man        U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_cn_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A30_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_CN_SEL_BW20MADC160M : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_LPF_CN_SEL_BW40MADC160M : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RX_LPF_CN_SEL_BW80MADC160M : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_RX_LPF_CN_SEL_BW80MADC320M : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF_RX_LPF_CN_SEL_BW160MADC320M : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF_RX_LPF_CN_SEL_MAN : 3;
    unsigned int rsvd_5 : 8;
    unsigned int RG_WF_RX_LPF_CN_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A30_FIELD_T;

#define RG_RX_A31                                 (RF_D_RX_REG_BASE + 0x7c)
// Bit 4   :0      rg_wf_rx_lpf_cnfix_sel_bw20madc160m     U     RW        default = 'h1a
// Bit 9   :5      rg_wf_rx_lpf_cnfix_sel_bw40madc160m     U     RW        default = 'h1a
// Bit 14  :10     rg_wf_rx_lpf_cnfix_sel_bw80madc160m     U     RW        default = 'h1a
// Bit 19  :15     rg_wf_rx_lpf_cnfix_sel_bw80madc320m     U     RW        default = 'h1a
// Bit 24  :20     rg_wf_rx_lpf_cnfix_sel_bw160madc320m     U     RW        default = 'h1a
// Bit 29  :25     rg_wf_rx_lpf_cnfix_sel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_cnfix_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A31_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_BW20MADC160M : 5;
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_BW40MADC160M : 5;
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_BW80MADC160M : 5;
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_BW80MADC320M : 5;
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_BW160MADC320M : 5;
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_MAN : 5;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_LPF_CNFIX_SEL_MAN_MODE : 1;
  } b;
} RG_RX_A31_FIELD_T;

#define RG_RX_A32                                 (RF_D_RX_REG_BASE + 0x80)
// Bit 6   :0      rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg4     U     RW        default = 'h40
typedef union RG_RX_A32_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A32_FIELD_T;

#define RG_RX_A33                                 (RF_D_RX_REG_BASE + 0x84)
// Bit 6   :0      rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg8     U     RW        default = 'h40
typedef union RG_RX_A33_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A33_FIELD_T;

#define RG_RX_A34                                 (RF_D_RX_REG_BASE + 0x88)
// Bit 6   :0      rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_i_dcoc_sel_tialglpfg12     U     RW        default = 'h40
typedef union RG_RX_A34_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIALGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A34_FIELD_T;

#define RG_RX_A35                                 (RF_D_RX_REG_BASE + 0x8c)
// Bit 6   :0      rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg4     U     RW        default = 'h40
typedef union RG_RX_A35_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A35_FIELD_T;

#define RG_RX_A36                                 (RF_D_RX_REG_BASE + 0x90)
// Bit 6   :0      rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg8     U     RW        default = 'h40
typedef union RG_RX_A36_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A36_FIELD_T;

#define RG_RX_A37                                 (RF_D_RX_REG_BASE + 0x94)
// Bit 6   :0      rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_i_dcoc_sel_tiahglpfg12     U     RW        default = 'h40
typedef union RG_RX_A37_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_I_DCOC_SEL_TIAHGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A37_FIELD_T;

#define RG_RX_A38                                 (RF_D_RX_REG_BASE + 0x98)
// Bit 6   :0      rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg4     U     RW        default = 'h40
typedef union RG_RX_A38_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A38_FIELD_T;

#define RG_RX_A39                                 (RF_D_RX_REG_BASE + 0x9c)
// Bit 6   :0      rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg8     U     RW        default = 'h40
typedef union RG_RX_A39_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A39_FIELD_T;

#define RG_RX_A40                                 (RF_D_RX_REG_BASE + 0xa0)
// Bit 6   :0      rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_q_dcoc_sel_tialglpfg12     U     RW        default = 'h40
typedef union RG_RX_A40_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIALGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A40_FIELD_T;

#define RG_RX_A41                                 (RF_D_RX_REG_BASE + 0xa4)
// Bit 6   :0      rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg4     U     RW        default = 'h40
typedef union RG_RX_A41_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A41_FIELD_T;

#define RG_RX_A42                                 (RF_D_RX_REG_BASE + 0xa8)
// Bit 6   :0      rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg8     U     RW        default = 'h40
typedef union RG_RX_A42_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A42_FIELD_T;

#define RG_RX_A43                                 (RF_D_RX_REG_BASE + 0xac)
// Bit 6   :0      rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf5g_rx_lpf_q_dcoc_sel_tiahglpfg12     U     RW        default = 'h40
typedef union RG_RX_A43_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A43_FIELD_T;

#define RG_RX_A44                                 (RF_D_RX_REG_BASE + 0xb0)
// Bit 6   :0      rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg4     U     RW        default = 'h40
typedef union RG_RX_A44_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A44_FIELD_T;

#define RG_RX_A45                                 (RF_D_RX_REG_BASE + 0xb4)
// Bit 6   :0      rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg8     U     RW        default = 'h40
typedef union RG_RX_A45_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A45_FIELD_T;

#define RG_RX_A46                                 (RF_D_RX_REG_BASE + 0xb8)
// Bit 6   :0      rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_i_dcoc_sel_tialglpfg12     U     RW        default = 'h40
typedef union RG_RX_A46_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIALGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A46_FIELD_T;

#define RG_RX_A47                                 (RF_D_RX_REG_BASE + 0xbc)
// Bit 6   :0      rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg4     U     RW        default = 'h40
typedef union RG_RX_A47_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A47_FIELD_T;

#define RG_RX_A48                                 (RF_D_RX_REG_BASE + 0xc0)
// Bit 6   :0      rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg8     U     RW        default = 'h40
typedef union RG_RX_A48_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A48_FIELD_T;

#define RG_RX_A49                                 (RF_D_RX_REG_BASE + 0xc4)
// Bit 6   :0      rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_i_dcoc_sel_tiahglpfg12     U     RW        default = 'h40
typedef union RG_RX_A49_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_I_DCOC_SEL_TIAHGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A49_FIELD_T;

#define RG_RX_A50                                 (RF_D_RX_REG_BASE + 0xc8)
// Bit 6   :0      rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg4     U     RW        default = 'h40
typedef union RG_RX_A50_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A50_FIELD_T;

#define RG_RX_A51                                 (RF_D_RX_REG_BASE + 0xcc)
// Bit 6   :0      rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg8     U     RW        default = 'h40
typedef union RG_RX_A51_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A51_FIELD_T;

#define RG_RX_A52                                 (RF_D_RX_REG_BASE + 0xd0)
// Bit 6   :0      rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_q_dcoc_sel_tialglpfg12     U     RW        default = 'h40
typedef union RG_RX_A52_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIALGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A52_FIELD_T;

#define RG_RX_A53                                 (RF_D_RX_REG_BASE + 0xd4)
// Bit 6   :0      rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg1     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg2     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg3     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg4     U     RW        default = 'h40
typedef union RG_RX_A53_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG1 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG2 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG3 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG4 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A53_FIELD_T;

#define RG_RX_A54                                 (RF_D_RX_REG_BASE + 0xd8)
// Bit 6   :0      rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg5     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg6     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg7     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg8     U     RW        default = 'h40
typedef union RG_RX_A54_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG5 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG6 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG7 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG8 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A54_FIELD_T;

#define RG_RX_A55                                 (RF_D_RX_REG_BASE + 0xdc)
// Bit 6   :0      rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg9     U     RW        default = 'h40
// Bit 14  :8      rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg10     U     RW        default = 'h40
// Bit 22  :16     rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg11     U     RW        default = 'h40
// Bit 30  :24     rg_wf2g_rx_lpf_q_dcoc_sel_tiahglpfg12     U     RW        default = 'h40
typedef union RG_RX_A55_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG9 : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG10 : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG11 : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LPF_Q_DCOC_SEL_TIAHGLPFG12 : 7;
    unsigned int rsvd_3 : 1;
  } b;
} RG_RX_A55_FIELD_T;

#define RG_RX_A56                                 (RF_D_RX_REG_BASE + 0xe0)
// Bit 6   :0      rg_wf_rx_lpf_i_dcoc_sel_man     U     RW        default = 'h40
// Bit 8           rg_wf_rx_lpf_i_dcoc_sel_man_mode     U     RW        default = 'h0
// Bit 22  :16     rg_wf_rx_lpf_q_dcoc_sel_man     U     RW        default = 'h40
// Bit 24          rg_wf_rx_lpf_q_dcoc_sel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A56_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_I_DCOC_SEL_MAN : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_LPF_I_DCOC_SEL_MAN_MODE : 1;
    unsigned int rsvd_1 : 7;
    unsigned int RG_WF_RX_LPF_Q_DCOC_SEL_MAN : 7;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_RX_LPF_Q_DCOC_SEL_MAN_MODE : 1;
    unsigned int rsvd_3 : 7;
  } b;
} RG_RX_A56_FIELD_T;

#define RG_RX_A57                                 (RF_D_RX_REG_BASE + 0xe4)
// Bit 1   :0      rg_wf_rx_rck_t1                U     RW        default = 'h0
// Bit 3   :2      rg_wf_rx_rck_t2                U     RW        default = 'h0
// Bit 5   :4      rg_wf_rx_rck_t3                U     RW        default = 'h0
// Bit 7   :6      rg_wf_rx_rck_t4                U     RW        default = 'h0
// Bit 9   :8      rg_wf_rx_rck_t5                U     RW        default = 'h0
// Bit 11  :10     rg_wf_rx_rck_t6                U     RW        default = 'h0
// Bit 22  :20     rg_wf_rx_rck_refsel_man        U     RW        default = 'h1
// Bit 24          rg_wf_rx_rck_refsel_man_mode     U     RW        default = 'h0
// Bit 31          rg_wf_rx_rck_cali_en           U     RW        default = 'h0
typedef union RG_RX_A57_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_RCK_T1 : 2;
    unsigned int RG_WF_RX_RCK_T2 : 2;
    unsigned int RG_WF_RX_RCK_T3 : 2;
    unsigned int RG_WF_RX_RCK_T4 : 2;
    unsigned int RG_WF_RX_RCK_T5 : 2;
    unsigned int RG_WF_RX_RCK_T6 : 2;
    unsigned int rsvd_0 : 8;
    unsigned int RG_WF_RX_RCK_REFSEL_MAN : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_RX_RCK_REFSEL_MAN_MODE : 1;
    unsigned int rsvd_2 : 6;
    unsigned int RG_WF_RX_RCK_CALI_EN : 1;
  } b;
} RG_RX_A57_FIELD_T;

#define RG_RX_A58                                 (RF_D_RX_REG_BASE + 0xe8)
// Bit 3   :0      rg_wf_rx_lpf_rck_ioffset_bw20m     U     RW        default = 'h0
// Bit 7   :4      rg_wf_rx_lpf_rck_ioffset_bw40m     U     RW        default = 'h0
// Bit 11  :8      rg_wf_rx_lpf_rck_ioffset_bw80m     U     RW        default = 'h0
// Bit 15  :12     rg_wf_rx_lpf_rck_ioffset_bw160m     U     RW        default = 'h0
// Bit 26  :20     rg_wf_rx_lpf_ctune_isel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_ctune_isel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A58_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_RCK_IOFFSET_BW20M : 4;
    unsigned int RG_WF_RX_LPF_RCK_IOFFSET_BW40M : 4;
    unsigned int RG_WF_RX_LPF_RCK_IOFFSET_BW80M : 4;
    unsigned int RG_WF_RX_LPF_RCK_IOFFSET_BW160M : 4;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_RX_LPF_CTUNE_ISEL_MAN : 7;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF_RX_LPF_CTUNE_ISEL_MAN_MODE : 1;
  } b;
} RG_RX_A58_FIELD_T;

#define RG_RX_A59                                 (RF_D_RX_REG_BASE + 0xec)
// Bit 3   :0      rg_wf_rx_lpf_rck_qoffset_bw20m     U     RW        default = 'h0
// Bit 7   :4      rg_wf_rx_lpf_rck_qoffset_bw40m     U     RW        default = 'h0
// Bit 11  :8      rg_wf_rx_lpf_rck_qoffset_bw80m     U     RW        default = 'h0
// Bit 15  :12     rg_wf_rx_lpf_rck_qoffset_bw160m     U     RW        default = 'h0
// Bit 26  :20     rg_wf_rx_lpf_ctune_qsel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_ctune_qsel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A59_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_RCK_QOFFSET_BW20M : 4;
    unsigned int RG_WF_RX_LPF_RCK_QOFFSET_BW40M : 4;
    unsigned int RG_WF_RX_LPF_RCK_QOFFSET_BW80M : 4;
    unsigned int RG_WF_RX_LPF_RCK_QOFFSET_BW160M : 4;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_RX_LPF_CTUNE_QSEL_MAN : 7;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF_RX_LPF_CTUNE_QSEL_MAN_MODE : 1;
  } b;
} RG_RX_A59_FIELD_T;

#define RG_RX_A60                                 (RF_D_RX_REG_BASE + 0xf0)
// Bit 3   :0      rg_wf_rx_tia_rck_ioffset       U     RW        default = 'h0
// Bit 10  :4      rg_wf_rx_tia_ctune_isel_man     U     RW        default = 'h0
// Bit 15          rg_wf_rx_tia_ctune_isel_man_mode     U     RW        default = 'h0
// Bit 19  :16     rg_wf_rx_tia_rck_qoffset       U     RW        default = 'h0
// Bit 26  :20     rg_wf_rx_tia_ctune_qsel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_tia_ctune_qsel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A60_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_TIA_RCK_IOFFSET : 4;
    unsigned int RG_WF_RX_TIA_CTUNE_ISEL_MAN : 7;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_RX_TIA_CTUNE_ISEL_MAN_MODE : 1;
    unsigned int RG_WF_RX_TIA_RCK_QOFFSET : 4;
    unsigned int RG_WF_RX_TIA_CTUNE_QSEL_MAN : 7;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF_RX_TIA_CTUNE_QSEL_MAN_MODE : 1;
  } b;
} RG_RX_A60_FIELD_T;

#define RG_RX_A61                                 (RF_D_RX_REG_BASE + 0xf4)
// Bit 3   :0      rg_wf_rx_lpf_notch_rck_offset_adc160m     U     RW        default = 'h0
// Bit 10  :4      rg_wf_rx_lpf_notch_ctune_sel_man     U     RW        default = 'h0
// Bit 15          rg_wf_rx_lpf_notch_ctune_sel_man_mode     U     RW        default = 'h0
// Bit 19  :16     rg_wf_rx_lpf_notch_rck_offset_adc320m     U     RW        default = 'h0
typedef union RG_RX_A61_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_NOTCH_RCK_OFFSET_ADC160M : 4;
    unsigned int RG_WF_RX_LPF_NOTCH_CTUNE_SEL_MAN : 7;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_RX_LPF_NOTCH_CTUNE_SEL_MAN_MODE : 1;
    unsigned int RG_WF_RX_LPF_NOTCH_RCK_OFFSET_ADC320M : 4;
    unsigned int rsvd_1 : 12;
  } b;
} RG_RX_A61_FIELD_T;

#define RG_RX_A62                                 (RF_D_RX_REG_BASE + 0xf8)
// Bit 3   :0      rg_wf_rx_rssipga_rck_ioffset     U     RW        default = 'h0
// Bit 10  :4      rg_wf_rx_rssipga_ctune_isel_man     U     RW        default = 'h0
// Bit 15          rg_wf_rx_rssipga_ctune_isel_man_mode     U     RW        default = 'h0
// Bit 19  :16     rg_wf_rx_rssipga_rck_qoffset     U     RW        default = 'h0
// Bit 26  :20     rg_wf_rx_rssipga_ctune_qsel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_rssipga_ctune_qsel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A62_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_RSSIPGA_RCK_IOFFSET : 4;
    unsigned int RG_WF_RX_RSSIPGA_CTUNE_ISEL_MAN : 7;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_RX_RSSIPGA_CTUNE_ISEL_MAN_MODE : 1;
    unsigned int RG_WF_RX_RSSIPGA_RCK_QOFFSET : 4;
    unsigned int RG_WF_RX_RSSIPGA_CTUNE_QSEL_MAN : 7;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF_RX_RSSIPGA_CTUNE_QSEL_MAN_MODE : 1;
  } b;
} RG_RX_A62_FIELD_T;

#define RG_RX_A63                                 (RF_D_RX_REG_BASE + 0xfc)
// Bit 6   :0      ro_wf_rx_rck                   U     RO        default = 'h0
// Bit 8           ro_wf_rx_rck_vld               U     RO        default = 'h0
typedef union RG_RX_A63_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_RX_RCK : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RO_WF_RX_RCK_VLD : 1;
    unsigned int rsvd_1 : 23;
  } b;
} RG_RX_A63_FIELD_T;

#define RG_RX_A64                                 (RF_D_RX_REG_BASE + 0x100)
// Bit 0           rg_wf_rx_rssipga_sca_en        U     RW        default = 'h1
// Bit 5   :1      rg_wf_rx_rssipga_cfix_sel      U     RW        default = 'h1a
// Bit 8   :6      rg_wf_rx_rssipga_ictrl         U     RW        default = 'h3
// Bit 11  :9      rg_wf_rx_rssipga_ictrl_agclock     U     RW        default = 'h0
// Bit 15  :12     rg_wf_rx_rssipga_drvctrl       U     RW        default = 'h3
// Bit 19  :16     rg_wf_rx_rssipga_drvctrl_agclock     U     RW        default = 'h0
// Bit 21  :20     rg_wf_rx_rssipga_drv           U     RW        default = 'h1
// Bit 23  :22     rg_wf_rx_rssipga_drv_agclock     U     RW        default = 'h0
// Bit 26  :24     rg_wf_rx_rssipga_vcmsel        U     RW        default = 'h4
// Bit 29  :27     rg_wf_rx_rssipga_gain          U     RW        default = 'h0
typedef union RG_RX_A64_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_RSSIPGA_SCA_EN : 1;
    unsigned int RG_WF_RX_RSSIPGA_CFIX_SEL : 5;
    unsigned int RG_WF_RX_RSSIPGA_ICTRL : 3;
    unsigned int RG_WF_RX_RSSIPGA_ICTRL_AGCLOCK : 3;
    unsigned int RG_WF_RX_RSSIPGA_DRVCTRL : 4;
    unsigned int RG_WF_RX_RSSIPGA_DRVCTRL_AGCLOCK : 4;
    unsigned int RG_WF_RX_RSSIPGA_DRV : 2;
    unsigned int RG_WF_RX_RSSIPGA_DRV_AGCLOCK : 2;
    unsigned int RG_WF_RX_RSSIPGA_VCMSEL : 3;
    unsigned int RG_WF_RX_RSSIPGA_GAIN : 3;
    unsigned int rsvd_0 : 2;
  } b;
} RG_RX_A64_FIELD_T;

#define RG_RX_A65                                 (RF_D_RX_REG_BASE + 0x104)
// Bit 0           rg_wf_rx_irrcalfrom_tx_en      U     RW        default = 'h1
// Bit 3   :1      rg_wf5g_rx_irrcal_buf_ictrl     U     RW        default = 'h3
// Bit 6   :4      rg_wf5g_rx_lna_cal_att         U     RW        default = 'h0
// Bit 9   :7      rg_wf2g_rx_irrcal_buf_ictrl     U     RW        default = 'h3
// Bit 12  :10     rg_wf2g_rx_lna_cal_att         U     RW        default = 'h2
// Bit 15  :13     rg_wf_rx_gm_vcmsel             U     RW        default = 'h4
typedef union RG_RX_A65_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_IRRCALFROM_TX_EN : 1;
    unsigned int RG_WF5G_RX_IRRCAL_BUF_ICTRL : 3;
    unsigned int RG_WF5G_RX_LNA_CAL_ATT : 3;
    unsigned int RG_WF2G_RX_IRRCAL_BUF_ICTRL : 3;
    unsigned int RG_WF2G_RX_LNA_CAL_ATT : 3;
    unsigned int RG_WF_RX_GM_VCMSEL : 3;
    unsigned int rsvd_0 : 16;
  } b;
} RG_RX_A65_FIELD_T;

#define RG_RX_A66                                 (RF_D_RX_REG_BASE + 0x108)
// Bit 1   :0      rg_wf5g_rx_dpd_att_ctrl        U     RW        default = 'h1
// Bit 4   :2      rg_wf5g_rx_dpd_gm_gain         U     RW        default = 'h7
// Bit 7   :5      rg_wf5g_rx_dpd_gm_ictrl        U     RW        default = 'h3
// Bit 11  :8      rg_wf5g_rx_dpd_mxr_lodrv_ictrl     U     RW        default = 'h3
// Bit 14  :12     rg_wf5g_rx_gm_ictrl            U     RW        default = 'h3
// Bit 18  :15     rg_wf5g_rx_mxr_lodrv_ictrl     U     RW        default = 'h3
// Bit 22  :19     rg_wf5g_rx_mxr_vbsel           U     RW        default = 'h2
// Bit 25  :23     rg_wf5g_rx_mxr_vcmsel          U     RW        default = 'h3
typedef union RG_RX_A66_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_RX_DPD_ATT_CTRL : 2;
    unsigned int RG_WF5G_RX_DPD_GM_GAIN : 3;
    unsigned int RG_WF5G_RX_DPD_GM_ICTRL : 3;
    unsigned int RG_WF5G_RX_DPD_MXR_LODRV_ICTRL : 4;
    unsigned int RG_WF5G_RX_GM_ICTRL : 3;
    unsigned int RG_WF5G_RX_MXR_LODRV_ICTRL : 4;
    unsigned int RG_WF5G_RX_MXR_VBSEL : 4;
    unsigned int RG_WF5G_RX_MXR_VCMSEL : 3;
    unsigned int rsvd_0 : 6;
  } b;
} RG_RX_A66_FIELD_T;

#define RG_RX_A67                                 (RF_D_RX_REG_BASE + 0x10c)
// Bit 1           rg_wf_rx_tia_sca_en            U     RW        default = 'h1
// Bit 16  :14     rg_wf_rx_tia_ictrl             U     RW        default = 'h3
// Bit 19  :17     rg_wf_rx_tia_drvctrl           U     RW        default = 'h3
typedef union RG_RX_A67_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_RX_TIA_SCA_EN : 1;
    unsigned int rsvd_1 : 12;
    unsigned int RG_WF_RX_TIA_ICTRL : 3;
    unsigned int RG_WF_RX_TIA_DRVCTRL : 3;
    unsigned int rsvd_2 : 12;
  } b;
} RG_RX_A67_FIELD_T;

#define RG_RX_A68                                 (RF_D_RX_REG_BASE + 0x110)
// Bit 0           rg_wf_rx_lpf_sra1_en           U     RW        default = 'h1
// Bit 1           rg_wf_rx_lpf_sra2_en           U     RW        default = 'h1
// Bit 2           rg_wf_rx_lpf_sra3_en           U     RW        default = 'h1
// Bit 3           rg_wf_rx_lpf_sra4_en           U     RW        default = 'h1
// Bit 4           rg_wf_rx_lpf_sca1_en           U     RW        default = 'h1
// Bit 5           rg_wf_rx_lpf_sca2_en           U     RW        default = 'h1
// Bit 6           rg_wf_rx_lpf_rn1_en            U     RW        default = 'h1
// Bit 7           rg_wf_rx_lpf_notch_en          U     RW        default = 'h1
// Bit 8           rg_wf_rx_lpf_op1_en            U     RW        default = 'h1
// Bit 9           rg_wf_rx_lpf_op2_en            U     RW        default = 'h1
// Bit 10          rg_wf5g_txlpbk2tia_en          U     RW        default = 'h0
// Bit 11          rg_wf2g_txlpbk2tia_en          U     RW        default = 'h0
// Bit 12          rg_wf_rx_adcbuf_en             U     RW        default = 'h0
// Bit 16  :13     rg_wf_rx_adcbuf_sra_sel        U     RW        default = 'h0
typedef union RG_RX_A68_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_SRA1_EN : 1;
    unsigned int RG_WF_RX_LPF_SRA2_EN : 1;
    unsigned int RG_WF_RX_LPF_SRA3_EN : 1;
    unsigned int RG_WF_RX_LPF_SRA4_EN : 1;
    unsigned int RG_WF_RX_LPF_SCA1_EN : 1;
    unsigned int RG_WF_RX_LPF_SCA2_EN : 1;
    unsigned int RG_WF_RX_LPF_RN1_EN : 1;
    unsigned int RG_WF_RX_LPF_NOTCH_EN : 1;
    unsigned int RG_WF_RX_LPF_OP1_EN : 1;
    unsigned int RG_WF_RX_LPF_OP2_EN : 1;
    unsigned int RG_WF5G_TXLPBK2TIA_EN : 1;
    unsigned int RG_WF2G_TXLPBK2TIA_EN : 1;
    unsigned int RG_WF_RX_ADCBUF_EN : 1;
    unsigned int RG_WF_RX_ADCBUF_SRA_SEL : 4;
    unsigned int rsvd_0 : 15;
  } b;
} RG_RX_A68_FIELD_T;

#define RG_RX_A69                                 (RF_D_RX_REG_BASE + 0x114)
// Bit 2   :0      rg_wf_rx_vcmsel                U     RW        default = 'h4
// Bit 5   :3      rg_wf_rx_lpf_op1_drvctrl       U     RW        default = 'h3
// Bit 9   :6      rg_wf_rx_lpf_op2_drvctrl       U     RW        default = 'h3
typedef union RG_RX_A69_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_VCMSEL : 3;
    unsigned int RG_WF_RX_LPF_OP1_DRVCTRL : 3;
    unsigned int RG_WF_RX_LPF_OP2_DRVCTRL : 4;
    unsigned int rsvd_0 : 22;
  } b;
} RG_RX_A69_FIELD_T;

#define RG_RX_A71                                 (RF_D_RX_REG_BASE + 0x11c)
// Bit 0           rg_wf_rx_fe_tp_en              U     RW        default = 'h0
// Bit 8   :1      rg_wf_rx_fe_tp_sel             U     RW        default = 'h0
// Bit 9           rg_wf_rtx_abb_tp_en            U     RW        default = 'h0
// Bit 17  :10     rg_wf_rtx_abb_tp_sel           U     RW        default = 'h0
// Bit 31  :18     rg_wf_rx_reserved8             U     RW        default = 'h0
typedef union RG_RX_A71_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_FE_TP_EN : 1;
    unsigned int RG_WF_RX_FE_TP_SEL : 8;
    unsigned int RG_WF_RTX_ABB_TP_EN : 1;
    unsigned int RG_WF_RTX_ABB_TP_SEL : 8;
    unsigned int RG_WF_RX_RESERVED8 : 14;
  } b;
} RG_RX_A71_FIELD_T;

#define RG_RX_A72                                 (RF_D_RX_REG_BASE + 0x120)
// Bit 31  :0      rg_wf_rx_rsv0                  U     RW        default = 'h0
typedef union RG_RX_A72_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_RSV0 : 32;
  } b;
} RG_RX_A72_FIELD_T;

#define RG_RX_A73                                 (RF_D_RX_REG_BASE + 0x124)
// Bit 31  :0      rg_wf_rx_rsv1                  U     RW        default = 'h0
typedef union RG_RX_A73_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_RSV1 : 32;
  } b;
} RG_RX_A73_FIELD_T;

#define RG_RX_A74                                 (RF_D_RX_REG_BASE + 0x128)
// Bit 2   :0      rg_wf_rx_lpf_dcoc_rsel_bw20m     U     RW        default = 'h3
// Bit 5   :3      rg_wf_rx_lpf_dcoc_rsel_bw40m     U     RW        default = 'h3
// Bit 8   :6      rg_wf_rx_lpf_dcoc_rsel_bw80m     U     RW        default = 'h0
// Bit 11  :9      rg_wf_rx_lpf_dcoc_rsel_bw160m     U     RW        default = 'h0
// Bit 14  :12     rg_wf_rx_lpf_dcoc_rgsel_bw20m     U     RW        default = 'h3
// Bit 17  :15     rg_wf_rx_lpf_dcoc_rgsel_bw40m     U     RW        default = 'h3
// Bit 20  :18     rg_wf_rx_lpf_dcoc_rgsel_bw80m     U     RW        default = 'h0
// Bit 23  :21     rg_wf_rx_lpf_dcoc_rgsel_bw160m     U     RW        default = 'h0
// Bit 26  :24     rg_wf_rx_lpf_dcoc_rsel_man     U     RW        default = 'h0
// Bit 27          rg_wf_rx_lpf_dcoc_rsel_man_mode     U     RW        default = 'h0
// Bit 30  :28     rg_wf_rx_lpf_dcoc_rgsel_man     U     RW        default = 'h0
// Bit 31          rg_wf_rx_lpf_dcoc_rgsel_man_mode     U     RW        default = 'h0
typedef union RG_RX_A74_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_DCOC_RSEL_BW20M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RSEL_BW40M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RSEL_BW80M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RSEL_BW160M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RGSEL_BW20M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RGSEL_BW40M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RGSEL_BW80M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RGSEL_BW160M : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RSEL_MAN : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RSEL_MAN_MODE : 1;
    unsigned int RG_WF_RX_LPF_DCOC_RGSEL_MAN : 3;
    unsigned int RG_WF_RX_LPF_DCOC_RGSEL_MAN_MODE : 1;
  } b;
} RG_RX_A74_FIELD_T;

#define RG_RX_A75                                 (RF_D_RX_REG_BASE + 0x12c)
// Bit 2   :0      rg_wf_rx_lpf_op1_ictrl_bw20m     U     RW        default = 'h1
// Bit 5   :3      rg_wf_rx_lpf_op1_ictrl_bw40m     U     RW        default = 'h2
// Bit 8   :6      rg_wf_rx_lpf_op1_ictrl_bw80m     U     RW        default = 'h3
// Bit 11  :9      rg_wf_rx_lpf_op1_ictrl_bw160m     U     RW        default = 'h4
// Bit 14  :12     rg_wf_rx_lpf_op1_ictrl_man     U     RW        default = 'h0
// Bit 15          rg_wf_rx_lpf_op1_ictrl_man_mode     U     RW        default = 'h0
// Bit 17  :16     rg_wf_rx_lpf_op1_drv_bw20m     U     RW        default = 'h0
// Bit 19  :18     rg_wf_rx_lpf_op1_drv_bw40m     U     RW        default = 'h1
// Bit 21  :20     rg_wf_rx_lpf_op1_drv_bw80m     U     RW        default = 'h2
// Bit 23  :22     rg_wf_rx_lpf_op1_drv_bw160m     U     RW        default = 'h3
// Bit 25  :24     rg_wf_rx_lpf_op1_drv_man       U     RW        default = 'h0
// Bit 26          rg_wf_rx_lpf_op1_drv_man_mode     U     RW        default = 'h0
typedef union RG_RX_A75_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_OP1_ICTRL_BW20M : 3;
    unsigned int RG_WF_RX_LPF_OP1_ICTRL_BW40M : 3;
    unsigned int RG_WF_RX_LPF_OP1_ICTRL_BW80M : 3;
    unsigned int RG_WF_RX_LPF_OP1_ICTRL_BW160M : 3;
    unsigned int RG_WF_RX_LPF_OP1_ICTRL_MAN : 3;
    unsigned int RG_WF_RX_LPF_OP1_ICTRL_MAN_MODE : 1;
    unsigned int RG_WF_RX_LPF_OP1_DRV_BW20M : 2;
    unsigned int RG_WF_RX_LPF_OP1_DRV_BW40M : 2;
    unsigned int RG_WF_RX_LPF_OP1_DRV_BW80M : 2;
    unsigned int RG_WF_RX_LPF_OP1_DRV_BW160M : 2;
    unsigned int RG_WF_RX_LPF_OP1_DRV_MAN : 2;
    unsigned int RG_WF_RX_LPF_OP1_DRV_MAN_MODE : 1;
    unsigned int rsvd_0 : 5;
  } b;
} RG_RX_A75_FIELD_T;

#define RG_RX_A76                                 (RF_D_RX_REG_BASE + 0x130)
// Bit 2   :0      rg_wf_rx_lpf_op2_ictrl_bw20m     U     RW        default = 'h1
// Bit 5   :3      rg_wf_rx_lpf_op2_ictrl_bw40m     U     RW        default = 'h2
// Bit 8   :6      rg_wf_rx_lpf_op2_ictrl_bw80m     U     RW        default = 'h3
// Bit 11  :9      rg_wf_rx_lpf_op2_ictrl_bw160m     U     RW        default = 'h4
// Bit 14  :12     rg_wf_rx_lpf_op2_ictrl_man     U     RW        default = 'h0
// Bit 15          rg_wf_rx_lpf_op2_ictrl_man_mode     U     RW        default = 'h0
// Bit 17  :16     rg_wf_rx_lpf_op2_drv_bw20m     U     RW        default = 'h1
// Bit 19  :18     rg_wf_rx_lpf_op2_drv_bw40m     U     RW        default = 'h1
// Bit 21  :20     rg_wf_rx_lpf_op2_drv_bw80m     U     RW        default = 'h1
// Bit 23  :22     rg_wf_rx_lpf_op2_drv_bw160m     U     RW        default = 'h2
// Bit 25  :24     rg_wf_rx_lpf_op2_drv_man       U     RW        default = 'h0
// Bit 26          rg_wf_rx_lpf_op2_drv_man_mode     U     RW        default = 'h0
typedef union RG_RX_A76_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_OP2_ICTRL_BW20M : 3;
    unsigned int RG_WF_RX_LPF_OP2_ICTRL_BW40M : 3;
    unsigned int RG_WF_RX_LPF_OP2_ICTRL_BW80M : 3;
    unsigned int RG_WF_RX_LPF_OP2_ICTRL_BW160M : 3;
    unsigned int RG_WF_RX_LPF_OP2_ICTRL_MAN : 3;
    unsigned int RG_WF_RX_LPF_OP2_ICTRL_MAN_MODE : 1;
    unsigned int RG_WF_RX_LPF_OP2_DRV_BW20M : 2;
    unsigned int RG_WF_RX_LPF_OP2_DRV_BW40M : 2;
    unsigned int RG_WF_RX_LPF_OP2_DRV_BW80M : 2;
    unsigned int RG_WF_RX_LPF_OP2_DRV_BW160M : 2;
    unsigned int RG_WF_RX_LPF_OP2_DRV_MAN : 2;
    unsigned int RG_WF_RX_LPF_OP2_DRV_MAN_MODE : 1;
    unsigned int rsvd_0 : 5;
  } b;
} RG_RX_A76_FIELD_T;

#define RG_RX_A77                                 (RF_D_RX_REG_BASE + 0x134)
// Bit 1   :0      rg_wf_rx_lpf_op1_rcc_lg        U     RW        default = 'h3
// Bit 3   :2      rg_wf_rx_lpf_op1_rcc_mg        U     RW        default = 'h2
// Bit 5   :4      rg_wf_rx_lpf_op1_rcc_hg        U     RW        default = 'h1
// Bit 7   :6      rg_wf_rx_lpf_op1_rcc_man       U     RW        default = 'h0
// Bit 8           rg_wf_rx_lpf_op1_rcc_man_mode     U     RW        default = 'h0
// Bit 10  :9      rg_wf_rx_lpf_op1_rcm_lg        U     RW        default = 'h1
// Bit 12  :11     rg_wf_rx_lpf_op1_rcm_mg        U     RW        default = 'h2
// Bit 14  :13     rg_wf_rx_lpf_op1_rcm_hg        U     RW        default = 'h3
// Bit 16  :15     rg_wf_rx_lpf_op1_rcm_man       U     RW        default = 'h0
// Bit 17          rg_wf_rx_lpf_op1_rcm_man_mode     U     RW        default = 'h0
// Bit 19  :18     rg_wf_rx_lpf_op2_rcc_bw20m     U     RW        default = 'h3
// Bit 21  :20     rg_wf_rx_lpf_op2_rcc_bw40m     U     RW        default = 'h2
// Bit 23  :22     rg_wf_rx_lpf_op2_rcc_bw80m     U     RW        default = 'h1
// Bit 25  :24     rg_wf_rx_lpf_op2_rcc_bw160m     U     RW        default = 'h1
// Bit 27  :26     rg_wf_rx_lpf_op2_rcc_man       U     RW        default = 'h0
// Bit 28          rg_wf_rx_lpf_op2_rcc_man_mode     U     RW        default = 'h0
typedef union RG_RX_A77_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_RX_LPF_OP1_RCC_LG : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCC_MG : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCC_HG : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCC_MAN : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCC_MAN_MODE : 1;
    unsigned int RG_WF_RX_LPF_OP1_RCM_LG : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCM_MG : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCM_HG : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCM_MAN : 2;
    unsigned int RG_WF_RX_LPF_OP1_RCM_MAN_MODE : 1;
    unsigned int RG_WF_RX_LPF_OP2_RCC_BW20M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCC_BW40M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCC_BW80M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCC_BW160M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCC_MAN : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCC_MAN_MODE : 1;
    unsigned int rsvd_0 : 3;
  } b;
} RG_RX_A77_FIELD_T;

#define RG_RX_A78                                 (RF_D_RX_REG_BASE + 0x138)
// Bit 10  :9      rg_wf_rx_lpf_op2_rcm_bw20m     U     RW        default = 'h3
// Bit 12  :11     rg_wf_rx_lpf_op2_rcm_bw40m     U     RW        default = 'h3
// Bit 14  :13     rg_wf_rx_lpf_op2_rcm_bw80m     U     RW        default = 'h3
// Bit 16  :15     rg_wf_rx_lpf_op2_rcm_bw160m     U     RW        default = 'h3
// Bit 18  :17     rg_wf_rx_lpf_op2_rcm_man       U     RW        default = 'h0
// Bit 19          rg_wf_rx_lpf_op2_rcm_man_mode     U     RW        default = 'h0
// Bit 31          rg_wf2g_rx_lna_slice_man_mode     U     RW        default = 'h0
typedef union RG_RX_A78_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 9;
    unsigned int RG_WF_RX_LPF_OP2_RCM_BW20M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCM_BW40M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCM_BW80M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCM_BW160M : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCM_MAN : 2;
    unsigned int RG_WF_RX_LPF_OP2_RCM_MAN_MODE : 1;
    unsigned int rsvd_1 : 11;
    unsigned int RG_WF2G_RX_LNA_SLICE_MAN_MODE : 1;
  } b;
} RG_RX_A78_FIELD_T;

#define RG_RX_A79                                 (RF_D_RX_REG_BASE + 0x13c)
// Bit 2   :0      rg_wf2g_rx_lna_ictrl_g1        U     RW        default = 'h3
// Bit 6   :4      rg_wf2g_rx_lna_ictrl_g2        U     RW        default = 'h4
// Bit 10  :8      rg_wf2g_rx_lna_ictrl_g3        U     RW        default = 'h4
// Bit 14  :12     rg_wf2g_rx_lna_ictrl_g4        U     RW        default = 'h4
// Bit 18  :16     rg_wf2g_rx_lna_ictrl_g5        U     RW        default = 'h2
// Bit 22  :20     rg_wf2g_rx_lna_ictrl_g6        U     RW        default = 'h2
// Bit 26  :24     rg_wf2g_rx_lna_ictrl_g7        U     RW        default = 'h3
// Bit 30  :28     rg_wf2g_rx_lna_ictrl_man       U     RW        default = 'h0
// Bit 31          rg_wf2g_rx_lna_ictrl_man_mode     U     RW        default = 'h0
typedef union RG_RX_A79_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_ICTRL_G1 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G2 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G3 : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G4 : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G5 : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G6 : 3;
    unsigned int rsvd_5 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G7 : 3;
    unsigned int rsvd_6 : 1;
    unsigned int RG_WF2G_RX_LNA_ICTRL_MAN : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_MAN_MODE : 1;
  } b;
} RG_RX_A79_FIELD_T;

#define RG_RX_A80                                 (RF_D_RX_REG_BASE + 0x140)
// Bit 7   :0      rg_wf2g_rx_lna_slice_g1        U     RW        default = 'h49
// Bit 15  :8      rg_wf2g_rx_lna_slice_g2        U     RW        default = 'h92
// Bit 23  :16     rg_wf2g_rx_lna_slice_g3        U     RW        default = 'h93
// Bit 31  :24     rg_wf2g_rx_lna_slice_g4        U     RW        default = 'ha
typedef union RG_RX_A80_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_SLICE_G1 : 8;
    unsigned int RG_WF2G_RX_LNA_SLICE_G2 : 8;
    unsigned int RG_WF2G_RX_LNA_SLICE_G3 : 8;
    unsigned int RG_WF2G_RX_LNA_SLICE_G4 : 8;
  } b;
} RG_RX_A80_FIELD_T;

#define RG_RX_A81                                 (RF_D_RX_REG_BASE + 0x144)
// Bit 7   :0      rg_wf2g_rx_lna_slice_g5        U     RW        default = 'h1a
// Bit 15  :8      rg_wf2g_rx_lna_slice_g6        U     RW        default = 'h7b
// Bit 23  :16     rg_wf2g_rx_lna_slice_g7        U     RW        default = 'hfa
// Bit 31  :24     rg_wf2g_rx_lna_slice_man       U     RW        default = 'h0
typedef union RG_RX_A81_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_SLICE_G5 : 8;
    unsigned int RG_WF2G_RX_LNA_SLICE_G6 : 8;
    unsigned int RG_WF2G_RX_LNA_SLICE_G7 : 8;
    unsigned int RG_WF2G_RX_LNA_SLICE_MAN : 8;
  } b;
} RG_RX_A81_FIELD_T;

#define RG_RX_A82                                 (RF_D_RX_REG_BASE + 0x148)
// Bit 4   :0      rg_wf2g_rx_lna_rfb_g1          U     RW        default = 'h0
// Bit 9   :5      rg_wf2g_rx_lna_rfb_g2          U     RW        default = 'h0
// Bit 14  :10     rg_wf2g_rx_lna_rfb_g3          U     RW        default = 'h0
// Bit 19  :15     rg_wf2g_rx_lna_rfb_g4          U     RW        default = 'h2
// Bit 24  :20     rg_wf2g_rx_lna_rfb_g5          U     RW        default = 'h4
// Bit 29  :25     rg_wf2g_rx_lna_rfb_g6          U     RW        default = 'h8
typedef union RG_RX_A82_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_RFB_G1 : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_G2 : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_G3 : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_G4 : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_G5 : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_G6 : 5;
    unsigned int rsvd_0 : 2;
  } b;
} RG_RX_A82_FIELD_T;

#define RG_RX_A83                                 (RF_D_RX_REG_BASE + 0x14c)
// Bit 4   :0      rg_wf2g_rx_lna_rfb_g7          U     RW        default = 'h0
// Bit 9   :5      rg_wf2g_rx_lna_rfb_man         U     RW        default = 'h0
// Bit 10          rg_wf2g_rx_lna_rfb_man_mode     U     RW        default = 'h0
typedef union RG_RX_A83_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_RFB_G7 : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_MAN : 5;
    unsigned int RG_WF2G_RX_LNA_RFB_MAN_MODE : 1;
    unsigned int rsvd_0 : 21;
  } b;
} RG_RX_A83_FIELD_T;

#define RG_RX_A84                                 (RF_D_RX_REG_BASE + 0x150)
// Bit 2   :0      rg_wf2g_rx_lna_rt_g1           U     RW        default = 'h5
// Bit 5   :3      rg_wf2g_rx_lna_rt_g2           U     RW        default = 'h5
// Bit 8   :6      rg_wf2g_rx_lna_rt_g3           U     RW        default = 'h5
// Bit 11  :9      rg_wf2g_rx_lna_rt_g4           U     RW        default = 'h5
// Bit 14  :12     rg_wf2g_rx_lna_rt_g5           U     RW        default = 'h5
// Bit 17  :15     rg_wf2g_rx_lna_rt_g6           U     RW        default = 'h0
// Bit 26  :24     rg_wf2g_rx_lna_rt_man          U     RW        default = 'h0
// Bit 31          rg_wf2g_rx_lna_rt_man_mode     U     RW        default = 'h0
typedef union RG_RX_A84_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_RT_G1 : 3;
    unsigned int RG_WF2G_RX_LNA_RT_G2 : 3;
    unsigned int RG_WF2G_RX_LNA_RT_G3 : 3;
    unsigned int RG_WF2G_RX_LNA_RT_G4 : 3;
    unsigned int RG_WF2G_RX_LNA_RT_G5 : 3;
    unsigned int RG_WF2G_RX_LNA_RT_G6 : 3;
    unsigned int rsvd_0 : 6;
    unsigned int RG_WF2G_RX_LNA_RT_MAN : 3;
    unsigned int rsvd_1 : 4;
    unsigned int RG_WF2G_RX_LNA_RT_MAN_MODE : 1;
  } b;
} RG_RX_A84_FIELD_T;

#define RG_RX_A85                                 (RF_D_RX_REG_BASE + 0x154)
// Bit 2   :0      rg_wf2g_rx_lna_att_g1          U     RW        default = 'h0
// Bit 5   :3      rg_wf2g_rx_lna_att_g2          U     RW        default = 'h2
// Bit 8   :6      rg_wf2g_rx_lna_att_g3          U     RW        default = 'h3
// Bit 11  :9      rg_wf2g_rx_lna_att_man         U     RW        default = 'h0
// Bit 12          rg_wf2g_rx_lna_att_man_mode     U     RW        default = 'h0
typedef union RG_RX_A85_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_ATT_G1 : 3;
    unsigned int RG_WF2G_RX_LNA_ATT_G2 : 3;
    unsigned int RG_WF2G_RX_LNA_ATT_G3 : 3;
    unsigned int RG_WF2G_RX_LNA_ATT_MAN : 3;
    unsigned int RG_WF2G_RX_LNA_ATT_MAN_MODE : 1;
    unsigned int rsvd_0 : 19;
  } b;
} RG_RX_A85_FIELD_T;

#define RG_RX_A86                                 (RF_D_RX_REG_BASE + 0x158)
// Bit 1   :0      rg_wf2g_rx_dpd_att_ctrl        U     RW        default = 'h1
// Bit 4   :2      rg_wf2g_rx_dpd_gm_gain         U     RW        default = 'h7
// Bit 7   :5      rg_wf2g_rx_dpd_gm_ictrl        U     RW        default = 'h3
// Bit 11  :8      rg_wf2g_rx_dpd_mxr_lodrv_ictrl     U     RW        default = 'h3
// Bit 14  :12     rg_wf2g_rx_gm_ictrl            U     RW        default = 'h3
// Bit 18  :15     rg_wf2g_rx_mxr_lodrv_ictrl     U     RW        default = 'h3
// Bit 22  :19     rg_wf2g_rx_mxr_vbsel           U     RW        default = 'h2
// Bit 25  :23     rg_wf2g_rx_mxr_vcmsel          U     RW        default = 'h3
typedef union RG_RX_A86_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_DPD_ATT_CTRL : 2;
    unsigned int RG_WF2G_RX_DPD_GM_GAIN : 3;
    unsigned int RG_WF2G_RX_DPD_GM_ICTRL : 3;
    unsigned int RG_WF2G_RX_DPD_MXR_LODRV_ICTRL : 4;
    unsigned int RG_WF2G_RX_GM_ICTRL : 3;
    unsigned int RG_WF2G_RX_MXR_LODRV_ICTRL : 4;
    unsigned int RG_WF2G_RX_MXR_VBSEL : 4;
    unsigned int RG_WF2G_RX_MXR_VCMSEL : 3;
    unsigned int rsvd_0 : 6;
  } b;
} RG_RX_A86_FIELD_T;

#define RG_RX_A87                                 (RF_D_RX_REG_BASE + 0x15c)
// Bit 7   :0      rg_wf2g_rx_lna_priority        U     RW        default = 'hdb
typedef union RG_RX_A87_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_PRIORITY : 8;
    unsigned int rsvd_0 : 24;
  } b;
} RG_RX_A87_FIELD_T;

#define RG_RX_A88                                 (RF_D_RX_REG_BASE + 0x160)
// Bit 0           rg_m4_wf_rx_fe_topbias_en      U     RW        default = 'h1
// Bit 1           rg_m4_wf_rx_lna_bias_en        U     RW        default = 'h1
// Bit 2           rg_m4_wf_rx_gm_bias_en         U     RW        default = 'h1
// Bit 3           rg_m4_wf_rx_mxr_bias_en        U     RW        default = 'h1
// Bit 4           rg_m4_wf5g_rx_lna_trsw_en      U     RW        default = 'h1
// Bit 5           rg_m4_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m4_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m4_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m4_wf5g_rx_dpd_en           U     RW        default = 'h1
// Bit 9           rg_m4_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m4_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m4_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m4_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m4_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m4_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m4_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m4_wf_rtx_abb_topbias_en     U     RW        default = 'h1
// Bit 18          rg_m4_wf_rtx_abb_rx_en         U     RW        default = 'h1
// Bit 22          rg_m4_wf_rx_tia_bias_en        U     RW        default = 'h1
// Bit 23          rg_m4_wf_rx_rssipga_bias_en     U     RW        default = 'h1
// Bit 24          rg_m4_wf_rx_lpf_bias_en        U     RW        default = 'h1
// Bit 25          rg_m4_wf_rx_tia_en             U     RW        default = 'h1
// Bit 26          rg_m4_wf_rx_rssipga_en         U     RW        default = 'h1
// Bit 27          rg_m4_wf_rx_lpf_en             U     RW        default = 'h1
// Bit 28          rg_m4_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h1
// Bit 29          rg_m4_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h1
typedef union RG_RX_A88_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M4_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M4_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M4_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M4_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M4_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M4_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M4_WF5G_RX_GM_EN : 1;
    unsigned int RG_M4_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M4_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M4_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M4_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M4_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M4_WF2G_RX_GM_EN : 1;
    unsigned int RG_M4_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M4_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M4_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M4_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M4_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M4_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M4_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M4_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M4_WF_RX_TIA_EN : 1;
    unsigned int RG_M4_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M4_WF_RX_LPF_EN : 1;
    unsigned int RG_M4_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M4_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A88_FIELD_T;

#define RG_RX_A89                                 (RF_D_RX_REG_BASE + 0x164)
// Bit 0           rg_m5_wf_rx_fe_topbias_en      U     RW        default = 'h0
// Bit 1           rg_m5_wf_rx_lna_bias_en        U     RW        default = 'h0
// Bit 2           rg_m5_wf_rx_gm_bias_en         U     RW        default = 'h0
// Bit 3           rg_m5_wf_rx_mxr_bias_en        U     RW        default = 'h0
// Bit 4           rg_m5_wf5g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 5           rg_m5_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m5_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m5_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m5_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m5_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m5_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m5_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m5_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m5_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m5_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m5_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m5_wf_rtx_abb_topbias_en     U     RW        default = 'h0
// Bit 18          rg_m5_wf_rtx_abb_rx_en         U     RW        default = 'h0
// Bit 22          rg_m5_wf_rx_tia_bias_en        U     RW        default = 'h0
// Bit 23          rg_m5_wf_rx_rssipga_bias_en     U     RW        default = 'h0
// Bit 24          rg_m5_wf_rx_lpf_bias_en        U     RW        default = 'h0
// Bit 25          rg_m5_wf_rx_tia_en             U     RW        default = 'h0
// Bit 26          rg_m5_wf_rx_rssipga_en         U     RW        default = 'h0
// Bit 27          rg_m5_wf_rx_lpf_en             U     RW        default = 'h0
// Bit 28          rg_m5_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h0
// Bit 29          rg_m5_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h0
typedef union RG_RX_A89_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M5_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M5_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M5_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M5_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M5_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M5_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M5_WF5G_RX_GM_EN : 1;
    unsigned int RG_M5_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M5_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M5_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M5_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M5_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M5_WF2G_RX_GM_EN : 1;
    unsigned int RG_M5_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M5_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M5_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M5_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M5_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M5_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M5_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M5_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M5_WF_RX_TIA_EN : 1;
    unsigned int RG_M5_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M5_WF_RX_LPF_EN : 1;
    unsigned int RG_M5_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M5_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A89_FIELD_T;

#define RG_RX_A90                                 (RF_D_RX_REG_BASE + 0x168)
// Bit 0           rg_m6_wf_rx_fe_topbias_en      U     RW        default = 'h1
// Bit 1           rg_m6_wf_rx_lna_bias_en        U     RW        default = 'h0
// Bit 2           rg_m6_wf_rx_gm_bias_en         U     RW        default = 'h0
// Bit 3           rg_m6_wf_rx_mxr_bias_en        U     RW        default = 'h0
// Bit 4           rg_m6_wf5g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 5           rg_m6_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m6_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m6_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m6_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m6_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m6_wf2g_rx_lna_trsw_en      U     RW        default = 'h1
// Bit 11          rg_m6_wf2g_rx_lna_en           U     RW        default = 'h0
// Bit 12          rg_m6_wf2g_rx_gm_en            U     RW        default = 'h0
// Bit 13          rg_m6_wf2g_rx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m6_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m6_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m6_wf_rtx_abb_topbias_en     U     RW        default = 'h1
// Bit 18          rg_m6_wf_rtx_abb_rx_en         U     RW        default = 'h0
// Bit 22          rg_m6_wf_rx_tia_bias_en        U     RW        default = 'h0
// Bit 23          rg_m6_wf_rx_rssipga_bias_en     U     RW        default = 'h0
// Bit 24          rg_m6_wf_rx_lpf_bias_en        U     RW        default = 'h0
// Bit 25          rg_m6_wf_rx_tia_en             U     RW        default = 'h0
// Bit 26          rg_m6_wf_rx_rssipga_en         U     RW        default = 'h0
// Bit 27          rg_m6_wf_rx_lpf_en             U     RW        default = 'h0
// Bit 28          rg_m6_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h0
// Bit 29          rg_m6_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h0
typedef union RG_RX_A90_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M6_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M6_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M6_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M6_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M6_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M6_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M6_WF5G_RX_GM_EN : 1;
    unsigned int RG_M6_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M6_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M6_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M6_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M6_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M6_WF2G_RX_GM_EN : 1;
    unsigned int RG_M6_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M6_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M6_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M6_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M6_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M6_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M6_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M6_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M6_WF_RX_TIA_EN : 1;
    unsigned int RG_M6_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M6_WF_RX_LPF_EN : 1;
    unsigned int RG_M6_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M6_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A90_FIELD_T;

#define RG_RX_A91                                 (RF_D_RX_REG_BASE + 0x16c)
// Bit 0           rg_m7_wf_rx_fe_topbias_en      U     RW        default = 'h1
// Bit 1           rg_m7_wf_rx_lna_bias_en        U     RW        default = 'h1
// Bit 2           rg_m7_wf_rx_gm_bias_en         U     RW        default = 'h1
// Bit 3           rg_m7_wf_rx_mxr_bias_en        U     RW        default = 'h1
// Bit 4           rg_m7_wf5g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 5           rg_m7_wf5g_rx_lna_en           U     RW        default = 'h0
// Bit 6           rg_m7_wf5g_rx_gm_en            U     RW        default = 'h0
// Bit 7           rg_m7_wf5g_rx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m7_wf5g_rx_dpd_en           U     RW        default = 'h0
// Bit 9           rg_m7_wf5g_rx_irrcal_en        U     RW        default = 'h0
// Bit 10          rg_m7_wf2g_rx_lna_trsw_en      U     RW        default = 'h0
// Bit 11          rg_m7_wf2g_rx_lna_en           U     RW        default = 'h1
// Bit 12          rg_m7_wf2g_rx_gm_en            U     RW        default = 'h1
// Bit 13          rg_m7_wf2g_rx_mxr_en           U     RW        default = 'h1
// Bit 14          rg_m7_wf2g_rx_irrcal_en        U     RW        default = 'h0
// Bit 15          rg_m7_wf2g_rx_dpd_en           U     RW        default = 'h0
// Bit 17          rg_m7_wf_rtx_abb_topbias_en     U     RW        default = 'h1
// Bit 18          rg_m7_wf_rtx_abb_rx_en         U     RW        default = 'h1
// Bit 22          rg_m7_wf_rx_tia_bias_en        U     RW        default = 'h1
// Bit 23          rg_m7_wf_rx_rssipga_bias_en     U     RW        default = 'h1
// Bit 24          rg_m7_wf_rx_lpf_bias_en        U     RW        default = 'h1
// Bit 25          rg_m7_wf_rx_tia_en             U     RW        default = 'h1
// Bit 26          rg_m7_wf_rx_rssipga_en         U     RW        default = 'h1
// Bit 27          rg_m7_wf_rx_lpf_en             U     RW        default = 'h1
// Bit 28          rg_m7_wf_rx_lpf_i_dcoc_en      U     RW        default = 'h1
// Bit 29          rg_m7_wf_rx_lpf_q_dcoc_en      U     RW        default = 'h1
typedef union RG_RX_A91_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M7_WF_RX_FE_TOPBIAS_EN : 1;
    unsigned int RG_M7_WF_RX_LNA_BIAS_EN : 1;
    unsigned int RG_M7_WF_RX_GM_BIAS_EN : 1;
    unsigned int RG_M7_WF_RX_MXR_BIAS_EN : 1;
    unsigned int RG_M7_WF5G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M7_WF5G_RX_LNA_EN : 1;
    unsigned int RG_M7_WF5G_RX_GM_EN : 1;
    unsigned int RG_M7_WF5G_RX_MXR_EN : 1;
    unsigned int RG_M7_WF5G_RX_DPD_EN : 1;
    unsigned int RG_M7_WF5G_RX_IRRCAL_EN : 1;
    unsigned int RG_M7_WF2G_RX_LNA_TRSW_EN : 1;
    unsigned int RG_M7_WF2G_RX_LNA_EN : 1;
    unsigned int RG_M7_WF2G_RX_GM_EN : 1;
    unsigned int RG_M7_WF2G_RX_MXR_EN : 1;
    unsigned int RG_M7_WF2G_RX_IRRCAL_EN : 1;
    unsigned int RG_M7_WF2G_RX_DPD_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_M7_WF_RTX_ABB_TOPBIAS_EN : 1;
    unsigned int RG_M7_WF_RTX_ABB_RX_EN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_M7_WF_RX_TIA_BIAS_EN : 1;
    unsigned int RG_M7_WF_RX_RSSIPGA_BIAS_EN : 1;
    unsigned int RG_M7_WF_RX_LPF_BIAS_EN : 1;
    unsigned int RG_M7_WF_RX_TIA_EN : 1;
    unsigned int RG_M7_WF_RX_RSSIPGA_EN : 1;
    unsigned int RG_M7_WF_RX_LPF_EN : 1;
    unsigned int RG_M7_WF_RX_LPF_I_DCOC_EN : 1;
    unsigned int RG_M7_WF_RX_LPF_Q_DCOC_EN : 1;
    unsigned int rsvd_2 : 2;
  } b;
} RG_RX_A91_FIELD_T;

#define RG_RX_A92                                 (RF_D_RX_REG_BASE + 0x170)
// Bit 2   :0      rg_wf2g_rx_lna_ictrl_g67       U     RW        default = 'h3
// Bit 5   :3      rg_wf2g_rx_lna_ictrl_g57       U     RW        default = 'h2
// Bit 8   :6      rg_wf2g_rx_lna_ictrl_g47       U     RW        default = 'h2
// Bit 11  :9      rg_wf2g_rx_lna_ictrl_g37       U     RW        default = 'h5
// Bit 14  :12     rg_wf2g_rx_lna_ictrl_g56       U     RW        default = 'h3
// Bit 17  :15     rg_wf2g_rx_lna_ictrl_g46       U     RW        default = 'h2
// Bit 20  :18     rg_wf2g_rx_lna_ictrl_g36       U     RW        default = 'h5
// Bit 23  :21     rg_wf2g_rx_lna_ictrl_g45       U     RW        default = 'h3
// Bit 26  :24     rg_wf2g_rx_lna_ictrl_g35       U     RW        default = 'h3
// Bit 29  :27     rg_wf2g_rx_lna_ictrl_g34       U     RW        default = 'h2
typedef union RG_RX_A92_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_ICTRL_G67 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G57 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G47 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G37 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G56 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G46 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G36 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G45 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G35 : 3;
    unsigned int RG_WF2G_RX_LNA_ICTRL_G34 : 3;
    unsigned int rsvd_0 : 2;
  } b;
} RG_RX_A92_FIELD_T;

#define RG_RX_A93                                 (RF_D_RX_REG_BASE + 0x174)
// Bit 4   :0      rg_wf2g_rx_lna_rfb_g77         U     RW        default = 'h0
// Bit 9   :5      rg_wf2g_btrx_lna_rfb_g77       U     RW        default = 'h0
// Bit 11  :10     rg_wf_agc_lock_option          U     RW        default = 'h3
// Bit 23  :16     rg_wf_rx_lna_en_delay_bnd      U     RW        default = 'h14
// Bit 24          rg_wf_rx_lna_en_delay_bypass     U     RW        default = 'h0
typedef union RG_RX_A93_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_RX_LNA_RFB_G77 : 5;
    unsigned int RG_WF2G_BTRX_LNA_RFB_G77 : 5;
    unsigned int RG_WF_AGC_LOCK_OPTION : 2;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_RX_LNA_EN_DELAY_BND : 8;
    unsigned int RG_WF_RX_LNA_EN_DELAY_BYPASS : 1;
    unsigned int rsvd_1 : 7;
  } b;
} RG_RX_A93_FIELD_T;

#define RG_RX_A94                                 (RF_D_RX_REG_BASE + 0x178)
// Bit 12          rg_wf_daclpbk2adc_en           U     RW        default = 'h0
// Bit 13          rg_wf_rx_lpf_testout_en        U     RW        default = 'h0
// Bit 14          rg_wf_rx_rssipga_testout_en     U     RW        default = 'h0
typedef union RG_RX_A94_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 12;
    unsigned int RG_WF_DACLPBK2ADC_EN : 1;
    unsigned int RG_WF_RX_LPF_TESTOUT_EN : 1;
    unsigned int RG_WF_RX_RSSIPGA_TESTOUT_EN : 1;
    unsigned int rsvd_1 : 17;
  } b;
} RG_RX_A94_FIELD_T;

#endif

