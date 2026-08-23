#include "wifi_clock_measure.h"
#include "wifi_hal_com.h"
#include "wifi_hif.h"
#include "wifi_mac_com.h"

static unsigned int clk_msr_time_set( unsigned int msr_freq)
{
   unsigned int us_gate_time;

   if (msr_freq / 1000000 >= 160) {
      us_gate_time = 1;
   } else {
       // if < 10Mhz, measure num is set about 320
      us_gate_time = 160 * 1000000 / msr_freq + 1;
   }
   return us_gate_time;
}

// test the clock measurement
static unsigned int clk_msr_calc(unsigned int clk_mux, unsigned int exp_freq)
{
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int measured_val;
    unsigned int measured_feq;
    unsigned int delt;

    unsigned int us_gate_time = clk_msr_time_set(exp_freq);

    hif->hif_ops.hi_write_word(DF_REG_A18, (hif->hif_ops.hi_read_word(DF_REG_A18) & ~(0xFFFF << 0)) | ((us_gate_time-1) << 0));
    hif->hif_ops.hi_write_word(DF_REG_A18, (hif->hif_ops.hi_read_word(DF_REG_A18) & ~((1 << 18) | (1 << 17))));
    hif->hif_ops.hi_write_word(DF_REG_A18, (hif->hif_ops.hi_read_word(DF_REG_A18) & ~(0xff << 20))
                                                        | ((clk_mux << 20) // Select MUX
                                                        | (1 << 19)        //clk_to_msr_en
                                                        ));

    // enable measuring
    hif->hif_ops.hi_write_word(DF_REG_A18, (hif->hif_ops.hi_read_word(DF_REG_A18)  | (1 << 16)) );

    // Wait for the measurement to be done
    while ((hif->hif_ops.hi_read_word(DF_REG_A18) & (1 << 31))) {}

    // disable measuring
    hif->hif_ops.hi_write_word(DF_REG_A18, (hif->hif_ops.hi_read_word(DF_REG_A18) & ~(1 << 16)) | (0 << 16));

    // only 20 bits //the count of measured clk at us_gate_time
    measured_val = (hif->hif_ops.hi_read_word(DF_REG_A20) & 0x000FFFFF);
    measured_feq = (measured_val * 1000000) / (us_gate_time);
    delt = exp_freq * 4 /100;

    pr_debug("exp_freq = %d(MHz)\n", exp_freq / 1000000);
    pr_debug("measured_feq = %d(MHz)\n", measured_feq / 1000000);

    if (measured_val == 0xFFFFF) {
        pr_warn("measured_val is 20 bits max\n");
        return 0;
    } else if ((measured_feq < (exp_freq - delt)) || (measured_feq > (exp_freq + delt))) {
        pr_err("clock measure failed, clock id : %d\n", clk_mux);
        return 0;
    } else {
        return 1;
    }
}

static void clk_msr_adda(CLOCK_MUX clk_mux)
{
    unsigned int exp_val;

    unsigned int div = 1;
    unsigned int div1= 2;
    unsigned int div2= 2;
    unsigned int div3= 2;
    unsigned int div4= 1;

    switch (clk_mux) {
        case CTS_CLK_ADDA_640M_POS:
            exp_val = WF_BBP_CLK_640M / div;
            break;

        case CTS_CLK_ADDA_320M_POS:
            exp_val = WF_BBP_CLK_640M / div1;
            break;

        case CTS_CLK_ADDA_160M_POS:
            exp_val = WF_BBP_CLK_640M / (div2 * 2);
            break;

        case CTS_CLK_ADDA_80M_POS:
            exp_val = WF_BBP_CLK_640M / (div3 * 4);
            break;

        case CTS_CLK_XDAC_640M_POS:
            exp_val = WF_BBP_CLK_640M / div4;
            break;

        case CTS_CLK_FEC_POS:
            exp_val = WF_BBP_CLK_640M / div1;
            break;
        default:
            return;
    }

    clk_msr_calc(clk_mux, exp_val);
}

static void clk_msr_cpu(void)
{
   unsigned int exp_val;

   exp_val = WF_BBP_CLK_640M / 8;
   clk_msr_calc(CTS_CLK_CPU_POS, exp_val);
}

static void clk_msr_apb(void)
{
    unsigned int exp_val;

    exp_val = WF_BBP_CLK_640M/8;
    clk_msr_calc(CTS_CLK_APB_POS, exp_val);
}

static void clk_msr_other(CLOCK_MUX clk_mux)
{
    unsigned int exp_val;

    switch (clk_mux) {
        case CTS_CLK_RADC_320M_POS:
            exp_val = WF_ADC_CLK_320M;
            break;
        case CTS_CLK_VIT_POS:
            pr_debug("not measure now\n");
            exp_val = BT_BBP_CLK_240M/2;
            return;
        case CTS_CLK_11BRX_POS:
            exp_val = BT_BBP_CLK_240M/5;
            break;
        default:
            return;
    }

    clk_msr_calc(clk_mux, exp_val);
}

static void clk_msr_mac(void)
{
    unsigned int exp_val;

    exp_val = WF_BBP_CLK_640M / 8;
    clk_msr_calc(CTS_CLK_MAC_POS, exp_val);
}

static void clk_msr_xjtag(void)
{
    unsigned int exp_val;

    exp_val= CTS_RTC_CLK;
    clk_msr_calc(XJTAG_TCK_I_POS, exp_val);
}

static void clk_msr_oscin(void)
{
    unsigned int exp_val;

    exp_val= SYS_OSCIN_I;
    clk_msr_calc(SYS_OSCIN_I_POS, exp_val);
}

static void clk_msr_wfdig(void)
{
    unsigned int exp_val;

    exp_val= WF_DIG_OUT;
    clk_msr_calc(WF_DIG_OUT_POS, exp_val);
}

static unsigned char * clk_mux_name(CLOCK_MUX clk_mux)
{
    unsigned char *clk_name;

    switch (clk_mux) {
        case CTS_CLK_11BRX_POS      : clk_name = "    cts_clk_11brx";        break;
        case CTS_CLK_VIT_POS        : clk_name = "    cts_clk_vit";          break;
        case CTS_CLK_RADC_320M_POS  : clk_name = "    cts_clk_radc_320M";    break;

        case CTS_CLK_ADDA_320M_POS  : clk_name = "    cts_clk_adda_320M";    break;
        case CTS_CLK_ADDA_80M_POS   : clk_name = "    cts_clk_adda_80M";     break;
        case CTS_CLK_ADDA_160M_POS  : clk_name = "    cts_clk_adda_160M";    break;
        case CTS_CLK_XDAC_640M_POS  : clk_name = "    cts_clk_xdac_640M";    break;
        case CTS_CLK_ADDA_640M_POS  : clk_name = "    cts_clk_adda_640M";    break;
        case CTS_CLK_FEC_POS        : clk_name = "    cts_clk_fec";          break;

        case CTS_CLK_MAC_POS        : clk_name = "    cts_clk_mac";          break;
        case CTS_CLK_APB_POS        : clk_name = "    cts_clk_apb";          break;
        case CTS_CLK_CPU_POS        : clk_name = "    cts_clk_cpu";          break;
        case XJTAG_TCK_I_POS        : clk_name = "    xjtag_tck";            break;
        case SYS_OSCIN_I_POS        : clk_name = "    sys_osc_in";           break;
        case WF_DIG_OUT_POS         : clk_name = "    wf_dig_out";           break;

        default                     : clk_name = "    unknown, not support";  break;
    }

    return clk_name;
}

//wire  [127:0]   clk_to_msr_in   = {  109'h0,
//                                     WF_DIG_OUT,
//                                     sys_oscin_i,
//                                     cts_clk_fec,
//                                     xjtag_tck_i,
//                                     cts_clk_adda_640m,
//                                     cts_clk_cpu,
//                                     cts_clk_apb,
//                                     cts_clk_mac,
//                                     cts_clk_xdac_640m,
//                                     cts_clk_adda_160m,
//                                     cts_clk_radc_320m,
//                                     cts_clk_adda_80m,
//                                     1'b0,
//                                     cts_clk_adda_320m,
//                                     cts_clk_11brx
//                                  };
static void clk_msr(CLOCK_MUX clk_mux)
{
    if (clk_mux != CTS_CLK_VIT_POS) {
        pr_debug("clk mux %s\n", clk_mux_name(clk_mux));
    }

    switch (clk_mux) {
        case CTS_CLK_11BRX_POS      :
        //case CTS_CLK_VIT_POS        :
        case CTS_CLK_RADC_320M_POS  :  clk_msr_other(clk_mux);    break;

        case CTS_CLK_ADDA_320M_POS  :
        case CTS_CLK_ADDA_80M_POS   :
        case CTS_CLK_ADDA_160M_POS  :
        case CTS_CLK_XDAC_640M_POS  :
        case CTS_CLK_ADDA_640M_POS  :
        case CTS_CLK_FEC_POS        :  clk_msr_adda(clk_mux);     break;

        case CTS_CLK_MAC_POS        :  clk_msr_mac();             break;
        case CTS_CLK_APB_POS        :  clk_msr_apb();             break;
        case CTS_CLK_CPU_POS        :  clk_msr_cpu();             break;
        case XJTAG_TCK_I_POS        :  clk_msr_xjtag();           break;
        case SYS_OSCIN_I_POS        :  clk_msr_oscin();           break;
        case WF_DIG_OUT_POS         :  clk_msr_wfdig();           break;

        default:                                                  break;
    }
}

static void clk_msr_all(void)
{
    CLOCK_MUX i;

    for (i = CTS_CLK_11BRX_POS; i < CLOCK_MUX_MAX; i++) {
        clk_msr(i);
    }
}

void host_clk_msr(CLOCK_MUX clk_mux)
{
    if (clk_mux == 0xff) {
        clk_msr_all();
    } else if (clk_mux >=  CTS_CLK_11BRX_POS && clk_mux < CLOCK_MUX_MAX) {
        clk_msr(clk_mux);
    }
}

