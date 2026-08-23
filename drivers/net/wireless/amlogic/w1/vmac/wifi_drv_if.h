
/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 DRIVER  layer Software
 *
 * Description:
 *   DRIVER and HAL layer interface function
 *
 *
 ****************************************************************************************
 */
#ifndef _DRV2HAL_IF_H_
#define _DRV2HAL_IF_H_

#include "wifi_hal_cmd.h"

struct wifi_station;

#define OFDM_SIFS_TIME        16
#define OFDM_PREAMBLE_TIME    20
#define OFDM_PLCP_BITS        22
#define OFDM_SYMBOL_TIME       4

#define CCK_SIFS_TIME        10
#define CCK_PREAMBLE_BITS   144
#define CCK_PLCP_BITS        48
/* value used to specify no encryption key for xmit */
#define HAL_TXKEYIX_INVALID    ((unsigned int) -1)

/* rcs_flags definition */
#define HAL_RATECTRL_USE_FIXED_RATE 0x0001
#define HAL_RATECTRL_CW40_FLAG 0x0002    /* CW 40 */
#define HAL_RATECTRL_SGI_FLAG 0x0004    /* Short Guard Interval */
#define HAL_RATECTRL_TX_SEND_SUCCESS 0X800
#define HAL_RATECTRL_USE_SAMPLE_RATE 0X1000

#define HAL_BEACON_PERIOD 0x0001ffff  /* beacon interval period */
#define WLAN_CTRL_FRAME_SIZE 14   /* ACK+FCS */
#define CO_WORK_GET 0
#define CO_WORK_FREE 1
#define CO_SF_WORK_NBR 2

/* WME stream classes */
#define WME_AC_BE 0       /* best effort */
#define WME_AC_BK 1       /* background */
#define WME_AC_VI 2       /* video */
#define WME_AC_VO 3       /* voice */

/*
 * Spatial Multiplexing Modes.
 */
enum
{
    HAL_SM_ENABLE,
    HAL_SM_PWRSAV_STATIC,
    HAL_SM_PWRSAV_DYNAMIC,
} ;

/**
 *  Protection Mode
 */
enum prot_mode
{
    PROT_M_NONE = 0,
    PROT_M_RTSCTS,
    PROT_M_CTSONLY
} ;



struct hal_key_val
{
    unsigned char    kv_type;        /* one of HAL_CIPHER */
    unsigned char    kv_pad;
    unsigned short   kv_len;         /* length in bits */
    unsigned char    kv_val[16];     /* enough for 128-bit keys */
    unsigned char    kv_mic[8];      /* TKIP Rx MIC key */
    unsigned char    kv_txmic[8];    /* TKIP Tx MIC key */
} ;



enum hal_key_type
{
    HAL_KEY_TYPE_CLEAR = WIFI_NO_WEP,
    HAL_KEY_TYPE_WEP    = WIFI_WEP64,
    HAL_KEY_TYPE_WEP128 = WIFI_WEP128,
    HAL_KEY_TYPE_AES = WIFI_AES,
    HAL_KEY_TYPE_TKIP = WIFI_TKIP,
    HAL_KEY_TYPE_WPI = WIFI_WPI,
} ;



enum
{
    HAL_SLOT_TIME_6  = 6,
    HAL_SLOT_TIME_9  = 9,
    HAL_SLOT_TIME_20 = 20,
};

/* 11n */
 enum
{
    HAL_HT_MACMODE_20       = 0,            /* 20 MHz operation */
    HAL_HT_MACMODE_2040     = 1,            /* 20/40 MHz operation */
} ;

 enum
{
    HAL_HT_EXTPROTSPACING_20    = 0,            /* 20 MHZ spacing */
    HAL_HT_EXTPROTSPACING_25    = 1,            /* 25 MHZ spacing */
} ;



/* 11n */
 enum
{
    HAL_BAND_2G      = 0,            /*2.4G hz*/
    HAL_BAND_5G     = 1,            /* 5G hz */
} ;

struct country_chan_mapping {
    char country[2];
    unsigned char chplan;
};

struct country_chan_plan {
    char support_class_num;
    int support_class[17];
    char class_na_freq_plan;
    unsigned char dfs_chan_flag;
    unsigned char tx_power_plan;
};

enum
{
    TX_POWER_DEFAULT = 0,
    TX_POWER_CE,          //EU
    TX_POWER_FCC,         //US
    TX_POWER_ARIB,        //JP
    TX_POWER_SRRC,        //CN
    TX_POWER_ANATEL,      //BR
};

struct tx_power_plan {
    unsigned char cffc_num;
    unsigned char band_pwr_table[4];
    unsigned char coefficient[57];
};

/*
 * Calculate the transmit duration of a frame.
 */
unsigned short 
drv_hal_calc_txtime(const struct drv_rate_table *rates,
    unsigned int frameLen,unsigned short rateix, int shortPreamble);

static inline int drv_hal_setmac(unsigned char wnet_vif_id, unsigned char *mac)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_set_mac_addr(wnet_vif_id,mac);
    return 1;
}

static inline int drv_hal_interset( int ints)
{
    return 0;
}

static inline int drv_hal_setpower(int hal_ps_mode,int wnet_vif_id )
{
    struct hal_private* hal_priv = hal_get_priv();
    unsigned char tmpflag = hal_priv->powersave_init_flag;
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d hal_ps_mode=%d wnet_vif_id=%d\n",
			__func__,__LINE__, hal_ps_mode, wnet_vif_id);
    if (wnet_vif_id == PHY_VMAC_ID)
        hal_priv->powersave_init_flag = 1;
    hal_priv->hal_ops.phy_pwr_save_mode(wnet_vif_id,hal_ps_mode);
    hal_priv->powersave_init_flag = tmpflag;
    return 0;
}

//Reset_Key_Cmd
static inline int drv_hal_keyclear(unsigned char wnet_vif_id, int staid)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_KEY,"<running> %s %d \n",__func__,__LINE__);
    hal_priv->hal_ops.phy_clr_key( wnet_vif_id,staid&0xff);
    return 0;

}

//Reset_Key_Cmd
static inline int drv_hal_keyreset( unsigned char wnet_vif_id,unsigned short _ix)
{
    
    struct hal_private* hal_priv = hal_get_priv();

    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_KEY,"<running> %s %d \n",__func__,__LINE__);
    hal_priv->hal_ops.phy_rst_all_key( wnet_vif_id);
    hal_priv->hal_ops.phy_rst_mcast_key( wnet_vif_id);
    hal_priv->hal_ops.phy_rst_ucast_key( wnet_vif_id);

    return 0;
}

//UniCast_Key_Set_Cmd   Multicast_Key_Set_Cmd
static inline int 
drv_hal_keyset(unsigned char wnet_vif_id, unsigned short _ix,
    struct hal_key_val *_pk, unsigned char *_mac, unsigned short staaid)
{
    struct hal_private* hal_priv = hal_get_priv();


    if (_pk->kv_type == HAL_KEY_TYPE_WEP)        //fix WEP for wep 64 and wep 128
    {
        if (_pk->kv_len==13)
        {
            _pk->kv_type = WIFI_WEP128;
        }
        else
        {
            _pk->kv_type = WIFI_WEP64;
        }
    }

    if ((_mac[0]&0x01) == 0x01)                //mutil cast
    {
        hal_priv->hal_ops.phy_set_mcast_key(wnet_vif_id,(unsigned char*)_pk->kv_val,
                (unsigned char)_pk->kv_len, (unsigned int)_ix, 
                (unsigned char)_pk->kv_type, 1);
    }
    else                                     //ucast
    {
        hal_priv->hal_ops.phy_set_ucast_key(wnet_vif_id,staaid & 0xff, _mac, 
                _pk->kv_val,32, _pk->kv_type,_ix);

    }
    return 1;
}

static inline int 
drv_hal_rekey_data_set(unsigned char wnet_vif_id, 
    void *rekey_data, unsigned short staaid)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_set_rekey_data(wnet_vif_id, rekey_data, staaid & 0xff);
    return 1;
}

// Chip_Reset_Cmd 
static inline int drv_hal_reset(void)
{


    DPRINTF(AML_DEBUG_HAL| AML_DEBUG_ACL,"<running> %s %d \n",__func__,__LINE__);

    return 1;
}

static inline int drv_hal_set_chan_support(struct hal_channel *chan)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_set_chan_support_type(chan);
    return 0;
}

//Channel_Set_Cmd
static inline void drv_hal_setchannel(struct hal_channel *hchan, unsigned char flag, unsigned char vid, unsigned char opmode)
{
    struct hal_private* hal_priv = hal_get_priv();

    hal_priv->hal_ops.phy_set_rf_chan(hchan, flag, vid, opmode);
}
//Channel_Set_Cmd
static inline void drv_hal_scancmd(unsigned int start)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_scan_cmd(start);
}

//BCNInterval_Cmd
static inline int 
drv_hal_beaconinit(unsigned char wnet_vif_id,unsigned int _bperiod)
{
   struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON|AML_DEBUG_PWR_SAVE,
        "%s %d: wnet_vif_id=%d _bperiod = %d\n",
        __func__,__LINE__, wnet_vif_id, _bperiod);
    hal_priv->hal_ops.phy_set_bcn_intvl(wnet_vif_id,_bperiod&HAL_BEACON_PERIOD);
    return 0;
}

static inline int drv_hal_disable(void)
{
    struct hal_private* hal_priv = hal_get_priv();
    return hal_priv->hal_ops.hal_close(hal_priv->drv_priv);
}

static inline int drv_hal_enable(void)
{
    struct hal_private* hal_priv = hal_get_priv();
    return hal_priv->hal_ops.hal_open(hal_priv->drv_priv);
}

static inline int drv_hal_setopmode(unsigned char wnet_vif_id, enum hal_op_mode opmode)
{
    struct hal_private* hal_priv = hal_get_priv();

    DPRINTF(AML_DEBUG_HAL,"%s %d: opmode = %d\n",__func__,__LINE__,opmode);
    hal_priv->hal_ops.phy_set_cam_mode( wnet_vif_id,opmode);
    return 0;

}

static inline int drv_hal_setdhcp(unsigned char wnet_vif_id, unsigned int ip)
{
    struct hal_private* hal_priv = hal_get_priv();

    if (hal_priv->dhcp_offload == 1) {
        pr_notice("NOTICE: DHCP OFFLOAD function is enabled, caller must ensure upper layer's dhcp module is closed.\n");
        pr_notice("Because under this situation, the FW will handle the dhcp pkts and never transmit to driver.\n");
        DPRINTF(AML_DEBUG_HAL, "%s %d: ip %03d.%03d.%03d.%03d\n",__func__,__LINE__,
            (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
        hal_priv->hal_ops.phy_set_dhcp(wnet_vif_id, ip);
    }
    return 0;
}

static inline int drv_hal_setslottime(int us)
{
     struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_set_slot_time( us);
    return 0;
}


/* 11n */
static inline int drv_hal_get11nextbusy(void)
{
     struct hal_private* hal_priv = hal_get_priv();

    return hal_priv->hal_ops.phy_get_extern_chan_status() ;
}


static inline int drv_hal_set11nmac2040( enum wifi_mac_chanbw  chan_bw)  
{
     struct hal_private* hal_priv = hal_get_priv();

    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BWC,"drv_hal_set11nmac2040 %d \n",chan_bw);

    hal_priv->hal_ops.phy_set_chan_phy_type(chan_bw);
    return 0;
}

static inline void drv_hal_set_bmfm_info(int wnet_vif_id, unsigned char * group_id,
        unsigned char * user_position, unsigned char feedback_type)
{
    struct hal_private* hal_priv = hal_get_priv();

    hal_priv->hal_ops.phy_set_bmfm_info(wnet_vif_id, group_id, user_position, feedback_type);
}

static inline void 
drv_hal_put_bcn_buf(unsigned char wnet_vif_id,unsigned char *pBeacon,
    unsigned short len,unsigned char Rate,unsigned short Flag)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON,"<running> %s %d\n",__func__,__LINE__);
    hal_priv->hal_ops.phy_set_bcn_buf(wnet_vif_id, pBeacon, len, Rate, Flag);
}


static inline void 
drv_hal_set_bcn_start(unsigned char wnet_vif_id,unsigned short intval, 
    unsigned char dtim_count,unsigned short  bsstype)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON,"<running> %s %d intval=%d\n",__func__,__LINE__,intval);
    hal_priv->hal_ops.phy_enable_bcn(wnet_vif_id,intval,dtim_count,bsstype);
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON,"<running> %s %d bsstype=%d\n",__func__,__LINE__,bsstype);
}

static inline void drv_hal_wnet_vifdisconnect(unsigned char wnet_vif_id)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON,"<running> %s %d wnet_vif_id=%d\n",
            __func__,__LINE__,wnet_vif_id);
    hal_priv->hal_ops.phy_vmac_disconnect(wnet_vif_id);
}

static inline void drv_hal_wnet_vifStop(unsigned char wnet_vif_id)
{
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON,"<running> %s %d wnet_vif_id=%d\n",
            __func__,__LINE__,wnet_vif_id);
}

static inline void drv_hal_wnet_vifStart(unsigned char wnet_vif_id)
{
    DPRINTF(AML_DEBUG_HAL|AML_DEBUG_BEACON,"<running> %s %d wnet_vif_id=%d\n",
            __func__,__LINE__,wnet_vif_id);
}

static inline unsigned int drv_low_register_behind_task(void *func)
{
    struct hal_private* hal_priv = hal_get_priv();
    return hal_priv->hal_ops.hal_reg_task(func);
}

static inline unsigned int drv_low_call_register_task(SYS_TYPE taskid,SYS_TYPE param1)
{
    struct hal_private* hal_priv = hal_get_priv();
    return hal_priv->hal_ops.hal_call_task(taskid,param1);
}

static inline unsigned int 
hal_phy_addba_ok(unsigned char wnet_vif_id,unsigned short StaAid,
    unsigned char TID,unsigned short SeqNumStart,unsigned char BA_Size,
    unsigned char AuthRole,unsigned char BA_TYPE)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_addba_ok(wnet_vif_id,StaAid & 0xff,TID,
        SeqNumStart,BA_Size,AuthRole,BA_TYPE);
    return 0;
}

static inline unsigned int 
hal_phy_delt_ba_ok(unsigned char wnet_vif_id,unsigned short StaAid,
    unsigned char TID,unsigned char AuthRole)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_delt_ba_ok(wnet_vif_id,StaAid&0xff,TID,AuthRole);
    return 0;
}


static inline unsigned int 
hal_phy_register_sta_id(unsigned char wnet_vif_id,unsigned short StaAid,
    unsigned char *pMac, unsigned char encrypt)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_register_sta_id(wnet_vif_id,StaAid&0x3fff,pMac, encrypt);
    return 0;
}

static inline unsigned int 
hal_phy_unregister_sta_id(unsigned char wnet_vif_id,unsigned short StaAid)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_unregister_sta_id(wnet_vif_id,StaAid&0xff);
    return 0;
}

static inline unsigned int hal_phy_unregister_all_sta_id(unsigned char wnet_vif_id)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.phy_unregister_all_sta_id(wnet_vif_id);
    return 0;
}



static inline unsigned int 
drv_hal_settxqueueprops(unsigned char wnet_vif_id,int ac,unsigned char aifsn,
    unsigned char cwminmax,unsigned short txop)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_PWR_SAVE, "%s %d\n",__func__,__LINE__);
    hal_priv->hal_ops.phy_set_ac_param(wnet_vif_id,ac, aifsn, cwminmax, txop);
    return 0 ;
}

static inline unsigned int drv_hal_settxqueueflush(unsigned char vid)
{
    struct hal_private* hal_priv = hal_get_priv();
    hal_priv->hal_ops.hal_tx_flush(vid);
    return 0 ;
}

int drv_hal_detach(void);

static inline void drv_hal_setretrynum(unsigned short retrynum)
{
    struct hal_private* hal_priv = hal_get_priv();
    unsigned int temp;

    temp = 0xff & retrynum;

    DPRINTF(AML_DEBUG_HAL,"<running> %s %d longretry=%x\n",__func__,__LINE__,temp );
    hal_priv->hal_ops.phy_set_lretry_limit(temp);
    temp = 0xff & (retrynum>>8);
    DPRINTF(AML_DEBUG_HAL,"<running> %s %d shortretry=%x\n",__func__,__LINE__,temp );
    hal_priv->hal_ops.phy_set_sretry_limit(temp);

}

static inline void drv_hal_txlivetime(unsigned int timems)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL,"<running> %s %d txlivetime =%x ms\n",__func__,__LINE__,timems);
    hal_priv->hal_ops.phy_set_txlive_time(timems);
}

static inline void drv_hal_enable_coexist(unsigned int coexist_en)
{
    struct hal_private* hal_priv = hal_get_priv();
    DPRINTF(AML_DEBUG_HAL,"<running> %s %d coexist_en =%x ms\n",__func__,__LINE__,coexist_en);
    hal_priv->hal_ops.phy_set_coexist_en(coexist_en);
}

void drv_hal_setupratetable(struct drv_rate_table *rt);
unsigned char drv_hal_wnet_vif_staid(unsigned char vm_opmode,unsigned short sta_associd);
unsigned short drv_hal_staid(enum hal_op_mode hal_opmode,unsigned short sta_associd);
unsigned char drv_hal_nsta_staid(struct wifi_station *sta);
struct drv_rate_table *drv_hal_get_rate_tbl(int mode);
int drv_hal_workitem_inital(void);
void drv_hal_workitem_free(void);
int drv_hal_add_workitem(WorkHandler task, WorkHandler taskcallback, SYS_TYPE param1,
    SYS_TYPE param2, SYS_TYPE param3, SYS_TYPE param4, SYS_TYPE param5);
void drv_hal_attach( void *drv_priv, void *cbptr);
#endif //_DRV2HAL_IF_H_

