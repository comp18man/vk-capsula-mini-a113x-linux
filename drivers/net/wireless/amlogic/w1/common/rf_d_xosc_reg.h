#ifdef RF_D_XOSC_REG
#else
#define RF_D_XOSC_REG

#define CHIP_ANA_REG_BASE   (0xf01000)

#define RG_XOSC_A7                                (CHIP_ANA_REG_BASE + 0x20)
// Bit 0           rg_xo_bg_en_man                U     RW        default = 'h1
// Bit 2           rg_xo_bg_en_man_sel            U     RW        default = 'h0
// Bit 3           rg_xo_ldo_bypass_en            U     RW        default = 'h0
// Bit 4           rg_xo_ldo_en_man               U     RW        default = 'h1
// Bit 5           rg_xo_ldo_en_man_sel           U     RW        default = 'h0
// Bit 7   :6      rg_xo_ldo_fbr_ctrl             U     RW        default = 'h2
// Bit 13  :8      rg_xo_ldo_vref_cal             U     RW        default = 'h10
// Bit 14          rg_xo_en_man                   U     RW        default = 'h1
// Bit 15          rg_xo_en_man_sel               U     RW        default = 'h0
// Bit 16          rg_xo_sel                      U     RW        default = 'h0
// Bit 21  :17     rg_xo_core_bias_rcal_ctrl      U     RW        default = 'h10
typedef union RG_XOSC_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_XO_BG_EN_MAN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_XO_BG_EN_MAN_SEL : 1;
    unsigned int RG_XO_LDO_BYPASS_EN : 1;
    unsigned int RG_XO_LDO_EN_MAN : 1;
    unsigned int RG_XO_LDO_EN_MAN_SEL : 1;
    unsigned int RG_XO_LDO_FBR_CTRL : 2;
    unsigned int RG_XO_LDO_VREF_CAL : 6;
    unsigned int RG_XO_EN_MAN : 1;
    unsigned int RG_XO_EN_MAN_SEL : 1;
    unsigned int RG_XO_SEL : 1;
    unsigned int RG_XO_CORE_BIAS_RCAL_CTRL : 5;
    unsigned int rsvd_1 : 10;
  } b;
} RG_XOSC_A7_FIELD_T;

#define RG_XOSC_A8                                (CHIP_ANA_REG_BASE + 0x24)
// Bit 3   :0      rg_xo_core_cfixed_ctrl         U     RW        default = 'h8
// Bit 11  :4      rg_xo_core_ctune_ctrl          U     RW        default = 'h32
// Bit 15  :12     rg_xo_core_gmos_vb_sel         U     RW        default = 'h8
// Bit 20  :17     rg_xo_pd_vref_sel              U     RW        default = 'h8
typedef union RG_XOSC_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_XO_CORE_CFIXED_CTRL : 4;
    unsigned int RG_XO_CORE_CTUNE_CTRL : 8;
    unsigned int RG_XO_CORE_GMOS_VB_SEL : 4;
    unsigned int rsvd_0 : 1;
    unsigned int RG_XO_PD_VREF_SEL : 4;
    unsigned int rsvd_1 : 11;
  } b;
} RG_XOSC_A8_FIELD_T;

#define RG_XOSC_A9                                (CHIP_ANA_REG_BASE + 0x28)
// Bit 3   :0      rg_xo_buf1_bias_ctrl           U     RW        default = 'h8
// Bit 7   :4      rg_xo_buf2_bias_ctrl           U     RW        default = 'h8
// Bit 11  :8      rg_xo_buf3_bias_ctrl           U     RW        default = 'h8
// Bit 12          rg_xo_aml_cal_error_ctrl       U     RW        default = 'h0
typedef union RG_XOSC_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_XO_BUF1_BIAS_CTRL : 4;
    unsigned int RG_XO_BUF2_BIAS_CTRL : 4;
    unsigned int RG_XO_BUF3_BIAS_CTRL : 4;
    unsigned int RG_XO_AML_CAL_ERROR_CTRL : 1;
    unsigned int rsvd_0 : 19;
  } b;
} RG_XOSC_A9_FIELD_T;

#define RG_XOSC_A11                               (CHIP_ANA_REG_BASE + 0x30)
// Bit 0           rg_xo_ldo_fc_en_man_sel        U     RW        default = 'h0
// Bit 1           rg_xo_ldo_fc_en_man            U     RW        default = 'h0
// Bit 2           rg_xo_pd_en_man_sel            U     RW        default = 'h0
// Bit 3           rg_xo_pd_en_man                U     RW        default = 'h1
// Bit 4           rg_xo_buf_core_en_man_sel      U     RW        default = 'h0
// Bit 5           rg_xo_buf_core_en_man          U     RW        default = 'h1
// Bit 6           rg_xo_buf_wf_en_man_sel        U     RW        default = 'h0
// Bit 7           rg_xo_buf_wf_en_man            U     RW        default = 'h1
// Bit 8           rg_xo_buf_dig_en_man_sel       U     RW        default = 'h0
// Bit 9           rg_xo_buf_dig_en_man           U     RW        default = 'h1
// Bit 10          rg_xo_buf_bb_en_man_sel        U     RW        default = 'h0
// Bit 11          rg_xo_buf_bb_en_man            U     RW        default = 'h1
// Bit 12          rg_xo_buf_bt_en_man_sel        U     RW        default = 'h0
// Bit 13          rg_xo_buf_bt_en_man            U     RW        default = 'h1
// Bit 15          rg_xo_core_bias_ctrl_man_sel     U     RW        default = 'h0
// Bit 23  :16     rg_xo_core_bias_ctrl_man       U     RW        default = 'hff
// Bit 31  :24     rg_xosc_wait_bnd               U     RW        default = 'h3f
typedef union RG_XOSC_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_XO_LDO_FC_EN_MAN_SEL : 1;
    unsigned int RG_XO_LDO_FC_EN_MAN : 1;
    unsigned int RG_XO_PD_EN_MAN_SEL : 1;
    unsigned int RG_XO_PD_EN_MAN : 1;
    unsigned int RG_XO_BUF_CORE_EN_MAN_SEL : 1;
    unsigned int RG_XO_BUF_CORE_EN_MAN : 1;
    unsigned int RG_XO_BUF_WF_EN_MAN_SEL : 1;
    unsigned int RG_XO_BUF_WF_EN_MAN : 1;
    unsigned int RG_XO_BUF_DIG_EN_MAN_SEL : 1;
    unsigned int RG_XO_BUF_DIG_EN_MAN : 1;
    unsigned int RG_XO_BUF_BB_EN_MAN_SEL : 1;
    unsigned int RG_XO_BUF_BB_EN_MAN : 1;
    unsigned int RG_XO_BUF_BT_EN_MAN_SEL : 1;
    unsigned int RG_XO_BUF_BT_EN_MAN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_XO_CORE_BIAS_CTRL_MAN_SEL : 1;
    unsigned int RG_XO_CORE_BIAS_CTRL_MAN : 8;
    unsigned int RG_XOSC_WAIT_BND : 8;
  } b;
} RG_XOSC_A11_FIELD_T;

#endif