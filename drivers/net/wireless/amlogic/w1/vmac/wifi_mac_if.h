/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer interface function/interface use by driver layer
 *
 *
 ****************************************************************************************
 */

#ifndef _DRV_IEEE80211_INTERFACES_H
#define _DRV_IEEE80211_INTERFACES_H

#include "wifi_drv_main.h"
#include "wifi_pkt_desc.h"
#include "wifi_mac_action.h"

#define VM_WIFI_CONNECT_STATE(s )       (((s)== WIFINET_S_CONNECTING)||\
            ((s)== WIFINET_S_AUTH)||\
            ((s)== WIFINET_S_ASSOC))

static __inline void
wifi_mac_ComSetCap(struct wifi_mac *wifimac, unsigned int cap)
{
    wifimac->wm_caps |= cap;
}

static __inline void
wifi_mac_com_clear_cap(struct wifi_mac *wifimac, unsigned int cap)
{
    wifimac->wm_caps &= ~cap;
}


static __inline void
wifi_mac_com_set_flag(struct wifi_mac *wifimac, unsigned int flag)
{
    wifimac->wm_flags |= flag;
}

static __inline void
wifi_mac_com_clear_flag(struct wifi_mac *wifimac, unsigned int flag)
{
    wifimac->wm_flags &= ~flag;
}

static __inline int
wifi_mac_com_has_flag(struct wifi_mac *wifimac, unsigned int flag)
{
    return ((wifimac->wm_flags & flag) != 0);
}

static __inline void
wifi_mac_com_set_extflag(struct wifi_mac *wifimac, unsigned int extflag)
{
    wifimac->wm_flags_ext |= extflag;
}

static __inline void
wifi_mac_com_clear_extflag(struct wifi_mac *wifimac, unsigned int extflag)
{
    wifimac->wm_flags_ext &= ~extflag;
}

static __inline int
wifi_mac_com_has_extflag(struct wifi_mac *wifimac, unsigned int extflag)
{
    return ((wifimac->wm_flags_ext & extflag) != 0);
}

static __inline void
wifi_mac_com_set_maxampdu(struct wifi_mac *wifimac, unsigned char maxampdu)
{
    wifimac->wm_maxampdu = maxampdu;
}

static __inline void
wifi_mac_com_set_mpdudensity(struct wifi_mac *wifimac, unsigned char mpdudensity)
{
    wifimac->wm_mpdudensity = mpdudensity;
    wifimac->wm_flags_ext |= WIFINET_FEXT_HTUPDATE;
}

static __inline void
wifi_macwnet_vif_set_extflag(struct wlan_net_vif *wnet_vif, unsigned int extflag)
{
    wnet_vif->vm_flags_ext |= extflag;
}

static __inline void
wifi_macwnet_vif_clear_extflag(struct wlan_net_vif *wnet_vif, unsigned int extflag)
{
    wnet_vif->vm_flags_ext &= ~extflag;
}

static __inline enum wifi_mac_opmode
wifi_macwnet_vif_get_opmode(struct wlan_net_vif *wnet_vif)
{
    return wnet_vif->vm_opmode;
}

#define WIFINET_NODE_USEAMPDU(_sta) \
    (((_sta)->sta_flags & WIFINET_NODE_HT) != 0) && \
    (((_sta)->sta_wnet_vif->vm_flags_ext & WIFINET_FEXT_AMPDU) != 0)


static __inline void
wifi_macnsta_set_txpower(struct wifi_station *sta, unsigned short txpower)
{
    sta->sta_txpower = txpower;
}


static __inline int
wifi_macnsta_has_cap(struct wifi_station *sta, unsigned short cap)
{
    return ((sta->sta_capinfo & cap) != 0);
}

static __inline struct wmeParams *
wifi_mac_wmm_chanparams(struct wlan_net_vif *wnet_vif,
                                    struct wifi_mac *wifimac, int ac)
{
    return &wifimac->wm_wme[wnet_vif->wnet_vif_id].wme_chanParams.cap_wmeParams[ac];
}



#define wifi_mac_enumerate_wnet_vifs(_wnet_vif, _netcom)  \
        list_for_each_entry(_wnet_vif,&(_netcom)->wm_wnet_vifs,vm_next)

#define NET80211_HANDLE(_ieee)      ((struct wifi_mac *)(_ieee))
#define MAC_ADDR_LEN        6
#define ETHER_TYPE_LEN      2
#define ETHER_CRC_LEN       4
#define ETHER_HDR_LEN       (MAC_ADDR_LEN*2+ETHER_TYPE_LEN)
#define ETHER_MAX_LEN       1518

#ifndef ETHERTYPE_PAE
#define ETHERTYPE_PAE   0x888e   /* EAPOL PAE/802.1x */
#endif

#ifndef ETHERTYPE_WPI
#define ETHERTYPE_WPI   0x88b4
#endif
#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP    0x0800
#endif
#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP    0x0806
#endif

#define IP_PRI_SHIFT        5
#define VLAN_PRI_SHIFT  13
#define VLAN_PRI_MASK   7


#define llc_control     llc_un.type_u.control
#define llc_frmr_control    llc_un.type_frmr.frmr_control
#define llc_snap        llc_un.type_snap


#define LLC_SNAPFRAMELEN 8

#define LLC_UI      0x3
#define LLC_DISC    0x43
#define LLC_UA      0x63
#define LLC_TEST    0xe3
#define LLC_FRMR    0x87
#define LLC_DM      0x0f
#define LLC_XID     0xaf
#define LLC_SAME    0x6f



#define LLC_SNAP_LSAP   0xaa


#define MAC_MAX_GAIN -82
#define MAC_MID_GAIN -70
#define MAC_MIN_GAIN -65

// when scan ap count more than MIN_GIAN_THRESHOLD , use MAC_MIN_GAIN
#define MAX_GIAN_THRESHOLD 10
#define CONNECT_MIN_GIAN_THRESHOLD 25
#define UNCONNECT_MIN_GIAN_THRESHOLD 80
#define OVERLAPPING_24G_GIAN_THRESHOLD 5
#define OVERLAPPING_5G_GIAN_THRESHOLD 10

struct  ether_header
{
    u_char  ether_dhost[MAC_ADDR_LEN];
    u_char  ether_shost[MAC_ADDR_LEN];
    u_short ether_type;
} __packed;



struct llc
{
    unsigned char llc_dsap;
    unsigned char llc_ssap;
    union
    {
        struct
        {
            unsigned char control;
            unsigned char format_id;
            unsigned char class;
            unsigned char window_x2;
        } __packed type_u;
        struct
        {
            unsigned char num_snd_x2;
            unsigned char num_rcv_x2;
        } __packed type_i;
        struct
        {
            unsigned char control;
            unsigned char num_rcv_x2;
        } __packed type_s;
        struct
        {
            unsigned char control;
            unsigned char frmr_rej_pdu0;
            unsigned char frmr_rej_pdu1;
            unsigned char frmr_control;
            unsigned char frmr_control_ext;
            unsigned char frmr_cause;
        } __packed type_frmr;
        struct
        {
            unsigned char  control;
            unsigned char  org_code[3];
            unsigned short ether_type;
        } __packed type_snap;
        struct
        {
            unsigned char control;
            unsigned char control_ext;
        } __packed type_raw;
    } llc_un ;
} __packed;

struct l2_update_frame
{
    struct ether_header eh;
    unsigned char dsap;
    unsigned char ssap;
    unsigned char control;
    unsigned char xid[3];
}  __packed;

 int wifi_mac_entry(struct wifi_mac *wifimac,void *  drv_priv);
 void wifi_mac_get_sts(struct wifi_mac *wifimac,unsigned int op_code, unsigned int ctrl_code);
 int wifi_mac_mac_exit(struct wifi_mac *wifimac);
void wifi_mac_scan_start(struct wifi_mac *wifimac);

int wifi_mac_reset(struct wifi_mac *wifimac);
void wifi_mac_addba_set_rsp(struct wifi_station *sta, unsigned char tid_index, unsigned short statuscode);
void wifi_mac_new_assoc(struct wifi_station *sta, int isnew);
void wifi_mac_updateslot(struct wifi_mac *wifimac);
int wifi_mac_set_quiet(struct wifi_station *sta, unsigned char *quiet_elm);
int wifi_mac_set_country(struct wifi_mac *wifimac, char *isoName);
unsigned int wifi_mac_all_txq_all_qcnt(struct wifi_mac *wifimac);
void wifi_mac_update_protmode(struct wifi_mac *wifimac);
void wifi_mac_vmac_delt(struct wlan_net_vif *wnet_vif);
void wifi_mac_scan_end(struct wifi_mac *wifimac);
void wifi_mac_connect_start(struct wifi_mac *wifimac);
void wifi_mac_connect_end(struct wifi_mac *wifimac);
void wifi_mac_scan_set_gain(struct wifi_mac *wifimac, unsigned char rssi);
void wifi_mac_set_channel_rssi(struct wifi_mac *wifimac, unsigned char rssi);
int wifi_mac_is_in_noisy_environment(struct wifi_mac *wifimac);
int wifi_mac_is_in_clear_environment(struct wifi_mac *wifimac);


void wifi_mac_get_channel_rssi_before_scan(struct wifi_mac *wifimac, int *rssi);

void wifi_mac_set_tx_power_accord_rssi(struct wifi_mac *wifimac, unsigned char rssi);


unsigned int wifi_mac_add_work_task(struct wifi_mac *wifimac,
    void *func,void *func_cb, SYS_TYPE param1, SYS_TYPE param2, SYS_TYPE param3, SYS_TYPE param4, SYS_TYPE param5);

void wifi_mac_com_ps_set_state(struct wifi_mac *wifimac, enum wifinet_ps_state newstate, int wnet_vif_id);
void wifi_mac_set_ampduparams(struct wifi_station *sta);
void wifi_mac_addba_req_setup(struct wifi_station *sta, unsigned char tid_index,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short *batimeout, struct wifi_mac_ba_seqctrl *basequencectrl, unsigned short buffersize);
void wifi_mac_addba_rsp_setup(struct wifi_station *sta, unsigned char tid_index);

int wifi_mac_addba_req(struct wifi_station *sta, unsigned char dialogtoken,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short batimeout, struct wifi_mac_ba_seqctrl basequencectrl);

void wifi_mac_addba_rsp(struct wifi_station *sta, unsigned short statuscode,
    struct wifi_mac_ba_parameterset *baparamset, unsigned short batimeout);

void wifi_mac_delba(struct wifi_station *sta, struct wifi_mac_delba_parameterset *delbaparamset, unsigned short reasoncode);
void wifi_mac_config(struct wifi_mac *wifimac, int paramid,int data);
int wifi_mac_get_config(struct wifi_mac *wifimac, int paramid);
void wifi_mac_set_txpower_limit(struct wifi_mac *wifimac, unsigned int limit, unsigned short txpowerdb);
unsigned int  wifi_mac_sta_get_txrate(struct wifi_station *sta);
int wifi_mac_netdev_open(void * ieee);
int wifi_mac_netdev_stop(void * ieee);
int wifi_mac_wmm_update2hal(struct wifi_mac *wifimac,struct wlan_net_vif *wnet_vif);
unsigned int wifi_mac_register_behindTask(struct wifi_mac *wifimac,void *func);
unsigned int wifi_mac_call_task(struct wifi_mac *wifimac,SYS_TYPE taskid,SYS_TYPE param1);
void wifi_mac_sta_free(struct wifi_station *sta);
void wifi_mac_sta_clean(struct wifi_station *sta);
void wifi_mac_get_country(struct wifi_mac *wifimac, struct _wifi_mac_country_iso *ciso);

int wifi_mac_cap_attach(struct wifi_mac *wifimac, struct drv_private*  drv_priv);
int wifi_mac_cap_detach(struct wifi_mac *wifimac);
int wifi_mac_rx_complete(void * ieee,  struct sk_buff * skbbuf, struct wifi_mac_rx_status* rs);
int wifi_mac_tx_send( struct sk_buff * skbbuf);
int wifi_mac_tx_mgmt_frm(struct wifi_mac *wifimac,  struct sk_buff * skbbuf);
int wifi_mac_build_txinfo(struct wifi_mac *wifimac,  struct sk_buff * skbbuf, struct wifi_mac_tx_info *txinfo);
void wifi_mac_ChangeChannel(void * ieee, struct wifi_channel *chan, unsigned char flag, unsigned char vid, unsigned char opmode);
int wifi_mac_get_netif_cfg(void *);
void *wifi_mac_alloc_ndev(unsigned int len);
void wifi_mac_free_vmac(void *netif);
struct wlan_net_vif * wifi_mac_get_wnet_vif_by_vid(struct wifi_mac *wifimac, int wnet_vif_id);
void wifi_mac_com_vattach(struct wlan_net_vif *);
void wifi_mac_com_vdetach(struct wlan_net_vif *);
void wifi_mac_notify_queue_status(struct wifi_mac *wifimac, unsigned int qqcnt);
void wifi_mac_ht_prot(struct wifi_mac *wifimac, struct wifi_station *sta, enum wifi_mac_ht_prot_update_flag flag);
void wifi_mac_reset_vmac(struct wlan_net_vif *wnet_vif);
void wifi_mac_reset_tspecs(struct wlan_net_vif *wnet_vif);
void wifi_mac_reset_ht(struct wifi_mac *wifimac);
void wifi_mac_reset_vht(struct wifi_mac *wifimac);
void wifi_mac_reset_erp(struct wifi_mac *, enum wifi_mac_macmode);
void wifi_mac_set_shortslottime(struct wifi_mac *, int onoff);
void wifi_mac_change_cbw(struct wifi_mac *wifimac, int to20);
void wifi_mac_wme_initparams(struct wlan_net_vif *);
void wifi_mac_wme_initparams_locked(struct wlan_net_vif *);
void wifi_mac_wme_updateparams(struct wlan_net_vif *);
void wifi_mac_wme_updateparams_locked(struct wlan_net_vif *);
int wifi_mac_open(struct net_device *);
int wifi_mac_initial(struct net_device *, int force);
int wifi_mac_stop(struct net_device *);
void wifi_mac_stop_running(struct wifi_mac *);
int wifi_mac_top_sm(struct wlan_net_vif *,enum wifi_mac_state, int);
enum hal_op_mode wifi_mac_opmode_2_halmode(enum wifi_mac_opmode opmode);
int wifi_mac_create_vmac(struct wifi_mac *wifimac, void *ifr,int cmdFromwhr);
int wifi_mac_device_ip_config(struct wlan_net_vif *wnet_vif, void *event);
void wifi_mac_tbtt_handle(struct wlan_net_vif *wnet_vif);
void wifi_mac_channel_switch_complete(struct wlan_net_vif *wnet_vif);
void wme_update_ex(struct wifi_mac *wifimac,struct wlan_net_vif *wnet_vif );
void wifi_mac_tx_addba_check(struct wifi_station *sta,unsigned char tid_index);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
int wifi_mac_ioctrl_wrapper(struct net_device *dev, struct ifreq *ifr,
    void __user *data, int cmd);
#endif
int wifi_mac_ioctrl(struct net_device *dev, struct ifreq *ifr, int cmd);
void wnet_vif_vht_cap_init( struct wlan_net_vif *wnet_vif );
int wifi_mac_setup(struct wifi_mac *, struct wlan_net_vif *, int opmode);
void wifi_mac_init_ops (struct wifi_mac* wmac);
void wifi_mac_set_reg_val(unsigned int reg_addr, enum wifi_mac_bwc_width bw);

int vm_wlan_net_vif_setup_forchvif(struct wifi_mac *wifimac, struct wlan_net_vif *wnet_vif, const char *name,  int opmode);
int vm_wlan_net_vif_register(struct wlan_net_vif *, char *);
void vm_wlan_net_vif_unregister(struct wlan_net_vif *);
void wifi_mac_build_country_ie(struct wlan_net_vif *);
int wifi_mac_get_new_vmac_id(struct wifi_mac *wifimac);
void wifi_mac_delt_vmac_id(struct wifi_mac *wifimac,int vid);
enum wifi_mac_macmode p2p_phy_mode_filter (enum wifi_mac_macmode in);
enum wifi_mac_macmode wifi_mac_get_sta_mode(struct wifi_scan_info *se);
int wifi_mac_mode_vattach_ex   (struct wlan_net_vif *wnet_vif, enum wifi_mac_opmode opmode);
void wifi_mac_write_word(unsigned int addr,unsigned int data);
unsigned int wifi_mac_read_word(unsigned int addr);
void wifi_mac_bt_write_word(unsigned int addr,unsigned int data);
unsigned int wifi_mac_bt_read_word(unsigned int addr);
void wifi_mac_pt_rx_start(unsigned int qos);
void wifi_mac_pt_rx_stop(void);
struct wifi_mac* wifi_mac_get_mac_handle(void);
void wifi_mac_restore_wnet_vif_channel_task(struct wlan_net_vif *wnet_vif);
void wifi_mac_roaming_trigger(struct wlan_net_vif * wnet_vif);
void wifi_mac_sm_switch (SYS_TYPE param1,SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5);
int wifi_mac_trigger_recovery(void * arg);
void wifi_mac_fw_recovery(struct wlan_net_vif *wnet_vif);
int wifi_mac_connect_repair(struct wifi_mac *wifimac);

void wifi_mac_tx_lock_timer_attach(void);
void wifi_mac_tx_lock_timer_cancel(void);

#endif
