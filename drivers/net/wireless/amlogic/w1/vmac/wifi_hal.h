#ifndef __FI_CONTENT_H__
#define __FI_CONTENT_H__

#include "wifi_hal_com.h"
#include "wifi_aon_addr.h"
#include "wifi_skbbuf.h"

#define WIFI_ADDR_ISGROUP(wh) (((unsigned char *)wh)[4] & 1)
enum irqreturn  hal_irq_top(int irq, void *dev_id);
#ifdef CONFIG_AML_USE_STATIC_BUF
enum aml_prealloc_index {
        AML_RX_FIFO = 0,
        AML_TX_DESC_BUF = 1
};
#endif

enum aml_key_type {
    AML_UCAST_TYPE = 0,
    AML_MCAST_TYPE = 1,
    AML_TYPE_MAX
};

enum aml_pkt_position {
    AML_PKT_NOT_IN_HAL = 0,
    AML_PKT_IN_HAL = 1
};


int hal_host_init(struct hal_private *hal_priv);
int hal_host_recovery_init(struct hal_private *hal_priv);

int hal_probe(void);
int hal_free(void);
int hal_open(void *   drv_priv);
void hal_get_sts(unsigned int op_code, unsigned int ctrl_code);
int hal_close(void *   drv_priv);
unsigned char hal_tx_empty(void);
struct aml_hal_call_backs  *hal_get_drv_func(void);
void hal_ops_attach(void);
void hal_ops_detach(void);
int hal_init_priv(void);
int hal_recovery_init_priv(void);
int hal_reinit_priv(void);
void hal_exit_priv(void);
int hal_init_task(void);
int hal_fw_repair(void);

int hal_calc_mpdu_page (int mpdulen);
int hal_get_agg_pend_cnt(void);
void hal_txinfo_show(void);
void hal_pn_win_init(enum aml_key_type type, unsigned char wnet_vif_id);
void hal_mrep_cnt_init(struct hal_private *hal_priv,unsigned char wnet_vif_id,unsigned char encryType);
void hal_urep_cnt_init(struct unicastReplayCnt  *RepCnt,unsigned char encryType);
unsigned char hal_chk_replay_cnt(struct hal_private *hal_priv,HW_RxDescripter_bit*RxPrivHdr);
unsigned char hal_set_pn_win(unsigned long long *pn_win);
unsigned long long hal_check_dup_pn(unsigned long long *pn_win, unsigned int repcnt_diff);
void hal_wpi_pn_self_plus(  unsigned long long *WpiPN );
void hal_wpi_pn_self_plus_plus(  unsigned long long *WpiPN );
unsigned char * hal_get_config(void);
void hal_show_txagg_desc( struct hi_agg_tx_desc*  HiTxDesc);
void hal_tx_priv_desc_show(const struct hi_tx_priv_hdr* const HiTxPrivDesc);
void hal_tx_frame(void);
unsigned char hal_wpi_chk_pn_increase(  unsigned char *RcvPN,  unsigned char *SavedPn );
void hal_show_txframe (struct hi_tx_desc *pTxDPape);
void hal_show_rxframe (HW_RxDescripter_bit *RxPrivHdr);
void show_tx_vector (struct hw_tx_vector_bits *TxVector);
int hal_tx_flush(unsigned char vid);
int hal_set_suspend(unsigned char enable);
unsigned char hal_wake_fw_req(void);
unsigned char hal_check_fw_wake(void);
unsigned char hal_set_fw_wake(void);
unsigned char hal_clear_fw_wake(void);
unsigned char hal_get_fw_ps_status(void);

#if defined (HAL_SIM_VER)
extern void Test_Done(int pass);
void hal_enable_gpio_int(unsigned char enable);

/*max length of an ampdu  aligned with page len */
#define MULTI_DATA_LEN ((((STA1_VMAC0_SEND_LEN + 104/*HI_TXDESC_DATAOFFSET*/ + STA2_VMAC1_SEND_FRAME_NUM) \
                                 + PAGE_LEN - 1) / PAGE_LEN) * PAGE_LEN * STA1_VMAC0_AGG_NUM)
#endif

void hal_soft_rx_cs(struct hal_private *hal_priv, struct sk_buff *skb);
void show_rxvector (HW_RxDescripter_bit *RxPrivHdr);
void hal_free_txcmp_buf(struct hal_private *hal_priv);
unsigned char hal_alloc_txcmp_buf(struct hal_private *hal_priv);

void hal_free_fw_event_buf(struct hal_private *hal_priv);
unsigned char hal_alloc_fw_event_buf(struct hal_private *hal_priv);

int hal_is_empty_tx_id(struct hal_private* hal_priv);
int hal_free_tx_id(struct hal_private* hal_priv, struct txdonestatus *txstatus, unsigned long *callback, unsigned char *queue_id);

void hal_txframe_pre(void);
int  hal_txframe_pause(int pause);
void show_mpdu_buf_flag (unsigned int MPDUBufFlag);
void show_tx_vector (struct hw_tx_vector_bits *TxVector);
void show_tx_priv (struct Fw_TxPriv *TxPriv);
 void show_frame_control (unsigned short frame_control);
 void show_qos_control (unsigned short qos_control);
void show_macframe (unsigned char *start, unsigned char len);
void hal_show_txframe (struct hi_tx_desc *pTxDPape);
int hal_calc_mpdu_page (int mpdulen);
struct sk_buff *hal_fill_agg_start(struct hi_agg_tx_desc* HI_AGG,struct hi_tx_priv_hdr* HI_TxPriv);
struct sk_buff *hal_fill_priv(struct hi_tx_priv_hdr* HI_TxPriv,unsigned char TID);
int hal_get_agg_cnt(unsigned char TID);
int hal_get_priv_cnt(unsigned char TID);
int hal_reg_task(TaskHandler task);
void hal_unreg_task(int taskid);
int hal_call_task(SYS_TYPE taskid,SYS_TYPE param1);

int hal_txok_thread(void *param);
int hal_rx_thread(void *param);
int hi_irq_thread(void *param);
int hal_work_thread(void *param);
int hal_kill_thread(void);
int hal_create_thread(void);
void hal_dpd_memory_download(void);
void hal_dpd_calibration(void);

void hal_get_fwlog(void);
#endif
