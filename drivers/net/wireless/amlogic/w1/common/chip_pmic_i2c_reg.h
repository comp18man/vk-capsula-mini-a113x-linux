#ifdef CHIP_PMIC_I2C_REG
#else
#define CHIP_PMIC_I2C_REG


#define CHIP_PMIC_I2C_REG_BASE                    (0xf05000)

#define PMU_I2C_M_CONTROL_REG                     (CHIP_PMIC_I2C_REG_BASE + 0x880)
// Bit 0           cntl_start_bit                 U     RW        default = 'h0
// Bit 1           cntl_ack_ignore                U     RW        default = 'h0
// Bit 2           busy                           U     RO        default = 'h0
// Bit 3           error                          U     RO        default = 'h0
// Bit 7   :4      list_index                     U     RO        default = 'h0
// Bit 11  :8      rdata                          U     RO        default = 'h0
// Bit 21  :12     cntl_qtr_clk_dly               U     RW        default = 'h14c
// Bit 22          cntl_manual_i2c                U     RW        default = 'h0
// Bit 23          cntl_scl_level_o               U     RW        default = 'h1
// Bit 24          cntl_sda_level_o               U     RW        default = 'h1
// Bit 25          scl_i                          U     RO        default = 'h0
// Bit 26          sda_i                          U     RO        default = 'h0
// Bit 29  :28     cnt1_qtr_clk_dly               U     RW        default = 'h0
// Bit 31          cnt1_jic                       U     RW        default = 'h0
typedef union PMU_I2C_M_CONTROL_REG_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int cntl_start_bit : 1;
    unsigned int cntl_ack_ignore : 1;
    unsigned int busy : 1;
    unsigned int error : 1;
    unsigned int list_index : 4;
    unsigned int rdata : 4;
    unsigned int cntl_qtr_clk_dly : 10;
    unsigned int cntl_manual_i2c : 1;
    unsigned int cntl_scl_level_o : 1;
    unsigned int cntl_sda_level_o : 1;
    unsigned int scl_i : 1;
    unsigned int sda_i : 1;
    unsigned int rsvd_0 : 1;
    unsigned int cnt1_qtr_clk_dly : 2;
    unsigned int rsvd_1 : 1;
    unsigned int cnt1_jic : 1;
  } b;
} PMU_I2C_M_CONTROL_REG_FIELD_T;

#define PMU_I2C_M_SLAVE_ADDR                      (CHIP_PMIC_I2C_REG_BASE + 0x884)
// Bit 7   :0      slave_addr                     U     RW        default = 'h0
// Bit 10  :8      sda_filter_sel                 U     RW        default = 'h2
// Bit 13  :11     scl_filter_sel                 U     RW        default = 'h2
// Bit 27  :16     cntl_scl_low                   U     RW        default = 'h14c
// Bit 28          use_cntl_scl_low               U     RW        default = 'h0
typedef union PMU_I2C_M_SLAVE_ADDR_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int slave_addr : 8;
    unsigned int sda_filter_sel : 3;
    unsigned int scl_filter_sel : 3;
    unsigned int rsvd_0 : 2;
    unsigned int cntl_scl_low : 12;
    unsigned int use_cntl_scl_low : 1;
    unsigned int rsvd_1 : 3;
  } b;
} PMU_I2C_M_SLAVE_ADDR_FIELD_T;

#define PMU_I2C_M_TOKEN_LIST0                     (CHIP_PMIC_I2C_REG_BASE + 0x888)
// Bit 31  :0      i2c_m_token_list0              U     RW        default = 'h0
typedef union PMU_I2C_M_TOKEN_LIST0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int i2c_m_token_list0 : 32;
  } b;
} PMU_I2C_M_TOKEN_LIST0_FIELD_T;

#define PMU_I2C_M_TOKEN_LIST1                     (CHIP_PMIC_I2C_REG_BASE + 0x88c)
// Bit 31  :0      i2c_m_token_list1              U     RW        default = 'h0
typedef union PMU_I2C_M_TOKEN_LIST1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int i2c_m_token_list1 : 32;
  } b;
} PMU_I2C_M_TOKEN_LIST1_FIELD_T;

#define PMU_I2C_M_WDATA_REG0                      (CHIP_PMIC_I2C_REG_BASE + 0x890)
// Bit 31  :0      i2c_m_wdata_reg0               U     RW        default = 'h0
typedef union PMU_I2C_M_WDATA_REG0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int i2c_m_wdata_reg0 : 32;
  } b;
} PMU_I2C_M_WDATA_REG0_FIELD_T;

#define PMU_I2C_M_WDATA_REG1                      (CHIP_PMIC_I2C_REG_BASE + 0x894)
// Bit 31  :0      i2c_m_wdata_reg1               U     RW        default = 'h0
typedef union PMU_I2C_M_WDATA_REG1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int i2c_m_wdata_reg1 : 32;
  } b;
} PMU_I2C_M_WDATA_REG1_FIELD_T;

#define PMU_I2C_M_RDATA_REG0                      (CHIP_PMIC_I2C_REG_BASE + 0x898)
// Bit 31  :0      i2c_m_rdata_reg0               U     RO        default = 'h0
typedef union PMU_I2C_M_RDATA_REG0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int i2c_m_rdata_reg0 : 32;
  } b;
} PMU_I2C_M_RDATA_REG0_FIELD_T;

#define PMU_I2C_M_RDATA_REG1                      (CHIP_PMIC_I2C_REG_BASE + 0x89c)
// Bit 31  :0      i2c_m_rdata_reg1               U     RO        default = 'h0
typedef union PMU_I2C_M_RDATA_REG1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int i2c_m_rdata_reg1 : 32;
  } b;
} PMU_I2C_M_RDATA_REG1_FIELD_T;

#endif

