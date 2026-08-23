#ifndef __VM_WIFI_OUTPUT_H__
#define __VM_WIFI_OUTPUT_H__

#include "wifi_mac_var.h"
#include <linux/netdevice.h>

#define ETHERTYPE_IPV6 (0x86dd)
#define DEFAULT_LISTEN_INTERVAL (10)

#define WH4(wh) ((struct WIFINET_S_FRAME_ADDR4 *)wh)
#define KEY_UNDEFINED(k)    ((k).wk_cipher == &wifi_mac_cipher_none)

#define ADDSHORT(frm, v) do {           \
                frm[0] = (v) & 0xff;            \
                frm[1] = (v) >> 8;          \
                frm += 2;               \
        } while (0)
#define ADDSELECTOR(frm, sel) do {      \
                memcpy(frm, sel, 4);            \
                frm += 4;               \
        } while (0)

#define SM(_v, _f)  (((_v) << _f##_S) & _f)
#define EN_AP_MODE_IN_QUIET_IE (1)
#define DIS_AP_MODE_IN_QUIET_IE (0)

#define CSA_COUNT               1
#define CSA_BLOCK_TX            1

struct wifi_mac_csa_wrapper{
    unsigned char ie;
    unsigned char len;
    unsigned char info[3];
} __packed;

struct wifi_mac_csa_data {
    //struct ieee80211_header mac_header;
    struct wifi_mac_action_csa_frame extend_csa;
    struct wifi_mac_csa_wrapper csa_wrapper;
} __packed;

void wifi_mac_complete_wbuf(struct sk_buff * skbbuf, int errcode);
void wifi_mac_send_setup(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, struct wifi_frame *wh,
    int type, const unsigned char sa[WIFINET_ADDR_LEN], const unsigned char da[WIFINET_ADDR_LEN], const unsigned char bssid[WIFINET_ADDR_LEN]);

int wifi_mac_send_testdata(struct wifi_station *sta, int length);
int wifi_mac_is_allow_send(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, struct sk_buff *skb);
void wifi_mac_xmit_pkt_parse(struct sk_buff *skb, struct wifi_mac *wifimac);
unsigned short wifi_mac_udp_sum_calc(unsigned short len_udp, unsigned char * src_addr, unsigned char *dest_addr, unsigned char *buff);
int wifi_mac_udp_csum(struct sk_buff *skb);
netdev_tx_t wifi_mac_hardstart(struct sk_buff *, struct net_device *);
int wifi_mac_send_nulldata(struct wifi_station *sta, unsigned char pwr_save, unsigned char pwr_flag, unsigned char qos, int ac);
int wifi_mac_send_nulldata_for_ap(struct wifi_station *sta, unsigned char pwr_save,unsigned char pwr_flag, unsigned char qos, int ac);

int wifi_mac_send_qosnulldata(struct wifi_station *, int);
struct sk_buff *wifi_mac_skbhdr_adjust(struct wlan_net_vif *wnet_vif, int hdrsize, struct wifi_mac_key *key, struct sk_buff *skb, int ismulticast);
struct sk_buff *wifi_mac_encap(struct wifi_station *, struct sk_buff *);
unsigned char *wifi_mac_add_rates(unsigned char *, const struct wifi_mac_rateset *);
unsigned char *wifi_mac_add_xrates(unsigned char *, const struct wifi_mac_rateset *);
unsigned char *wifi_mac_add_wpa(unsigned char *, struct wlan_net_vif *);
unsigned char *wifi_mac_add_erp(unsigned char *, struct wifi_mac *);
unsigned char *wifi_mac_add_wme_param(unsigned char *, struct wifi_mac_wme_state *, int uapsd_enable);
unsigned char *wifi_mac_add_country(unsigned char *, struct wifi_mac *);
unsigned char *wifi_mac_add_htcap(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_htinfo(unsigned char *, struct wifi_station *);

unsigned char *wifi_mac_add_vht_cap(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vht_opt(unsigned char *, struct wifi_station *, unsigned int sub_type);
unsigned char *wifi_mac_add_vht_txpw(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vht_ch_sw_wrp(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vht_ch_sw_an(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vht_ext_bss_ld(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vht_quiet_ch(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vht_op_md_ntf(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_obss_scan(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_extcap(unsigned char *, struct wifi_station *);
unsigned char *wifi_mac_add_vendor_ie(unsigned char *frm, struct wifi_mac *wifimac, unsigned char index);

unsigned char *wifi_mac_add_timeout_interval(unsigned char *, struct wifi_station *);

int wifi_mac_send_probereq(struct wifi_station *, const unsigned char sa[WIFINET_ADDR_LEN], const unsigned char da[WIFINET_ADDR_LEN],
    const unsigned char bssid[WIFINET_ADDR_LEN], const unsigned char *ssid, size_t ssidlen, const void *optie, size_t optielen);

int wifi_mac_send_probe_rsp(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,void *arg);
int wifi_mac_send_auth(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,void *arg);
int wifi_mac_send_deauth(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,void *arg);
int wifi_mac_send_assoc_req(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,int type);
int wifi_mac_send_assoc_rsp(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,void *arg,int type);
int wifi_mac_send_disassoc(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,void *arg);
int wifi_mac_send_actionframe(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,void *arg);
int wifi_mac_send_mgmt(struct wifi_station *, int, void *);
int wifi_mac_udp_send_timeout_ex(void *arg);
void wifi_mac_check_opt_ie(struct wlan_net_vif *wnet_vif);
unsigned char *wifi_mac_add_chansw_wrapper(unsigned char *frm,struct wifi_station *sta);
unsigned char *wifi_mac_add_extended_chanswitch(unsigned char *frm,struct wifi_station *sta);
unsigned char *wifi_mac_add_chanswitch(unsigned char *frm,struct wifi_station *sta);
int wifi_mac_csa_send_action(struct wifi_mac *wifimac, struct wlan_net_vif *wnet_vif, struct wifi_station *sta, struct cfg80211_chan_def chan_def);
void wifi_mac_csa_send_action_task(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5);
#endif
