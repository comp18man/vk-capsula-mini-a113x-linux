#include "wifi_hal_com.h"

struct sts_cfg_data g_sts_cfg;
static char* func_prt_info[] = {
"-----------clear the statistic:-----------",
"-----------start to do statistic:-----------",
"-----------read the statistic:-----------",
"-----------stop the statistic:-----------",
"-----------show the configuration:-----------"
};

static char * sts_man[] ={"-----------format:-----------\n",
    "iw dev wlan0 vendor send 0xc3 0xc4 op_code,addr high 8bits,addr low 8bits,function code,data_0,data_1,data_2 data_3\n",
    "-----------op_code:-----------\n",
"0x96 + systems:set the default system cfg: sw statistic 0x1, sw probe 0x0\n",
"0x97 + addr high 8bits,addr low 8bits,data_0,data_1,data_2 data_3 to update the cfg;\n",
"0x98 + function code to clear/start/read/stop by the function code;\n",
"-----------function code: -----------\n",
"clear 0x0, start 0x1, read 0x2, stop 0x3\n",
"please follow the hardware registers spec to set the configuration\n"
};

static unsigned short sts_reg_chk_pool[] = {
            RG_MAC_CNT_CTRL_FIQ,
            RG_MAC_FRM_TYPE_CNT_CTRL,
            RG_MAC_TX_STT_CHK,
            RG_MAC_TX_END_CHK,
            RG_MAC_TX_EN_CHK,
            RG_MAC_TX_ENWR_CHK,
            RG_MAC_RX_DATA_LOCAL_CNT,
            RG_MAC_RX_DATA_OTHER_CNT,
            RG_MAC_RX_DATA_MUTIL_CNT,
            RG_AGC_STATE,
            RG_AGC_OB_IC_GAIN,
            RG_AGC_WATCH1,
            RG_AGC_WATCH2,
            RG_AGC_WATCH3,
            RG_AGC_WATCH4,
            RG_AGC_OB_ANT_NFLOOR,
            RG_AGC_OB_CCA_RES,
            RG_AGC_OB_CCA_COND01,
            RG_AGC_OB_CCA_COND23,
            RG_WIFI_IF_HOST_IRQ_ST,
            HOST_SW_VADDR,
        };



/*
---description---
it is used to do all kinds of statistics information initialization
for log showing, the information content is mapped by the array index which defined
by the hardware
@(void)
*/

static char* sts_mac_irq_prt_tag[5]={NULL,NULL,"mac irq  ","mac irq  ",NULL};
static char* sts_mac_11frm_prt_tag[5]={NULL,NULL,"mac frame", "mac frame",NULL};
static char* sts_mac_tx_prt_tag[5]={NULL,NULL,"hw mac tx", "hw mac tx",NULL};
static char* sts_mac_rx_prt_tag[5] ={NULL,NULL,"hw mac rx", "hw mac rx",NULL};
static char* sts_agc_prt_tag[5] ={NULL,NULL,"hw agc   ", "hw agc   ",NULL};
static char* sts_hst_sw_prt_tag[5] ={NULL,NULL,"host sw ", "host sw ",NULL};

static char* sts_sys_prt_info[sts_hst_sw_max_idx];
static unsigned int sts_sys_idx_val[sts_hst_sw_max_idx] ;

static void sts_chk_idx_val(unsigned int  *idx_val)
{
    int i =0;
    int j = 0;
    int rpt = 0;

    for (i = 0; i < sts_hst_sw_max_idx; i++)
        {
           //pr_debug("idx[%d] -->val 0x%x \n", i, sts_sys_idx_val[i]);
            for (j = i+1; j < sts_hst_sw_max_idx; j++)
                {
                    //pr_debug("val1 0x%x, val2 0x%x \n", sts_sys_idx_val[i], sts_sys_idx_val[j]);
                    if(idx_val[i] == idx_val[j])
                        {
                           rpt = 1;
                           break;
                        }
                        else
                        {
                            continue;
                        }
                }

            if(rpt == 1)
                {
                    pr_err("mapping err !the repeat val is 0x%x, i %d, j %d\n", idx_val[i], i, j);
                    break;
                }
        }


}

void sts_prt_info_map(void)
{
    /*
    mapping all of the idx_val
    */
    sts_sys_idx_val[sts_mac_irq_txstart_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 | IDX_TXSTART_IT;
    sts_sys_idx_val[sts_mac_irq_txend_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TXEND_IT ;
    sts_sys_idx_val[sts_mac_irq_txenderr_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TXENDERR;
    sts_sys_idx_val[sts_mac_irq_ackto_idx] =   (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_ACKTO_IT;
    sts_sys_idx_val[sts_mac_irq_txcol_flag_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TXCOL_FLAG;
    sts_sys_idx_val[sts_mac_irq_rxstart_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_RXSTART_IT;
    sts_sys_idx_val[sts_mac_irq_mac_rxend_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_RXEND_IT;
    sts_sys_idx_val[sts_mac_irq_mac_rxmpdu_ok_idx] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_RXMPDU_OK ;
    sts_sys_idx_val[sts_mac_irq_unicast_idx ] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_UNICAST ;
    sts_sys_idx_val[sts_mac_irq_srambuf_full_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_SRAMBUF_FULL ;
    sts_sys_idx_val[sts_mac_irq_swirq_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_SWIRQ  ;
    sts_sys_idx_val[sts_mac_irq_tx_frame_type_idx ] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TX_FRAME_TYPE ;
    sts_sys_idx_val[sts_mac_irq_abscount_intr_idx ] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_ABSCOUNT_INTR ;
    sts_sys_idx_val[sts_mac_irq_fiq_enable_idx] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_FIQ_ENABLE ;
    sts_sys_idx_val[sts_mac_irq_fiac_fw_timeout_err_idx] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_FIAC_FW_TIMEOUT_ERR;
    sts_sys_idx_val[sts_mac_irq_hw_timeout_err_idx] =   (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_HW_TIMEOUT_ERR ;
    sts_sys_idx_val[sts_mac_irq_tx_wd_timeout_err_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TX_WD_TIMEOUT_ERR ;
    sts_sys_idx_val[sts_mac_irq_txinvalidlen_err_idx] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TXINVALIDLEN_ERR  ;
    sts_sys_idx_val[sts_mac_irq_tx_keymiss_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TX_KEYMISS  ;
    sts_sys_idx_val[sts_mac_irq_tximstop_err_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16|IDX_TXIMSTOP_ERR  ;
    sts_sys_idx_val[sts_mac_irq_tx_phyerror_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TX_PHYERROR ;
    sts_sys_idx_val[sts_mac_irq_txcol_queue_idx] =   (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_TXCOL_QUEUE   ;
    sts_sys_idx_val[sts_mac_irq_tx_not_grant_idx] =   (RG_MAC_CNT_CTRL_FIQ)<<16|IDX_TX_NOT_GRANT  ;
    sts_sys_idx_val[sts_mac_irq_srambuf_full_err_idx ] =   (RG_MAC_CNT_CTRL_FIQ)<<16|IDX_SRAMBUF_FULL_ERR  ;
    sts_sys_idx_val[sts_mac_irq_fcserr_idx ] = (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_FCSERR   ;
    sts_sys_idx_val[sts_mac_irq_rxfifoov_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16|IDX_RXFIFOOV    ;
    sts_sys_idx_val[sts_mac_irq_rx_wd_timeout_err_idx ] =(RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_RX_WD_TIMEOUT_ERR  ;
    sts_sys_idx_val[sts_mac_irq_short_length_idx] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_SHORT_LENGTH ;
    sts_sys_idx_val[sts_mac_irq_length_error_idx ] =   (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_LENGTH_ERROR  ;
    sts_sys_idx_val[sts_mac_irq_rx_phyerror_idx ] =  (RG_MAC_CNT_CTRL_FIQ)<<16 |IDX_RX_PHYERROR   ;
    sts_sys_idx_val[sts_frm_type_qos_data_idx ] =   (RG_MAC_FRM_TYPE_CNT_CTRL)<< 16 | IDX_QOS_DATA;
    sts_sys_idx_val[sts_frm_type_rts_idx ] =  (RG_MAC_FRM_TYPE_CNT_CTRL)<< 16 | IDX_RTS   ;
    sts_sys_idx_val[sts_frm_type_cts_idx] =   (RG_MAC_FRM_TYPE_CNT_CTRL)<< 16 | IDX_CTS ;
    sts_sys_idx_val[sts_frm_type_ack_idx] =   (RG_MAC_FRM_TYPE_CNT_CTRL)<< 16 | IDX_ACK  ;
    sts_sys_idx_val[sts_mac_tx_start_chk_idx] =  RG_MAC_TX_STT_CHK;
    sts_sys_idx_val[sts_mac_tx_end_chk_idx ] =   RG_MAC_TX_END_CHK;
    sts_sys_idx_val[sts_mac_tx_en_chk_idx] =     RG_MAC_TX_EN_CHK;
    sts_sys_idx_val[sts_mac_tx_enwr_chk_idx] =   RG_MAC_TX_ENWR_CHK;
    sts_sys_idx_val[sts_mac_rx_data_local_cnt_idx] =   RG_MAC_RX_DATA_LOCAL_CNT;
    sts_sys_idx_val[sts_mac_rx_data_other_cnt_idx] =  RG_MAC_RX_DATA_OTHER_CNT;
    sts_sys_idx_val[sts_mac_rx_data_mutil_cnt_idx] =   RG_MAC_RX_DATA_MUTIL_CNT;

    sts_sys_idx_val[sts_agc_st_idx] = ((RG_AGC_STATE) << 16) |  sts_agc_st_idx;
    sts_sys_idx_val[sts_agc_gain_rx_ic_gain_idx] = ( (RG_AGC_OB_IC_GAIN) << 16) |  sts_agc_gain_rx_ic_gain_idx;
    sts_sys_idx_val[sts_agc_gain_fine_db_idx] =  ((RG_AGC_OB_IC_GAIN) << 16) |  sts_agc_gain_fine_db_idx;
    sts_sys_idx_val[sts_agc_wtc1_pw_sat_db_idx] = ( (RG_AGC_WATCH1) << 16) |  sts_agc_wtc1_pw_sat_db_idx;
    sts_sys_idx_val[sts_agc_wtc1_prm20_db_idx] =  ((RG_AGC_WATCH1) << 16) |  sts_agc_wtc1_prm20_db_idx;
    sts_sys_idx_val[sts_agc_wtc2_prm40_db_idx] =  ((RG_AGC_WATCH2) << 16) |  sts_agc_wtc2_prm40_db_idx;
    sts_sys_idx_val[sts_agc_wtc2_prm80_db_idx] =  ((RG_AGC_WATCH2) << 16) |  sts_agc_wtc2_prm80_db_idx;
    sts_sys_idx_val[sts_agc_wtc3_rssi0_db_idx] =  ((RG_AGC_WATCH3) << 16) |  sts_agc_wtc3_rssi0_db_idx;
    sts_sys_idx_val[sts_agc_wtc3_rssi1_db_idx] =  ((RG_AGC_WATCH3) << 16) |  sts_agc_wtc3_rssi1_db_idx;
    sts_sys_idx_val[sts_agc_wtc4_rssi2_db_idx] =  ((RG_AGC_WATCH4) << 16) |  sts_agc_wtc4_rssi2_db_idx;
    sts_sys_idx_val[sts_agc_wtc4_rssi3_db_idx] =  ((RG_AGC_WATCH4) << 16) |  sts_agc_wtc4_rssi3_db_idx;
    sts_sys_idx_val[sts_agc_ant_flr_snr_qdb_idx] =  ((RG_AGC_OB_ANT_NFLOOR) << 16) |sts_agc_ant_flr_snr_qdb_idx;

    sts_sys_idx_val[sts_agc_ant_flr_ant_flr_qdb_idx] =  ((RG_AGC_OB_ANT_NFLOOR) << 16) | sts_agc_ant_flr_ant_flr_qdb_idx;
    sts_sys_idx_val[sts_cca_res_rdy_idx] =  ((RG_AGC_OB_CCA_RES) << 16) |sts_cca_res_rdy_idx;
    sts_sys_idx_val[sts_cca_res_cond8_idx] =  ((RG_AGC_OB_CCA_RES) << 16) | sts_cca_res_cond8_idx;
    sts_sys_idx_val[sts_cca_cnd01_cond0_idx] =  ((RG_AGC_OB_CCA_COND01) << 16) |sts_cca_cnd01_cond0_idx;
    sts_sys_idx_val[sts_cca_cnd01_cond1_idx] =  ((RG_AGC_OB_CCA_COND01) << 16) | sts_cca_cnd01_cond1_idx;
    sts_sys_idx_val[sts_cca_cnd23_cond2_idx] =  ((RG_AGC_OB_CCA_COND23) << 16) |sts_cca_cnd23_cond2_idx;
    sts_sys_idx_val[sts_cca_cnd23_cond3_idx] =  ((RG_AGC_OB_CCA_COND23) << 16) | sts_cca_cnd23_cond3_idx;

    sts_sys_idx_val[sts_hst_irq_tx_error_idx] = ((HOST_SW_VADDR)<<16) |sts_hst_irq_tx_error_idx;
    sts_sys_idx_val[sts_hst_irq_rx_error_idx ] = ((HOST_SW_VADDR)<<16) |sts_hst_irq_rx_error_idx;
    sts_sys_idx_val[sts_hst_irq_rx_ok_idx] = ((HOST_SW_VADDR)<<16) |sts_hst_irq_rx_ok_idx;
    sts_sys_idx_val[sts_hst_irq_tx_ok_idx ] =  ((HOST_SW_VADDR)<<16) | sts_hst_irq_tx_ok_idx;
    sts_sys_idx_val[sts_hst_irq_beacon_send_ok_vid0_idx] = (HOST_SW_VADDR)<<16 |sts_hst_irq_beacon_send_ok_vid0_idx;
    sts_sys_idx_val[sts_hst_irq_beacon_send_ok_vid1_idx ] =(HOST_SW_VADDR)<<16 |sts_hst_irq_beacon_send_ok_vid1_idx;
    sts_sys_idx_val[sts_hst_irq_goto_wakeup_vid0_idx] =   (HOST_SW_VADDR)<<16 |sts_hst_irq_goto_wakeup_vid0_idx;
    sts_sys_idx_val[sts_hst_irq_goto_wakeup_vid1_idx] =   (HOST_SW_VADDR)<<16 | sts_hst_irq_goto_wakeup_vid1_idx;
    sts_sys_idx_val[sts_hst_irq_p2p_oppps_cwend_vid0_idx] = (HOST_SW_VADDR)<<16 | sts_hst_irq_p2p_oppps_cwend_vid0_idx;
    sts_sys_idx_val[sts_hst_irq_p2p_oppps_cwend_vid1_idx] = (HOST_SW_VADDR)<<16 | sts_hst_irq_p2p_oppps_cwend_vid1_idx;
    sts_sys_idx_val[sts_hst_irq_p2p_noa_start_vid0_idx ] =(HOST_SW_VADDR)<<16 |sts_hst_irq_p2p_noa_start_vid0_idx;
    sts_sys_idx_val[sts_hst_irq_p2p_noa_start_vid1_idx] = (HOST_SW_VADDR)<<16 | sts_hst_irq_p2p_noa_start_vid1_idx;
    sts_sys_idx_val[sts_hst_irq_beacon_recv_ok_vid0_idx ] = (HOST_SW_VADDR)<<16 |sts_hst_irq_beacon_recv_ok_vid0_idx;
    sts_sys_idx_val[sts_hst_irq_beacon_recv_ok_vid1_idx ] = (HOST_SW_VADDR)<<16 | sts_hst_irq_beacon_recv_ok_vid1_idx;
    sts_sys_idx_val[sts_hst_irq_beacon_miss_vid0_idx] = (HOST_SW_VADDR)<<16 |sts_hst_irq_beacon_miss_vid0_idx;
    sts_sys_idx_val[sts_hst_irq_beacon_miss_vid1_idx ] =(HOST_SW_VADDR)<<16 | sts_hst_irq_beacon_miss_vid1_idx;

    sts_sys_idx_val[sts_hal_agg_start_tid0_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid0_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid1_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid1_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid2_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid2_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid3_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid3_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid4_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid4_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid5_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid5_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid6_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid6_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid7_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid7_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid8_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid8_idx;
    sts_sys_idx_val[sts_hal_agg_start_tid9_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_agg_start_tid9_idx;
    sts_sys_idx_val[sts_hal_rx_fifo_full_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_rx_fifo_full_idx;
    sts_sys_idx_val[sts_hal_rx_up_skb_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_rx_up_skb_idx;
    sts_sys_idx_val[sts_hal_rx_vif_id_err_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_rx_vif_id_err_idx;
    sts_sys_idx_val[sts_hal_rx_dcrypt_err_idx ] =  (HOST_SW_VADDR) << 16 | sts_hal_rx_dcrypt_err_idx;
    sts_sys_idx_val[sts_hal_tx_num_idx] = (HOST_SW_VADDR) << 16 | sts_hal_tx_num_idx;
    sts_sys_idx_val[sts_hal_tx_mpdus_idx] = (HOST_SW_VADDR) << 16 | sts_hal_tx_mpdus_idx;
    sts_sys_idx_val[sts_hal_tx_pages_idx] = (HOST_SW_VADDR) << 16 | sts_hal_tx_pages_idx;
    sts_sys_idx_val[sts_hal_dw_mpdu_idx] = (HOST_SW_VADDR) << 16 | sts_hal_dw_mpdu_idx;
    sts_sys_idx_val[sts_hal_cur_mpdu_idx] = (HOST_SW_VADDR) << 16 | sts_hal_cur_mpdu_idx;
    sts_sys_idx_val[sts_hal_cur_agg_page_idx] = (HOST_SW_VADDR) << 16 | sts_hal_cur_agg_page_idx;
    sts_sys_idx_val[sts_hal_allc_txds_idx] = (HOST_SW_VADDR) << 16 | sts_hal_allc_txds_idx;
    sts_sys_idx_val[sts_hal_free_txds_idx] = (HOST_SW_VADDR) << 16 | sts_hal_free_txds_idx;
    sts_sys_idx_val[sts_hal_send_null_idx] = (HOST_SW_VADDR) << 16 | sts_hal_send_null_idx;
    sts_sys_idx_val[sts_hal_keep_alive_idx] = (HOST_SW_VADDR) << 16 |sts_hal_keep_alive_idx ;
    sts_sys_idx_val[sts_hal_beacon_miss_idx] = (HOST_SW_VADDR) << 16 |sts_hal_beacon_miss_idx;
    sts_sys_idx_val[sts_drv_tx_total_bytes_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_total_bytes_idx;
    sts_sys_idx_val[sts_drv_tx_pkts_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_pkts_idx;
    sts_sys_idx_val[sts_drv_tx_drops_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_drops_idx;
    sts_sys_idx_val[sts_drv_tx_normal_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_normal_idx;
    sts_sys_idx_val[sts_drv_tx_end_normal_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_end_normal_idx;
    sts_sys_idx_val[sts_drv_tx_end_ampdu_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_end_ampdu_idx;
    sts_sys_idx_val[sts_drv_tx_ampdu_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_ampdu_idx;
    sts_sys_idx_val[sts_drv_tx_ampdu_on_frm_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_ampdu_on_frm_idx;
    sts_sys_idx_val[sts_drv_tx_end_fail_idx] = (HOST_SW_VADDR) << 16 | sts_drv_tx_end_fail_idx;
    sts_sys_idx_val[sts_drv_rx_ampdu_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_ampdu_idx;
    sts_sys_idx_val[sts_drv_rx_bar_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_bar_idx;
    sts_sys_idx_val[sts_drv_rx_no_qos_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_no_qos_idx;
    sts_sys_idx_val[sts_drv_rx_dup_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_dup_idx;
    sts_sys_idx_val[sts_drv_rx_ok_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_ok_idx;
    sts_sys_idx_val[sts_drv_rx_flush_drop_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_flush_drop_idx;
    sts_sys_idx_val[sts_drv_rx_bar_drop_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_bar_drop_idx;
    sts_sys_idx_val[sts_drv_rx_skipped_idx] = (HOST_SW_VADDR) << 16 | sts_drv_rx_skipped_idx;
    sts_sys_idx_val[sts_drv_txlist0_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist0_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist1_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist1_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist2_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist2_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist3_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist3_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist4_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist4_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist5_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist5_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist6_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist6_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist_qcnt_idx7] = (HOST_SW_VADDR) << 16 | sts_drv_txlist_qcnt_idx7;
    sts_sys_idx_val[sts_drv_send_skb_idx] = (HOST_SW_VADDR) << 16 | sts_drv_send_skb_idx;
    sts_sys_idx_val[sts_drv_txlist0_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist0_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist1_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist1_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist2_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist2_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist3_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist3_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist4_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist4_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist5_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist5_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist6_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist6_pend_qcnt_idx;
    sts_sys_idx_val[sts_drv_txlist7_pend_qcnt_idx] = (HOST_SW_VADDR) << 16 | sts_drv_txlist7_pend_qcnt_idx;
    sts_sys_idx_val[sts_vif_ip_pkt_total_idx] = (HOST_SW_VADDR) << 16 | sts_vif_ip_pkt_total_idx;
    sts_sys_idx_val[sts_vif_ip_pkt_to_drv_idx] = (HOST_SW_VADDR) << 16 | sts_vif_ip_pkt_to_drv_idx;
    sts_sys_idx_val[sts_vif_ip_pkt_drop_idx] = (HOST_SW_VADDR) << 16 | sts_vif_ip_pkt_drop_idx;
    sts_sys_idx_val[sts_vif_tx_list_full_idx] = (HOST_SW_VADDR) << 16 | sts_vif_tx_list_full_idx;
    sts_sys_idx_val[sts_vif_tx_ok_idx] = (HOST_SW_VADDR) << 16 | sts_vif_tx_ok_idx;
    sts_sys_idx_val[sts_vif_tx_cmp_idx] = (HOST_SW_VADDR) << 16 | sts_vif_tx_cmp_idx;
    sts_sys_idx_val[sts_vif_tx_free_page_idx] = (HOST_SW_VADDR) << 16 | sts_vif_tx_free_page_idx;
    sts_sys_idx_val[sts_vif_in_amsdu_idx] = (HOST_SW_VADDR) << 16 | sts_vif_in_amsdu_idx;
    sts_sys_idx_val[sts_vif_out_amsdu_idx] = (HOST_SW_VADDR) << 16 | sts_vif_out_amsdu_idx;

    //sts_sys_idx_val[sts_hst_sw_max_idx] = (HOST_SW_VADDR) << 16 | sts_hst_sw_max_idx;
    sts_chk_idx_val(sts_sys_idx_val);

    /*
    mapping all of the prt info
    */

    sts_sys_prt_info[sts_mac_irq_txstart_idx  ] = "txstart";
    sts_sys_prt_info[sts_mac_irq_txend_idx   ] = "txend";
    sts_sys_prt_info[sts_mac_irq_txenderr_idx] = "txenderr";
    sts_sys_prt_info[sts_mac_irq_ackto_idx  ] = "ackto";
    sts_sys_prt_info[sts_mac_irq_txcol_flag_idx ] = "txcol_flag";
    sts_sys_prt_info[sts_mac_irq_rxstart_idx  ] = "rxstart";
    sts_sys_prt_info[sts_mac_irq_mac_rxend_idx] = "rxend";
    sts_sys_prt_info[sts_mac_irq_mac_rxmpdu_ok_idx] = "rxmpdu_ok";
    sts_sys_prt_info[sts_mac_irq_unicast_idx ] = "unicast";
    sts_sys_prt_info[sts_mac_irq_srambuf_full_idx ] = "srambuf_full";
    sts_sys_prt_info[sts_mac_irq_swirq_idx ] = "swirq";
    sts_sys_prt_info[sts_mac_irq_tx_frame_type_idx ] = "tx_frame_type";
    sts_sys_prt_info[sts_mac_irq_abscount_intr_idx ] = "abscount_intr";
    sts_sys_prt_info[sts_mac_irq_fiq_enable_idx] = "fiq_enable";
    sts_sys_prt_info[sts_mac_irq_fiac_fw_timeout_err_idx] = "fiac_fw_timeout_err";
    sts_sys_prt_info[sts_mac_irq_hw_timeout_err_idx] = "hw_timeout_err";
    sts_sys_prt_info[sts_mac_irq_tx_wd_timeout_err_idx] = "tx_wd_timeout_err";
    sts_sys_prt_info[sts_mac_irq_txinvalidlen_err_idx] = "txinvalidlen_err";
    sts_sys_prt_info[sts_mac_irq_tx_keymiss_idx] = "tx_keymiss";
    sts_sys_prt_info[sts_mac_irq_tximstop_err_idx ] = "tximstop_err";
    sts_sys_prt_info[sts_mac_irq_tx_phyerror_idx] = "tx_phyerror";
    sts_sys_prt_info[sts_mac_irq_txcol_queue_idx] = "txcol_queue";
    sts_sys_prt_info[sts_mac_irq_tx_not_grant_idx] = "tx_not_grant";
    sts_sys_prt_info[sts_mac_irq_srambuf_full_err_idx ] = "srambuf_full_err";
    sts_sys_prt_info[sts_mac_irq_fcserr_idx ] = "fcserr";
    sts_sys_prt_info[sts_mac_irq_rxfifoov_idx ] = "rxfifoov";
    sts_sys_prt_info[sts_mac_irq_rx_wd_timeout_err_idx ] = "rx_wd_timeout_err";
    sts_sys_prt_info[sts_mac_irq_short_length_idx] = "short_length";
    sts_sys_prt_info[sts_mac_irq_length_error_idx ] = "length_error";
    sts_sys_prt_info[sts_mac_irq_rx_phyerror_idx ] = "rx_phyerror";
    sts_sys_prt_info[sts_frm_type_qos_data_idx ] = "qos_data";
    sts_sys_prt_info[sts_frm_type_rts_idx ] = "rts";
    sts_sys_prt_info[sts_frm_type_cts_idx] = "cts";
    sts_sys_prt_info[sts_frm_type_ack_idx] = "ack";
    sts_sys_prt_info[sts_mac_tx_start_chk_idx] = "start_chk";
    sts_sys_prt_info[sts_mac_tx_end_chk_idx ] = "tx_end_chk";
    sts_sys_prt_info[sts_mac_tx_en_chk_idx] = "tx_en_chk";
    sts_sys_prt_info[sts_mac_tx_enwr_chk_idx] = "tx_enwr_chk";
    sts_sys_prt_info[sts_mac_rx_data_local_cnt_idx] = "rx_data_local_cnt";
    sts_sys_prt_info[sts_mac_rx_data_other_cnt_idx] = "rx_data_other_cnt";
    sts_sys_prt_info[sts_mac_rx_data_mutil_cnt_idx] = "rx_data_mutil_cnt";
    sts_sys_prt_info[sts_agc_st_idx] =  "agc_st";
    sts_sys_prt_info[sts_agc_gain_rx_ic_gain_idx] =  "rx_ic_gain";
    sts_sys_prt_info[sts_agc_gain_fine_db_idx] =  "fine_db";
    sts_sys_prt_info[sts_agc_wtc1_pw_sat_db_idx] =  "pw_sat_db";
    sts_sys_prt_info[sts_agc_wtc1_prm20_db_idx] = "prm20_db";
    sts_sys_prt_info[sts_agc_wtc2_prm40_db_idx] =  "rm40_db";
    sts_sys_prt_info[sts_agc_wtc2_prm80_db_idx] =  "prm80_db";
    sts_sys_prt_info[sts_agc_wtc3_rssi0_db_idx] =  "rssi0_db";
    sts_sys_prt_info[sts_agc_wtc3_rssi1_db_idx] =  "rssi1_db";
    sts_sys_prt_info[sts_agc_wtc4_rssi2_db_idx] =  "rssi2_db";
    sts_sys_prt_info[sts_agc_wtc4_rssi3_db_idx] =  "rssi3_db";
    sts_sys_prt_info[sts_agc_ant_flr_snr_qdb_idx] =  "snr_qdb";
    sts_sys_prt_info[sts_agc_ant_flr_ant_flr_qdb_idx] =  "ant_flr_qdb";
    sts_sys_prt_info[sts_cca_res_rdy_idx] =  "cca_res_rdy";
    sts_sys_prt_info[sts_cca_res_cond8_idx] =  "cca_res_cond8";
    sts_sys_prt_info[sts_cca_cnd01_cond0_idx] =  "cca_cnd01_cond0";
    sts_sys_prt_info[sts_cca_cnd01_cond1_idx] =  "cca_cnd01_cond1";
    sts_sys_prt_info[sts_cca_cnd23_cond2_idx] =  "cca_cnd23_cond2";
    sts_sys_prt_info[sts_cca_cnd23_cond3_idx] =  "cca_cnd23_cond3";
    sts_sys_prt_info[sts_hst_irq_tx_error_idx] = "int tx_error";
    sts_sys_prt_info[sts_hst_irq_rx_error_idx ] = "int rx_error";
    sts_sys_prt_info[sts_hst_irq_rx_ok_idx] = "int rx_ok";
    sts_sys_prt_info[sts_hst_irq_tx_ok_idx ] = "int tx_ok";
    sts_sys_prt_info[sts_hst_irq_beacon_send_ok_vid0_idx] = "int bcn_send_ok_vid0";
    sts_sys_prt_info[sts_hst_irq_beacon_send_ok_vid1_idx ] = "int bcn_send_ok_vid1";
    sts_sys_prt_info[sts_hst_irq_goto_wakeup_vid0_idx] = "int goto_wakeup_vid0";
    sts_sys_prt_info[sts_hst_irq_goto_wakeup_vid1_idx] = "int goto_wakeup_vid1";
    sts_sys_prt_info[sts_hst_irq_p2p_oppps_cwend_vid0_idx] = "int p2p_oppps_cwend_vid0";
    sts_sys_prt_info[sts_hst_irq_p2p_oppps_cwend_vid1_idx] = "int p2p_oppps_cwend_vid1";
    sts_sys_prt_info[sts_hst_irq_p2p_noa_start_vid0_idx ] = "int p2p_noa_start_vid0";
    sts_sys_prt_info[sts_hst_irq_p2p_noa_start_vid1_idx] = "int p2p_noa_start_vid1";
    sts_sys_prt_info[sts_hst_irq_beacon_recv_ok_vid0_idx ] = "int bcn_recv_ok_vid0";
    sts_sys_prt_info[sts_hst_irq_beacon_recv_ok_vid1_idx ] = "int bcn_recv_ok_vid1";
    sts_sys_prt_info[sts_hst_irq_beacon_miss_vid0_idx] = "int bcn_miss_vid0";
    sts_sys_prt_info[sts_hst_irq_beacon_miss_vid1_idx ] = "int bcn_miss_vid1";
    sts_sys_prt_info[sts_hal_agg_start_tid0_idx ] = "hal_agg_start_tid0";
    sts_sys_prt_info[sts_hal_agg_start_tid1_idx ] = "hal_agg_start_tid1";
    sts_sys_prt_info[sts_hal_agg_start_tid2_idx ] = "hal_agg_start_tid2";
    sts_sys_prt_info[sts_hal_agg_start_tid3_idx ] = "hal_agg_start_tid3";
    sts_sys_prt_info[sts_hal_agg_start_tid4_idx ] = "hal_agg_start_tid4";
    sts_sys_prt_info[sts_hal_agg_start_tid5_idx ] = "hal_agg_start_tid5";
    sts_sys_prt_info[sts_hal_agg_start_tid6_idx ] = "hal_agg_start_tid6";
    sts_sys_prt_info[sts_hal_agg_start_tid7_idx ] = "hal_agg_start_tid7";
    sts_sys_prt_info[sts_hal_agg_start_tid8_idx ] = "hal_agg_start_tid8";
    sts_sys_prt_info[sts_hal_agg_start_tid9_idx ] = "hal_agg_start_tid9";
    sts_sys_prt_info[sts_hal_rx_fifo_full_idx ] =  "hal_rx_fifo_full"; ;
    sts_sys_prt_info[sts_hal_rx_up_skb_idx ] =  "hal_to_up_skb";
    sts_sys_prt_info[sts_hal_rx_vif_id_err_idx ] =  "hal_rx_vif_id_err";
    sts_sys_prt_info[sts_hal_rx_dcrypt_err_idx ] =   "hal_rx_dcrypt_err";
    sts_sys_prt_info[sts_hal_tx_num_idx] = "hal_tx_num"; ;
    sts_sys_prt_info[sts_hal_tx_mpdus_idx] = "hal_tx_mpdus";
    sts_sys_prt_info[sts_hal_tx_pages_idx] =  "hal_tx_pages";
    sts_sys_prt_info[sts_hal_dw_mpdu_idx] = "hal_dw_mpdu";
    sts_sys_prt_info[sts_hal_cur_mpdu_idx] = "hal_cur_mpdu";
    sts_sys_prt_info[sts_hal_cur_agg_page_idx] = "hal_cur_agg_page";
    sts_sys_prt_info[sts_hal_allc_txds_idx] = "hal_allc_txds_idx";
    sts_sys_prt_info[sts_hal_free_txds_idx] = "hal_free_txds";
    sts_sys_prt_info[sts_hal_send_null_idx  ] = "hal_send_null";
    sts_sys_prt_info[sts_hal_keep_alive_idx  ] = "hal_keep_alive";
    sts_sys_prt_info[sts_hal_beacon_miss_idx ] = "hal_bcn_miss";
    sts_sys_prt_info[sts_drv_tx_total_bytes_idx] = "drv_tx_total_bytes";
    sts_sys_prt_info[sts_drv_tx_pkts_idx] = "drv_tx_pkts";
    sts_sys_prt_info[sts_drv_tx_drops_idx] = "drv_tx_drops";
    sts_sys_prt_info[sts_drv_tx_normal_idx] = "drv_tx_normal";
    sts_sys_prt_info[sts_drv_tx_end_normal_idx] = "drv_tx_end_normal";
    sts_sys_prt_info[sts_drv_tx_ampdu_idx] = "drv_tx_ampdu";
    sts_sys_prt_info[sts_drv_tx_ampdu_on_frm_idx] = "drv_tx_ampdu_on_frm";
    sts_sys_prt_info[sts_drv_tx_end_fail_idx] = "drv_tx_end_fail";
    sts_sys_prt_info[sts_drv_rx_ampdu_idx] = "drv_rx_input";
    sts_sys_prt_info[sts_drv_rx_bar_idx] = "drv_rx_bar";
    sts_sys_prt_info[sts_drv_rx_no_qos_idx] = "drv_rx_no_qos";
    sts_sys_prt_info[sts_drv_rx_dup_idx] = "drv_rx_dup";
    sts_sys_prt_info[sts_drv_rx_ok_idx] = "drv_rx_ok";
    sts_sys_prt_info[sts_drv_rx_flush_drop_idx] = "drv_rx_flush_drop";
    sts_sys_prt_info[sts_drv_rx_bar_drop_idx] = "drv_rx_bar_drop";
    sts_sys_prt_info[sts_drv_rx_skipped_idx] = "drv_rx_skipped";
    sts_sys_prt_info[sts_drv_txlist0_qcnt_idx] = "drv_txlist0_qcnt";
    sts_sys_prt_info[sts_drv_txlist1_qcnt_idx] = "drv_txlist1_qcnt";
    sts_sys_prt_info[sts_drv_txlist2_qcnt_idx] = "drv_txlist2_qcnt";
    sts_sys_prt_info[sts_drv_txlist3_qcnt_idx] = "drv_txlist3_qcnt";
    sts_sys_prt_info[sts_drv_txlist4_qcnt_idx] = "drv_txlist4_qcnt";
    sts_sys_prt_info[sts_drv_txlist5_qcnt_idx] = "drv_txlist5_qcnt";
    sts_sys_prt_info[sts_drv_txlist6_qcnt_idx] = "drv_txlist6_qcnt";
    sts_sys_prt_info[sts_drv_txlist_qcnt_idx7] = "drv_txlist7_qcnt";
    sts_sys_prt_info[sts_drv_txlist0_pend_qcnt_idx] = "drv_txlist0_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist1_pend_qcnt_idx] = "drv_txlist1_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist2_pend_qcnt_idx] = "drv_txlist2_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist3_pend_qcnt_idx] = "drv_txlist3_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist4_pend_qcnt_idx] = "drv_txlist4_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist5_pend_qcnt_idx] = "drv_txlist5_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist6_pend_qcnt_idx] = "drv_txlist6_pend_qcnt";
    sts_sys_prt_info[sts_drv_txlist7_pend_qcnt_idx] = "drv_txlist7_pend_qcnt";
    sts_sys_prt_info[sts_drv_send_skb_idx] = "drv_send_skb_cnt";
    sts_sys_prt_info[sts_vif_ip_pkt_total_idx] = "vif_ip_pkt_total";
    sts_sys_prt_info[sts_vif_ip_pkt_to_drv_idx] = "vif_ip_pkt_to_drv";
    sts_sys_prt_info[sts_vif_ip_pkt_drop_idx] = "vif_ip_pkt_drop";
    sts_sys_prt_info[sts_vif_tx_list_full_idx] = "vif_tx_list_full";
    sts_sys_prt_info[sts_vif_tx_ok_idx] = "vif_tx_ok_idx";
    sts_sys_prt_info[sts_vif_tx_cmp_idx] = "vif_tx_cmp_idx";
    sts_sys_prt_info[sts_vif_tx_free_page_idx] = "vif_tx_free_page";
    sts_sys_prt_info[sts_vif_in_amsdu_idx] = "vif_in_amsdu";
    sts_sys_prt_info[sts_vif_out_amsdu_idx] = "vif_out_amsdu";


    //sts_sys_prt_info[sts_hst_sw_max_idx] = "sts_hst_sw_max";

}

/*
---description---
it is used to get the tag information for log showing, indexed by the address
@addr, the statistic address
*/
 char** get_tag_info(unsigned short addr, unsigned int tag_info)
{

    char** ret = NULL;

    if (tag_info == SYS_STS_PRT_INFO)
    {
        ret = sts_sys_prt_info;
        return ret;
    }

    switch (addr)
    {
        case RG_MAC_CNT_CTRL_FIQ:
            ret=   sts_mac_irq_prt_tag ;
            break;
        case RG_MAC_FRM_TYPE_CNT_CTRL :
            ret= sts_mac_11frm_prt_tag;
            break;
        case RG_MAC_TX_STT_CHK :
        case RG_MAC_TX_END_CHK:
        case RG_MAC_TX_EN_CHK:
        case RG_MAC_TX_ENWR_CHK:
            ret=  sts_mac_tx_prt_tag;
            break;

        case RG_MAC_RX_DATA_LOCAL_CNT:
        case RG_MAC_RX_DATA_OTHER_CNT:
        case RG_MAC_RX_DATA_MUTIL_CNT:
            ret = sts_mac_rx_prt_tag ;
            break;

        default:
            pr_err("%s,%d, statistic address 0x%x not supported\n", __func__, __LINE__, addr);
            break;

    }

    if(IS_AGC_STS(addr))
    {
        ret =  sts_agc_prt_tag;
    }

     if(IS_HOST_SW_STS(addr))
    {
          ret = sts_hst_sw_prt_tag;
    }
    return ret;
}

unsigned int val_to_idx(unsigned int idx_val[], unsigned int val)
{
    int i = 0;
    int ret = 0;
    for (i = 0; i < sts_hst_sw_max_idx; i++ )
        {
            if(idx_val[i] == val)
            {
                ret =  i;
                //pr_debug("idx_val[%d] is 0x%x, val is 0x%x\n", i, idx_val[i], val);
                break;
            }
            else
            {
                continue;
            }
        }

      if(i ==  sts_hst_sw_max_idx)
        {
            pr_err("%s,%d: no find the item in statistic system: val = 0x%x max i = %d \n", __func__, __LINE__, val, i);
        }
      return ret;
}

void sts_sw_probe(unsigned int sts_idx, unsigned int unit)
{
   struct hal_private* hal_priv = hal_get_priv();
   struct  sts_sw_cnt_ctrl* s_cnt_ctrl = &hal_priv->sts_hst_sw[sts_idx];
    if(s_cnt_ctrl->func_code ==  SYS_STS_START)
    {
       s_cnt_ctrl->cnt+= unit;
    }
}


/*
---description---
it is used to set the default configuration for the statistic register.
@(void)
*/
void sts_default_cfg(struct sts_cfg_data* cfg_data, unsigned char sts_sys_type)
{
     int i = 0;
     int apd_idx = 0;

     sts_prt_info_map();

     cfg_data->addr[0] = RG_MAC_CNT_CTRL_FIQ;
     cfg_data->addr[1] = RG_MAC_FRM_TYPE_CNT_CTRL;
     cfg_data->addr[2] = RG_MAC_TX_STT_CHK;
     cfg_data->addr[3] = RG_MAC_TX_END_CHK;
     cfg_data->addr[4] = RG_MAC_TX_EN_CHK;
     cfg_data->addr[5] = RG_MAC_TX_ENWR_CHK;
     cfg_data->addr[6] = RG_MAC_RX_DATA_LOCAL_CNT;
     cfg_data->addr[7] = RG_MAC_RX_DATA_OTHER_CNT;
     cfg_data->addr[8] = RG_MAC_RX_DATA_MUTIL_CNT;
     cfg_data->addr[9] = HOST_SW_VADDR;
     apd_idx =  cfg_data->cfg_num = 10;

    /*idx val*/
     cfg_data->cfg_ie[0].op_idx.idx0 = IDX_TXEND_IT;
     cfg_data->cfg_ie[0].op_idx.idx1 = IDX_RXEND_IT;
     cfg_data->cfg_ie[0].op_idx.idx2 = IDX_ACKTO_IT;
     cfg_data->cfg_ie[0].op_idx.idx3 = IDX_FCSERR;

     cfg_data->cfg_ie[1].op_idx.idx0 = IDX_QOS_DATA;
     cfg_data->cfg_ie[1].op_idx.idx1 = IDX_RTS;
     cfg_data->cfg_ie[1].op_idx.idx2 = IDX_CTS;
     cfg_data->cfg_ie[1].op_idx.idx3 = IDX_ACK;

     cfg_data->cfg_ie[6].rx_local_cnt.data_local_cnt_qos = 1;

    for(i = 0 ; i < sizeof(sts_man) / sizeof(char*) ; i++)
    {
        pr_debug("%s\n",sts_man[i]);
    }

    pr_debug("-----------the default configuration is-----------\n");
    for(i = 0; i<apd_idx; i++)
    {
        pr_debug("addr 0x%4x: data_0 0x%x data_1 0x%x data_2 0x%x data_3 0x%x\n",
            cfg_data->addr[i],
            cfg_data->cfg_ie[i].x&0xff, (cfg_data->cfg_ie[i].x>>8)&0xff,(cfg_data->cfg_ie[i].x>>16)&0xff,(cfg_data->cfg_ie[i].x>>24)&0xff
            );
    }

    sts_opt_by_cfg(cfg_data, SYS_STS_CLEAR);
    sts_opt_by_cfg(cfg_data, SYS_STS_START);
}

/*
---description---
it is used to update the default configuration for the statistic register.
@data: update information,
@len: update information length
1)check the registers statistic pool
2)look up the registers to update, update the exist one  or append the new one
*/
void sts_update_cfg(struct sts_cfg_data* cfg_data, const char* data,unsigned int len)
{
    int i = 0;

    struct sts_iw_if* iw_cfg_if = NULL;
    unsigned int apd_idx = cfg_data->cfg_num;
    ASSERT(data);
    ASSERT(cfg_data)
    ASSERT(len != 0);

    pr_debug("input: \n");
    for (i = 0; i< len; i++)
        {
            pr_debug ("0x%x ",*(data+i));
        }
    pr_debug(" \n");

    iw_cfg_if = (struct sts_iw_if*)data;

    iw_cfg_if->addr = ntohs(iw_cfg_if->addr);
    iw_cfg_if->val = ntohl(iw_cfg_if->val);

    pr_debug("addr: 0x%4x val: 0x%x \n",  iw_cfg_if->addr, iw_cfg_if->val);

    /*avoid the error address to result in system crash*/
    for(i = 0; i < sizeof(sts_reg_chk_pool)/sizeof(sts_reg_chk_pool[0]); i++)
        {
            if ( iw_cfg_if->addr != sts_reg_chk_pool[i])
                {
                    continue;
                }
                else
                {
                    break;
                }
        }

        if(i == sizeof(sts_reg_chk_pool)/sizeof(sts_reg_chk_pool[0]))
        {
            pr_debug("%s,%d: the statistic address 0x%x error, check it please...\n", __func__, __LINE__, iw_cfg_if->addr);
            return;
        }

       pr_debug("-----------the previous configuration is-----------\n");
       for(i = 0; i<apd_idx; i++)
        {
            pr_debug("addr 0x%4x: data_0 0x%x data_1 0x%x data_2 0x%x data_3 0x%x\n",
                cfg_data->addr[i],
                cfg_data->cfg_ie[i].x&0xff,(cfg_data->cfg_ie[i].x>>8)&0xff,(cfg_data->cfg_ie[i].x>>16)&0xff,(cfg_data->cfg_ie[i].x>>24)&0xff
                );
        }

        sts_opt_by_cfg(cfg_data, SYS_STS_READ);

     /*update  the add/val*/
    for(i =0; i <  apd_idx; i++)
    {
        if(iw_cfg_if->addr ==cfg_data->addr[i])
        {
             cfg_data->cfg_ie[i].x = iw_cfg_if->val;
            break;
        }
    }

      /*append the add/val*/
    if(i == apd_idx)
    {
        cfg_data->addr[apd_idx] = iw_cfg_if->addr;
        cfg_data->cfg_ie[apd_idx].x= iw_cfg_if->val;
        cfg_data->cfg_num++;
    }

    pr_debug("-----------the current configuration is-----------\n");
   for(i = 0; i<apd_idx; i++)
    {
        pr_debug("addr 0x%4x: data_0 0x%x data_1 0x%x data_2 0x%x data_3 0x%x\n",
            cfg_data->addr[i],
            cfg_data->cfg_ie[i].x&0xff,
            (cfg_data->cfg_ie[i].x>>8)&0xff,
            (cfg_data->cfg_ie[i].x>>16)&0xff,
            (cfg_data->cfg_ie[i].x>>24)&0xff);
    }

    sts_opt_by_cfg(cfg_data, SYS_STS_CLEAR);
    sts_opt_by_cfg(cfg_data, SYS_STS_START);
}


/*
---description---
it is used to clear statistic register by address
@addr: the statistic register address
*/
void sts_clr_cnt(unsigned short addr)
{
    int i = 0;
    int chg_data = 0;
    int bk_data = 0;
    struct  cnt_ctrl_bits* cnt_ctrl = NULL;
    struct  tx_cnt_ctrl_bits* tx_cnt_ctrl = NULL;
    struct data_rx_local_cnt_bits* data_rx_local_cnt = NULL;
    struct data_rx_cnt_bits* data_rx_cnt = NULL;

    struct hw_interface* hif = hif_get_hw_interface();
    struct hal_private* hal_priv = hal_get_priv();

    chg_data = hif->hif_ops.hi_read_word(addr);
    bk_data = chg_data;

    switch (addr) {
        case RG_MAC_CNT_CTRL_FIQ:
        case RG_MAC_FRM_TYPE_CNT_CTRL:
            cnt_ctrl = (struct  cnt_ctrl_bits*)&chg_data;
            cnt_ctrl->en0 = 1;
            cnt_ctrl->en1 = 1;
            cnt_ctrl->en2 = 1;
            cnt_ctrl->en3 = 1;

            cnt_ctrl->clr0 = 1;
            cnt_ctrl->clr1 = 1;
            cnt_ctrl->clr2 = 1;
            cnt_ctrl->clr3 = 1;

            hif->hif_ops.hi_write_word(addr , chg_data);

            /*recover the previous setting */
            chg_data = bk_data;

            cnt_ctrl->clr0 = 0;
            cnt_ctrl->clr1 = 0;
            cnt_ctrl->clr2 = 0;
            cnt_ctrl->clr3 = 0;
            break;

        case RG_MAC_TX_STT_CHK:
        case RG_MAC_TX_END_CHK:
        case RG_MAC_TX_EN_CHK:
        case RG_MAC_TX_ENWR_CHK:
            tx_cnt_ctrl = (struct tx_cnt_ctrl_bits*)&chg_data;
            tx_cnt_ctrl->en = 1;
            tx_cnt_ctrl->clr = 1;

            hif->hif_ops.hi_write_word(addr , chg_data);

            chg_data = bk_data;
            tx_cnt_ctrl->clr = 0;
            break;

        case RG_MAC_RX_DATA_LOCAL_CNT:
            data_rx_local_cnt = ( struct data_rx_local_cnt_bits*)&chg_data;
            data_rx_local_cnt->data_local_cnt_en = 1;
            data_rx_local_cnt->data_local_cnt_clr = 1;

            hif->hif_ops.hi_write_word(addr , chg_data);

            chg_data = bk_data;
            data_rx_local_cnt->data_local_cnt_clr = 0;
            break;

        case RG_MAC_RX_DATA_OTHER_CNT:
        case RG_MAC_RX_DATA_MUTIL_CNT:
            data_rx_cnt = (struct data_rx_cnt_bits*)&chg_data;
            data_rx_cnt->en = 1;
            data_rx_cnt->clr = 1;

            hif->hif_ops.hi_write_word(addr , chg_data);

            chg_data = bk_data;
            data_rx_cnt->clr = 0;
            break;

        default:
            pr_err("%s,%d, statistic address 0x%x not supported\n", __func__, __LINE__, addr);
            break;
   }


   hif->hif_ops.hi_write_word(addr , chg_data);

   if(IS_HOST_SW_STS(addr)) {
      for(i= sts_hst_irq_tx_error_idx;  i < sts_hst_sw_max_idx; i++) {
         hal_priv->sts_hst_sw[i].cnt = 0;
      }
   }
}


/*
---description---
it is used to start a statistic by the index code
@unsigned short addr: which register to do statistic
@char obj_0, index 0
@char obj_1, index 1
@char obj_2, index 2
@char obj_3  index 3
*/
void sts_start_cnt(unsigned short addr,
                             char obj_0,char obj_1,char obj_2,char obj_3)
{
    int i = 0;
    struct  cnt_ctrl_bits* cnt_irq_ctrl = NULL;
    struct  tx_cnt_ctrl_bits* tx_cnt_ctrl = NULL;

    unsigned int chg_data = 0;
    struct hal_private* hal_priv = hal_get_priv();

    struct hw_interface* hif = hif_get_hw_interface();
    struct data_rx_local_cnt_bits* data_rx_local_cnt = NULL;
    struct data_rx_cnt_bits* data_rx_cnt = NULL;
    /**for irq status cnt**/
    chg_data = hif->hif_ops.hi_read_word(addr);

    switch (addr)
    {
        case RG_MAC_CNT_CTRL_FIQ:
        case RG_MAC_FRM_TYPE_CNT_CTRL:
            //set the frame type index
            cnt_irq_ctrl = (struct cnt_ctrl_bits*)&chg_data;

            cnt_irq_ctrl->type_idx0 =  obj_0;
            cnt_irq_ctrl->type_idx1 =  obj_1;
            cnt_irq_ctrl->type_idx2 =  obj_2;
            cnt_irq_ctrl->type_idx3 =  obj_3;

            cnt_irq_ctrl->en0=   1;
            cnt_irq_ctrl->en1 =  1;
            cnt_irq_ctrl->en2 =  1;
            cnt_irq_ctrl->en3 =  1;

            cnt_irq_ctrl->clr0=   0;
            cnt_irq_ctrl->clr1 =  0;
            cnt_irq_ctrl->clr2 =  0;
            cnt_irq_ctrl->clr3 =  0;
            break;

        case RG_MAC_TX_STT_CHK:
        case RG_MAC_TX_END_CHK:
        case RG_MAC_TX_EN_CHK:
        case RG_MAC_TX_ENWR_CHK:
            tx_cnt_ctrl = (struct tx_cnt_ctrl_bits*)&chg_data;
            tx_cnt_ctrl->en = 1;
            tx_cnt_ctrl->clr = 0;
            break;

         case RG_MAC_RX_DATA_LOCAL_CNT:
            data_rx_local_cnt = ( struct data_rx_local_cnt_bits*)&chg_data;
            data_rx_local_cnt->data_local_cnt_qos = 1;
            data_rx_local_cnt->data_local_cnt_en = 1;
            data_rx_local_cnt->data_local_cnt_clr = 0;
            break;

        case RG_MAC_RX_DATA_OTHER_CNT:
        case RG_MAC_RX_DATA_MUTIL_CNT:
            data_rx_cnt = (struct data_rx_cnt_bits*)&chg_data;
            data_rx_cnt->en = 1;
            data_rx_cnt->clr = 0;
            break;

        default:
            pr_err("%s,%d: statistic address 0x%x not supported\n", __func__, __LINE__,addr);
            break;
    }

    // start to the counter
    hif->hif_ops.hi_write_word(addr , chg_data);

    if(IS_HOST_SW_STS(addr))
    {
        for(i= sts_hst_irq_tx_error_idx;  i < sts_hst_sw_max_idx; i++)
        {
            hal_priv->sts_hst_sw[i].func_code = SYS_STS_START;
        }
    }

}


void sts_prt_agc_field(unsigned short addr,
                         char* sts_tag[], char* sts_info[], char func_code)
{
    unsigned int rd_data0 = 0;
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned short agc_rd1 = 0;
    unsigned short agc_rd2 = 0;

    unsigned int prt_idx1 = 0;
    unsigned int prt_idx2 = 0;

    union sts_agc_reg_u* agc_u = NULL;

        rd_data0 = hif->hif_ops.hi_read_word(addr);

        agc_u = (union sts_agc_reg_u*)&rd_data0;

        switch (addr)
            {
                case RG_AGC_STATE:
                    agc_rd1 =agc_u->state.state;
                    agc_rd2 = agc_u->state.state;
                    prt_idx1 = prt_idx2= val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_st_idx);
                    break;

                case RG_AGC_OB_IC_GAIN:
                    agc_rd1 =agc_u->ic_gain.r_rx_ic_gain;
                    agc_rd2 =agc_u->ic_gain.r_agc_fine_db;

                    prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_gain_rx_ic_gain_idx);
                    prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_gain_fine_db_idx);
                    break;

                case RG_AGC_WATCH1:
                    agc_rd1 =agc_u->wtc1.r_pow_sat_db;
                    agc_rd2 =agc_u->wtc1.r_primary20_db;

                    prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc1_pw_sat_db_idx);
                    prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc1_prm20_db_idx);
                    break;

                case RG_AGC_WATCH2:
                 agc_rd1 =agc_u->wtc2.r_primary40_db;
                 agc_rd2 =agc_u->wtc2.r_primary80_db;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc2_prm40_db_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc2_prm80_db_idx);
                  break;

                case RG_AGC_WATCH3:
                 agc_rd1 =agc_u->wtc3.r_rssi0_db;
                 agc_rd2 =agc_u->wtc3.r_rssi1_db;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc3_rssi0_db_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc3_rssi1_db_idx);
                    break;

                case RG_AGC_WATCH4:
                 agc_rd1 =agc_u->wtc4.r_rssi2_db;
                 agc_rd2 =agc_u->wtc4.r_rssi3_db;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc4_rssi2_db_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_wtc4_rssi3_db_idx);
                    break;

                case RG_AGC_OB_ANT_NFLOOR:
                 agc_rd1 =agc_u->ant_nfloor.r_snr_qdb;
                 agc_rd2 =agc_u->ant_nfloor.r_ant_nfloor_qdb;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_ant_flr_snr_qdb_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_agc_ant_flr_ant_flr_qdb_idx);
                    break;

                case RG_AGC_OB_CCA_RES:
                 agc_rd1 =agc_u->cca_res.cca_cond_rdy;
                 agc_rd2 =agc_u->cca_res.cca_cond8;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_cca_res_rdy_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_cca_res_cond8_idx);
                    break;

                case RG_AGC_OB_CCA_COND01:
                 agc_rd1 =agc_u->cca_cond_m1.cca_cond0;
                 agc_rd2 =agc_u->cca_cond_m1.cca_cond1;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_cca_cnd01_cond0_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_cca_cnd01_cond1_idx);
                    break;

                case RG_AGC_OB_CCA_COND23:
                 agc_rd1 =agc_u->cca_cond_m2.cca_cond2;
                 agc_rd2 =agc_u->cca_cond_m2.cca_cond3;
                 prt_idx1 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_cca_cnd23_cond2_idx);
                 prt_idx2 = val_to_idx(sts_sys_idx_val,addr << 16 | sts_cca_cnd23_cond3_idx);
                    break;
                default:
                    pr_err("%s,%d agc addr 0x%x err:\n", __func__, __LINE__, addr);
                    return;
            }
     pr_debug("[%s]: %s=%d, %s=%d\n",
     func_code==SYS_STS_STOP ? sts_tag[SYS_STS_STOP]:sts_tag[SYS_STS_READ],
     sts_info[prt_idx1], agc_rd1,sts_info[prt_idx2], agc_rd2);

}



void sts_gen_prt_info(unsigned short addr,
                         unsigned int idx_start, unsigned idx_end,
                         char* sts_tag[], char* sts_info[], unsigned int *idx_val, char func_code)
{
    int i = 0;
    unsigned int prt_idx = 0;
    struct hal_private* hal_priv = hal_get_priv();

    for(i = idx_start; i < idx_end; i++)
            {
                prt_idx = val_to_idx(idx_val, (addr << 16) | i );

                if(sts_info[prt_idx] != NULL)
                    {
                         pr_debug("[%s]: %s=%d \n",
                         func_code==SYS_STS_STOP ?  sts_tag[SYS_STS_STOP]: sts_tag[SYS_STS_READ],
                         sts_info[prt_idx] ,hal_priv->sts_hst_sw[i].cnt);
                    }
            }
}

void sts_read_reg_array(unsigned int* rd0,unsigned int* rd1, unsigned int* rd2, unsigned int* rd3,
                                            unsigned short reg0,unsigned short reg1,unsigned short reg2,unsigned short reg3)
{
        struct hw_interface* hif = hif_get_hw_interface();

        *rd0 =  hif->hif_ops.hi_read_word(reg0);

        *rd1 = hif->hif_ops.hi_read_word(reg1);

        *rd2 = hif->hif_ops.hi_read_word(reg2);

        *rd3 = hif->hif_ops.hi_read_word(reg3);
}

/*
---description---
it is used to finish a statistic by the index code
@addr: which register to read statistic result
@sts_info[], statistic to show
@char func_code,   funtion code, 0: stop and read; 1, only read
@char obj_0, index 0
@char obj_1, index 1
@char obj_2, index 2
@char obj_3  index 3
*/
void sts_read_cnt(unsigned short addr,
                         unsigned char obj_0,unsigned char obj_1,unsigned char obj_2, unsigned char obj_3,
                         char* sts_tag[], char* sts_info[], unsigned int* idx_val, char func_code)
{
    unsigned int rd_data0 = 0;
    unsigned int rd_data1 = 0;
    unsigned int rd_data2 = 0;
    unsigned int rd_data3 = 0;
    unsigned int rd_show = 0;

    unsigned int prt_idx0 = 0;
    unsigned int prt_idx1 = 0;
    unsigned int prt_idx2 = 0;
    unsigned int prt_idx3 = 0;
    int i = 0;

    struct hw_interface* hif = hif_get_hw_interface();
    struct hal_private * hal_priv = hal_get_priv();

    switch (addr)
    {
        case RG_MAC_CNT_CTRL_FIQ:
        {
            prt_idx0 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_0);
            prt_idx1 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_1);
            prt_idx2 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_2);
            prt_idx3 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_3);

            sts_read_reg_array(&rd_data0,&rd_data1,&rd_data2,&rd_data3,
                RG_MAC_IRQ_STATUS_CNT0,RG_MAC_IRQ_STATUS_CNT1,RG_MAC_IRQ_STATUS_CNT2,RG_MAC_IRQ_STATUS_CNT3);

             pr_debug("[%s]: %s=%8d,%s=%8d,%s=%8d,%s=%8d\n",
                func_code==SYS_STS_STOP ? sts_tag[SYS_STS_STOP]:sts_tag[SYS_STS_READ],
                sts_info[prt_idx0],rd_data0,
                sts_info[prt_idx1],rd_data1,
                sts_info[prt_idx2],rd_data2,
                sts_info[prt_idx3],rd_data3);
        }
        break;

        case RG_MAC_FRM_TYPE_CNT_CTRL:
        {
            prt_idx0 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_0);
            prt_idx1 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_1);
            prt_idx2 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_2);
            prt_idx3 = val_to_idx(idx_val, (unsigned int)addr << 16 | (unsigned int)obj_3);


             sts_read_reg_array(&rd_data0,&rd_data1,&rd_data2,&rd_data3,
                RG_MAC_FRM_TYPE_CNT0,RG_MAC_FRM_TYPE_CNT1,RG_MAC_FRM_TYPE_CNT2,RG_MAC_FRM_TYPE_CNT3);

            pr_debug("[%s]: %s=%8d,%s=%8d,%s=%8d,%s=%8d\n",
            func_code==SYS_STS_STOP ? sts_tag[SYS_STS_STOP]:sts_tag[SYS_STS_READ],
            sts_info[prt_idx0],rd_data0,
            sts_info[prt_idx1],rd_data1,
            sts_info[prt_idx2],rd_data2,
            sts_info[prt_idx3],rd_data3
            );
        }
        break;

        case RG_MAC_TX_STT_CHK:
        case RG_MAC_TX_END_CHK:
        case RG_MAC_TX_EN_CHK:
        case RG_MAC_TX_ENWR_CHK:

            prt_idx0 = val_to_idx(idx_val, addr);

             rd_data0 = hif->hif_ops.hi_read_word(addr);

             pr_debug("[%s]: %s=%8d\n",
             func_code==SYS_STS_STOP ? sts_tag[SYS_STS_STOP]:sts_tag[SYS_STS_READ],
             sts_info[prt_idx0 ],(rd_data0 << 6 ) >> 6);
        break;

        case RG_MAC_RX_DATA_LOCAL_CNT:
        case RG_MAC_RX_DATA_OTHER_CNT:
        case RG_MAC_RX_DATA_MUTIL_CNT:
            prt_idx0 = val_to_idx(idx_val, addr);
            rd_data0 = hif->hif_ops.hi_read_word(addr);
            if(addr == RG_MAC_RX_DATA_LOCAL_CNT)
            {
                rd_show = (rd_data0 << 3 ) >> 3;
            }
            else
            {
                rd_show = (rd_data0 << 2 ) >> 2;
            }

             pr_debug("[%s]: %s=%8d\n",
             func_code==SYS_STS_STOP ? sts_tag[SYS_STS_STOP]:sts_tag[SYS_STS_READ],
             sts_info[prt_idx0],rd_show);
            break;
        default:
             pr_err("%s,%d, statistic address 0x%x not supported\n", __func__, __LINE__, addr);
            break;
    }

    if(IS_AGC_STS(addr))
        {
           sts_prt_agc_field(addr, sts_tag, sts_info,  func_code);
        }

    if(IS_HOST_SW_STS(addr) )
        {
              for(i = hirq_tx_err_idx; i < hirq_max_idx; i++)
                {
                        hal_priv->sts_hst_sw[sts_hst_irq_tx_error_idx + i].cnt = hal_priv->sts_hirq[i];
                }
             sts_gen_prt_info(addr,
                                       sts_hst_irq_tx_error_idx,
                                       sts_hst_sw_max_idx,
                                       sts_tag, sts_info,  idx_val, func_code );
        }
}

/*
---description---
it is used to stop statistic register by address
@addr: the statistic register address
*/
void sts_stop_cnt(unsigned short addr)
{
    int i = 0;
    unsigned int chg_data = 0;
    struct  cnt_ctrl_bits* cnt_ctrl = NULL;
    struct  tx_cnt_ctrl_bits* tx_cnt_ctrl = NULL;
    struct data_rx_local_cnt_bits* data_rx_local_cnt = NULL;
    struct data_rx_cnt_bits* data_rx_cnt = NULL;

    struct hw_interface* hif = hif_get_hw_interface();
    struct hal_private* hal_priv = hal_get_priv();

    chg_data = hif->hif_ops.hi_read_word(addr);

    //to set clear bit
    switch (addr )
    {
        case RG_MAC_CNT_CTRL_FIQ:
        case RG_MAC_FRM_TYPE_CNT_CTRL:
            cnt_ctrl = (struct  cnt_ctrl_bits*)&chg_data;
            cnt_ctrl->en0 = 0;
            cnt_ctrl->en1 = 0;
            cnt_ctrl->en2 = 0;
            cnt_ctrl->en3 = 0;

            cnt_ctrl->clr0 = 0;
            cnt_ctrl->clr1 = 0;
            cnt_ctrl->clr2 = 0;
            cnt_ctrl->clr3 = 0;
            break;

        case RG_MAC_TX_STT_CHK:
        case RG_MAC_TX_END_CHK:
        case RG_MAC_TX_EN_CHK:
        case RG_MAC_TX_ENWR_CHK:
            tx_cnt_ctrl = (struct tx_cnt_ctrl_bits*)&chg_data;
            tx_cnt_ctrl->en = 0;
            tx_cnt_ctrl->clr = 0;
            break;

        case RG_MAC_RX_DATA_LOCAL_CNT:
            data_rx_local_cnt = ( struct data_rx_local_cnt_bits*)&chg_data;
            data_rx_local_cnt->data_local_cnt_en = 0;
            data_rx_local_cnt->data_local_cnt_clr = 0;
            break;

        case RG_MAC_RX_DATA_OTHER_CNT:
        case RG_MAC_RX_DATA_MUTIL_CNT:
            data_rx_cnt = ( struct data_rx_cnt_bits*)&chg_data;
            data_rx_cnt->en = 0;
            data_rx_cnt->clr = 0;
            break;

        default:
            pr_err("%s,%d: statistic address 0x%x not supported\n", __func__, __LINE__, addr);
            break;

    }

    hif->hif_ops.hi_write_word(addr , chg_data);

    if(IS_HOST_SW_STS(addr))
    {
        for(i= sts_hst_irq_tx_error_idx;  i < sts_hst_sw_max_idx; i++)
        {
            hal_priv->sts_hst_sw[i].func_code = SYS_STS_STOP;
        }
    }
}


void sts_show_cfg(  struct sts_cfg_data* cfg_data)
{
    int i = 0;
    pr_debug("-----------the current configuration is-----------\n");
   for(i = 0; i<cfg_data->cfg_num; i++)
    {
        pr_debug("addr 0x%4x: data_a 0x%x data_b 0x%x data_c 0x%x data_d 0x%x\n",
            cfg_data->addr[i],
            cfg_data->cfg_ie[i].x&0xff,(cfg_data->cfg_ie[i].x>>8)&0xff,(cfg_data->cfg_ie[i].x>>16)&0xff,(cfg_data->cfg_ie[i].x>>24)&0xff
            );
    }
}

/*
---description---
it is used to operate the statistic register by the address
@func_code: the statistic register operation code=>clear:0/start:1/read:2/stop:3
*/

void sts_opt_by_cfg(struct sts_cfg_data* cfg_data, unsigned  char func_code)
{
    unsigned int i = 0;
    char **tag ;
    char **info;
    pr_debug("%s\n--------function code is 0x%x--------\n", func_prt_info[func_code], func_code);

   if(func_code == SYS_STS_SHOW_CFG )
    {
            sts_show_cfg(cfg_data);
            return;
    }

    for(i = 0; i < cfg_data->cfg_num; i++)
    {
        switch(func_code)
            {
                case SYS_STS_CLEAR:
                    sts_clr_cnt(cfg_data->addr[i]);
                 break;

                case SYS_STS_STOP:
                    sts_stop_cnt(cfg_data->addr[i]);
                break;

                case SYS_STS_START:
                    sts_start_cnt(cfg_data->addr[i],
                                cfg_data->cfg_ie[i].op_idx.idx0,cfg_data->cfg_ie[i].op_idx.idx1,
                                cfg_data->cfg_ie[i].op_idx.idx2,cfg_data->cfg_ie[i].op_idx.idx3);
                break;

                case SYS_STS_READ:
                   tag = (char**)get_tag_info(cfg_data->addr[i],SYS_STS_PRT_TAG);
                   info = (char**)get_tag_info(cfg_data->addr[i],SYS_STS_PRT_INFO);

                   if(tag != NULL && info!= NULL)
                    {
                        sts_read_cnt(cfg_data->addr[i] ,
                                           cfg_data->cfg_ie[i].op_idx.idx0,cfg_data->cfg_ie[i].op_idx.idx1,
                                           cfg_data->cfg_ie[i].op_idx.idx2,cfg_data->cfg_ie[i].op_idx.idx3,
                                           tag,info,sts_sys_idx_val, func_code);
                    }
                    else
                        {
                            pr_err("%s, %d: ptr err: tag %p info %p\n", __func__, __LINE__, tag, info);
                        }
                break;
                default :
                    ERROR_DEBUG_OUT("not support the function code 0x%x\n", func_code);
                break;
            }
    }
}

#if 0
static unsigned int _get_tv_us (const struct timeval  *tvnew, const struct timeval  *tvold)
{
    return ((tvnew->tv_sec-tvold->tv_sec)*1000000+(tvnew->tv_usec - tvold->tv_usec));
}
#endif

