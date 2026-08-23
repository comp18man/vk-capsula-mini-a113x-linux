#ifdef CHIP_INTF_REG
#else
#define CHIP_INTF_REG


#define CHIP_INTF_REG_BASE                        (0xf00000)

#define RG_AON_A0                                 (CHIP_INTF_REG_BASE + 0x0)
// Bit 31  :0      rg_pin_mux0                    U     RW        default = 'h0
typedef union RG_AON_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux0 : 32;
  } b;
} RG_AON_A0_FIELD_T;

#define RG_AON_A1                                 (CHIP_INTF_REG_BASE + 0x4)
// Bit 31  :0      rg_pin_mux1                    U     RW        default = 'hffbf0f0
typedef union RG_AON_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux1 : 32;
  } b;
} RG_AON_A1_FIELD_T;

#define RG_AON_A2                                 (CHIP_INTF_REG_BASE + 0x8)
// Bit 31  :0      rg_pin_mux2                    U     RW        default = 'h40f0f
typedef union RG_AON_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux2 : 32;
  } b;
} RG_AON_A2_FIELD_T;

#define RG_AON_A3                                 (CHIP_INTF_REG_BASE + 0xc)
// Bit 31  :0      rg_pin_mux3                    U     RW        default = 'h0
typedef union RG_AON_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux3 : 32;
  } b;
} RG_AON_A3_FIELD_T;

#define RG_AON_A4                                 (CHIP_INTF_REG_BASE + 0x10)
// Bit 31  :0      rg_pin_mux4                    U     RW        default = 'h0
typedef union RG_AON_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux4 : 32;
  } b;
} RG_AON_A4_FIELD_T;

#define RG_AON_A5                                 (CHIP_INTF_REG_BASE + 0x14)
// Bit 31  :0      rg_pin_mux5                    U     RW        default = 'h0
typedef union RG_AON_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux5 : 32;
  } b;
} RG_AON_A5_FIELD_T;

#define RG_AON_A6                                 (CHIP_INTF_REG_BASE + 0x18)
// Bit 31  :0      rg_pin_mux6                    U     RW        default = 'h0
typedef union RG_AON_A6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux6 : 32;
  } b;
} RG_AON_A6_FIELD_T;

#define RG_AON_A7                                 (CHIP_INTF_REG_BASE + 0x1c)
// Bit 31  :0      rg_pin_mux7                    U     RW        default = 'h0
typedef union RG_AON_A7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pin_mux7 : 32;
  } b;
} RG_AON_A7_FIELD_T;

#define RG_AON_A8                                 (CHIP_INTF_REG_BASE + 0x20)
// Bit 0           rg_test_n_gpio_en_n            U     RW        default = 'h0
typedef union RG_AON_A8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_test_n_gpio_en_n : 1;
    unsigned int rsvd_0 : 31;
  } b;
} RG_AON_A8_FIELD_T;

#define RG_AON_A9                                 (CHIP_INTF_REG_BASE + 0x24)
// Bit 18  :0      rg_pad_gpioa_pull_up           U     RW        default = 'h18000
typedef union RG_AON_A9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pad_gpioA_pull_up : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_AON_A9_FIELD_T;

#define RG_AON_A10                                (CHIP_INTF_REG_BASE + 0x28)
// Bit 18  :0      rg_pad_gpioa_o                 U     RW        default = 'h7ffff
typedef union RG_AON_A10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pad_gpioA_o : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_AON_A10_FIELD_T;

#define RG_AON_A11                                (CHIP_INTF_REG_BASE + 0x2c)
// Bit 18  :0      rg_pad_gpioa_en_n              U     RW        default = 'h7ffff
typedef union RG_AON_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pad_gpioA_en_n : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_AON_A11_FIELD_T;

#define RG_AON_A12                                (CHIP_INTF_REG_BASE + 0x30)
// Bit 18  :0      ro_pad_gpioa_i                 U     RO        default = 'h0
typedef union RG_AON_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_pad_gpioA_i : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_AON_A12_FIELD_T;

#define RG_AON_A13                                (CHIP_INTF_REG_BASE + 0x34)
// Bit 18  :0      ro_pad_gpioa_pull_up           U     RO        default = 'h0
typedef union RG_AON_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_pad_gpioA_pull_up : 19;
    unsigned int rsvd_0 : 13;
  } b;
} RG_AON_A13_FIELD_T;

#define RG_AON_A14                                (CHIP_INTF_REG_BASE + 0x38)
// Bit 31  :0      rg_prod_tst_cfg0               U     RW        default = 'h24
typedef union RG_AON_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_prod_tst_cfg0 : 32;
  } b;
} RG_AON_A14_FIELD_T;

#define RG_AON_A15                                (CHIP_INTF_REG_BASE + 0x3c)
// Bit 31  :0      rg_prod_tst_cfg1               U     RW        default = 'h0
typedef union RG_AON_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_prod_tst_cfg1 : 32;
  } b;
} RG_AON_A15_FIELD_T;

#define RG_AON_A16                                (CHIP_INTF_REG_BASE + 0x40)
// Bit 31  :0      rg_wf_debug0                   U     RW        default = 'h0
typedef union RG_AON_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wf_debug0 : 32;
  } b;
} RG_AON_A16_FIELD_T;

#define RG_AON_A17                                (CHIP_INTF_REG_BASE + 0x44)
// Bit 31  :0      rg_wf_debug1                   U     RW        default = 'h0
typedef union RG_AON_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wf_debug1 : 32;
  } b;
} RG_AON_A17_FIELD_T;

#define RG_AON_A18                                (CHIP_INTF_REG_BASE + 0x48)
// Bit 31  :0      rg_mac_timer_cfg1              U     RW        default = 'h10a00
typedef union RG_AON_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_mac_timer_cfg1 : 32;
  } b;
} RG_AON_A18_FIELD_T;

#define RG_AON_A19                                (CHIP_INTF_REG_BASE + 0x4c)
// Bit 31  :0      rg_mac_time_low                U     RW        default = 'h0
typedef union RG_AON_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_mac_time_low : 32;
  } b;
} RG_AON_A19_FIELD_T;

#define RG_AON_A20                                (CHIP_INTF_REG_BASE + 0x50)
// Bit 31  :0      rg_mac_time_high               U     RW        default = 'h0
typedef union RG_AON_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_mac_time_high : 32;
  } b;
} RG_AON_A20_FIELD_T;

#define RG_AON_A21                                (CHIP_INTF_REG_BASE + 0x54)
// Bit 31  :0      rg_tsf_clk_cfg                 U     RW        default = 'h9
typedef union RG_AON_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_tsf_clk_cfg : 32;
  } b;
} RG_AON_A21_FIELD_T;

#define RG_AON_A22                                (CHIP_INTF_REG_BASE + 0x58)
// Bit 31  :0      ro_prod_tst_sts0               U     RO        default = 'h0
typedef union RG_AON_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_prod_tst_sts0 : 32;
  } b;
} RG_AON_A22_FIELD_T;

#define RG_AON_A23                                (CHIP_INTF_REG_BASE + 0x5c)
// Bit 31  :0      ro_prod_tst_sts1               U     RO        default = 'h0
typedef union RG_AON_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_prod_tst_sts1 : 32;
  } b;
} RG_AON_A23_FIELD_T;

#define RG_AON_A24                                (CHIP_INTF_REG_BASE + 0x60)
// Bit 31  :0      rg_wf_debug2                   U     RW        default = 'h0
typedef union RG_AON_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wf_debug2 : 32;
  } b;
} RG_AON_A24_FIELD_T;

#define RG_AON_A25                                (CHIP_INTF_REG_BASE + 0x64)
// Bit 31  :0      rg_wf_debug3                   U     RW        default = 'h0
typedef union RG_AON_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_wf_debug3 : 32;
  } b;
} RG_AON_A25_FIELD_T;

#define RG_AON_A29                                (CHIP_INTF_REG_BASE + 0x74)
// Bit 31  :0      rg_ana_bpll_cfg                U     RW        default = 'h0
typedef union RG_AON_A29_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_ana_bpll_cfg : 32;
  } b;
} RG_AON_A29_FIELD_T;

#define RG_AON_A30                                (CHIP_INTF_REG_BASE + 0x78)
// Bit 31  :0      rg_always_on_cfg4              U     RW        default = 'hc1b0
typedef union RG_AON_A30_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_always_on_cfg4 : 32;
  } b;
} RG_AON_A30_FIELD_T;

#define RG_AON_A31                                (CHIP_INTF_REG_BASE + 0x7c)
// Bit 31  :0      rg_always_on_cfg5              U     RW        default = 'hc1b0
typedef union RG_AON_A31_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_always_on_cfg5 : 32;
  } b;
} RG_AON_A31_FIELD_T;

#define RG_AON_A32                                (CHIP_INTF_REG_BASE + 0x80)
// Bit 31  :0      rg_mac_timer_cfg2              U     RW        default = 'h7d0001
typedef union RG_AON_A32_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_mac_timer_cfg2 : 32;
  } b;
} RG_AON_A32_FIELD_T;

#define RG_AON_A33                                (CHIP_INTF_REG_BASE + 0x84)
// Bit 31  :0      rg_mac_timer_cfg3              U     RW        default = 'h800100
typedef union RG_AON_A33_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_mac_timer_cfg3 : 32;
  } b;
} RG_AON_A33_FIELD_T;

#define RG_AON_A34                                (CHIP_INTF_REG_BASE + 0x88)
// Bit 31  :0      ro_always_on_sts1              U     RO        default = 'h0
typedef union RG_AON_A34_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_always_on_sts1 : 32;
  } b;
} RG_AON_A34_FIELD_T;

#define RG_AON_A35                                (CHIP_INTF_REG_BASE + 0x8c)
// Bit 31  :0      ro_always_on_sts2              U     RO        default = 'h0
typedef union RG_AON_A35_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_always_on_sts2 : 32;
  } b;
} RG_AON_A35_FIELD_T;

#define RG_AON_A36                                (CHIP_INTF_REG_BASE + 0x90)
// Bit 31  :0      ro_always_on_sts3              U     RO        default = 'h0
typedef union RG_AON_A36_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_always_on_sts3 : 32;
  } b;
} RG_AON_A36_FIELD_T;

#define RG_AON_A37                                (CHIP_INTF_REG_BASE + 0x94)
// Bit 31  :0      rg_intf_rsvd0                  U     RW        default = 'h0
typedef union RG_AON_A37_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_intf_rsvd0 : 32;
  } b;
} RG_AON_A37_FIELD_T;

#define RG_AON_A38                                (CHIP_INTF_REG_BASE + 0x98)
// Bit 31  :0      rg_pmu_power_dur_thr           U     RW        default = 'h0
typedef union RG_AON_A38_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_power_dur_thr : 32;
  } b;
} RG_AON_A38_FIELD_T;

#define RG_AON_A39                                (CHIP_INTF_REG_BASE + 0x9c)
// Bit 31  :0      rg_gated_ctrl                  U     RW        default = 'h2
typedef union RG_AON_A39_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_gated_ctrl : 32;
  } b;
} RG_AON_A39_FIELD_T;

#define RG_AON_A40                                (CHIP_INTF_REG_BASE + 0xa0)
// Bit 7   :0      rg_dig_timebase                U     RW        default = 'h28
typedef union RG_AON_A40_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dig_timebase : 8;
    unsigned int rsvd_0 : 24;
  } b;
} RG_AON_A40_FIELD_T;

#define RG_AON_A41                                (CHIP_INTF_REG_BASE + 0xa4)
// Bit 31  :0      ro_metal_rev                   U     RO        default = 'h0
typedef union RG_AON_A41_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_metal_rev : 32;
  } b;
} RG_AON_A41_FIELD_T;

#define RG_AON_A42                                (CHIP_INTF_REG_BASE + 0xa8)
// Bit 31  :0      rg_pmu_clk_cfg                 U     RW        default = 'h113
typedef union RG_AON_A42_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_pmu_clk_cfg : 32;
  } b;
} RG_AON_A42_FIELD_T;

#define RG_AON_A44                                (CHIP_INTF_REG_BASE + 0xb0)
// Bit 31  :0      ro_efuse_rd0                   U     RO        default = 'h0
typedef union RG_AON_A44_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd0 : 32;
  } b;
} RG_AON_A44_FIELD_T;

#define RG_AON_A45                                (CHIP_INTF_REG_BASE + 0xb4)
// Bit 31  :0      ro_efuse_rd1                   U     RO        default = 'h0
typedef union RG_AON_A45_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd1 : 32;
  } b;
} RG_AON_A45_FIELD_T;

#define RG_AON_A46                                (CHIP_INTF_REG_BASE + 0xb8)
// Bit 31  :0      ro_efuse_rd2                   U     RO        default = 'h0
typedef union RG_AON_A46_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd2 : 32;
  } b;
} RG_AON_A46_FIELD_T;

#define RG_AON_A47                                (CHIP_INTF_REG_BASE + 0xbc)
// Bit 31  :0      ro_efuse_rd3                   U     RO        default = 'h0
typedef union RG_AON_A47_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd3 : 32;
  } b;
} RG_AON_A47_FIELD_T;

#define RG_AON_A48                                (CHIP_INTF_REG_BASE + 0xc0)
// Bit 31  :0      ro_efuse_rd4                   U     RO        default = 'h0
typedef union RG_AON_A48_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd4 : 32;
  } b;
} RG_AON_A48_FIELD_T;

#define RG_AON_A49                                (CHIP_INTF_REG_BASE + 0xc4)
// Bit 31  :0      ro_efuse_rd5                   U     RO        default = 'h0
typedef union RG_AON_A49_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd5 : 32;
  } b;
} RG_AON_A49_FIELD_T;

#define RG_AON_A50                                (CHIP_INTF_REG_BASE + 0xc8)
// Bit 31  :0      ro_efuse_rd6                   U     RO        default = 'h0
typedef union RG_AON_A50_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd6 : 32;
  } b;
} RG_AON_A50_FIELD_T;

#define RG_AON_A51                                (CHIP_INTF_REG_BASE + 0xcc)
// Bit 31  :0      ro_efuse_rd7                   U     RO        default = 'h0
typedef union RG_AON_A51_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_efuse_rd7 : 32;
  } b;
} RG_AON_A51_FIELD_T;

#endif

