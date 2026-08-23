#ifndef _AML_TXDESC_H_
#define _AML_TXDESC_H_

#include "wifi_hal_com.h"

void hal_tx_desc_init(void);
void hal_tx_desc_build(struct hi_agg_tx_desc* HiTxDesc,
    struct hi_tx_priv_hdr* HI_TxPriv, struct hi_tx_desc *pTxDPape);
void hal_tx_desc_build_sub(struct hi_tx_priv_hdr* HI_TxPriv,
    struct hi_tx_desc *pTxDPape, struct hi_tx_desc *pFirstTxDPape);

unsigned int hal_tx_desc_nonht_mode(unsigned char date_rate,
    unsigned char channel_bw);
unsigned short hal_tx_desc_get_len(unsigned char rate, unsigned short pktlen,
    unsigned char shortGI,unsigned char bw,unsigned char green);

unsigned char hal_mac_frame_type( unsigned int frame_control, unsigned int type );
unsigned int max_send_packet_len(unsigned char rate,unsigned char bw, unsigned char short_gi, unsigned char streams);

char hw_rate2tv_format(unsigned char rate);
void assign_tx_desc_pn(unsigned char is_bc, unsigned char vid,
    unsigned char sta_id, struct hi_tx_desc *tx_page, unsigned char encrypt_type);

extern struct wifi_cfg_mib  wifi_conf_mib;
#endif//_AML_TXDESC_H_
