#ifndef __AML_PHY_H__
#define __AML_PHY_H__

#include "wifi_hal_com.h"

#ifndef ALIGN_POINT
#define ALIGN_POINT(x,a) ((unsigned char *)(((unsigned long)(x)+(a)-1)&~((a)-1)))
#endif

unsigned int phy_init_hmac(unsigned char wnet_vif_id);
unsigned int phy_disable_net_traffic(void);
unsigned int phy_set_param_cmd(unsigned char cmd,unsigned char vid,unsigned int data);
unsigned int phy_get_param_cmd_ul(unsigned char cmd,unsigned char vid);
unsigned int phy_enable_bcn(unsigned char vid,unsigned short BeaconInterval, unsigned short DtimPeriod, unsigned char BssType);
unsigned int phy_set_bcn_buf(unsigned char wnet_vif_id,unsigned char *pBeacon, unsigned short len,unsigned short Rate,unsigned short Flag);
unsigned int phy_switch_chan(unsigned short channel, unsigned char bw, unsigned char restore);
void phy_rf_channel_restore(unsigned short channel, int bw);
unsigned int phy_set_mac_bssid(unsigned char wnet_vif_id,unsigned char * Bssid);
unsigned int phy_init_hmac(unsigned char wnet_vif_id);
unsigned int phy_update_bcn_intvl(unsigned char wnet_vif_id,unsigned short BcnInterval);
unsigned int phy_rst_mcast_key(unsigned char wnet_vif_id);
unsigned int phy_rst_ucast_key(unsigned char wnet_vif_id);
unsigned int phy_rst_all_key(unsigned char wnet_vif_id);
unsigned int phy_clr_key(unsigned char wnet_vif_id,unsigned short StaAid);
unsigned int phy_set_ucast_key(unsigned char wnet_vif_id,unsigned short StaAid, unsigned char *pMac, unsigned char *pKey, unsigned char keyLen, unsigned char encryType, unsigned char keyId);
unsigned int phy_set_mcast_key(unsigned char wnet_vif_id,unsigned char *pKey, unsigned char keyLen, unsigned int keyId,unsigned char encryType,unsigned char AuthRole);
unsigned int phy_set_rekey_data(unsigned char wnet_vif_id, void *rekey_data, unsigned short StaAid);
unsigned int phy_set_preamble_type( unsigned char PreambleType);
unsigned int phy_register_sta_id(unsigned char vid,unsigned short StaAid,unsigned char *pMac, unsigned char encrypt);
unsigned int phy_addba_ok(unsigned char wnet_vif_id,unsigned short StaAid,unsigned char TID,unsigned short SeqNumStart,unsigned char BA_Size,unsigned char AuthRole,unsigned char BA_TYPE);
unsigned int phy_delt_ba_ok(unsigned char wnet_vif_id,unsigned short StaAid,unsigned char TID,unsigned char AuthRole);
unsigned int phy_get_extern_chan_status(void);
unsigned int phy_set_rf_chan(struct hal_channel *hchan, unsigned char flag, unsigned char vid, unsigned char opmode);
unsigned int phy_set_mac_addr(unsigned char wnet_vif_id,unsigned char * MacAddr);
unsigned int phy_unregister_sta_id(unsigned char wnet_vif_id,unsigned short StaAid);
unsigned int phy_unregister_all_sta_id(unsigned char wnet_vif_id);
unsigned int phy_set_lretry_limit(unsigned int data);
unsigned int phy_set_sretry_limit(unsigned int data);
unsigned int phy_set_cam_mode(unsigned char wnet_vif_id,unsigned char OpMode);
unsigned int phy_set_dhcp(unsigned char wnet_vif_id,unsigned int ip);
unsigned int phy_set_chan_support_type(struct hal_channel *chan);
unsigned int phy_set_chan_phy_type(int);
void phy_scan_cmd(unsigned int data);
void phy_set_tx_power_accord_rssi(int bw, unsigned short channel, unsigned char rssi, unsigned char power_mode);
void phy_set_channel_rssi(unsigned char rssi);
unsigned int phy_set_rd_support(unsigned char wnet_vif_id, unsigned int data);
unsigned int phy_pwr_save_mode(unsigned char wnet_vif_id,unsigned int data);
unsigned int phy_vmac_disconnect(unsigned char wnet_vif_id);
unsigned int phy_vmac_connect(unsigned char wnet_vif_id);
unsigned int phy_set_txlive_time(unsigned int  txlivetime);
unsigned int phy_set_slot_time(unsigned int slot);
unsigned int phy_set_bcn_intvl(unsigned char wnet_vif_id,unsigned int bcninterval);
unsigned int phy_set_ac_param(unsigned char wnet_vif_id,unsigned char ac,unsigned char aifsn,unsigned char cwminmax,unsigned short txop);
unsigned int phy_debug( unsigned short len,unsigned int offset,unsigned char * buffer);

int phy_set_p2p_opps_cwend_enable(unsigned char vid, unsigned char p2p_oppps_cw);
int phy_set_p2p_noa_enable(unsigned char vid, unsigned int duration, unsigned int interval, unsigned int starttime, unsigned char count, unsigned char flag);
unsigned long long phy_get_tsf(unsigned char vid);
unsigned int phy_set_rtc_enable(int enrtc);
unsigned char hal_tx_desc_get_power( unsigned char rate );
unsigned long long phy_get_param_cmd_ull(unsigned char cmd,unsigned char vid);
unsigned int phy_send_null_data(struct NullDataCmd null_data, int len);
unsigned int phy_keep_alive(struct NullDataCmd null_data_param, int null_data_len, int enable, int period);
unsigned int phy_set_beacon_miss(unsigned char vid, unsigned char enable, int period);
unsigned int phy_set_vsdb(unsigned char vid, unsigned char enable);
unsigned int phy_set_arp_agent(unsigned char vid, unsigned char enable, unsigned int ipv4, unsigned char * ipv6);
unsigned int phy_set_pattern(unsigned char vid, unsigned char offset, unsigned char len,
            unsigned char id, unsigned char *mask, unsigned char *pattern);
int phy_set_suspend(unsigned char vid, unsigned char enable, unsigned char mode, unsigned int filters);
unsigned int phy_get_rw_ptr(unsigned char vid);

unsigned int phy_send_ndp_announcement(struct NDPAnncmntCmd ndp_anncmnt);
void phy_set_bmfm_info(int wnet_vif_id, unsigned char *group_id,
        unsigned char * user_position, unsigned char feedback_type);
unsigned int hal_dpd_memory_download_cmd(void);

unsigned int phy_set_coexist_en( unsigned char enable);
unsigned int phy_set_coexist_max_miss_bcn( unsigned int miss_bcn_cnt);
unsigned int phy_set_coexist_req_timeslice_timeout_value( unsigned int timeout_value);
unsigned int phy_set_coexist_not_grant_weight( unsigned int not_grant_weight);
unsigned int phy_set_coexist_max_not_grant_cnt( unsigned int coexist_max_not_grant_cnt);
unsigned int phy_set_coexist_scan_priority_range( unsigned int coexist_scan_priority_range);
unsigned int phy_set_coexist_be_bk_noqos_priority_range( unsigned int coexist_scan_priority_range);
unsigned int phy_coexist_config(const void *data, int data_len);
unsigned int phy_interface_enable(unsigned char enable, unsigned char vid);
unsigned int hal_set_fwlog_cmd(unsigned char mode);
unsigned int hal_cfg_cali_param(void);
unsigned int hal_cfg_txpwr_cffc_param(void * chan, void * txpwr_plan);

unsigned char get_s8_item(char *varbuf, int len, char *item, char *item_value);
unsigned char get_s16_item(char *varbuf, int len, char *item, short *item_value);
unsigned char get_s32_item(char *varbuf, int len, char *item, unsigned int *item_value);
#endif  //__AML_PHY_H__
