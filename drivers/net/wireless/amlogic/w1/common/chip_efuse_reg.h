#ifdef CHIP_EFUSE_REG
#else
#define CHIP_EFUSE_REG


#define CHIP_EFUSE_REG_BASE                       (0xf04000)

#define EFUSE_CNTL0                               (CHIP_EFUSE_REG_BASE + 0x0)
// Bit 7   :0      tclock_high                    U     RW        default = 'h87
// Bit 15  :8      tclock_low                     U     RW        default = 'h27
// Bit 23  :16     tps_delay                      U     RW        default = 'ha
// Bit 31  :24     tunit_delay                    U     RW        default = 'h2
typedef union EFUSE_CNTL0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int Tclock_high : 8;
    unsigned int Tclock_low : 8;
    unsigned int Tps_delay : 8;
    unsigned int Tunit_delay : 8;
  } b;
} EFUSE_CNTL0_FIELD_T;

#define EFUSE_CNTL1                               (CHIP_EFUSE_REG_BASE + 0x4)
// Bit 9   :0      byte_addr                      U     RW        default = 'h0
// Bit 11          byte_addr_set                  U     RW        default = 'h0
// Bit 12          auto_wr_enable                 U     RW        default = 'h0
// Bit 13          auto_wr_start                  U     RW        default = 'h0
// Bit 14          auto_wr_busy                   U     RO        default = 'h0
// Bit 23  :16     byte_wdata                     U     RW        default = 'h0
// Bit 24          auto_rd_enable                 U     RW        default = 'h0
// Bit 25          auto_rd_start                  U     RW        default = 'h0
// Bit 26          auto_rd_busy                   U     RO        default = 'h0
// Bit 27          ctrl_pd                        U     RW        default = 'h1
// Bit 31          sim_mask_rd_dat                U     RW        default = 'h0
typedef union EFUSE_CNTL1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int byte_addr : 10;
    unsigned int rsvd_0 : 1;
    unsigned int byte_addr_set : 1;
    unsigned int auto_wr_enable : 1;
    unsigned int auto_wr_start : 1;
    unsigned int auto_wr_busy : 1;
    unsigned int rsvd_1 : 1;
    unsigned int byte_wdata : 8;
    unsigned int auto_rd_enable : 1;
    unsigned int auto_rd_start : 1;
    unsigned int auto_rd_busy : 1;
    unsigned int ctrl_PD : 1;
    unsigned int rsvd_2 : 3;
    unsigned int sim_mask_rd_dat : 1;
  } b;
} EFUSE_CNTL1_FIELD_T;

#define EFUSE_CNTL2                               (CHIP_EFUSE_REG_BASE + 0x8)
// Bit 31  :0      auto_rd_rdata                  U     RO        default = 'h0
typedef union EFUSE_CNTL2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int auto_rd_rdata : 32;
  } b;
} EFUSE_CNTL2_FIELD_T;

#define EFUSE_CNTL3                               (CHIP_EFUSE_REG_BASE + 0xc)
// Bit 12  :0      manual_d_addr                  U     RW        default = 'h0
// Bit 13          use_gen_strobe                 U     RW        default = 'h0
// Bit 14          gen_strobe_clk                 U     RW        default = 'h0
// Bit 15          gen_strobe                     U     RO        default = 'h0
// Bit 23  :16     manual_rdata                   U     RO        default = 'h0
// Bit 26  :24     man_chip_sel                   U     RW        default = 'h0
// Bit 27          manual_ps                      U     RW        default = 'h0
// Bit 28          manual_strobe                  U     RW        default = 'h0
// Bit 29          manual_load                    U     RW        default = 'h1
// Bit 30          manual_pgenb                   U     RW        default = 'h1
// Bit 31          manual_csb                     U     RW        default = 'h1
typedef union EFUSE_CNTL3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int manual_D_ADDR : 13;
    unsigned int use_gen_STROBE : 1;
    unsigned int gen_strobe_clk : 1;
    unsigned int gen_STROBE : 1;
    unsigned int manual_rdata : 8;
    unsigned int man_chip_sel : 3;
    unsigned int manual_PS : 1;
    unsigned int manual_STROBE : 1;
    unsigned int manual_LOAD : 1;
    unsigned int manual_PGENB : 1;
    unsigned int manual_CSB : 1;
  } b;
} EFUSE_CNTL3_FIELD_T;

#define EFUSE_CNTL4                               (CHIP_EFUSE_REG_BASE + 0x10)
// Bit 7   :0      encrypt_seed                   U     RW        default = 'hbe
// Bit 8           encrypt_fixed                  U     RW        default = 'h1
// Bit 9           encrypt_set_seed               U     RW        default = 'h0
// Bit 10          encrypt_en                     U     RW        default = 'h1
// Bit 23  :16     trd_clock_high                 U     RW        default = 'ha
// Bit 31  :24     trd_clock_low                  U     RW        default = 'ha
typedef union EFUSE_CNTL4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int encrypt_seed : 8;
    unsigned int encrypt_fixed : 1;
    unsigned int encrypt_set_seed : 1;
    unsigned int encrypt_en : 1;
    unsigned int rsvd_0 : 5;
    unsigned int Trd_clock_high : 8;
    unsigned int Trd_clock_low : 8;
  } b;
} EFUSE_CNTL4_FIELD_T;

#define EFUSE_CNTL5                               (CHIP_EFUSE_REG_BASE + 0x14)
// Bit 2   :0      rd_strobe_time                 U     RW        default = 'h2
typedef union EFUSE_CNTL5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rd_strobe_time : 3;
    unsigned int rsvd_0 : 29;
  } b;
} EFUSE_CNTL5_FIELD_T;

#endif

