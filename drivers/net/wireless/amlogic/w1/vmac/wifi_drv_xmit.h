#ifndef __DRV_XMIT_H__
#define __DRV_XMIT_H__

#include "wifi_drv_main.h"

#define ADDBA_EXCHANGE_ATTEMPTS 10
#define ADDBA_TIMEOUT 1500 /* ms */

#define BITS_PER_BYTE 8
#define OFDM_PLCP_BITS 22
#define HT_RC_2_STREAMS(_rc) ((((_rc) & 0x78) >> 3) + 1)
#define L_STF 8
#define L_LTF 8
#define L_SIG 4
#define HT_SIG 8
#define HT_STF 4
#define SYMBOL_TIME(_ns) ((_ns) << 2)            // ns * 4 us
#define SYMBOL_TIME_HALFGI(_ns) (((_ns) * 18 + 4) / 5)  // ns * 3.6 us
#define NUM_SYMBOLS_PER_USEC(_usec) (_usec >> 2)
#define NUM_SYMBOLS_PER_USEC_HALFGI(_usec) (((_usec*5)-4)/18)
#define IS_11B_RATE(_r) ((_r) <= WIFI_11B_11M)

enum
{
    RATE_11B_1M = 0x2,
    RATE_11B_2M = 0x4,
    RATE_11B_5M = 0xb,

    RATE_11A_6M = 0xc,
    RATE_11A_9M = 0x12,
    RATE_11A_12M = 0x18,
    RATE_11A_18M = 0x24
};

int aml_tx_hal_buffer_full(struct drv_private *drv_priv, unsigned char queue_id,int txaggrneed,int txprivneed);
void drv_txlist_task(struct drv_private *drv_priv, struct drv_txlist *txlist);
int drv_tx_get_mgmt_frm_rate(struct drv_private *drv_priv, struct wlan_net_vif *wnet_vif, unsigned char fc_type,
    unsigned char *rate, unsigned short *flag);

unsigned int drv_txlist_qcnt(struct drv_private *drv_priv, int);
void drv_txdesc_set_rts_cts(struct drv_private *drv_priv, struct drv_txdesc *ptxdesc);
void drv_update_tx_pwr(struct drv_private *drv_priv, unsigned short txpowerdb);
void drv_set_tx_pwr_limit(struct drv_private * drv_priv, unsigned int limit, unsigned short txpowerdb);
int drv_reset_start(void *, unsigned int);
int drv_reset_end(void *, unsigned int);
int drv_reset(void *);
int drv_tx_init(struct drv_private *drv_priv, int nbufs);
int drv_txlist_cleanup(struct drv_private *drv_priv);
struct drv_txlist *drv_txlist_initial(struct drv_private *drv_priv, int subtype);
void drv_txlist_destroy(struct drv_private *drv_priv, struct drv_txlist *txlist);
int drv_txlist_setup(struct drv_private *drv_priv);
int drv_update_wmmq_param( struct drv_private *drv_priv, unsigned char wnet_vif_id,int ac, int  aifs,int cwmin,int cwmax,int txoplimit);
void drv_tx_irq_tasklet(void *drv_priv, struct txdonestatus *tx_done_status, SYS_TYPE callback, unsigned char queue_id);
void drv_txlist_flushfree(struct drv_private *drv_priv, unsigned char vid);
void free_txlist_when_free_sta(struct drv_private *drv_priv, void *nsta);
void drv_set_pkt_drop(struct drv_private *drv_priv, unsigned char vid, unsigned char enable);
void drv_set_is_mother_channel(struct drv_private *drv_priv, unsigned char vid, unsigned char enable);
void drv_flush_normal_buffer_queue(struct drv_private *drv_priv, unsigned char vid);
void drv_free_normal_buffer_queue(struct drv_private *drv_priv, unsigned char vid);
unsigned short drv_tx_pending_pkt(struct drv_private *drv_priv);
void drv_txlist_init_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta, unsigned char vid);
void drv_txlist_cleanup_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_txlist_free_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_txlist_pause_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_txlist_resume_for_sta(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_handle_tx_intr(struct drv_private *drv_priv);
int drv_tx_start(struct drv_private *drv_priv,  struct sk_buff * skbbuf);

unsigned int drv_txlist_all_qcnt(struct drv_private *drv_priv, int);
int drv_txlist_isfull(struct drv_private *drv_priv,int queue_id,struct sk_buff * skbbuf, void * drv_sta);
int drv_send( struct sk_buff * skbbuf, struct drv_private *drv_priv);
void drv_set_protmode(struct drv_private *drv_priv, enum prot_mode mode);
enum tx_frame_flag drv_set_tx_frame_flag(struct sk_buff *skbbuf);

int drv_to_hal(struct drv_private *drv_priv, struct drv_txlist *txlist, struct list_head *head);
void drv_tx_complete(struct drv_private *drv_priv, struct drv_txdesc *ptxdesc, int txok);

int drv_aggr_check(struct drv_private *drv_priv, void * nsta, unsigned char tid_index);
int drv_aggr_tid( struct drv_private *drv_priv, void * nsta, unsigned char tid_index);
int drv_aggr_allow_to_send(struct drv_private *drv_priv, void * nsta, unsigned char tid_index);
void drv_set_ampduparams(struct drv_private *drv_priv, void *, unsigned short maxampdu, unsigned int mpdudensity);
void drv_addba_req_setup(struct drv_private *drv_priv, void *, unsigned char tid_index, struct wifi_mac_ba_parameterset *baparamset,
    unsigned short *batimeout, struct wifi_mac_ba_seqctrl *basequencectrl, unsigned short buffersize);

void drv_addba_rsp_process(struct drv_private *drv_priv, void *,
    unsigned short statuscode, struct wifi_mac_ba_parameterset *baparamset, unsigned short batimeout);
void drv_addba_clear(struct drv_private *drv_priv, void *);

unsigned short drv_addba_status(struct drv_private *drv_priv, void * nsta, unsigned char tid_index);
void drv_txrxampdu_del(struct drv_private *drv_priv, void * nsta, unsigned char tid_index, unsigned char initiator);
void drv_txampdu_del(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta, unsigned char tid_index);
int drv_rate_findindex_from_ratecode(const struct drv_rate_table *rt, int vendor_rate_code);
int drv_get_amsdu_supported(struct drv_private *drv_priv, void * nsta, int tid_index);

int drv_hal_tx_frm_pause(struct drv_private *drv_priv, int pause);
unsigned int drv_low_add_worktask( struct drv_private *drv_priv,void *func,void *func_cb,
    SYS_TYPE param1,SYS_TYPE param2,SYS_TYPE param3, SYS_TYPE param4,SYS_TYPE param5);
unsigned int drv_lookup_rate(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta,   struct aml_ratecontrol *txdesc_rateinfo);


#endif
