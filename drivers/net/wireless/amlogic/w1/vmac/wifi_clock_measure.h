#ifndef __CLOCK_MEASURE__
#define __CLOCK_MEASURE__
//clock mux
typedef enum clock_mux {

    CTS_CLK_11BRX_POS      = 0,
    CTS_CLK_ADDA_320M_POS  = 1,
    CTS_CLK_VIT_POS        = 2,
    CTS_CLK_ADDA_80M_POS   = 3,
    CTS_CLK_RADC_320M_POS  = 4,
    CTS_CLK_ADDA_160M_POS  = 5,
    CTS_CLK_XDAC_640M_POS  = 6,
    CTS_CLK_MAC_POS        = 7,
    CTS_CLK_APB_POS        = 8,
    CTS_CLK_CPU_POS        = 9,
    CTS_CLK_ADDA_640M_POS  = 10,
    XJTAG_TCK_I_POS        = 11,
    CTS_CLK_FEC_POS        = 12,
    SYS_OSCIN_I_POS        = 13,
    WF_DIG_OUT_POS         = 14,

    CLOCK_MUX_MAX          = WF_DIG_OUT_POS + 1,
} CLOCK_MUX;

#define  SYS_OSCIN_I          40000000 
//#define  VIRTUAL_CLK_500M     500000000 
#define  CTS_RTC_CLK          32000 
#define  CTS_CLK_CPU          40000000 
#define  WF_ADC_CLK_320M      320000000 
#define  BT_BBP_CLK_240M      240000000 
#define  CTS_MSR_CLK          6000000 
#define  WF_BBP_CLK_640M      640000000 
#define  WF_BBP_CLK_320M      320000000 
#define  WF_BBP_CLK_240M      240000000 
#define  WF_DIG_OUT           1250000

//reg addr
#define INTF_BASE_ADDR     0xa0d000

#define DF_REG_A18  (INTF_BASE_ADDR +0x048)
#define DF_REG_A19  (INTF_BASE_ADDR +0x04c)
#define DF_REG_A20  (INTF_BASE_ADDR +0x050)
#define DF_REG_A21  (INTF_BASE_ADDR +0x054)
#define DF_REG_A22  (INTF_BASE_ADDR +0x058)
#define DF_REG_A23  (INTF_BASE_ADDR +0x05c)

#define DF_REG_A36  (INTF_BASE_ADDR +0x090)
#define DF_REG_A38  (INTF_BASE_ADDR +0x098)
#define DF_REG_A40  (INTF_BASE_ADDR +0x0a0)
#define DF_REG_A41  (INTF_BASE_ADDR +0x0a4)
#define DF_REG_A49  (INTF_BASE_ADDR +0x0c4)

#define CPU_CLOCK_RTC (0x4f070033)
#define CPU_CLOCK_DPLL (0x4f0700f3)
#define CPU_CLOCK_XOSC (0x4f070001)

void host_clk_msr(CLOCK_MUX clk_mux);

#endif

