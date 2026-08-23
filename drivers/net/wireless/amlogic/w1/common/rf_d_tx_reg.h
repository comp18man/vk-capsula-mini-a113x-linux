#ifdef RF_D_TX_REG
#else
#define RF_D_TX_REG


#define RF_D_TX_REG_BASE                          (0x800)

#define RG_TX_A0                                  (RF_D_TX_REG_BASE + 0x0)
// Bit 2   :0      rg_wf5g_tx_pa_vgcg0            U     RW        default = 'h4
// Bit 6   :4      rg_wf5g_tx_mxr_vgcg1           U     RW        default = 'h3
// Bit 10  :8      rg_wf5g_tx_mxr_lo_vb           U     RW        default = 'h3
// Bit 14  :12     rg_wf5g_tx_mxr_lobuf_ict       U     RW        default = 'h4
typedef union RG_TX_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_TX_PA_VGCG0 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_TX_MXR_VGCG1 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_TX_MXR_LO_VB : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_TX_MXR_LOBUF_ICT : 3;
    unsigned int rsvd_3 : 17;
  } b;
} RG_TX_A0_FIELD_T;

#define RG_TX_A1                                  (RF_D_TX_REG_BASE + 0x4)
// Bit 2   :0      rg_wf5g_tx_adb_cbank           U     RW        default = 'h7
// Bit 6   :3      rg_wf5g_tx_adb_dc              U     RW        default = 'h4
// Bit 10  :7      rg_wf5g_tx_adb_vb_ct           U     RW        default = 'h0
// Bit 11          rg_wf5g_tx_adb_en              U     RW        default = 'h1
// Bit 12          rg_wf5g_tx_adb_ed_en           U     RW        default = 'h1
// Bit 16  :13     rg_wf5g_tx_adb_ed_gain         U     RW        default = 'h4
// Bit 19  :17     rg_wf5g_tx_adb_icgm            U     RW        default = 'h3
// Bit 22  :20     rg_wf5g_tx_adb_iptat           U     RW        default = 'h0
// Bit 25  :23     rg_wf5g_tx_ib_adj              U     RW        default = 'h0
typedef union RG_TX_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_TX_ADB_CBANK : 3;
    unsigned int RG_WF5G_TX_ADB_DC : 4;
    unsigned int RG_WF5G_TX_ADB_VB_CT : 4;
    unsigned int RG_WF5G_TX_ADB_EN : 1;
    unsigned int RG_WF5G_TX_ADB_ED_EN : 1;
    unsigned int RG_WF5G_TX_ADB_ED_GAIN : 4;
    unsigned int RG_WF5G_TX_ADB_ICGM : 3;
    unsigned int RG_WF5G_TX_ADB_IPTAT : 3;
    unsigned int RG_WF5G_TX_IB_ADJ : 3;
    unsigned int rsvd_0 : 6;
  } b;
} RG_TX_A1_FIELD_T;

#define RG_TX_A2                                  (RF_D_TX_REG_BASE + 0x8)
// Bit 0           rg_wf5g_tx_tssi_en             U     RW        default = 'h0
// Bit 3   :1      rg_wf5g_tx_tssi_attn           U     RW        default = 'h7
// Bit 6   :4      rg_wf5g_tx_tssi_lo_vb          U     RW        default = 'h3
// Bit 7           rg_wf5g_tx_iqcal_en            U     RW        default = 'h0
// Bit 8           rg_wf5g_tx_rxiqcal_en          U     RW        default = 'h0
// Bit 16  :9      rg_wf5g_tx_tp_sel              U     RW        default = 'h0
// Bit 17          rg_wf5g_tx_tp_en               U     RW        default = 'h0
// Bit 20  :18     rg_wf5g_tx_pa_gain_man         U     RW        default = 'h7
// Bit 23  :21     rg_wf5g_tx_mxr_gain_man        U     RW        default = 'h7
// Bit 24          rg_wf5g_tx_man_mode            U     RW        default = 'h0
typedef union RG_TX_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_TX_TSSI_EN : 1;
    unsigned int RG_WF5G_TX_TSSI_ATTN : 3;
    unsigned int RG_WF5G_TX_TSSI_LO_VB : 3;
    unsigned int RG_WF5G_TX_IQCAL_EN : 1;
    unsigned int RG_WF5G_TX_RXIQCAL_EN : 1;
    unsigned int RG_WF5G_TX_TP_SEL : 8;
    unsigned int RG_WF5G_TX_TP_EN : 1;
    unsigned int RG_WF5G_TX_PA_GAIN_MAN : 3;
    unsigned int RG_WF5G_TX_MXR_GAIN_MAN : 3;
    unsigned int RG_WF5G_TX_MAN_MODE : 1;
    unsigned int rsvd_0 : 7;
  } b;
} RG_TX_A2_FIELD_T;

#define RG_TX_A3                                  (RF_D_TX_REG_BASE + 0xc)
// Bit 0           rg_m0_wf5g_tx_pa_en            U     RW        default = 'h0
// Bit 1           rg_m0_wf5g_tx_mxr_en           U     RW        default = 'h0
// Bit 2           rg_m0_wf5g_tx_ib_en            U     RW        default = 'h0
// Bit 3           rg_m1_wf5g_tx_pa_en            U     RW        default = 'h0
// Bit 4           rg_m1_wf5g_tx_mxr_en           U     RW        default = 'h0
// Bit 5           rg_m1_wf5g_tx_ib_en            U     RW        default = 'h0
// Bit 6           rg_m2_wf5g_tx_pa_en            U     RW        default = 'h1
// Bit 7           rg_m2_wf5g_tx_mxr_en           U     RW        default = 'h1
// Bit 8           rg_m2_wf5g_tx_ib_en            U     RW        default = 'h1
// Bit 9           rg_m3_wf5g_tx_pa_en            U     RW        default = 'h0
// Bit 10          rg_m3_wf5g_tx_mxr_en           U     RW        default = 'h0
// Bit 11          rg_m3_wf5g_tx_ib_en            U     RW        default = 'h0
// Bit 31  :16     rg_wf5g_tx_rsv                 U     RW        default = 'h0
typedef union RG_TX_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M0_WF5G_TX_PA_EN : 1;
    unsigned int RG_M0_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M0_WF5G_TX_IB_EN : 1;
    unsigned int RG_M1_WF5G_TX_PA_EN : 1;
    unsigned int RG_M1_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M1_WF5G_TX_IB_EN : 1;
    unsigned int RG_M2_WF5G_TX_PA_EN : 1;
    unsigned int RG_M2_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M2_WF5G_TX_IB_EN : 1;
    unsigned int RG_M3_WF5G_TX_PA_EN : 1;
    unsigned int RG_M3_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M3_WF5G_TX_IB_EN : 1;
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF5G_TX_RSV : 16;
  } b;
} RG_TX_A3_FIELD_T;

#define RG_TX_A4                                  (RF_D_TX_REG_BASE + 0x10)
// Bit 2   :0      rg_wf2g_tx_pa_vgcg0            U     RW        default = 'h7
// Bit 7   :3      rg_wf2g_tx_mxr_cbank           U     RW        default = 'h4
// Bit 10  :8      rg_wf2g_tx_mxr_vgcg1           U     RW        default = 'h7
// Bit 13  :11     rg_wf2g_tx_mxr_lo_vb           U     RW        default = 'h7
// Bit 16  :14     rg_wf2g_tx_mxr_lobuf_ict       U     RW        default = 'h1
// Bit 20  :17     rg_wf2g_tx_var_u               U     RW        default = 'h8
// Bit 24  :21     rg_wf2g_tx_var_d               U     RW        default = 'h8
typedef union RG_TX_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_TX_PA_VGCG0 : 3;
    unsigned int RG_WF2G_TX_MXR_CBANK : 5;
    unsigned int RG_WF2G_TX_MXR_VGCG1 : 3;
    unsigned int RG_WF2G_TX_MXR_LO_VB : 3;
    unsigned int RG_WF2G_TX_MXR_LOBUF_ICT : 3;
    unsigned int RG_WF2G_TX_VAR_U : 4;
    unsigned int RG_WF2G_TX_VAR_D : 4;
    unsigned int rsvd_0 : 7;
  } b;
} RG_TX_A4_FIELD_T;

#define RG_TX_A5                                  (RF_D_TX_REG_BASE + 0x14)
// Bit 2   :0      rg_wf2g_tx_adb_cbank           U     RW        default = 'h3
// Bit 6   :3      rg_wf2g_tx_adb_dcib            U     RW        default = 'h0
// Bit 10  :7      rg_wf2g_tx_adb_dcvb            U     RW        default = 'h0
// Bit 11          rg_wf2g_tx_adb_en              U     RW        default = 'h1
// Bit 12          rg_wf2g_tx_adb_ed_en           U     RW        default = 'h1
// Bit 16  :13     rg_wf2g_tx_adb_ed_gain         U     RW        default = 'h3
// Bit 19  :17     rg_wf2g_tx_adb_icgm            U     RW        default = 'h2
// Bit 22  :20     rg_wf2g_tx_adb_iptat           U     RW        default = 'h2
// Bit 25  :23     rg_wf2g_tx_ib_adj              U     RW        default = 'h3
// Bit 26          rg_wf2g_tx_tssi_en             U     RW        default = 'h0
// Bit 29  :27     rg_wf2g_tx_tssi_attn           U     RW        default = 'h7
typedef union RG_TX_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_TX_ADB_CBANK : 3;
    unsigned int RG_WF2G_TX_ADB_DCIB : 4;
    unsigned int RG_WF2G_TX_ADB_DCVB : 4;
    unsigned int RG_WF2G_TX_ADB_EN : 1;
    unsigned int RG_WF2G_TX_ADB_ED_EN : 1;
    unsigned int RG_WF2G_TX_ADB_ED_GAIN : 4;
    unsigned int RG_WF2G_TX_ADB_ICGM : 3;
    unsigned int RG_WF2G_TX_ADB_IPTAT : 3;
    unsigned int RG_WF2G_TX_IB_ADJ : 3;
    unsigned int RG_WF2G_TX_TSSI_EN : 1;
    unsigned int RG_WF2G_TX_TSSI_ATTN : 3;
    unsigned int rsvd_0 : 2;
  } b;
} RG_TX_A5_FIELD_T;

#define RG_TX_A6                                  (RF_D_TX_REG_BASE + 0x18)
// Bit 2   :0      rg_wf2g_tx_tssi_lo_vb          U     RW        default = 'h3
// Bit 3           rg_wf2g_tx_iqcal_en            U     RW        default = 'h0
// Bit 4           rg_wf2g_tx_rxiqcal_en          U     RW        default = 'h0
// Bit 12  :5      rg_wf2g_tx_tp_sel              U     RW        default = 'h0
// Bit 13          rg_wf2g_tx_tp_en               U     RW        default = 'h0
// Bit 16  :14     rg_wf2g_tx_pa_gain_man         U     RW        default = 'h7
// Bit 19  :17     rg_wf2g_tx_mxr_gain_man        U     RW        default = 'h7
// Bit 20          rg_wf2g_tx_man_mode            U     RW        default = 'h0
typedef union RG_TX_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_TX_TSSI_LO_VB : 3;
    unsigned int RG_WF2G_TX_IQCAL_EN : 1;
    unsigned int RG_WF2G_TX_RXIQCAL_EN : 1;
    unsigned int RG_WF2G_TX_TP_SEL : 8;
    unsigned int RG_WF2G_TX_TP_EN : 1;
    unsigned int RG_WF2G_TX_PA_GAIN_MAN : 3;
    unsigned int RG_WF2G_TX_MXR_GAIN_MAN : 3;
    unsigned int RG_WF2G_TX_MAN_MODE : 1;
    unsigned int rsvd_0 : 11;
  } b;
} RG_TX_A6_FIELD_T;

#define RG_TX_A7                                  (RF_D_TX_REG_BASE + 0x1c)
// Bit 0           rg_m0_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 1           rg_m0_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 2           rg_m0_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 3           rg_m0_wf2g_tx_ib_en            U     RW        default = 'h0
// Bit 4           rg_m1_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 5           rg_m1_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 6           rg_m1_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 7           rg_m1_wf2g_tx_ib_en            U     RW        default = 'h0
// Bit 8           rg_m2_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 9           rg_m2_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 10          rg_m2_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 11          rg_m2_wf2g_tx_ib_en            U     RW        default = 'h0
// Bit 12          rg_m3_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 13          rg_m3_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m3_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 15          rg_m3_wf2g_tx_ib_en            U     RW        default = 'h0
// Bit 31  :16     rg_wf2g_tx_rsv                 U     RW        default = 'h0
typedef union RG_TX_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M0_WF2G_TX_PA_EN : 1;
    unsigned int RG_M0_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M0_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M0_WF2G_TX_IB_EN : 1;
    unsigned int RG_M1_WF2G_TX_PA_EN : 1;
    unsigned int RG_M1_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M1_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M1_WF2G_TX_IB_EN : 1;
    unsigned int RG_M2_WF2G_TX_PA_EN : 1;
    unsigned int RG_M2_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M2_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M2_WF2G_TX_IB_EN : 1;
    unsigned int RG_M3_WF2G_TX_PA_EN : 1;
    unsigned int RG_M3_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M3_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M3_WF2G_TX_IB_EN : 1;
    unsigned int RG_WF2G_TX_RSV : 16;
  } b;
} RG_TX_A7_FIELD_T;

#define RG_TX_A8                                  (RF_D_TX_REG_BASE + 0x20)
// Bit 2   :0      rg_wf5g_tx_pa_gain_g1          U     RW        default = 'h7
// Bit 6   :4      rg_wf5g_tx_pa_gain_g2          U     RW        default = 'h7
// Bit 10  :8      rg_wf5g_tx_pa_gain_g3          U     RW        default = 'h3
// Bit 14  :12     rg_wf5g_tx_pa_gain_g4          U     RW        default = 'h1
// Bit 18  :16     rg_wf5g_tx_mxr_gain_g1         U     RW        default = 'h7
// Bit 22  :20     rg_wf5g_tx_mxr_gain_g2         U     RW        default = 'h7
// Bit 26  :24     rg_wf5g_tx_mxr_gain_g3         U     RW        default = 'h3
// Bit 30  :28     rg_wf5g_tx_mxr_gain_g4         U     RW        default = 'h1
typedef union RG_TX_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_TX_PA_GAIN_G1 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF5G_TX_PA_GAIN_G2 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF5G_TX_PA_GAIN_G3 : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF5G_TX_PA_GAIN_G4 : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF5G_TX_MXR_GAIN_G1 : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF5G_TX_MXR_GAIN_G2 : 3;
    unsigned int rsvd_5 : 1;
    unsigned int RG_WF5G_TX_MXR_GAIN_G3 : 3;
    unsigned int rsvd_6 : 1;
    unsigned int RG_WF5G_TX_MXR_GAIN_G4 : 3;
    unsigned int rsvd_7 : 1;
  } b;
} RG_TX_A8_FIELD_T;

#define RG_TX_A9                                  (RF_D_TX_REG_BASE + 0x24)
// Bit 2   :0      rg_wf2g_tx_pa_gain_g1          U     RW        default = 'h7
// Bit 6   :4      rg_wf2g_tx_pa_gain_g2          U     RW        default = 'h7
// Bit 10  :8      rg_wf2g_tx_pa_gain_g3          U     RW        default = 'h3
// Bit 14  :12     rg_wf2g_tx_pa_gain_g4          U     RW        default = 'h1
// Bit 18  :16     rg_wf2g_tx_mxr_gain_g1         U     RW        default = 'h7
// Bit 22  :20     rg_wf2g_tx_mxr_gain_g2         U     RW        default = 'h7
// Bit 26  :24     rg_wf2g_tx_mxr_gain_g3         U     RW        default = 'h3
// Bit 30  :28     rg_wf2g_tx_mxr_gain_g4         U     RW        default = 'h1
typedef union RG_TX_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF2G_TX_PA_GAIN_G1 : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF2G_TX_PA_GAIN_G2 : 3;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF2G_TX_PA_GAIN_G3 : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF2G_TX_PA_GAIN_G4 : 3;
    unsigned int rsvd_3 : 1;
    unsigned int RG_WF2G_TX_MXR_GAIN_G1 : 3;
    unsigned int rsvd_4 : 1;
    unsigned int RG_WF2G_TX_MXR_GAIN_G2 : 3;
    unsigned int rsvd_5 : 1;
    unsigned int RG_WF2G_TX_MXR_GAIN_G3 : 3;
    unsigned int rsvd_6 : 1;
    unsigned int RG_WF2G_TX_MXR_GAIN_G4 : 3;
    unsigned int rsvd_7 : 1;
  } b;
} RG_TX_A9_FIELD_T;

#define RG_TX_A10                                 (RF_D_TX_REG_BASE + 0x28)
// Bit 0           rg_m4_wf5g_tx_pa_en            U     RW        default = 'h1
// Bit 1           rg_m4_wf5g_tx_mxr_en           U     RW        default = 'h1
// Bit 2           rg_m4_wf5g_tx_ib_en            U     RW        default = 'h1
// Bit 3           rg_m5_wf5g_tx_pa_en            U     RW        default = 'h0
// Bit 4           rg_m5_wf5g_tx_mxr_en           U     RW        default = 'h0
// Bit 5           rg_m5_wf5g_tx_ib_en            U     RW        default = 'h0
// Bit 6           rg_m6_wf5g_tx_pa_en            U     RW        default = 'h0
// Bit 7           rg_m6_wf5g_tx_mxr_en           U     RW        default = 'h0
// Bit 8           rg_m6_wf5g_tx_ib_en            U     RW        default = 'h0
// Bit 9           rg_m7_wf5g_tx_pa_en            U     RW        default = 'h0
// Bit 10          rg_m7_wf5g_tx_mxr_en           U     RW        default = 'h0
// Bit 11          rg_m7_wf5g_tx_ib_en            U     RW        default = 'h0
typedef union RG_TX_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M4_WF5G_TX_PA_EN : 1;
    unsigned int RG_M4_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M4_WF5G_TX_IB_EN : 1;
    unsigned int RG_M5_WF5G_TX_PA_EN : 1;
    unsigned int RG_M5_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M5_WF5G_TX_IB_EN : 1;
    unsigned int RG_M6_WF5G_TX_PA_EN : 1;
    unsigned int RG_M6_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M6_WF5G_TX_IB_EN : 1;
    unsigned int RG_M7_WF5G_TX_PA_EN : 1;
    unsigned int RG_M7_WF5G_TX_MXR_EN : 1;
    unsigned int RG_M7_WF5G_TX_IB_EN : 1;
    unsigned int rsvd_0 : 20;
  } b;
} RG_TX_A10_FIELD_T;

#define RG_TX_A11                                 (RF_D_TX_REG_BASE + 0x2c)
// Bit 0           rg_m4_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 1           rg_m4_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 2           rg_m4_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 3           rg_m4_wf2g_tx_ib_en            U     RW        default = 'h0
// Bit 4           rg_m5_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 5           rg_m5_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 6           rg_m5_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 7           rg_m5_wf2g_tx_ib_en            U     RW        default = 'h0
// Bit 8           rg_m6_wf2g_tx_pa_en            U     RW        default = 'h1
// Bit 9           rg_m6_wf2g_tx_mxr_en           U     RW        default = 'h1
// Bit 10          rg_m6_wf2g_tx_var_en           U     RW        default = 'h1
// Bit 11          rg_m6_wf2g_tx_ib_en            U     RW        default = 'h1
// Bit 12          rg_m7_wf2g_tx_pa_en            U     RW        default = 'h0
// Bit 13          rg_m7_wf2g_tx_mxr_en           U     RW        default = 'h0
// Bit 14          rg_m7_wf2g_tx_var_en           U     RW        default = 'h0
// Bit 15          rg_m7_wf2g_tx_ib_en            U     RW        default = 'h0
typedef union RG_TX_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M4_WF2G_TX_PA_EN : 1;
    unsigned int RG_M4_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M4_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M4_WF2G_TX_IB_EN : 1;
    unsigned int RG_M5_WF2G_TX_PA_EN : 1;
    unsigned int RG_M5_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M5_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M5_WF2G_TX_IB_EN : 1;
    unsigned int RG_M6_WF2G_TX_PA_EN : 1;
    unsigned int RG_M6_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M6_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M6_WF2G_TX_IB_EN : 1;
    unsigned int RG_M7_WF2G_TX_PA_EN : 1;
    unsigned int RG_M7_WF2G_TX_MXR_EN : 1;
    unsigned int RG_M7_WF2G_TX_VAR_EN : 1;
    unsigned int RG_M7_WF2G_TX_IB_EN : 1;
    unsigned int rsvd_0 : 16;
  } b;
} RG_TX_A11_FIELD_T;

#define RG_TX_A12                                 (RF_D_TX_REG_BASE + 0x30)
// Bit 7   :0      rg_wf_tx_gain_man              U     RW        default = 'h0
// Bit 8           rg_wf_tx_gain_man_mode         U     RW        default = 'h0
// Bit 23  :16     rg_wf_tx_pa_en_delay_bnd       U     RW        default = 'h14
// Bit 24          rg_wf_tx_pa_en_delay_bypass     U     RW        default = 'h0
typedef union RG_TX_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_TX_GAIN_MAN : 8;
    unsigned int RG_WF_TX_GAIN_MAN_MODE : 1;
    unsigned int rsvd_0 : 7;
    unsigned int RG_WF_TX_PA_EN_DELAY_BND : 8;
    unsigned int RG_WF_TX_PA_EN_DELAY_BYPASS : 1;
    unsigned int rsvd_1 : 7;
  } b;
} RG_TX_A12_FIELD_T;

#define RG_TX_A13                                 (RF_D_TX_REG_BASE + 0x34)
// Bit 4   :0      rg_wf5g_tx_mxr_cbank_g1        U     RW        default = 'h7
// Bit 9   :5      rg_wf5g_tx_mxr_cbank_g2        U     RW        default = 'h7
// Bit 14  :10     rg_wf5g_tx_mxr_cbank_g3        U     RW        default = 'h7
// Bit 19  :15     rg_wf5g_tx_mxr_cbank_g4        U     RW        default = 'h3
// Bit 24  :20     rg_wf5g_tx_mxr_cbank_man       U     RW        default = 'h4
typedef union RG_TX_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF5G_TX_MXR_CBANK_G1 : 5;
    unsigned int RG_WF5G_TX_MXR_CBANK_G2 : 5;
    unsigned int RG_WF5G_TX_MXR_CBANK_G3 : 5;
    unsigned int RG_WF5G_TX_MXR_CBANK_G4 : 5;
    unsigned int RG_WF5G_TX_MXR_CBANK_MAN : 5;
    unsigned int rsvd_0 : 7;
  } b;
} RG_TX_A13_FIELD_T;

#endif

