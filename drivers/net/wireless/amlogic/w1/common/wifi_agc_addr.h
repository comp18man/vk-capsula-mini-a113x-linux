#ifndef __ADDR_AGC_H__
#define __ADDR_AGC_H__

#define WIFI_AGC          (0x8000)

#define RG_AGC_STF_LIMIT_CCA_COND_EN (WIFI_AGC + 0x6c)
#define RG_AGC_CCA_COND_TS_CTRL (WIFI_AGC + 0x70)
#define RG_AGC_RG_AGC_OB_CCA_RES (WIFI_AGC + 0x274)
#define RG_AGC_RG_AGC_OB_CCA_COND01 (WIFI_AGC + 0x278)
#define RG_AGC_RG_AGC_OB_CCA_COND23 (WIFI_AGC + 0x27c)
#define RG_AGC_STATE (WIFI_AGC+0x258)
#define RG_AGC_OB_IC_GAIN (WIFI_AGC+0x25c)
#define RG_AGC_WATCH1 (WIFI_AGC+0x260)
#define RG_AGC_WATCH2 (WIFI_AGC+0x264)
#define RG_AGC_WATCH3 (WIFI_AGC+0x268)
#define RG_AGC_WATCH4 (WIFI_AGC+0x26c)
#define RG_AGC_OB_ANT_NFLOOR (WIFI_AGC+0x270)
#define RG_AGC_OB_CCA_RES (WIFI_AGC+0x274)
#define RG_AGC_OB_CCA_COND01 (WIFI_AGC+0x278)
#define RG_AGC_OB_CCA_COND23 (WIFI_AGC+0x27c)
#define RG_AGC_GAIN_SEL (WIFI_AGC+ 0x84)
#define RG_AGC_STS_REG_0 (WIFI_AGC+ 0x64)
#define RG_AGC_EN (WIFI_AGC+ 0x10)


struct	AGC_CCA_COND_TS_CTRL_BITS {
    unsigned int cca_cond_ts :8,
                    cca_cond_ts_num :20,
                    ed_avglen :3,
                    reserved41 :1;
};

struct	AGC_OB_CCA_RES_BITS {
    unsigned int cca_cond_rdy :1,
                    reserved170 :15,
                    cca_cond8 :16;
};

struct	AGC_STF_LIMIT_CCA_COND_EN_BITS {
    unsigned int stf_cfo_limit :19,
                    reserved39 :1,
                    cca_cond_en :9,
                    cca_cal_force :1,
                    reserved40 :2;
};

#pragma pack(push ,1)
struct agc_prt_info
{
    unsigned short addr;
    char *prt1;
    char *prt2;
};

struct sts_agc_state {
     unsigned int rsv1:8,state:4,rsv2:20;
};

struct sts_agc_ob_ic_gain {
     unsigned int r_rx_ic_gain:8,r_agc_fine_db:9,rsv:15;
};

struct sts_agc_watch1 {
    unsigned int r_pow_sat_db:9, rsv1:3, r_primary20_db:9, rsv2:11;
};

struct sts_agc_watch2 {
    unsigned int r_primary40_db:9, rsv1:3,r_primary80_db:9,rsv2:11;
};

struct sts_agc_watch3 {
    unsigned int r_rssi0_db:10, rsv1:2,r_rssi1_db:10,rsv2:10;
};

struct sts_agc_watch4 {
    unsigned int r_rssi2_db:10, rsv1:2,r_rssi3_db:10,rsv2:10;
};

struct sts_ob_ant_nfloor {
    unsigned int r_snr_qdb:10, rsv1:2,r_ant_nfloor_qdb:10,rsv2:10;
};

struct sts_ob_cca_res {
    unsigned int cca_cond_rdy:1, rsv1:15, cca_cond8:16;
};

struct sts_ob_cca_cond01 {
    unsigned int cca_cond0:16, cca_cond1:16;
};

struct sts_ob_cca_cond23 {
     unsigned int cca_cond2:16, cca_cond3:16;
};

union sts_agc_reg_u {
    struct sts_agc_state state;
    struct sts_agc_ob_ic_gain ic_gain;
    struct sts_agc_watch1 wtc1;
    struct sts_agc_watch2 wtc2;
    struct sts_agc_watch3 wtc3;
    struct sts_agc_watch4 wtc4;
    struct sts_ob_ant_nfloor ant_nfloor;
    struct sts_ob_cca_res cca_res;
    struct sts_ob_cca_cond01 cca_cond_m1;
    struct sts_ob_cca_cond23 cca_cond_m2;
};

#pragma  pack(pop)

#endif
