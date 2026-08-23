/*
 ****************************************************************************************
 *
 * Copyright (C) amlogic.com 2009
 *
 * Project: amlogic Software
 *
 * Description:
 *      wifi soft mac hardware function
 *
 * Revision History:
 * $Source: wifi_hal_cmd.c
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
#include "wifi_common.h"
#include "wifi_hal_cmd.h"
#include "wifi_hal_txdesc.h"
#include "wifi_hal.h"
#include "patch_fi_cmd.h"
#include "wifi_mac_if.h"
#include "wifi_mac_chan.h"
#include "version.h"
#include <linux/firmware.h>
#include "wifi_mac_com.h"

#if defined (HAL_FPGA_VER)
#include "wifi_drv_statistic.h"
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#endif

#define WIFI_CONF_PATH "/lib/firmware/aml"

static char * conf_path = WIFI_CONF_PATH;
module_param(conf_path, charp, S_IRUGO);

unsigned char g_tx_power_change_disable = 0;
unsigned char g_wftx_pwrtbl_en = 0;
unsigned char g_initial_gain_change_disable = 0;
static struct WF2G_Txpwr_Param g_wf2g_txpwr_param;
static struct WF5G_Txpwr_Param g_wf5g_txpwr_param;
extern unsigned char tpc_mode;

//set cmd to firmware
unsigned int phy_set_param_cmd(unsigned char cmd,unsigned char vid,unsigned int data)
{
    unsigned char CmdData[64] = {0};
    struct hal_private * hal_priv = hal_get_priv();
    bool ret = false;
#ifdef POWER_SAVE_NO_SDIO
    unsigned int ptr = 0;
#endif
    DBG_ENTER();

    CmdData[0] = cmd;
    CmdData[3] = vid;
    CmdData[4] = (unsigned char)(data&0x00FF);
    CmdData[5] = (unsigned char)((data>>8)&0x00FF);
    CmdData[6] = (unsigned char)((data>>16)&0x00FF);
    CmdData[7] = (unsigned char)((data>>24)&0x00FF);

    if (cmd < MAX_HI_CMD)
        wifi_conf_mib.cmddata[cmd] = data;

    POWER_BEGIN_LOCK();
    if ((cmd == Power_Save_Cmd)
            //&& (data == PS_DOZE)
            && (hal_priv->powersave_init_flag == 0))
    {
        /* whatever driver ps status (active or sleep), just return when new cmd is PS_DOZE */
        if ((hal_priv->hal_drv_ps_status & HAL_DRV_STS_MASK) != 0)
        {
            POWER_END_LOCK();
            if (((hal_priv->hal_drv_ps_status & HAL_DRV_IN_ACTIVE) != 0) && (data == PS_DOZE))
            {
                hal_priv->hal_call_back->drv_pwrsave_wake_req(hal_priv->drv_priv, 1);
                return 0;
            }
            else if ((hal_priv->hal_drv_ps_status & HAL_DRV_IN_SLEEPING) != 0)
            {
                return 0;
            }
        }
        else
        {
            // set driver sleep flag to protect power save flow
            hal_priv->hal_drv_ps_status |= HAL_DRV_IN_SLEEPING;
        }

        if (data == PS_DOZE)
        {
            hal_priv->hal_fw_ps_status = HAL_FW_IN_SLEEP;
        }
    }

    POWER_END_LOCK();

    HAL_BEGIN_LOCK();
    ret = hi_set_cmd(CmdData,8);

    POWER_BEGIN_LOCK();

    if ((ret && (cmd == Power_Save_Cmd) && (data == PS_ACTIVE))
            || (!ret && (cmd == Power_Save_Cmd) && (data == PS_DOZE)))
    {
        if (hal_priv->hal_suspend_mode == HIF_SUSPEND_STATE_NONE)
        {
            hal_priv->hal_fw_ps_status = HAL_FW_IN_ACTIVE;
        }
    }
    hal_priv->hal_drv_ps_status &= ~HAL_DRV_IN_SLEEPING;

    POWER_END_LOCK();
    HAL_END_LOCK();

#ifdef POWER_SAVE_NO_SDIO
    if ((ret && (cmd == Power_Save_Cmd) && (data == PS_ACTIVE))
            || (!ret && (cmd == Power_Save_Cmd) && (data == PS_DOZE)))
    {
        ptr = hal_priv->hal_ops.phy_get_rw_ptr(0);
        hal_priv->rx_host_offset = ((ptr >> 16) & 0xffff) * 4;
    }
#endif

    if (cmd == Power_Save_Cmd)
        hal_priv->hal_call_back->drv_pwrsave_wake_req(hal_priv->drv_priv, 0);
    DBG_EXIT();
    return 1;
}

unsigned int phy_get_param_cmd_ul(unsigned char cmd,unsigned char vid)
{
    unsigned int CmdData[2] = {0};

    CmdData[0] = cmd|(vid<<24);
    HAL_BEGIN_LOCK();
    hi_get_cmd((unsigned char *)&CmdData[0],8);
    HAL_END_LOCK();

    return CmdData[1];
}

unsigned long long phy_get_param_cmd_ull(unsigned char cmd,unsigned char vid)
{
    CommonCmdULL scmd = {0};

    scmd.Cmd = cmd;
    scmd.vid = vid;
    HAL_BEGIN_LOCK();
    hi_get_cmd((unsigned char *)&scmd, sizeof(CommonCmdULL));
    HAL_END_LOCK();
    return scmd.param0;
}

#if defined (HAL_FPGA_VER)
unsigned int phy_set_rtc_enable(int enrtc)
{
    PRINT("Mac_Rtc_Cmd %x \n", enrtc);
    phy_set_param_cmd( Mac_Rtc_Cmd, 0, enrtc);
    return 1;
}

#endif

unsigned int phy_register_sta_id(unsigned char vid,unsigned short StaAid,unsigned char *pMac, unsigned char encrypt)
{
    RegStaIdParam CmdParam;
    struct hal_private * hal_priv =hal_get_priv();

    AML_OUTPUT("[%d]aid:%d, mac[%02x:%02x:%02x:%02x:%02x:%02x]\n", vid, StaAid, pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
    CmdParam.Cmd = RegStaID_CMD;
    CmdParam.StaId= StaAid;
    CmdParam.vid= vid;
    memcpy(CmdParam.StaMac, pMac, MAC_ADDR_LEN);
    CmdParam.encrypt = encrypt;
    memcpy((void *)&(hal_priv->sta_con_msg.CmdParam) , (void *)&CmdParam, sizeof(RegStaIdParam));

    if(StaAid >0)
    {
        HAL_BEGIN_LOCK();
        hi_set_cmd((unsigned char *)&CmdParam,sizeof(RegStaIdParam));
        HAL_END_LOCK();
    }

    return 1;
}

unsigned int phy_addba_ok(unsigned char wnet_vif_id,unsigned short StaAid,unsigned char TID,
    unsigned short SeqNumStart,unsigned char BA_Size,unsigned char AuthRole,unsigned char BA_TYPE)
{
    struct hal_private *hal_priv = hal_get_priv();
    Add_BA_Struct AddBA = {0};

    //PRINT("phy_addba_okwnet_vif_id %d StaAid %d,tid %d,sn 0x%x,baw,%d,authorless %d,batype %d \n",
      //  wnet_vif_id, StaAid, TID, SeqNumStart, BA_Size, AuthRole, BA_TYPE);

    DBG_ENTER();
    AddBA.Cmd = ADDBA_OK_CMD;
    AddBA.vid = wnet_vif_id;
    AddBA.STA_ID = StaAid;
    AddBA.TID = TID;
    AddBA.BA_TYPE = BA_TYPE /*immidiate_BA_TYPE*/;
    AddBA.AuthRole = AuthRole;
    AddBA.BA_Size = BA_Size;
    AddBA.SeqNumStart = SeqNumStart;

    memcpy((void *)&(hal_priv->sta_con_msg.AddBA) , (void *)&AddBA, sizeof(Add_BA_Struct));
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&AddBA,sizeof(Add_BA_Struct) );
    HAL_END_LOCK();

    return 1;
}

unsigned int phy_get_extern_chan_status(void)
{
    Extern_Channel_Status Channel_Status = {0};
    DBG_ENTER();
    Channel_Status.Cmd = EXTERN_CHANNEL_STATUS_CMD;
    Channel_Status.Status = 0;
    HAL_BEGIN_LOCK();
    hi_get_cmd((unsigned char*)&Channel_Status,sizeof(Channel_Status));
    HAL_END_LOCK();

    return (Channel_Status.Status/10000);
}

unsigned int phy_delt_ba_ok(unsigned char wnet_vif_id,unsigned short StaAid,unsigned char TID,
    unsigned char AuthRole)
{
    Delete_TxBA_Struct Delete_TxBA = {0};

    DBG_ENTER();
    Delete_TxBA.Cmd = Del_BA_CMD;
    Delete_TxBA.vid = wnet_vif_id;
    Delete_TxBA.STA_ID = StaAid;
    Delete_TxBA.TID = TID;
    Delete_TxBA.AuthRole = AuthRole;
    PRINT("phy_delt_ba_ok StaAid %d,TID %d,AuthRole=%d\n",StaAid ,TID,AuthRole);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&Delete_TxBA,sizeof(Delete_TxBA_Struct) );
    HAL_END_LOCK();

    return 1;
}

unsigned int phy_enable_bcn(unsigned char wnet_vif_id,unsigned short BeaconInterval,
     unsigned short DtimPeriod, unsigned char BssType)
{
     struct hal_private *hal_priv = hal_get_priv();
    if (BssType == WIFI_M_IBSS)
    {
        PRINT("Mode: IBSS_MODE\n");
    }
    else if (BssType == WIFI_M_HOSTAP)
    {
        PRINT("Mode: WIFI_M_HOSTAP\n");
    }
    else
    {
        return 0;
    }
    phy_set_bcn_intvl(wnet_vif_id,BeaconInterval);
    phy_set_param_cmd( Beacon_Enable_Cmd, wnet_vif_id,ENABLE_BEACON );
#if defined (HAL_FPGA_VER)
    hal_priv->sts_en_bcn[wnet_vif_id]++;
#endif

    return 1;
}

unsigned int phy_set_preamble_type( unsigned char PreambleType)
{
    struct hal_private *hal_priv = hal_get_priv();
    pr_debug("%s(%d) %x \n\n",__FUNCTION__, __LINE__,PreambleType);
    wifi_conf_mib.dot11PreambleType = PreambleType;
    hal_priv->sta_con_msg.PreambleType = PreambleType;
    return 1;
}

unsigned int phy_set_bcn_buf(unsigned char wnet_vif_id,unsigned char *pBeacon,
    unsigned short len,unsigned short Rate,unsigned short Flag)
{
    unsigned char buffer[1024] = {0};
    struct hal_private * hal_priv = hal_get_priv();
    struct  hw_interface* hif = hif_get_hw_interface();
    struct hw_tx_vector_bits *TxVector_Bit = NULL;
    unsigned int BcnAddr = hif->hw_config.beaconframeaddress;
    struct TxDescPage *pbeacon_s = (struct TxDescPage *)buffer;
    struct OtherTxPage  *other_page = NULL;
    int total_valid_len = len + sizeof(struct TxDescPage) - 1; /*not include fcs len*/
    /* beacon len per page */
    int firstpagelen = 0, otherpagelen = 0;

    ASSERT(len < (sizeof(buffer) - sizeof(struct TxDescPage) - sizeof(struct HW_TxBufferInfo)));
    /* get beacon frame address in firmware */
    if (hal_priv->beaconaddr[wnet_vif_id] == 0) {
        hal_priv->beaconaddr[wnet_vif_id] = phy_get_param_cmd_ul(Bcn_Frm_Addr_Cmd, wnet_vif_id);
    }
    BcnAddr = hal_priv->beaconaddr[wnet_vif_id];
    memcpy(pbeacon_s->txdata,  pBeacon,  len);

    /* build page flag */
    if (total_valid_len <= PAGE_LEN) {
        /* set firstpagelen and otherpagelen to 0 to be flag */
        firstpagelen = otherpagelen = 0;
        pbeacon_s->BufferInfo.MPDUBufFlag = 0;
        pbeacon_s->BufferInfo.MPDUBufFlag =  HW_FIRST_MPDUBUF_FLAG
            |HW_FIRST_AGG_FLAG | HW_LAST_MPDUBUF_FLAG | HW_LAST_AGG_FLAG;
        pbeacon_s->BufferInfo.MPDUBufFlag |= HW_MPDU_LEN_SET(len);
        pbeacon_s->BufferInfo.MPDUBufFlag |= HW_BUFFER_LEN_SET(firstpagelen);
        pbeacon_s->BufferInfo.MPDUBufNext = 0;

        pbeacon_s->BufferInfo.MPDUBufAddr =
            (unsigned int)(SYS_TYPE)((struct TxDescPage*)(SYS_TYPE)BcnAddr)->txdata;
        pbeacon_s->TxPriv.PageNum = 1;

    } else {
        firstpagelen = PAGE_LEN - (sizeof(struct TxDescPage) - 1);
        otherpagelen = len - firstpagelen;
        other_page = (struct OtherTxPage *)(buffer + PAGE_LEN);

        if (otherpagelen > 0) {
            /* move or push the beacon data to later position to prepare BufferInfo of 'other_page' */
            memmove((void *)(other_page->txdata), (void *)other_page, otherpagelen);
            memset((void *)other_page, 0, sizeof(struct HW_TxBufferInfo));
        }
        /*first page */
        pbeacon_s->BufferInfo.MPDUBufFlag = 0;
        pbeacon_s->BufferInfo.MPDUBufFlag = HW_FIRST_MPDUBUF_FLAG
            | HW_FIRST_AGG_FLAG | HW_LAST_AGG_FLAG;
        pbeacon_s->BufferInfo.MPDUBufFlag |= HW_MPDU_LEN_SET(len);
        pbeacon_s->BufferInfo.MPDUBufFlag |= HW_BUFFER_LEN_SET(firstpagelen);
        pbeacon_s->BufferInfo.MPDUBufAddr =
                    (unsigned int)(SYS_TYPE)((struct TxDescPage*)(SYS_TYPE)BcnAddr)->txdata;
        pbeacon_s->BufferInfo.MPDUBufNext =
                    (unsigned int)(SYS_TYPE)(BcnAddr + PAGE_LEN);

        /* other page */
        other_page->BufferInfo.MPDUBufFlag = 0;
        other_page->BufferInfo.MPDUBufFlag = HW_LAST_MPDUBUF_FLAG
            |HW_FIRST_AGG_FLAG | HW_LAST_AGG_FLAG;
        other_page->BufferInfo.MPDUBufFlag |= HW_MPDU_LEN_SET(len);
        other_page->BufferInfo.MPDUBufFlag |= HW_BUFFER_LEN_SET(0);
        other_page->BufferInfo.MPDUBufNext = 0;
        other_page->BufferInfo.MPDUBufAddr =
                    (unsigned int)(SYS_TYPE)((struct OtherTxPage*)(SYS_TYPE)(BcnAddr + PAGE_LEN))->txdata;
        otherpagelen += (sizeof(struct HW_TxBufferInfo) + FCS_LEN);
        pbeacon_s->TxPriv.PageNum = 2;
    }

    /* build tx vector */
    TxVector_Bit = &pbeacon_s->TxVector;
    TxVector_Bit->tv_reserved0 = 0;
    TxVector_Bit->tv_fw_duration_valid = 0;
    TxVector_Bit->tv_rty_flag = 0;
    TxVector_Bit->tv_ack_policy = NOACK;
    TxVector_Bit->tv_single_ampdu = 0;
    TxVector_Bit->tv_wnet_vif_id = wnet_vif_id;
    TxVector_Bit->tv_ack_to_en = 0;
    TxVector_Bit->tv_ack_timeout = 0;

    TxVector_Bit->tv_ht.htbit.tv_format = hw_rate2tv_format(Rate);
    TxVector_Bit->tv_ht.htbit.tv_nonht_mod  = (hal_tx_desc_nonht_mode(Rate, Flag));
    TxVector_Bit->tv_ht.htbit.tv_tx_pwr = hal_tx_desc_get_power(Rate);
    TxVector_Bit->tv_ht.htbit.tv_tx_rate = Rate;
    TxVector_Bit->tv_ht.htbit.tv_Channel_BW = Flag;
    TxVector_Bit->tv_ht.htbit.tv_preamble_type = wifi_conf_mib.dot11PreambleType;
    TxVector_Bit->tv_ht.htbit.tv_GI_type = 0;
    TxVector_Bit->tv_ht.htbit.tv_antenna_set = 0;
    TxVector_Bit->tv_ht.htbit.tv_cfend_flag = 0;
    TxVector_Bit->tv_ht.htbit.tv_bar_flag = 0;
    TxVector_Bit->tv_ht.htbit.tv_beamformed = 0;
    TxVector_Bit->tv_ht.htbit.tv_rts_flag = 0;
    TxVector_Bit->tv_ht.htbit.tv_cts_flag = 0;

    TxVector_Bit->tv_L_length = hal_tx_desc_get_len(Rate, len + FCS_LEN, 0, Flag, 0);
    TxVector_Bit->tv_sounding = 0;
    TxVector_Bit->tv_ampdu_flag = 0;
    TxVector_Bit->tv_length = len + FCS_LEN;
    TxVector_Bit->tv_STBC = 0;
    TxVector_Bit->tv_FEC_coding = 0;
    TxVector_Bit->tv_num_exten_ss = 0;
    TxVector_Bit->tv_num_tx= 1;
    TxVector_Bit->tv_expansion_mat_type = 1;
    TxVector_Bit->tv_no_sig_extn = ((Rate>WIFI_11B_11M)?0:1);
    TxVector_Bit->tv_SequenceNum = 0;
    TxVector_Bit->tv_FrameControl = *(unsigned short *)pBeacon;
    TxVector_Bit->tv_sq_valid = 0;
    TxVector_Bit->tv_fc_valid = 1;
    TxVector_Bit->tv_du_valid = 0;
    TxVector_Bit->tv_beacon_flag = 1;
    TxVector_Bit->tv_htc_modify =0;
    TxVector_Bit->tv_txcsum_enbale = 1;
    TxVector_Bit->tv_encrypted_disable = 0;
    TxVector_Bit->tv_tkip_mic_enable = 1;
    TxVector_Bit->tv_dyn_bw_nonht = 0;
    TxVector_Bit->tv_txop_ps_not = 0;
    TxVector_Bit->tv_usr_postion = 0;
    TxVector_Bit->tv_group_id = 0;
    TxVector_Bit->tv_partial_id = 0;
    TxVector_Bit->tv_num_users = 0;
    TxVector_Bit->tv_num_sts = 0;
    TxVector_Bit->tv_ch_bw_nonht = Flag;

    TxVector_Bit->tv_MPDUInfoAddr  = BcnAddr;
    TxVector_Bit->tv_tid_num = 0;
    TxVector_Bit->tv_control_wrapper = 0;
    TxVector_Bit->tv_llength_hw_calc = 0;
    TxVector_Bit->tv_tid_num = 0;
    TxVector_Bit->tv_control_wrapper = 0;
    TxVector_Bit->tv_llength_hw_calc = 0;

    /* build firmware private data */
    pbeacon_s->TxPriv.TID = QUEUE_BEACON;
    pbeacon_s->TxPriv.StaId = 1;
    pbeacon_s->TxPriv.Flag = WIFI_IS_Group|WIFI_IS_NOACK|(Flag << 8);
    pbeacon_s->TxPriv.vid = wnet_vif_id;
    pbeacon_s->TxPriv.TxCurrentRate = Rate;
    pbeacon_s->TxPriv.AggrNum = 1;
    pbeacon_s->TxPriv.AggrLen = len + FCS_LEN;
    pbeacon_s->TxPriv.txretrynum = 0;
    pbeacon_s->TxPriv.txsretrynum = 0;
    pbeacon_s->TxPriv.pTxInfo = 0;
    pbeacon_s->TxPriv.Flag2 = TX_DESCRIPTER_NOT_FREE|TX_DESCRIPTER_BEACON;
    pbeacon_s->TxPriv.FrameExchDur = 30;

    if (otherpagelen == 0)
    {
        hif->hif_ops.hi_write_sram((unsigned char*)(SYS_TYPE)buffer,
            (unsigned char*)(SYS_TYPE)BcnAddr, total_valid_len);
    }
    else
    {
        hif->hif_ops.hi_write_sram((unsigned char*)(SYS_TYPE)buffer,
            (unsigned char*)(SYS_TYPE)BcnAddr, PAGE_LEN);
        hif->hif_ops.hi_write_sram((unsigned char*)(SYS_TYPE)(buffer + PAGE_LEN),
            (unsigned char*)(SYS_TYPE)(BcnAddr + PAGE_LEN), otherpagelen);
    }
    return 1;
}


unsigned int phy_init_hmac(unsigned char wnet_vif_id)
{
    //clear all key
    phy_set_param_cmd(Reset_Key_Cmd,wnet_vif_id,ALL_KEY_RST);
    //life time
    phy_set_param_cmd(MaxTxLifetime_Cmd,0,512);
    pr_debug("%s(%d)\n", __func__, __LINE__);
    return 1;
}

//vmac disconnect to ap
unsigned int  phy_vmac_disconnect(unsigned char wnet_vif_id)
{
    struct hal_private *hal_priv = hal_get_priv();
    // trun off beacon send
    phy_set_param_cmd(Beacon_Enable_Cmd,wnet_vif_id,DISABLE_BEACON);
#if defined (HAL_FPGA_VER)
    hal_priv->sts_dis_bcn[wnet_vif_id]++;
#endif
    return 1;
}

unsigned int phy_update_bcn_intvl(unsigned char wnet_vif_id,unsigned short BcnInterval)
{
    PRINT("phy_update_bcn_intvl  0x%x \n",BcnInterval);
    // Make sure that BcnInterval  > ATIMWnd >= 0
    if (BcnInterval < 10)
    {
        BcnInterval = 10;
    }
    phy_set_param_cmd( BCNInterval_Cmd, wnet_vif_id, BcnInterval);
    return 1;
}

unsigned int phy_unregister_sta_id(unsigned char wnet_vif_id,unsigned short StaAid)
{
    PRINT("phy_unregister_sta_id StaAid %d\n",StaAid );
    if((StaAid >0) && (StaAid<= WIFI_MAX_STA))
    {
        phy_set_param_cmd(UnRegStaID_CMD,wnet_vif_id, (unsigned int)StaAid);
    }
    else
    {
        PRINT("WARNING!!phy_unregister_sta_id StaAid %d ERROR\n",StaAid );
    }
    return 1;
}

unsigned int phy_unregister_all_sta_id(unsigned char wnet_vif_id)
{
    phy_set_param_cmd(UnRegAllStaID_CMD,  wnet_vif_id,0);
    return 1;
}

unsigned int phy_switch_chan(unsigned short channel, unsigned char bw, unsigned char restore)
{
    struct hw_interface* hif =hif_get_hw_interface();
    // fix me, it works,but why len = sizeof(unsigned int) for read unsigned short channel

    hif->hif_ops.hi_write_word(RG_MAC_CHANNEL_INDEX, channel);
    return 1;
}

unsigned int phy_set_rf_chan(struct hal_channel *hchan, unsigned char flag, unsigned char vid, unsigned char opmode)
{
    struct hal_private *halPriv = hal_get_priv();

#ifndef FW_RF_CALIBRATION
    phy_switch_chan(hchan->cchan_num, hchan->chan_bw, flag);

#else
    struct PHY_PRIMARY_CHANNEL_BIT primary_chan;
    struct Channel_Switch channel_switch = {0};

    channel_switch.Cmd = CHANNEL_SWITCH_CMD;
    channel_switch.channel = hchan->cchan_num;
    channel_switch.bw = hchan->chan_bw;
    channel_switch.flag = flag;
    channel_switch.vid = vid;
    channel_switch.rssi = 0;
    channel_switch.res[0] = (opmode == WIFINET_M_HOSTAP); //bit0 : 1 - softap mode

    primary_chan.rf_fs = RF_SMP_160;
    primary_chan.ap_bw = hchan->chan_bw;

    switch (hchan->pchan_num - hchan->cchan_num)
    {
        case 0:
            primary_chan.def_pri_ch = SW_COFF_0;
            break;
        case 2:
            primary_chan.def_pri_ch = SW_COFF_U10M;
            break;
        case 6:
            primary_chan.def_pri_ch = SW_COFF_U30M;
            break;
         case -2:
            primary_chan.def_pri_ch = SW_COFF_L10M;
            break;
         case -6:
            primary_chan.def_pri_ch = SW_COFF_L30M;
            break;
        default:
            primary_chan.def_pri_ch = SW_COFF_0;
            ERROR_DEBUG_OUT("channel relation err!!!\n");
            break;
    }

    channel_switch.pri_chan = *(unsigned int *)&primary_chan;

    if ((flag & CHANNEL_CONNECT_FLAG) != 0)
    {
        hal_dpd_memory_download();
    }

    //pr_debug("%s:%d, channel %d, bw %d, flag %d \n", __func__, __LINE__, channel, bw, flag);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&channel_switch, sizeof(struct Channel_Switch));
    HAL_END_LOCK();
#endif

    halPriv->sta_con_msg.channel = hchan->cchan_num;
    return 1;
}

unsigned int phy_set_cam_mode(unsigned char wnet_vif_id,unsigned char OpMode)
{
    PRINT("phy_set_cam_mode %d \n",OpMode);
    wifi_conf_mib.dot11CamMode = OpMode;
    phy_set_param_cmd( CAM_Mode_Cmd,wnet_vif_id, OpMode);
    return 1;
}

unsigned int phy_set_dhcp(unsigned char wnet_vif_id, unsigned int ip)
{
    phy_set_param_cmd(dhcp_start_cmd,wnet_vif_id, ip);
    return 1;
}

unsigned int phy_set_chan_support_type(struct hal_channel *chan)
{
#ifndef FW_RF_CALIBRATION
    unsigned int to_set = 0;
    struct PHY_PRIMARY_CHANNEL_BIT * to_change = (struct  PHY_PRIMARY_CHANNEL_BIT *)&to_set;
    struct hw_interface* hif = hif_get_hw_interface();

    to_set = hif->hif_ops.hi_read_word(RG_PHY_BW_REG);

    to_change->rf_fs = RF_SMP_160;
    to_change->ap_bw = chan->chan_bw;

    switch (chan->pchan_num - chan->cchan_num)
    {
        case 0:
            to_change->def_pri_ch = SW_COFF_0;
            break;
        case 2:
            to_change->def_pri_ch = SW_COFF_U10M;
            break;
        case 6:
            to_change->def_pri_ch = SW_COFF_U30M;
            break;
         case -2:
            to_change->def_pri_ch = SW_COFF_L10M;
            break;
         case -6:
            to_change->def_pri_ch = SW_COFF_L30M;
            break;
        default:
            to_change->def_pri_ch = SW_COFF_0;
            ERROR_DEBUG_OUT("channel relation err!!!\n");
            break;
    }


    hif->hif_ops.hi_write_word(RG_PHY_BW_REG, to_set);

    to_set = 0;
    to_set = hif->hif_ops.hi_read_word( RG_PHY_BW_REG);
    //pr_debug("%s(%d) check: reg_0x%x is 0x%x cchan_num %d, chan_bw %d, cchan %d\n", __func__, __LINE__,
        //RG_PHY_BW_REG, to_set, chan->cchan_num, chan->chan_bw, chan->cchan_num);
    return 1;
#else
    return 0;
#endif
}

unsigned int phy_set_chan_phy_type(int chbw)
{
    return 1;
}

unsigned int phy_set_mac_bssid(unsigned char wnet_vif_id,unsigned char * Bssid)
{
    struct hal_private *halPriv;
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int temp = 0;
    unsigned int addr_low =  RG_MAC_BSSIDLO_N(wnet_vif_id);
    unsigned int addr_high = RG_MAC_BSSIDHI_N(wnet_vif_id);

    halPriv = hal_get_priv();
    temp = Bssid[0] | ( Bssid[1] << 8 ) | ( Bssid[2] << 16 ) | ( Bssid[3] << 24 );
    hif->hif_ops.hi_write_word(addr_low,temp);

    temp = 0;
    temp = Bssid[4] | ( Bssid[5] << 8 );
    hif->hif_ops.hi_write_word(addr_high, temp);
    PRINT(" %s:wnet_vif_id= %d bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",__func__,
		wnet_vif_id, Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]);
    return 1;
}

unsigned int phy_set_mac_addr(unsigned char wnet_vif_id,unsigned char * MacAddr)
{
    struct hal_private *halPriv;
    unsigned int temp = 0;
    struct hw_interface* hif = hif_get_hw_interface();

    halPriv = hal_get_priv();
    /* BUP stores the first 4 bytes and last 2 bytes in two separate registers*/
    temp = MacAddr[0] | ( MacAddr[1] << 8 ) | ( MacAddr[2] << 16 ) | ( MacAddr[3] << 24 );
    hif->hif_ops.hi_write_word(RG_MAC_LOCAL_ADDRLN(wnet_vif_id), temp);

    temp = 0;
    temp = MacAddr[4] | ( MacAddr[5] << 8 );
    hif->hif_ops.hi_write_word(RG_MAC_LOCAL_ADDRHN(wnet_vif_id), temp);

    PRINT("%s-->%02x:%02x:%02x:%02x:%02x:%02x\n",__func__,
        MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    return 1;
}

unsigned int phy_debug( unsigned short len,unsigned int offset,unsigned char * buffer)
{
    struct phydebugcmd ebug = {0};
    ebug.Cmd = PHY_DEBUG;
    ebug.offset = offset;
    ebug.len = len;
    HAL_BEGIN_LOCK();
    hi_get_cmd((unsigned char *)&ebug,sizeof(struct phydebugcmd ));
    HAL_END_LOCK();
    memcpy(buffer,ebug.buf,ebug.len);
    return 0;
}

// reset key table
unsigned int phy_rst_all_key(unsigned char wnet_vif_id)
{
    struct hal_private * hal_priv =hal_get_priv();
    PRINT(" ++  %s \n",__FUNCTION__);
    if (hal_priv->bhalUKeySet[wnet_vif_id] || hal_priv->bhalMKeySet[wnet_vif_id] )
        phy_set_param_cmd(Reset_Key_Cmd,  wnet_vif_id,ALL_KEY_RST);
    hal_priv->bhalUKeySet[wnet_vif_id] =0;
    hal_priv->bhalMKeySet[wnet_vif_id] =0;
    return 1;
}

// reset key table
unsigned int phy_rst_ucast_key(unsigned char wnet_vif_id)
{
    struct hal_private * hal_priv =hal_get_priv();
    PRINT(" ++  %s \n",__FUNCTION__);
    if (hal_priv->bhalUKeySet[wnet_vif_id] )
        phy_set_param_cmd(Reset_Key_Cmd,  wnet_vif_id,ALL_UNIKEY_RST);
    hal_priv->bhalUKeySet[wnet_vif_id] =0;
    return 1;
}

// reset key table
unsigned int phy_rst_mcast_key(unsigned char wnet_vif_id)
{
    struct hal_private * hal_priv =hal_get_priv();
    PRINT(" ++  %s \n",__FUNCTION__);
    if (hal_priv->bhalMKeySet[wnet_vif_id] )
        phy_set_param_cmd(Reset_Key_Cmd, wnet_vif_id, MULTI_KEY_RST);
    hal_priv->bhalMKeySet[wnet_vif_id] =0;
    return 1;
}

//reset StaAid key
unsigned int phy_clr_key(unsigned char wnet_vif_id,unsigned short StaAid)
{
    PRINT(" ++  %s StaAid = %x\n",__FUNCTION__,StaAid);
    phy_set_param_cmd(Reset_Key_Cmd,wnet_vif_id, StaAid);
    return 1;
}

unsigned int phy_set_ucast_key(unsigned char wnet_vif_id,unsigned short StaAid,
    unsigned char *pMac, unsigned char *pKey, unsigned char keyLen, unsigned char encryType,
    unsigned char keyId)
{
    struct hal_private * hal_priv =hal_get_priv();
    //UniCastKeyCmd
    unsigned char       cmdbuf[68]= {0};
    struct UniCastKeyCmd *unitikey = (struct UniCastKeyCmd *)ALIGN_POINT(cmdbuf,4);

    unitikey->Cmd = UniCast_Key_Set_Cmd;
    unitikey->vid = wnet_vif_id;
    unitikey->StaId= StaAid;
    unitikey->KeyType =encryType;
    unitikey->KeyLen = keyLen;
    unitikey->KeyId= keyId;

    if(encryType== WIFI_WPI)
        unitikey->PNStep=2;
    else
        unitikey->PNStep=1;

    memcpy(unitikey->Key,pKey,keyLen);
    memcpy((void *)&(hal_priv->sta_con_msg.unitikey), (void *)unitikey, sizeof(struct UniCastKeyCmd));
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)unitikey, sizeof(struct UniCastKeyCmd) );
    HAL_END_LOCK();
    PRINT("phy_set_ucast_key wnet_vif_id %x encryType=%d StaAid %x \n",wnet_vif_id,encryType, StaAid);
    hal_urep_cnt_init(&hal_priv->uRepCnt[wnet_vif_id][StaAid],encryType);
    hal_pn_win_init(AML_UCAST_TYPE, wnet_vif_id);
    return 1;
}

unsigned int phy_set_mcast_key(unsigned char wnet_vif_id, unsigned char *pKey,
    unsigned char keyLen, unsigned int keyId,unsigned char encryType,unsigned char AuthRole)
{
    struct hal_private * hal_priv =hal_get_priv();

//Multicast_Key_Set_Cmd
    unsigned char       cmdbuf[68]= {0};
    struct MultiCastKeyCmd *multikey = (struct MultiCastKeyCmd *)ALIGN_POINT(cmdbuf,4);
    if (( encryType !=  WIFI_WEP64)
        &&(encryType != WIFI_WEP128)
        &&(encryType != WIFI_WEP256))
        keyLen = 32;
    PRINT("Multicast_Key_Set_Cmd wnet_vif_id %d encryType %x keyId %x\n",wnet_vif_id,encryType,keyId);
    multikey->Cmd = Multicast_Key_Set_Cmd;
    multikey->vid  = wnet_vif_id;
    multikey->AuthRole = AuthRole;
    multikey->KeyType =encryType;
    multikey->KeyLen = keyLen;
    multikey->KeyId= keyId;
    multikey->PNStep=1;
    memcpy(multikey->Key,pKey,keyLen);
    memcpy((void *)&(hal_priv->sta_con_msg.multikey), (void *)multikey, sizeof(struct MultiCastKeyCmd));
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)multikey, sizeof(struct MultiCastKeyCmd) );
    HAL_END_LOCK();
    hal_priv->bhalMKeySet[0] =1;
    hal_priv->bhalMKeySet[1] =1;
    hal_priv->bhalMKeySet[2] =1;
    hal_priv->bhalMKeySet[3] =1;
    hal_mrep_cnt_init(hal_priv,wnet_vif_id,encryType);
    hal_pn_win_init(AML_MCAST_TYPE, wnet_vif_id);
    return 1;
}

unsigned int phy_set_rekey_data(unsigned char wnet_vif_id, void *rekey_data, unsigned short StaAid)
{
    unsigned char cmdbuf[68]= {0};

    struct RekeyDataCmd *rekey_cmd = (struct RekeyDataCmd *)ALIGN_POINT(cmdbuf, 4);

    rekey_cmd->Cmd = Rekey_Data_Set_Cmd;
    rekey_cmd->vid = wnet_vif_id;
    rekey_cmd->StaId= StaAid;
    memcpy((void *)(&rekey_cmd->kek[0]), rekey_data, KEY_KEK_LENGTH);
    memcpy((void *)(&rekey_cmd->kck[0]), rekey_data + KEY_KEK_LENGTH, KEY_KEK_LENGTH);
    memcpy((void *)(&rekey_cmd->replay_counter[0]), rekey_data + 32, KEY_REPLAY_CTR_LENGTH);

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)rekey_cmd, sizeof(struct RekeyDataCmd));
    HAL_END_LOCK();

    PRINT("phy_set_rekey_data wnet_vif_id %x StaAid %x \n",wnet_vif_id, StaAid);
    return 1;
}

unsigned int phy_set_lretry_limit(unsigned int data)
{
    struct hal_private * hal_priv =hal_get_priv();

    DBG_ENTER();
    PRINT("************>set_long_retry_limit %d\n",data);
    hal_priv->sta_con_msg.lr_limit = data;
    phy_set_param_cmd(LongRetryLimit_Cmd,0,data);
    return 1;
}

unsigned int phy_set_sretry_limit(unsigned int data)
{
    struct hal_private * hal_priv =hal_get_priv();
    DBG_ENTER();
    PRINT("************>phy_set_sretry_limit %d\n",data);
    hal_priv->sta_con_msg.sr_limit = data;
    phy_set_param_cmd(ShortRetryLimit_Cmd,0,data);
    return 1;
}
unsigned int phy_set_rd_support(unsigned char wnet_vif_id, unsigned int data)
{
    phy_set_param_cmd(RD_SUPPORT_CMD, wnet_vif_id, data);
    return 1;
}

unsigned int phy_pwr_save_mode(unsigned char wnet_vif_id,unsigned int data)
{
#ifdef CONFIG_RTC_ENABLE
    Phy_WakeUp_RTC(wnet_vif_id, data);
    phy_set_param_cmd(Power_Save_Cmd,wnet_vif_id,data);
    Phy_Restore_Wakeup_Bit(data);
    PhySetFw2Rtc(wnet_vif_id,  data);
#else
    phy_set_param_cmd(Power_Save_Cmd,wnet_vif_id,data);
#endif
    return 1;
}

unsigned int phy_set_txlive_time(unsigned int  txlivetime)
{
    phy_set_param_cmd(MaxTxLifetime_Cmd,0,txlivetime);
    return 1;
}

void  phy_scan_cmd(unsigned int data)
{
    struct hal_private * hal_priv =hal_get_priv();

    hal_priv->sta_con_msg.scan = data & 0xff;
    phy_set_param_cmd(MAC_SCAN_CMD,0,data);
}

void phy_set_channel_rssi(unsigned char rssi)
{
    struct Channel_Switch channel_switch = {0};

    channel_switch.Cmd = CHANNEL_SWITCH_CMD;
    channel_switch.rssi = rssi;
    channel_switch.flag = 0;
    channel_switch.flag |= CHANNEL_RSSI_FLAG;

    if (g_initial_gain_change_disable) {
        return;
    }

    //DPRINTF(AML_DEBUG_WARNING, "%s channel rssi:%d\n", __func__, rssi);

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char*)&channel_switch, sizeof(struct Channel_Switch));
    HAL_END_LOCK();
}
unsigned int phy_set_slot_time(unsigned int slot)
{
    struct hal_private * hal_priv =hal_get_priv();
    unsigned int temp = 0;
    struct hw_interface* hif = hif_get_hw_interface();

    temp = hif->hif_ops.hi_read_word(RG_MAC_COUNT2);
    temp =( temp &(~SLOT_MASK) )|(slot<<16);
    hif->hif_ops.hi_write_word( RG_MAC_COUNT2, temp);
    hal_priv->sta_con_msg.slot = slot;
    return 0;
}


unsigned int phy_set_bcn_intvl(unsigned char vid,unsigned int bcninterval)
{
    phy_set_param_cmd(BCNInterval_Cmd,vid, bcninterval);
    return 0;
}

unsigned int phy_set_ac_param(unsigned char wnet_vif_id,unsigned char ac,unsigned char aifsn,
    unsigned char cwminmax,unsigned short txop)
{
    struct hal_private * hal_priv =hal_get_priv();
    unsigned int wme_acparam = aifsn|(cwminmax<<8)|(txop<<16);

    PRINT("%s-->wnet_vif_id:%d, aifsn:%d, cwminmax:%02x txop:%d\n",__func__, wnet_vif_id, aifsn, cwminmax, txop);

    if((cwminmax ==0)||(aifsn==0))
    {
        PRINT("phy_set_ac_param wnet_vif_id %d ERROR!\n",wnet_vif_id);
        return -1;
    }

    hal_priv->sta_con_msg.wme_acparam[ac].ac = ac;
    hal_priv->sta_con_msg.wme_acparam[ac].aifsn = aifsn;
    hal_priv->sta_con_msg.wme_acparam[ac].cwminmax = cwminmax;
    hal_priv->sta_con_msg.wme_acparam[ac].txop = txop;

    switch (ac)
    {
        case WME_AC_BE:
            phy_set_param_cmd(AC_BE_Param_Cmd,wnet_vif_id,wme_acparam);
            break;
        case WME_AC_BK:
            phy_set_param_cmd(AC_BK_Param_Cmd,wnet_vif_id,wme_acparam);
            break;
        case WME_AC_VI:
            phy_set_param_cmd(AC_VI_Param_Cmd,wnet_vif_id,wme_acparam);
            break;
        case WME_AC_VO:
            phy_set_param_cmd(AC_VO_Param_Cmd,wnet_vif_id,wme_acparam);
            break;
        default:
            PRINT(" phy_set_ac_param Err: queue_id=%d is fault \n",ac);
            break;
    }

    return 0;
}

int phy_set_p2p_opps_cwend_enable(unsigned char vid, unsigned char p2p_oppps_cw)
{
    return phy_set_param_cmd(P2P_OPPPS_CWEND_IRQ_ENABLE_CMD, vid, p2p_oppps_cw);
}

int phy_set_p2p_noa_enable(unsigned char vid, unsigned int duration, unsigned int interval,
    unsigned int starttime, unsigned char count, unsigned char flag)
{
    NoACmd cmd = {0};

    cmd.Cmd = P2P_NoA_ENABLE_CMD;
    cmd.vid = vid;
    cmd.count = count;
    cmd.flag = flag;
    cmd.duration = duration;
    cmd.interval = interval;
    cmd.starttime = starttime;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&cmd, sizeof(struct NoACmd));
    HAL_END_LOCK();

    return 0;
}

unsigned long long phy_get_tsf(unsigned char vid)
{
    return phy_get_param_cmd_ull(GET_TSF_CMD, vid);
}

static unsigned long long phy_get_pn(unsigned char vid)
{
    return phy_get_param_cmd_ull(GET_PN_CMD, vid);
}

unsigned int phy_get_rw_ptr(unsigned char vid)
{
    return phy_get_param_cmd_ul(GET_SDIO_PTR_CMD, vid);
}

//for beamform test
unsigned int phy_send_ndp_announcement(struct NDPAnncmntCmd ndp_anncmnt)
{
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&ndp_anncmnt, sizeof(struct NDPAnncmntCmd));
    HAL_END_LOCK();
    return 0;
}

unsigned int phy_send_null_data(struct NullDataCmd null_data, int len)
{
    null_data.Cmd = Send_NullData_Cmd;
    null_data.tv_l_length = hal_tx_desc_get_len(null_data.rate, len,
        0/*GI*/, null_data.bw/*bw*/, 0/*green mode*/);

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&null_data, sizeof(struct NullDataCmd));
    HAL_END_LOCK();
    return 0;
}

unsigned int phy_keep_alive(struct NullDataCmd null_data_param, int null_data_len,
    int enable, int period)
{
    struct KeepAliveCmd keep_alive_cmd;

    null_data_param.tv_l_length = hal_tx_desc_get_len(null_data_param.rate, null_data_len,
        0/*GI*/, null_data_param.bw/*bw*/, 0/*green mode*/);

    keep_alive_cmd.Cmd = Keep_Alive_Cmd;
    keep_alive_cmd.null_data_param = null_data_param;
    keep_alive_cmd.enable = enable;
    keep_alive_cmd.period = period;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&keep_alive_cmd, sizeof(struct KeepAliveCmd));
    HAL_END_LOCK();
    return 0;
}

unsigned int phy_set_beacon_miss(unsigned char vid, unsigned char enable, int period)
{
    struct BeaconMissCmd beacon_miss_cmd = {0};

    beacon_miss_cmd.Cmd = Beacon_Miss_Cmd;
    beacon_miss_cmd.enable = enable;
    beacon_miss_cmd.period = period;
    beacon_miss_cmd.vid = vid;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&beacon_miss_cmd, sizeof(struct BeaconMissCmd));
    HAL_END_LOCK();
    return 0;
}

unsigned int phy_set_vsdb(unsigned char vid, unsigned char enable)
{
    struct vsdb_cmd set_vsdb_cmd = {0};

    set_vsdb_cmd.Cmd = vsdb_set_cmd;
    set_vsdb_cmd.enable = enable;
    set_vsdb_cmd.vid = vid;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&set_vsdb_cmd, sizeof(struct vsdb_cmd));
    HAL_END_LOCK();
    return 0;
}

unsigned int phy_set_arp_agent(unsigned char vid, unsigned char enable, unsigned int ipv4,
        unsigned char * ipv6)
{
    struct ArpAgentCmd arp_agent_cmd = {0};

    arp_agent_cmd.Cmd = ARP_Agent_Cmd;
    arp_agent_cmd.enable = enable;
    arp_agent_cmd.vid = vid;
    arp_agent_cmd.ip_addr = ipv4;

    if (ipv6 != NULL)
        memcpy(arp_agent_cmd.ip6_addr, ipv6, IPV6_ADDR_BUF_LEN);

    pr_debug("%s:%d, set arp agent, enable %d \n", __func__, __LINE__, enable);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&arp_agent_cmd, sizeof(struct ArpAgentCmd));
    HAL_END_LOCK();

    return 0;
}

unsigned int phy_set_pattern(unsigned char vid, unsigned char offset, unsigned char len,
            unsigned char id, unsigned char *mask, unsigned char *pattern)
{
    struct AddPatternCmd add_pattern_cmd;
    int mask_len = 0;

    mask_len = ALIGN(len, WOW_PATTERN_MASK_SIZE) / WOW_PATTERN_MASK_SIZE;
    add_pattern_cmd.Cmd = Add_Pattern_Cmd;
    add_pattern_cmd.vid = vid;
    add_pattern_cmd.pattern_offset = offset;
    /*we just compare pattern with max pattern size */
    add_pattern_cmd.pattern_len = len > WOW_PATTERN_SIZE ? WOW_PATTERN_SIZE : len;
    add_pattern_cmd.pattern_id = id;
    memcpy(add_pattern_cmd.pattern, pattern, add_pattern_cmd.pattern_len);
    memcpy(add_pattern_cmd.mask, mask, mask_len);

    pr_debug("%s:%d, wow len %d, mask 0x%x, pattern 0x%lx\n", __func__, __LINE__, len,
        *(unsigned int *)mask, *(unsigned long*)pattern);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&add_pattern_cmd, sizeof(struct AddPatternCmd));
    HAL_END_LOCK();
    return 0;
}

int phy_set_suspend(unsigned char vid, unsigned char enable,
        unsigned char mode, unsigned int filters)
{
    struct SuspendCmd suspend_cmd = {0};
    struct hal_private * hal_priv = hal_get_priv();
    unsigned long long PN = 0;
    bool ret = false;
    int cnt = 0;
    unsigned int ptr = 0;

    suspend_cmd.Cmd = WoW_Enable_Cmd;
    suspend_cmd.vid = vid;
    suspend_cmd.enable = enable;

    if (enable == 1)
    {
        memcpy(suspend_cmd.PN, hal_priv->uRepCnt[vid][1].txPN[TX_UNICAST_REPCNT_ID], 8);
        /* wait for fw upload tx status */
        while (hal_priv->bhaltxdrop == 1)
        {
            msleep(20);
            cnt++;
            if (cnt > 10)
            {
                pr_debug("%s:%d, haltxdrop = 1 0x%lx, 0x%lx\n", __func__, __LINE__,
                    hal_priv->tx_frames_map[0], hal_priv->tx_frames_map[1]);
                break;
            }
        }
    }
    else
    {
        while ((enable == 0) && (atomic_read(&hal_priv->sdio_suspend_cnt) > 0))
        {
            msleep(20);
            cnt++;
            if (cnt > 10)
            {
                pr_debug("%s:%d, no resume cnt 0x%x\n", __func__, __LINE__,
                        atomic_read(&hal_priv->sdio_suspend_cnt));
                return -1;
            }
        }
        /* in case */
        if (hal_priv->bhaltxdrop == 1)
        {
            pr_debug("%s:%d, haltxdrop = 1 0x%lx, 0x%lx, pagenum %d \n", __func__, __LINE__,
                hal_priv->tx_frames_map[0], hal_priv->tx_frames_map[1], hal_priv->txPageFreeNum);
            hal_priv->bhaltxdrop = 0;
            hal_priv->tx_frames_map[0] = 0;
            hal_priv->tx_frames_map[1] = 0;
            hal_priv->txPageFreeNum = DEFAULT_TXPAGENUM;
        }
    }

    suspend_cmd.filters = filters;

    atomic_set(&hal_priv->sdio_suspend_cnt, 0);

    /* if last suspend fail,this time is resume,just return */
    if ((enable == 0) && (atomic_read(&hal_priv->drv_suspend_cnt) == 0))
    {
        ERROR_DEBUG_OUT("last suspend fail,don't need resume, just return -1\n");
        return -1;
    }

    /* wait for set driver sleep flag */
    while ((enable == 1) && (hal_priv->hal_drv_ps_status & HAL_DRV_IN_ACTIVE))
    {
        msleep(20);
        cnt++;
        if (cnt > 10)
        {
            //AML_OUTPUT("suspend hal_drv_ps_status=%d   time=%d\n",hal_priv->hal_drv_ps_status,cnt);
            break;
        }
    }

    POWER_BEGIN_LOCK();
    if ((enable == 1) && (hal_priv->powersave_init_flag == 0))
    {
        if ((hal_priv->hal_drv_ps_status & HAL_DRV_STS_MASK) != 0)
        {
            POWER_END_LOCK();
            atomic_set(&hal_priv->drv_suspend_cnt, 0);
            return -1;
        }
        else
        {
            // set driver sleep flag to protect power save flow
            hal_priv->hal_drv_ps_status |= HAL_DRV_IN_SLEEPING;
        }
    }

    if (mode == 0)
    {
        suspend_cmd.wow = 0;
        suspend_cmd.deepsleep = 0;
        hal_priv->hal_suspend_mode = HIF_SUSPEND_STATE_NONE;
        atomic_set(&hal_priv->drv_suspend_cnt, 0);
    }
    else if (mode == 1)
    {
        suspend_cmd.wow = 1;
        suspend_cmd.deepsleep = 0;
        hal_priv->hal_suspend_mode = HIF_SUSPEND_STATE_WOW;
    }
    else
    {
        suspend_cmd.deepsleep = 1;
        suspend_cmd.wow = 0;
        hal_priv->hal_suspend_mode = HIF_SUSPEND_STATE_DEEPSLEEP;
    }

    POWER_END_LOCK();

    if (enable == 0)
    {
        ptr = hal_priv->hal_ops.phy_get_rw_ptr(0);
        hal_priv->rx_host_offset = ((ptr >> 16) & 0xffff) * 4;
    }

    HAL_BEGIN_LOCK();
    ret = hi_set_cmd((unsigned char *)&suspend_cmd, sizeof(struct SuspendCmd));

    POWER_BEGIN_LOCK();
    if (ret && (enable == 1) && (hal_priv->powersave_init_flag == 0))
    {
        hal_priv->hal_fw_ps_status = HAL_FW_IN_SLEEP;
        atomic_set(&hal_priv->drv_suspend_cnt, 1);
    }
    else if (ret && (enable == 0) && (hal_priv->powersave_init_flag == 0))
    {
        hal_priv->hal_fw_ps_status = HAL_FW_IN_ACTIVE;
    }
    hal_priv->hal_drv_ps_status &= ~HAL_DRV_IN_SLEEPING;

    POWER_END_LOCK();
    HAL_END_LOCK();

    if (enable == 0)
    {
        struct hw_interface* hif = hif_get_hw_interface();
        unsigned int hw_txc_addr = 0;
        struct tx_complete_status *txcompletestatus = NULL;

        ptr = hal_priv->hal_ops.phy_get_rw_ptr(0);
        hal_priv->tx_page_offset = ptr & 0xffff;

        PN = phy_get_pn(vid);
        memcpy(hal_priv->uRepCnt[vid][1].txPN[TX_UNICAST_REPCNT_ID], &PN, 8);

        hw_txc_addr = hif->hw_config.txcompleteaddress;
        txcompletestatus = hal_priv->txcompletestatus;

        hif->hif_ops.hi_read_sram((unsigned char *)txcompletestatus,
            (unsigned char *)(SYS_TYPE)hw_txc_addr, (SYS_TYPE)sizeof(struct tx_complete_status));

        if (hal_priv->HalTxFrameDoneCounter != hal_priv->txcompletestatus->txdoneframecounter)
        {
            pr_debug("%s:%d, txdoneframecounter:%x, HalTxFrameDoneCounter:%x\n", __func__, __LINE__,
                hal_priv->txcompletestatus->txdoneframecounter, hal_priv->HalTxFrameDoneCounter);
        }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
        if ((hif->HiStatus.Tx_Free_num != hif->HiStatus.Tx_Send_num)
            || (hif->HiStatus.Tx_Done_num != hif->HiStatus.Tx_Send_num))
#else
        if ((atomic_read(&hif->HiStatus.Tx_Free_num) != atomic_read(&hif->HiStatus.Tx_Send_num))
            || (atomic_read(&hif->HiStatus.Tx_Done_num) != atomic_read(&hif->HiStatus.Tx_Send_num)))
#endif
        {
            pr_debug("%s:%d, free %d, done %d, send %d\n",  __func__, __LINE__,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
                hif->HiStatus.Tx_Free_num, hif->HiStatus.Tx_Done_num, hif->HiStatus.Tx_Send_num);
#else
                atomic_read(&hif->HiStatus.Tx_Free_num),
                atomic_read(&hif->HiStatus.Tx_Done_num),
                atomic_read(&hif->HiStatus.Tx_Send_num));
#endif
        }
        /* flush packetes when suspend and when resume restore initial value */
        hal_priv->HalTxFrameDoneCounter = hal_priv->txcompletestatus->txdoneframecounter;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0) && !defined (LINUX_PLATFORM)
        hif->HiStatus.Tx_Free_num = hif->HiStatus.Tx_Send_num;
        hif->HiStatus.Tx_Done_num = hif->HiStatus.Tx_Send_num;
#else
        atomic_set(&hif->HiStatus.Tx_Free_num, atomic_read(&hif->HiStatus.Tx_Send_num));
        atomic_set(&hif->HiStatus.Tx_Done_num, atomic_read(&hif->HiStatus.Tx_Send_num));
#endif
    }
    pr_debug("%s end, wow enable %d, mode %d, filter 0x%x, ret %d\n",
        ((enable == 1) ? "suspend" : "resume"), enable, mode, filters, ret);
    return 0;
}

void phy_set_bmfm_info(int wnet_vif_id, unsigned char *group_id,
        unsigned char * user_position, unsigned char feedback_type)
{
    struct Bmfm_Info_Cmd bmfm_info_cmd = {0};
#ifdef PROJECT_T9026
    struct hw_interface* hif =hif_get_hw_interface();

    if (group_id == NULL) {
        hif->hif_ops.hi_write_word(0x000001d0, 0x20000000);
        hif->hif_ops.hi_write_word(0x000001d4, 0x20000000);
        hif->hif_ops.hi_write_word(0x000001d8, 0x20000000);
        hif->hif_ops.hi_write_word(0x000001dC, 0x20000000);
        return;
    }
#else
    if (group_id == NULL) {
        ERROR_DEBUG_OUT("group id null\n");
        return;
    }
#endif

    bmfm_info_cmd.Cmd = BmfmInfo_Cmd;
    bmfm_info_cmd.vid = wnet_vif_id;
    bmfm_info_cmd.feedback_type = feedback_type;

    memcpy(bmfm_info_cmd.group_id, group_id, sizeof(bmfm_info_cmd.group_id));
    memcpy(bmfm_info_cmd.user_position, user_position, sizeof(bmfm_info_cmd.user_position));

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&bmfm_info_cmd, sizeof(struct Bmfm_Info_Cmd));
    HAL_END_LOCK();
}

unsigned int hal_dpd_memory_download_cmd(void)
{
    struct DPD_Memory_Download dpd_download;

    memset(&dpd_download, 0, sizeof(struct DPD_Memory_Download));
    dpd_download.Cmd = DPD_MEMORY_CMD;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&dpd_download, sizeof(struct DPD_Memory_Download));
    HAL_END_LOCK();

    return 0;
}

unsigned int phy_set_coexist_en( unsigned char enable)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_EN_CMD;
    coexist_cmd.coexist_enable = enable;

    pr_debug("%s:%d, phy_set_coexist_en, enable %d \n", __func__, __LINE__, enable);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;
}

unsigned int phy_set_coexist_max_miss_bcn( unsigned int miss_bcn_cnt)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));

    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_MAX_MISS_BCN_CNT;
    coexist_cmd.max_miss_bcn_cnt = miss_bcn_cnt;

    pr_debug("%s:%d, miss_bcn_cnt, %d \n", __func__, __LINE__, miss_bcn_cnt);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;
}

unsigned int phy_set_coexist_req_timeslice_timeout_value( unsigned int timeout_value)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_REQ_TIMEOUT;
    coexist_cmd.req_timeout_value = timeout_value;

    pr_debug("%s:%d, req_timeslice_timeout_value %d \n", __func__, __LINE__, timeout_value);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();
    return 0;


}



unsigned int phy_set_coexist_not_grant_weight( unsigned int not_grant_weight)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_NOT_GRANT_WEIGHT;
    coexist_cmd.not_grant_weight = not_grant_weight;

    pr_debug("%s:%d, not_grant_weight %d \n", __func__, __LINE__, not_grant_weight);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;


}


unsigned int phy_set_coexist_max_not_grant_cnt( unsigned int coexist_max_not_grant_cnt)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_MAX_NOT_GRANT_CNT;
    coexist_cmd.max_not_grant_cnt = coexist_max_not_grant_cnt;

    pr_debug("%s:%d, coexist_max_not_grant_cnt %d \n", __func__, __LINE__, coexist_max_not_grant_cnt);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;

}

/*
bit31-bit16 : minimal  priority
bit15:bit0: max priority
*/
unsigned int phy_set_coexist_scan_priority_range( unsigned int coexist_scan_priority_range)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_SCAN_PRIORITY_RANGE;
    coexist_cmd.scan_priority_range = coexist_scan_priority_range;

    pr_debug("%s:%d, coexist  scan min priority is %d,max priority is %d\n", __func__, __LINE__, coexist_scan_priority_range&0xffff,  coexist_scan_priority_range>>16);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;

}


/*
bit31-bit16 : minimal  priority
bit15:bit0: max priority
*/
unsigned int phy_set_coexist_be_bk_noqos_priority_range( unsigned int coexist_scan_priority_range)
{
    struct Coexist_Cmd coexist_cmd;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_BE_BK_NOQOS_PRI_RANGE;
    coexist_cmd.be_bk_no_qos_priority_range = coexist_scan_priority_range;

    pr_debug("%s:%d, coexist be/bk/noqos  min priority is %d,max priority is %d\n", __func__, __LINE__, coexist_scan_priority_range&0xffff,  coexist_scan_priority_range>>16);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;

}


unsigned int phy_coexist_config(const void *data, int data_len)
{
    struct Coexist_Cmd coexist_cmd;
    unsigned char *opt_data = (unsigned char *)data;

    memset(&coexist_cmd, 0 , sizeof( struct Coexist_Cmd));
    coexist_cmd.Cmd = COEXIST_CMD;
    coexist_cmd.coexist_id_bitmap = COEXIST_PARAM_CMD_CONFIG;
    if ((data_len - 1) < sizeof(coexist_cmd.reserve1))
    {
        memcpy(&(coexist_cmd.reserve1), opt_data + 1, data_len - 1);
    }
    else
    {
        ERROR_DEBUG_OUT("coex cmd len error\n");
        return -1;
    }

    pr_debug("%s:%d, coex cmd\n", __func__, __LINE__);
    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&coexist_cmd, sizeof(struct Coexist_Cmd));
    HAL_END_LOCK();

    return 0;

}

unsigned int phy_interface_enable(unsigned char enable, unsigned char vid)
{
#if 0
    struct hw_interface* hif = hif_get_hw_interface();
    struct hal_private *hal_priv = hal_get_priv();
    unsigned int reg_data = 0;

    if (enable == 1)
    {
        if (hal_priv->bRfInit == 0)
            return 0;

        /* 0x00a08010 */
        reg_data = hif->hif_ops.hi_read_word(RG_AGC_EN);
        hif->hif_ops.hi_write_word(RG_AGC_EN, reg_data & ~BIT(28));

        phy_set_coexist_en(enable);
    }
    else
    {
        phy_set_coexist_en(enable);

        reg_data = hif->hif_ops.hi_read_word(RG_AGC_EN);
        hif->hif_ops.hi_write_word(RG_AGC_EN, reg_data | BIT(28));
    }
    reg_data = hif->hif_ops.hi_read_word(RG_AGC_EN);
    pr_debug("%s:%d, enable %d, agc 0x%x \n", __func__, __LINE__, enable, reg_data);
#endif
    struct Phy_Interface_Param phy_interface;
    memset(&phy_interface, 0, sizeof(struct Phy_Interface_Param));

    phy_interface.Cmd = PHY_INTERFACE_CMD;
    phy_interface.vid = vid;
    phy_interface.interface_enable = enable;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&phy_interface, sizeof(struct Phy_Interface_Param));
    HAL_END_LOCK();
    pr_debug("%s:%d, vid %d, enable %d\n", __func__, __LINE__, vid, enable);

    return 0;
}

unsigned char print_type = 0;
unsigned int hal_set_fwlog_cmd(unsigned char mode)
{
    struct hw_interface* hif = hif_get_hw_interface();
    struct Fwlog_Mode_Control fwlog_mode;
    memset(&fwlog_mode, 0, sizeof(struct Fwlog_Mode_Control));
    pr_debug("%s:%d, mode %d \n", __func__, __LINE__, mode);

    fwlog_mode.Cmd = FWLOG_MODE_CMD;
    if (mode == 0)
    {
        fwlog_mode.mode = 0;
        /* reset ram share */
        hif->hif_ops.hi_write_word(0x00a0d0e4, 0x0000007f);
    }
    else
    {
        fwlog_mode.mode = mode;
        if (mode == 1)
        {
            /* set ram share */
            hif->hif_ops.hi_write_word(0x00a0d0e4, 0x8000007f);
            print_type = 0;
        }
        else if (mode == 3)
        {
            hal_get_fwlog();
        }
        else if (mode == 4) //open auto print
        {
            print_type = 1;
        }
        else if (mode == 5) //close auto print
        {
            print_type = 0;
        }
    }

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&fwlog_mode, sizeof(struct Fwlog_Mode_Control));
    HAL_END_LOCK();

    return 0;
}

static unsigned int process_cali_content(char *varbuf, unsigned int len)
{
    char *dp;
    bool findNewline;
    int column;
    unsigned int buf_len, n;
    unsigned int pad = 0;

    dp = varbuf;
    findNewline = false;
    column = 0;

    for (n = 0; n < len; n++) {
        if (varbuf[n] == '\r')
            continue;

        if (findNewline && varbuf[n] != '\n')
            continue;
        findNewline = false;
        if (varbuf[n] == '#') {
            findNewline = true;
            continue;
        }
        if (varbuf[n] == '\n') {
            if (column == 0)
                continue;
            *dp++ = 0;
            column = 0;
            continue;
        }
        *dp++ = varbuf[n];
        column++;
    }
    buf_len = (unsigned int)(dp - varbuf);
    if (buf_len % 4) {
        pad = 4 - buf_len % 4;
        if (pad && (buf_len + pad <= len)) {
            buf_len += pad;
        }
    }

    while (dp < varbuf + n)
        *dp++ = 0;

    return buf_len;
}

unsigned char get_s8_item(char *varbuf, int len, char *item, char *item_value)
{
    unsigned int n;
    char tmpbuf[20];
    char *p = item_value;
    int ret = 0;
    unsigned int pos = 0;
    unsigned int index = 0;

    while (pos  < len) {
        index = pos;
        ret = 0;

        while ((varbuf[pos] != 0) && (varbuf[pos] != '=')) {
            if (((pos - index) >= strlen(item)) || (varbuf[pos] != item[pos - index])) {
                ret = 1;
                break;
            }
            else {
                pos++;
            }
        }

        pos++;

        if ((ret == 0) && (strlen(item) == pos - index - 1)) {
            do {
                memset(tmpbuf, 0, sizeof(tmpbuf));
                n = 0;
                while ((varbuf[pos] != 0) && (varbuf[pos] != ',') && (pos < len))
                    tmpbuf[n++] = varbuf[pos++];

                *p++ = (char)simple_strtol(tmpbuf, NULL, 0);
            }
            while (varbuf[pos++] == ',');

            return 0;
        }
    }

    return 1;
}

unsigned char get_s16_item(char *varbuf, int len, char *item, short *item_value)
{
    unsigned int n;
    char tmpbuf[60];
    short *p = item_value;
    int ret = 0;
    unsigned int pos = 0;
    unsigned int index = 0;

    while (pos  < len) {
        index = pos;
        ret = 0;

        while ((varbuf[pos] != 0) && (varbuf[pos] != '=')) {
            if (((pos - index) >= strlen(item)) || (varbuf[pos] != item[pos - index])) {
                ret = 1;
                break;
            }
            else {
                pos++;
            }
        }

        pos++;

        if ((ret == 0) && (strlen(item) == pos - index - 1)) {
            do {
                memset(tmpbuf, 0, sizeof(tmpbuf));
                n = 0;
                while ((varbuf[pos] != 0) && (varbuf[pos] != ',') && (pos < len))
                    tmpbuf[n++] = varbuf[pos++];

                *p++ = (short)simple_strtol(tmpbuf, NULL, 0);
            }
            while (varbuf[pos++] == ',');

            return 0;
        }
    }

    return 1;
}

unsigned char get_s32_item(char *varbuf, int len, char *item, unsigned int *item_value)
{
    unsigned int n;
    char tmpbuf[120];
    unsigned int *p = item_value;
    int ret = 0;
    unsigned int pos = 0;
    unsigned int index = 0;

    while (pos  < len) {
        index = pos;
        ret = 0;

        while ((varbuf[pos] != 0) && (varbuf[pos] != '=')) {
            if (((pos - index) >= strlen(item)) || (varbuf[pos] != item[pos - index])) {
                ret = 1;
                break;
            }
            else {
                pos++;
            }
        }

        pos++;

        if ((ret == 0) && (strlen(item) == pos - index - 1)) {
            do {
                memset(tmpbuf, 0, sizeof(tmpbuf));
                n = 0;
                while ((varbuf[pos] != 0) && (varbuf[pos] != ',') && (pos < len))
                    tmpbuf[n++] = varbuf[pos++];

                *p++ = (unsigned int)simple_strtol(tmpbuf, NULL, 0);
            }
            while (varbuf[pos++] == ',');

            return 0;
        }
    }

    return 1;
}

static unsigned char parse_tx_power_coefficient(char *varbuf, int len, char str_pwr_coefficient[])
{
    unsigned short pwr_coefficient[57];
    memset(pwr_coefficient,0,sizeof(pwr_coefficient));

    get_s16_item(varbuf, len, str_pwr_coefficient, &pwr_coefficient[0]);

    if (pwr_coefficient[0]) {
        if (memcmp(str_pwr_coefficient,"ce_pwr_coefficient",strlen(str_pwr_coefficient)) == 0) {
            update_tx_power_coefficient_plan(TX_POWER_CE, pwr_coefficient);
        }

        if (memcmp(str_pwr_coefficient,"fcc_pwr_coefficient",strlen(str_pwr_coefficient)) == 0) {
            update_tx_power_coefficient_plan(TX_POWER_FCC, pwr_coefficient);
        }

        if (memcmp(str_pwr_coefficient,"arib_pwr_coefficient",strlen(str_pwr_coefficient)) == 0) {
        update_tx_power_coefficient_plan(TX_POWER_ARIB, pwr_coefficient);
        }

        if (memcmp(str_pwr_coefficient,"srrc_pwr_coefficient",strlen(str_pwr_coefficient)) == 0) {
        update_tx_power_coefficient_plan(TX_POWER_SRRC, pwr_coefficient);
        }

        if (memcmp(str_pwr_coefficient,"anatel_pwr_coefficient",strlen(str_pwr_coefficient)) == 0) {
        update_tx_power_coefficient_plan(TX_POWER_ANATEL, pwr_coefficient);
        }
    }

    return 0;
}

static unsigned char parse_tx_power_band(char *varbuf, int len, char str_pwr_plan[])
{
    unsigned short band_pwr_table[4];
    memset(band_pwr_table,0,sizeof(band_pwr_table));

    get_s16_item(varbuf, len, str_pwr_plan, &band_pwr_table[0]);

    pr_debug("======>>>>>>%s: %d\n",str_pwr_plan, band_pwr_table[3]);

    if (memcmp(str_pwr_plan,"ce_band_pwr_tbl",strlen(str_pwr_plan)) == 0) {
    update_tx_power_band(TX_POWER_CE, band_pwr_table);
    }

    return 0;
}

static unsigned char parse_cali_param(char *varbuf, int len, struct Cali_Param *cali_param)
{
    unsigned short platform_verid = 0; // default: 0
    unsigned short cali_config = 0;
    unsigned int version = 0;

    get_s32_item(varbuf, len, "version", &version);
    get_s16_item(varbuf, len, "cali_config", &cali_config);
    get_s8_item(varbuf, len, "freq_offset", &cali_param->freq_offset);
    get_s8_item(varbuf, len, "htemp_freq_offset", &cali_param->htemp_freq_offset);
    get_s8_item(varbuf, len, "cca_ed_det", &cali_param->cca_ed_det);
    get_s8_item(varbuf, len, "tssi_2g_offset", &cali_param->tssi_2g_offset);
    get_s8_item(varbuf, len, "tssi_5g_offset_5200", &cali_param->tssi_5g_offset[0]);
    get_s8_item(varbuf, len, "tssi_5g_offset_5400", &cali_param->tssi_5g_offset[1]);
    get_s8_item(varbuf, len, "tssi_5g_offset_5600", &cali_param->tssi_5g_offset[2]);
    get_s8_item(varbuf, len, "tssi_5g_offset_5800", &cali_param->tssi_5g_offset[3]);
    get_s8_item(varbuf, len, "wf2g_spur_rmen", &cali_param->wf2g_spur_rmen);
    get_s16_item(varbuf, len, "spur_freq", &cali_param->spur_freq);
    get_s8_item(varbuf, len, "rf_count", &cali_param->rf_num);
    get_s8_item(varbuf, len, "wftx_pwrtbl_en", &cali_param->wftx_pwrtbl_en);
    get_s8_item(varbuf, len, "wftx_tssi_ratio_limit", (char *)&cali_param->txtssi_ratio_limit);
    get_s8_item(varbuf, len, "digital_code_gain_limit", (char *)&cali_param->digital_gain_limit);
    get_s8_item(varbuf, len, "wftx_power_change_disable", &g_tx_power_change_disable);
    get_s8_item(varbuf, len, "initial_gain_change_disable", &g_initial_gain_change_disable);

    cali_param->version = version;
    cali_param->cali_config = cali_config;

    g_wftx_pwrtbl_en = cali_param->wftx_pwrtbl_en;

    if (g_wftx_pwrtbl_en != 2) {
        tpc_mode = 1;
    } else {
        tpc_mode = 2;
    }

    /* maybe get from txt */
#ifdef HAL_FPGA_VER
    platform_verid = aml_wifi_get_platform_verid();
#endif
    cali_param->platform_versionid = platform_verid;

    pr_info("======>>>>>> version = %ld\n", cali_param->version);
    pr_info("======>>>>>> cali_config = %d\n", cali_param->cali_config);
    pr_info("======>>>>>> freq_offset = %d\n", cali_param->freq_offset);
    pr_info("======>>>>>> htemp_freq_offset = %d\n", cali_param->htemp_freq_offset);
    pr_info("======>>>>>> cca_ed_det = %d\n", cali_param->cca_ed_det);
    pr_info("======>>>>>> tssi_2g_offset = 0x%x\n", cali_param->tssi_2g_offset);
    pr_info("======>>>>>> tssi_5g_offset_5200 = 0x%x\n", cali_param->tssi_5g_offset[0]);
    pr_info("======>>>>>> tssi_5g_offset_5400 = 0x%x\n", cali_param->tssi_5g_offset[1]);
    pr_info("======>>>>>> tssi_5g_offset_5600 = 0x%x\n", cali_param->tssi_5g_offset[2]);
    pr_info("======>>>>>> tssi_5g_offset_5800 = 0x%x\n", cali_param->tssi_5g_offset[3]);
    pr_info("======>>>>>> wf2g_spur_rmen = %d\n", cali_param->wf2g_spur_rmen);
    pr_info("======>>>>>> spur_freq = %d\n", cali_param->spur_freq);
    pr_info("======>>>>>> rf_count = %d\n", cali_param->rf_num);
    pr_info("======>>>>>> wftx_pwrtbl_en = %d\n", cali_param->wftx_pwrtbl_en);
    pr_info("======>>>>>> platform_versionid = %d\n", cali_param->platform_versionid);
    pr_info("======>>>>>> wftx_power_change_disable = %d\n", g_tx_power_change_disable);
    pr_info("======>>>>>> initial_gain_change_disable = %d\n", g_initial_gain_change_disable);
    pr_info("======>>>>>> digital gain = %s min_2g:0x%x max_2g:0x%x min_5g:0x%x max_5g:0x%x\n",
            (cali_param->digital_gain_limit.enable == 1 ? "enable" : "disable"),
            cali_param->digital_gain_limit.min_2g, cali_param->digital_gain_limit.max_2g,
            cali_param->digital_gain_limit.min_5g, cali_param->digital_gain_limit.max_5g);

    parse_tx_power_band(varbuf, len, "ce_band_pwr_tbl");
    parse_tx_power_coefficient(varbuf, len, "ce_pwr_coefficient");
    parse_tx_power_coefficient(varbuf, len, "fcc_pwr_coefficient");
    parse_tx_power_coefficient(varbuf, len, "arib_pwr_coefficient");
    parse_tx_power_coefficient(varbuf, len, "srrc_pwr_coefficient");
    parse_tx_power_coefficient(varbuf, len, "anatel_pwr_coefficient");

    hal_cfg_txpwr_cffc_param_init(0);

    return 0;
}

static unsigned char set_tx_power_param_default(struct WF2G_Txpwr_Param *wf2g_txpwr_param, struct WF5G_Txpwr_Param *wf5g_txpwr_param)
{
    unsigned char wf2g_pwr_tbl_dft[2][16] = {{0xBA,0xBA,0xCB,0xCB,0xB5,0xB5,0xC0,0xB5,0xA1,0xA1,0x84,0x7C,0x6C,0x6C,0x63,0x58},
                                                                  {0xBA,0xBA,0xCB,0xCB,0xB5,0xB5,0xA6,0xA1,0x94,0x94,0x7C,0x75,0x6C,0x66,0x5D,0x58}};
    unsigned char wf5g_pwr_tbl_dft[3][16] = {{0xBA,0xBA,0x84,0x84,0x75,0x75,0x84,0x7C,0x6C,0x6C,0x84,0x7C,0x6C,0x6C,0x63,0x58},
                                                                  {0xBA,0xBA,0x84,0x84,0x75,0x75,0x7C,0x75,0x6C,0x6C,0x7C,0x75,0x6C,0x66,0x5D,0x58},
                                                                  {0xBA,0xBA,0x84,0x84,0x75,0x75,0x7C,0x75,0x6C,0x6C,0x6F,0x6F,0x60,0x60,0x60,0x58}};

    memcpy(&wf2g_txpwr_param->wf2g_pwr_tbl, wf2g_pwr_tbl_dft, sizeof(wf2g_txpwr_param->wf2g_pwr_tbl));
    memcpy(&wf5g_txpwr_param->wf5g_pwr_tbl, wf5g_pwr_tbl_dft, sizeof(wf5g_txpwr_param->wf5g_pwr_tbl));
    return 0;
}
static unsigned char parse_tx_power_param(char *varbuf, int len, struct WF2G_Txpwr_Param *wf2g_txpwr_param,
                                                           struct WF5G_Txpwr_Param *wf5g_txpwr_param)
{
    get_s8_item(varbuf, len, "wf2g_20M_pwr_tbl", &wf2g_txpwr_param->wf2g_pwr_tbl[0][0]);
    get_s8_item(varbuf, len, "wf2g_40M_pwr_tbl", &wf2g_txpwr_param->wf2g_pwr_tbl[1][0]);
    get_s8_item(varbuf, len, "wf5g_20M_pwr_tbl", &wf5g_txpwr_param->wf5g_pwr_tbl[0][0]);
    get_s8_item(varbuf, len, "wf5g_40M_pwr_tbl", &wf5g_txpwr_param->wf5g_pwr_tbl[1][0]);
    get_s8_item(varbuf, len, "wf5g_80M_pwr_tbl", &wf5g_txpwr_param->wf5g_pwr_tbl[2][0]);

    memcpy(&g_wf2g_txpwr_param.wf2g_pwr_tbl, wf2g_txpwr_param->wf2g_pwr_tbl, sizeof(wf2g_txpwr_param->wf2g_pwr_tbl));
    memcpy(&g_wf5g_txpwr_param.wf5g_pwr_tbl, wf5g_txpwr_param->wf5g_pwr_tbl, sizeof(wf5g_txpwr_param->wf5g_pwr_tbl));

    pr_debug("======>>>>>> %s ===>>> aml_wifi_rf txt => 2g 20/40 5g 20/40/80\n", __func__);

    return 0;
}

static unsigned char set_tx_power_param_enhance(struct WF2G_Txpwr_Param *wf2g_txpwr_param, struct WF5G_Txpwr_Param *wf5g_txpwr_param)
{
    unsigned char wf2g_pwr_tbl_dft[2][16] = {{0xea,0xea,0xea,0xb6,0x9e,0x7e,0xea,0xae,0x9b,0x78,0x84,0x7C,0x6C,0x6C,0x63,0x58},
                                                                  {0xBA,0xBA,0xCB,0xCB,0xB5,0xB5,0xda,0xae,0x9b,0x78,0x7C,0x75,0x6C,0x66,0x5D,0x58}};
    unsigned char wf5g_pwr_tbl_dft[3][16] = {{0xBA,0xBA,0xbe,0x7a,0x6a,0x6a,0xbe,0x7C,0x62,0x62,0xb8,0x7a,0x6C,0x6C,0x5c,0x58},
                                                                  {0xBA,0xBA,0x84,0x84,0x75,0x75,0xaC,0x6a,0x62,0x62,0xae,0x75,0x6a,0x66,0x66,0x5c},
                                                                  {0xBA,0xBA,0x84,0x84,0x75,0x75,0x7C,0x75,0x6C,0x6C,0xae,0x6b,0x60,0x60,0x60,0x58}};

    memcpy(&wf2g_txpwr_param->wf2g_pwr_tbl, wf2g_pwr_tbl_dft, sizeof(wf2g_txpwr_param->wf2g_pwr_tbl));
    memcpy(&wf5g_txpwr_param->wf5g_pwr_tbl, wf5g_pwr_tbl_dft, sizeof(wf5g_txpwr_param->wf5g_pwr_tbl));
    return 0;
}

static void set_cca_energy_detection(unsigned int reg_addr)
{
    struct hw_interface* hif =hif_get_hw_interface();
    unsigned int reg_val = 0;

    if (reg_addr == DF_AGC_REG_A12) {
        reg_val = hif->hif_ops.hi_read_word(reg_addr);
        reg_val &= 0xFFFF0000; //bit0~bit15 set to 0
        hif->hif_ops.hi_write_word(reg_addr, reg_val);

    } else if (reg_addr == DF_AGC_REG_A27) {
        reg_val = hif->hif_ops.hi_read_word(reg_addr);
        reg_val &= 0xE00FFFFF; //bit20~bit28 set to 0
        hif->hif_ops.hi_write_word(reg_addr, reg_val);

    } else if (reg_addr == REG_ED_THR_DB) {
        hif->hif_ops.hi_write_word(reg_addr, 0x00328328);

    } else if (reg_addr == REG_STF_AC_Q_THR) {
        hif->hif_ops.hi_write_word(reg_addr, 0x301a4080);
    }
    return;
}

void phy_rf_channel_restore(unsigned short channel, int bw)
{
    struct Channel_Switch channel_switch = {0};

    channel_switch.Cmd = CHANNEL_SWITCH_CMD;
    channel_switch.bw = bw;
    channel_switch.channel = channel;
    channel_switch.flag = 0;
    channel_switch.flag |= CHANNEL_RSSI_PWR_FLAG;

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char*)&channel_switch, sizeof(struct Channel_Switch));
    HAL_END_LOCK();
}

void phy_set_tx_power_accord_rssi(int bw, unsigned short channel, unsigned char rssi, unsigned char power_mode)
{
    struct WF2G_Txpwr_Param wf2g_txpwr_param;
    struct WF5G_Txpwr_Param wf5g_txpwr_param;
    struct Channel_Switch channel_switch = {0};

    memset((void *)&wf2g_txpwr_param, 0, sizeof(struct WF2G_Txpwr_Param));
    memset((void *)&wf5g_txpwr_param, 0, sizeof(struct WF5G_Txpwr_Param));

    wf2g_txpwr_param.Cmd = WF2G_TXPWR_PARAM_CMD;
    wf5g_txpwr_param.Cmd = WF5G_TXPWR_PARAM_CMD;
    g_wf2g_txpwr_param.Cmd = WF2G_TXPWR_PARAM_CMD;
    g_wf5g_txpwr_param.Cmd = WF5G_TXPWR_PARAM_CMD;

    channel_switch.Cmd = CHANNEL_SWITCH_CMD;
    channel_switch.bw = bw;
    channel_switch.channel = channel;
    channel_switch.flag = 0;
    channel_switch.flag |= CHANNEL_RSSI_PWR_FLAG;

    if (!g_tx_power_change_disable) {

        if (power_mode == 2) {
            pr_debug("\n*** %s, change power enhance,bw %d channel %d \n", __func__, channel_switch.bw, channel_switch.channel);
            set_tx_power_param_enhance(&wf2g_txpwr_param, &wf5g_txpwr_param);
            memcpy(&wf5g_txpwr_param.wf5g_pwr_tbl, g_wf5g_txpwr_param.wf5g_pwr_tbl, 32);

            HAL_BEGIN_LOCK();
            hi_set_cmd((unsigned char *)&g_wf2g_txpwr_param, sizeof(struct WF2G_Txpwr_Param));
            hi_set_cmd((unsigned char *)&wf5g_txpwr_param, sizeof(struct WF5G_Txpwr_Param));
            HAL_END_LOCK();

            HAL_BEGIN_LOCK();
            hi_set_cmd((unsigned char*)&channel_switch, sizeof(struct Channel_Switch));
            HAL_END_LOCK();
            return;
        }

        if (power_mode == 1) {
            if (g_wftx_pwrtbl_en == 0) {
                pr_debug("\n*** %s, change power default,bw %d channel %d \n", __func__, channel_switch.bw, channel_switch.channel);
                set_tx_power_param_default(&wf2g_txpwr_param, &wf5g_txpwr_param);

                HAL_BEGIN_LOCK();
                hi_set_cmd((unsigned char *)&wf2g_txpwr_param, sizeof(struct WF2G_Txpwr_Param));
                hi_set_cmd((unsigned char *)&wf5g_txpwr_param, sizeof(struct WF5G_Txpwr_Param));
                HAL_END_LOCK();

            } else if (g_wftx_pwrtbl_en == 1){
                pr_debug("\n*** %s, change power mode 1,bw %d channel %d \n", __func__, channel_switch.bw, channel_switch.channel);
                pr_debug("\n*** %s, change power mode 1,0x%x  0x%x \n", __func__, g_wf2g_txpwr_param.wf2g_pwr_tbl[0][0] , g_wf2g_txpwr_param.wf2g_pwr_tbl[0][1]);
                HAL_BEGIN_LOCK();
                hi_set_cmd((unsigned char *)&g_wf2g_txpwr_param, sizeof(struct WF2G_Txpwr_Param));
                hi_set_cmd((unsigned char *)&g_wf5g_txpwr_param, sizeof(struct WF5G_Txpwr_Param));
                HAL_END_LOCK();
            }

            HAL_BEGIN_LOCK();
            hi_set_cmd((unsigned char*)&channel_switch, sizeof(struct Channel_Switch));
            HAL_END_LOCK();
           return;
        }


    } else {
        return;
    }

}

static unsigned char get_cali_param(struct Cali_Param *cali_param, struct WF2G_Txpwr_Param *wf2g_txpwr_param,
                                                             struct WF5G_Txpwr_Param *wf5g_txpwr_param)
{

    const struct firmware *fw = NULL;
    struct device *dev = vm_cfg80211_get_parent_dev();
    int error = 0;
    int size, len;
    char *content =  NULL;
    unsigned int chip_id_l = 0;
    unsigned char chip_id_buf[100];
    unsigned char file_name[100];

    chip_id_l = efuse_manual_read(0xf);
    chip_id_l = chip_id_l & 0xffff;

    memset(file_name, 0, 100);
    memset(chip_id_buf, 0, 100);
    sprintf(file_name, "aml/aml_wifi_rf_%04x.txt", chip_id_l);
    if (request_firmware(&fw, file_name, dev)) {
        memset(chip_id_buf,'\0',sizeof(chip_id_buf));
        switch ((chip_id_l & 0xff00) >> 8) {
            case MODULE_ITON:
                sprintf(file_name, "aml/aml_wifi_rf_iton.txt");
                break;
            case MODULE_AMPAK:
                sprintf(file_name, "aml/aml_wifi_rf_ampak.txt");
                break;
            case MODULE_FN_LINK:
                sprintf(file_name, "aml/aml_wifi_rf_fn_link.txt");
                break;
            default:
                sprintf(file_name, "aml/aml_wifi_rf.txt");
        }
        pr_info("aml wifi module SN:%04x  sn txt not found, the rf config: %s\n", chip_id_l, chip_id_buf);
    } else
        pr_info("aml wifi module SN:%04x  the rf config: %s\n", chip_id_l, chip_id_buf);

    error = request_firmware(&fw, file_name, dev);
    if (error) {
        // sn txt not found, the rf set default config
        memset(file_name, 0, 100);
        sprintf(file_name, "aml/aml_wifi_rf.txt");
        pr_info("aml wifi module SN:%04x  sn txt not found, the rf config: %s\n", chip_id_l, file_name);
        error = request_firmware(&fw, file_name, dev);
        if (error) {
            pr_info("default txt not found again, Please ensure that txt file include in (vendor/)lib/firamware/w1 \n");
            pr_info("Is there a missing mandatory patch here, %s not used again, Please call Aml FAE\n",conf_path);
            goto err;
        }
    }
    size = fw->size;
    content = (char *)fw->data;

    len = process_cali_content(content, size);
    parse_cali_param(content, len, cali_param);

    if (cali_param->wftx_pwrtbl_en == 0)
    {
        set_tx_power_param_default(wf2g_txpwr_param, wf5g_txpwr_param);
    }
    else if (cali_param->wftx_pwrtbl_en == 1)
    {
        parse_tx_power_param(content, len, wf2g_txpwr_param, wf5g_txpwr_param);
    }
    else if (cali_param->wftx_pwrtbl_en == 2)
    {
        set_tx_power_param_enhance(wf2g_txpwr_param, wf5g_txpwr_param);
    }

    if (cali_param->cca_ed_det == 1) {
        set_cca_energy_detection(DF_AGC_REG_A12);
        set_cca_energy_detection(DF_AGC_REG_A27);

    } else if (cali_param->cca_ed_det == 2) {
        set_cca_energy_detection(DF_AGC_REG_A12);
        set_cca_energy_detection(DF_AGC_REG_A27);
        set_cca_energy_detection(REG_ED_THR_DB);
        set_cca_energy_detection(REG_STF_AC_Q_THR);
    }

    if (chip_id_l & 0xf0) {
        if (chip_id_l & BIT(5))
            cali_param->rf_num = 2;
        else
            cali_param->rf_num = 1;
        pr_debug("rf_cout is: %d\n", cali_param->rf_num);
    }
    release_firmware(fw);
    return 0;
err:
    return 1;
}

unsigned int hal_cfg_txpwr_cffc_param(void * chan, void * txpwr_plan)
{
    struct wifi_channel * s_chan = (struct wifi_channel *)chan;
    struct tx_power_plan * s_txpwr_plan = ( struct tx_power_plan *)txpwr_plan;
    struct Txpwr_Cffc_Cfg_Param txpwr_cffc_param={0};
    unsigned short center_channel = 0;

    txpwr_cffc_param.Cmd = TXPWR_CFFC_CFG_CMD;
    memcpy(txpwr_cffc_param.band, s_txpwr_plan->band_pwr_table, 4);
    memcpy(txpwr_cffc_param.coefficient, s_txpwr_plan->coefficient, s_txpwr_plan->cffc_num);

    AML_OUTPUT("coefficient[%d]:\n", s_txpwr_plan->cffc_num);

    HAL_BEGIN_LOCK();
    hi_set_cmd((unsigned char *)&txpwr_cffc_param, sizeof(struct Txpwr_Cffc_Cfg_Param));
    HAL_END_LOCK();

    if (chan) {
        center_channel =  wifi_mac_Mhz2ieee(s_chan->chan_cfreq1, 0);
        AML_OUTPUT("restore chan:%d bw:%d \n", center_channel, s_chan->chan_bw);
        phy_rf_channel_restore(center_channel, s_chan->chan_bw);
    }

    return 0;
}

unsigned int hal_cfg_cali_param(void)
{
    struct Cali_Param cali_param;
    struct WF2G_Txpwr_Param wf2g_txpwr_param;
    struct WF5G_Txpwr_Param wf5g_txpwr_param;
    unsigned char err = 0;

    memset((void *)&cali_param, 0, sizeof(struct Cali_Param));
    memset((void *)&wf2g_txpwr_param, 0, sizeof(struct WF2G_Txpwr_Param));
    memset((void *)&wf5g_txpwr_param, 0, sizeof(struct WF5G_Txpwr_Param));

    cali_param.Cmd = CALI_PARAM_CMD;
    wf2g_txpwr_param.Cmd = WF2G_TXPWR_PARAM_CMD;
    wf5g_txpwr_param.Cmd = WF5G_TXPWR_PARAM_CMD;
    err = get_cali_param(&cali_param, &wf2g_txpwr_param, &wf5g_txpwr_param);

    pr_debug("calibration parameter: version %d, config %d, freq_offset %d, tssi_2g %d, tssi_5g %d %d %d %d tx_en %d\n",
            cali_param.version, cali_param.cali_config, cali_param.freq_offset, cali_param.tssi_2g_offset,
            cali_param.tssi_5g_offset[0], cali_param.tssi_5g_offset[1], cali_param.tssi_5g_offset[2], cali_param.tssi_5g_offset[3], cali_param.wftx_pwrtbl_en);

    if (err == 0) {
        pr_debug("%s:%d, set calibration parameter \n", __func__, __LINE__);
        HAL_BEGIN_LOCK();
        hi_set_cmd((unsigned char *)&cali_param, sizeof(struct Cali_Param));
        hi_set_cmd((unsigned char *)&wf2g_txpwr_param, sizeof(struct WF2G_Txpwr_Param));
        hi_set_cmd((unsigned char *)&wf5g_txpwr_param, sizeof(struct WF5G_Txpwr_Param));
        HAL_END_LOCK();
    }
    else {
        ERROR_DEBUG_OUT("set calibration parameter failed\n");
        return false;
    }

    return true;
}

void print_driver_version(void) {
  printk("driver version: %s\n", DRIVERVERSION);
}

#ifdef HAL_SIM_VER
#ifdef FW_NAME
}
#endif
#endif
