#ifdef CHIP_ANA_REG
#else
#define CHIP_ANA_REG


#define CHIP_ANA_REG_BASE                         (0xf01000)

#define RG_DPLL_A0                                (CHIP_ANA_REG_BASE + 0x0)
// Bit 4   :0      rg_wifi_bb_pll_n               U     RW        default = 'h1
// Bit 12  :5      rg_wifi_bb_pll_m               U     RW        default = 'h30
typedef union RG_DPLL_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_pll_n : 5;
    unsigned int rg_wifi_bb_pll_m : 8;
    unsigned int rsvd_0 : 19;
  } b;
} RG_DPLL_A0_FIELD_T;

#define RG_DPLL_A1                                (CHIP_ANA_REG_BASE + 0x4)
// Bit 3   :0      rg_wifi_bb_pll_cal_num         U     RW        default = 'h0
// Bit 4           rg_wifi_bb_pll_cal_bypass      U     RW        default = 'h0
// Bit 5           rg_wifi_bb_pll_gen_sel         U     RW        default = 'h1
// Bit 8   :7      rg_wifi_bb_pll_afc_hold        U     RW        default = 'h1
// Bit 9           rg_wifi_bb_pll_afc_clk_sel     U     RW        default = 'h0
// Bit 10          rg_wifi_bb_pll_ssc_en          U     RW        default = 'h0
// Bit 12  :11     rg_wifi_bb_pll_ssc_os          U     RW        default = 'h0
// Bit 16  :13     rg_wifi_bb_pll_ssc_str_m       U     RW        default = 'h0
// Bit 19  :17     rg_wifi_bb_pll_ssc_fref        U     RW        default = 'h0
// Bit 21  :20     rg_wifi_bb_pll_ssc_mode        U     RW        default = 'h0
// Bit 25  :22     rg_wifi_bb_pll_ssc_dep         U     RW        default = 'h0
typedef union RG_DPLL_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_pll_cal_num : 4;
    unsigned int rg_wifi_bb_pll_cal_bypass : 1;
    unsigned int rg_wifi_bb_pll_gen_sel : 1;
    unsigned int rsvd_0 : 1;
    unsigned int rg_wifi_bb_pll_afc_hold : 2;
    unsigned int rg_wifi_bb_pll_afc_clk_sel : 1;
    unsigned int rg_wifi_bb_pll_ssc_en : 1;
    unsigned int rg_wifi_bb_pll_ssc_os : 2;
    unsigned int rg_wifi_bb_pll_ssc_str_m : 4;
    unsigned int rg_wifi_bb_pll_ssc_fref : 3;
    unsigned int rg_wifi_bb_pll_ssc_mode : 2;
    unsigned int rg_wifi_bb_pll_ssc_dep : 4;
    unsigned int rsvd_1 : 6;
  } b;
} RG_DPLL_A1_FIELD_T;

#define RG_DPLL_A2                                (CHIP_ANA_REG_BASE + 0x8)
// Bit 0           rg_wifi_bb_pll_afc_bypass      U     RW        default = 'h0
// Bit 8   :1      rg_wifi_bb_pll_afc_in          U     RW        default = 'h0
// Bit 9           rg_wifi_bb_pll_tdc_mode        U     RW        default = 'h0
// Bit 10          rg_wifi_bb_pll_dead_time       U     RW        default = 'h0
// Bit 11          rg_wifi_bb_pll_dco_m_ctrl      U     RW        default = 'h0
// Bit 12          rg_wifi_bb_pll_div_pw_select     U     RW        default = 'h1
// Bit 13          rg_wifi_bb_pll_div_pwen        U     RW        default = 'h1
// Bit 19  :14     rg_wifi_bb_pll_reve            U     RW        default = 'h0
// Bit 21  :20     rg_wifi_bb_pll_data_sel        U     RW        default = 'h0
// Bit 29  :22     ro_wifi_bb_pll_data_out        U     RO        default = 'h0
// Bit 30          rg_wifi_bb_pll_ldo_stb         U     RW        default = 'h1
typedef union RG_DPLL_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_pll_afc_bypass : 1;
    unsigned int rg_wifi_bb_pll_afc_in : 8;
    unsigned int rg_wifi_bb_pll_tdc_mode : 1;
    unsigned int rg_wifi_bb_pll_dead_time : 1;
    unsigned int rg_wifi_bb_pll_dco_m_ctrl : 1;
    unsigned int rg_wifi_bb_pll_div_pw_select : 1;
    unsigned int rg_wifi_bb_pll_div_pwen : 1;
    unsigned int rg_wifi_bb_pll_reve : 6;
    unsigned int rg_wifi_bb_pll_data_sel : 2;
    unsigned int ro_wifi_bb_pll_data_out : 8;
    unsigned int rg_wifi_bb_pll_ldo_stb : 1;
    unsigned int rsvd_0 : 1;
  } b;
} RG_DPLL_A2_FIELD_T;

#define RG_DPLL_A3                                (CHIP_ANA_REG_BASE + 0xc)
// Bit 0           rg_wifi_bb_pll_rst             U     RW        default = 'h1
// Bit 1           rg_wifi_bb_pll_bias_en         U     RW        default = 'h1
// Bit 5   :2      rg_wifi_bb_pll_lpf_res         U     RW        default = 'he
// Bit 10  :6      rg_wifi_bb_pll_lpf_cap         U     RW        default = 'ha
// Bit 14  :11     rg_wifi_bb_pll_ibcntlc         U     RW        default = 'h2
// Bit 18  :15     rg_wifi_bb_pll_lpf_neg_cap     U     RW        default = 'h3
// Bit 22  :19     rg_wifi_bb_pll_ibcntlr         U     RW        default = 'h3
// Bit 23          rg_wifi_bb_pll_dco_gain        U     RW        default = 'h1
// Bit 26  :24     rg_wifi_bb_pll_ldo_ref_adj     U     RW        default = 'h2
typedef union RG_DPLL_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_pll_rst : 1;
    unsigned int rg_wifi_bb_pll_bias_en : 1;
    unsigned int rg_wifi_bb_pll_lpf_res : 4;
    unsigned int rg_wifi_bb_pll_lpf_cap : 5;
    unsigned int rg_wifi_bb_pll_ibcntlc : 4;
    unsigned int rg_wifi_bb_pll_lpf_neg_cap : 4;
    unsigned int rg_wifi_bb_pll_ibcntlr : 4;
    unsigned int rg_wifi_bb_pll_dco_gain : 1;
    unsigned int rg_wifi_bb_pll_ldo_ref_adj : 3;
    unsigned int rsvd_0 : 5;
  } b;
} RG_DPLL_A3_FIELD_T;

#define RG_DPLL_A4                                (CHIP_ANA_REG_BASE + 0x10)
// Bit 0           rg_wifi_bb_bt_adc_div4         U     RW        default = 'h0
// Bit 7   :4      rg_wifi_bb_wifi_clk_div        U     RW        default = 'h3
// Bit 11  :8      rg_wifi_bb_digi_clk1_div       U     RW        default = 'h4
// Bit 12          rg_wifi_bb_wifi_adc_div        U     RW        default = 'h0
// Bit 19  :16     rg_wifi_bb_bt_adc_div          U     RW        default = 'h5
// Bit 21  :20     rg_wifi_bb_bt_dac_clk_div      U     RW        default = 'h3
typedef union RG_DPLL_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_bt_adc_div4 : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_wifi_bb_wifi_clk_div : 4;
    unsigned int rg_wifi_bb_digi_clk1_div : 4;
    unsigned int rg_wifi_bb_wifi_adc_div : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_wifi_bb_bt_adc_div : 4;
    unsigned int rg_wifi_bb_bt_dac_clk_div : 2;
    unsigned int rsvd_2 : 10;
  } b;
} RG_DPLL_A4_FIELD_T;

#define RG_DPLL_A5                                (CHIP_ANA_REG_BASE + 0x14)
// Bit 0           rg_wifi_bb_pll_shift_en        U     RW        default = 'h0
// Bit 2   :1      rg_wifi_bb_pll_shift_v         U     RW        default = 'h0
// Bit 3           rg_wifi_bb_pll_load_en         U     RW        default = 'h0
// Bit 4           rg_wifi_bb_pll_load            U     RW        default = 'h0
// Bit 6   :5      rg_wifi_bb_pll_lock_long       U     RW        default = 'h1
// Bit 7           rg_wifi_bb_pll_lock_f          U     RW        default = 'h0
// Bit 9   :8      rg_wifi_bb_pll_lkw_sel         U     RW        default = 'h1
// Bit 10          rg_wifi_bb_pll_temp2x          U     RW        default = 'h0
// Bit 11          rg_wifi_bb_pll_temp4x          U     RW        default = 'h0
// Bit 12          rg_wifi_bb_pll_clk_test_en     U     RW        default = 'h0
// Bit 13          rg_wifi_bb_pll_doubler_en      U     RW        default = 'h0
// Bit 14          rg_wifi_bb_pll_sdm_en          U     RW        default = 'h0
// Bit 28          ro_wifi_bb_pll_vco_clk_done     U     RO        default = 'h0
// Bit 29          ro_wifi_bb_pll_fb_clk_done     U     RO        default = 'h0
// Bit 30          ro_wifi_bb_pll_ref_done        U     RO        default = 'h0
// Bit 31          ro_wifi_bb_pll_done            U     RO        default = 'h0
typedef union RG_DPLL_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_pll_shift_en : 1;
    unsigned int rg_wifi_bb_pll_shift_v : 2;
    unsigned int rg_wifi_bb_pll_load_en : 1;
    unsigned int rg_wifi_bb_pll_load : 1;
    unsigned int rg_wifi_bb_pll_lock_long : 2;
    unsigned int rg_wifi_bb_pll_lock_f : 1;
    unsigned int rg_wifi_bb_pll_lkw_sel : 2;
    unsigned int rg_wifi_bb_pll_temp2x : 1;
    unsigned int rg_wifi_bb_pll_temp4x : 1;
    unsigned int rg_wifi_bb_pll_clk_test_en : 1;
    unsigned int rg_wifi_bb_pll_doubler_en : 1;
    unsigned int rg_wifi_bb_pll_sdm_en : 1;
    unsigned int rsvd_0 : 13;
    unsigned int ro_wifi_bb_pll_vco_clk_done : 1;
    unsigned int ro_wifi_bb_pll_fb_clk_done : 1;
    unsigned int ro_wifi_bb_pll_ref_done : 1;
    unsigned int ro_wifi_bb_pll_done : 1;
  } b;
} RG_DPLL_A5_FIELD_T;

#define RG_DPLL_A6                                (CHIP_ANA_REG_BASE + 0x1c)
// Bit 18  :0      rg_wifi_bb_pll_fra             U     RW        default = 'h1
typedef union RG_DPLL_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wifi_bb_pll_fra : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_DPLL_A6_FIELD_T;

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
// Bit 7   :4      rg_xo_buf2_bias_ctrl           U     RW        default = 'h1
// Bit 11  :8      rg_xo_buf3_bias_ctrl           U     RW        default = 'hf
// Bit 23  :16     rg_xo_core_ctune_offset_ctrl     U     RW        default = 'h0
typedef union RG_XOSC_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_XO_BUF1_BIAS_CTRL : 4;
    unsigned int RG_XO_BUF2_BIAS_CTRL : 4;
    unsigned int rsvd_0 : 3;
    unsigned int RG_XO_CORE_CTUNE_OFFSET_CTRL : 8;
    unsigned int rsvd_1 : 8;
  } b;
} RG_XOSC_A9_FIELD_T;

#define RG_XOSC_A10                               (CHIP_ANA_REG_BASE + 0x2c)
// Bit 15  :0      rg_xo_reserved                 U     RW        default = 'h0
// Bit 23  :16     ro_xo_core_bias_ctrl           U     RO        default = 'h0
// Bit 24          ro_xosc_cali_ready             U     RO        default = 'h0
// Bit 31          ro_xo_aml_cal_done             U     RO        default = 'h0
typedef union RG_XOSC_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_XO_RESERVED : 16;
    unsigned int RO_XO_CORE_BIAS_CTRL : 8;
    unsigned int RO_XOSC_CALI_READY : 1;
    unsigned int rsvd_0 : 6;
    unsigned int RO_XO_AML_CAL_DONE : 1;
  } b;
} RG_XOSC_A10_FIELD_T;

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

#define RG_BG_A12                                 (CHIP_ANA_REG_BASE + 0x34)
// Bit 0           rg_wf_bg_en                    U     RW        default = 'h1
// Bit 1           rg_wf_bg_rcal_en               U     RW        default = 'h0
// Bit 14  :8      rg_wf_bg_ptat_cal_i_man        U     RW        default = 'h40
// Bit 21  :15     rg_wf_bg_zt_cal_i_man          U     RW        default = 'h40
// Bit 22          rg_wf_bg_iout_en               U     RW        default = 'h1
// Bit 23          rg_wf_bg_iout_sel              U     RW        default = 'h1
// Bit 24          rg_wf_bg_zt_cal_i_man_sel      U     RW        default = 'h1
// Bit 25          rg_wf_bg_ptat_cal_i_man_sel     U     RW        default = 'h1
// Bit 27  :26     rg_wf_bg_rcal_waittime         U     RW        default = 'h1
typedef union RG_BG_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_BG_EN : 1;
    unsigned int RG_WF_BG_RCAL_EN : 1;
    unsigned int rsvd_0 : 6;
    unsigned int RG_WF_BG_PTAT_CAL_I_MAN : 7;
    unsigned int RG_WF_BG_ZT_CAL_I_MAN : 7;
    unsigned int RG_WF_BG_IOUT_EN : 1;
    unsigned int RG_WF_BG_IOUT_SEL : 1;
    unsigned int RG_WF_BG_ZT_CAL_I_MAN_SEL : 1;
    unsigned int RG_WF_BG_PTAT_CAL_I_MAN_SEL : 1;
    unsigned int RG_WF_BG_RCAL_WAITTIME : 2;
    unsigned int rsvd_1 : 4;
  } b;
} RG_BG_A12_FIELD_T;

#define RG_BG_A13                                 (CHIP_ANA_REG_BASE + 0x38)
// Bit 6   :0      ro_wf_bg_zt_cal_i              U     RO        default = 'h0
// Bit 14  :8      ro_wf_bg_ptat_cal_i            U     RO        default = 'h0
// Bit 29          ro_wf_bg_rcal_finish           U     RO        default = 'h0
// Bit 30          ro_wf_bg_cmp_out               U     RO        default = 'h0
typedef union RG_BG_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_BG_ZT_CAL_I : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RO_WF_BG_PTAT_CAL_I : 7;
    unsigned int rsvd_1 : 14;
    unsigned int RO_WF_BG_RCAL_FINISH : 1;
    unsigned int RO_WF_BG_CMP_OUT : 1;
    unsigned int rsvd_2 : 1;
  } b;
} RG_BG_A13_FIELD_T;

#define RG_BG_A14                                 (CHIP_ANA_REG_BASE + 0x3c)
// Bit 0           rg_bt_bg_en                    U     RW        default = 'h1
// Bit 1           rg_bt_bg_rcal_en               U     RW        default = 'h0
// Bit 14  :8      rg_bt_bg_ptat_cal_i_man        U     RW        default = 'h40
// Bit 21  :15     rg_bt_bg_zt_cal_i_man          U     RW        default = 'h40
// Bit 22          rg_bt_bg_iout_en               U     RW        default = 'h1
// Bit 23          rg_bt_bg_iout_sel              U     RW        default = 'h1
// Bit 24          rg_bt_bg_zt_cal_i_man_sel      U     RW        default = 'h1
// Bit 25          rg_bt_bg_ptat_cal_i_man_sel     U     RW        default = 'h1
// Bit 27  :26     rg_bt_bg_rcal_waittime         U     RW        default = 'h1
typedef union RG_BG_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_BT_BG_EN : 1;
    unsigned int RG_BT_BG_RCAL_EN : 1;
    unsigned int rsvd_0 : 6;
    unsigned int RG_BT_BG_PTAT_CAL_I_MAN : 7;
    unsigned int RG_BT_BG_ZT_CAL_I_MAN : 7;
    unsigned int RG_BT_BG_IOUT_EN : 1;
    unsigned int RG_BT_BG_IOUT_SEL : 1;
    unsigned int RG_BT_BG_ZT_CAL_I_MAN_SEL : 1;
    unsigned int RG_BT_BG_PTAT_CAL_I_MAN_SEL : 1;
    unsigned int RG_BT_BG_RCAL_WAITTIME : 2;
    unsigned int rsvd_1 : 4;
  } b;
} RG_BG_A14_FIELD_T;

#define RG_BG_A15                                 (CHIP_ANA_REG_BASE + 0x40)
// Bit 6   :0      ro_bt_bg_zt_cal_i              U     RO        default = 'h0
// Bit 14  :8      ro_bt_bg_ptat_cal_i            U     RO        default = 'h0
// Bit 29          ro_bt_bg_rcal_finish           U     RO        default = 'h0
// Bit 30          ro_bt_bg_cmp_out               U     RO        default = 'h0
typedef union RG_BG_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_BT_BG_ZT_CAL_I : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RO_BT_BG_PTAT_CAL_I : 7;
    unsigned int rsvd_1 : 14;
    unsigned int RO_BT_BG_RCAL_FINISH : 1;
    unsigned int RO_BT_BG_CMP_OUT : 1;
    unsigned int rsvd_2 : 1;
  } b;
} RG_BG_A15_FIELD_T;

#define RG_BG_A16                                 (CHIP_ANA_REG_BASE + 0x44)
// Bit 0           rg_wf_sleep_enb                U     RW        default = 'h1
// Bit 1           rg_wf_dvdd_ldo_en              U     RW        default = 'h1
// Bit 8   :4      rg_wf_dvdd_vsel                U     RW        default = 'h10
// Bit 10  :9      rg_wf_top_mux_en               U     RW        default = 'h0
typedef union RG_BG_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SLEEP_ENB : 1;
    unsigned int RG_WF_DVDD_LDO_EN : 1;
    unsigned int rsvd_0 : 2;
    unsigned int RG_WF_DVDD_VSEL : 5;
    unsigned int RG_WF_TOP_MUX_EN : 2;
    unsigned int rsvd_1 : 21;
  } b;
} RG_BG_A16_FIELD_T;

#define RG_BG_A17                                 (CHIP_ANA_REG_BASE + 0x48)
// Bit 16          rg_bt_sleep_enb                U     RW        default = 'h1
// Bit 17          rg_bt_dvdd_ldo_en              U     RW        default = 'h1
// Bit 24  :20     rg_bt_dvdd_vsel                U     RW        default = 'h10
// Bit 26  :25     rg_bt_top_mux_en               U     RW        default = 'h0
typedef union RG_BG_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 16;
    unsigned int RG_BT_SLEEP_ENB : 1;
    unsigned int RG_BT_DVDD_LDO_EN : 1;
    unsigned int rsvd_1 : 2;
    unsigned int RG_BT_DVDD_VSEL : 5;
    unsigned int RG_BT_TOP_MUX_EN : 2;
    unsigned int rsvd_2 : 5;
  } b;
} RG_BG_A17_FIELD_T;

#define RG_PMIC_A18                               (CHIP_ANA_REG_BASE + 0x4c)
// Bit 0           rg_pmu_buck_bypass             U     RW        default = 'h0
// Bit 1           rg_pmu_buck_fpwm               U     RW        default = 'h0
// Bit 2           rg_pmu_buck_ndis_en            U     RW        default = 'h1
// Bit 3           rg_pmu_buck_dis_lpmode         U     RW        default = 'h0
// Bit 7   :4      rg_pmu_buck_zx_offset          U     RW        default = 'h5
// Bit 8           rg_pmu_buck_zx_pdn             U     RW        default = 'h0
// Bit 9           rg_pmu_buck_force_2m_internal     U     RW        default = 'h1
// Bit 10          rg_pmu_buck_1m_en              U     RW        default = 'h0
// Bit 11          rg_pmu_buck_intclk_dis         U     RW        default = 'h0
// Bit 15  :12     rg_pmu_buck_vsel_vh            U     RW        default = 'h7
// Bit 19  :16     rg_pmu_buck_vsel_vl            U     RW        default = 'h6
// Bit 22  :20     rg_pmu_buck_ndri_on_sr         U     RW        default = 'h4
// Bit 25  :23     rg_pmu_buck_ndri_off_sr        U     RW        default = 'h4
// Bit 28  :26     rg_pmu_buck_pdri_on_sr         U     RW        default = 'h4
// Bit 31  :29     rg_pmu_buck_pdri_off_sr        U     RW        default = 'h4
typedef union RG_PMIC_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_PMU_BUCK_BYPASS : 1;
    unsigned int RG_PMU_BUCK_FPWM : 1;
    unsigned int RG_PMU_BUCK_NDIS_EN : 1;
    unsigned int RG_PMU_BUCK_DIS_LPMODE : 1;
    unsigned int RG_PMU_BUCK_ZX_OFFSET : 4;
    unsigned int RG_PMU_BUCK_ZX_PDN : 1;
    unsigned int RG_PMU_BUCK_FORCE_2M_INTERNAL : 1;
    unsigned int RG_PMU_BUCK_1M_EN : 1;
    unsigned int RG_PMU_BUCK_INTCLK_DIS : 1;
    unsigned int RG_PMU_BUCK_VSEL_VH : 4;
    unsigned int RG_PMU_BUCK_VSEL_VL : 4;
    unsigned int RG_PMU_BUCK_NDRI_ON_SR : 3;
    unsigned int RG_PMU_BUCK_NDRI_OFF_SR : 3;
    unsigned int RG_PMU_BUCK_PDRI_ON_SR : 3;
    unsigned int RG_PMU_BUCK_PDRI_OFF_SR : 3;
  } b;
} RG_PMIC_A18_FIELD_T;

#define RG_PMIC_A19                               (CHIP_ANA_REG_BASE + 0x50)
// Bit 0           rg_pmu_buck_trimtest_ea        U     RW        default = 'h0
// Bit 1           rg_pmu_buck_trimtest_vref      U     RW        default = 'h0
// Bit 2           rg_pmu_buck_trimtest_csp       U     RW        default = 'h0
// Bit 4   :3      rg_pmu_buck_voutfb_cap         U     RW        default = 'h1
// Bit 7   :5      rg_pmu_buck_ea_ccomp           U     RW        default = 'h2
// Bit 11  :8      rg_pmu_buck_ea_rcomp           U     RW        default = 'h4
// Bit 17  :12     rg_pmu_buck_vref_sel           U     RW        default = 'h8
// Bit 19  :18     rg_pmu_buck_capslp_ramp        U     RW        default = 'h2
// Bit 23  :20     rg_pmu_buck_cs_gain_ramp       U     RW        default = 'h8
// Bit 27  :24     rg_pmu_buck_offset_ramp        U     RW        default = 'h8
// Bit 31  :28     rg_pmu_buck_slp_ramp           U     RW        default = 'h8
typedef union RG_PMIC_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_PMU_BUCK_TRIMTEST_EA : 1;
    unsigned int RG_PMU_BUCK_TRIMTEST_VREF : 1;
    unsigned int RG_PMU_BUCK_TRIMTEST_CSP : 1;
    unsigned int RG_PMU_BUCK_VOUTFB_CAP : 2;
    unsigned int RG_PMU_BUCK_EA_CCOMP : 3;
    unsigned int RG_PMU_BUCK_EA_RCOMP : 4;
    unsigned int RG_PMU_BUCK_VREF_SEL : 6;
    unsigned int RG_PMU_BUCK_CAPSLP_RAMP : 2;
    unsigned int RG_PMU_BUCK_CS_GAIN_RAMP : 4;
    unsigned int RG_PMU_BUCK_OFFSET_RAMP : 4;
    unsigned int RG_PMU_BUCK_SLP_RAMP : 4;
  } b;
} RG_PMIC_A19_FIELD_T;

#define RG_PMIC_A20                               (CHIP_ANA_REG_BASE + 0x54)
// Bit 3   :0      rg_pmu_buck_oc_level_csp       U     RW        default = 'h8
// Bit 4           rg_pmu_buck_csoffset_ramp      U     RW        default = 'h0
// Bit 7   :5      rg_pmu_buck_rsv1               U     RW        default = 'h7
// Bit 15  :8      rg_pmu_buck_rsv2               U     RW        default = 'h0
// Bit 23  :16     rg_pmu_buck_rsv3               U     RW        default = 'h88
// Bit 31  :24     rg_pmu_buck_rsv4               U     RW        default = 'hff
typedef union RG_PMIC_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_PMU_BUCK_OC_LEVEL_CSP : 4;
    unsigned int RG_PMU_BUCK_CSOFFSET_RAMP : 1;
    unsigned int RG_PMU_BUCK_RSV1 : 3;
    unsigned int RG_PMU_BUCK_RSV2 : 8;
    unsigned int RG_PMU_BUCK_RSV3 : 8;
    unsigned int RG_PMU_BUCK_RSV4 : 8;
  } b;
} RG_PMIC_A20_FIELD_T;

#define RG_PMIC_A21                               (CHIP_ANA_REG_BASE + 0x58)
// Bit 0           rg_pmu_buck_lg_blank_dis       U     RW        default = 'h0
// Bit 2   :1      rg_pmu_buck_mode_testmux       U     RW        default = 'h0
// Bit 4   :3      rg_pmu_buck_testmux_ib         U     RW        default = 'h0
// Bit 7   :5      rg_pmu_buck_testmux_ana        U     RW        default = 'h0
// Bit 11  :8      rg_pmu_buck_testmux_dig        U     RW        default = 'h0
// Bit 12          rg_pmu_testmux_ana_bypass      U     RW        default = 'h0
// Bit 14  :13     rg_pmu_testmux_mode_sel        U     RW        default = 'h0
// Bit 17  :15     rg_pmu_testmux_ib_sel          U     RW        default = 'h0
// Bit 20  :18     rg_pmu_testmux_ana_sel         U     RW        default = 'h0
// Bit 23  :21     rg_pmu_testmux_dig_sel         U     RW        default = 'h0
// Bit 25  :24     rg_pmu_buck_con_trim           U     RW        default = 'h0
// Bit 29  :26     rg_pmu_buck_toff_min_trim      U     RW        default = 'h8
typedef union RG_PMIC_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_PMU_BUCK_LG_BLANK_DIS : 1;
    unsigned int RG_PMU_BUCK_MODE_TESTMUX : 2;
    unsigned int RG_PMU_BUCK_TESTMUX_IB : 2;
    unsigned int RG_PMU_BUCK_TESTMUX_ANA : 3;
    unsigned int RG_PMU_BUCK_TESTMUX_DIG : 4;
    unsigned int RG_PMU_TESTMUX_ANA_BYPASS : 1;
    unsigned int RG_PMU_TESTMUX_MODE_SEL : 2;
    unsigned int RG_PMU_TESTMUX_IB_SEL : 3;
    unsigned int RG_PMU_TESTMUX_ANA_SEL : 3;
    unsigned int RG_PMU_TESTMUX_DIG_SEL : 3;
    unsigned int RG_PMU_BUCK_CON_TRIM : 2;
    unsigned int RG_PMU_BUCK_TOFF_MIN_TRIM : 4;
    unsigned int rsvd_0 : 2;
  } b;
} RG_PMIC_A21_FIELD_T;

#define RG_PMIC_A22                               (CHIP_ANA_REG_BASE + 0x5c)
// Bit 3   :0      rg_pmu_buck_ton_min_trim       U     RW        default = 'h8
// Bit 8   :4      rg_pmu_buck_efuse_trim_offset_vref     U     RW        default = 'h10
// Bit 12  :9      rg_pmu_buck_efuse_trim_iref_vhreg     U     RW        default = 'h8
// Bit 16  :13     rg_pmu_buck_efuse_trim_iref_vlreg     U     RW        default = 'h8
// Bit 19  :17     rg_pmu_buck_efuse_trim_vref_burst     U     RW        default = 'h4
// Bit 24  :20     rg_pmu_buck_efuse_trim_ea      U     RW        default = 'h10
// Bit 29  :25     rg_pmu_buck_efuse_trim_offset_csp     U     RW        default = 'h10
typedef union RG_PMIC_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_PMU_BUCK_TON_MIN_TRIM : 4;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_OFFSET_VREF : 5;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_IREF_VHREG : 4;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_IREF_VLREG : 4;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_VREF_BURST : 3;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_EA : 5;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_OFFSET_CSP : 5;
    unsigned int rsvd_0 : 2;
  } b;
} RG_PMIC_A22_FIELD_T;

#define RG_PMIC_A23                               (CHIP_ANA_REG_BASE + 0x60)
// Bit 3   :0      rg_pmu_buck_efuse_trim_slp_csp     U     RW        default = 'h8
// Bit 6   :4      rg_pmu_buck_efuse_trim_cap_osc     U     RW        default = 'h4
// Bit 10  :7      rg_pmu_buck_efuse_trim_i_osc     U     RW        default = 'h8
// Bit 11          ro_oc_status                   U     RO        default = 'h0
// Bit 12          ro_status_dac                  U     RO        default = 'h0
// Bit 13          ro_status_offset_csp           U     RO        default = 'h0
// Bit 14          ro_status_trim_ea              U     RO        default = 'h0
typedef union RG_PMIC_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_SLP_CSP : 4;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_CAP_OSC : 3;
    unsigned int RG_PMU_BUCK_EFUSE_TRIM_I_OSC : 4;
    unsigned int RO_OC_STATUS : 1;
    unsigned int RO_STATUS_DAC : 1;
    unsigned int RO_STATUS_OFFSET_CSP : 1;
    unsigned int RO_STATUS_TRIM_EA : 1;
    unsigned int rsvd_0 : 17;
  } b;
} RG_PMIC_A23_FIELD_T;

#define RG_PMIC_A24                               (CHIP_ANA_REG_BASE + 0x64)
// Bit 3   :0      rg_rough_bg_adj                U     RW        default = 'h8
// Bit 7   :4      rg_bg650_adj                   U     RW        default = 'h8
// Bit 11  :8      rg_bg_tc_adj                   U     RW        default = 'h8
// Bit 12          rg_force_pmu_32k_off           U     RW        default = 'h0
// Bit 16  :13     rg_pmu_32k_adj                 U     RW        default = 'h8
// Bit 17          rg_force_aldo_off              U     RW        default = 'h0
// Bit 18          rg_force_buck_off              U     RW        default = 'h0
// Bit 19          rg_force_cldo_off              U     RW        default = 'h0
// Bit 22  :20     rg_prereg_dmmy_ld              U     RW        default = 'h4
// Bit 26  :23     rg_strup_rsva                  U     RW        default = 'h8
// Bit 30  :27     rg_strup_rsvb                  U     RW        default = 'h8
typedef union RG_PMIC_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_ROUGH_BG_ADJ : 4;
    unsigned int RG_BG650_ADJ : 4;
    unsigned int RG_BG_TC_ADJ : 4;
    unsigned int RG_FORCE_PMU_32K_OFF : 1;
    unsigned int RG_PMU_32K_ADJ : 4;
    unsigned int RG_FORCE_ALDO_OFF : 1;
    unsigned int RG_FORCE_BUCK_OFF : 1;
    unsigned int RG_FORCE_CLDO_OFF : 1;
    unsigned int RG_PREREG_DMMY_LD : 3;
    unsigned int RG_STRUP_RSVA : 4;
    unsigned int RG_STRUP_RSVB : 4;
    unsigned int rsvd_0 : 1;
  } b;
} RG_PMIC_A24_FIELD_T;

#define RG_PMIC_A25                               (CHIP_ANA_REG_BASE + 0x68)
// Bit 3   :0      rg_strup_rsvc                  U     RW        default = 'h8
// Bit 7   :4      rg_strup_rsvd                  U     RW        default = 'h0
// Bit 11  :8      rg_strup_rsve                  U     RW        default = 'hf
// Bit 15  :12     ro_strup_status_rsva           U     RO        default = 'h0
// Bit 19  :16     ro_strup_status_rsvb           U     RO        default = 'hf
typedef union RG_PMIC_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_STRUP_RSVC : 4;
    unsigned int RG_STRUP_RSVD : 4;
    unsigned int RG_STRUP_RSVE : 4;
    unsigned int RO_STRUP_STATUS_RSVA : 4;
    unsigned int RO_STRUP_STATUS_RSVB : 4;
    unsigned int rsvd_0 : 12;
  } b;
} RG_PMIC_A25_FIELD_T;

#define RG_PMIC_A26                               (CHIP_ANA_REG_BASE + 0x6c)
// Bit 0           rg_aoldo_oc_en                 U     RW        default = 'h1
// Bit 1           rg_aoldo_dischr_en             U     RW        default = 'h1
// Bit 5   :2      rg_aoldo_vosel                 U     RW        default = 'h8
// Bit 9   :6      rg_aoldo_ocsel                 U     RW        default = 'h8
// Bit 13  :10     rg_aoldo_comp_adj              U     RW        default = 'h8
// Bit 16  :14     rg_aoldo_dummy_ld              U     RW        default = 'h4
// Bit 20  :17     rg_aoldo_rsva                  U     RW        default = 'h8
// Bit 24  :21     rg_aoldo_rsvb                  U     RW        default = 'h8
// Bit 28  :25     rg_aoldo_rsvc                  U     RW        default = 'h8
typedef union RG_PMIC_A26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_AOLDO_OC_EN : 1;
    unsigned int RG_AOLDO_DISCHR_EN : 1;
    unsigned int RG_AOLDO_VOSEL : 4;
    unsigned int RG_AOLDO_OCSEL : 4;
    unsigned int RG_AOLDO_COMP_ADJ : 4;
    unsigned int RG_AOLDO_DUMMY_LD : 3;
    unsigned int RG_AOLDO_RSVA : 4;
    unsigned int RG_AOLDO_RSVB : 4;
    unsigned int RG_AOLDO_RSVC : 4;
    unsigned int rsvd_0 : 3;
  } b;
} RG_PMIC_A26_FIELD_T;

#define RG_PMIC_A27                               (CHIP_ANA_REG_BASE + 0x70)
// Bit 3   :0      rg_aoldo_rsvd                  U     RW        default = 'h0
// Bit 7   :4      rg_aoldo_rsve                  U     RW        default = 'hf
// Bit 8           ro_aoldo_oc_status             U     RO        default = 'h0
typedef union RG_PMIC_A27_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_AOLDO_RSVD : 4;
    unsigned int RG_AOLDO_RSVE : 4;
    unsigned int RO_AOLDO_OC_STATUS : 1;
    unsigned int rsvd_0 : 23;
  } b;
} RG_PMIC_A27_FIELD_T;

#define RG_PMIC_A28                               (CHIP_ANA_REG_BASE + 0x74)
// Bit 0           rg_cldo_dischr_en              U     RW        default = 'h1
// Bit 6   :1      rg_cldo_vo_adj                 U     RW        default = 'h1f
// Bit 10  :7      rg_cldo_oc_adj                 U     RW        default = 'h8
// Bit 11          rg_cldo_oc_en                  U     RW        default = 'h1
// Bit 12          rg_cldo_uv_en                  U     RW        default = 'h0
// Bit 16  :13     rg_cldo_comp_adj               U     RW        default = 'h8
// Bit 19  :17     rg_cldo_dummy_ld               U     RW        default = 'h4
// Bit 23  :20     rg_cldo_rsva                   U     RW        default = 'h8
// Bit 27  :24     rg_cldo_rsvb                   U     RW        default = 'h8
// Bit 31  :28     rg_cldo_rsvc                   U     RW        default = 'h8
typedef union RG_PMIC_A28_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_CLDO_DISCHR_EN : 1;
    unsigned int RG_CLDO_VO_ADJ : 6;
    unsigned int RG_CLDO_OC_ADJ : 4;
    unsigned int RG_CLDO_OC_EN : 1;
    unsigned int RG_CLDO_UV_EN : 1;
    unsigned int RG_CLDO_COMP_ADJ : 4;
    unsigned int RG_CLDO_DUMMY_LD : 3;
    unsigned int RG_CLDO_RSVA : 4;
    unsigned int RG_CLDO_RSVB : 4;
    unsigned int RG_CLDO_RSVC : 4;
  } b;
} RG_PMIC_A28_FIELD_T;

#define RG_PMIC_A29                               (CHIP_ANA_REG_BASE + 0x78)
// Bit 3   :0      rg_cldo_rsvd                   U     RW        default = 'h0
// Bit 7   :4      rg_cldo_rsve                   U     RW        default = 'hf
// Bit 8           ro_cldo_oc_status              U     RO        default = 'h0
// Bit 9           ro_cldo_uv_status              U     RO        default = 'h0
typedef union RG_PMIC_A29_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_CLDO_RSVD : 4;
    unsigned int RG_CLDO_RSVE : 4;
    unsigned int RO_CLDO_OC_STATUS : 1;
    unsigned int RO_CLDO_UV_STATUS : 1;
    unsigned int rsvd_0 : 22;
  } b;
} RG_PMIC_A29_FIELD_T;

#define RG_PMIC_A30                               (CHIP_ANA_REG_BASE + 0x7c)
// Bit 7   :0      rg_aldo_vo_adj                 U     RW        default = 'h80
// Bit 11  :8      rg_aldo_comp_adj               U     RW        default = 'h8
// Bit 15  :12     rg_aldo_bias_adj               U     RW        default = 'h8
// Bit 19  :16     rg_aldo_oc_adj                 U     RW        default = 'h8
// Bit 20          rg_aldo_fast_dischr_en         U     RW        default = 'h1
// Bit 21          rg_aldo_fc_en                  U     RW        default = 'h1
// Bit 23  :22     rg_aldo_fc_adj                 U     RW        default = 'h2
// Bit 31  :24     rg_aldo_rsva                   U     RW        default = 'h80
typedef union RG_PMIC_A30_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_ALDO_VO_ADJ : 8;
    unsigned int RG_ALDO_COMP_ADJ : 4;
    unsigned int RG_ALDO_BIAS_ADJ : 4;
    unsigned int RG_ALDO_OC_ADJ : 4;
    unsigned int RG_ALDO_FAST_DISCHR_EN : 1;
    unsigned int RG_ALDO_FC_EN : 1;
    unsigned int RG_ALDO_FC_ADJ : 2;
    unsigned int RG_ALDO_RSVA : 8;
  } b;
} RG_PMIC_A30_FIELD_T;

#define RG_PMIC_A31                               (CHIP_ANA_REG_BASE + 0x80)
// Bit 7   :0      rg_aldo_rsvb                   U     RW        default = 'h80
// Bit 15  :8      rg_aldo_rsvc                   U     RW        default = 'h80
// Bit 23  :16     rg_aldo_rsvd                   U     RW        default = 'h80
// Bit 24          ro_aldo_oc_status              U     RO        default = 'h0
// Bit 25          ro_aldo_status_rsva            U     RO        default = 'h0
// Bit 26          ro_aldo_status_rsvb            U     RO        default = 'h0
typedef union RG_PMIC_A31_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_ALDO_RSVB : 8;
    unsigned int RG_ALDO_RSVC : 8;
    unsigned int RG_ALDO_RSVD : 8;
    unsigned int RO_ALDO_OC_STATUS : 1;
    unsigned int RO_ALDO_STATUS_RSVA : 1;
    unsigned int RO_ALDO_STATUS_RSVB : 1;
    unsigned int rsvd_0 : 5;
  } b;
} RG_PMIC_A31_FIELD_T;

#endif

