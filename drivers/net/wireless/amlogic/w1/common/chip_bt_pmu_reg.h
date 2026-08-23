#ifdef CHIP_BT_PMU_REG
#else
#define CHIP_BT_PMU_REG


#define CHIP_BT_PMU_REG_BASE                      (0xf03000)

#define RG_BT_PMU_A0                              (CHIP_BT_PMU_REG_BASE + 0x0)
// Bit 15  :0      rg_pmu_pwr_xosc_bnd            U     RW        default = 'h6
typedef union RG_BT_PMU_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_pwr_xosc_bnd : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A0_FIELD_T;

#define RG_BT_PMU_A1                              (CHIP_BT_PMU_REG_BASE + 0x4)
// Bit 30  :0      rg_pmu_xosc_wait_bnd           U     RW        default = 'h80
// Bit 31          rg_pmu_xosc_wait_man           U     RW        default = 'h1
typedef union RG_BT_PMU_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_xosc_wait_bnd : 31;
    unsigned int rg_pmu_xosc_wait_man : 1;
  } b;
} RG_BT_PMU_A1_FIELD_T;

#define RG_BT_PMU_A2                              (CHIP_BT_PMU_REG_BASE + 0x8)
// Bit 15  :0      rg_pmu_xosc_dpll_bnd           U     RW        default = 'h6
typedef union RG_BT_PMU_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_xosc_dpll_bnd : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A2_FIELD_T;

#define RG_BT_PMU_A3                              (CHIP_BT_PMU_REG_BASE + 0xc)
// Bit 30  :0      rg_pmu_dpll_wait_bnd           U     RW        default = 'h10
// Bit 31          rg_pmu_dpll_wait_man           U     RW        default = 'h1
typedef union RG_BT_PMU_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_dpll_wait_bnd : 31;
    unsigned int rg_pmu_dpll_wait_man : 1;
  } b;
} RG_BT_PMU_A3_FIELD_T;

#define RG_BT_PMU_A4                              (CHIP_BT_PMU_REG_BASE + 0x10)
// Bit 15  :0      rg_pmu_dpll_act_bnd            U     RW        default = 'h6
typedef union RG_BT_PMU_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_dpll_act_bnd : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A4_FIELD_T;

#define RG_BT_PMU_A5                              (CHIP_BT_PMU_REG_BASE + 0x14)
// Bit 30  :0      rg_pmu_act_mode_bnd            U     RW        default = 'h20
// Bit 31          rg_pmu_act_mode_man            U     RW        default = 'h0
typedef union RG_BT_PMU_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_act_mode_bnd : 31;
    unsigned int rg_pmu_act_mode_man : 1;
  } b;
} RG_BT_PMU_A5_FIELD_T;

#define RG_BT_PMU_A6                              (CHIP_BT_PMU_REG_BASE + 0x18)
// Bit 15  :0      rg_pmu_act_sleep_bnd           U     RW        default = 'h6
typedef union RG_BT_PMU_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_act_sleep_bnd : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A6_FIELD_T;

#define RG_BT_PMU_A7                              (CHIP_BT_PMU_REG_BASE + 0x1c)
// Bit 30  :0      rg_pmu_sleep_mode_bnd          U     RW        default = 'h0
// Bit 31          rg_pmu_sleep_mode_man          U     RW        default = 'h0
typedef union RG_BT_PMU_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_sleep_mode_bnd : 31;
    unsigned int rg_pmu_sleep_mode_man : 1;
  } b;
} RG_BT_PMU_A7_FIELD_T;

#define RG_BT_PMU_A8                              (CHIP_BT_PMU_REG_BASE + 0x20)
// Bit 15  :0      rg_pmu_sleep_wake_bnd          U     RW        default = 'h6
typedef union RG_BT_PMU_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_sleep_wake_bnd : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A8_FIELD_T;

#define RG_BT_PMU_A9                              (CHIP_BT_PMU_REG_BASE + 0x24)
// Bit 30  :0      rg_pmu_wake_wait_bnd           U     RW        default = 'h20
// Bit 31          rg_pmu_wake_wait_man           U     RW        default = 'h1
typedef union RG_BT_PMU_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_wake_wait_bnd : 31;
    unsigned int rg_pmu_wake_wait_man : 1;
  } b;
} RG_BT_PMU_A9_FIELD_T;

#define RG_BT_PMU_A10                             (CHIP_BT_PMU_REG_BASE + 0x28)
// Bit 30  :0      rg_pmu_wake_xosc_bnd           U     RW        default = 'h6
// Bit 31          rg_pmu_wake_xosc_man           U     RW        default = 'h1
typedef union RG_BT_PMU_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_wake_xosc_bnd : 31;
    unsigned int rg_pmu_wake_xosc_man : 1;
  } b;
} RG_BT_PMU_A10_FIELD_T;

#define RG_BT_PMU_A11                             (CHIP_BT_PMU_REG_BASE + 0x2c)
// Bit 30  :0      rg_pmu_start_cpu_bnd           U     RW        default = 'h10
// Bit 31          rg_pmu_start_cpu_man           U     RW        default = 'h0
typedef union RG_BT_PMU_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_start_cpu_bnd : 31;
    unsigned int rg_pmu_start_cpu_man : 1;
  } b;
} RG_BT_PMU_A11_FIELD_T;

#define RG_BT_PMU_A12                             (CHIP_BT_PMU_REG_BASE + 0x30)
// Bit 0           rg_rtc_mode_val                U     RW        default = 'h0
// Bit 1           rg_rtc_mode_man                U     RW        default = 'h0
// Bit 2           rg_osc_mode_val                U     RW        default = 'h1
// Bit 3           rg_osc_mode_man                U     RW        default = 'h1
// Bit 4           rg_gated_mode_val              U     RW        default = 'h0
// Bit 5           rg_gated_mode_man              U     RW        default = 'h1
// Bit 6           rg_bb_reset_val                U     RW        default = 'h0
// Bit 7           rg_bb_reset_man                U     RW        default = 'h0
// Bit 8           rg_iso_en_m_val                U     RW        default = 'h0
// Bit 9           rg_iso_en_m_man                U     RW        default = 'h1
// Bit 10          rg_wake_int_val                U     RW        default = 'h0
// Bit 11          rg_wake_int_man                U     RW        default = 'h1
// Bit 12          rg_start_cpu_val               U     RW        default = 'h0
// Bit 13          rg_start_cpu_man               U     RW        default = 'h1
// Bit 14          rg_ana_osc_val                 U     RW        default = 'h0
// Bit 15          rg_ana_osc_man                 U     RW        default = 'h0
// Bit 16          rg_ana_dpll_val                U     RW        default = 'h0
// Bit 17          rg_ana_dpll_man                U     RW        default = 'h0
// Bit 18          rg_efuse_read_val              U     RW        default = 'h0
// Bit 19          rg_efuse_read_man              U     RW        default = 'h0
typedef union RG_BT_PMU_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rtc_mode_val : 1;
    unsigned int rg_rtc_mode_man : 1;
    unsigned int rg_osc_mode_val : 1;
    unsigned int rg_osc_mode_man : 1;
    unsigned int rg_gated_mode_val : 1;
    unsigned int rg_gated_mode_man : 1;
    unsigned int rg_bb_reset_val : 1;
    unsigned int rg_bb_reset_man : 1;
    unsigned int rg_iso_en_m_val : 1;
    unsigned int rg_iso_en_m_man : 1;
    unsigned int rg_wake_int_val : 1;
    unsigned int rg_wake_int_man : 1;
    unsigned int rg_start_cpu_val : 1;
    unsigned int rg_start_cpu_man : 1;
    unsigned int rg_ana_osc_val : 1;
    unsigned int rg_ana_osc_man : 1;
    unsigned int rg_ana_dpll_val : 1;
    unsigned int rg_ana_dpll_man : 1;
    unsigned int rg_efuse_read_val : 1;
    unsigned int rg_efuse_read_man : 1;
    unsigned int rsvd_0 : 12;
  } b;
} RG_BT_PMU_A12_FIELD_T;

#define RG_BT_PMU_A13                             (CHIP_BT_PMU_REG_BASE + 0x34)
// Bit 15  :0      rg_pmu_power_en_th             U     RW        default = 'h6
// Bit 30          rg_pmu_power_en_val            U     RW        default = 'h0
// Bit 31          rg_pmu_power_en_man            U     RW        default = 'h1
typedef union RG_BT_PMU_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_power_en_th : 16;
    unsigned int rsvd_0 : 14;
    unsigned int rg_pmu_power_en_val : 1;
    unsigned int rg_pmu_power_en_man : 1;
  } b;
} RG_BT_PMU_A13_FIELD_T;

#define RG_BT_PMU_A14                             (CHIP_BT_PMU_REG_BASE + 0x38)
// Bit 0           rg_pmu_work_flg                U     RW        default = 'h1
// Bit 1           rg_pmu_ana_xosc_msk            U     RW        default = 'h0
// Bit 2           rg_pmu_ana_dpll_msk            U     RW        default = 'h0
// Bit 3           rg_pmu_power_en_msk            U     RW        default = 'h0
// Bit 4           rg_pmu_aon_iso_en_msk          U     RW        default = 'h0
typedef union RG_BT_PMU_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_work_flg : 1;
    unsigned int rg_pmu_ana_xosc_msk : 1;
    unsigned int rg_pmu_ana_dpll_msk : 1;
    unsigned int rg_pmu_power_en_msk : 1;
    unsigned int rg_pmu_aon_iso_en_msk : 1;
    unsigned int rsvd_0 : 27;
  } b;
} RG_BT_PMU_A14_FIELD_T;

#define RG_BT_PMU_A15                             (CHIP_BT_PMU_REG_BASE + 0x3c)
// Bit 3   :0      ro_pmu_cur_obs                 U     RO        default = 'h0
typedef union RG_BT_PMU_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_pmu_cur_obs : 4;
    unsigned int rsvd_0 : 28;
  } b;
} RG_BT_PMU_A15_FIELD_T;

#define RG_BT_PMU_A16                             (CHIP_BT_PMU_REG_BASE + 0x40)
// Bit 31  :0      rg_pmu_power_cfg               U     RW        default = 'h0
typedef union RG_BT_PMU_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_power_cfg : 32;
  } b;
} RG_BT_PMU_A16_FIELD_T;

#define RG_BT_PMU_A17                             (CHIP_BT_PMU_REG_BASE + 0x44)
// Bit 7   :0      rg_pwr_off_sw                  U     RW        default = 'h0
// Bit 15  :8      rg_pwr_off_hw                  U     RW        default = 'h7
typedef union RG_BT_PMU_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pwr_off_sw : 8;
    unsigned int rg_pwr_off_hw : 8;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A17_FIELD_T;

#define RG_BT_PMU_A18                             (CHIP_BT_PMU_REG_BASE + 0x48)
// Bit 7   :0      rg_iso_en_sw                   U     RW        default = 'h0
// Bit 15  :8      rg_iso_en_hw                   U     RW        default = 'h17
typedef union RG_BT_PMU_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_iso_en_sw : 8;
    unsigned int rg_iso_en_hw : 8;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A18_FIELD_T;

#define RG_BT_PMU_A19                             (CHIP_BT_PMU_REG_BASE + 0x4c)
// Bit 7   :0      ro_pwrup_ack                   U     RO        default = 'h0
typedef union RG_BT_PMU_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_pwrup_ack : 8;
    unsigned int rsvd_0 : 24;
  } b;
} RG_BT_PMU_A19_FIELD_T;

#define RG_BT_PMU_A20                             (CHIP_BT_PMU_REG_BASE + 0x50)
// Bit 31  :0      rg_ram_pd_shutdown_sw          U     RW        default = 'h1f000000
typedef union RG_BT_PMU_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_ram_pd_shutdown_sw : 32;
  } b;
} RG_BT_PMU_A20_FIELD_T;

#define RG_BT_PMU_A21                             (CHIP_BT_PMU_REG_BASE + 0x54)
// Bit 31  :0      rg_ram_pd_disb_gate_sw         U     RW        default = 'h0
typedef union RG_BT_PMU_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_ram_pd_disb_gate_sw : 32;
  } b;
} RG_BT_PMU_A21_FIELD_T;

#define RG_BT_PMU_A22                             (CHIP_BT_PMU_REG_BASE + 0x58)
// Bit 7   :0      rg_dev_reset_sw                U     RW        default = 'h4
// Bit 15  :8      rg_dev_reset_hw                U     RW        default = 'h7
typedef union RG_BT_PMU_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dev_reset_sw : 8;
    unsigned int rg_dev_reset_hw : 8;
    unsigned int rsvd_0 : 16;
  } b;
} RG_BT_PMU_A22_FIELD_T;

#define RG_BT_PMU_A23                             (CHIP_BT_PMU_REG_BASE + 0x5c)
// Bit 7   :0      ro_pmu_sdio_cfg                U     RO        default = 'h0
typedef union RG_BT_PMU_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_pmu_sdio_cfg : 8;
    unsigned int rsvd_0 : 24;
  } b;
} RG_BT_PMU_A23_FIELD_T;

#define RG_BT_PMU_A24                             (CHIP_BT_PMU_REG_BASE + 0x60)
// Bit 31  :0      rg_ram_pd_shutdown_hw          U     RW        default = 'h1f000000
typedef union RG_BT_PMU_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_ram_pd_shutdown_hw : 32;
  } b;
} RG_BT_PMU_A24_FIELD_T;

#define RG_BT_PMU_A25                             (CHIP_BT_PMU_REG_BASE + 0x64)
// Bit 31  :0      rg_ram_pd_disb_gate_hw         U     RW        default = 'h0
typedef union RG_BT_PMU_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_ram_pd_disb_gate_hw : 32;
  } b;
} RG_BT_PMU_A25_FIELD_T;

#define RG_BT_PMU_A26                             (CHIP_BT_PMU_REG_BASE + 0x68)
// Bit 7   :0      rg_mem_pd_en_th                U     RW        default = 'h6
// Bit 31          rg_mem_pd_en                   U     RW        default = 'h1
typedef union RG_BT_PMU_A26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_mem_pd_en_th : 8;
    unsigned int rsvd_0 : 23;
    unsigned int rg_mem_pd_en : 1;
  } b;
} RG_BT_PMU_A26_FIELD_T;

#define RG_BT_PMU_A27                             (CHIP_BT_PMU_REG_BASE + 0x6c)
// Bit 30  :0      rg_pmu_rdy_wait_bnd            U     RW        default = 'h0
// Bit 31          rg_pmu_rdy_wait_man            U     RW        default = 'h0
typedef union RG_BT_PMU_A27_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_rdy_wait_bnd : 31;
    unsigned int rg_pmu_rdy_wait_man : 1;
  } b;
} RG_BT_PMU_A27_FIELD_T;

#endif

