#ifdef RF_D_SX_REG
#else
#define RF_D_SX_REG


#define RF_D_SX_REG_BASE                          (0x400)

#define RG_SX_A0                                  (RF_D_SX_REG_BASE + 0x0)
// Bit 0           rg_wf_sx_fcal_en               U     RW        default = 'h0
// Bit 2           rg_wf_sx_cali_start_man_mode     U     RW        default = 'h1
// Bit 4           rg_wf_sx_sdm_frac_en           U     RW        default = 'h1
// Bit 12  :8      rg_wf_sx_fcal_mode_sel         U     RW        default = 'h1b
// Bit 16          rg_wf_sx_rf_osc_double         U     RW        default = 'h0
// Bit 20          rg_wf_sx_dig_osc_double        U     RW        default = 'h0
// Bit 31          rg_wf_sx_soft_reset            U     RW        default = 'h0
typedef union RG_SX_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_FCAL_EN : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_SX_CALI_START_MAN_MODE : 1;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_SX_SDM_FRAC_EN : 1;
    unsigned int rsvd_2 : 3;
    unsigned int RG_WF_SX_FCAL_MODE_SEL : 5;
    unsigned int rsvd_3 : 3;
    unsigned int RG_WF_SX_RF_OSC_DOUBLE : 1;
    unsigned int rsvd_4 : 3;
    unsigned int RG_WF_SX_DIG_OSC_DOUBLE : 1;
    unsigned int rsvd_5 : 10;
    unsigned int RG_WF_SX_SOFT_RESET : 1;
  } b;
} RG_SX_A0_FIELD_T;

#define RG_SX_A1                                  (RF_D_SX_REG_BASE + 0x4)
// Bit 0           rg_wf_sx_fcal_sel_p            U     RW        default = 'h0
// Bit 10  :4      rg_wf_sx_vcoc_p                U     RW        default = 'h40
// Bit 12          rg_wf_sx_fcal_sel_t            U     RW        default = 'h0
// Bit 20  :16     rg_wf_sx_vcoc_t                U     RW        default = 'h10
// Bit 24          rg_wf_sx_ldo_fc_man            U     RW        default = 'h0
// Bit 28          rg_wf_sx_ldo_fc                U     RW        default = 'h0
typedef union RG_SX_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_FCAL_SEL_P : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_SX_VCOC_P : 7;
    unsigned int rsvd_1 : 1;
    unsigned int RG_WF_SX_FCAL_SEL_T : 1;
    unsigned int rsvd_2 : 3;
    unsigned int RG_WF_SX_VCOC_T : 5;
    unsigned int rsvd_3 : 3;
    unsigned int RG_WF_SX_LDO_FC_MAN : 1;
    unsigned int rsvd_4 : 3;
    unsigned int RG_WF_SX_LDO_FC : 1;
    unsigned int rsvd_5 : 3;
  } b;
} RG_SX_A1_FIELD_T;

#define RG_SX_A2                                  (RF_D_SX_REG_BASE + 0x8)
// Bit 6   :0      ro_da_wf_sx_vcoc_p             U     RO        default = 'h0
// Bit 20  :16     ro_da_wf_sx_vcoc_t             U     RO        default = 'h0
// Bit 24          ro_wf_sx_mxrtank_cali_ready     U     RO        default = 'h0
// Bit 25          ro_wf_sx_chn_map_ready         U     RO        default = 'h0
// Bit 26          ro_wf_sx_p_cali_ready          U     RO        default = 'h0
// Bit 27          ro_wf_sx_a_cali_ready          U     RO        default = 'h0
// Bit 28          ro_wf_sx_t_cali_ready          U     RO        default = 'h0
// Bit 29          ro_ad_wf_sx_ready              U     RO        default = 'h0
// Bit 31          ro_cali_end_flg                U     RO        default = 'h0
typedef union RG_SX_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_DA_WF_SX_VCOC_P : 7;
    unsigned int rsvd_0 : 9;
    unsigned int RO_DA_WF_SX_VCOC_T : 5;
    unsigned int rsvd_1 : 3;
    unsigned int RO_WF_SX_MXRTANK_CALI_READY : 1;
    unsigned int RO_WF_SX_CHN_MAP_READY : 1;
    unsigned int RO_WF_SX_P_CALI_READY : 1;
    unsigned int RO_WF_SX_A_CALI_READY : 1;
    unsigned int RO_WF_SX_T_CALI_READY : 1;
    unsigned int RO_AD_WF_SX_READY : 1;
    unsigned int rsvd_2 : 1;
    unsigned int RO_CALI_END_FLG : 1;
  } b;
} RG_SX_A2_FIELD_T;

#define RG_SX_A3                                  (RF_D_SX_REG_BASE + 0xc)
// Bit 1   :0      rg_wf_sx_ldo_fc_waittime       U     RW        default = 'h2
// Bit 5   :4      rg_wf_sx_set_code_waittime     U     RW        default = 'h1
// Bit 9   :8      rg_wf_sx_fcal_cw_waittime      U     RW        default = 'h1
// Bit 13  :12     rg_wf_sx_tr_wait_waittime      U     RW        default = 'h1
// Bit 17  :16     rg_wf_sx_acal_vco_waittime     U     RW        default = 'h1
// Bit 21  :20     rg_wf_sx_tr_switch_bnd         U     RW        default = 'h0
typedef union RG_SX_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LDO_FC_WAITTIME : 2;
    unsigned int rsvd_0 : 2;
    unsigned int RG_WF_SX_SET_CODE_WAITTIME : 2;
    unsigned int rsvd_1 : 2;
    unsigned int RG_WF_SX_FCAL_CW_WAITTIME : 2;
    unsigned int rsvd_2 : 2;
    unsigned int RG_WF_SX_TR_WAIT_WAITTIME : 2;
    unsigned int rsvd_3 : 2;
    unsigned int RG_WF_SX_ACAL_VCO_WAITTIME : 2;
    unsigned int rsvd_4 : 2;
    unsigned int RG_WF_SX_TR_SWITCH_BND : 2;
    unsigned int rsvd_5 : 10;
  } b;
} RG_SX_A3_FIELD_T;

#define RG_SX_A4                                  (RF_D_SX_REG_BASE + 0x10)
// Bit 0           rg_wf_sx_vco_i_man             U     RW        default = 'h0
// Bit 7   :4      rg_wf_sx_vco_i                 U     RW        default = 'h8
// Bit 15  :8      rg_wf_sx_pd_vctrl_sel          U     RW        default = 'h3
typedef union RG_SX_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_VCO_I_MAN : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_SX_VCO_I : 4;
    unsigned int RG_WF_SX_PD_VCTRL_SEL : 8;
    unsigned int rsvd_1 : 16;
  } b;
} RG_SX_A4_FIELD_T;

#define RG_SX_A5                                  (RF_D_SX_REG_BASE + 0x14)
// Bit 3   :0      ro_wf_sx_vco_i                 U     RO        default = 'h0
// Bit 4           ro_da_wf_sx_vco_acfinish       U     RO        default = 'h0
// Bit 8           ro_ad_wf_sx_vco_alc_out        U     RO        default = 'h0
// Bit 23  :16     ro_wf_sx_pt_fsm                U     RO        default = 'h0
// Bit 30  :24     ro_wf_sx_top_fsm               U     RO        default = 'h0
typedef union RG_SX_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_SX_VCO_I : 4;
    unsigned int RO_DA_WF_SX_VCO_ACFINISH : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RO_AD_WF_SX_VCO_ALC_OUT : 1;
    unsigned int rsvd_1 : 7;
    unsigned int RO_WF_SX_PT_FSM : 8;
    unsigned int RO_WF_SX_TOP_FSM : 7;
    unsigned int rsvd_2 : 1;
  } b;
} RG_SX_A5_FIELD_T;

#define RG_SX_A6                                  (RF_D_SX_REG_BASE + 0x18)
// Bit 0           rg_sx_pdiv_sel                 U     RW        default = 'h1
// Bit 7   :4      rg_sx_osc_freq                 U     RW        default = 'h3
// Bit 15  :8      rg_2gb_sx_channel              U     RW        default = 'h8
// Bit 23  :16     rg_5gb_sx_channel              U     RW        default = 'h64
// Bit 27  :24     rg_wifi_freq_mode_man          U     RW        default = 'h0
// Bit 31          rg_wifi_freq_mode_man_mode     U     RW        default = 'h0
typedef union RG_SX_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_SX_PDIV_SEL : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RG_SX_OSC_FREQ : 4;
    unsigned int RG_2GB_SX_CHANNEL : 8;
    unsigned int RG_5GB_SX_CHANNEL : 8;
    unsigned int RG_WIFI_FREQ_MODE_MAN : 4;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WIFI_FREQ_MODE_MAN_MODE : 1;
  } b;
} RG_SX_A6_FIELD_T;

#define RG_SX_A7                                  (RF_D_SX_REG_BASE + 0x1c)
// Bit 0           rg_sx_rfch_man_en              U     RW        default = 'h0
// Bit 14  :4      rg_wf_sx_rfctrl_int            U     RW        default = 'h0
typedef union RG_SX_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_SX_RFCH_MAN_EN : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_SX_RFCTRL_INT : 11;
    unsigned int rsvd_1 : 17;
  } b;
} RG_SX_A7_FIELD_T;

#define RG_SX_A8                                  (RF_D_SX_REG_BASE + 0x20)
// Bit 24  :0      rg_wf_sx_rfctrl_frac           U     RW        default = 'h0
typedef union RG_SX_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_RFCTRL_FRAC : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_SX_A8_FIELD_T;

#define RG_SX_A9                                  (RF_D_SX_REG_BASE + 0x24)
// Bit 15  :0      rg_sx_fcal_ntargt              U     RW        default = 'h0
// Bit 16          rg_sx_fcal_ntargt_man          U     RW        default = 'h0
typedef union RG_SX_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_SX_FCAL_NTARGT : 16;
    unsigned int RG_SX_FCAL_NTARGT_MAN : 1;
    unsigned int rsvd_0 : 15;
  } b;
} RG_SX_A9_FIELD_T;

#define RG_SX_A10                                 (RF_D_SX_REG_BASE + 0x28)
// Bit 15  :0      ro_wf_sx_fcal_cnt              U     RO        default = 'h0
// Bit 28  :16     ro_wf_sx_rf_freq_mhz           U     RO        default = 'h0
typedef union RG_SX_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_SX_FCAL_CNT : 16;
    unsigned int RO_WF_SX_RF_FREQ_MHZ : 13;
    unsigned int rsvd_0 : 3;
  } b;
} RG_SX_A10_FIELD_T;

#define RG_SX_A11                                 (RF_D_SX_REG_BASE + 0x2c)
// Bit 1   :0      rg_wf_sx_fcal_hvt              U     RW        default = 'h1
// Bit 5   :4      rg_wf_sx_fcal_lvt              U     RW        default = 'h1
typedef union RG_SX_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_FCAL_HVT : 2;
    unsigned int rsvd_0 : 2;
    unsigned int RG_WF_SX_FCAL_LVT : 2;
    unsigned int rsvd_1 : 26;
  } b;
} RG_SX_A11_FIELD_T;

#define RG_SX_A12                                 (RF_D_SX_REG_BASE + 0x30)
// Bit 10  :0      ro_sx_rfctrl_nint              U     RO        default = 'h0
typedef union RG_SX_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_SX_RFCTRL_NINT : 11;
    unsigned int rsvd_0 : 21;
  } b;
} RG_SX_A12_FIELD_T;

#define RG_SX_A13                                 (RF_D_SX_REG_BASE + 0x34)
// Bit 24  :0      ro_sx_rfctrl_nfrac             U     RO        default = 'h0
typedef union RG_SX_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_SX_RFCTRL_NFRAC : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_SX_A13_FIELD_T;

#define RG_SX_A14                                 (RF_D_SX_REG_BASE + 0x38)
// Bit 15  :0      ro_sx_fcal_ntargt              U     RO        default = 'h0
typedef union RG_SX_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_SX_FCAL_NTARGT : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_SX_A14_FIELD_T;

#define RG_SX_A15                                 (RF_D_SX_REG_BASE + 0x3c)
// Bit 6           rg_tr_switch_man               U     RW        default = 'h0
// Bit 7           rg_tr_switch_val               U     RW        default = 'h0
typedef union RG_SX_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 6;
    unsigned int RG_TR_SWITCH_MAN : 1;
    unsigned int RG_TR_SWITCH_VAL : 1;
    unsigned int rsvd_1 : 24;
  } b;
} RG_SX_A15_FIELD_T;

#define RG_SX_A16                                 (RF_D_SX_REG_BASE + 0x40)
// Bit 2   :0      rg_wf_sx_log_pdthreshold_man     U     RW        default = 'h7
// Bit 4           rg_wf_sx_log_pd_man_mode       U     RW        default = 'h0
// Bit 8           rg_wf_sx_log_pdtest_en         U     RW        default = 'h0
typedef union RG_SX_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LOG_PDTHRESHOLD_MAN : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_SX_LOG_PD_MAN_MODE : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WF_SX_LOG_PDTEST_EN : 1;
    unsigned int rsvd_2 : 23;
  } b;
} RG_SX_A16_FIELD_T;

#define RG_SX_A17                                 (RF_D_SX_REG_BASE + 0x44)
// Bit 3   :0      rg_wf_sx_log5g_mxr_banksel_man0     U     RW        default = 'he
// Bit 7   :4      rg_wf_sx_log5g_mxr_banksel_man1     U     RW        default = 'hd
// Bit 11  :8      rg_wf_sx_log5g_mxr_banksel_man2     U     RW        default = 'hc
// Bit 15  :12     rg_wf_sx_log5g_mxr_banksel_man3     U     RW        default = 'hb
// Bit 19  :16     rg_wf_sx_log5g_mxr_banksel_man4     U     RW        default = 'ha
// Bit 23  :20     rg_wf_sx_log5g_mxr_banksel_man5     U     RW        default = 'h9
// Bit 27  :24     rg_wf_sx_log5g_mxr_banksel_man6     U     RW        default = 'h8
// Bit 31  :28     rg_wf_sx_log5g_mxr_banksel_man7     U     RW        default = 'h7
typedef union RG_SX_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN0 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN1 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN2 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN3 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN4 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN5 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN6 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN7 : 4;
  } b;
} RG_SX_A17_FIELD_T;

#define RG_SX_A18                                 (RF_D_SX_REG_BASE + 0x48)
// Bit 3   :0      rg_wf_sx_log5g_mxr_banksel_man8     U     RW        default = 'h6
// Bit 7   :4      rg_wf_sx_log5g_mxr_banksel_man9     U     RW        default = 'h4
// Bit 11  :8      rg_wf_sx_log2g_mxr_banksel_man     U     RW        default = 'h6
typedef union RG_SX_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN8 : 4;
    unsigned int RG_WF_SX_LOG5G_MXR_BANKSEL_MAN9 : 4;
    unsigned int RG_WF_SX_LOG2G_MXR_BANKSEL_MAN : 4;
    unsigned int rsvd_0 : 20;
  } b;
} RG_SX_A18_FIELD_T;

#define RG_SX_A20                                 (RF_D_SX_REG_BASE + 0x50)
// Bit 3   :0      ro_wf_sx_log5g_mxr_banksel_0     U     RO        default = 'h0
// Bit 7   :4      ro_wf_sx_log5g_mxr_banksel_1     U     RO        default = 'h0
// Bit 11  :8      ro_wf_sx_log5g_mxr_banksel_2     U     RO        default = 'h0
// Bit 15  :12     ro_wf_sx_log5g_mxr_banksel_3     U     RO        default = 'h0
// Bit 19  :16     ro_wf_sx_log5g_mxr_banksel_4     U     RO        default = 'h0
// Bit 23  :20     ro_wf_sx_log5g_mxr_banksel_5     U     RO        default = 'h0
// Bit 27  :24     ro_wf_sx_log5g_mxr_banksel_6     U     RO        default = 'h0
// Bit 31  :28     ro_wf_sx_log5g_mxr_banksel_7     U     RO        default = 'h0
typedef union RG_SX_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_0 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_1 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_2 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_3 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_4 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_5 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_6 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_7 : 4;
  } b;
} RG_SX_A20_FIELD_T;

#define RG_SX_A21                                 (RF_D_SX_REG_BASE + 0x54)
// Bit 3   :0      ro_wf_sx_log5g_mxr_banksel_8     U     RO        default = 'h0
// Bit 7   :4      ro_wf_sx_log5g_mxr_banksel_9     U     RO        default = 'h0
// Bit 11  :8      ro_wf_sx_log2g_mxr_banksel     U     RO        default = 'h0
typedef union RG_SX_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_8 : 4;
    unsigned int RO_WF_SX_LOG5G_MXR_BANKSEL_9 : 4;
    unsigned int RO_WF_SX_LOG2G_MXR_BANKSEL : 4;
    unsigned int rsvd_0 : 20;
  } b;
} RG_SX_A21_FIELD_T;

#define RG_SX_A23                                 (RF_D_SX_REG_BASE + 0x5c)
// Bit 0           rg_m0_wf_sx_vco_en             U     RW        default = 'h0
// Bit 1           rg_m0_wf_sx_vco_acal_en        U     RW        default = 'h0
// Bit 2           rg_m0_wf_sx_pfd_en             U     RW        default = 'h0
// Bit 3           rg_m0_wf_sx_cp_en              U     RW        default = 'h0
// Bit 4           rg_m0_wf_sx_vco_ldo_en         U     RW        default = 'h0
// Bit 5           rg_m0_wf_sx_cp_ldo_en          U     RW        default = 'h0
// Bit 6           rg_m0_wf_sx_lpf_comp_en        U     RW        default = 'h0
// Bit 7           rg_m0_wf_sx_log5g_mxr_en       U     RW        default = 'h0
// Bit 8           rg_m0_wf_sx_log5g_inv_en       U     RW        default = 'h0
// Bit 9           rg_m0_wf_sx_log5g_div_en       U     RW        default = 'h0
// Bit 10          rg_m0_wf_sx_log5g_iqdiv_en     U     RW        default = 'h0
// Bit 11          rg_m0_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m0_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m0_wf_sx_log5g_txlo_en      U     RW        default = 'h0
// Bit 14          rg_m0_wf_sx_log2g_mxr_en       U     RW        default = 'h0
// Bit 15          rg_m0_wf_sx_log2g_div_en       U     RW        default = 'h0
// Bit 16          rg_m0_wf_sx_log2g_iqdiv_en     U     RW        default = 'h0
// Bit 17          rg_m0_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m0_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m0_wf_sx_log2g_txlo_en      U     RW        default = 'h0
typedef union RG_SX_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M0_WF_SX_VCO_EN : 1;
    unsigned int RG_M0_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M0_WF_SX_PFD_EN : 1;
    unsigned int RG_M0_WF_SX_CP_EN : 1;
    unsigned int RG_M0_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M0_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M0_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M0_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M0_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M0_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M0_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M0_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M0_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M0_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A23_FIELD_T;

#define RG_SX_A24                                 (RF_D_SX_REG_BASE + 0x60)
// Bit 0           rg_m1_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m1_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m1_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m1_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m1_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m1_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m1_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m1_wf_sx_log5g_mxr_en       U     RW        default = 'h1
// Bit 8           rg_m1_wf_sx_log5g_inv_en       U     RW        default = 'h1
// Bit 9           rg_m1_wf_sx_log5g_div_en       U     RW        default = 'h1
// Bit 10          rg_m1_wf_sx_log5g_iqdiv_en     U     RW        default = 'h1
// Bit 11          rg_m1_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m1_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m1_wf_sx_log5g_txlo_en      U     RW        default = 'h0
// Bit 14          rg_m1_wf_sx_log2g_mxr_en       U     RW        default = 'h0
// Bit 15          rg_m1_wf_sx_log2g_div_en       U     RW        default = 'h0
// Bit 16          rg_m1_wf_sx_log2g_iqdiv_en     U     RW        default = 'h0
// Bit 17          rg_m1_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m1_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m1_wf_sx_log2g_txlo_en      U     RW        default = 'h0
typedef union RG_SX_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M1_WF_SX_VCO_EN : 1;
    unsigned int RG_M1_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M1_WF_SX_PFD_EN : 1;
    unsigned int RG_M1_WF_SX_CP_EN : 1;
    unsigned int RG_M1_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M1_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M1_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M1_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M1_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M1_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M1_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M1_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M1_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M1_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A24_FIELD_T;

#define RG_SX_A25                                 (RF_D_SX_REG_BASE + 0x64)
// Bit 0           rg_m2_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m2_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m2_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m2_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m2_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m2_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m2_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m2_wf_sx_log5g_mxr_en       U     RW        default = 'h1
// Bit 8           rg_m2_wf_sx_log5g_inv_en       U     RW        default = 'h1
// Bit 9           rg_m2_wf_sx_log5g_div_en       U     RW        default = 'h1
// Bit 10          rg_m2_wf_sx_log5g_iqdiv_en     U     RW        default = 'h1
// Bit 11          rg_m2_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m2_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m2_wf_sx_log5g_txlo_en      U     RW        default = 'h1
// Bit 14          rg_m2_wf_sx_log2g_mxr_en       U     RW        default = 'h0
// Bit 15          rg_m2_wf_sx_log2g_div_en       U     RW        default = 'h0
// Bit 16          rg_m2_wf_sx_log2g_iqdiv_en     U     RW        default = 'h0
// Bit 17          rg_m2_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m2_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m2_wf_sx_log2g_txlo_en      U     RW        default = 'h1
typedef union RG_SX_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M2_WF_SX_VCO_EN : 1;
    unsigned int RG_M2_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M2_WF_SX_PFD_EN : 1;
    unsigned int RG_M2_WF_SX_CP_EN : 1;
    unsigned int RG_M2_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M2_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M2_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M2_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M2_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M2_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M2_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M2_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M2_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M2_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A25_FIELD_T;

#define RG_SX_A26                                 (RF_D_SX_REG_BASE + 0x68)
// Bit 0           rg_m3_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m3_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m3_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m3_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m3_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m3_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m3_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m3_wf_sx_log5g_mxr_en       U     RW        default = 'h1
// Bit 8           rg_m3_wf_sx_log5g_inv_en       U     RW        default = 'h1
// Bit 9           rg_m3_wf_sx_log5g_div_en       U     RW        default = 'h1
// Bit 10          rg_m3_wf_sx_log5g_iqdiv_en     U     RW        default = 'h1
// Bit 11          rg_m3_wf_sx_log5g_rxlo_en      U     RW        default = 'h1
// Bit 12          rg_m3_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m3_wf_sx_log5g_txlo_en      U     RW        default = 'h0
// Bit 14          rg_m3_wf_sx_log2g_mxr_en       U     RW        default = 'h0
// Bit 15          rg_m3_wf_sx_log2g_div_en       U     RW        default = 'h0
// Bit 16          rg_m3_wf_sx_log2g_iqdiv_en     U     RW        default = 'h0
// Bit 17          rg_m3_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m3_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m3_wf_sx_log2g_txlo_en      U     RW        default = 'h0
typedef union RG_SX_A26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M3_WF_SX_VCO_EN : 1;
    unsigned int RG_M3_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M3_WF_SX_PFD_EN : 1;
    unsigned int RG_M3_WF_SX_CP_EN : 1;
    unsigned int RG_M3_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M3_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M3_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M3_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M3_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M3_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M3_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M3_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M3_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M3_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A26_FIELD_T;

#define RG_SX_A27                                 (RF_D_SX_REG_BASE + 0x6c)
// Bit 31  :0      rg_wf_sx_dig_rsv0              U     RW        default = 'hffff
typedef union RG_SX_A27_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_DIG_RSV0 : 32;
  } b;
} RG_SX_A27_FIELD_T;

#define RG_SX_A32                                 (RF_D_SX_REG_BASE + 0x80)
// Bit 2   :0      rg_wf_sx_lpf_bw                U     RW        default = 'h7
// Bit 5   :4      rg_wf_sx_pfd_delay             U     RW        default = 'h0
// Bit 10  :8      rg_wf_sx_cp_offset             U     RW        default = 'h5
// Bit 13  :12     rg_wf_sx_stable_cnt            U     RW        default = 'h0
// Bit 16          rg_wf_sx_stable_en_man_mode     U     RW        default = 'h0
// Bit 20          rg_wf_sx_stable_en_man         U     RW        default = 'h0
typedef union RG_SX_A32_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LPF_BW : 3;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_SX_PFD_DELAY : 2;
    unsigned int rsvd_1 : 2;
    unsigned int RG_WF_SX_CP_OFFSET : 3;
    unsigned int rsvd_2 : 1;
    unsigned int RG_WF_SX_STABLE_CNT : 2;
    unsigned int rsvd_3 : 2;
    unsigned int RG_WF_SX_STABLE_EN_MAN_MODE : 1;
    unsigned int rsvd_4 : 3;
    unsigned int RG_WF_SX_STABLE_EN_MAN : 1;
    unsigned int rsvd_5 : 11;
  } b;
} RG_SX_A32_FIELD_T;

#define RG_SX_A34                                 (RF_D_SX_REG_BASE + 0x88)
// Bit 8   :4      rg_wf_sx_vco_ldo_vref_adj      U     RW        default = 'h14
// Bit 20  :16     rg_wf_sx_cp_ldo_vref_adj       U     RW        default = 'h10
typedef union RG_SX_A34_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 4;
    unsigned int RG_WF_SX_VCO_LDO_VREF_ADJ : 5;
    unsigned int rsvd_1 : 7;
    unsigned int RG_WF_SX_CP_LDO_VREF_ADJ : 5;
    unsigned int rsvd_2 : 11;
  } b;
} RG_SX_A34_FIELD_T;

#define RG_SX_A35                                 (RF_D_SX_REG_BASE + 0x8c)
// Bit 2   :0      rg_wf_sx_cp_i                  U     RW        default = 'h5
// Bit 4   :3      rg_wf_sx_pfd_lock_t            U     RW        default = 'h0
// Bit 5           rg_wf_sx_pfd_ph_en             U     RW        default = 'h1
// Bit 6           rg_wf_sx_pfd_sel               U     RW        default = 'h0
// Bit 8           rg_wf_sx_mmd_pw_en             U     RW        default = 'h1
// Bit 9           rg_wf_sx_cp_bypass_mode        U     RW        default = 'h0
// Bit 10          rg_wf_sx_lpf_bypass_mode       U     RW        default = 'h0
// Bit 11          rg_wf_sx_lpf_resample_mode     U     RW        default = 'h1
// Bit 12          rg_wf_sx_cp_cali_ctrl          U     RW        default = 'h0
// Bit 13          rg_wf_sx_lpf_cali_ctrl         U     RW        default = 'h0
// Bit 14          rg_wf_sx_cp_rc_ctrl            U     RW        default = 'h0
// Bit 15          rg_wf_sx_ldo_hf_rc_ctrl        U     RW        default = 'h1
// Bit 16          rg_wf_sx_ldo_hf_rc_modesel     U     RW        default = 'h0
// Bit 17          rg_wf_sx_ldo_lf_rc_ctrl        U     RW        default = 'h1
// Bit 18          rg_wf_sx_ldo_lf_rc_modesel     U     RW        default = 'h0
// Bit 19          rg_wf_sx_pll_rst_man           U     RW        default = 'h0
// Bit 20          rg_wf_sx_pll_rst_man_val       U     RW        default = 'h0
// Bit 22  :21     rg_wf_sx_vctrl_sel             U     RW        default = 'h0
// Bit 24  :23     rg_wf_sx_vco_var_dc            U     RW        default = 'h3
// Bit 25          rg_wf_sx_fcal_div              U     RW        default = 'h1
// Bit 27  :26     rg_wf_sx_vco_2nd_capn          U     RW        default = 'h1
// Bit 29  :28     rg_wf_sx_vco_2nd_capp          U     RW        default = 'h1
// Bit 30          rg_sx_vco_lpf_testmode         U     RW        default = 'h0
typedef union RG_SX_A35_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_CP_I : 3;
    unsigned int RG_WF_SX_PFD_LOCK_T : 2;
    unsigned int RG_WF_SX_PFD_PH_EN : 1;
    unsigned int RG_WF_SX_PFD_SEL : 1;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_SX_MMD_PW_EN : 1;
    unsigned int RG_WF_SX_CP_BYPASS_MODE : 1;
    unsigned int RG_WF_SX_LPF_BYPASS_MODE : 1;
    unsigned int RG_WF_SX_LPF_RESAMPLE_MODE : 1;
    unsigned int RG_WF_SX_CP_CALI_CTRL : 1;
    unsigned int RG_WF_SX_LPF_CALI_CTRL : 1;
    unsigned int RG_WF_SX_CP_RC_CTRL : 1;
    unsigned int RG_WF_SX_LDO_HF_RC_CTRL : 1;
    unsigned int RG_WF_SX_LDO_HF_RC_MODESEL : 1;
    unsigned int RG_WF_SX_LDO_LF_RC_CTRL : 1;
    unsigned int RG_WF_SX_LDO_LF_RC_MODESEL : 1;
    unsigned int RG_WF_SX_PLL_RST_MAN : 1;
    unsigned int RG_WF_SX_PLL_RST_MAN_VAL : 1;
    unsigned int RG_WF_SX_VCTRL_SEL : 2;
    unsigned int RG_WF_SX_VCO_VAR_DC : 2;
    unsigned int RG_WF_SX_FCAL_DIV : 1;
    unsigned int RG_WF_SX_VCO_2ND_CAPN : 2;
    unsigned int RG_WF_SX_VCO_2ND_CAPP : 2;
    unsigned int RG_SX_VCO_LPF_TESTMODE : 1;
    unsigned int rsvd_1 : 1;
  } b;
} RG_SX_A35_FIELD_T;

#define RG_SX_A36                                 (RF_D_SX_REG_BASE + 0x90)
// Bit 0           rg_wf_sx_log5g_pd_en_man       U     RW        default = 'h0
// Bit 4           rg_wf_sx_log2g_pd_en_man       U     RW        default = 'h0
// Bit 11  :8      rg_wf_sx_log5g_mxr_ictrl       U     RW        default = 'ha
// Bit 15  :12     rg_wf_sx_log5g_div_vdd         U     RW        default = 'h1
// Bit 31          rg_wf_sx_log5g_reserve1        U     RW        default = 'h0
typedef union RG_SX_A36_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LOG5G_PD_EN_MAN : 1;
    unsigned int rsvd_0 : 3;
    unsigned int RG_WF_SX_LOG2G_PD_EN_MAN : 1;
    unsigned int rsvd_1 : 3;
    unsigned int RG_WF_SX_LOG5G_MXR_ICTRL : 4;
    unsigned int RG_WF_SX_LOG5G_DIV_VDD : 4;
    unsigned int rsvd_2 : 15;
    unsigned int RG_WF_SX_LOG5G_RESERVE1 : 1;
  } b;
} RG_SX_A36_FIELD_T;

#define RG_SX_A37                                 (RF_D_SX_REG_BASE + 0x94)
// Bit 3   :0      rg_wf_sx_log5g_rxlo_vdd        U     RW        default = 'h7
// Bit 7   :4      rg_wf_sx_log5g_dpdlo_vdd       U     RW        default = 'h7
// Bit 11  :8      rg_wf_sx_log5g_txlo_vdd        U     RW        default = 'h7
// Bit 15  :12     rg_wf_sx_log5g_iqdiv_rctrl     U     RW        default = 'h0
// Bit 20  :16     rg_wf_sx_log5g_iqdiv_ictrl     U     RW        default = 'h6
typedef union RG_SX_A37_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LOG5G_RXLO_VDD : 4;
    unsigned int RG_WF_SX_LOG5G_DPDLO_VDD : 4;
    unsigned int RG_WF_SX_LOG5G_TXLO_VDD : 4;
    unsigned int RG_WF_SX_LOG5G_IQDIV_RCTRL : 4;
    unsigned int RG_WF_SX_LOG5G_IQDIV_ICTRL : 5;
    unsigned int rsvd_0 : 11;
  } b;
} RG_SX_A37_FIELD_T;

#define RG_SX_A38                                 (RF_D_SX_REG_BASE + 0x98)
// Bit 3   :0      rg_wf_sx_log2g_mxr_vdd         U     RW        default = 'hf
// Bit 7   :4      rg_wf_sx_log2g_div_vdd         U     RW        default = 'h1
// Bit 11  :8      rg_wf_sx_log2g_iqdiv_vdd       U     RW        default = 'h1
// Bit 15  :12     rg_wf_sx_log2g_rxlo_vdd        U     RW        default = 'h4
// Bit 19  :16     rg_wf_sx_log2g_dpdlo_vdd       U     RW        default = 'h4
// Bit 23  :20     rg_wf_sx_log2g_txlo_vdd        U     RW        default = 'h4
typedef union RG_SX_A38_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_LOG2G_MXR_VDD : 4;
    unsigned int RG_WF_SX_LOG2G_DIV_VDD : 4;
    unsigned int RG_WF_SX_LOG2G_IQDIV_VDD : 4;
    unsigned int RG_WF_SX_LOG2G_RXLO_VDD : 4;
    unsigned int RG_WF_SX_LOG2G_DPDLO_VDD : 4;
    unsigned int RG_WF_SX_LOG2G_TXLO_VDD : 4;
    unsigned int rsvd_0 : 8;
  } b;
} RG_SX_A38_FIELD_T;

#define RG_SX_A42                                 (RF_D_SX_REG_BASE + 0xa8)
// Bit 31  :0      rg_wf_sx_rsv0                  U     RW        default = 'hfffb
typedef union RG_SX_A42_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_RSV0 : 32;
  } b;
} RG_SX_A42_FIELD_T;

#define RG_SX_A43                                 (RF_D_SX_REG_BASE + 0xac)
// Bit 31  :0      rg_wf_sx_rsv1                  U     RW        default = 'hffff
typedef union RG_SX_A43_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_RSV1 : 32;
  } b;
} RG_SX_A43_FIELD_T;

#define RG_SX_A44                                 (RF_D_SX_REG_BASE + 0xb0)
// Bit 0           rg_wf_sx_double_edge_sel       U     RW        default = 'h0
// Bit 1           rg_wf_sx_double_bypass_mode     U     RW        default = 'h1
// Bit 2           rg_wf_sx_double_dig_loop_en     U     RW        default = 'h0
// Bit 3           rg_wf_sx_double_ana_loop_en     U     RW        default = 'h0
// Bit 4           rg_wf_sx_double_half_lsb_ctrl     U     RW        default = 'h0
// Bit 5           rg_wf_sx_double_half_lsb_sel     U     RW        default = 'h1
// Bit 6           rg_wf_sx_double_post_delay_en     U     RW        default = 'h1
// Bit 7           rg_wf_sx_double_pre_delay_en     U     RW        default = 'h1
// Bit 8           rg_wf_sx_double_op_en          U     RW        default = 'h1
// Bit 11  :9      rg_wf_sx_double_vref_sel       U     RW        default = 'h4
// Bit 19  :12     rg_wf_sx_double_reserved       U     RW        default = 'h0
// Bit 21          rg_wf_sx_double_rst_man_mode     U     RW        default = 'h0
// Bit 22          rg_wf_sx_double_rst_man        U     RW        default = 'h0
// Bit 23          rg_wf_sx_double_tune_man_mode     U     RW        default = 'h0
// Bit 30  :24     rg_wf_sx_double_tune_man       U     RW        default = 'h0
typedef union RG_SX_A44_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_DOUBLE_EDGE_SEL : 1;
    unsigned int RG_WF_SX_DOUBLE_BYPASS_MODE : 1;
    unsigned int RG_WF_SX_DOUBLE_DIG_LOOP_EN : 1;
    unsigned int RG_WF_SX_DOUBLE_ANA_LOOP_EN : 1;
    unsigned int RG_WF_SX_DOUBLE_HALF_LSB_CTRL : 1;
    unsigned int RG_WF_SX_DOUBLE_HALF_LSB_SEL : 1;
    unsigned int RG_WF_SX_DOUBLE_POST_DELAY_EN : 1;
    unsigned int RG_WF_SX_DOUBLE_PRE_DELAY_EN : 1;
    unsigned int RG_WF_SX_DOUBLE_OP_EN : 1;
    unsigned int RG_WF_SX_DOUBLE_VREF_SEL : 3;
    unsigned int RG_WF_SX_DOUBLE_RESERVED : 8;
    unsigned int rsvd_0 : 1;
    unsigned int RG_WF_SX_DOUBLE_RST_MAN_MODE : 1;
    unsigned int RG_WF_SX_DOUBLE_RST_MAN : 1;
    unsigned int RG_WF_SX_DOUBLE_TUNE_MAN_MODE : 1;
    unsigned int RG_WF_SX_DOUBLE_TUNE_MAN : 7;
    unsigned int rsvd_1 : 1;
  } b;
} RG_SX_A44_FIELD_T;

#define RG_SX_A45                                 (RF_D_SX_REG_BASE + 0xb4)
// Bit 0           rg_wf_sx_open_drain_tp_en      U     RW        default = 'h0
// Bit 1           rg_wf_sx_vco_vctrl_tp_sel      U     RW        default = 'h0
// Bit 2           rg_wf_sx_tp_en                 U     RW        default = 'h0
// Bit 5   :3      rg_wf_sx_tp_sel                U     RW        default = 'h4
// Bit 6           rg_wf_sx_fb_clk_edge_sel       U     RW        default = 'h0
// Bit 7           rg_wf_sx_clk_edge_sel          U     RW        default = 'h0
// Bit 8           rg_wf_sx_double_cali_en        U     RW        default = 'h0
// Bit 9           rg_wf_sx_double_en             U     RW        default = 'h0
// Bit 13  :12     rg_wf_sx_double_waittime_25u     U     RW        default = 'h0
// Bit 15  :14     rg_wf_sx_double_waittime_40u     U     RW        default = 'h0
// Bit 23  :16     rg_wf_sx_double_check_code_cnt     U     RW        default = 'h80
// Bit 31          rg_wf_sx_double_duty_flag_sel     U     RW        default = 'h0
typedef union RG_SX_A45_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_WF_SX_OPEN_DRAIN_TP_EN : 1;
    unsigned int RG_WF_SX_VCO_VCTRL_TP_SEL : 1;
    unsigned int RG_WF_SX_TP_EN : 1;
    unsigned int RG_WF_SX_TP_SEL : 3;
    unsigned int RG_WF_SX_FB_CLK_EDGE_SEL : 1;
    unsigned int RG_WF_SX_CLK_EDGE_SEL : 1;
    unsigned int RG_WF_SX_DOUBLE_CALI_EN : 1;
    unsigned int RG_WF_SX_DOUBLE_EN : 1;
    unsigned int rsvd_0 : 2;
    unsigned int RG_WF_SX_DOUBLE_WAITTIME_25U : 2;
    unsigned int RG_WF_SX_DOUBLE_WAITTIME_40U : 2;
    unsigned int RG_WF_SX_DOUBLE_CHECK_CODE_CNT : 8;
    unsigned int rsvd_1 : 7;
    unsigned int RG_WF_SX_DOUBLE_DUTY_FLAG_SEL : 1;
  } b;
} RG_SX_A45_FIELD_T;

#define RG_SX_A46                                 (RF_D_SX_REG_BASE + 0xb8)
// Bit 0           rg_m4_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m4_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m4_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m4_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m4_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m4_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m4_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m4_wf_sx_log5g_mxr_en       U     RW        default = 'h1
// Bit 8           rg_m4_wf_sx_log5g_inv_en       U     RW        default = 'h1
// Bit 9           rg_m4_wf_sx_log5g_div_en       U     RW        default = 'h1
// Bit 10          rg_m4_wf_sx_log5g_iqdiv_en     U     RW        default = 'h1
// Bit 11          rg_m4_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m4_wf_sx_log5g_dpdlo_en     U     RW        default = 'h1
// Bit 13          rg_m4_wf_sx_log5g_txlo_en      U     RW        default = 'h1
// Bit 14          rg_m4_wf_sx_log2g_mxr_en       U     RW        default = 'h0
// Bit 15          rg_m4_wf_sx_log2g_div_en       U     RW        default = 'h0
// Bit 16          rg_m4_wf_sx_log2g_iqdiv_en     U     RW        default = 'h0
// Bit 17          rg_m4_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m4_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m4_wf_sx_log2g_txlo_en      U     RW        default = 'h0
typedef union RG_SX_A46_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M4_WF_SX_VCO_EN : 1;
    unsigned int RG_M4_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M4_WF_SX_PFD_EN : 1;
    unsigned int RG_M4_WF_SX_CP_EN : 1;
    unsigned int RG_M4_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M4_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M4_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M4_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M4_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M4_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M4_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M4_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M4_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M4_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A46_FIELD_T;

#define RG_SX_A47                                 (RF_D_SX_REG_BASE + 0xbc)
// Bit 0           rg_m5_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m5_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m5_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m5_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m5_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m5_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m5_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m5_wf_sx_log5g_mxr_en       U     RW        default = 'h0
// Bit 8           rg_m5_wf_sx_log5g_inv_en       U     RW        default = 'h0
// Bit 9           rg_m5_wf_sx_log5g_div_en       U     RW        default = 'h0
// Bit 10          rg_m5_wf_sx_log5g_iqdiv_en     U     RW        default = 'h0
// Bit 11          rg_m5_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m5_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m5_wf_sx_log5g_txlo_en      U     RW        default = 'h0
// Bit 14          rg_m5_wf_sx_log2g_mxr_en       U     RW        default = 'h1
// Bit 15          rg_m5_wf_sx_log2g_div_en       U     RW        default = 'h1
// Bit 16          rg_m5_wf_sx_log2g_iqdiv_en     U     RW        default = 'h1
// Bit 17          rg_m5_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m5_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m5_wf_sx_log2g_txlo_en      U     RW        default = 'h0
typedef union RG_SX_A47_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M5_WF_SX_VCO_EN : 1;
    unsigned int RG_M5_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M5_WF_SX_PFD_EN : 1;
    unsigned int RG_M5_WF_SX_CP_EN : 1;
    unsigned int RG_M5_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M5_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M5_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M5_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M5_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M5_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M5_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M5_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M5_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M5_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A47_FIELD_T;

#define RG_SX_A48                                 (RF_D_SX_REG_BASE + 0xc0)
// Bit 0           rg_m6_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m6_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m6_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m6_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m6_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m6_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m6_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m6_wf_sx_log5g_mxr_en       U     RW        default = 'h0
// Bit 8           rg_m6_wf_sx_log5g_inv_en       U     RW        default = 'h0
// Bit 9           rg_m6_wf_sx_log5g_div_en       U     RW        default = 'h0
// Bit 10          rg_m6_wf_sx_log5g_iqdiv_en     U     RW        default = 'h0
// Bit 11          rg_m6_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m6_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m6_wf_sx_log5g_txlo_en      U     RW        default = 'h0
// Bit 14          rg_m6_wf_sx_log2g_mxr_en       U     RW        default = 'h1
// Bit 15          rg_m6_wf_sx_log2g_div_en       U     RW        default = 'h1
// Bit 16          rg_m6_wf_sx_log2g_iqdiv_en     U     RW        default = 'h1
// Bit 17          rg_m6_wf_sx_log2g_rxlo_en      U     RW        default = 'h0
// Bit 18          rg_m6_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m6_wf_sx_log2g_txlo_en      U     RW        default = 'h1
typedef union RG_SX_A48_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M6_WF_SX_VCO_EN : 1;
    unsigned int RG_M6_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M6_WF_SX_PFD_EN : 1;
    unsigned int RG_M6_WF_SX_CP_EN : 1;
    unsigned int RG_M6_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M6_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M6_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M6_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M6_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M6_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M6_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M6_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M6_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M6_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A48_FIELD_T;

#define RG_SX_A49                                 (RF_D_SX_REG_BASE + 0xc4)
// Bit 0           rg_m7_wf_sx_vco_en             U     RW        default = 'h1
// Bit 1           rg_m7_wf_sx_vco_acal_en        U     RW        default = 'h1
// Bit 2           rg_m7_wf_sx_pfd_en             U     RW        default = 'h1
// Bit 3           rg_m7_wf_sx_cp_en              U     RW        default = 'h1
// Bit 4           rg_m7_wf_sx_vco_ldo_en         U     RW        default = 'h1
// Bit 5           rg_m7_wf_sx_cp_ldo_en          U     RW        default = 'h1
// Bit 6           rg_m7_wf_sx_lpf_comp_en        U     RW        default = 'h1
// Bit 7           rg_m7_wf_sx_log5g_mxr_en       U     RW        default = 'h0
// Bit 8           rg_m7_wf_sx_log5g_inv_en       U     RW        default = 'h0
// Bit 9           rg_m7_wf_sx_log5g_div_en       U     RW        default = 'h0
// Bit 10          rg_m7_wf_sx_log5g_iqdiv_en     U     RW        default = 'h0
// Bit 11          rg_m7_wf_sx_log5g_rxlo_en      U     RW        default = 'h0
// Bit 12          rg_m7_wf_sx_log5g_dpdlo_en     U     RW        default = 'h0
// Bit 13          rg_m7_wf_sx_log5g_txlo_en      U     RW        default = 'h0
// Bit 14          rg_m7_wf_sx_log2g_mxr_en       U     RW        default = 'h1
// Bit 15          rg_m7_wf_sx_log2g_div_en       U     RW        default = 'h1
// Bit 16          rg_m7_wf_sx_log2g_iqdiv_en     U     RW        default = 'h1
// Bit 17          rg_m7_wf_sx_log2g_rxlo_en      U     RW        default = 'h1
// Bit 18          rg_m7_wf_sx_log2g_dpdlo_en     U     RW        default = 'h0
// Bit 19          rg_m7_wf_sx_log2g_txlo_en      U     RW        default = 'h0
typedef union RG_SX_A49_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RG_M7_WF_SX_VCO_EN : 1;
    unsigned int RG_M7_WF_SX_VCO_ACAL_EN : 1;
    unsigned int RG_M7_WF_SX_PFD_EN : 1;
    unsigned int RG_M7_WF_SX_CP_EN : 1;
    unsigned int RG_M7_WF_SX_VCO_LDO_EN : 1;
    unsigned int RG_M7_WF_SX_CP_LDO_EN : 1;
    unsigned int RG_M7_WF_SX_LPF_COMP_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_MXR_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_INV_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_DIV_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_IQDIV_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_RXLO_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_DPDLO_EN : 1;
    unsigned int RG_M7_WF_SX_LOG5G_TXLO_EN : 1;
    unsigned int RG_M7_WF_SX_LOG2G_MXR_EN : 1;
    unsigned int RG_M7_WF_SX_LOG2G_DIV_EN : 1;
    unsigned int RG_M7_WF_SX_LOG2G_IQDIV_EN : 1;
    unsigned int RG_M7_WF_SX_LOG2G_RXLO_EN : 1;
    unsigned int RG_M7_WF_SX_LOG2G_DPDLO_EN : 1;
    unsigned int RG_M7_WF_SX_LOG2G_TXLO_EN : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_SX_A49_FIELD_T;

#define RG_SX_A50                                 (RF_D_SX_REG_BASE + 0xc8)
// Bit 6   :0      ro_wf_sx_double_tune           U     RO        default = 'h0
// Bit 8           ro_wf_sx_double_tune_ready     U     RO        default = 'h0
typedef union RG_SX_A50_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int RO_WF_SX_DOUBLE_TUNE : 7;
    unsigned int rsvd_0 : 1;
    unsigned int RO_WF_SX_DOUBLE_TUNE_READY : 1;
    unsigned int rsvd_1 : 23;
  } b;
} RG_SX_A50_FIELD_T;

#endif

