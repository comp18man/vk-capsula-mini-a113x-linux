#ifndef __WIFI_DRV_RECV_H__
#define __WIFI_DRV_RECV_H__

#include "wifi_drv_main.h"

int drv_rx_input(struct drv_private *drv_priv, void *nsta, struct sk_buff * skbbuf, struct wifi_mac_rx_status* rs);
int drv_rx_init( struct drv_private *drv_priv, int nbufs);
void drv_rx_nsta_init(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_rx_nsta_free(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_rx_nsta_clean(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta);
void drv_set_addba_rsp(struct drv_private *drv_priv ,void * nsta, unsigned char tid_index, unsigned short statuscode);

void drv_clr_addba_rsp_status(struct drv_private *drv_priv, void * nsta);
int drv_rx_addbareq( struct drv_private *drv_priv, void *, unsigned char dialogtoken, struct wifi_mac_ba_parameterset *baparamset,
    unsigned short batimeout, struct wifi_mac_ba_seqctrl basequencectrl);

void drv_rx_addbarsp(struct drv_private *drv_priv, void * nsta, unsigned char tid_index);


void drv_rx_delba(struct drv_private *drv_priv, void *, struct wifi_mac_delba_parameterset *delbaparamset, unsigned short reasoncode);
void drv_rxampdu_del(struct drv_private *drv_priv, struct aml_driver_nsta *drv_sta, unsigned char tid_index);

#endif

