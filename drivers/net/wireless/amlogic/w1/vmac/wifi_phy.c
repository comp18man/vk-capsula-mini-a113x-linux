/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 HAL  layer Software
 *
 * Description:
 *  phy control interface module
 *
 *
 ****************************************************************************************
 */
#ifdef HAL_SIM_VER
#ifdef FW_NAME
namespace FW_NAME
{
#endif
#endif

#include "wifi_hal_com.h"
#include "wifi_hif.h"

static const struct reg_table wifi_fpga_regtable[] =
{
    /*The following two registers are modified for B2B & Phy2Phy platform*/
#ifdef WITH_RF
    { 0x0000b080, 0x00001100},
    { 0x0000b228, 0x004000a0 }, //rf rx tx turn around time, default as 2us ^M
#else
    { 0x0000b080, 0x00001010},
    { 0x00008090, 0x00000104},
#endif
    /*product platform init reg*/
#ifdef RF_T9023
    { 0x00008114, 0x00000005 }, // rf  offset for min gain
    { 0x00008300, 0x00000011 },
    { 0x00008304, 0x00000012 },
    { 0x00008308, 0x00000013 },
    { 0x0000830c, 0x00000014 },
    { 0x00008310, 0x00000015 },
    { 0x00008314, 0x00000016 },
    { 0x00008318, 0x00000024 },
    { 0x0000831c, 0x00000025 },
    { 0x00008320, 0x00000026 },
    { 0x00008324, 0x00000034 },
    { 0x00008328, 0x00000035 },
    { 0x0000832c, 0x00000036 },
    { 0x00008330, 0x00000044 },
    { 0x00008334, 0x00000045 },
    { 0x00008338, 0x00000046 },
    { 0x0000833c, 0x00000054 },
    { 0x00008340, 0x00000055 },
    { 0x00008344, 0x00000056 },
    { 0x00008348, 0x00000064 },
    { 0x0000834c, 0x00000065 },
    { 0x00008350, 0x00000066 },
    { 0x00008354, 0x00000074 },
    { 0x00008358, 0x00000075 },
    { 0x0000835c, 0x00000076 },
    { 0x00008360, 0x00000084 },
    { 0x00008364, 0x00000085 },
    { 0x00008368, 0x00000086 },
    { 0x0000836c, 0x00000094 },
    { 0x00008370, 0x00000095 },
    { 0x00008374, 0x00000096 },
    { 0x00008378, 0x00000097 },
    { 0x0000837c, 0x00000098 },
    { 0x00008380, 0x00000099 },
    { 0x00008384, 0x0000009a },
    { 0x00008388, 0x0000009b },
    { 0x0000838c, 0x0000009c },
    { 0x00008390, 0x0000009c },
    { 0x00008114, 0x00000105 }, // rf  offset for min gain
    
    { 0x00008050, 0x003ec104 },
    { 0x00008054, 0x003ec104 },
    { 0x00008058, 0x00050104 },
       
    {RG_PHY_FS_ADC, 0x12000120},
    { 0x0000c02c, 0x00000000 }//coex wifi thread0 priority
#endif
};

int get_snr(void) {
    int snr = 0;
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int reg_tmp = 0x01;

    hif->hif_ops.hi_write_word(RG_PHY_ADR_2C20, reg_tmp);
    snr = hif->hif_ops.hi_read_word(RG_PHY_STS_REG_0 + 3 *sizeof(unsigned int));

    return (snr & 0xffff);
}

void phy_stc(void)
{
    int i;
    unsigned int reg[8] = {0};
    unsigned int trig_num[4] = {0};
    unsigned int min_num[4]  = {0};
    unsigned int max_num[4]  = {0};
    unsigned int avg_num[4]  = {0};

    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int reg_tmp = 0x01;


    hif->hif_ops.hi_write_word(RG_PHY_ADR_2C20, reg_tmp);

    for(i = 0; i< 8; i++)
    {
        reg[i] = hif->hif_ops.hi_read_word(RG_PHY_STS_REG_0 + i *sizeof(unsigned int));
    }
        
    trig_num[0] = reg[0]&0xffff;
    trig_num[1] = (reg[0]>>16)&0xffff;
    trig_num[2] = reg[4]&0xffff;
    trig_num[3] = (reg[4]>>16)&0xffff;

    min_num[0] = (reg[1]>>16)&0xffff;
    min_num[1] = (reg[3]>>16)&0xffff;
    min_num[2] = (reg[5]>>16)&0xffff;
    min_num[3] = (reg[7]>>16)&0xffff;   

    max_num[0] = (reg[1])&0xffff;
    max_num[1] = (reg[3])&0xffff;
    max_num[2] = (reg[5])&0xffff;
    max_num[3] = (reg[7])&0xffff;   

    avg_num[0] = reg[2]&0xffff;
    avg_num[1] = (reg[2]>>16)&0xffff;
    avg_num[2] = reg[6]&0xffff;
    avg_num[3] = (reg[6]>>16)&0xffff;

    pr_debug("phy statistic(dec): \n");
    pr_debug("0) CP1   detect:%8d  avg:%8d  min:%8d  max:%8d  \n", trig_num[0],avg_num[0],min_num[0],max_num[0]);
    pr_debug("1) L-SIG   SNR :%8d  avg:%8d  min:%8d  max:%8d  \n", trig_num[1],avg_num[1],min_num[1],max_num[1]);
    pr_debug("2) data CRC err:%8d  avg:%8d  min:%8d  max:%8d  \n", trig_num[2],avg_num[2],min_num[2],max_num[2]);
    pr_debug("3) data CRC OK:%8d  avg:%8d  min:%8d  max:%8d  \n", trig_num[3],avg_num[3],min_num[3],max_num[3]);

}

unsigned int cca_busy_check(void)
{
    unsigned int v0 = 0, v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    struct AGC_OB_CCA_RES_BITS *data0 = NULL;
    struct AGC_STF_LIMIT_CCA_COND_EN_BITS *data1 = NULL;
    struct AGC_CCA_COND_TS_CTRL_BITS *data3 = NULL;
    unsigned int reg_tmp = 0x13000026;
    struct hw_interface* hif = hif_get_hw_interface();
     
    set_reg_fragment(RG_AGC_STF_LIMIT_CCA_COND_EN, 28,20,0x1ff);//cca_cond_en
    hif->hif_ops.hi_write_word(RG_AGC_STS_REG_0, reg_tmp);
    v0 = hif->hif_ops.hi_read_word(RG_AGC_OB_CCA_RES);
    data0 = (struct AGC_OB_CCA_RES_BITS *)&v0;
    v1 = hif->hif_ops.hi_read_word(RG_AGC_STF_LIMIT_CCA_COND_EN);
    data1 = (struct AGC_STF_LIMIT_CCA_COND_EN_BITS *)&v1;
    v3 =  hif->hif_ops.hi_read_word(RG_AGC_CCA_COND_TS_CTRL);
    
    data3 = (struct AGC_CCA_COND_TS_CTRL_BITS *)&v3;
    data3->cca_cond_ts_num = 0x0ffff;
    data3->cca_cond_ts = 0xc8;

    hif->hif_ops.hi_write_word(RG_AGC_CCA_COND_TS_CTRL, v3);
    if (data0->cca_cond_rdy && data1->cca_cond_en)
    {
        v2 = hif->hif_ops.hi_read_word(RG_AGC_OB_CCA_COND01);
        v4 = hif->hif_ops.hi_read_word(RG_AGC_OB_CCA_COND23);
        pr_debug("cca: ts 0x%x, ts_num %d, cond0 %d cond1 %d cond2 %d cond3 %d \n",
                data3->cca_cond_ts, data3->cca_cond_ts_num,
                v2 & 0xffff,  (v2 >> 16) & 0xffff,  v4 & 0xffff, (v4 >> 16 ) & 0xffff);
    }

    return 0;

}

void get_phy_stc_info(unsigned int *arr)
{
    int i;
    unsigned int reg[8] = {0};
    unsigned int trig_num[4] = {0};
    unsigned int min_num[4]  = {0};
    unsigned int max_num[4]  = {0};
    unsigned int avg_num[4]  = {0};
    unsigned int noise_floor = 0;

    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int reg_tmp = 0x01;


    hif->hif_ops.hi_write_word(RG_PHY_ADR_2C20, reg_tmp);

    udelay(320);

    for (i = 0; i < 8; i++) {
        reg[i] = hif->hif_ops.hi_read_word(RG_PHY_STS_REG_0 + i *sizeof(unsigned int));
    }
    noise_floor = hif->hif_ops.hi_read_word(RG_AGC_OB_ANT_NFLOOR);

    trig_num[0] = reg[0] & 0xffff;
    trig_num[1] = (reg[0] >> 16) & 0xffff;
    trig_num[2] = reg[4] & 0xffff;
    trig_num[3] = (reg[4] >> 16) & 0xffff;

    min_num[0] = (reg[1] >> 16) & 0xffff;
    min_num[1] = (reg[3] >> 16) & 0xffff;
    min_num[2] = (reg[5] >> 16) & 0xffff;
    min_num[3] = (reg[7] >> 16) & 0xffff;

    max_num[0] = (reg[1]) & 0xffff;
    max_num[1] = (reg[3]) & 0xffff;
    max_num[2] = (reg[5]) & 0xffff;
    max_num[3] = (reg[7]) & 0xffff;

    avg_num[0] = reg[2] & 0xffff;
    avg_num[1] = (reg[2] >> 16) & 0xffff;
    avg_num[2] = reg[6] & 0xffff;
    avg_num[3] = (reg[6] >> 16) & 0xffff;

    arr[0] = avg_num[0]; //CP1
    arr[1] = avg_num[1]; //L-SIG avg SNR
    arr[2] = avg_num[2]; //crc err avg
    arr[3] = avg_num[3]; //crc ok avg
    arr[4] = (noise_floor >> 12) & 0x3ff;
    arr[5] = noise_floor & 0x3ff;//snr_qdb
}


#if defined (HAL_SIM_VER)

void MAC_WR_REG(unsigned int addr,unsigned int data)
{
    struct hw_interface* hif = hif_get_hw_interface();

    hif->hif_ops.hi_write_word((addr), data);
}

unsigned int MAC_RD_REG(unsigned int addr)
{
    unsigned int regdata = 0;
    struct hw_interface* hif = hif_get_hw_interface();

    regdata = hif->hif_ops.hi_read_word((addr));
    return regdata;
}

/* aon module address from 0x00f00000, we need read/write by sdio func5 */
void AON_WR_REG(unsigned int addr,unsigned int data)
{
    struct hw_interface* hw_if = hif_get_hw_interface();

    hw_if->hif_ops.bt_hi_write_word((addr), data);
}

unsigned int AON_RD_REG(unsigned int addr)
{
    unsigned int regdata = 0;
    struct hw_interface* hw_if = hif_get_hw_interface();

    regdata = hw_if->hif_ops.bt_hi_read_word((addr));
    return regdata;
}
#endif
void phy_register_set(void)
{
#ifdef PHY_TEST_FUNC
    struct hw_interface* hif = hif_get_hw_interface();
    int i =0;
    unsigned int data =0;
    unsigned int ori_data = 0;


    for (i=0; i<(sizeof(wifi_fpga_regtable)/sizeof(struct reg_table)); i++)
    {
        ori_data = hif->hif_ops.hi_read_word(wifi_fpga_regtable[i].regaddr);
        hif->hif_ops.hi_write_word(wifi_fpga_regtable[i].regaddr,wifi_fpga_regtable[i].regdata);
        
        data = hif->hif_ops.hi_read_word(wifi_fpga_regtable[i].regaddr);
        data = hif->hif_ops.hi_read_word(wifi_fpga_regtable[i].regaddr);
        
        if (data != wifi_fpga_regtable[i].regdata)
        {
            pr_err("%s(%d): --Error------write reg=0x%x failed dataori=0x%x, dataread=0x%x\n",
                __func__, __LINE__,
                wifi_fpga_regtable[i].regaddr, wifi_fpga_regtable[i].regdata, data);
        }else{
            pr_debug("wifi reg 0x%04x data 0x%08x, ori_data=0x%08x\n",
                wifi_fpga_regtable[i].regaddr, wifi_fpga_regtable[i].regdata, ori_data);
        }
    }
    pr_debug("%s(%d)-- \n",__func__,__LINE__);
#endif //PHY_TEST_FUNC

#ifdef HAL_FPGA_VER

#ifdef WITH_RF
#ifndef RF_T9026
    hif->hif_ops.hi_write_word(RG_IQ_SWAP_CTRL, 0x01100002);
#endif
#else
    hif->hif_ops.hi_write_word(RG_ADDA_ADR_88, 0x200a0000);
#endif

#endif
   /* rf phase and the iq swap value need to change by the final FPGA version*/
    //hif->hif_ops.hi_write_word(RG_PHY_FPGA_CONTROL, 0x00600701);

}

void coexit_bt_thread_enable(void)
{
    unsigned int bt_prd,bt_act,bt_offset;
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int testbus_num = 6;

    
    bt_prd = 7500 * 10;//us
    bt_act = 1500 * 10;//us
    bt_offset = 3500;//us
    pr_debug("%s(%d)\n",__func__,__LINE__);
    
    /*bt wifi switch no channel conf*/
    hif->hif_ops.hi_write_word(RG_COEX_RF_STABLE_CTRL, 0x03002000 | (testbus_num << 26));
    //bt thread set
    hif->hif_ops.hi_write_word(RG_COEX_PRIORITY_M6, 0x2);
    hif->hif_ops.hi_write_word(RG_COEX_PRD_NUM_M6, bt_prd);
    hif->hif_ops.hi_write_word(RG_COEX_ACT_NUM_M6, bt_act);
    hif->hif_ops.hi_write_word(RG_COEX_OFFSET_NUM_M6, bt_offset);
    
    hif->hif_ops.hi_write_word(RG_COEX_INT_OFFSET_M6, bt_act + 5);
    hif->hif_ops.hi_write_word(RG_COEX_INI2_OFFSET_M6, 2000);
    hif->hif_ops.hi_write_word(RG_COEX_BT_OWNER_CTRL, 0x1001003f);
    set_reg_fragment(RG_COEX_HS5W_MANUAL4,0,0,1);
    
    pr_debug("%s(%d) BT priority=%d (%dus %dus %dus)--\n",
        __func__,__LINE__,hif->hif_ops.hi_read_word(RG_COEX_PRIORITY_M6),
        bt_prd,bt_act,bt_offset);
}

#ifdef HAL_SIM_VER
#ifdef FW_NAME
}
#endif
#endif
