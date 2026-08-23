#ifndef __DRV_STATISTIC_H
#define __DRV_STATISTIC_H

/////////////////////////////// global statistic defination////////////////////////

#define SYS_STS_CLEAR  (0)
#define SYS_STS_START (1)
#define SYS_STS_READ   (2)
#define SYS_STS_STOP   (3)
#define SYS_STS_SHOW_CFG (4)

#define IDX_TXSTART_IT          (0)
#define IDX_TXEND_IT              (1)
#define IDX_TXENDERR             (8)
#define IDX_ACKTO_IT              (9)
#define IDX_TXCOL_FLAG          (10)
#define IDX_RXSTART_IT           (11)
#define IDX_RXEND_IT              (12)
#define IDX_RXMPDU_OK           (13)
#define IDX_UNICAST                (14)
#define IDX_SRAMBUF_FULL       (18)
#define IDX_SWIRQ                  (19)
#define IDX_TX_FRAME_TYPE      (20)
#define IDX_ABSCOUNT_INTR     (30)
#define IDX_FIQ_ENABLE           (31)
                                
#define IDX_FIAC_FW_TIMEOUT_ERR  (32+0)
#define IDX_HW_TIMEOUT_ERR       (32+1)
#define IDX_TX_WD_TIMEOUT_ERR    (32+2)
#define IDX_TXINVALIDLEN_ERR     (32+3)
#define IDX_TX_KEYMISS           (32+4)
#define IDX_TXIMSTOP_ERR         (32+5)
#define IDX_TX_PHYERROR          (32+6)
#define IDX_TXCOL_QUEUE          (32+10)
#define IDX_TX_NOT_GRANT         (32+19)
#define IDX_SRAMBUF_FULL_ERR   (32+22)
#define IDX_FCSERR               (32+23)
#define IDX_RXFIFOOV             (32+24)
#define IDX_RX_WD_TIMEOUT_ERR    (32+25)
#define IDX_SHORT_LENGTH         (32+26)
#define IDX_LENGTH_ERROR         (32+27)
#define IDX_RX_PHYERROR          (32+28)
                              
#define IDX_QOS_DATA        (0x28)
#define IDX_RTS                  (0x1b)
#define IDX_CTS                  (0x1c)
#define IDX_ACK                  (0x1d)


#define STS_MAX_CFG_FILE  (0x40)
#define SYS_STS_PRT_TAG (0x1)
#define SYS_STS_PRT_INFO (0x0)

#define IS_MAC_STS(addr) (((addr) == RG_MAC_CNT_CTRL_FIQ) ||\
                                               ((addr) == RG_MAC_FRM_TYPE_CNT_CTRL) ||\
                                               ((addr) == RG_MAC_TX_STT_CHK) || \
                                               ((addr) == RG_MAC_TX_END_CHK) ||\
                                               ((addr) == RG_MAC_TX_EN_CHK) ||\
                                               ((addr) == RG_MAC_TX_ENWR_CHK) ||\
                                               ((addr) == RG_MAC_RX_DATA_LOCAL_CNT) ||\
                                               ((addr) == RG_MAC_RX_DATA_OTHER_CNT) ||\
                                               ((addr) == RG_MAC_RX_DATA_MUTIL_CNT) )


#define IS_AGC_STS(addr) ((addr ) == RG_AGC_STATE||\
                                        (addr ) == RG_AGC_OB_IC_GAIN||\
                                        (addr ) == RG_AGC_WATCH1||\
                                        (addr ) == RG_AGC_WATCH2||\
                                        (addr ) == RG_AGC_WATCH3||\
                                        (addr ) == RG_AGC_WATCH4||\
                                        (addr ) == RG_AGC_OB_ANT_NFLOOR||\
                                        (addr ) == RG_AGC_OB_CCA_RES||\
                                        (addr ) == RG_AGC_OB_CCA_COND01||\
                                        (addr ) == RG_AGC_OB_CCA_COND23)

#define IS_HOST_SW_STS(addr) ((addr & MOD_VADDR) == MOD_VADDR)


enum sts_sys_idx{ 
        sts_mac_irq_txstart_idx,
        sts_mac_irq_txend_idx,
        sts_mac_irq_txenderr_idx,
        sts_mac_irq_ackto_idx,
        sts_mac_irq_txcol_flag_idx,
        sts_mac_irq_rxstart_idx,
        sts_mac_irq_mac_rxend_idx,
        sts_mac_irq_mac_rxmpdu_ok_idx,
        sts_mac_irq_unicast_idx,
        sts_mac_irq_srambuf_full_idx,
        sts_mac_irq_swirq_idx,
        sts_mac_irq_tx_frame_type_idx,
        sts_mac_irq_abscount_intr_idx,
        sts_mac_irq_fiq_enable_idx,
        sts_mac_irq_fiac_fw_timeout_err_idx,
        sts_mac_irq_hw_timeout_err_idx,
        sts_mac_irq_tx_wd_timeout_err_idx,
        sts_mac_irq_txinvalidlen_err_idx,
        sts_mac_irq_tx_keymiss_idx,
        sts_mac_irq_tximstop_err_idx,
        sts_mac_irq_tx_phyerror_idx,
        sts_mac_irq_txcol_queue_idx,
        sts_mac_irq_tx_not_grant_idx,
        sts_mac_irq_srambuf_full_err_idx,
        sts_mac_irq_fcserr_idx,
        sts_mac_irq_rxfifoov_idx,
        sts_mac_irq_rx_wd_timeout_err_idx,
        sts_mac_irq_short_length_idx,
        sts_mac_irq_length_error_idx,
        sts_mac_irq_rx_phyerror_idx,
        sts_frm_type_qos_data_idx,
        sts_frm_type_rts_idx,
        sts_frm_type_cts_idx,
        sts_frm_type_ack_idx,
        sts_mac_tx_start_chk_idx,
        sts_mac_tx_end_chk_idx,
        sts_mac_tx_en_chk_idx,
        sts_mac_tx_enwr_chk_idx,
        sts_mac_rx_data_local_cnt_idx,
        sts_mac_rx_data_other_cnt_idx,
        sts_mac_rx_data_mutil_cnt_idx,
        sts_agc_st_idx,
        sts_agc_gain_rx_ic_gain_idx,
        sts_agc_gain_fine_db_idx,
        sts_agc_wtc1_pw_sat_db_idx,
        sts_agc_wtc1_prm20_db_idx,
        sts_agc_wtc2_prm40_db_idx,
        sts_agc_wtc2_prm80_db_idx,
        sts_agc_wtc3_rssi0_db_idx, 
        sts_agc_wtc3_rssi1_db_idx,
        sts_agc_wtc4_rssi2_db_idx,
        sts_agc_wtc4_rssi3_db_idx,
        sts_agc_ant_flr_snr_qdb_idx,
        sts_agc_ant_flr_ant_flr_qdb_idx,
        sts_cca_res_rdy_idx,
        sts_cca_res_cond8_idx,
        sts_cca_cnd01_cond0_idx,
        sts_cca_cnd01_cond1_idx,
        sts_cca_cnd23_cond2_idx,
        sts_cca_cnd23_cond3_idx,   
        
        sts_hst_irq_tx_error_idx,
        sts_hst_irq_rx_error_idx,
        sts_hst_irq_rx_ok_idx,
        sts_hst_irq_tx_ok_idx,
        sts_hst_irq_beacon_send_ok_vid0_idx	,
        sts_hst_irq_beacon_send_ok_vid1_idx	,
        sts_hst_irq_goto_wakeup_vid0_idx		,
        sts_hst_irq_goto_wakeup_vid1_idx		,
        sts_hst_irq_p2p_oppps_cwend_vid0_idx,
        sts_hst_irq_p2p_oppps_cwend_vid1_idx,
        sts_hst_irq_p2p_noa_start_vid0_idx	,
        sts_hst_irq_p2p_noa_start_vid1_idx	,
        sts_hst_irq_beacon_recv_ok_vid0_idx	,
        sts_hst_irq_beacon_recv_ok_vid1_idx	,
        sts_hst_irq_beacon_miss_vid0_idx		,
        sts_hst_irq_beacon_miss_vid1_idx		,     
        
        sts_hal_agg_start_tid0_idx,
        sts_hal_agg_start_tid1_idx,
        sts_hal_agg_start_tid2_idx,
        sts_hal_agg_start_tid3_idx,
        sts_hal_agg_start_tid4_idx,
        sts_hal_agg_start_tid5_idx,
        sts_hal_agg_start_tid6_idx,
        sts_hal_agg_start_tid7_idx,
        sts_hal_agg_start_tid8_idx,
        sts_hal_agg_start_tid9_idx,
        sts_hal_rx_fifo_full_idx,
        sts_hal_rx_up_skb_idx,
        sts_hal_rx_vif_id_err_idx,
        sts_hal_rx_dcrypt_err_idx,
        sts_hal_tx_num_idx,
        sts_hal_tx_mpdus_idx,
        sts_hal_tx_pages_idx,
        sts_hal_dw_mpdu_idx, 
        sts_hal_cur_mpdu_idx,
        sts_hal_cur_agg_page_idx,
        sts_hal_allc_txds_idx,
        sts_hal_free_txds_idx,
        sts_hal_send_null_idx,
        sts_hal_keep_alive_idx,
        sts_hal_beacon_miss_idx,
        sts_drv_tx_total_bytes_idx,
        sts_drv_tx_pkts_idx, 
        sts_drv_tx_drops_idx,
        sts_drv_tx_normal_idx,
        sts_drv_tx_end_normal_idx,
        sts_drv_tx_end_ampdu_idx,
        sts_drv_tx_ampdu_idx,
        sts_drv_tx_ampdu_on_frm_idx,
        sts_drv_tx_end_fail_idx,
        sts_drv_rx_ampdu_idx,
        sts_drv_rx_bar_idx,
        sts_drv_rx_no_qos_idx,
        sts_drv_rx_dup_idx,
        sts_drv_rx_ok_idx,
        sts_drv_rx_flush_drop_idx,
        sts_drv_rx_bar_drop_idx,
        sts_drv_rx_skipped_idx,
        sts_drv_txlist0_qcnt_idx,
        sts_drv_txlist1_qcnt_idx,
        sts_drv_txlist2_qcnt_idx,
        sts_drv_txlist3_qcnt_idx,
        sts_drv_txlist4_qcnt_idx,
        sts_drv_txlist5_qcnt_idx,
        sts_drv_txlist6_qcnt_idx,
        sts_drv_txlist_qcnt_idx7,
        sts_drv_txlist0_pend_qcnt_idx,
        sts_drv_txlist1_pend_qcnt_idx,
        sts_drv_txlist2_pend_qcnt_idx,
        sts_drv_txlist3_pend_qcnt_idx,
        sts_drv_txlist4_pend_qcnt_idx,
        sts_drv_txlist5_pend_qcnt_idx,
        sts_drv_txlist6_pend_qcnt_idx,
        sts_drv_txlist7_pend_qcnt_idx,
        sts_drv_send_skb_idx,
        sts_vif_ip_pkt_total_idx,
        sts_vif_ip_pkt_to_drv_idx,
        sts_vif_ip_pkt_drop_idx,
        sts_vif_tx_list_full_idx,
        sts_vif_tx_ok_idx,
        sts_vif_tx_cmp_idx,
        sts_vif_tx_free_page_idx,
        sts_vif_in_amsdu_idx,
        sts_vif_out_amsdu_idx,




        sts_hst_sw_max_idx,
}; 

/*
    iw dev wlan0 vendor send 0xc3 0xc4  op_code func_code hw_addr obj_0, obj_1, obj_2,obj_3
*/
#pragma pack(push ,1)
struct sts_cfg_data{
      unsigned short addr[STS_MAX_CFG_FILE];
      union sts_mac_reg_u cfg_ie[STS_MAX_CFG_FILE];
      unsigned char cfg_num;
};

struct statistic_cmd
{
    unsigned char op_code;    // 0x99
    
    unsigned char iaddr_high; // hardware reg high part, 
    unsigned char iaddr_low;  // hardware reg low part
    unsigned char iobj_0;       // obj_0 to work
    unsigned char iobj_1;       // obj_1 to work
    unsigned char iobj_2;      // obj_2 to work
    unsigned char iobj_3;     // obj_3 to work

    unsigned char faddr_high; // hardware reg high part, 
    unsigned char faddr_low;  // hardware reg low part
    unsigned char fobj_0;       // obj_0 to work
    unsigned char fobj_1;       // obj_1 to work
    unsigned char fobj_2;      // obj_2 to work
    unsigned char fobj_3;     // obj_3 to work

    unsigned char func_code; //(0x0, stop); (0x1, start) ;(0x2, read)
    
};


struct sts_iw_if
    {
        unsigned char op_code;
        unsigned short addr;
        unsigned int val;
    };

#pragma  pack(pop)

void sts_sw_probe(unsigned int sts_idx, unsigned int unit) ;
void phy_stc(void);
int get_snr(void);
void get_phy_stc_info(unsigned int *arr);

unsigned int cca_busy_check(void);
void sts_prt_info_map(void);
void sts_default_cfg(struct sts_cfg_data* cfg_data, unsigned char sts_sys_type);
void sts_update_cfg(struct sts_cfg_data* cfg_data, const char* data,unsigned int len);
char** get_tag_info(unsigned short addr,unsigned int tag_info);
void sts_clr_cnt(unsigned short addr);
void sts_start_cnt(unsigned short addr,
                             char obj_0,char obj_1,char obj_2,char obj_3);

void sts_read_cnt(unsigned short addr, 
                         unsigned char obj_0,unsigned char obj_1,unsigned char obj_2, unsigned char obj_3,
                         char* sts_tag[], char* sts_info[], unsigned int* idx_val, char func_code);

void sts_stop_cnt(unsigned short addr);
void sts_show_cfg(  struct sts_cfg_data* cfg_data);
void sts_opt_by_cfg(struct sts_cfg_data* cfg_data, unsigned  char func_code);
void sts_prt_agc_field(unsigned short addr, 
                         char* sts_tag[], char* sts_info[], char func_code);

unsigned int val_to_idx(unsigned int idx_val[], unsigned int val);

/////////////////////////////// layer statistic defination////////////////////////
/*
    iw dev wlan0 vendor send 0xc3 0xc4  op_code(B) ctrl_msg3, ctrl_msg2, ctrl_msg1, ctrl_msg0
    bit31~bit28: sts module: bit31 wifi_mac layer; bit30 driver layer; bit29 hal layer; bit28 hw  interface 
    bit27~bit24: sts type:  bit27 tx; bit26 rx; bit25 allocate resource; bit24 free resource
    bit23~bit0: the detail message by sts type and sts module
*/

#define STS_MOD_WMAC        BIT(31)
#define STS_MOD_DRV           BIT(30)
#define STS_MOD_HAL            BIT(29)
#define STS_MOD_HIF             BIT(28)

#define STS_TYP_TX                (BIT(27))
#define STS_TYP_RX                (BIT(26))

// RF capture

#define INBUFFER_LEN_VALID_BIT 			(256-2)
#define INBUFFER_LEN 				(256/8)
#define ADC_WIDTH               		24
#define BLOCK_RAW_LEN       			INBUFFER_LEN
#define BLOCK_REAL_LEN       			(INBUFFER_LEN_VALID_BIT/ADC_WIDTH*4)
#define BLOCK_REAL_COUNT_WORD    		(BLOCK_REAL_LEN/4)

#define TESTBUSSRAM_LEN 			0x200000
#define INBUFFER_LEN 				(256/8)
#define INBUFFER_LEN_VALID_BIT 			(256-2)
#define INBUFFER_LEN_VALID 			((INBUFFER_LEN_VALID_BIT)/8)
#define TESTBUFFER_LEN				(256-8)

#define SELFTEST 				1  //LG
#define DA 					0	//LG

#define SW					9
#define FCS_OK					6
#define FCS_ERR					3
#define FCS_ERR_OFDM				5
#define FCS_ERR_DSS				4
#define LSIG_PARITY_ERR	        		8
#define HTSIG_CRC_ERR				7
#define CCA_FSM_TRIG				2
#define AGC_FSM_TRIG				0
#define CCA_AGC_FSM_TRIG			1


#define SDIO_BASE_A		  		   0x00a0b000
#define CAP_SRAM_SWITCH_BASE	   0x00a0d000

#define TBC_OFFSET_100				(0x100 + SDIO_BASE_A)   //0x2d00
#define TBC_OFFSET_104				(0x104 + SDIO_BASE_A)   //0x2d04
#define TBC_OFFSET_108				(0x108 + SDIO_BASE_A)   //0x2d08
#define TBC_OFFSET_10C				(0x10c + SDIO_BASE_A)   //0x2d0C
#define TBC_OFFSET_110		       	(0x110 + SDIO_BASE_A)   //0x2d10
#define TBC_OFFSET_114				(0x114 + SDIO_BASE_A)   //0x2d14
#define TBC_OFFSET_118				(0x118 + SDIO_BASE_A)   //0x2d18
#define TBC_OFFSET_11C				(0x11C + SDIO_BASE_A)   //0x2d1C
#define TBC_OFFSET_120		    	      (0x120 + SDIO_BASE_A)   //0x2d20
#define TBC_OFFSET_124				(0x124 + SDIO_BASE_A)   //0x2d24
#define TBC_OFFSET_128				(0x128 + SDIO_BASE_A)   //0x2d28

#define TBC_RAM_SHARE				(0xe4 + CAP_SRAM_SWITCH_BASE)//INTF SRAM SHARE ENABLE BIT31: 1-> CAPTURE BUF

void batch_dump_reg(struct wiphy *wiphy,unsigned int addr[], unsigned int addr_num);

#endif
