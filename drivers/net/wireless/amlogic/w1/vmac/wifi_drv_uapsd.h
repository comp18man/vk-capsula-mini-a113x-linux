#ifndef __DRV_UAPSD_H__
#define __DRV_UAPSD_H__

#include "wifi_drv_main.h"

#ifdef DRV_SUPPORT_TX_WITHDRAW
int drv_tx_withdraw_legacyps_send (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
#endif

void drv_tx_mcastq_init (struct drv_private *drv_priv, int wnet_vif_id);
void drv_tx_mcastq_cleanup (struct drv_private *drv_priv, int wnet_vif_id);
void drv_tx_uapsd_nsta_init (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_tx_uapsd_nsta_cleanup(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_tx_bk_list_init (struct drv_txlist *txlist);
void drv_tx_queue_uapsd_nsta(struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct aml_driver_nsta *drv_sta);
void drv_tx_mcastq_addbuf(struct drv_private *drv_priv, struct list_head *head);
int drv_tx_mcastq_send (struct drv_private *drv_priv);
void drv_txq_backup_drain (struct drv_private *drv_priv, struct drv_txlist *txlist, unsigned char vid);
void drv_txq_backup_cleanup (struct drv_private *drv_priv, struct drv_txlist *txlist, struct aml_driver_nsta *drv_sta);
unsigned int drv_tx_uapsd_nsta_qcnt(void * nsta);
int drv_process_uapsd_nsta_trigger(struct drv_private *drv_priv,                void * nsta, unsigned char maxsp,
    unsigned char ac, unsigned char flush);
void drv_tx_withdraw_init (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_tx_withdraw_cleanup (struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_tx_withdraw_uapsd_addbuf(struct drv_private *drv_priv, 
    struct list_head *txdesc_list_head, struct aml_driver_nsta *drv_sta);
void drv_tx_withdraw_legacyps_addbuf(struct drv_private *drv_priv, 
    struct list_head *txdesc_list_head, struct aml_driver_nsta *drv_sta);
int drv_txq_backup_qcnt(struct drv_txlist *txlist);
int drv_txq_backup_send (struct drv_private *drv_priv, struct drv_txlist *txlist);
void drv_txq_backup_addbuf(struct drv_private *drv_priv, struct list_head *txdesc_list_head, struct drv_txlist *txlist);
#endif
