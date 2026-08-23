/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer nl80211 iocontrol module
 *
 *
 ****************************************************************************************
 */
#include "wifi_hal_com.h"
#include "wifi_mac_com.h"
#include "wifi_drv_statistic.h"
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include "wifi_clock_measure.h"
#include "rf_d_adda_recv_reg.h"
#include "chip_intf_reg.h"
#include <linux/nl80211.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0) && defined (CONFIG_AMLOGIC_KERNEL_VERSION))
#include <linux/upstream_version.h>
#endif
#include "wifi_pkt_desc.h"
#include "wifi_mac_sae.h"
#include "wifi_mac_action.h"
#include "version.h"
#include "wifi_mac_tx_reg.h"
#include "wifi_drv_capture.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#endif

#ifdef CONFIG_AML_CFG80211

/** vendor events */
const struct nl80211_vendor_cmd_info vendor_events[] = {
	{.vendor_id = AMLOGIC_VENDOR_ID,.subcmd = event_hang,},	/* event_id 0, not used currently*/
    {.vendor_id = AMLOGIC_VENDOR_ID,.subcmd = event_reg_value,},	/* event_id 1 */
	/**add vendor event here*/
};

unsigned long long g_dbg_info_enable = 0;
unsigned long long g_dbg_modules = 0;
extern int g_auto_gain_base;

static struct device *cfg80211_parent_dev = NULL;
static const unsigned int aml_cipher_suites[] =
{
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP,
#ifdef AML_WPA3
	/*
	 * Advertising AES_CMAC cipher suite to userspace would imply that we
	 * are supporting MFP. So advertise only when MFP support is enabled.
	 */
	WLAN_CIPHER_SUITE_AES_CMAC,
#if 0
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
	WLAN_CIPHER_SUITE_BIP_GMAC_128,
	WLAN_CIPHER_SUITE_BIP_CMAC_256,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0) */
#endif//hw not support
#endif
};

/*Table 31-DPP Public Action Frame Type [easy connect v2.0]*/
char dpp_pub_action_str[][50] = {
    "Authentication Request",
    "Authentication Response",
    "Authentication Confirm",
    "Reserved",
    "Reserved",
    "Peer Discovery Request",
    "Peer Discovery Response",
    "PKEX Version 1 Exchange Request",
    "PKEX Exchange Response",
    "PKEX Commit-Reveal Request",
    "PKEX Commit-Reveal Response",
    "Configuration Result",
    "Connection Status Result",
    "Presence Announcement",
    "Reconfiguration Announcement",
    "Reconfiguration Authentication Request",
    "Reconfiguration Authentication Response",
    "Reconfiguration Authentication Confirm",
    "PKEX Exchange Request"
};

/* if wowlan is not supported, kernel generate a disconnect at each suspend
 * cf: /net/wireless/sysfs.c, so register a stub wowlan.
 * Moreover wowlan has to be enabled via a the nl80211_set_wowlan callback.
 * (from user space, e.g. iw phy0 wowlan enable)
 */
static const struct wiphy_wowlan_support wowlan_stub =
{
    .flags = WIPHY_WOWLAN_ANY,
    .n_patterns = 0,
    .pattern_max_len = 0,
    .pattern_min_len = 0,
    .max_pkt_offset = 0,
};

struct ieee80211_channel aml_2ghz_channels[AML_2G_CHANNELS_NUM] =
{
    CHAN2G(1, 2412, 0),
    CHAN2G(2, 2417, 0),
    CHAN2G(3, 2422, 0),
    CHAN2G(4, 2427, 0),
    CHAN2G(5, 2432, 0),
    CHAN2G(6, 2437, 0),
    CHAN2G(7, 2442, 0),
    CHAN2G(8, 2447, 0),
    CHAN2G(9, 2452, 0),
    CHAN2G(10, 2457, 0),
    CHAN2G(11, 2462, 0),
    CHAN2G(12, 2467, 0),
    CHAN2G(13, 2472, 0),
    CHAN2G(14, 2484, 0),
};

struct ieee80211_channel aml_5ghz_channels[AML_5G_CHANNELS_NUM] =
{
    CHAN5G(36, 0),      CHAN5G(40, 0),
    CHAN5G(44, 0),      CHAN5G(48, 0),
    CHAN5G(52, 0),      CHAN5G(56, 0),
    CHAN5G(60, 0),      CHAN5G(64, 0),
    CHAN5G(100, 0),     CHAN5G(104, 0),
    CHAN5G(108, 0),     CHAN5G(112, 0),
    CHAN5G(116, 0),     CHAN5G(120, 0),
    CHAN5G(124, 0),     CHAN5G(128, 0),
    CHAN5G(132, 0),     CHAN5G(136, 0),
    CHAN5G(140, 0),     CHAN5G(144, 0),
    CHAN5G(149, 0),     CHAN5G(153, 0),
    CHAN5G(157, 0),     CHAN5G(161, 0),
    CHAN5G(165, 0),
};


struct ieee80211_rate aml_rates[AML_G_RATES_NUM] =
{
    RATETAB_ENT(10,  0x1,   0),
    RATETAB_ENT(20,  0x2,   0),
    RATETAB_ENT(55,  0x4,   0),
    RATETAB_ENT(110, 0x8,   0),
    RATETAB_ENT(60,  0x10,  0),
    RATETAB_ENT(90,  0x20,  0),
    RATETAB_ENT(120, 0x40,  0),
    RATETAB_ENT(180, 0x80,  0),
    RATETAB_ENT(240, 0x100, 0),
    RATETAB_ENT(360, 0x200, 0),
    RATETAB_ENT(480, 0x400, 0),
    RATETAB_ENT(540, 0x800, 0),
};

static const struct ieee80211_txrx_stypes
vm_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] =
{
    [NL80211_IFTYPE_ADHOC] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
#ifdef AML_WPA3
        | BIT(IEEE80211_STYPE_AUTH >> 4)
#endif /* WL_CLIENT_SAE */
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_AP_VLAN] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
    [NL80211_IFTYPE_P2P_GO] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
};



static void aml_2g_channels_init(struct ieee80211_channel *channels)
{
    memcpy((void*)channels, (void*)aml_2ghz_channels,
           sizeof(struct ieee80211_channel)*AML_2G_CHANNELS_NUM);
}

static void aml_5g_channels_init(struct ieee80211_channel *channels)
{
    memcpy((void*)channels, (void*)aml_5ghz_channels,
           sizeof(struct ieee80211_channel)*AML_5G_CHANNELS_NUM);
}

static void aml_2g_rates_init(struct ieee80211_rate *rates)
{
    memcpy(rates, aml_g_rates,
           sizeof(struct ieee80211_rate)*AML_G_RATES_NUM);
}

static void aml_5g_rates_init(struct ieee80211_rate *rates)
{
    memcpy(rates, aml_a_rates,
           sizeof(struct ieee80211_rate)*AML_A_RATES_NUM);
}

static struct ieee80211_supported_band*
aml_spt_band_alloc (enum ieee80211_band band)
{
    struct ieee80211_supported_band *spt_band = NULL;
    int n_channels, n_bitrates;
    int totalled;

    if (band == IEEE80211_BAND_2GHZ)
    {
        n_channels = AML_2G_CHANNELS_NUM;
        n_bitrates = AML_G_RATES_NUM;
    }
    else if (band == IEEE80211_BAND_5GHZ)
    {
        n_channels = AML_5G_CHANNELS_NUM;
        n_bitrates = AML_A_RATES_NUM;
    }
    else
    {
        goto exit;
    }

    totalled = sizeof(struct ieee80211_supported_band)
                + sizeof(struct ieee80211_channel)*n_channels
                + sizeof(struct ieee80211_rate)*n_bitrates;
    spt_band = (struct ieee80211_supported_band *)ZMALLOC(totalled,"spt_band", GFP_ATOMIC);
    if (!spt_band)
        goto exit;

    spt_band->channels = (struct ieee80211_channel*)(((unsigned char*)spt_band)
                        + sizeof(struct ieee80211_supported_band));
    spt_band->bitrates= (struct ieee80211_rate*)(((unsigned char*)spt_band->channels)
                        + sizeof(struct ieee80211_channel)*n_channels);
    spt_band->band = (enum nl80211_band)band;
    spt_band->n_channels = n_channels;
    spt_band->n_bitrates = n_bitrates;

    if (band == IEEE80211_BAND_2GHZ)
    {
        aml_2g_channels_init(spt_band->channels);
        aml_2g_rates_init(spt_band->bitrates);
    }
    else if (band == IEEE80211_BAND_5GHZ)
    {
        aml_5g_channels_init(spt_band->channels);
        aml_5g_rates_init(spt_band->bitrates);
    }

exit:

    return spt_band;
}

static void
aml_spt_band_free(struct ieee80211_supported_band *spt_band)
{
    unsigned int size;

    if (!spt_band)
        return;

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
    if (spt_band->band == (enum nl80211_band)IEEE80211_BAND_2GHZ)
#else
    if (spt_band->band == IEEE80211_BAND_2GHZ)
#endif
    {
        size = sizeof(struct ieee80211_supported_band)
               + sizeof(struct ieee80211_channel)*AML_2G_CHANNELS_NUM
               + sizeof(struct ieee80211_rate)*AML_G_RATES_NUM;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
    if (spt_band->band == (enum nl80211_band)IEEE80211_BAND_5GHZ)
#else
    else if (spt_band->band == IEEE80211_BAND_5GHZ)
#endif
    {
        size = sizeof(struct ieee80211_supported_band)
               + sizeof(struct ieee80211_channel)*AML_5G_CHANNELS_NUM
               + sizeof(struct ieee80211_rate)*AML_A_RATES_NUM;
    }
    else
    {

    }
    FREE((unsigned char*)spt_band,"spt_band");
}

struct device *vm_cfg80211_get_parent_dev(void)
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    return cfg80211_parent_dev;
}

void vm_cfg80211_set_parent_dev(void *dev)
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    cfg80211_parent_dev = dev;
}

void vm_cfg80211_clear_parent_dev(void)
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    cfg80211_parent_dev = NULL;
}

struct iwscanreq
{
    struct wlan_net_vif *wnet_vif;
};

static int aml_ieee80211_chan_to_frq(int chan, int band)
{

    if (band == IEEE80211_BAND_5GHZ)
    {
        if (chan >= 182 && chan <= 196)
            return 4000 + chan * 5;
        else
            return 5000 + chan * 5;
    }
    else
    {
        if (chan == 14)
            return 2484;
        else if (chan < 14)
            return 2407 + chan * 5;
        else
            return 0;
    }
}

int
nl80211_iftype_2_drv_opmode(enum nl80211_iftype NL80211_IFTYPE)
{
    enum nl80211_iftype in = NL80211_IFTYPE;
    enum wifi_mac_opmode out;
    switch (in)
    {
        case NL80211_IFTYPE_ADHOC:
            out = WIFINET_M_IBSS;
            break;
#ifdef      CONFIG_P2P
        case NL80211_IFTYPE_P2P_CLIENT:
#endif
        case NL80211_IFTYPE_STATION:
            out = WIFINET_M_STA;
            break;
#ifdef      CONFIG_P2P
        case NL80211_IFTYPE_P2P_GO:
#endif
        case NL80211_IFTYPE_AP:
            out = WIFINET_M_HOSTAP;
            break;
        case NL80211_IFTYPE_MONITOR:
            out = WIFINET_M_MONITOR;
            break;
        default:
            DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_CFG80211,
                "%s %d ERROR NL80211_IFTYPE=%d\n",
                __func__, __LINE__, in);
            out= -EOPNOTSUPP;
    }
    return out;
}

static int
drv_opmode_2_nl80211_iftype (enum wifi_mac_opmode drv_type,
    enum NET80211_P2P_ROLE p2p_role)
{
    enum nl80211_iftype out;
    enum wifi_mac_opmode in = drv_type;

    switch (in)
    {
        case WIFINET_M_IBSS:
            out = NL80211_IFTYPE_ADHOC;
            break;
        case WIFINET_M_STA:
#ifdef  CONFIG_P2P
            if (NET80211_P2P_ROLE_CLIENT == p2p_role)
            {
                out = NL80211_IFTYPE_P2P_CLIENT;
            }
            else
#endif
            {
                out = NL80211_IFTYPE_STATION;
            }
            break;
        case WIFINET_M_HOSTAP:
#ifdef  CONFIG_P2P
            if (NET80211_P2P_ROLE_GO == p2p_role)
            {
                out = NL80211_IFTYPE_P2P_GO;
            }
            else
#endif
            {
                out = NL80211_IFTYPE_AP;
            }
            break;
        case WIFINET_M_MONITOR:
            out = NL80211_IFTYPE_MONITOR;
            break;
        default:
            DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_CFG80211,
                "%s %d ERROR drv_type=%d\n", __func__, __LINE__, in);
            out= -EOPNOTSUPP;
    }
    return out;
}

static unsigned int get_cipher_rsn (unsigned int ecipher)
{
    unsigned int cipher;

    switch (ecipher)
    {
        case WLAN_CIPHER_SUITE_WEP40:
        case WLAN_CIPHER_SUITE_WEP104:
            cipher = WIFINET_CIPHER_WEP;
            break;
        case WLAN_CIPHER_SUITE_TKIP:
            cipher = WIFINET_CIPHER_TKIP;
            break;
        case WLAN_CIPHER_SUITE_CCMP:
            cipher = WIFINET_CIPHER_AES_CCM;
            break;
        case WLAN_CIPHER_SUITE_AES_CMAC:
            cipher = WIFINET_CIPHER_AES_CMAC;
            break;
        default:
            cipher = -ENOTSUPP;
    }
    return cipher;
}

#if 0
static int get_cipher_wpa (unsigned int ecipher)
{
    int cipher;
    switch (ecipher)
    {
        case WLAN_CIPHER_SUITE_WEP40:
        case WLAN_CIPHER_SUITE_WEP104:
            cipher = WIFINET_CIPHER_WEP;
            break;
        case WLAN_CIPHER_SUITE_TKIP:
            cipher = WIFINET_CIPHER_TKIP;
            break;
        case WLAN_CIPHER_SUITE_CCMP:
            cipher = WIFINET_CIPHER_AES_CCM;
            break;
        default:
            cipher = -ENOTSUPP;
    }
    return cipher;
}
#endif

int translate_to_dbm(int rssi)
{
    int SignalPower = rssi - 256;

    return SignalPower;
}

static int
wifi_mac_copy_ie (unsigned char *dst,
    const unsigned char * src,
    unsigned char max_ie_data_len)
{
    unsigned char len = src[1];

    dst[0] = src[0];
    if (len > max_ie_data_len)
    {
        DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_WARNING,
            "%s %d warning ie id=%d len=%d maxlen=%d\n",
            __func__, __LINE__, src[0], len, max_ie_data_len);
        len = max_ie_data_len;
    }
    dst[1] = len;
    memcpy(dst+2, src+2, len);
    return (len+2);
}

static int
cfg80211_informbss_cb(void *arg, const struct wifi_scan_info *se)
{
    struct iwscanreq *req = arg;
    struct wlan_net_vif *wnet_vif = req->wnet_vif;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wiphy *wiphy = wnet_vif->vm_wdev->wiphy;
    const struct wifi_scan_info *lse = se;
    struct scaninfo_table *st = wifimac->wm_scan->ScanTablePriv;

    int ret = 0;
    struct ieee80211_channel *notify_channel;
    unsigned short channel;
    unsigned int freq;

    struct cfg80211_bss *bss;
    int rssi;
    unsigned short cap_info_bitmap;
    unsigned short beacon_period;
    u64 timestamp;

    unsigned char *buf, *pbuf;
    size_t len = 0;
    size_t ielen;
    int index = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
    struct wifi_frame *p80211mgmt;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    struct timespec ts;
    get_monotonic_boottime(&ts);
#endif

    if ((lse == NULL) || (lse->SI_chan == NULL))
        goto exit;

    channel = lse->SI_chan->chan_pri_num;
    if (channel <= AML_2G_CHANNELS_NUM)
        freq = aml_ieee80211_chan_to_frq(channel, IEEE80211_BAND_2GHZ);
    else
        freq = aml_ieee80211_chan_to_frq(channel, IEEE80211_BAND_5GHZ);

    notify_channel = ieee80211_get_channel(wiphy, freq);
    if (!notify_channel) {
        DPRINTF(AML_DEBUG_ERROR, "%s channel info is null\n", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    rssi = 100 * translate_to_dbm(lse->SI_rssi);
    if ((rssi > -2000) || (rssi < -10000)) {
        //DPRINTF(AML_DEBUG_ERROR, "%s rssi exceeds scope:%d\n", __func__, rssi);
        rssi = -2000;
    }

    buf = ZMALLOC(AML_SCAN_IE_LEN_MAX,"buf", GFP_ATOMIC);
    if (!buf)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d ERROR NOMEM\n", __func__, __LINE__);
        ret = -ENOMEM;
        goto exit;
    }
    pbuf = buf;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
    p80211mgmt = (struct wifi_frame *)buf;
    p80211mgmt->i_fc[0] = WIFINET_FC0_VERSION_0 | WIFINET_FC0_TYPE_MGT | WIFINET_FC0_SUBTYPE_PROBE_RESP;
    memcpy(p80211mgmt->i_addr1, wnet_vif->vm_myaddr, ETH_ALEN);
    memcpy(p80211mgmt->i_addr2, lse->SI_macaddr, ETH_ALEN);
    memcpy(p80211mgmt->i_addr3, lse->SI_bssid, ETH_ALEN);
    pbuf += sizeof(struct wifi_frame);
    len += sizeof (struct wifi_frame);

    //memcpy(pbuf, lse->SI_tstamp.data, sizeof(u_int64_t));     //default to zero
    pbuf += sizeof(u_int64_t);
    len += sizeof(u_int64_t);

    memcpy(pbuf, &(lse->SI_intval), sizeof(unsigned short));
    pbuf += sizeof(unsigned short);
    len += sizeof(unsigned short);

    memcpy(pbuf, &(lse->SI_capinfo), sizeof(unsigned short));
    pbuf += sizeof(unsigned short);
    len += sizeof(unsigned short);
#endif

    {
        unsigned char  xse_ssid[2+WIFINET_NWID_LEN] = {0};
        memcpy(xse_ssid, lse->SI_ssid, WIFINET_NWID_LEN+2);
        ielen = wifi_mac_copy_ie(pbuf, xse_ssid, WIFINET_NWID_LEN);
        pbuf += ielen;
        len += ielen;
    }

    ielen = wifi_mac_copy_ie(pbuf, lse->SI_rates, WIFINET_RATE_MAXSIZE);
    pbuf += ielen;
    len += ielen;

    if (lse->SI_exrates[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_exrates, WIFINET_RATE_MAXSIZE);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_erp)
    {
        pbuf[0] = WIFINET_ELEMID_ERP;
        pbuf[1] = 1;
        pbuf[2] = lse->SI_erp;
        pbuf += sizeof(struct wifi_mac_erp_ie);
        len += sizeof(struct wifi_mac_erp_ie);
    }

    if (lse->SI_rsn_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_rsn_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_rsnx_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_rsnx_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_country_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_country_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_bss_load_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_bss_load_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_htcap_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_htcap_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_htinfo_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_htinfo_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_wpa_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_wpa_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

    if (lse->SI_wme_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_wme_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
    if (lse->SI_wps_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_wps_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }

#ifdef CONFIG_WAPI
    if (lse->SI_wai_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_wai_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
#endif
#ifdef CONFIG_P2P
    for (index = 0; index < MAX_P2PIE_NUM; index++)
    {
        if (lse->SI_p2p_ie[index][1])
        {
            ielen = wifi_mac_copy_ie(pbuf, lse->SI_p2p_ie[index], SCANINFO_IE_DATA_LENGTH);
            pbuf += ielen;
            len += ielen;
            DPRINTF(AML_DEBUG_SCAN, "<%s>: scan %s %s %s,notify_channel = %d,channel %d\n",
             wnet_vif->vm_ndev->name ,__func__, ssidie_sprintf(lse->SI_ssid), ether_sprintf(lse->SI_macaddr), freq, channel);
        }
    }
#endif//CONFIG_P2P
#ifdef CONFIG_WFD
    if (lse->SI_wfd_ie[1])
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->SI_wfd_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
#endif//CONFIG_WFD
    if (lse->ie_ext_cap[0] == WIFINET_ELEMID_EXTCAP )
    {
       ielen = wifi_mac_copy_ie(pbuf, lse->ie_ext_cap, SCANINFO_IE_DATA_LENGTH);
       ie_dbg(pbuf);
       pbuf += ielen;
       len += ielen;
    }

     if(lse->ie_vht_cap[0] == WIFINET_ELEMID_VHTCAP )
     {
        ielen = wifi_mac_copy_ie(pbuf, lse->ie_vht_cap, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len += ielen;
     }

     if(lse->ie_vht_opt[0] == WIFINET_ELEMID_VHTOP)
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->ie_vht_opt, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len += ielen;
    }

    if(lse->ie_vht_tx_pwr[0] == WIFINET_ELEMID_VHT_TX_PWR_ENVLP)
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->ie_vht_tx_pwr, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len  += ielen;
    }

    if(lse->ie_vht_ch_sw[0] == WIFINET_ELEMID_CHAN_SWITCH_WRAP)
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->ie_vht_ch_sw, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len += ielen;
    }

    if(lse->ie_vht_ext_bss_ld[0] == WIFINET_ELEMID_EXT_BSS_LOAD)
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->ie_vht_ext_bss_ld, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len += ielen;
    }

    if(lse->ie_vht_quiet_ch[0] == WIFINET_ELEMID_QUIET_CHANNEL)
    {
        ielen =  wifi_mac_copy_ie(pbuf, lse->ie_vht_quiet_ch, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len += ielen;
    }

    if(lse->ie_vht_opt_md_ntf[0] == WIFINET_ELEMID_OP_MODE_NOTIFY)
    {
        ielen = wifi_mac_copy_ie(pbuf, lse->ie_vht_opt_md_ntf, SCANINFO_IE_DATA_LENGTH);
        ie_dbg(pbuf);
        pbuf += ielen;
        len += ielen;
    }

    cap_info_bitmap = le16_to_cpu(lse->SI_capinfo);
    beacon_period = le16_to_cpu(lse->SI_intval);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0))
    timestamp = ktime_to_us(ktime_get_boottime());
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    timestamp = ((u64)ts.tv_sec * 1000000) + ts.tv_nsec / 1000;
#endif
    AML_PRINT(AML_DBG_MODULES_SCAN, "timestamp:%lld %s, rssi:%d, bw %d, chan_freq %d, pri_channel %d\n", timestamp, lse->SI_ssid + 2, rssi,
        lse->SI_chan->chan_bw, lse->SI_chan->chan_cfreq1, lse->SI_chan->chan_pri_num);

    WIFI_SCAN_SE_LIST_UNLOCK(st);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
    bss = cfg80211_inform_bss_frame(wiphy, notify_channel, (struct ieee80211_mgmt *)buf, len, rssi, GFP_ATOMIC);
#else
    bss = cfg80211_inform_bss(wiphy, notify_channel, CFG80211_BSS_FTYPE_UNKNOWN, lse->SI_bssid,
        timestamp, cap_info_bitmap, beacon_period, buf, len, rssi, GFP_KERNEL);
#endif

    if (unlikely(!bss))
    {
        DPRINTF(AML_DEBUG_ERROR, "%s %d vm_cfg80211_inform_bss error\n", __func__, __LINE__);
        ret = -EINVAL;
        WIFI_SCAN_SE_LIST_LOCK(st);
        goto malloc_exit;
    }

    cfg80211_put_bss(wiphy, bss);
    WIFI_SCAN_SE_LIST_LOCK(st);

malloc_exit:
    FREE(buf,"buf");
exit:
    return ret;
}

static unsigned char*
vm_get_wps_ie(unsigned char *in_ie, uint in_len,
    unsigned char *wps_ie, uint *wps_ielen)
{
    uint cnt;
    unsigned char *wpsie_ptr=NULL;
    unsigned char eid;

    ASSERT(wps_ielen != NULL);

    *wps_ielen = 0;
    if (!in_ie || in_len<=0)
        return wpsie_ptr;

    cnt = 0;
    while (cnt<in_len)
    {
        eid = in_ie[cnt];
        if ((eid==WIFINET_ELEMID_VENDOR)&&iswpsoui(&in_ie[cnt]))
        {
            wpsie_ptr = &in_ie[cnt];
            *wps_ielen = saveie(wps_ie,wpsie_ptr);
            cnt+=in_ie[cnt+1]+2;

            break;
        }
        else
        {
            cnt+=in_ie[cnt+1]+2; //goto next
        }

    }

    return wpsie_ptr;
}
static unsigned char*
vm_get_ie(const unsigned char *in_ie, uint in_len,
    unsigned char *save_ie, unsigned int *out_ielen,
    unsigned char ieid, uint be_oui)
{
    uint cnt;
    unsigned char *ie_ptr=NULL;
    ASSERT(out_ielen != NULL);
    *out_ielen = 0;
    if (!in_ie || in_len<=0)
        return ie_ptr;
    cnt = 0;
    while (cnt<in_len)
    {
        if (in_ie[cnt] == ieid)
        {
            if ((ieid == WIFINET_ELEMID_VENDOR)
                && (READ_32B((unsigned char *)(in_ie+cnt+2))!=be_oui))
            {
                cnt+=in_ie[cnt+1]+2;
                continue;
            }
            ie_ptr = (unsigned char *)&in_ie[cnt];
            *out_ielen = saveie(save_ie,ie_ptr);
            cnt+=in_ie[cnt+1]+2;
            break;
        }
        else
        {
            cnt+=in_ie[cnt+1]+2;
            continue;
        }
    }
    return ie_ptr;
}

static int
vm_get_vendor_ie(const unsigned char *in_ie, uint in_len,
    unsigned char *save_ie, unsigned char out_ielen[],
    unsigned char ieid, unsigned char* ie_ptr[])
{
    uint cnt;
    unsigned char index = 0;
    ASSERT(out_ielen != NULL);
    *out_ielen = 0;

    if (!in_ie || in_len <= 0)
        return -1;

    cnt = 0;
    while (cnt<in_len && index < VENDOR_IE_MAX)
    {
        if (in_ie[cnt] == ieid)
        {
            if ((ieid == WIFINET_ELEMID_VENDOR)
                && (READ_32B((unsigned char *)(in_ie+cnt+2))==WPA_OUI_BE
                   || READ_32B((unsigned char *)(in_ie+cnt+2))==WSC_OUI_BE
                   || READ_32B((unsigned char *)(in_ie+cnt+2))==P2P_OUI_BE
                   || READ_32B((unsigned char *)(in_ie+cnt+2))==WME_OUI_BE))
            {
                cnt+=in_ie[cnt+1]+2;
                continue;
            }
            ie_ptr[index] = (unsigned char *)&in_ie[cnt+2];
            out_ielen[index] = (unsigned char)in_ie[cnt+1];
            cnt+=in_ie[cnt+1]+2;
            index++;
        }
        else
        {
            cnt+=in_ie[cnt+1]+2;
            continue;
        }
    }
    return 0;
}

static int
vm_p2p_set_probersp_ie(struct wlan_net_vif *wnet_vif, char *frm, int len)
{
    int ret = 0;
    struct wifi_mac_app_ie_t *wps_app_ie = &wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_RESP];
    int tmplen = 0;
    unsigned char *tmp = NULL;
#ifdef CONFIG_WFD
    unsigned char wfd_ie[WIFINET_MAX_WDF_IE] = {0};
#endif

#ifdef  CONFIG_P2P
    const unsigned char *p2p_ie = NULL;

    {
        struct wifi_mac_app_ie_t *p2p_app_ie = &wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP];
        p2p_ie = vm_get_p2p_ie(frm, len, NULL, &tmplen);
        wifi_mac_save_app_ie(p2p_app_ie, p2p_ie, tmplen);
    }
#endif

    tmp = vm_get_wps_ie(frm, len,NULL, &tmplen);
    wifi_mac_save_app_ie(wps_app_ie, tmp, tmplen);

#ifdef CONFIG_WFD
    {
        struct wifi_mac_app_ie_t *wfd_app_ie = &wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP];
        tmp = vm_get_wfd_ie(frm, len, NULL, &tmplen);

        if (((tmp == NULL) && (p2p_ie != NULL)) && wnet_vif->vm_p2p->wfd_info.wfd_enable) {
            tmplen = vm_wfd_add_probersp_ie(wnet_vif->vm_p2p,wfd_ie);
            tmp = wfd_ie;
        }
        wifi_mac_save_app_ie(wfd_app_ie,tmp,tmplen);
    }
#endif //CONFIG_WFD

    DPRINTF(AML_DEBUG_CFG80211, "%s set_probersp %s wps_ie len %d\n", __func__, tmp, tmplen);
    return ret;
}

static int
vm_p2p_set_probereq_ie(struct wlan_net_vif *wnet_vif, char *frm, int len)
{
    int ret = 0;
#ifdef  CONFIG_P2P
    int tmplen=0;
    unsigned char * tmp = NULL;
    const unsigned char * p2p_ie = NULL;
#endif

#ifdef  CONFIG_P2P
    {
        //struct wifi_mac_app_ie_t  *p2p_app_ie=&vmac->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_REQ];

        p2p_ie =vm_get_p2p_ie(frm, len,NULL, &tmplen);
        if (p2p_ie && tmplen)
        {
            //DPRINTF(AML_DEBUG_P2P,"%s %d found p2p ie\n", __func__, __LINE__);
            //vm_change_p2pie_listenchannel(vmac->vm_p2p,p2p_ie,tmplen);
        }
    }
#endif
#ifdef CONFIG_WFD
    {
        struct wifi_mac_app_ie_t *wfd_app_ie = &wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_REQ];
        unsigned char wfd_ie[WIFINET_MAX_WDF_IE]= {0};
        tmp =vm_get_wfd_ie(frm, len,NULL, &tmplen);

        if (((tmp == NULL) && (p2p_ie != NULL))
            && (wnet_vif->vm_p2p->wfd_info.wfd_enable==true))
        {
            tmplen=vm_wfd_add_probereq_ie(wnet_vif->vm_p2p,wfd_ie);
            tmp = wfd_ie;
        }
        wifi_mac_save_app_ie(wfd_app_ie,tmp,tmplen);
    }
#endif //CONFIG_WFD
    return ret;

}

static int
vm_p2p_set_assocrsp_ie(struct wlan_net_vif *wnet_vif, char *frm, int len)
{
    int ret = 0;
    struct wifi_mac_app_ie_t *wps_app_ie=&wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP];
    int tmplen=0;
#ifdef  CONFIG_P2P
    const unsigned char * p2p_ie = NULL;
#endif

    unsigned char * tmp = NULL;
    AML_PRINT(AML_DBG_MODULES_P2P, "++ len=%d\n", len);

#ifdef  CONFIG_P2P
    {
        struct wifi_mac_app_ie_t *p2p_app_ie = &wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP];
        p2p_ie =vm_get_p2p_ie(frm, len,NULL, &tmplen);
        wifi_mac_save_app_ie(p2p_app_ie,p2p_ie,tmplen);
#if 1
        if (p2p_ie && tmplen && (WIFINET_M_HOSTAP == wnet_vif->vm_opmode))
        {
            AML_PRINT(AML_DBG_MODULES_P2P, " found p2p ie vm_opmode %d\n",  wnet_vif->vm_opmode);
        }
#endif
    }
#endif
    tmp =vm_get_wps_ie(frm, len,NULL, &tmplen);
    wifi_mac_save_app_ie(wps_app_ie,tmp,tmplen);
#ifdef CONFIG_WFD
    {
        struct wifi_mac_app_ie_t *wfd_app_ie = &wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP];
        unsigned char wfd_ie[WIFINET_MAX_WDF_IE]= {0};
        tmp =vm_get_wfd_ie(frm, len,NULL, &tmplen);
        if (((tmp == NULL)&&(p2p_ie != NULL))&&(wnet_vif->vm_p2p->wfd_info.wfd_enable==true))
        {
            tmplen=vm_wfd_add_assocrsp_ie(wnet_vif->vm_p2p,wfd_ie);
            tmp = wfd_ie;
        }
        wifi_mac_save_app_ie(wfd_app_ie,tmp,tmplen);
    }
#endif //CONFIG_WFD
    return ret;

}

static int
vm_p2p_set_assocreq_ie(struct wlan_net_vif *wnet_vif, char *frm, int len)
{
    int ret = 0;
#ifdef  CONFIG_P2P
    int tmplen=0;
    unsigned char * tmp = NULL;
    const unsigned char * p2p_ie = NULL;
#endif

#ifdef  CONFIG_P2P
    {
        p2p_ie =vm_get_p2p_ie(frm, len,NULL, &tmplen);
#if 1
        if (p2p_ie && tmplen && (WIFINET_M_HOSTAP == wnet_vif->vm_opmode))
        {
           AML_PRINT(AML_DBG_MODULES_P2P, "found p2p ie\n");
        }
#endif
    }
#endif

#ifdef CONFIG_WFD
    {
        struct wifi_mac_app_ie_t  *wfd_app_ie=&wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ];
        unsigned char wfd_ie[WIFINET_MAX_WDF_IE]= {0};
        tmp =vm_get_wfd_ie(frm, len,NULL, &tmplen);
        if (((tmp == NULL)&&(p2p_ie != NULL))&&(wnet_vif->vm_p2p->wfd_info.wfd_enable==true))
        {
            tmplen=vm_wfd_add_assocreq_ie(wnet_vif->vm_p2p,wfd_ie);
            tmp = wfd_ie;
            wifi_mac_save_app_ie(wfd_app_ie,tmp,tmplen);
        }
    }
#endif //#ifdef CONFIG_WFD
    return ret;

}

int vm_p2p_update_beacon_app_ie (struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac_app_ie_t  *beacon_app_ie=&(wnet_vif->app_ie[WIFINET_APPIE_FRAME_BEACON]);
    int total_beacon_app_ie_len = 0;
    void * ie=NULL, *ptr=NULL;

#ifdef  CONFIG_P2P
    total_beacon_app_ie_len += wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON].length;
    total_beacon_app_ie_len += wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON].length;
    total_beacon_app_ie_len += wnet_vif->vm_p2p->wps_beacon_ie.length;
#endif

#ifdef CONFIG_WFD
    total_beacon_app_ie_len += wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON].length;
#endif
    if (!total_beacon_app_ie_len)
    {
        DPRINTF(AML_DEBUG_ERROR,"%s %d total_beacon_app_ie_len=%d\n",
                __func__, __LINE__, total_beacon_app_ie_len);
        return -1;
    }

    wifi_mac_rm_app_ie(beacon_app_ie);

    ie = NET_MALLOC(total_beacon_app_ie_len, GFP_ATOMIC, "vm_p2p_update_beacon_app_ie.ie");
    if (ie == NULL)
    {
        DPRINTF(AML_DEBUG_ERROR,"%s %d no mem\n", __func__, __LINE__);
        return -ENOMEM;
    }
    ptr = ie;
    if (wnet_vif->vm_p2p->wps_beacon_ie.length)
    {
        memcpy(ptr, wnet_vif->vm_p2p->wps_beacon_ie.ie,
               wnet_vif->vm_p2p->wps_beacon_ie.length);
        ptr += wnet_vif->vm_p2p->wps_beacon_ie.length;
    }

#ifdef CONFIG_WFD
    if (wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON].length)
    {
        memcpy(ptr, wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON].ie,
               wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON].length);
        ptr += wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON].length;
    }
#endif

#ifdef  CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON].length)
    {
        memcpy(ptr, wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON].ie,
               wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON].length);
        ptr += wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON].length;
    }
    if (wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON].length)
    {
        memcpy(ptr, wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON].ie,
               wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON].length);
        ptr += wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON].length;
    }

#endif

    beacon_app_ie->ie = ie;
    beacon_app_ie->length = total_beacon_app_ie_len;

    wnet_vif->vm_flags_ext |= WIFINET_FEXT_APPIE_UPDATE;
    return 0;
}

static int
vm_p2p_set_beacon_ie(struct wlan_net_vif *wnet_vif, char *frm, int len)
{
    int ret = 0;
#ifdef  CONFIG_P2P
    int tmplen=0;
    unsigned char * tmp = NULL;
    const unsigned char * p2p_ie = NULL;
#endif
    DPRINTF(AML_DEBUG_SCAN,"%s ++\n", __func__);
#ifdef  CONFIG_P2P
    {
        struct wifi_mac_app_ie_t *p2p_app_ie =
            &wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON];

        p2p_ie =vm_get_p2p_ie(frm, len,NULL, &tmplen);
#if 1
        if (p2p_ie && tmplen && (WIFINET_M_HOSTAP == wnet_vif->vm_opmode))
        {
            AML_PRINT(AML_DBG_MODULES_P2P, "found p2p ie\n");
        }
#endif
        wifi_mac_save_app_ie(p2p_app_ie, p2p_ie, tmplen);
    }
    {
        struct wifi_mac_app_ie_t  *wps_app_ie=&wnet_vif->vm_p2p->wps_beacon_ie;

        tmp =vm_get_wps_ie(frm, len,NULL, &tmplen);
        wifi_mac_save_app_ie(wps_app_ie,tmp,tmplen);
    }
#endif

#ifdef CONFIG_WFD
    {
        struct wifi_mac_app_ie_t *wfd_app_ie =
            &wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON];
        unsigned char wfd_ie[WIFINET_MAX_WDF_IE] = {0};
        p2p_ie =vm_get_p2p_ie(frm, len,NULL, &tmplen);
        tmp =vm_get_wfd_ie(frm, len,NULL, &tmplen);
        if (((tmp == NULL) && (p2p_ie != NULL))
            && (wnet_vif->vm_p2p->wfd_info.wfd_enable == true))
        {
            tmplen=vm_wfd_add_beacon_ie(wnet_vif->vm_p2p,wfd_ie);
            tmp = wfd_ie;
        }
        wifi_mac_save_app_ie(wfd_app_ie,tmp,tmplen);
    }
#endif //#ifdef CONFIG_WFD
    ret = vm_p2p_update_beacon_app_ie(wnet_vif);
    return ret;
}

int vm_p2p_set_wpsp2pie(struct net_device *dev, char *frm, int len, int type)
{
    int ret = 0;
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);

    if (dev == NULL)
    {
        DPRINTF(AML_DEBUG_ANY,"%s ++ type %d ,dev=NULL drop !!!\n", __func__,type);
        return ret;
    }
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> type=%d\n", __func__, __LINE__, dev->name, type);

    {
        WIFINET_BEACON_LOCK(wnet_vif->vm_wmac);
        switch (type)
        {
            case P2P_BEACON_IE:
                ret = vm_p2p_set_beacon_ie(wnet_vif, frm, len);
                break;
            case P2P_PROBE_RESP_IE:
                ret = vm_p2p_set_probersp_ie(wnet_vif, frm, len);
                break;
            case P2P_ASSOC_RESP_IE:
                ret = vm_p2p_set_assocrsp_ie(wnet_vif, frm, len);
                break;
            case P2P_ASSOC_REQ_IE:
                ret = vm_p2p_set_assocreq_ie(wnet_vif, frm, len);
                break;
            case P2P_PROBE_REQ_IE:
                ret = vm_p2p_set_probereq_ie(wnet_vif, frm, len);
                break;
        }
        WIFINET_BEACON_UNLOCK(wnet_vif->vm_wmac);
    }
    return ret;

}
int vm_p2p_set_p2p_noa(struct net_device *dev, char* buf, int len)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    // struct p2p_noa        *p_noa = NULL;
    int ret = -1;
    int count, start, duration;
#ifdef  CONFIG_P2P
    struct wireless_dev *pwdev = wnet_vif->vm_wdev;
#endif

    if (buf == NULL) {
        return 0;
    }

#if 1
    if (buf && len && (WIFINET_M_HOSTAP == wnet_vif->vm_opmode))
    {
        AML_PRINT(AML_DBG_MODULES_P2P, "len=%d\n", len);
        //dump_memory_internal(buf, len);
    }
#endif

    sscanf(buf, "%d %d %d", &count, &start, &duration);
    AML_PRINT(AML_DBG_MODULES_P2P, "<%s> count=%d start=%d duration=%d\n",
        dev->name, count, start, duration);

#ifdef  CONFIG_P2P
    if (pwdev->iftype == NL80211_IFTYPE_P2P_GO)
    {
        struct p2p_noa noa = {0};
        do
        {
            if ((count < 0) || (start < 0) || (duration < 0)
                || (count > NET80211_P2P_SCHED_REPEAT))
            {
                ERROR_DEBUG_OUT("illegal setting for count=%d start=%d duration=%d\n",
                    count, start, duration);
                break;
            }

            if ((count==NET80211_P2P_SCHED_RSVD) && (duration>0))
            {
                ERROR_DEBUG_OUT("illegal setting for count=%d duration=%d\n",
                    count, duration);
                break;
            }

            if ((count==NET80211_P2P_SCHED_RSVD) || (duration==0))
            {
                noa.count = 0;
                noa.interval = 0;
                noa.duration = 0;
                noa.start = 0;
            }
            else
            {
                noa.count = count;
                noa.duration = duration*1000;
                noa.interval = start*1000;
            }
            wnet_vif->vm_p2p->noa_index ++;
            vm_p2p_noa_start(wnet_vif->vm_p2p, &noa);
            ret = vm_p2p_update_noa_ie(wnet_vif->vm_p2p);
        }
        while (0);
    }
    else
    {
        ERROR_DEBUG_OUT("not supported, iftype=%d\n", pwdev->iftype);
    }
#endif

    return ret;
}

int vm_p2p_set_p2p_ps(struct net_device *dev, char* buf, int len)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
#ifdef  CONFIG_P2P
    struct wireless_dev *pwdev = wnet_vif->vm_wdev;
    static union type_ctw_opps_u ctw_opps_u;
#endif
    int ret = -1;
    int legacy_ps, opps, ctw;

    sscanf(buf, "%d %d %d", &legacy_ps, &opps, &ctw);
    AML_PRINT(AML_DBG_MODULES_P2P, "<%s> legacy_ps %d opps %d ctw %d\n",
             dev->name, legacy_ps, opps, ctw);

#ifdef  CONFIG_P2P
    if (pwdev->iftype == NL80211_IFTYPE_P2P_GO)
    {
        memset(&ctw_opps_u,0,sizeof(union type_ctw_opps_u));
        if (ctw != -1)
        {
            if (ctw <= 0x7f){
                ctw_opps_u.ctw_opps_s.ctw = ctw;
                if(wnet_vif->vm_p2p->ctw_opps_u.ctw_opps_s.ctw != ctw_opps_u.ctw_opps_s.ctw)
                    wnet_vif->vm_p2p->ctw_opps_u.ctw_opps_s.ctw = ctw_opps_u.ctw_opps_s.ctw;
            }
            else
                ERROR_DEBUG_OUT("illegal setting for ctw=%d\n", ctw);
        }

        if (opps != -1)
        {
            do
            {
                if ((opps != 0) && (opps != 1))
                {
                    ERROR_DEBUG_OUT("opps=%d not support\n", opps);
                    break;
                }

                if ((opps == 1) && (!wnet_vif->vm_p2p->ctw_opps_u.ctw_opps_s.ctw))
                {
                    ERROR_DEBUG_OUT( "ctw=%d need to set\n", ctw);
                    break;
                }

                ctw_opps_u.ctw_opps_s.opps = opps;
                ctw_opps_u.ctw_opps_s.ctw = wnet_vif->vm_p2p->ctw_opps_u.ctw_opps_s.ctw;
                wnet_vif->vm_p2p->noa_index ++;
                vm_p2p_opps_start(wnet_vif->vm_p2p, &ctw_opps_u);
                ret = vm_p2p_update_noa_ie(wnet_vif->vm_p2p);
            }
            while (0);
        }

        switch (legacy_ps)
        {
            case PM_OFF:
                wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_NONE);
                break;
            case PM_MAX:
                wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_NORMAL);
                break;
            case -1:
                break;
            default:
               ERROR_DEBUG_OUT("illegal setting for legacy_ps=%d\n", legacy_ps);
                break;
        }
    }
    else
    {
         ERROR_DEBUG_OUT("not supported, iftype=%d\n", pwdev->iftype);
    }
#endif

    return ret;
}

int vm_cfg80211_inform_bss (struct wlan_net_vif *wnet_vif)
{
    int ret = 0;
    struct iwscanreq req;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);

    DPRINTF(AML_DEBUG_CFG80211, "%s %d pwdev_priv->scan_request=%p wnet_vif_id=%d\n",
            __func__, __LINE__, pwdev_priv->scan_request,wnet_vif->wnet_vif_id);

    req.wnet_vif = wnet_vif;
    if (pwdev_priv->scan_request != NULL)
    {
        ret = wifi_mac_scan_parse(wnet_vif, cfg80211_informbss_cb, &req);
    }
    return ret;
}

void
vm_cfg80211_indicate_scan_done(struct vm_wdev_priv *pwdev_priv,
    unsigned char aborted)
{
    struct cfg80211_scan_request *scan_request = NULL;
    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
        struct cfg80211_scan_info info = {
            .aborted = true,
        };
    #endif

    scan_request = pwdev_priv->scan_request;
    if (scan_request != NULL)
    {
        DPRINTF(AML_DEBUG_SCAN,"<%s> :%s++ %d scan_request=%p aborted=%d\n",
            pwdev_priv->wnet_vif->vm_ndev->name, __func__, __LINE__,scan_request, aborted);

        OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
        pwdev_priv->scan_request = NULL;

        if (scan_request->wiphy != priv_to_wiphy(pwdev_priv))
        {
            DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_SCAN,
                    "%s %d error wiphy compare\n", __func__, __LINE__);
        }
        else
        {
            wifi_mac_rm_app_ie(&(pwdev_priv->wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ]));
            #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
                info.aborted = aborted;
                cfg80211_scan_done(scan_request, &info);
            #else
                cfg80211_scan_done(scan_request, aborted);
            #endif
        }
        OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);

        DPRINTF(AML_DEBUG_SCAN,"%s <%s> scan_request=%p\n", __func__, pwdev_priv->wnet_vif->vm_ndev->name, scan_request);

#ifdef  CONFIG_P2P
        if (pwdev_priv->wnet_vif->vm_p2p_support)
        {
            vm_p2p_scanend(&(pwdev_priv->p2p));
        }
#endif
    }

    return;
}

void vm_cfg80211_indicate_connect(struct wlan_net_vif *wnet_vif)
{
    struct wireless_dev *pwdev;
    struct vm_wdev_priv *pwdev_priv;
    struct wifi_station *sta;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    struct cfg80211_bss *bss = NULL;
    struct ieee80211_channel *notify_channel = NULL;
    unsigned short channel;
    unsigned int freq;
#endif

    if (!wnet_vif)
        return;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    if (!wnet_vif->vm_curchan)
        return;

    channel = wnet_vif->vm_curchan->chan_pri_num;
#endif

    pwdev = wnet_vif->vm_wdev;
    pwdev_priv = wdev_to_priv(pwdev);
    sta = wnet_vif->vm_mainsta;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, VMAC_DEV_NAME(wnet_vif));

    if ((pwdev->iftype != NL80211_IFTYPE_STATION)
#ifdef  CONFIG_P2P
       && (pwdev->iftype != NL80211_IFTYPE_P2P_CLIENT)
#endif       //CONFIG_P2P
      )
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d ERROR type=%d wnet_vif->vm_opmode = %d\n",
            __func__, __LINE__, pwdev->iftype, wnet_vif->vm_opmode);
        return;
    }

    DPRINTF(AML_DEBUG_CFG80211, "%s %d wnet_vif->vm_state=%d pwdev->iftype =%d\n",
        __func__, __LINE__, wnet_vif->vm_state,pwdev->iftype);
    pwdev_priv->connect_request = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    if (channel <= AML_2G_CHANNELS_NUM)
        freq = aml_ieee80211_chan_to_frq(channel, IEEE80211_BAND_2GHZ);
    else
        freq = aml_ieee80211_chan_to_frq(channel, IEEE80211_BAND_5GHZ);

    notify_channel = ieee80211_get_channel(wnet_vif->vm_wdev->wiphy, freq);
    if (!notify_channel) {
        ERROR_DEBUG_OUT("notify_channel info is null\n");
        return;
    }

    bss = cfg80211_get_bss(pwdev->wiphy, notify_channel, sta->sta_bssid, sta->sta_essid, sta->sta_esslen,
        IEEE80211_BSS_TYPE_ANY, IEEE80211_PRIVACY_ANY);

    if (bss != NULL) {
        cfg80211_connect_bss(wnet_vif->vm_ndev, sta->sta_bssid, bss, wnet_vif->assocreq_ie.ie, wnet_vif->assocreq_ie.length, wnet_vif->assocrsp_ie.ie,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 12, 0)
            wnet_vif->assocrsp_ie.length, WLAN_STATUS_SUCCESS, GFP_ATOMIC);
#else
            wnet_vif->assocrsp_ie.length, WLAN_STATUS_SUCCESS, GFP_ATOMIC,NL80211_TIMEOUT_UNSPECIFIED);
#endif
    } else {
        ERROR_DEBUG_OUT("can't find bss in kernel: %s\n", sta->sta_essid);
    }
#else
    cfg80211_connect_result(wnet_vif->vm_ndev, sta->sta_bssid,  wnet_vif->assocreq_ie.ie, wnet_vif->assocreq_ie.length, wnet_vif->assocrsp_ie.ie,
        wnet_vif->assocrsp_ie.length, WLAN_STATUS_SUCCESS, GFP_ATOMIC);
#endif

    DPRINTF(AML_DEBUG_CFG80211, "%s %d wnet_vif->vm_state=%d\n", __func__, __LINE__, wnet_vif->vm_state);
    return;
}

static void
vm_cfg80211_connect_timeout_task(SYS_TYPE net)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif*)net;
    vm_cfg80211_indicate_disconnect(wnet_vif);
    wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
    wnet_vif->vm_des_nssid = 0;
    memset(wnet_vif->vm_des_ssid, 0, IV_SSID_SCAN_AMOUNT*sizeof(struct wifi_mac_ScanSSID));
    return;
}

static int vm_cfg80211_connect_timeout_timer (void * arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)arg;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wireless_dev *pwdev = wnet_vif->vm_wdev;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(pwdev);
    static unsigned int connect_timeout_taskid = 0, connect_timeout_once = 0;

    DPRINTF(AML_DEBUG_CFG80211, "%s vid:%d, state:%d\n", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_state);
    if ((pwdev_priv->connect_request && (wnet_vif->vm_mainsta->connect_status != CONNECT_DHCP_GET_ACK))
        || (wifimac->recovery_stat != WIFINET_RECOVERY_END)) {
        if (connect_timeout_once == 0) {
            connect_timeout_taskid = wifi_mac_register_behindTask(wifimac, vm_cfg80211_connect_timeout_task);
            wifi_mac_call_task(wifimac,connect_timeout_taskid,(SYS_TYPE)arg);
            connect_timeout_once = 1;

        } else {
            wifi_mac_call_task(wifimac,connect_timeout_taskid,(SYS_TYPE)arg);
        }
    }
    return OS_TIMER_NOT_REARMED;
}

static void   vm_cfg80211_connect_result_ex (SYS_TYPE param1,
        SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param1;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, VMAC_DEV_NAME(wnet_vif));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    cfg80211_connect_bss(wnet_vif->vm_ndev, NULL, NULL, NULL, 0, NULL,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 12, 0)
        0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL);
#else
        0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL,NL80211_TIMEOUT_UNSPECIFIED);
#endif
#else
    cfg80211_connect_result(wnet_vif->vm_ndev, NULL, NULL, 0, NULL, 0,
        WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_ATOMIC);
#endif
}

static void vm_cfg80211_connect_result(struct wlan_net_vif *wnet_vif)
{
    wifi_mac_add_work_task(wnet_vif->vm_wmac, vm_cfg80211_connect_result_ex, NULL, (SYS_TYPE)wnet_vif, 0, 0, 0, 0);
}

void vm_cfg80211_indicate_disconnect(struct wlan_net_vif *wnet_vif)
{
    struct wireless_dev *pwdev = wnet_vif->vm_wdev;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(pwdev);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, VMAC_DEV_NAME(wnet_vif));

    if ((pwdev->iftype != NL80211_IFTYPE_STATION)
#ifdef CONFIG_P2P
        && (pwdev->iftype != NL80211_IFTYPE_P2P_CLIENT)
#endif
       )
    {
        DPRINTF(AML_DEBUG_ERROR, "%s %d ERROR type=%d\n", __func__, __LINE__, pwdev->iftype);
        return;
    }

    if (pwdev_priv->connect_request) {
        pwdev_priv->connect_request = NULL;
        os_timer_ex_cancel(&pwdev_priv->connect_timeout, CANCEL_SLEEP);
    }

    if (wnet_vif->vm_state == WIFINET_S_CONNECTING) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
        cfg80211_connect_bss(wnet_vif->vm_ndev, NULL, NULL, NULL, 0, NULL,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 12, 0)
            0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL);
#else
            0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL,NL80211_TIMEOUT_UNSPECIFIED);
#endif
#else
        cfg80211_connect_result(wnet_vif->vm_ndev, NULL, NULL, 0, NULL, 0,
            WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_ATOMIC);
#endif

    } else {
        #if LINUX_VERSION_CODE < KERNEL_VERSION(4,16,1)
            if ((wnet_vif->vm_flags & WIFINET_F_PRIVACY) && !(wnet_vif->vm_flags & WIFINET_F_WPA)) {
                vm_cfg80211_connect_result(wnet_vif);
            }
        #endif

        #if LINUX_VERSION_CODE > KERNEL_VERSION(4,2,0)
            cfg80211_disconnected(wnet_vif->vm_ndev, 0, NULL, 0, false,  GFP_ATOMIC);
        #else
            cfg80211_disconnected(wnet_vif->vm_ndev, 0, NULL, 0, GFP_ATOMIC);
        #endif
    }
    wnet_vif->vm_phase_flags = 0;
    wifimac->recovery_stat = WIFINET_RECOVERY_END;
    os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);
    return;
}

void vm_cfg80211_indicate_sta_assoc(const struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct net_device *dev = wnet_vif->vm_ndev;
    struct station_info sinfo;
    unsigned char *buf = NULL, *pbuf = NULL;
    size_t len = 0;
    int ielen = 0;
    int index = 0;
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, VMAC_DEV_NAME(wnet_vif));
    memset(&sinfo, 0, sizeof(struct station_info));
    //sinfo.generation = 0;
    //sinfo.filled = 0;
    //sinfo.signal = 0;

    //sinfo.assoc_req_ies = NULL;   //not ready for kernel=2.6.34
    //sinfo.assoc_req_ies_len = 0;  //not ready for kernel=2.6.34
#ifndef KERNEL_VERSION_SPECIFIC_AML_ICS_308
    buf = ZMALLOC(AML_SCAN_IE_LEN_MAX,"buf", GFP_ATOMIC);
    if (!buf)
    {
        DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_ERROR, "%s %d ERROR NOMEM\n", __func__, __LINE__);
        goto exit;
    }

    #if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
        //sinfo.filled |= STATION_INFO_ASSOC_REQ_IES;
    #else
        sinfo.filled |= STATION_INFO_ASSOC_REQ_IES;
    #endif

    pbuf = buf;
    len = 0;
    if (sta->sta_wpa_ie)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_wpa_ie\n", __func__, __LINE__);
        ielen = wifi_mac_copy_ie(pbuf, sta->sta_wpa_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
    if (sta->sta_rsn_ie)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_rsn_ie\n", __func__, __LINE__);
        ielen = wifi_mac_copy_ie(pbuf, sta->sta_rsn_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
    if (sta->sta_wps_ie)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_wps_ie\n", __func__, __LINE__);
        ielen = wifi_mac_copy_ie(pbuf, sta->sta_wps_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
    if (sta->sta_wme_ie)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_wme_ie\n", __func__, __LINE__);
        ielen = wifi_mac_copy_ie(pbuf, sta->sta_wme_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
#ifdef CONFIG_P2P
    for (index = 0; index < MAX_P2PIE_NUM; index++)
    {
        if (sta->sta_p2p_ie[index])
        {
            DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_p2p_ie\n", __func__, __LINE__);
            ielen = wifi_mac_copy_ie(pbuf, sta->sta_p2p_ie[index], SCANINFO_IE_DATA_LENGTH);
            pbuf += ielen;
            len += ielen;
        }
    }
#endif//CONFIG_P2P
#ifdef CONFIG_WFD
    if (sta->sta_wfd_ie)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_p2p_ie\n", __func__, __LINE__);
        ielen = wifi_mac_copy_ie(pbuf, sta->sta_wfd_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
#endif//CONFIG_WFD
#ifdef CONFIG_WAPI
    if (sta->sta_wai_ie)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d cp sta_wai_ie\n", __func__, __LINE__);
        ielen = wifi_mac_copy_ie(pbuf, sta->sta_wai_ie, SCANINFO_IE_DATA_LENGTH);
        pbuf += ielen;
        len += ielen;
    }
#endif
    sinfo.assoc_req_ies = buf;
    sinfo.assoc_req_ies_len = len;
#endif
	exit:
    cfg80211_new_sta(dev, sta->sta_macaddr, &sinfo, GFP_ATOMIC);

    //not ready for kernel=2.6.34
    //cfg80211_rx_mgmt(dev, freq, buf, len, gfp);

    FREE(buf,"buf");
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    return;
}
void
vm_cfg80211_indicate_sta_disassoc(const struct wifi_station *sta,
    unsigned short reason)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct net_device *dev = wnet_vif->vm_ndev;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, VMAC_DEV_NAME(wnet_vif));
    cfg80211_del_sta(dev, sta->sta_macaddr, GFP_ATOMIC);
    return;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
static struct wireless_dev * vm_cfg80211_add_vif(struct wiphy *wiphy,
        const char *name, unsigned char name_assign_type,
        enum nl80211_iftype type,
        struct vif_params *params)
#elif LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
static struct wireless_dev * vm_cfg80211_add_vif(struct wiphy *wiphy,
        const char *name, unsigned char name_assign_type,
        enum nl80211_iftype type, unsigned int *flags,
        struct vif_params *params)
#else
static struct wireless_dev * vm_cfg80211_add_vif(struct wiphy *wiphy,
        const char *name,
        enum nl80211_iftype type, unsigned int *flags,
        struct vif_params *params)
#endif

{
    int ret = -EOPNOTSUPP;
    struct net_device* ndev = NULL;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, name);

    switch (type)
    {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
            DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
            break;
        case NL80211_IFTYPE_MONITOR:
            DPRINTF(AML_DEBUG_CFG80211,
                "<%s>:%s %d MONITOR\n", wnet_vif->vm_ndev->name,__func__, __LINE__);
            ndev=vm_cfg80211_add_monitor_if(wnet_vif,name);
            if (ndev)
            {
                ret=0;
            }
            break;

        case NL80211_IFTYPE_STATION:
            DPRINTF(AML_DEBUG_CFG80211,
                "%s %d CLIENT\n", __func__, __LINE__);
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
        case NL80211_IFTYPE_P2P_GO:
            DPRINTF(AML_DEBUG_CFG80211,
                "<%s>: %s %d %s\n", wnet_vif->vm_ndev->name,__func__, __LINE__,name);
            //ndev = wifimac->wm_p2p_vmac;
            //p2p-p2p0-1
            // wifimac->wm_nops.wm_vmac_create(wifimac, name, 2,  nl80211_iftype_2_drv_opmode(type),  NULL);
            vm_cfg80211_add_p2p_go_if(wnet_vif,name,type);
            aml_wifi_set_con_mode(wnet_vif->vm_wmac);
            break;
        case NL80211_IFTYPE_AP:
            DPRINTF(AML_DEBUG_CFG80211, "%s %d AP\n", __func__, __LINE__);
            break;
        default:
            break;
    }
    return ndev ? ndev->ieee80211_ptr : ERR_PTR(ret);

}

static int
vm_cfg80211_del_vif(struct wiphy *wiphy,
    struct wireless_dev *wdev)
{
    struct net_device *ndev = wdev_to_ndev(wdev);
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    int ret = 0;

    DPRINTF(AML_DEBUG_ANY, "%s:%s: [%s(%d)]\n",
            wnet_vif->vm_ndev->name,ndev->name, __func__, __LINE__);
    if (!ndev)
    {
        ret = -EINVAL;
        goto exit;
    }

    if (ndev)
    {
        if (ndev->reg_state == NETREG_REGISTERED)
            unregister_netdevice(ndev);
        if (ndev->reg_state == NETREG_UNREGISTERED)
            free_netdev(ndev);
        if (ndev == pwdev_priv->pmon_ndev)
        {
            DPRINTF(AML_DEBUG_CFG80211, "%s remove monitor interface \n", __func__);
            pwdev_priv->pmon_ndev =     NULL;
            pwdev_priv->ifname_mon[0] = '\0';
        }
        if (ndev == pwdev_priv->pGo_ndev)
        {
#ifdef NEW_WIPHY
            wiphy_free(pwdev_priv->pGo_wdev->wiphy);
#endif//NEW_WIPHY
            FREE((unsigned char*)pwdev_priv->pGo_wdev,pwdev_priv->ifname_go);
            DPRINTF(AML_DEBUG_CFG80211, "%s remove pGo_ndev interface \n", __func__);
            pwdev_priv->pGo_ndev =  NULL;
            pwdev_priv->ifname_go[0] = '\0';
        }
    }
exit:
    return ret;
}

int wait_for_ap_run (struct wlan_net_vif *wnet_vif,
    int total_ms, int step_ms)
{
    int delay_ms = 0;
    int ret = -1;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d total_ms=%d step_ms=%d\n",
        __func__, __LINE__, total_ms, step_ms);
    while ((wnet_vif->vm_state != WIFINET_S_CONNECTED)
            && (delay_ms <= total_ms))
    {
        msleep(step_ms);
        delay_ms += step_ms;
    }
    if (wnet_vif->vm_state == WIFINET_S_CONNECTED)
    {
        ret  = 0;
    }
    return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
static int
vm_cfg80211_change_vif(struct wiphy *wiphy,
    struct net_device *ndev,enum nl80211_iftype type,
    struct vif_params *params)
#else
static int
vm_cfg80211_change_vif(struct wiphy *wiphy,
    struct net_device *ndev,enum nl80211_iftype type,
    unsigned int *flags,struct vif_params *params)
#endif
{
    enum nl80211_iftype old_nl80211type;
    enum wifi_mac_opmode networkType, oldnetworkType;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wireless_dev *pwdev = wiphy_to_wdev(wiphy);
    int ret = 0;
    int retry;
    for (retry = 0; ++retry; ) {
        if (!aml_insmod_flag) {
            printk("%s %d, aml insmod not complete\n",__func__,__LINE__);
            if (retry > 6) {
                return -1;
            } else {
                msleep(1000);
            }
        } else {
            break;
        }
    }

    old_nl80211type = pwdev->iftype;
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> old_iftype=%d, new_iftype=%d,<%s>\n",
            __func__, __LINE__, ndev->name, old_nl80211type, type,wnet_vif->vm_ndev->name);

    networkType = nl80211_iftype_2_drv_opmode(type);
    if (networkType < 0)
    {
        goto exit;
    }

    if (type == NL80211_IFTYPE_AP) {
        wnet_vif->vm_p2p_support = 0;
    }

    pwdev->iftype = type;
    ndev->ieee80211_ptr->iftype = type;

    if (ndev != wnet_vif->vm_ndev)
    {
        DPRINTF(AML_DEBUG_WARNING, "WARNING::%s %s old_iftype=%d, new_iftype=%d,%s\n",
            ndev->name, wnet_vif->vm_ndev->name, pwdev->iftype,type,__func__ );
        return  ret;
    }

    oldnetworkType = wnet_vif->vm_opmode;
    DPRINTF(AML_DEBUG_CFG80211, "%s %d old opmode=%d, new opmode=%d, p2p support %d\n",
            __func__, __LINE__, oldnetworkType, networkType, wnet_vif->vm_p2p_support);

#ifdef  CONFIG_P2P
    if (wnet_vif->vm_p2p_support)
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d old p2p_role=%d\n",
            __func__, __LINE__, vm_p2p_role(wnet_vif->vm_p2p));
        vm_p2p_set_role(wnet_vif->vm_p2p, NL80211_IFTYPE_2_p2p_role(type));
        DPRINTF(AML_DEBUG_CFG80211, "%s %d new p2p_role=%d\n",
            __func__, __LINE__, vm_p2p_role(wnet_vif->vm_p2p));
    }
#endif

    chan_dbg(wnet_vif->vm_wmac->wm_curchan, "x1", 1800);

    if (oldnetworkType != networkType)
    {
        struct vm_wlan_net_vif_params cp;

        DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
        memset(&cp, 0, sizeof(struct vm_wlan_net_vif_params));
        preempt_scan(wnet_vif->vm_ndev, 100, 100);
        strncpy(cp.vm_param_name,wnet_vif->vm_ndev->name, strlen(wnet_vif->vm_ndev->name));
        cp.vm_param_opmode = networkType;
        vm_wlan_net_vif_mode_change(wnet_vif,&cp);

        if ((WIFINET_M_HOSTAP == networkType) && (wnet_vif->vm_des_nssid))
        {
            DPRINTF(AML_DEBUG_CFG80211, "%s %d mode change over\n", __func__, __LINE__);
            wait_for_ap_run(wnet_vif, 10000, 100);
            DPRINTF(AML_DEBUG_CFG80211, "%s %d wait_for_ap_run over\n", __func__, __LINE__);
        }
        aml_wifi_set_con_mode(wnet_vif->vm_wmac);
    }

exit:
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    return ret;
}

static int
vm_cfg80211_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct net_device *ndev = wdev_to_ndev(request->wdev);
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct cfg80211_ssid *ssids = request->ssids;
    int nssids = request->n_ssids;
    struct wifi_mac_ScanSSID lssids[IV_SSID_SCAN_AMOUNT];
    int lnssids = 0;
    int lssidlen = 0;

    int i;
    int ret = 0;
    int ret_startscan = 0;
    struct wifi_mac_app_ie_t *probereq_app_ie = &wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ];

    if (aml_wifi_is_enable_rf_test())
        return 0;

    if (wnet_vif->vm_ndev && !(wnet_vif->vm_ndev->flags & IFF_RUNNING)) {
        ERROR_DEBUG_OUT("IFF_RUNNING\n");
        return -EINVAL;
    }

    DPRINTF(AML_DEBUG_CFG80211, "<%s>:%s vm_opmode:%d, vm_state:%d, connect_status:%d," \
        "nssids:%d, recovery_stat:%d, vm_phase_flags:%02x, wifimac->wm_flags:%08x, wifimac->wm_scan->scan_CfgFlags:%08x\n",
        ndev->name, __func__, wnet_vif->vm_opmode, wnet_vif->vm_state, wnet_vif->vm_mainsta->connect_status,
        nssids, wifimac->recovery_stat, wnet_vif->vm_phase_flags, wifimac->wm_flags, wifimac->wm_scan->scan_CfgFlags);


    if (wnet_vif->vm_opmode == WIFINET_M_STA) {
        if (((wnet_vif->vm_state >= WIFINET_S_CONNECTING) && (wnet_vif->vm_mainsta->connect_status != CONNECT_DHCP_GET_ACK))
            || ((wifimac->wm_flags & WIFINET_F_SCAN) && (wifimac->wm_scan->scan_CfgFlags & WIFINET_SCANCFG_CONNECT))
            || (wnet_vif->vm_phase_flags & (PHASE_CONNECTING|PHASE_DISCONNECTING))
            || (wifimac->recovery_stat != WIFINET_RECOVERY_END)
            || (wnet_vif->vm_chan_roaming_scan_flag)) {
            ERROR_DEBUG_OUT("rejected scan due to connecting\n");

            OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
            if (pwdev_priv->scan_request == NULL) {
                pwdev_priv->scan_request = request;
            }
            OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);

            wifi_mac_notify_scan_done(wnet_vif);
            return 0;
        }
    }

#ifdef CONFIG_P2P
    /*in order to p2p handshark protect from station scanning */
    if ((wnet_vif->vm_p2p_support == 0) && wifimac->wm_p2p_connection_protect) {
        if (time_after(jiffies, wifimac->wm_p2p_connection_protect_period)) {
            wifimac->wm_p2p_connection_protect = 0;

        } else {
            ERROR_DEBUG_OUT("rejected scan due to p2p negotiation\n");
            return -EINVAL;
        }
    }

    if ((wifimac->wm_nrunning == 1) && (wifimac->is_miracast_connect == 1) &&
        (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED)) {
        DPRINTF(AML_DEBUG_WARNING, "rejected wlan0 scan due to is_miracast_connect\n");
        return -EINVAL;
    }
#endif

    if (wifimac->wm_flags & WIFINET_F_CHANSWITCH) {
        DPRINTF(AML_DEBUG_WARNING, "%s(%d), rejected wlan0 scan due to channel switch\n",__func__,__LINE__);
        return -EINVAL;
    }

    /*need thinking more, maybe there are bugs happen*/
    if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode()) && (wifimac->wm_nrunning == 2) &&
        !concurrent_check_is_vmac_same_pri_channel(wifimac)) {
      DPRINTF(AML_DEBUG_WARNING, "%s(%d), rejected wlan0 scan due to softap working\n",__func__,__LINE__);
      return -EINVAL;
    }

    if (wifimac->wm_flags & WIFINET_F_SCAN) {
        if ((wnet_vif->wnet_vif_id != NET80211_MAIN_VMAC) || (pwdev_priv->scan_request == NULL)) {
            if (preempt_scan(ndev, 100, 100)) {
                return -EINVAL;
            }

        } else {
            DPRINTF(AML_DEBUG_WARNING, "%s wlan0 scan should not interrupt any scan, wm_flags:%08x\n", __func__, wifimac->wm_flags);
            return -EINVAL;
        }
    }

    if (pwdev_priv->scan_request != NULL) {
        DPRINTF(AML_DEBUG_WARNING |AML_DEBUG_CFG80211,
            "%s multiple scanreq, should not happen scan_request %p\n", __func__, pwdev_priv->scan_request);
        vm_cfg80211_indicate_scan_done(pwdev_priv, true);
    }

    if ((nssids > 0) && (request->ssids[0].ssid_len == 0)) {
        ssids = &request->ssids[1];
        nssids = request->n_ssids - 1;
    }

    OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
    pwdev_priv->scan_request = request;
    OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);

    if (request->ie && (request->ie_len > 0)) {
        DPRINTF(AML_DEBUG_SCAN,"%s %d ie_len=%zd\n", __func__, __LINE__, request->ie_len);
#ifdef  CONFIG_P2P
        vm_change_p2pie_listenchannel(wnet_vif->vm_p2p,request->ie, request->ie_len);
#endif
        wifi_mac_save_app_ie(probereq_app_ie, (unsigned char *)request->ie, request->ie_len);

    } else {
        wifi_mac_rm_app_ie(probereq_app_ie);
    }

    if (!IS_RUNNING(wnet_vif->vm_ndev))
    {
        DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_CFG80211,
                "<%s> %s %d flags %x,IFF_RUNNING= %x,IFF_UP=%x scan abort\n",
                ndev->name, __func__, __LINE__, wnet_vif->vm_ndev->flags, IFF_RUNNING, IFF_UP);

        ret = -EINVAL;
        goto exit;
    }

    memset(lssids, 0, IV_SSID_SCAN_AMOUNT*sizeof(struct wifi_mac_ScanSSID));
    for (i = 0; (i < nssids) && (i < IV_SSID_SCAN_AMOUNT); i++)
    {
        DPRINTF(AML_DEBUG_CFG80211, "<%s> %s ssid=%s, len=%d\n", ndev->name, __func__, ssids[i].ssid, ssids[i].ssid_len);
        lssids[i].len = ssids[i].ssid_len;
        memcpy(lssids[i].ssid, ssids[i].ssid, ssids[i].ssid_len);
        lnssids++;
        lssidlen += ssids[i].ssid_len;
    }

#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p_support)
    {
        vm_p2p_scanstart(wiphy, ndev, request);
        concurrent_channel_protection(wnet_vif, 1000);
    }
#endif //CONFIG_P2P
    DPRINTF(AML_DEBUG_CFG80211, "%s lnssids %d, lssidlen %d, n_channels:%d\n", __func__, lnssids, lssidlen, request->n_channels);

    if(wnet_vif->vm_state == WIFINET_S_INIT){
        wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
    }

    if (lnssids && lssidlen)
    {
        vm_scan_user_set_chan(wnet_vif, request);
        ret_startscan =  wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_CHANSET | WIFINET_SCANCFG_ACTIVE
            | WIFINET_SCANCFG_NOPICK | WIFINET_SCANCFG_USERREQ| WIFINET_SCANCFG_FLUSH, lnssids, lssids);
    }
    else
    {
        if (wnet_vif->vm_p2p_support) {
            vm_scan_user_set_chan(wnet_vif, request);
            ret_startscan =  wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_CHANSET | WIFINET_SCANCFG_ACTIVE
                | WIFINET_SCANCFG_NOPICK | WIFINET_SCANCFG_USERREQ| WIFINET_SCANCFG_FLUSH, lnssids, lssids);

        } else {
            ret_startscan =  wifi_mac_start_scan(wnet_vif, WIFINET_SCANCFG_ACTIVE | WIFINET_SCANCFG_NOPICK
                | WIFINET_SCANCFG_USERREQ | WIFINET_SCANCFG_FLUSH, 0, wnet_vif->vm_des_ssid);
        }
    }

    if (ret_startscan == 0)
    {
        ret = -EAGAIN;
        DPRINTF(AML_DEBUG_ANY, "<%s> :%s %d start scan fail, abort\n", ndev->name,__func__, __LINE__);
    }

exit:
    if (ret < 0)
    {
        DPRINTF(AML_DEBUG_ANY, "<%s> :%s %d scan_request=%p vid=%d scan abort\n",
            ndev->name,__func__, __LINE__, pwdev_priv->scan_request,wnet_vif->wnet_vif_id);

        OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
        pwdev_priv->scan_request = NULL;
        OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);
    }

    DPRINTF(AML_DEBUG_INFO, "Exit  %s %d\n", __func__, __LINE__);
    return ret;
}

static int vm_cfg80211_set_wiphy_params(struct wiphy *wiphy, int radio_idx, unsigned int changed)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    unsigned short retry_short = 0;
    unsigned short retry_long = 0;

    pr_debug("enter %s, %d\n", __func__, __LINE__);
    retry_short = wiphy->retry_short;
    retry_long = wiphy->retry_long;
    if (changed & WIPHY_PARAM_RETRY_LONG || changed & WIPHY_PARAM_RETRY_SHORT) {
        ERROR_DEBUG_OUT("retry_short:%d, retry_long:%d\n", retry_short, retry_long);
        if (!wnet_vif->vm_wmac->drv_priv->hal_priv) {
            ERROR_DEBUG_OUT("drv_priv->hal_priv = null\n");
            return -1;
        }
        drv_hal_setretrynum(retry_short << 8 | retry_long);
     }

    if (changed & WIPHY_PARAM_RTS_THRESHOLD &&
        wnet_vif->vm_rtsthreshold != wiphy->rts_threshold) {
        wnet_vif->vm_rtsthreshold = wiphy->rts_threshold;
        pr_debug("vm_rtsthreshold:%d\n", wnet_vif->vm_rtsthreshold);
     }

    if (changed & WIPHY_PARAM_FRAG_THRESHOLD &&
        wnet_vif->vm_fragthreshold != wiphy->frag_threshold) {
        wnet_vif->vm_fragthreshold = wiphy->frag_threshold;
        pr_debug("vm_fragthreshold: %d\n", wnet_vif->vm_fragthreshold);
    }

    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    return 0;
}

//for update channel params
static int
vm_cfg80211_update_wiphy_params(struct wiphy *wiphy)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct cfg80211_chan_def *chandef;
    struct wireless_dev *vm_wdev;

    vm_wdev = wnet_vif->vm_wdev;
    pr_debug("vm_wdev:0x%p, %s, %d\n", vm_wdev, __func__, __LINE__);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
    chandef = &vm_wdev->preset_chandef;
#else
    chandef = &vm_wdev->u.ap.preset_chandef;
#endif
    if (chandef->chan == NULL) {
        ERROR_DEBUG_OUT("chandef->chan NULL\n");
        return -1;
    } else {
        //TODO
    }

    //chan_dbg(wnet_vif->vm_wmac->wm_curchan, "x2", 2084);

    return 0;
}

static int
vm_cfg80211_join_ibss(struct wiphy *wiphy,
    struct net_device *dev,
    struct cfg80211_ibss_params *params)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct ieee80211_channel *chan;


    DPRINTF(AML_DEBUG_CFG80211, "<%s>:<%s>  %s++ %d\n",
            wnet_vif->vm_ndev->name,dev->name, __func__, __LINE__);

    if (params->bssid)
    {
        ERROR_DEBUG_OUT("Invalid bssid\n");
        return -EOPNOTSUPP;
    }
    chan = params->chandef.chan;


    if (chan && chan->center_freq)
    {
        int bw = 0, center_chan = 0, pri_chan = 0;

        pri_chan = ieee80211_frequency_to_channel(chan->center_freq);
        switch(params->chandef.width)
        {
            case NL80211_CHAN_WIDTH_20_NOHT:
            case NL80211_CHAN_WIDTH_20:
                bw = WIFINET_BWC_WIDTH20;
                center_chan = pri_chan;
                break;
            case NL80211_CHAN_WIDTH_40:
                bw = WIFINET_BWC_WIDTH40;
                center_chan = ieee80211_frequency_to_channel(params->chandef.center_freq1);
                break;
            case NL80211_CHAN_WIDTH_80:
                bw = WIFINET_BWC_WIDTH80;
                center_chan = ieee80211_frequency_to_channel(params->chandef.center_freq1);
                break;
            case NL80211_CHAN_WIDTH_80P80:
            case NL80211_CHAN_WIDTH_160:
                pr_warn("%s:%d, not support bandwidth %d yet \n",
                __func__, __LINE__, params->chandef.width);
                bw = WIFINET_BWC_WIDTH20;
                center_chan = pri_chan;
                break;
            default:
                bw = WIFINET_BWC_WIDTH20;
                center_chan = pri_chan;
                break;
        }
    }

    if (params->ssid_len > WIFINET_NWID_LEN)
        params->ssid_len = WIFINET_NWID_LEN;

    memcpy(wnet_vif->vm_des_ssid[0].ssid,(void *)params->ssid,params->ssid_len);
    wnet_vif->vm_des_ssid[0].len = params->ssid_len;
    wnet_vif->vm_des_nssid = 1;
    wnet_vif->vm_flags &= ~WIFINET_F_DESBSSID;

    if (params->ssid_len > 0 &&
        wnet_vif->vm_des_ssid[0].ssid[params->ssid_len-1] == '\0')
        wnet_vif->vm_des_ssid[0].len--;
    return IS_RUNNING(dev) ? -wifi_mac_initial(wnet_vif->vm_ndev, RESCAN) : 0;
}

static int
vm_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    DPRINTF(AML_DEBUG_CFG80211, "<%s>:<%s>  %s++ %d\n",
            wnet_vif->vm_ndev->name,dev->name, __func__, __LINE__);

    wifi_mac_top_sm(wnet_vif, WIFINET_S_INIT,0);

    DPRINTF(AML_DEBUG_CFG80211, "%s-- %d\n", __func__, __LINE__);
    return 0;
}


static int
vm_cfg80211_set_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev, int radio_idx, enum nl80211_tx_power_setting type, int dbm)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    pr_debug("---tx_power_setting: %d, dbm: %d\n", type, dbm);

    wifimac->drv_priv->drv_ops.drv_set_txPwrLimit(wifimac->drv_priv, type, dbm);

    return 0;
}

static int
vm_cfg80211_get_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev, int radio_idx, unsigned int link_id, int *dbm)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    pr_debug("---drv_curtxpower:%d\n", *dbm);
    *dbm = wifimac->drv_priv->drv_curtxpower;

    return 0;
}

static int get_keyix(int kid, int vm_def_txkey)
{
    int key_idx = kid;

    if (key_idx >= WIFINET_WEP_NKID) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d key_idx=%d, may use vm_def_txkey=%d\n",
            __func__, __LINE__, key_idx, vm_def_txkey);

        if (vm_def_txkey == WIFINET_KEYIX_NONE) {
            key_idx = 0;

        } else {
            key_idx = vm_def_txkey;
        }
    }
    return key_idx;
}

static int
vm_set_wep_key (struct wlan_net_vif *wnet_vif,
    unsigned char ekey_len, unsigned char ekey_idx,
    const unsigned char *key)
{
    unsigned char key_len = ekey_len;
    unsigned char key_idx = ekey_idx;
    struct wifi_mac_key *k = NULL;
    int ret = 0;
    if (key_len > WIFINET_KEYBUF_SIZE)
    {
        ret = -EINVAL;
        ERROR_DEBUG_OUT("key_len=%d err\n", key_len);
        goto exit;
    }
    key_idx = get_keyix(key_idx, wnet_vif->vm_def_txkey);
    wifi_mac_KeyUpdateBegin(wnet_vif);
    k = &wnet_vif->vm_nw_keys[key_idx];
    k->wk_keyix = key_idx;
    if (wifi_mac_security_req(wnet_vif, WIFINET_CIPHER_WEP,
                             WIFINET_KEY_XMIT | WIFINET_KEY_RECV | WIFINET_KEY_GROUP, k))
    {
        k->wk_keylen = key_len;
        memcpy(k->wk_key, key, key_len);
        memset(k->wk_key + key_len, 0, WIFINET_KEYBUF_SIZE - key_len);
        //if (aml_debug & AML_DEBUG_CFG80211) {
        //        dump_memory_internal(k->wk_key, WIFINET_KEYBUF_SIZE+16);
        //}
        if (!wifi_mac_security_setkey(wnet_vif, k, wnet_vif->vm_myaddr, NULL))
        {
            ret = -EINVAL;
            ERROR_DEBUG_OUT("wifi_mac_security_setkey err\n");
            goto exit;
        }
    }
    else
    {
        ret = -EINVAL;
        ERROR_DEBUG_OUT("wifi_mac_security_req err\n");
        goto exit;
    }
    wnet_vif->vm_def_txkey = key_idx;
    wifi_mac_KeyUpdateEnd(wnet_vif);
exit:
    return ret;
}

int softap_get_sta_num(struct wlan_net_vif *wnet_vif)
{
    struct wifi_station *sta_next = NULL;
    struct wifi_station *sta = NULL;
    int sta_num = 0;

    struct wifi_station_tbl *nt = &wnet_vif->vm_sta_tbl;
    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        sta_num += 1;
    }
    WIFINET_NODE_UNLOCK(nt);
    return sta_num -1;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static  int vm_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev, unsigned char key_index,
    bool pairwise, const unsigned char *mac_addr, struct key_params *params)
#else
static  int vm_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev, int link_id,
        unsigned char key_index, bool pairwise, const unsigned char *mac_addr, struct key_params *params)
#endif
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    const struct key_params *lparams = params;
    const unsigned char *lmac_addr = mac_addr;
    int kid = key_index;
    int ret = 0;
    struct wifi_macreq_key kr;
    struct wifi_mac_key *wk;
    struct wifi_station *sta;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int total_delay = 0;
    struct drv_private *drv_priv = wifimac->drv_priv;

    DPRINTF(AML_DEBUG_CFG80211, "%s adding key for <%s> vm_state=%d, cipher=0x%x key_len=%d seq_len=%d kid=%d, pairwise :%d\n",
        __func__, dev->name, wnet_vif->vm_state, lparams->cipher, lparams->key_len, lparams->seq_len, kid, pairwise);

    DPRINTF(AML_DEBUG_CFG80211, "key is %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:\n",
        lparams->key[0], lparams->key[1], lparams->key[2], lparams->key[3], lparams->key[4], lparams->key[5], lparams->key[6],
        lparams->key[7], lparams->key[8], lparams->key[8], lparams->key[10], lparams->key[11], lparams->key[12], lparams->key[13],
        lparams->key[14], lparams->key[15]);

    if (mac_addr != NULL) {
        DPRINTF(AML_DEBUG_CFG80211, "mac_addr:%02x:%02x:%02x:%02x:%02x:%02x\n",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    }

    if ((lparams->cipher == WLAN_CIPHER_SUITE_WEP40) || (lparams->cipher == WLAN_CIPHER_SUITE_WEP104)) {
        DPRINTF(AML_DEBUG_KEY, "%s %d lmac_addr=%p, cipher=0x%x,should be wep,vm_opmode=%d\n",
            __func__, __LINE__, lmac_addr, lparams->cipher, wnet_vif->vm_opmode);

        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            DPRINTF(AML_DEBUG_KEY, "%s %d wnet_vif->vm_opmode=%d set wep key\n",
                __func__, __LINE__, wnet_vif->vm_opmode);
            ret = vm_set_wep_key(wnet_vif, lparams->key_len, kid, lparams->key);

        } else {
            DPRINTF(AML_DEBUG_KEY, "%s %d wnet_vif->vm_opmode=%d set wep key\n",
                __func__, __LINE__, wnet_vif->vm_opmode);
            ret = vm_set_wep_key(wnet_vif, lparams->key_len, kid, lparams->key);
        }
        goto exit;
    }

    memset(&kr, 0, sizeof(kr));
    kr.ik_type = get_cipher_rsn(lparams->cipher);
    kr.ik_keyix = kid;

    if (lparams->key_len > sizeof(kr.ik_keydata)) {
        ret = -E2BIG;
        DPRINTF(AML_DEBUG_CFG80211, "%s %d lparams->key_len=%d > sizeof(kr.ik_keydata)=%zd\n",
            __func__, __LINE__, lparams->key_len, sizeof(kr.ik_keydata));
        goto exit;
    }

    memcpy(kr.ik_keydata, lparams->key, lparams->key_len);
    kr.ik_keylen = lparams->key_len;
    kr.ik_flags = WIFINET_KEY_RECV|WIFINET_KEY_XMIT;

    if (!pairwise) {
        kr.ik_flags |= WIFINET_KEY_GROUP ;

    } else {
        memcpy(kr.ik_macaddr, lmac_addr, WIFINET_ADDR_LEN);
    }

    if (lparams->seq_len && lparams->seq) {
        memcpy(&(kr.ik_keyrsc), lparams->seq, lparams->seq_len);
    }

    if (!(kr.ik_flags & WIFINET_KEY_GROUP)) {
        DPRINTF(AML_DEBUG_KEY, "%s %d wnet_vif->vm_opmode=%d\n", __func__, __LINE__, wnet_vif->vm_opmode);

        if (wnet_vif->vm_opmode == WIFINET_M_STA) {
            sta = wnet_vif->vm_mainsta;
            if (!WIFINET_ADDR_EQ(kr.ik_macaddr, sta->sta_bssid)) {
                DPRINTF(AML_DEBUG_CFG80211, "%s %d  ERR mode=sta, mac not match\n", __func__, __LINE__);
                ret = -ENOENT;
                goto exit;
            }

        } else {
            sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, kr.ik_macaddr,wnet_vif->wnet_vif_id);

            if (sta == NULL) {
                DPRINTF(AML_DEBUG_CFG80211, "%s %d  ERR mode=ap/ibss, sta not found\n", __func__, __LINE__);
                ret = -ENOENT;
                goto exit;
            }
        }
        wk = &sta->sta_ucastkey;

    } else {
        if (kr.ik_type == WIFINET_CIPHER_AES_CMAC) {
            kr.ik_keyix = 0;
            if (wnet_vif->vm_opmode == WIFINET_M_STA) {
                wnet_vif->vm_def_mgmt_txkey = key_index;
                wk = &wnet_vif->vm_mainsta->pmf_key;

            } else {
                sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, kr.ik_macaddr,wnet_vif->wnet_vif_id);

                if (sta == NULL) {
                    sta = wnet_vif->vm_mainsta;
                }
                wk = &sta->pmf_key;
            }

        } else {
            wk = &wnet_vif->vm_nw_keys[kr.ik_keyix];
            kr.ik_flags |= WIFINET_KEY_DEFAULT;
        }
        sta = NULL;
    }
    DPRINTF(AML_DEBUG_KEY, "%s kr.ik_flags=0x%x kr.ik_keyix=%d\n", __func__, kr.ik_flags, kr.ik_keyix);

    if (sta != NULL) {
        DPRINTF(AML_DEBUG_CONNECT, "%s %d 4-way handshake completed\n", __func__, __LINE__);
        sta->connect_status = CONNECT_FOUR_WAY_HANDSHAKE_COMPLETED;
        if (wifimac->recovery_stat == WIFINET_RECOVERY_VIF_UP) {
            sta->connect_status = CONNECT_DHCP_GET_ACK;
            wifimac->recovery_stat = WIFINET_RECOVERY_END;
            AML_OUTPUT("!!!!!!!!! fw recovery end !!!!!!!!!\n");
        }
    }

    wifi_mac_KeyUpdateBegin(wnet_vif);
    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
        wifi_softap_allsta_stopping(wnet_vif, 1);
        while (total_delay < 1000 && (!drv_priv->hal_priv->hal_ops.hal_tx_empty())) {
        msleep(10);
        total_delay += 10;
        }
    }
    if (wifi_mac_security_req(wnet_vif, kr.ik_type, (kr.ik_flags & WIFINET_KEY_COMMON), wk)) {
        int i;
        wk->wk_keylen = kr.ik_keylen;

        if (wk->wk_keylen > WIFINET_KEYBUF_SIZE)
            wk->wk_keylen = WIFINET_KEYBUF_SIZE;

        for (i = 0; i < WIFINET_TID_SIZE; ++i)
            wk->wk_keyrsc[i] = kr.ik_keyrsc[i];

        wk->wk_keytsc = 0;
        memset(wk->wk_key, 0, sizeof(wk->wk_key));
        memcpy(wk->wk_key, kr.ik_keydata, kr.ik_keylen);

        wk->wk_keyix = kr.ik_keyix;
        wk->wk_key_type = kr.ik_type;

        if (kr.ik_type == WIFINET_CIPHER_AES_CMAC) {
            wk->wk_valid = 1;
            wk->wk_keyix = key_index;
            pr_debug("not set pmf key to fw\n");
            goto new_key_exit;
        }

        DPRINTF(AML_DEBUG_KEY, "%s %d sta=%p\n", __func__, __LINE__, sta);
        if (wifi_mac_security_setkey(wnet_vif, wk, sta != NULL ? sta->sta_macaddr : kr.ik_macaddr, sta)) {
            if (kr.ik_flags & WIFINET_KEY_DEFAULT) {
                wnet_vif->vm_def_txkey = kr.ik_keyix;
                DPRINTF(AML_DEBUG_CFG80211, "%s set vm_def_txkey=%d\n", __func__, wnet_vif->vm_def_txkey);
            }

        } else {
            DPRINTF(AML_DEBUG_CFG80211, "%s %d wifi_mac_security_setkey err\n", __func__, __LINE__);
            ret = -ENXIO;
            goto new_key_exit;
        }
    } else {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d wifi_mac_security_req err\n", __func__, __LINE__);
        ret = -ENXIO;
        goto new_key_exit;
    }
new_key_exit:
    wifi_mac_KeyUpdateEnd(wnet_vif);
    if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
        wifi_softap_allsta_stopping(wnet_vif, 0);

exit:
    DPRINTF(AML_DEBUG_CFG80211, "%s(%d): %d\n", __func__, __LINE__, ret);
    return ret;
}

static int 
vm_cfg80211_set_rekey_data(struct wiphy *wiphy,
    struct net_device *dev,struct cfg80211_gtk_rekey_data *data)
{
    struct wifi_mac_rekey_data rekey_data;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);

    if (wnet_vif->vm_opmode == WIFINET_M_STA)
    {
        memset((void *)(&rekey_data), 0, 40);
        memcpy(rekey_data.kek, data->kek, 16);
        memcpy(rekey_data.kck, data->kck, 16);
        memcpy(rekey_data.replay_counter, data->replay_ctr, 8);

        wifi_mac_security_set_rekey_data(wnet_vif, rekey_data);
        return 0;
    }

    return -1;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static int vm_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
    unsigned char key_index, bool pairwise, const unsigned char *mac_addr)
#else
static int vm_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev, int link_id,
    unsigned char key_index, bool pairwise, const unsigned char *mac_addr)
#endif
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_station *sta = NULL;
    int total_delay = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private *drv_priv = wifimac->drv_priv;

    DPRINTF(AML_DEBUG_CFG80211, "%s vm_flags:%08x, kid=%d, pairwise:%d\n", __func__, wnet_vif->vm_flags, key_index, pairwise);
    if (mac_addr != NULL) {
        DPRINTF(AML_DEBUG_CFG80211, "mac_addr:%02x:%02x:%02x:%02x:%02x:%02x\n",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, mac_addr,wnet_vif->wnet_vif_id);

    } else {
        if (wnet_vif->vm_opmode == WIFINET_M_STA
            || wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT) {
            sta = wnet_vif->vm_mainsta;
        }
    }

    if (sta == NULL) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d  WARN\n", __func__, __LINE__);
        goto exit;
    }

    while (total_delay < 1000 && ((sta->connect_status != CONNECT_IDLE)
        || (!drv_priv->hal_priv->hal_ops.hal_tx_empty()))) {
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP && sta->is_disconnecting == 0)
            break;
        msleep(10);
        total_delay += 10;
    }

    if (key_index < WIFINET_WEP_NKID) {
        if (key_index == sta->sta_ucastkey.wk_keyix) {
            DPRINTF(AML_DEBUG_CFG80211, "%s %d del SINGLE KEY\n", __func__, __LINE__);
            wifi_mac_sec_delt_key(wnet_vif, &sta->sta_ucastkey, sta);

        } else {
            if (wnet_vif->vm_nw_keys[key_index].wk_keyix == key_index) {
                DPRINTF(AML_DEBUG_CFG80211, "%s %d del GROUP KEY\n", __func__, __LINE__);
                wifi_mac_sec_delt_key(wnet_vif, &wnet_vif->vm_nw_keys[key_index], sta);
            }
        }

    } else {
        if (key_index == wnet_vif->vm_def_mgmt_txkey) {
            DPRINTF(AML_DEBUG_CFG80211, "%s %d del MGMT GROUP KEY\n", __func__, __LINE__);
            memset(&sta->pmf_key, 0, sizeof(struct wifi_mac_key));
            wifi_mac_security_resetkey(wnet_vif, &sta->pmf_key, WIFINET_KEYIX_NONE);
            wnet_vif->vm_def_mgmt_txkey = WIFINET_KEYIX_NONE;
        }
    }

exit:
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static int
vm_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
    unsigned char key_index, bool pairwise, const unsigned char *mac_addr,
    void *cookie,void (*callback)(void *cookie,struct key_params*))
#else
static int
vm_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev, int link_id,
    unsigned char key_index, bool pairwise, const unsigned char *mac_addr,
    void *cookie,void (*callback)(void *cookie,struct key_params*))
#endif
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, dev->name);
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static int vm_cfg80211_config_default_key(struct wiphy *wiphy,
    struct net_device *dev,unsigned char key_index, bool unicast, bool multicast)
#else
static int vm_cfg80211_config_default_key(struct wiphy *wiphy, struct net_device *dev,
        int link_id, unsigned char key_index, bool unicast, bool multicast)
#endif
{
    DPRINTF(AML_DEBUG_CFG80211, "%s<%s>, index:%d, uni:%d, mul:%d\n", __func__, dev->name, key_index, unicast, multicast);
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static int vm_cfg80211_set_default_mgmt_key(struct wiphy *wiphy,
    struct net_device *dev, unsigned char key_idx)
#else
static int vm_cfg80211_set_default_mgmt_key(struct wiphy *wiphy,
    struct net_device *dev, int link_id, unsigned char key_idx)
#endif
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);

    DPRINTF(AML_DEBUG_CFG80211, "%s<%s>, index:%d\n", __func__, dev->name, key_idx);
    wnet_vif->vm_def_mgmt_txkey = key_idx;
    return 0;
}


static int
vm_cfg80211_set_power_mgmt(struct wiphy *wiphy,
    struct net_device *dev,bool enabled, int timeout)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);

    if (aml_wifi_is_enable_rf_test())
        return 0;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> enabled=%d timeout=%d\n",
        __func__, __LINE__, dev->name, enabled, timeout);

    if(enabled)
    {
        wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_NORMAL);
        if (wnet_vif->vm_wdev)
            wnet_vif->vm_wdev->ps = 1;
    }
    else
    {
        wifi_mac_pwrsave_set_mode(wnet_vif, WIFINET_PWRSAVE_NONE);
        if (wnet_vif->vm_wdev)
            wnet_vif->vm_wdev->ps = 0;
    }

    return 0;
}

static int vm_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
    struct cfg80211_connect_params *sme)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct cfg80211_connect_params *lsme = sme;

    int ret = 0;
    unsigned char *rsn_ie = NULL;
    unsigned int rsn_ie_len = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_scan_state *ss = wifimac->wm_scan;
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *p2p_vmac = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> privacy=%d, key=%p, key_len=%d, key_idx=%d\n",
        __func__, __LINE__, dev->name, lsme->privacy, lsme->key, lsme->key_len, lsme->key_idx);

    if ((wnet_vif->vm_opmode != WIFINET_M_STA) || (wnet_vif->vm_mainsta == NULL)) {
        ret = -EINVAL;
        ERROR_DEBUG_OUT("mode=%d not support connect\n", wnet_vif->vm_opmode);
        goto exit ;

    } else {
        DPRINTF(AML_DEBUG_ERROR,  "%s %d vm opmode (1:Station,2: ap) :%d\n", __func__, __LINE__, wnet_vif->vm_opmode);
    }

    if (pwdev_priv->block) {
        ret = -EBUSY;
        ERROR_DEBUG_OUT("wdev_priv.block is set\n");
        goto exit;
    }

    if (!lsme->ssid || !lsme->ssid_len) {
        ERROR_DEBUG_OUT("ssid is none\n");
        ret = -EINVAL;
        goto exit;
    }

    if (lsme->ssid_len > IW_ESSID_MAX_SIZE) {
        ERROR_DEBUG_OUT("ssid_len=%zd too long\n",lsme->ssid_len);
        ret= -E2BIG;
        goto exit;
    }

    if ((wifimac->wm_flags & WIFINET_F_SCAN) && (ss->scan_CfgFlags & WIFINET_SCANCFG_NOPICK)) {
        if (preempt_scan(dev, 100, 100)) {
            ret= -E2BIG;
            goto exit;
        }
    }

    if ((wifimac->wm_nrunning == 1) && (wifimac->is_miracast_connect == 1)
        && (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED)) {
        ERROR_DEBUG_OUT("rejected wlan0 connect due to is_miracast_connect\n");
        return ret;
    }

    if (pwdev_priv->connect_request) {
        DPRINTF(AML_DEBUG_WARNING, "%s %d multiple connect req, should not happen\n", __func__, __LINE__);
    }
    DPRINTF(AML_DEBUG_CFG80211, "%s ssid=%s, len=%zd\n", __func__, ssid_sprintf(lsme->ssid, lsme->ssid_len), lsme->ssid_len);

    memset(wnet_vif->vm_des_ssid, 0, IV_SSID_SCAN_AMOUNT * sizeof(struct wifi_mac_ScanSSID));
    memset(wnet_vif->vm_des_bssid, 0, WIFINET_ADDR_LEN);
    wnet_vif->vm_des_ssid[0].len= lsme->ssid_len;
    memcpy(wnet_vif->vm_des_ssid[0].ssid, lsme->ssid, lsme->ssid_len);
    wnet_vif->vm_des_nssid = 1;
    wnet_vif->vm_flags &= ~WIFINET_F_IGNORE_SSID;
    wnet_vif->vm_flags &= ~WIFINET_F_DESBSSID;

    wifimac->wm_lastroaming = jiffies;
    wnet_vif->vm_chan_roaming_scan_flag = 0;
    g_auto_gain_base = 0;

    /*if change AP,change roaming_ssid and clean roaming candidate channel*/
    if(!wnet_vif->vm_wmac->wm_scan->roaming_ssid.len
        || memcmp(wnet_vif->vm_wmac->wm_scan->roaming_ssid.ssid, lsme->ssid, wnet_vif->vm_wmac->wm_scan->roaming_ssid.len) != 0) {
        pr_debug("clean roaming candidate channels \n");
        wnet_vif->vm_wmac->wm_scan->roaming_ssid.len = lsme->ssid_len;
        memcpy(wnet_vif->vm_wmac->wm_scan->roaming_ssid.ssid, lsme->ssid, lsme->ssid_len);

        WIFI_ROAMING_CHANNLE_LOCK(wnet_vif->vm_wmac->wm_scan);
        wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans_cnt = 0;
        memset(wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans, 0, sizeof(wnet_vif->vm_wmac->wm_scan->roaming_candidate_chans));
        WIFI_ROAMING_CHANNLE_UNLOCK(wnet_vif->vm_wmac->wm_scan);
    }
    if (lsme->bssid) {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d bssid=%s\n", __func__, __LINE__, ether_sprintf(lsme->bssid));
        WIFINET_ADDR_COPY(wnet_vif->vm_des_bssid, lsme->bssid);
        wnet_vif->vm_flags |= WIFINET_F_DESBSSID;
    }
    if (lsme->privacy) {
        wnet_vif->vm_flags |= WIFINET_F_PRIVACY;
        DPRINTF(AML_DEBUG_CFG80211, "%s %d  wnet_vif->vm_flags=0x%x\n", __func__, __LINE__, wnet_vif->vm_flags);

    } else {
        wnet_vif->vm_flags &= ~WIFINET_F_PRIVACY;
    }

    {
        unsigned char sta_authmode = WIFINET_AUTH_OPEN;
        switch (lsme->auth_type) {
            case NL80211_AUTHTYPE_OPEN_SYSTEM:
                sta_authmode = WIFINET_AUTH_OPEN;
                break;

            case NL80211_AUTHTYPE_AUTOMATIC:
                if (lsme->crypto.n_ciphers_pairwise && lsme->key_len)
                    sta_authmode = WIFINET_AUTH_SHARED;
                else
                    sta_authmode = WIFINET_AUTH_OPEN;
                break;

            case NL80211_AUTHTYPE_SHARED_KEY:
                sta_authmode = WIFINET_AUTH_SHARED;
                break;

#ifdef AML_WPA3
            case NL80211_AUTHTYPE_SAE:
                if (aml_pmkid_cache_index(wnet_vif, lsme->bssid) == -1) {
                    sta_authmode = WIFINET_AUTH_SAE;

                } else {
                    sta_authmode = WIFINET_AUTH_OPEN;
                }
                break;
#endif

            default:
                ret = -ENOTSUPP;
                DPRINTF(AML_DEBUG_ERROR, "%s %d unsupported auth_type=%d\n", __func__, __LINE__, lsme->auth_type);
                goto exit;
        }

        wnet_vif->vm_mainsta->sta_authmode = sta_authmode;
    }
    pr_debug("%s %d lsme->auth_type=%d, sta_authmode=%d\n", __func__, __LINE__, lsme->auth_type, wnet_vif->vm_mainsta->sta_authmode);

    if (lsme->crypto.wpa_versions) {
        unsigned int wpa_versions = lsme->crypto.wpa_versions;
        DPRINTF(AML_DEBUG_CFG80211, "%s %d wpa_versions=0x%02x\n", __func__, __LINE__, wpa_versions);

        wnet_vif->vm_flags &= ~WIFINET_F_WPA;
        if ((wpa_versions & NL80211_WPA_VERSION_1) && (wpa_versions & NL80211_WPA_VERSION_2)) {
            wnet_vif->vm_flags |= WIFINET_F_WPA;

        } else if (wpa_versions & NL80211_WPA_VERSION_1) {
            wnet_vif->vm_flags |= WIFINET_F_WPA1;

        } else if (wpa_versions & NL80211_WPA_VERSION_2) {
            wnet_vif->vm_flags |= WIFINET_F_WPA2;

        } else {
            DPRINTF(AML_DEBUG_WARNING, "%s %d unsupported wpa_versions=0x%x\n", __func__, __LINE__, wpa_versions);
        }

    } else {
        wnet_vif->vm_flags &= ~WIFINET_F_WPA;
    }

    if (lsme->crypto.n_ciphers_pairwise) {
        int cipher = -1;

        DPRINTF(AML_DEBUG_CFG80211, "%s %d ciphers_pairwise[0]=0x%x\n", __func__, __LINE__, lsme->crypto.ciphers_pairwise[0]);

        cipher = get_cipher_rsn(lsme->crypto.ciphers_pairwise[0]);
        if (cipher < 0) {
            ret = cipher;
            ERROR_DEBUG_OUT("unsupported ciphers_pairwise=0x%x\n", lsme->crypto.ciphers_pairwise[0]);
            goto exit;
        }

        if (((wnet_vif->vm_caps & cipher2cap(cipher)) == 0) && !wifi_mac_security_available(cipher)) {
            ret = -EINVAL;
            ERROR_DEBUG_OUT("wnet_vif->vm_caps=0x%x\n", wnet_vif->vm_caps);
            goto exit;
        }

        wnet_vif->vm_mainsta->sta_rsn.rsn_ucastcipher = cipher;
    }

    if (lsme->crypto.cipher_group) {
        int cipher = -1;

        DPRINTF(AML_DEBUG_CFG80211, "%s %d cipher_group=0x%x\n", __func__, __LINE__, lsme->crypto.cipher_group);

        cipher = get_cipher_rsn(lsme->crypto.cipher_group);
        if (cipher < 0) {
            ret = cipher;
            ERROR_DEBUG_OUT("unsupported cipher_group=0x%x\n", lsme->crypto.cipher_group);
            goto exit;
        }

        if (((wnet_vif->vm_caps & cipher2cap(cipher)) == 0) && !wifi_mac_security_available(cipher)) {
            ret = -EINVAL;
            ERROR_DEBUG_OUT("wnet_vif->vm_caps=0x%x\n", wnet_vif->vm_caps);
            goto exit;
        }

        wnet_vif->vm_mainsta->sta_rsn.rsn_mcastcipher = cipher;
    }

    if (lsme->crypto.n_akm_suites) {
        int key_mgt = -1;

        DPRINTF(AML_DEBUG_CFG80211, "%s %d akm_suites[0]=0x%x\n", __func__, __LINE__, lsme->crypto.akm_suites[0]);
        if (lsme->crypto.akm_suites[0] == WLAN_AKM_SUITE_PSK) {
            key_mgt = WPA_ASE_8021X_PSK;

        } else if (lsme->crypto.akm_suites[0] == WLAN_AKM_SUITE_SAE) {
            key_mgt = RSN_ASE_8021X_SAE;

        } else if (lsme->crypto.akm_suites[0] == WLAN_AKM_SUITE_PSK_SHA256) {
            key_mgt = RSN_ASE_8021X_PSK_SHA256;

        } else if (lsme->crypto.akm_suites[0] == WLAN_AKM_SUITE_8021X) {
            key_mgt = RSN_ASE_8021X_UNSPEC;

        } else if (lsme->crypto.akm_suites[0] == WLAN_AKM_SUITE_8021X_SHA256) {
            key_mgt = RSN_ASE_8021X_EAP_SHA256;

        } else {
            ret = -ENOTSUPP;
            ERROR_DEBUG_OUT("unsupported akm_suites[0]=0x%x\n", lsme->crypto.akm_suites[0]);
            goto exit;
        }

        wnet_vif->vm_mainsta->sta_rsn.rsn_keymgmtset = (1 << key_mgt);
    }

#ifdef AML_WPA3
    if (lsme->mfp == NL80211_MFP_REQUIRED) {
        wnet_vif->vm_mainsta->sta_flags_ext |= WIFINET_NODE_MFP;
        wnet_vif->vm_mainsta->pmf_key.wk_keylen = 0;
        DPRINTF(AML_DEBUG_CFG80211, "%s NL80211_MFP_REQUIRED\n", __func__);

    } else {
        DPRINTF(AML_DEBUG_CFG80211, "%s NL80211_MFP_NO\n", __func__);
    }
#endif

    DPRINTF(AML_DEBUG_CFG80211, "%s %d lsme->ie_len=%zd\n", __func__, __LINE__, lsme->ie_len);
    if (lsme->ie && lsme->ie_len) {
        if (lsme->ie_len > WIFINET_MAX_IV_OPT_IE) {
            ret = -ENOTSUPP;
            ERROR_DEBUG_OUT("ie_len too long\n");
            goto exit;
        }
        memset(wnet_vif->vm_opt_ie, 0, WIFINET_MAX_IV_OPT_IE);
        wnet_vif->vm_opt_ie_len = 0;

        rsn_ie = vm_get_ie(lsme->ie, lsme->ie_len, NULL, (unsigned int *)&rsn_ie_len, WIFINET_ELEMID_RSN, 0);
        memcpy(wnet_vif->vm_opt_ie, lsme->ie, lsme->ie_len);
        wnet_vif->vm_opt_ie_len = lsme->ie_len;
        if ((wnet_vif->vm_mainsta->sta_flags_ext & WIFINET_NODE_MFP) && rsn_ie) {
            if (wifi_mac_parse_own_rsn(wnet_vif->vm_mainsta, rsn_ie)) {
                goto exit;
            }
        }

#ifdef CONFIG_P2P
        vm_p2p_set_wpsp2pie(dev, (unsigned char *)lsme->ie ,lsme->ie_len,P2P_ASSOC_REQ_IE);
#endif

    } else {
        wnet_vif->vm_opt_ie_len = 0;
    }

    if (!(lsme->crypto.wpa_versions & (NL80211_WPA_VERSION_1 | NL80211_WPA_VERSION_2)) &&  lsme->key && lsme->key_len
        && ((lsme->auth_type == NL80211_AUTHTYPE_SHARED_KEY) || (lsme->auth_type == NL80211_AUTHTYPE_AUTOMATIC)
        || (lsme->crypto.n_ciphers_pairwise & (WLAN_CIPHER_SUITE_WEP40 | WLAN_CIPHER_SUITE_WEP104)))) {
        ret = vm_set_wep_key(wnet_vif, lsme->key_len, lsme->key_idx, lsme->key);
        if (ret < 0) {
            ERROR_DEBUG_OUT("ret=%d err\n", ret);
            goto exit;
        }
    }

    DPRINTF(AML_DEBUG_CFG80211, "%s %d connect_timeout=%d ms\n", __func__, __LINE__, CFG80211_CONNECT_TIMER_OUT);

    if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())
        && concurrent_check_vmac_is_AP(wifimac)
        && (wnet_vif->vm_opmode == WIFINET_M_STA)
        && (p2p_vmac->vm_state == WIFINET_S_CONNECTED)) {
        unsigned int delay_ms = 0;
        struct wifi_channel *switch_chan = NULL;

        switch_chan = wifi_mac_connect_get_target_chan(wifimac->wm_scan, wnet_vif);
        channel_switch_announce_trigger(wifimac, switch_chan);
        while ((wifimac->wm_flags & WIFINET_F_DOTH) && (wifimac->wm_flags & WIFINET_F_CHANSWITCH) && (delay_ms < 1000)) {
            msleep(50);
            delay_ms += 50;
        }
        DPRINTF(AML_DEBUG_CONNECT, "delay_ms:%d\n",delay_ms);
    }

    if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);

    } else {
        wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
    }

    OS_SPIN_LOCK_IRQ(&pwdev_priv->connect_req_lock,pwdev_priv->connect_req_lock_flags);
    wnet_vif->vm_mainsta->connect_status = CONNECT_IDLE;
    pwdev_priv->connect_request = sme;
    os_timer_ex_start(&pwdev_priv->connect_timeout);
    OS_SPIN_UNLOCK_IRQ(&pwdev_priv->connect_req_lock,pwdev_priv->connect_req_lock_flags);

exit:
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    return ret;
}

static int
vm_cfg80211_disconnect(struct wiphy *wiphy,
    struct net_device *dev, unsigned short reason_code)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    int ret = 0;
    int mgmt_arg;
    int total_delay = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private *drv_priv = wifimac->drv_priv;
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> reason_code=%d\n",
            __func__, __LINE__, dev->name, reason_code);


    if (!IS_UP(dev))
    {
        ret = -EINVAL;
        ERROR_DEBUG_OUT("wifi_mac_security_req err\n");
        goto exit;
    }

    if (!(wnet_vif->vm_phase_flags & PHASE_DISCONNECTING) && wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        wnet_vif->vm_phase_flags |= PHASE_DISCONNECTING;

    } else {
        struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
        if (pwdev_priv->connect_request) {
            wnet_vif->vm_phase_flags |= PHASE_DISCONNECT_DELAY;
            AML_OUTPUT("current in connecting, disconnect later\n");
        }
        goto exit;
    }

    if (wnet_vif->vm_wmac->wm_flags & WIFINET_F_SCAN) {
        if (preempt_scan(wnet_vif->vm_ndev, 100, 100)) {
            ERROR_DEBUG_OUT("preempt_scan fail\n");
            return -EINVAL;
        }
        else {
            ERROR_DEBUG_OUT("preempt_scan %d\n", wnet_vif->wnet_vif_id);
        }
    }

    switch (wnet_vif->vm_opmode)
    {
        case WIFINET_M_STA:
            DPRINTF(AML_DEBUG_CFG80211, "%s %d sta mode\n", __func__, __LINE__);
            mgmt_arg = WIFINET_REASON_AUTH_LEAVE;
            wifi_mac_send_mgmt(wnet_vif->vm_mainsta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&mgmt_arg);
            if (wnet_vif->vm_p2p_support) {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
            }

            while (total_delay < 1000 && ((wifimac->wm_runningmask & BIT(wnet_vif->wnet_vif_id))
                || (!drv_priv->hal_priv->hal_ops.hal_tx_empty()))) {
                msleep(10);
                total_delay += 10;
            }
            break;
        case WIFINET_M_HOSTAP:
            DPRINTF(AML_DEBUG_CFG80211, "%s %d ap mode\n", __func__, __LINE__);
            break;
        default:
            ERROR_DEBUG_OUT("wnet_vif->vm_opmode=%d not support\n", wnet_vif->vm_opmode);
            ret = -EINVAL;
            goto exit;
    }

exit:
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
    return ret;
}

static int
vm_cfg80211_suspend(struct wiphy *wiphy, struct cfg80211_wowlan *wow)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int ret = 0;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);

    if (wow && (wow->n_patterns > WOW_MAX_PATTERNS))
        return -EINVAL;

    /* wnet_vif_id 1(p2p0) just follow wlan0 operations */
    if (wnet_vif->wnet_vif_id == 1)
        return 0;

    wifi_mac_pwrsave_wow_suspend((SYS_TYPE)wifimac, 0, (SYS_TYPE)wow, (SYS_TYPE)wnet_vif, 0);

    ret = (wifimac->wm_suspend_mode == WIFI_SUSPEND_STATE_WOW) ? 1 : 0;
    if (ret == 0) {
        ERROR_DEBUG_OUT("suspend failed\n");
        return -1;
    }

    DPRINTF(AML_DEBUG_CFG80211, "%s %d, end\n", __func__, __LINE__);
    return 0;
}

static int vm_cfg80211_resume(struct wiphy *wiphy)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int ret = 0;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);

    /* wnet_vif_id 1(p2p0) just follow wlan0 operations */
    if (wnet_vif->wnet_vif_id == 1)
        return 0;

    wifi_mac_pwrsave_wow_resume((SYS_TYPE)wifimac, 0, 0, (SYS_TYPE)wnet_vif, 0);

    ret = (wifimac->wm_suspend_mode == WIFI_SUSPEND_STATE_NONE) ? 1 : 0;
    if (ret == 0) {
        ERROR_DEBUG_OUT("resume failed\n");
        return -1;
    }

    DPRINTF(AML_DEBUG_CFG80211, "%s %d, end\n", __func__, __LINE__);
    return 0;
}

static unsigned int
aml_vm_legacy_24g_rate_map(unsigned int kernel_rate)
{
	switch(kernel_rate)
	{
		case 1:
			return 0x00;
		case 2:
			return 0x01;
		case 4:
			return 0x02;//actually 5.5
		case 8:
			return 0x03;
		case 16:
			return 0x04;
		case 32:
			return 0x05;
		case 64:
			return 0x06;
		case 128:
			return 0x07;
		case 256:
			return 0x08;
		case 512:
			return 0x09;
		case 1024:
			return 0x0a;
		case 2048:
			return 0x0b;
		default:
			ERROR_DEBUG_OUT("Legacy 2.4G Not supported rate \n");
			return 0;
	}
}

static unsigned int
aml_vm_legacy_5g_rate_map(unsigned int kernel_rate)
{
	switch(kernel_rate)
	{
		case 1:
			return 0x04;//6M;
		case 2:
			return 0x05;//9M;
		case 4:
			return 0x06;//12M;
		case 8:
			return 0x07;//18M;
		case 16:
			return 0x08;//24M;
		case 32:
			return 0x09;//36M;
		case 64:
			return 0x0a;//48M;
		case 128:
			return 0x0b;//54M;
		default:
			//pr_debug("Legacy 5G Warning: Not supported rate.\n");
			return 0;
	}
}

static unsigned int
aml_vm_ht_rate_map(unsigned int kernel_rate)
{
	switch(kernel_rate)
	{
		case 1:
			return 0x0080;
		case 2:
			return 0x0081;
		case 4:
			return 0x0082;
		case 8:
			return 0x0083;
		case 16:
			return 0x0084;
		case 32:
			return 0x0085;
		case 64:
			return 0x0086;
		case 128:
			return 0x0087;
		default:
			//pr_debug("HT Warning: Not supported rate: %d\n",kernel_rate);
			return 0;
	}
}

static unsigned int
aml_vm_vht_rate_map(unsigned int kernel_rate)
{
	switch(kernel_rate)
	{
		case 1:
			return 0x00c0;
		case 2:
			return 0x00c1;
		case 4:
			return 0x00c2;
		case 8:
			return 0x00c3;
		case 16:
			return 0x00c4;
		case 32:
			return 0x00c5;
		case 64:
			return 0x00c6;
		case 128:
			return 0x00c7;
		case 256:
			return 0x00c8;
		case 512:
			return 0x00c9;
		default:
			//pr_debug("VHT Warning: Not supported rate: %d\n",kernel_rate);
			return 0;
	}
}



static unsigned int
legacy_k2dot11_rate_map(unsigned int kernel_rate)
{

    //For 11b: (0x82 -0x80) * 500K =  1M
    //For 11g:  0xc * 500k = 6M
    unsigned int ret = 0;
    pr_debug("rate 0x%x %s(%d)\n", kernel_rate, __func__, __LINE__);
    switch (kernel_rate)
    {
        case BIT(0):
            ret =  0x02;// 1M
            break;
        case BIT(1):
            ret =  0x04;// 2M
            break;
        case BIT(2):
            ret =  0x0b;//5.5M
            break;
        case BIT(3):
            ret =  0x16;//11M
            break;
        case BIT(4):
            ret =  0x0c;//6M
            break;
        case BIT(5):
            ret =  0x12;//9M
            break;
        case BIT(6):
            ret =  0x18;//12M
            break;
        case BIT(7):
            ret =  0x24; //18M
            break;
        case BIT(8):
            ret =  0x30; //24M
            break;
        case BIT(9):
            ret =  0x48;// 36M
            break;
        case BIT(10):
            ret =  0x60;// 48M
            break;
        case BIT(11):
            ret =  0x6c;// 54M
            break;
        default:
            ERROR_DEBUG_OUT("Legacy 2.4G Not supported rate 0x%x\n", kernel_rate);
            ret =  0;
            break;
    }

    return ret ;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
int vm_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
    struct net_device *dev,
    const unsigned char *peer,
    const struct cfg80211_bitrate_mask *mask)
#else
int vm_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
    struct net_device *dev,
    unsigned int link_id,
    const unsigned char *peer,
    const struct cfg80211_bitrate_mask *mask)
#endif
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    unsigned int legacy_ratemask[AML_MAX_NUM_BANDS];
    unsigned int ht_ratemask[AML_MAX_NUM_BANDS];
    unsigned int vht_ratemask[AML_MAX_NUM_BANDS];
    unsigned int final_bit_rate = 0;
    int band;
    unsigned int protocol_rate = 0;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    memset(legacy_ratemask, 0, sizeof(legacy_ratemask));
    memset(ht_ratemask, 0, sizeof(ht_ratemask));
    memset(vht_ratemask, 0, sizeof(vht_ratemask));

    /* Legacy/HT/VHT rate setting */
	for (band = 0; band <= NL80211_BAND_5GHZ; band++)
	{
		if ( band == NL80211_BAND_2GHZ )
		{
			legacy_ratemask[band] = aml_vm_legacy_24g_rate_map(mask->control[band].legacy);
			protocol_rate = legacy_k2dot11_rate_map(mask->control[band].legacy);
			ht_ratemask[band] = aml_vm_ht_rate_map(mask->control[band].ht_mcs[0]);
			vht_ratemask[band] = aml_vm_vht_rate_map(mask->control[band].vht_mcs[0]);
		}
		if ( band == NL80211_BAND_5GHZ )
		{
			legacy_ratemask[band] = aml_vm_legacy_5g_rate_map(mask->control[band].legacy);
			ht_ratemask[band] = aml_vm_ht_rate_map(mask->control[band].ht_mcs[0]);
			vht_ratemask[band] = aml_vm_vht_rate_map(mask->control[band].vht_mcs[0]);
		}
	}
    /*For debug use only*/
    /*Get the bit rate*/
    if ( 1 == mask->control[0].legacy )
    {
        if ( legacy_ratemask[0] != 4095 && legacy_ratemask[0] != 255 )
		{
            final_bit_rate = protocol_rate;  // T.B.D
            if (aml_wifi_is_enable_rf_test())
                final_bit_rate = legacy_ratemask[0];

            goto SET_BITRATE;
		}
    }
    else
    {
        if( legacy_ratemask[0] != 0
            && legacy_ratemask[0] != 4095
            && legacy_ratemask[0] != 255 )
        {
            wifi_mac_config(wifimac, CHIP_PARAM_AMPDU, 0);
            //wifi_mac_config(wifimac, CHIP_PARAM_AMPDU_RX, 0);
            wifi_mac_config(wifimac, CHIP_PARAM_AMSDU_ENABLE, 0);

            final_bit_rate = protocol_rate;  // T.B.D
            if (aml_wifi_is_enable_rf_test())
                final_bit_rate = legacy_ratemask[0];

            goto SET_BITRATE;
        }
    }

    wifi_mac_config(wifimac, CHIP_PARAM_AMSDU_ENABLE, DEFAULT_TXAMSDU_EN);
    wifi_mac_config(wifimac, CHIP_PARAM_AMPDU, DEFAULT_TXAMPDU_EN);
    //wifi_mac_config(wifimac, CHIP_PARAM_AMPDU_RX, DEFAULT_RXAMPDU_EN);
    if ( legacy_ratemask[1] != 0
        && legacy_ratemask[1] != 4095
        && legacy_ratemask[1] != 255 )
    {
        final_bit_rate = protocol_rate; // T.B.D
        if (aml_wifi_is_enable_rf_test())
            final_bit_rate = legacy_ratemask[1];

        goto SET_BITRATE;
    }
    if ( ht_ratemask[0] != 0
        && ht_ratemask[0] != 4095
        && ht_ratemask[0] != 255 )
    {
        final_bit_rate = ht_ratemask[0];
        goto SET_BITRATE;
    }
    if ( ht_ratemask[1] != 0
        && ht_ratemask[1] != 4095
        && ht_ratemask[1] != 255 )
    {
        final_bit_rate = ht_ratemask[1];
        goto SET_BITRATE;
    }
    if ( vht_ratemask[0] != 0
        && vht_ratemask[0] != 4095
        && vht_ratemask[0] != 255 )
    {
        final_bit_rate = vht_ratemask[0];
        goto SET_BITRATE;
    }
    if ( vht_ratemask[1] != 0
        && vht_ratemask[1] != 4095
        && vht_ratemask[1] != 255 )
    {
        final_bit_rate = vht_ratemask[1];
        goto SET_BITRATE;
    }
SET_BITRATE:

    if (aml_wifi_is_enable_rf_test()) {
        gB2BTestCasePacket.data_rate = final_bit_rate;
        pr_debug("Cfg80211 B2B: %s %d, rate to be set:0x%04x\n",
                __func__,__LINE__,gB2BTestCasePacket.data_rate );
        return 0;
    }

    if (final_bit_rate != 0) {
        wnet_vif->vm_fixed_rate.rateinfo = final_bit_rate;
        wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_MCS;
        wnet_vif->vm_change_rate_enable = 0;
        pr_debug("Cfg80211: %s %d, rate to be set:0x%02x\n",
                __func__,__LINE__,wnet_vif->vm_fixed_rate.rateinfo );
    }

    return 0;
}

static int vm_cfg80211_change_bss(struct wiphy *wiphy,
    struct net_device *dev, struct bss_parameters *params)
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, dev->name);
    return 0;
}

static int _iv_cfg80211_add_set_beacon(struct wiphy *wiphy, struct net_device *dev,
    struct cfg80211_beacon_data *info, int dtim_period)
{
    struct cfg80211_beacon_data *linfo = info;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta = wnet_vif->vm_mainsta;
    int ret = 0;
    int set_beacon_flag = 0;
    unsigned char *buf = NULL;
    unsigned int buflen = linfo->head_len + linfo->tail_len;
    unsigned char *iebuf = NULL;
    unsigned short beacon_head_len = sizeof(struct wifi_frame);
    unsigned short ts_bintval_cap_len = sizeof(u_int64_t) + sizeof(unsigned short) + sizeof(unsigned short);
    unsigned short beacon_interval = 0;
    unsigned int ielen = buflen - beacon_head_len -ts_bintval_cap_len;
    unsigned char pri_chan = 0;
    unsigned char center_chan = 0;
    unsigned char concurrent_set_channel = 1;

#ifdef CONFIG_P2P
#ifdef CONFIG_CONCURRENT_MODE
    struct wifi_channel *main_vmac_chan = NULL;
#endif
    struct wifi_mac_p2p *p2p = wiphy_to_p2p(wiphy);
#endif

    DPRINTF(AML_DEBUG_BEACON, "%s %d <%s> head=%p head_len=%zd tail=%p tail_len=%zd \n",
        __func__, __LINE__, dev->name, linfo->head, linfo->head_len, linfo->tail, linfo->tail_len);

    if (buflen > (AML_SCAN_IE_LEN_MAX + beacon_head_len)) {
        DPRINTF(AML_DEBUG_ERROR, "%s(%d) buflen=%d too long, err\n", __func__, __LINE__, buflen);
        ret = -EINVAL;
        goto exit;
    }

    buf = ZMALLOC(buflen,"buf", GFP_ATOMIC);
    if (!buf) {
        DPRINTF(AML_DEBUG_ERROR, "%s(%d) buflen=%d no buf, err\n", __func__, __LINE__, buflen);
        ret = -ENOMEM;
        goto exit;
    }

    if (linfo->head && linfo->head_len) {
        memcpy(buf, linfo->head, linfo->head_len);
    }

    if (linfo->tail && linfo->tail_len) {
        memcpy(buf + linfo->head_len, linfo->tail, linfo->tail_len);
    }

    beacon_interval = READ_16L(buf + beacon_head_len + sizeof(u_int64_t));
    if (beacon_interval && (beacon_interval != wnet_vif->vm_bcn_intval)) {
        set_beacon_flag = 1;
        WIFINET_BEACON_LOCK(wifimac);
        wnet_vif->vm_bcn_intval = beacon_interval;
        wifi_mac_config(wifimac, CHIP_PARAM_BEACONINV, wnet_vif->vm_bcn_intval);
        WIFINET_BEACON_UNLOCK(wifimac);
    }
    DPRINTF(AML_DEBUG_BEACON, "%s %d sta_listen_intval=%d vm_bcn_intval=%d\n",
        __func__, __LINE__, sta->sta_listen_intval, wnet_vif->vm_bcn_intval);

    if (dtim_period && (dtim_period != wnet_vif->vm_dtim_period)) {
        set_beacon_flag = 1;
        wnet_vif->vm_dtim_period = dtim_period;
    }
    DPRINTF(AML_DEBUG_BEACON, "%s %d vm_dtim_period=%d\n", __func__, __LINE__, wnet_vif->vm_dtim_period);

    {
        unsigned short caps = READ_16L(buf + beacon_head_len + sizeof(u_int64_t) + sizeof(unsigned short));
        WIFINET_BEACON_LOCK(wifimac);
        if (caps & WIFINET_CAPINFO_PRIVACY) {
            if (!(wnet_vif->vm_flags & WIFINET_F_PRIVACY)) {
                wnet_vif->vm_flags |= WIFINET_F_PRIVACY;
                set_beacon_flag = 1;
            }

        } else {
            if (wnet_vif->vm_flags & WIFINET_F_PRIVACY) {
                wnet_vif->vm_flags &= ~WIFINET_F_PRIVACY;
                set_beacon_flag = 1;
            }
        }

        if (caps & WIFINET_CAPINFO_SHORT_PREAMBLE) {
            wnet_vif->vm_flags |= WIFINET_F_SHPREAMBLE;
            phy_set_preamble_type(1);

        } else {
            wnet_vif->vm_flags &= ~WIFINET_F_SHPREAMBLE;
            phy_set_preamble_type(0);
        }
        WIFINET_BEACON_UNLOCK(wifimac);
    }

    iebuf = buf + beacon_head_len + ts_bintval_cap_len;
    {
        unsigned char *dsps_ie = NULL;
        unsigned int dsps_ie_len = 0;

        unsigned char *ht_cap_ie = NULL;
        unsigned int ht_cap_ie_len = 0;
        unsigned short htcap_tmp = 0;
        unsigned char *ht_op_ie = NULL;
        unsigned int ht_op_ie_len = 0;

        dsps_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&dsps_ie_len, WIFINET_ELEMID_DSPARMS, 0);
        if (!dsps_ie) {
            ret = -EINVAL;
            DPRINTF(AML_DEBUG_ERROR, "%s %d no dsps_ie\n", __func__, __LINE__);
            goto exit_malloc;
        }
        if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())) {
            main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
            if (main_vmac_chan == NULL) {
                pri_chan = dsps_ie[2];
            } else {
                pri_chan = main_vmac_chan->chan_pri_num;
            }
        } else {
            pri_chan = dsps_ie[2];
            pr_debug("%s(%d),pri_chan:%d\n\n",__func__,__LINE__,pri_chan);

            if (wifi_mac_if_dfs_channel(wifimac, (int)pri_chan) == 1) {
                ERROR_DEBUG_OUT("Soft AP don't support in DFS channel \n");
                ret = -EINVAL;
                goto exit_malloc;
            }
        }

        ht_cap_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&ht_cap_ie_len, WIFINET_ELEMID_HTCAP, 0);
        ht_op_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&ht_op_ie_len, WIFINET_ELEMID_HTINFO, 0);

        if (ht_cap_ie) {
            htcap_tmp = READ_16L(ht_cap_ie + sizeof(unsigned short));

            if (wnet_vif->vm_p2p_support) {
                wnet_vif->vm_mac_mode = WIFINET_MODE_11GN;

            } else {
                if (pri_chan > 14) {
                    wnet_vif->vm_mac_mode = WIFINET_MODE_11GN;

                } else {
                    wnet_vif->vm_mac_mode = WIFINET_MODE_11BGN;
                }
            }

            if ((htcap_tmp & WIFINET_HTCAP_C_SHORTGI20)) {
                wifimac->wm_flags_ext |= WIFINET_FEXT_SHORTGI20;
            }

           if (htcap_tmp & WIFINET_HTCAP_C_SHORTGI40) {
                wifimac->wm_flags_ext |= WIFINET_FEXT_SHORTGI40;
           }

            /*For setting channel bandwidth*/
            if (htcap_tmp & WIFINET_HTCAP_SUPPORTCBW40) {
                wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH40;

            } else {
                wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH20;
            }

            if (htcap_tmp & WIFINET_HTCAP_LDPC) {
                wifimac->wm_flags |= WIFINET_F_LDPC;
                wnet_vif->vm_ldpc |= WIFINET_HTCAP_C_LDPC_RX;

            } else {
                wifimac->wm_flags &= ~WIFINET_F_LDPC;
                wnet_vif->vm_ldpc &= ~ WIFINET_HTCAP_C_LDPC_RX;
            }

        } else {
            unsigned char *basic_rate_ie = NULL;
            unsigned char *extend_rate_ie = NULL;
            unsigned int basic_rate_ie_len = 0;
            unsigned int extend_rate_ie_len = 0;

            DPRINTF(AML_DEBUG_ERROR, "%s(%d) no ht cap found, set to legacy mode.\n", __func__, __LINE__);
            wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH20;

            basic_rate_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&basic_rate_ie_len, WIFINET_ELEMID_RATES, 0);
            extend_rate_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&extend_rate_ie_len, WIFINET_ELEMID_XRATES, 0);

            if ((basic_rate_ie_len == 6) && (extend_rate_ie_len == 0)) {
                wnet_vif->vm_mac_mode = WIFINET_MODE_11B;

            } else if ((basic_rate_ie_len == 10) && (extend_rate_ie_len == 6)) {
                wnet_vif->vm_mac_mode = WIFINET_MODE_11BG;

            } else {
                wnet_vif->vm_mac_mode = WIFINET_MODE_11G;
            }
        }

        if (ht_op_ie) {
            if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())) {
                main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
                if (main_vmac_chan == NULL) {
                    pri_chan = ht_op_ie[2];
                } else {
                    pri_chan = main_vmac_chan->chan_pri_num;
                }
             } else {
                 pri_chan = ht_op_ie[2];

                 if (wifi_mac_if_dfs_channel(wifimac, (int)pri_chan) == 1) {
                     ERROR_DEBUG_OUT("Soft AP don't support in DFS channel \n");
                     ret = -EINVAL;
                     goto exit_malloc;
                 }
             }


            /*parse primary channel offset*/
            switch (ht_op_ie[3] & (BIT(0)|BIT(1))) {
                case WIFINET_HTINFO_EXTOFFSET_NA:
                    wnet_vif->vm_htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;
                    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH20;
                    center_chan = pri_chan;
                    break;

                case WIFINET_HTINFO_EXTOFFSET_ABOVE:
                    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH40;
                    center_chan = pri_chan + 2;
                    break;

                case WIFINET_HTINFO_EXTOFFSET_BELOW:
                    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH40;
                    center_chan = pri_chan - 2;
                    break;

                default:
                    ERROR_DEBUG_OUT("ht_op secondary channel offset: Not supported!!\n");
                    break;
            }
            wnet_vif->scnd_chn_offset = (ht_op_ie[3] & (BIT(0)|BIT(1)));

        } else {
            center_chan = pri_chan;
            DPRINTF(AML_DEBUG_ERROR, "%s(%d) no ht op IE found.\n", __func__, __LINE__);
        }

        /*Parse and dump vht cap and OP IE*/
        {
            unsigned char *vht_cap_ie = NULL;
            unsigned int vht_cap_ie_len = 0;
            struct wifi_mac_ie_vht_opt * vht_op_ie = NULL;
            unsigned int vht_op_ie_len = 0;

            vht_cap_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&vht_cap_ie_len, WIFINET_ELEMID_VHTCAP, 0);
            if (vht_cap_ie) {
                wnet_vif->vm_mac_mode = WIFINET_MODE_11GNAC;

                DPRINTF(AML_DEBUG_BEACON,"%s(%d),dump vht cap,len: %d\n",
                        __func__,__LINE__,vht_cap_ie_len);

                /*short GI, currently only parse SHORT-GI-80 item*/
                if ((vht_cap_ie[2] >> 5) & 0x1) {
                    sta->sta_vhtcap |= WIFINET_VHTCAP_SHORTGI_80;

                } else {
                    sta->sta_vhtcap &= ~WIFINET_VHTCAP_SHORTGI_80;
                }

                /*RXLDPC*/
                if ((vht_cap_ie[2] >> 4) & 0x1) {
                    sta->sta_vhtcap |= WIFINET_VHTCAP_RX_LDPC;
                    wifimac->wm_flags |= WIFINET_F_LDPC;

                } else {
                    sta->sta_vhtcap &= ~WIFINET_VHTCAP_RX_LDPC;
                    wifimac->wm_flags &= ~WIFINET_F_LDPC;
                }

            } else {
                ERROR_DEBUG_OUT("no vht cap ie found.\n");
            }

            /*Parse and dump vht OP IE*/
            vht_op_ie = (struct wifi_mac_ie_vht_opt *)vm_get_ie(iebuf,ielen,NULL,(unsigned int *)&vht_op_ie_len,WIFINET_ELEMID_VHTOP,0);
            if ((vht_op_ie != NULL) && (vht_op_ie->vht_op_chwidth > VHT_OPT_CHN_WD_2040M)) {
                DPRINTF(AML_DEBUG_BEACON,"\n%s(%d),dump vht_op_ie, len: %d, prim_ch %d, cf_ch %d\n",
                    __func__,__LINE__,vht_op_ie_len, pri_chan, vht_op_ie->vht_op_ch_freq_seg1);

                /*bandwidth/mode/channel offset*/
                pr_debug("%s(%d):chan %d, cntr_seg1 %d\n", __func__, __LINE__, pri_chan, vht_op_ie->vht_op_ch_freq_seg1);
                center_chan = vht_op_ie->vht_op_ch_freq_seg1;
                if (vht_op_ie->vht_op_chwidth == VHT_OPT_CHN_WD_80M) {
                    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH80;

                } else if (vht_op_ie->vht_op_chwidth == VHT_OPT_CHN_WD_160M) {
                    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH160;

                } else if (vht_op_ie->vht_op_chwidth == VHT_OPT_CHN_WD_80P80M) {
                    wnet_vif->vm_bandwidth = WIFINET_BWC_WIDTH80P80;

                } else {
                    ERROR_DEBUG_OUT("Not support such bw.\n");
                    ret = -EINVAL;
                    goto exit;
                }

            } else {
                ERROR_DEBUG_OUT("no vht op ie found.\n");
            }
        }
        wnet_vif->vm_mainsta->sta_bssmode = wnet_vif->vm_mac_mode;
        pr_debug("%s vm_mac_mode %d, offset %d, vm_bandwidth:%d, center_chan:%d\n", __func__,
            wnet_vif->vm_mac_mode, wnet_vif->scnd_chn_offset, wnet_vif->vm_bandwidth, center_chan);

#ifdef CONFIG_P2P
    vm_p2p_set_state(p2p, NET80211_P2P_STATE_LISTEN);
#ifdef CONFIG_CONCURRENT_MODE
    concurrent_channel_protection(wnet_vif, 3000);

    if (wifi_mac_is_others_wnet_vif_running(wnet_vif) == true) {
        main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
        if (main_vmac_chan == WIFINET_CHAN_ERR) {
            ret = -EINVAL;
            goto exit_malloc;
        }

        if ((main_vmac_chan->chan_pri_num == pri_chan)
             && (main_vmac_chan->chan_bw == wnet_vif->vm_bandwidth)
             && (center_chan == wifi_mac_Mhz2ieee(main_vmac_chan->chan_cfreq1, 0))) {

            concurrent_set_channel = 0;//no need set channel again due to bw
            wnet_vif->vm_curchan = wifi_mac_find_chan(wifimac, pri_chan, wnet_vif->vm_bandwidth, center_chan);

            if (wnet_vif->vm_curchan == WIFINET_CHAN_ERR) {
                DPRINTF(AML_DEBUG_ERROR,"WARNING<%s>:: %s %d can't support set this channel\n",
                    (wnet_vif)->vm_ndev->name, __func__, __LINE__);
                ret = -EINVAL;
                goto exit_malloc;
            }
        }
    } else {
        /*no other vmac running, no need set concurrent channel*/
        concurrent_set_channel = 0;
    }
#endif
#endif  /*#ifdef CONFIG_P2P*/

        /*Forcely refresh */
        if (IS_APSTA_CONCURRENT(aml_wifi_get_con_mode()) && concurrent_set_channel)
        {
            struct wlan_net_vif *main_wnet_vif = wifi_mac_running_main_wnet_vif(wifimac);
            if (main_wnet_vif != NULL) {
                /*sta connect to 5G ap, softap need update channel/band/mac_mode as sta*/
                wnet_vif->vm_curchan = main_vmac_chan;
                wnet_vif->vm_bandwidth = main_vmac_chan->chan_bw;
                wnet_vif->vm_mac_mode = main_wnet_vif->vm_mac_mode;
                wnet_vif->vm_mainsta->sta_bssmode = main_wnet_vif->vm_mac_mode;

                if (wnet_vif->vm_curchan->chan_bw >= WIFINET_BWC_WIDTH40) {
                    wnet_vif->vm_htcap |= WIFINET_HTCAP_SUPPORTCBW40;
                }

                if (main_vmac_chan->chan_pri_num < wifi_mac_Mhz2ieee(main_vmac_chan->chan_cfreq1, 0)) {
                    wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_ABOVE;
                } else if (main_vmac_chan->chan_pri_num > wifi_mac_Mhz2ieee(main_vmac_chan->chan_cfreq1, 0)) {
                    wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_BELOW;
                } else{
                    wnet_vif->scnd_chn_offset = WIFINET_HTINFO_EXTOFFSET_NA;
                }

                pri_chan = main_vmac_chan->chan_pri_num;
                center_chan = wifi_mac_Mhz2ieee(main_vmac_chan->chan_cfreq1, 0);
            }
        }

        if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
            struct wifi_channel *switch_chan = NULL;

            switch_chan = wifi_mac_find_chan(wifimac, pri_chan, wnet_vif->vm_bandwidth, center_chan);
            channel_switch_announce_trigger(wifimac, switch_chan);
        } else {
            struct wifi_channel *vmac_chan = wifi_mac_find_chan(wifimac, pri_chan, wnet_vif->vm_bandwidth, center_chan);

            if ((vmac_chan != WIFINET_CHAN_ERR) && vmac_chan != wnet_vif->vm_curchan) {
                wifi_mac_add_work_task(wifimac, vm_cfg80211_chan_switch_notify_task, NULL, (SYS_TYPE)wifimac, (SYS_TYPE)wnet_vif, 0, (SYS_TYPE)vmac_chan, 0);
            }
            /* find channel info from channel list by primary channel & bandwidth & center channel */
            if (!wifi_mac_set_wnet_vif_channel(wnet_vif,  pri_chan, wnet_vif->vm_bandwidth, center_chan, CHANNEL_CONNECT_FLAG | CHANNEL_RESTORE_FLAG)) {
                ret = -EINVAL;
                goto exit_malloc;
            }
        }

        DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_ERROR, "[%s]%d: current ap chan %d, mac mode %d, band %d\n",__func__,__LINE__,
                wnet_vif->vm_curchan->chan_pri_num, wnet_vif->vm_mac_mode, wnet_vif->vm_bandwidth);
    }

    {
        unsigned char *ssid_ie = NULL;
        unsigned int ssid_ie_len = 0;
        unsigned char *ssid = NULL;
        unsigned char ssidlen = 0;

        ssid_ie = vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&ssid_ie_len, WIFINET_ELEMID_SSID, 0);
        if ((!ssid_ie) || (ssid_ie_len > (2 + WIFINET_NWID_LEN))) {
            ret = -EINVAL;
            DPRINTF(AML_DEBUG_ERROR, "%s  %d ssid_ie err, ssid_ie_len=%d\n", __func__, __LINE__, ssid_ie_len);
            goto exit_malloc;
        }
        ssid = ssid_ie + 2;
        ssidlen = ssid_ie_len - 2;
        DPRINTF(AML_DEBUG_BEACON, "%s %d ssid=%s\n", __func__, __LINE__, ssid_sprintf(ssid, ssidlen));

        if (ssidlen && ((wnet_vif->vm_des_ssid[0].len != ssidlen) || (memcmp(wnet_vif->vm_des_ssid[0].ssid, ssid, ssidlen) != 0))) {
            WIFINET_BEACON_LOCK(wifimac);
            memset(wnet_vif->vm_des_ssid, 0, IV_SSID_SCAN_AMOUNT*sizeof(struct wifi_mac_ScanSSID));
            wnet_vif->vm_des_ssid[0].len = ssidlen;
            memcpy(wnet_vif->vm_des_ssid[0].ssid, ssid, ssidlen);
            wnet_vif->vm_des_nssid = 1;
            WIFINET_BEACON_UNLOCK(wifimac);
            set_beacon_flag = 1;
            DPRINTF(AML_DEBUG_CFG80211, "%s %d change ssid\n", __func__, __LINE__);
        }
    }

    {
        const struct wifi_mac_ie_wpa *wpaie = NULL;
        unsigned int wpaielen = 0;
        struct wifi_mac_Rsnparms wparsn;
        const struct wifi_mac_ie_rsn *rsnie = NULL;
        unsigned int rsnielen = 0;
        struct wifi_mac_Rsnparms *vm_rsn = &(sta->sta_rsn);
        unsigned int flags = 0;

        memset(&wparsn, 0, sizeof(struct wifi_mac_Rsnparms));
        memset(vm_rsn, 0, sizeof(struct wifi_mac_Rsnparms));

        wpaie = (const struct wifi_mac_ie_wpa *)vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&wpaielen, WIFINET_ELEMID_VENDOR, WPA_OUI_BE);
        if (wpaie && wpaielen) {
            wifi_mac_parse_wpa(&wparsn, (unsigned char *)wpaie, 1);
            flags |= WIFINET_F_WPA1;
            DPRINTF(AML_DEBUG_BEACON, "%s %d mcastcipher=0x%x ucastcipherset=0x%x keymgmtset=0x%x,rsn_caps=0x%x\n",
                    __func__, __LINE__, wparsn.rsn_mcastcipher, wparsn.rsn_ucastcipherset, wparsn.rsn_keymgmtset, wparsn.rsn_caps);
        }

        rsnie = (const struct wifi_mac_ie_rsn *)vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&rsnielen, WIFINET_ELEMID_RSN, 0);
        if (rsnie && rsnielen) {
            if (wifi_mac_parse_own_rsn(sta, (unsigned char *)rsnie)) {
                goto exit_malloc;;
            }

            if (vm_rsn->rsn_caps & MFP_MASK) {
                sta->sta_flags_ext |= WIFINET_NODE_MFP;
                pr_debug("%s disable hw mgmt decrypt\n", __func__);
                wifi_mac_disable_hw_mgmt_decrypt();
            }

            if (sta->sta_rsn.rsn_keymgmtset & (1 << RSN_ASE_8021X_SAE)) {
                wnet_vif->vm_mainsta->sta_authmode = WIFINET_AUTH_SAE;
            }
            flags |= WIFINET_F_WPA2;
        }

        if (wpaie && wpaielen && rsnie && rsnielen
            && ((wparsn.rsn_mcastcipher != vm_rsn->rsn_mcastcipher) || (wparsn.rsn_ucastcipherset != vm_rsn->rsn_ucastcipherset)
            || (wparsn.rsn_keymgmtset != vm_rsn->rsn_keymgmtset))) {
            ret = -EINVAL;
            DPRINTF(AML_DEBUG_BEACON, "%s %d mcastcipher=0x%x ucastcipherset=0x%x keymgmtset=0x%x\n",
                    __func__, __LINE__, wparsn.rsn_mcastcipher, wparsn.rsn_ucastcipherset, wparsn.rsn_keymgmtset);
            goto exit_malloc;
        }

        WIFINET_BEACON_LOCK(wifimac);
        if (!(flags & WIFINET_F_WPA2)) {
            vm_rsn->rsn_mcastcipher = wparsn.rsn_mcastcipher;
            vm_rsn->rsn_mcastkeylen = wparsn.rsn_mcastkeylen;
            vm_rsn->rsn_ucastcipherset = wparsn.rsn_ucastcipherset;
            vm_rsn->rsn_ucastkeylen = wparsn.rsn_ucastkeylen;
            vm_rsn->rsn_keymgmtset = wparsn.rsn_keymgmtset;
            vm_rsn->rsn_caps = wparsn.rsn_caps;
        }

        if (flags && ((wnet_vif->vm_flags & WIFINET_F_WPA) != flags)) {
            wnet_vif->vm_flags &= ~WIFINET_F_WPA;
            wnet_vif->vm_flags |= flags;
            set_beacon_flag = 1;

            DPRINTF(AML_DEBUG_CFG80211, "%s vm_flags=0x%x, rsn_mcastcipher=0x%x rsn_ucastcipherset=0x%x "
                "rsn_keymgmtset=0x%x, rsn_caps=0x%x\n", __func__, wnet_vif->vm_flags, vm_rsn->rsn_mcastcipher,
                vm_rsn->rsn_ucastcipherset, vm_rsn->rsn_keymgmtset, vm_rsn->rsn_caps);
        }
        WIFINET_BEACON_UNLOCK(wifimac);
    }

#ifdef CONFIG_P2P
    {
        const unsigned char *wpsie = NULL;
        uint wpsielen = 0;

        const unsigned char *p2pie = NULL;
        uint p2pielen = 0;

        const unsigned char *wdfie = NULL;
        uint wdfielen = 0;

        WIFINET_BEACON_LOCK(wifimac);
        wpsie = vm_get_ie(iebuf, ielen, NULL, &wpsielen, WIFINET_ELEMID_VENDOR, WSC_OUI_BE);
        if (wpsie && wpsielen)
        {
            struct wifi_mac_app_ie_t *wps_app_ie = &wnet_vif->vm_p2p->wps_beacon_ie;
            wifi_mac_save_app_ie(wps_app_ie,wpsie,wpsielen);
            DPRINTF(AML_DEBUG_BEACON, "%s %d found wpsie=%p wpsielen=%d \n",
                    __func__, __LINE__, wpsie, wpsielen);
        }

        p2pie = vm_get_ie(iebuf, ielen, NULL, &p2pielen, WIFINET_ELEMID_VENDOR, P2P_OUI_BE);
        if (p2pie && p2pielen)
        {
            struct wifi_mac_app_ie_t *p2p_app_ie =
                &wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_BEACON];

            wifi_mac_save_app_ie(p2p_app_ie,p2pie,p2pielen);
            DPRINTF(AML_DEBUG_BEACON, "%s %d found p2pie=%p p2pielen=%d\n",
                    __func__, __LINE__, p2pie, p2pielen);
        }

        wdfie = vm_get_wfd_ie(iebuf, ielen,NULL, &wdfielen);
        if ((wdfie != NULL)&&(wdfielen !=0))
        {
            struct wifi_mac_app_ie_t *wfd_app_ie =
                &wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_BEACON];

            wnet_vif->vm_p2p->wfd_info.wfd_enable = 1;
            wifi_mac_save_app_ie(wfd_app_ie,wdfie,wdfielen);
            DPRINTF(AML_DEBUG_BEACON, "%s %d found wdf =%p wdfielen=%d\n",
                    __func__, __LINE__, p2pie, p2pielen);
        }
        vm_p2p_update_beacon_app_ie(wnet_vif);
        WIFINET_BEACON_UNLOCK(wifimac);
    }
#endif //CONFIG_P2P

    {
        const struct wifi_mac_wme_param *wmmie = NULL;
        unsigned int wmmielen = 0;
        wmmie = (struct wifi_mac_wme_param *)vm_get_ie(iebuf, ielen, NULL, (unsigned int *)&wmmielen, WIFINET_ELEMID_VENDOR, WME_OUI_BE);
        if (wmmie && wmmielen)
        {
            struct wifi_mac_wme_state *wme = &wnet_vif->vm_wmac->wm_wme[wnet_vif->wnet_vif_id];
            DPRINTF(AML_DEBUG_BEACON, "%s %d found wmmie=%p wmmielen=%d\n",
                    __func__, __LINE__, wmmie, wmmielen);
            // dump_memory_internal(wmmie,wmmielen);
            if (wifimac->wm_caps & WIFINET_C_UAPSD)
            {
                WIFINET_BEACON_LOCK(wifimac);
                if (WME_AP_UAPSD_ENABLED(wmmie->param_qosInfo))
                {
                    WIFINET_VMAC_UAPSD_ENABLE(wnet_vif);
                }
                else
                {
                    WIFINET_VMAC_UAPSD_DISABLE(wnet_vif);
                }
                wifi_mac_config(wifimac, CHIP_PARAM_UAPSU_EN, WIFINET_VMAC_UAPSD_ENABLED(wnet_vif));
                WIFINET_BEACON_UNLOCK(wifimac);
                DPRINTF(AML_DEBUG_BEACON, "%s %d wmmuapsd=%d\n",
                        __func__, __LINE__, WIFINET_VMAC_UAPSD_ENABLED(wnet_vif));
            }
            DPRINTF(AML_DEBUG_BEACON, "%s %d wme_param_len_all=%zd\n",
                    __func__, __LINE__, sizeof(struct wifi_mac_wme_param));

            if (wmmielen == sizeof(struct wifi_mac_wme_param))
            {
                int ac = 0;
                int wme_updata_flag = 0;
                const struct wifi_mac_wme_acparams *acparam = NULL;
                struct wmeParams *lconfigacparam = NULL;
                // struct wmeParams *lchanacparam = NULL;
                DPRINTF(AML_DEBUG_BEACON, "%s %d is wmmparam\n", __func__, __LINE__);
                WIFINET_BEACON_LOCK(wifimac);
                for (ac=0; ac<WME_NUM_AC; ac++)
                {
                    acparam = &wmmie->params_acParams[ac];
                    if (WME_PARAM_ACI_GET(acparam->acp_aci_aifsn) != ac)
                    {
                        DPRINTF(AML_DEBUG_BEACON|AML_DEBUG_WARNING, "%s %d not match for ac=%d err\n",
                                __func__, __LINE__, ac);
                        continue;
                    }
                    lconfigacparam = &wme->wme_wmeBssChanParams.cap_wmeParams[ac];
                    lconfigacparam->wmep_aifsn = WME_PARAM_AIFSN_GET(acparam->acp_aci_aifsn);
                    lconfigacparam->wmep_acm = WME_PARAM_ACM_GET(acparam->acp_aci_aifsn);
                    lconfigacparam->wmep_txopLimit = acparam->acp_txop;
                    lconfigacparam->wmep_logcwmin = WME_PARAM_LOGCWMIN_GET(acparam->acp_logcwminmax);
                    lconfigacparam->wmep_logcwmax = WME_PARAM_LOGCWMAX_GET(acparam->acp_logcwminmax);
                    wme_updata_flag = 1;
                }
                WIFINET_BEACON_UNLOCK(wifimac);
                if (wme_updata_flag)
                {
                    DPRINTF(AML_DEBUG_BEACON, "%s %d wme_updata\n", __func__, __LINE__);
                    wifi_mac_wme_updateparams(wnet_vif);
                }
            }
            else
            {
                DPRINTF(AML_DEBUG_BEACON, "%s %d is wmminform\n", __func__, __LINE__);
                if (wmmielen != (WIFINET_WME_INFO_LEN+2))
                {
                    DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_WARNING, "%s %d wmminform len=%d err\n",
                            __func__, __LINE__, wmmielen);
                }
            }
        }
    }

    {
        unsigned char *ie_ptr[VENDOR_IE_MAX] = {NULL};
        unsigned char vendor_ielen[VENDOR_IE_MAX] = {0};
        unsigned char index = 0;
        vm_get_vendor_ie(iebuf, ielen, NULL, vendor_ielen,
            WIFINET_ELEMID_VENDOR, ie_ptr);
        DPRINTF(AML_DEBUG_BEACON, "set beacon %d %d %d\n", vendor_ielen[0], vendor_ielen[1], vendor_ielen[2]);

        while ((index < VENDOR_IE_MAX) && (vendor_ielen[index] > 0)) {
            memcpy(sta->sta_wmac->wm_vendorinfo[index].buf, ie_ptr[index], vendor_ielen[index]);
            sta->sta_wmac->wm_vendorinfo[index].ie = WIFINET_ELEMID_VENDOR;
            sta->sta_wmac->wm_vendorinfo[index].len = vendor_ielen[index];
            index++;
        }
    }

    pr_debug("%s set_beacon_flag %d, wm_flags_ext:%08x, wm_flags:%08x, sta_vhtcap:%08x\n", __func__,
        set_beacon_flag, wifimac->wm_flags_ext, wifimac->wm_flags, sta->sta_vhtcap);

    if (set_beacon_flag)
    {
        if (IS_RUNNING(wnet_vif->vm_ndev))
        {
            ret = wifi_mac_open(wnet_vif->vm_ndev);
            DPRINTF(AML_DEBUG_BEACON, "%s %d devopen over\n", __func__, __LINE__);
            wait_for_ap_run(wnet_vif, 1000, 50);
            DPRINTF(AML_DEBUG_BEACON, "%s %d wait_for_ap_run over\n", __func__, __LINE__);
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d only update beacon\n", __func__, __LINE__);
    }
exit_malloc:
    FREE(buf,"buf");
exit:
    return ret;
}

static int vm_cfg80211_start_ap(struct wiphy *wiphy, struct net_device *ndev,
    struct cfg80211_ap_settings *settings)
{
    int ret = 0;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private *drv_priv = drv_get_drv_priv();

    DPRINTF(AML_DEBUG_CFG80211, " %s(%d): <%s> hidden_ssid:%d, auth_type:%d\n", __func__, __LINE__, ndev->name,
        settings->hidden_ssid, settings->auth_type);

    if (settings->chandef.chan) {
        pr_debug("chandef center_freq:%d, hw_value:%d band:%d, at %s, %d\n",
            settings->chandef.chan->center_freq, settings->chandef.chan->hw_value,
            settings->chandef.chan->band, __func__, __LINE__);
        if (wifi_mac_if_dfs_channel(wifimac, (int)settings->chandef.chan->hw_value) == 1) {
            ERROR_DEBUG_OUT("Soft AP don't support in DFS channel \n");
            return -EINVAL;
        }

    } else {
        ERROR_DEBUG_OUT("settings->chandef.chan NULL\n");
    }

    if(settings->hidden_ssid)
        wnet_vif->vm_flags |= WIFINET_F_HIDESSID;
    else
        wnet_vif->vm_flags &= ~WIFINET_F_HIDESSID;

    wnet_vif->vm_p2p->go_hidden_mode = (wnet_vif->vm_flags & WIFINET_F_HIDESSID) >> WIFINET_F_HIDESSID_TO_BIT0_OFST;
    ret = _iv_cfg80211_add_set_beacon(wiphy, ndev, &settings->beacon,settings->dtim_period) ;
    tasklet_init(&drv_priv->forward_tasklet, drv_forward_tasklet, (unsigned long)(SYS_TYPE)wnet_vif);
    WIFINET_SAVEQ_INIT(&(wnet_vif->vm_forward_buffer_queue), "wnet_vif_forward_buffer_queue");
    if ((settings->ssid && settings->ssid_len) && ((wnet_vif->vm_des_ssid[0].len != settings->ssid_len)
      || memcmp(wnet_vif->vm_des_ssid[0].ssid, settings->ssid, settings->ssid_len) != 0)) {
        WIFINET_BEACON_LOCK(wifimac);
        memset(wnet_vif->vm_des_ssid, 0, IV_SSID_SCAN_AMOUNT*sizeof(struct wifi_mac_ScanSSID));
        wnet_vif->vm_des_ssid[0].len = settings->ssid_len;
        memcpy(wnet_vif->vm_des_ssid[0].ssid, settings->ssid, settings->ssid_len);
        wnet_vif->vm_des_nssid = 1;
        WIFINET_BEACON_UNLOCK(wifimac);

        DPRINTF(AML_DEBUG_BEACON, "%s %d ssid=%s\n", __func__, __LINE__, ssid_sprintf(settings->ssid, settings->ssid_len));
    }

    if ((wnet_vif->vm_mac_mode == WIFINET_MODE_11GNAC) && (drv_priv->drv_config.cfg_dynamic_bw == 1)) {
        #ifdef DYNAMIC_BW
            wifi_mac_set_reg_val(MAC_REG_BASE, wnet_vif->vm_bandwidth);
            wifi_mac_set_reg_val(PHY_AGC_BUSY_FSM, 0);
            wifi_mac_set_reg_val(MAC_RXPKT_CONTROL45, wnet_vif->vm_bandwidth);
        #endif
    }

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,7,0)
static int vm_cfg80211_change_beacon(struct wiphy *wiphy,
    struct net_device *ndev, struct cfg80211_ap_update *ap_update)
#else
static int vm_cfg80211_change_beacon(struct wiphy *wiphy,
    struct net_device *ndev, struct cfg80211_beacon_data *info)
#endif
{
    int ret = 0, time_delay = 0;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,7,0)
    struct cfg80211_beacon_data *info = &ap_update->beacon;
#endif

    while ((wnet_vif->vm_wmac->wm_flags & WIFINET_F_CHANSWITCH) && (time_delay < 1000)) {
        msleep(10);
        time_delay += 10;
    }

    DPRINTF(AML_DEBUG_CFG80211, " %s(%d): <%s>\n", __func__, __LINE__, ndev->name);
    ret = _iv_cfg80211_add_set_beacon(wiphy, ndev, info,wnet_vif->vm_dtim_period);

    return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static int vm_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *ndev)
#else
static int vm_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *ndev, unsigned int link_id)
#endif
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct drv_private *drv_priv = drv_get_drv_priv();
    if (wnet_vif == wnet_vif->vm_wmac->drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]) {
        wnet_vif->vm_p2p_support = 1;
    }
    DPRINTF(AML_DEBUG_CFG80211, " %s(%d): <%s>\n", __func__, __LINE__, ndev->name);
    wifi_mac_top_sm(wnet_vif, WIFINET_S_INIT,0);
    wifi_mac_buffer_txq_flush(&wnet_vif->vm_forward_buffer_queue);
    WIFINET_SAVEQ_DESTROY(&wnet_vif->vm_forward_buffer_queue);
    tasklet_kill(&drv_priv->forward_tasklet);
    return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
static int
vm_cfg80211_add_station(
    struct wiphy *wiphy, struct net_device *ndev,
     const  unsigned char *mac, struct station_parameters *params)
#else
static int
vm_cfg80211_add_station(
    struct wiphy *wiphy, struct net_device *ndev,
    unsigned char *mac, struct station_parameters *params)
#endif
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, ndev->name);
    return 0;
}

void wifi_softap_allsta_stopping(struct wlan_net_vif *wnet_vif, unsigned char is_disconnecting)
{

    struct wifi_station *sta_next = NULL;
    struct wifi_station *sta = NULL;

    struct wifi_station_tbl *nt = &wnet_vif->vm_sta_tbl;
    WIFINET_NODE_LOCK(nt);
    list_for_each_entry_safe(sta, sta_next, &nt->nt_nsta, sta_list) {
        sta->is_disconnecting = is_disconnecting;
    }
    WIFINET_NODE_UNLOCK(nt);
}

static void _del_station(void *arg, struct wifi_station *sta)
{
    int reason = WLAN_REASON_UNSPECIFIED;
    int total_delay = 0;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private *drv_priv = wifimac->drv_priv;
    unsigned short sta_flag = 0;

    DPRINTF(AML_DEBUG_CFG80211, "%s sta:%p, sta_associd:%d, sta_flags:0x%x\n", __func__, sta, sta->sta_associd, sta->sta_flags);
    sta->is_disconnecting = 1;
    if (sta->sta_associd != 0) {
        sta_flag = sta->sta_flags & WIFINET_NODE_PWR_MGT;
        wifi_mac_pwrsave_state_change(sta, 0);

        if (sta_flag && wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
            //do nothing
        } else {
            if (sta->sta_flags_ext & WIFINET_NODE_REASSOC) {
                reason = WLAN_REASON_PREV_AUTH_NOT_VALID;
                wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DISASSOC, (void *)&reason);

            } else {
                wifi_mac_send_mgmt(sta, WIFINET_FC0_SUBTYPE_DEAUTH, (void *)&reason);
            }
            while (total_delay < 1000 && (!drv_priv->hal_priv->hal_ops.hal_tx_empty())) {
                msleep(10);
                total_delay += 10;
            }
        }
    }
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
static int
vm_cfg80211_del_station(
    struct wiphy *wiphy,
    struct net_device *ndev, struct station_del_parameters *params)
#else
static int
vm_cfg80211_del_station(
    struct wiphy *wiphy,
    struct net_device *ndev,unsigned char *mac)
#endif
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    #if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
        unsigned char *mac =  (unsigned char* )params->mac;
    #endif

    unsigned char *lmac = mac;
    struct wifi_station *sta = NULL;
    int ret = 0;
    int arg = 0;
    DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);

    if (lmac == NULL) {
        return ret;

    } else if (WIFINET_ADDR_EQ(lmac, wnet_vif->vm_ndev->broadcast)) {
        wifi_mac_disassoc_all_sta(wnet_vif, _del_station, &arg);
        return ret;
    }

    DPRINTF(AML_DEBUG_CFG80211, "mac=%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, mac,wnet_vif->wnet_vif_id);
    if (sta == NULL) {
        ret = -EINVAL;
        DPRINTF(AML_DEBUG_CFG80211, "%s  %d not found sta\n", __func__, __LINE__);
        return ret;
    }

    if (sta->sta_associd != 0) {
        _del_station(&arg, sta);
        wifi_mac_sta_disconnect_from_ap(sta);
    }

    return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
static int
vm_cfg80211_change_station(struct wiphy *wiphy, struct net_device *ndev,
    const unsigned char *mac, struct station_parameters *params)

#else
static int
vm_cfg80211_change_station(struct wiphy *wiphy, struct net_device *ndev,
    unsigned char *mac, struct station_parameters *params)
#endif
{
    int ret;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_station *sta = NULL;

    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, ndev->name);

    if (mac) {
        DPRINTF(AML_DEBUG_CFG80211, "mac=%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, mac, wnet_vif->wnet_vif_id);
    }

    if (sta == NULL)
    {
        ret = -EINVAL;
        DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_ERROR, "%s(%d) not found sta\n", __func__, __LINE__);
        return ret;
    }

    ERROR_DEBUG_OUT("---find sta, not support yet\n");
    return 0;
}

static int vm_cfg80211_get_rate_index(struct wifi_station *sta)
{
    unsigned char rate_index = 0;
    unsigned char sta_vendor_rate_idx = sta->sta_vendor_rate_code & 0xf;
    int ht20_sgi_flag = 0;
    int ht40_sgi_flag = 0;
    int ht80_sgi_flag = 0;

    if (IS_MCS_RATE(sta->sta_vendor_rate_code)) {
        ht20_sgi_flag = (sta->sta_htcap & WMI_HT_CAP_HT20_SGI);
        ht40_sgi_flag = (sta->sta_htcap & WMI_HT_CAP_HT40_SGI);
        ht80_sgi_flag = (sta->sta_vhtcap & WIFINET_VHTCAP_SHORTGI_80);

        if (sta->sta_flags & WIFINET_NODE_VHT) {
            if(sta->sta_vendor_bw == WIFINET_BWC_WIDTH80) {
                if(ht80_sgi_flag) {
                    rate_index = sta_vendor_rate_idx + 92;
                } else {
                    rate_index = sta_vendor_rate_idx + 82;
                }
            } else if (sta->sta_vendor_bw == WIFINET_BWC_WIDTH40) {
                if(ht40_sgi_flag) {
                    rate_index = sta_vendor_rate_idx + 72;
                } else {
                    rate_index = sta_vendor_rate_idx + 62;
                }
            } else if (sta->sta_vendor_bw == WIFINET_BWC_WIDTH20){
                if(ht20_sgi_flag) {
                    rate_index = sta_vendor_rate_idx + 53;
              } else {
                    rate_index = sta_vendor_rate_idx + 44;
              }
            }
        } else if ( sta->sta_flags & WIFINET_NODE_HT) {
            if (sta->sta_vendor_bw == WIFINET_BWC_WIDTH40) {
                if(ht40_sgi_flag) {
                    rate_index = sta_vendor_rate_idx + 36;
                } else {
                    rate_index = sta_vendor_rate_idx + 28;
                }
            } else if (sta->sta_vendor_bw == WIFINET_BWC_WIDTH20) {
                if(ht20_sgi_flag) {
                    rate_index = sta_vendor_rate_idx + 20;
                } else {
                    rate_index = sta_vendor_rate_idx + 12;
                }
            }
        } else {
            ERROR_DEBUG_OUT("can not found rate: 0x%x\n", sta->sta_vendor_rate_code);
        }
    } else {
        rate_index = sta_vendor_rate_idx;
    }

    return rate_index;
}

static int vm_cfg80211_get_mcs(unsigned char code, unsigned char *flags)
{
    if ((code & 0xf0) == 0xc0) {
        *flags = 0x02;
    } else if ((code & 0xf0) == 0x80) {
        *flags = 0x01;
    }
    return (code & 0x0f);
}

static int
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
vm_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
        const u8 *mac, struct station_info *sinfo)
#else
vm_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
        u8 *mac, struct station_info *sinfo)
#endif // endif
{
    int ret = 0;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta = NULL;
    unsigned char rate_index = 0;
    unsigned char rate_flags = 0;
    const struct drv_rate_table *rt = wifimac->drv_priv->drv_currratetable;

    DPRINTF(AML_DEBUG_CONNECT, "%s vid:%d, mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
        __func__, wnet_vif->wnet_vif_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) || (wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT)) {
        if (memcmp(mac, wnet_vif->vm_des_bssid, ETH_ALEN) != 0) {
            return -ENOENT;
        }
    }

    if (wnet_vif->vm_state != WIFINET_S_CONNECTED) {
        return -ENOENT;
    }

    sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, mac, wnet_vif->wnet_vif_id);
    if (sta == NULL)
    {
        ret = -EINVAL;
        DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_ERROR, "%s(%d) not found sta\n", __func__, __LINE__);
        return ret;
    }

    sinfo->filled |= STATION_INFO_SIGNAL;
    sinfo->signal =  sta->sta_avg_bcn_rssi;
    if (sinfo->signal > -10) {
        //DPRINTF(AML_DEBUG_ERROR, "%s signal exceeds scope:%d\n", __func__, sinfo->signal);
        sinfo->signal = -10;
    }

    sinfo->filled |= STATION_INFO_TX_BYTES | STATION_INFO_TX_FAILED | STATION_INFO_TX_PACKETS;
    sinfo->tx_bytes = wnet_vif->vm_devstats.tx_bytes;
    sinfo->tx_packets = wnet_vif->vm_devstats.tx_packets;
    sinfo->tx_failed = wnet_vif->vm_devstats.tx_dropped + wnet_vif->vm_devstats.tx_errors;

    sinfo->filled |= STATION_INFO_TX_BITRATE;

    rate_index = vm_cfg80211_get_rate_index(sta);
    sinfo->txrate.legacy = rt->info[rate_index].rateKbps / 100;
    sinfo->txrate.mcs = vm_cfg80211_get_mcs(sta->sta_vendor_rate_code, &rate_flags);
    sinfo->txrate.flags = rate_flags;
    sinfo->txrate.nss = 1;
    sinfo->txrate.bw = sta->sta_vendor_bw == 0 ? 0 : sta->sta_vendor_bw == 1 ? 3 : 4;


    sinfo->filled |= STATION_INFO_RX_BITRATE;
    rate_flags = 0;
    sinfo->rxrate.mcs = vm_cfg80211_get_mcs(sta->sta_last_rx_vendor_rate, &rate_flags);
    sinfo->rxrate.flags = rate_flags;
    sinfo->rxrate.legacy = sta->sta_last_rxrate;
    if ( !rate_flags ) {
        sinfo->rxrate.legacy = sta->sta_last_rxrate / 100;
    }
    sinfo->rxrate.nss = 1;
    sinfo->rxrate.bw = sta->sta_last_rx_bw == 0 ? RATE_INFO_BW_20 : sta->sta_last_rx_bw == 1 ? RATE_INFO_BW_40 : RATE_INFO_BW_80;

    sinfo->filled |= STATION_INFO_BSS_PARAM;
    sinfo->bss_param.flags = 0;
    sinfo->bss_param.dtim_period = wnet_vif->vm_dtim_period;
    sinfo->bss_param.beacon_interval = wnet_vif->vm_bcn_intval;

    sinfo->filled |= STATION_INFO_INACTIVE_TIME;
    sinfo->inactive_time = (unsigned int)((jiffies - sta->sta_inact_time) * 1000 / HZ);

    DPRINTF(AML_DEBUG_CONNECT, "%s signal:%d, tx_bytes:%lld, tx_packets:%d, tx_failed:%d, txrate.legacy:%d, inactive_time:%x sinfo->rxrate.legacy=%d\n",
        __func__, sinfo->signal, sinfo->tx_bytes, sinfo->tx_packets, sinfo->tx_failed, sinfo->txrate.legacy, sinfo->inactive_time,sinfo->rxrate.legacy);
    return ret;
}


static int
vm_cfg80211_dump_station(
    struct wiphy *wiphy, struct net_device *ndev,
    int idx, unsigned char *mac, struct station_info *sinfo)
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, ndev->name);
    return -ENOENT;
}


static void vm_cancel_remain_channel(struct wlan_net_vif *wnet_vif)
{
    cfg80211_remain_on_channel_expired(wnet_vif->vm_wdev, wnet_vif->remain_on_ch_cookie,
        &wnet_vif->remain_on_ch_channel, GFP_KERNEL);
}

static int
vm_cfg80211_remain_on_channel(
    struct wiphy *wiphy,
    struct wireless_dev *wdev,
    struct ieee80211_channel * channel,
    unsigned int duration, unsigned long long *cookie)
{
    int err = 0;
    unsigned char p2p_mode = 0;

#ifdef CONFIG_P2P
    int target_channel;
    struct net_device *ndev = wdev_to_ndev(wdev);
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac_p2p *p2p = wiphy_to_p2p(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    //struct wireless_dev * sta_dev = wiphy_to_wdev(wiphy);
    unsigned char restore_duration = (duration % 100);
    struct drv_private* drv_priv = drv_get_drv_priv();
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);

    DPRINTF(AML_DEBUG_CFG80211, "<%s> %s  channel->center_freq=%d duration=%dms\n",
        ndev->name, __func__, channel->center_freq,  duration);

    p2p_mode = (wnet_vif->wnet_vif_id == NET80211_P2P_VMAC);

   // if ((p2p->wnet_vif == NULL) || (wnet_vif->vm_p2p_support == 0)) {
    //    ERROR_DEBUG_OUT("wnet_vif->vm_p2p_support==0 error!!!\n");
    //    err = -EFAULT;
   //     goto exit;
   // }
    if (drv_priv->drv_wnet_vif_table[(wnet_vif->wnet_vif_id+1)%2] != NULL)
    {
        struct wlan_net_vif *temp_vif = drv_priv->drv_wnet_vif_table[(wnet_vif->wnet_vif_id+1)%2];
        if (temp_vif->wnet_vif_id == 1 && temp_vif->remain_on_channel)
        {
            vm_p2p_cancel_remain_channel(temp_vif->vm_p2p);
            pr_debug("remain_on_channel: <%s> line 4498\n",temp_vif->vm_ndev->name);
            temp_vif->remain_on_channel = 0;
        }
        else if (temp_vif->wnet_vif_id == 0 && temp_vif->remain_on_channel)
        {
            vm_cancel_remain_channel(temp_vif);
            pr_debug("remain_on_channel: <%s> line 4504\n",temp_vif->vm_ndev->name);
            temp_vif->remain_on_channel = 0;
        }
    }

    if (duration > AML_MAX_REMAIN_ON_CHANNEL_DURATION)
    {
        DPRINTF(AML_DEBUG_WARNING, "%s %d duration=%d\n", __func__, __LINE__, duration);
        duration = AML_MAX_REMAIN_ON_CHANNEL_DURATION;
    }

    if (wifi_mac_pwrsave_is_wnet_vif_sleeping(wnet_vif) == 0)
    {
        wifi_mac_pwrsave_wakeup(wnet_vif, WKUP_FROM_REMAIN_ACTIVE);
    }

    preempt_scan(ndev, 100, 100);
    target_channel = ieee80211_frequency_to_channel(channel->center_freq);

    if (p2p->wnet_vif->vm_flags & WIFINET_F_NOSCAN)
    {
        DPRINTF(AML_DEBUG_WARNING, "%s %d center_freq %d ,multiple remainchan, should not happen\n",
            __func__, __LINE__, channel->center_freq);
        vm_p2p_cancel_remain_channel(p2p);
    }
    memcpy(&p2p->remain_on_ch_channel, channel, sizeof(struct ieee80211_channel));
    p2p->remain_on_ch_cookie = *cookie ;

#ifdef CONFIG_CONCURRENT_MODE
    concurrent_channel_protection(wnet_vif, 600);

    if (restore_duration < 2)
    {
        restore_duration = (++p2p->p2p_listen_count % 2 + 2);

    }
    else if (restore_duration > 5)
    {
        restore_duration = (++p2p->p2p_listen_count % 4 + 1);
    }

    if (p2p->p2p_listen_count == 12)
    {
        restore_duration = 5;
        p2p->p2p_listen_count = 0;
    }
#endif
    p2p->work_channel = wifi_mac_find_chan(wifimac, target_channel, WIFINET_BWC_WIDTH20, target_channel);

    if (wifi_mac_is_wm_running(wifimac) == true && wnet_vif->vm_p2p_support == 1)
    {
        p2p->need_restore_bsschan = REASON_RESOTRE_BSSCHAN_REMAIN;
        wifi_mac_scan_notify_leave_or_back(wnet_vif, 1);
        p2p->p2p_flag |= P2P_WAIT_SWITCH_CHANNEL;
    } else {
        wifi_mac_ChangeChannel(wifimac, p2p->work_channel, 0, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);
    }
    pr_debug("p2p_case: duration %d\n", restore_duration * 100);
    cfg80211_ready_on_channel(p2p->wnet_vif->vm_wdev, *cookie, channel,
    vm_p2p_discover_listen(p2p, target_channel, restore_duration * 100), GFP_KERNEL);

    wnet_vif->remain_on_channel = 1;

exit:
#endif //#ifdef CONFIG_P2P
    return err;
}

static int vm_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
        struct wireless_dev *wdev,
        unsigned long long cookie)
{
    unsigned char p2p_mode = 0;
#ifdef CONFIG_P2P
    struct wifi_mac_p2p *p2p = wiphy_to_p2p(wiphy);
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct net_device *dev = wdev_to_ndev(wdev);

    p2p_mode = (wnet_vif->wnet_vif_id == NET80211_P2P_VMAC);

    if (p2p_mode)
    {
        /*if(wnet_vif->vm_p2p_support == 0)
        {
            DPRINTF(AML_DEBUG_WARNING|AML_DEBUG_SCAN,
                "<%s>:%s %d wnet_vif->vm_p2p_support==0 error!!!\n",
                wnet_vif->vm_ndev->name,__func__, __LINE__);
            return 0;
        }*/
        DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, dev->name);
        vm_p2p_cancel_remain_channel(p2p);
    }
    else
    {
        /*int cnt = 0;
        if (wnet_vif->pub_state > WIFINET_PUB_INIT && wnet_vif->pub_state < WIFINET_PUB_GAS_RSP)
        {
            while (wnet_vif->pub_state != WIFINET_PUB_GAS_RSP && cnt < 50) {
                msleep(20);
                cnt++;
            }
        }
        AML_OUTPUT("cnt %d, pub_state %d\n", cnt, wnet_vif->pub_state);*/
        DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, dev->name);

        vm_cancel_remain_channel(wnet_vif);
    }

    wnet_vif->remain_on_channel = 0;
#endif //#ifdef CONFIG_P2P

    return 0;
}

int vm_cfg80211_send_mgmt(struct wlan_net_vif *wnet_vif, const unsigned char *buf, int len)
{
    struct wifi_skb_callback *cb;
    struct wifi_station *sta ;
    struct wifi_frame *wh;
    struct sk_buff *skb = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    skb = wifi_mac_alloc_skb(wifimac, len + WIFINET_MAX_WDF_IE);
    if (skb == NULL)
    {
        ERROR_DEBUG_OUT("skb_alloc fail \n");
        return -ENOMEM;
    }

    wh = (struct wifi_frame *)os_skb_data(skb);
    memcpy(wh, (void*)buf, len);
    os_skb_put(skb, len);

    sta = wifi_mac_find_mgmt_tx_sta(wnet_vif, wh->i_addr1);
    if (sta == NULL)
    {
        DPRINTF(AML_DEBUG_CFG80211,"%s %d:mac<%s>\n", __func__, __LINE__,ether_sprintf(wh->i_addr1));
        sta = wifi_mac_get_sta_node(&wnet_vif->vm_sta_tbl, wnet_vif, wh->i_addr1);
    }
    cb = (struct wifi_skb_callback *)skb->cb;
    cb->sta = sta;
    cb->flags = 0;
    cb->hdrsize = sizeof (struct wifi_frame);
    os_skb_set_priority(skb, WME_AC_VO);

    if (WIFINET_VMAC_IS_SLEEPING(sta->sta_wnet_vif))
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;

    *(unsigned short *)&wh->i_dur[0] = 0;
    *(unsigned short *)wh->i_seq = 0;

    wifi_mac_tx_mgmt_frm(wifimac, skb);

    return 0;
}

int vm_cfg80211_notify_mgmt_rx(struct wlan_net_vif *wnet_vif, unsigned short channel, void *data,int len)
{
    struct ieee80211_supported_band *band;
    struct wireless_dev *wdev = wnet_vif->vm_wdev;
    struct wifi_mac_p2p *p2p = wdev_to_p2p(wdev);
    int freq;
    struct net_device *dev = NULL;
    struct wifi_frame *wh = (struct wifi_frame *)data;
    struct wifi_mac_p2p_pub_act_frame * p2p_pub_act = NULL;
    struct wifi_mac_p2p_action_frame *p2p_act = NULL;
    struct wifi_mac_pub_gas_act_frame *pub_gas_act = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;
    unsigned char *p2p_noa;
    dev = wnet_vif->vm_ndev;

    //if (wnet_vif->vm_curchan && (channel != wnet_vif->vm_curchan->chan_pri_num)) {
        //channel = wnet_vif->vm_curchan->chan_pri_num;
    //}

    if (channel <= CFG_CH_MAX_2G_CHANNEL)
        band = wdev->wiphy->bands[IEEE80211_BAND_2GHZ];
    else
        band = wdev->wiphy->bands[IEEE80211_BAND_5GHZ];

    freq = ieee80211_channel_to_frequency(channel, band->band);
    if (!WIFINET_IS_ACTION(wh)
        || (WIFINET_IS_ACTION(wh)
        /*&& is_need_to_print((unsigned char *)data + sizeof(struct wifi_frame))*/)) {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> fc0=0x%x, fc1=0x%x, channel:%d, freq=%d\n",
            __func__, __LINE__, VMAC_DEV_NAME(wnet_vif), wh->i_fc[0], wh->i_fc[1], channel, freq);
    }
    if (wifimac->wm_nrunning == 1) {
        if ((freq != 2412) && (freq != 2437) && (freq != 2462)) {
            main_vmac_chan = wifi_mac_get_connect_wnet_vif_channel(wnet_vif);
            if (main_vmac_chan && ((main_vmac_chan->chan_pri_num == 1) ||
                (main_vmac_chan->chan_pri_num == 6) || (main_vmac_chan->chan_pri_num == 11))) {
                freq = 2407 + (main_vmac_chan->chan_pri_num * 5);
            }
        }
    }

    if (WIFINET_IS_ACTION(wh)) {
        vm_p2p_parse_negotiation_frames(p2p, data, &len, false);
        p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)((unsigned char *)data + sizeof(struct wifi_frame));
        p2p_act = (struct wifi_mac_p2p_action_frame *)((unsigned char *)data + sizeof(struct wifi_frame));
        pub_gas_act = (struct wifi_mac_pub_gas_act_frame *)((unsigned char *)data + sizeof(struct wifi_frame));
        if ((p2p_pub_act->category == AML_CATEGORY_PUBLIC)
            && (p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P)) {
            if (p2p_pub_act->oui_type == OUI_TYPE_P2P) {
                switch (p2p_pub_act->subtype) {
                    case P2P_GO_NEGO_RESP:
                    case P2P_INVITE_RESP:
                    case P2P_PROVISION_DISC_RESP:
                        if (p2p_pub_act->dialog_token == p2p->action_dialog_token) {
                            DPRINTF(AML_DEBUG_WARNING, "%s dialog_token:%d, send_tx_status_flag;%d\n", __func__, p2p_pub_act->dialog_token, p2p->send_tx_status_flag);
                            if (p2p->send_tx_status_flag) {
                                os_timer_ex_cancel(&wnet_vif->vm_actsend, CANCEL_SLEEP);
                                p2p->action_dialog_token = 0;
                                p2p->action_pkt_len = 0;
                            }

                        } else {
                            if (wnet_vif->vm_p2p->action_pkt_len) {
                                DPRINTF(AML_DEBUG_WARNING, "%s send %d again\n", __func__, p2p_pub_act->subtype);
                                vm_cfg80211_send_mgmt(wnet_vif, wnet_vif->vm_p2p->action_pkt, wnet_vif->vm_p2p->action_pkt_len);
                            }//else duplicate
                            goto _exit;
                        }
                        break;

                    default:
                        break;
                }

            }
            else if (p2p_pub_act->oui_type == OUI_TYPE_DPP)
            {
                unsigned char dpp_action_subtype = *((char*)data + sizeof(struct wifi_frame) + 7);
                AML_OUTPUT("DPP RX ACTION->%s\n", dpp_pub_action_str[dpp_action_subtype]);
                switch (dpp_action_subtype)
                {
                    case 0:
                        wnet_vif->pub_state = WIFINET_DPP_AUTH_REQ;
                        break;
                    case 1:
                        wnet_vif->pub_state = WIFINET_DPP_AUTH_RSP;
                        break;
                    case 2:
                        wnet_vif->pub_state = WIFINET_DPP_AUTH_CNF;
                        break;
                    default:
                        break;
                }
            }
        }
        else if ((pub_gas_act->category == AML_CATEGORY_PUBLIC)
              && ((pub_gas_act->action == WIFINET_ACT_PUBLIC_GAS_REQ) || (pub_gas_act->action == WIFINET_ACT_PUBLIC_GAS_RSP)))
        {
            unsigned char tag_len = *((char*)data + sizeof(struct wifi_frame) + 5);
            AML_OUTPUT("GAS ACTION,action code:%d\n", pub_gas_act->action);
            if ((tag_len >= 8)
                && (*((char*)data + sizeof(struct wifi_frame) + 7) == 0xdd)
                && (*((char*)data + sizeof(struct wifi_frame) + 8) == 0x05)
                && (*((char*)data + sizeof(struct wifi_frame) + 9) == 0x50)
                && (*((char*)data + sizeof(struct wifi_frame) + 10) == 0x6f)
                && (*((char*)data + sizeof(struct wifi_frame) + 11) == 0x9a)
                && (*((char*)data + sizeof(struct wifi_frame) + 12) == 0x1a)
                && (*((char*)data + sizeof(struct wifi_frame) + 13) == 0x01)) {
                pr_debug("[DPP Configuration]\n");
                }
            switch (pub_gas_act->action)
            {
                case WIFINET_ACT_PUBLIC_GAS_REQ:
                    wnet_vif->pub_state = WIFINET_PUB_GAS_REQ;
                    break;
                case WIFINET_ACT_PUBLIC_GAS_RSP:
                    wnet_vif->pub_state = WIFINET_PUB_GAS_RSP;
                    break;
                default:
                    break;
            }
        }
        else if ((p2p_act->category == AML_CATEGORY_P2P) && (p2p_act->subtype == P2P_PRESENCE_RESP)) {
            if (p2p_act->dialog_token == p2p->action_dialog_token) {
                DPRINTF(AML_DEBUG_WARNING, "%s, rx P2P_PRESENCE_RESP, dialog_token:%d\n", __func__, p2p_act->dialog_token);
                p2p_noa = vm_p2p_get_p2pie_noa_ie(p2p, p2p_act->elts, &len);
                if (p2p_noa && (wnet_vif->vm_state == WIFINET_S_CONNECTED)
                    && vm_p2p_chk_p2p_role(p2p, NET80211_P2P_ROLE_CLIENT)) {
                    vm_p2p_client_parse_noa_ie(p2p, (const unsigned char *)p2p_noa);
                }
                os_timer_ex_cancel(&wnet_vif->vm_actsend, CANCEL_SLEEP);
                p2p->action_dialog_token = 0;
                p2p->action_pkt_len = 0;
            } else {
                if (wnet_vif->vm_p2p->action_pkt_len) {
                    DPRINTF(AML_DEBUG_WARNING, "%s send %d again\n", __func__, p2p_act->subtype);
                    vm_cfg80211_send_mgmt(wnet_vif, wnet_vif->vm_p2p->action_pkt, wnet_vif->vm_p2p->action_pkt_len);
                }//else duplicate
                goto _exit;
            }
        }
    }
    else if (WIFINET_IS_MGM(wh))
    {
        //mgmt_frame = (unsigned char *)((wl_event_rx_frame_data_t *)rxframe + 1);
    }
    else
    {
        goto _exit;
    }
    if (!WIFINET_IS_ACTION(wh)
        || (WIFINET_IS_ACTION(wh)
        && is_need_to_print((unsigned char *)data + sizeof(struct wifi_frame)))) {
        DPRINTF(AML_DEBUG_CFG80211, "%s %d call cfg80211_rx_mgmt\n", __func__, __LINE__);
    }

    /**
     * enum nl80211_rxmgmt_flags - flags for received management frame.
     *
     * Used by cfg80211_rx_mgmt()
     *
     * @NL80211_RXMGMT_FLAG_ANSWERED: frame was answered by device/driver.
     */
    #if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
        cfg80211_rx_mgmt(wnet_vif->vm_wdev, freq, 0,data, len, GFP_ATOMIC);
    #else
        cfg80211_rx_mgmt(wnet_vif->vm_wdev, freq, 0,data, len,NL80211_RXMGMT_FLAG_ANSWERED, GFP_ATOMIC);
    #endif

_exit:
    return 0;
}

static int vm_cfg80211_mgmt_tx_p2p(struct wiphy *wiphy, struct wireless_dev *wdev,
    struct cfg80211_mgmt_tx_params *params, unsigned long long *cookie)
{
    struct wifi_mac_p2p_pub_act_frame *p2p_pub_act = NULL;
    struct wifi_mac_p2p_action_frame *p2p_act = NULL;
    struct net_device *ndev = wdev_to_ndev(wdev);

    int ret = 0;
    unsigned char ack = false;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac_p2p *p2p = wiphy_to_p2p(wiphy);
    struct vm_wdev_priv *pwdev_priv = wiphy_to_priv(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    const struct wifi_frame *wh;
    struct hal_private * hal_priv = hal_get_priv();
    int target_channel;
    struct iw_mlme wextmlme;
    unsigned long long id = 0;
    struct net_device *dev = ndev;
    unsigned int center_freq = params->chan->center_freq;

    p2p_act = (struct wifi_mac_p2p_action_frame *)(params->buf + sizeof(struct wifi_frame));
    p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)(params->buf + sizeof(struct wifi_frame));
    if (wnet_vif->vm_opmode != WIFINET_M_HOSTAP && wnet_vif->vm_state == WIFINET_S_CONNECTED) {
        if (is_need_process_p2p_action((unsigned char*)params->buf)) {
            pr_debug("%s, Need to send action frame in the connected state, params->len=%d\n", __func__, params->len);
        } else {
            return 0;//no need to send mgmt after connected,if so, disconnect first
        }
    }

    if(ndev == pwdev_priv->pGo_ndev) {
        dev = wnet_vif->vm_ndev;
    }

    ack = true;
    id = pwdev_priv->send_action_id++;
    if (id == 0)
        id = pwdev_priv->send_action_id++;
    *cookie = id;

    wh = (const struct wifi_frame*)params->buf;
    DPRINTF(AML_DEBUG_CFG80211, "<%s>: %s %d:addr2 %s dev_addr %s\n",
        dev->name,__func__, __LINE__,ether_sprintf(wh->i_addr2), ether_sprintf(ndev->dev_addr));

    DPRINTF(AML_DEBUG_ANY,"%s len=%zd, ch=%d, frameCtl [0]=0x%x [1]=0x%x wnet_vif_id=%d center_freq=%d\n",
        __func__, params->len, ieee80211_frequency_to_channel(center_freq), wh->i_fc[0], wh->i_fc[1],wnet_vif->wnet_vif_id,center_freq);

    if (WIFINET_IS_ACTION(wh)) {
        preempt_scan(dev, 100, 100);

    } else {
        if (WIFINET_IS_PROBERSP(wh)) {
            DPRINTF(AML_DEBUG_CFG80211, "%s,fc == WIFINET_STYPE_PROBE_RESP\n", __func__);

        } else if (WIFINET_IS_DEAUTH(wh) ||WIFINET_IS_DISASSOC(wh)) {
            memcpy(wextmlme.addr.sa_data, wh->i_addr1, ETH_ALEN);
            wextmlme.reason_code = 0;
            wextmlme.cmd =IW_MLME_DEAUTH;
            //wifi_mac_wext_Setwmine(vmac->vm_dev,NULL,NULL,&wextmlme);
            DPRINTF(AML_DEBUG_CFG80211, "Disconnect STA : %s\n",ether_sprintf(wh->i_addr1));
        }
        goto exit;
    }

    os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);
    os_timer_ex_cancel(&wnet_vif->vm_actsend, CANCEL_SLEEP);
    target_channel = ieee80211_frequency_to_channel(center_freq);

#ifdef CONFIG_CONCURRENT_MODE
    concurrent_channel_protection(wnet_vif, 1500);
    concurrent_channel_restore(target_channel,wnet_vif, 1500);
#endif

    if ((wifimac->wm_curchan != WIFINET_CHAN_ERR)) {
        if (target_channel != wifimac->wm_curchan->chan_pri_num) {
            int bw = 0, center_chan = 0;

            if (wnet_vif->vm_bandwidth <= WIFINET_BWC_WIDTH40) {
                if (wnet_vif->scnd_chn_offset == WIFINET_HTINFO_EXTOFFSET_ABOVE)
                    center_chan = target_channel + 2;
                else if (wnet_vif->scnd_chn_offset == WIFINET_HTINFO_EXTOFFSET_BELOW)
                    center_chan = target_channel - 2;
                else
                    center_chan = target_channel;

            } else if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH80) {
                center_chan = target_channel + (wifimac->wm_curchan->chan_pri_num
                    - wifi_mac_Mhz2ieee(wifimac->wm_curchan->chan_cfreq1, 0));
            }

            p2p->work_channel = wifi_mac_find_chan(wifimac, target_channel, bw, center_chan);
            if (p2p->work_channel == NULL) {
                p2p->work_channel = wifi_mac_find_chan(wifimac, target_channel, WIFINET_BWC_WIDTH20, target_channel);
            }

            if (wifi_mac_is_wm_running(wifimac) == true && !hal_priv->hal_ops.hal_tx_empty()) {
                p2p->need_restore_bsschan = REASON_RESOTRE_BSSCHAN_TXMGNT;
                wifi_mac_scan_notify_leave_or_back(wnet_vif, 1);
                p2p->p2p_flag |= P2P_WAIT_SWITCH_CHANNEL;
                while (p2p->p2p_flag & P2P_WAIT_SWITCH_CHANNEL) {
                   msleep(1);
                   AML_OUTPUT("p2p->work_channel->chan_pri_num:%d, wifimac->wm_curchan->chan_pri_num:%d\n",
                                p2p->work_channel->chan_pri_num, wifimac->wm_curchan->chan_pri_num);
                }
            } else {
                wifi_mac_ChangeChannel(wifimac, p2p->work_channel, 0, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);
            }
        } else {
            p2p->work_channel = wifimac->wm_curchan;
        }
    }
    AML_PRINT(AML_DBG_MODULES_P2P, "category:%d, subtype:%d, action:%d\n", p2p_pub_act->category, p2p_pub_act->subtype, p2p_pub_act->action);
    if ((p2p_pub_act->category == AML_CATEGORY_PUBLIC)
        && (p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P)) {
        switch (p2p_pub_act->subtype) {
            case P2P_PROVISION_DISC_REQ:
            case P2P_PROVISION_DISC_RESP:
            case P2P_GO_NEGO_REQ:
            case P2P_GO_NEGO_RESP:
            case P2P_INVITE_REQ:
            case P2P_INVITE_RESP:
            case P2P_GO_NEGO_CONF:
            case P2P_DEVDISC_REQ:
            case P2P_DEVDISC_RESP:
                if (params->len <= P2P_MAX_ACTION_LEN) {
                    p2p->cookie = *cookie;
                    p2p->action_pkt_len = params->len;
                    p2p->raw_action_pkt_len = params->len;
                    p2p->action_dialog_token = p2p_pub_act->dialog_token;
                    p2p->act_pkt_retry_count = 0;
                    p2p->tx_status_flag = WIFINET_TX_STATUS_NOSET;
                    p2p->send_tx_status_flag = 0;
                    memset(p2p->action_pkt, 0, P2P_MAX_ACTION_LEN);
                    memcpy(p2p->action_pkt, params->buf, params->len);
                    memset(p2p->raw_action_pkt, 0, P2P_MAX_ACTION_LEN);
                    memcpy(p2p->raw_action_pkt, params->buf, params->len);

                    if (vm_p2p_parse_negotiation_frames(p2p, (const unsigned char*)(p2p->action_pkt),
                       (unsigned int*)&p2p->action_pkt_len, true) == false) {
                        ack = false;
                        ERROR_DEBUG_OUT("print frame err\n");
                        goto exit;
                    }

                } else {
                    ERROR_DEBUG_OUT("p2p action pkt buffer not enough\n");
                    goto exit;
                }
                break;

            default:
                break;
        }
    } else {
        if (params->len <= P2P_MAX_ACTION_LEN) {
            p2p->cookie = *cookie;
            p2p->action_pkt_len = params->len;
            p2p->raw_action_pkt_len = params->len;
#ifdef CTS_VERIFIER_GAS
            p2p->action_code = p2p_pub_act->action;
#endif
            p2p->action_dialog_token = p2p_act->dialog_token;
            p2p->act_pkt_retry_count = 0;
            p2p->tx_status_flag = WIFINET_TX_STATUS_NOSET;
            p2p->send_tx_status_flag = 0;
            memset(p2p->action_pkt, 0, P2P_MAX_ACTION_LEN);
            memcpy(p2p->action_pkt, params->buf, params->len);
            memset(p2p->raw_action_pkt, 0, P2P_MAX_ACTION_LEN);
            memcpy(p2p->raw_action_pkt, params->buf, params->len);

        } else {
            ERROR_DEBUG_OUT("p2p action pkt buffer not enough\n");
            goto exit;
          }
      }

    if (vm_cfg80211_send_mgmt(wnet_vif,p2p->action_pkt, p2p->action_pkt_len) != 0)
    {
        ERROR_DEBUG_OUT("send frame err\n");
        ack = false;
        goto exit;
    }

#ifdef CTS_VERIFIER_GAS
    if (p2p_pub_act->action == WIFINET_ACT_PUBLIC_GAS_REQ) {
        wnet_vif->vm_p2p->p2p_flag &= (~P2P_GAS_RSP);
    }
#endif
    do {
        msleep(5);
        if ((p2p_pub_act->action == WIFINET_ACT_PUBLIC_GAS_REQ)
            && (wnet_vif->vm_p2p->p2p_flag & P2P_GAS_RSP)) {
            break;
        }
    } while (p2p->tx_status_flag != WIFINET_TX_STATUS_SUCC
            && p2p->act_pkt_retry_count <= DEFAULT_P2P_ACTION_RETRY_TIMES
            && !p2p->send_tx_status_flag);

    if (p2p->tx_status_flag != WIFINET_TX_STATUS_SUCC) {
        ack = false;
    }
    chan_dbg(wifimac->wm_curchan, "X9", 4509);

exit:
    DPRINTF(AML_DEBUG_CFG80211,"%s %d, ack=%d\n", __func__,__LINE__, ack);
    cfg80211_mgmt_tx_status(p2p->wnet_vif->vm_wdev, *cookie, params->buf, params->len, ack, GFP_KERNEL);
    return ret;
}

static int vm_cfg80211_mgmt_tx_sta(struct wiphy *wiphy, struct wireless_dev *wdev,
    struct cfg80211_mgmt_tx_params *params, unsigned long long *cookie)
{
    int ret = 0;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct net_device *ndev = wdev_to_ndev(wdev);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    const struct wifi_frame *wh;
    int target_channel;
    unsigned int center_freq = params->chan->center_freq;
    unsigned char ack = true;
    unsigned char auth_alg;
    struct wifi_channel *auth_channel;
    unsigned short seq = 0;
    struct wifi_mac_pub_gas_act_frame *pub_gas_act = NULL;

    wh = (const struct wifi_frame*)params->buf;
    pub_gas_act = (struct wifi_mac_pub_gas_act_frame *)(params->buf + sizeof(struct wifi_frame));
    DPRINTF(AML_DEBUG_CFG80211, "<%s>: %s:addr2 %s dev_addr %s\n",
        ndev->name, __func__, ether_sprintf(wh->i_addr2), ether_sprintf(ndev->dev_addr));

    DPRINTF(AML_DEBUG_ANY,"%s len=%zd, ch=%d, frameCtl [0]=0x%x [1]=0x%x wnet_vif_id=%d center_freq=%d\n",
        __func__, params->len, ieee80211_frequency_to_channel(center_freq), wh->i_fc[0], wh->i_fc[1],wnet_vif->wnet_vif_id,center_freq);

    if (WIFINET_IS_AUTH(wh)) {
        auth_alg = AML_GET_LE16((unsigned char *)wh + sizeof(struct wifi_frame));
        seq = AML_GET_LE16((unsigned char *)wh + sizeof(struct wifi_frame) + 2);

        if ((auth_alg != WLAN_AUTH_SAE) || (wnet_vif->vm_mainsta->sta_authmode != WIFINET_AUTH_SAE))
            goto exit;

        DPRINTF(AML_DEBUG_ANY,"auth_alg=%d, seq:%04x, status:%d\n", auth_alg,
            seq, AML_GET_LE16((unsigned char *)wh + sizeof(struct wifi_frame) + 4));
    }

    os_timer_ex_cancel(&wnet_vif->vm_mgtsend, CANCEL_SLEEP);

    target_channel = ieee80211_frequency_to_channel(center_freq);
    if ((wifimac->wm_curchan != WIFINET_CHAN_ERR)) {
        if (target_channel != wifimac->wm_curchan->chan_pri_num) {
            int bw = 0, center_chan = 0;

            if (wnet_vif->vm_bandwidth <= WIFINET_BWC_WIDTH40) {
                if (wnet_vif->scnd_chn_offset == WIFINET_HTINFO_EXTOFFSET_ABOVE)
                    center_chan = target_channel + 2;
                else if (wnet_vif->scnd_chn_offset == WIFINET_HTINFO_EXTOFFSET_BELOW)
                    center_chan = target_channel - 2;
                else
                    center_chan = target_channel;

            } else if (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH80) {
                center_chan = target_channel + (wifimac->wm_curchan->chan_pri_num
                    - wifi_mac_Mhz2ieee(wifimac->wm_curchan->chan_cfreq1, 0));
            }

            auth_channel = wifi_mac_find_chan(wifimac, target_channel, bw, center_chan);
            if (auth_channel == NULL) {
                auth_channel = wifi_mac_find_chan(wifimac, target_channel, WIFINET_BWC_WIDTH20, target_channel);
            }
            wifi_mac_ChangeChannel(wifimac, auth_channel, 0, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);

        } else {
            auth_channel = wifimac->wm_curchan;
        }
    }

    switch (pub_gas_act->action)
    {
        case WIFINET_ACT_PUBLIC_GAS_REQ:
            wnet_vif->pub_state = WIFINET_PUB_GAS_REQ;
            break;
        case WIFINET_ACT_PUBLIC_GAS_RSP:
            wnet_vif->pub_state = WIFINET_PUB_GAS_RSP;
            break;
    }

    if (vm_cfg80211_send_mgmt(wnet_vif, params->buf, params->len) != 0) {
        DPRINTF(AML_DEBUG_ERROR, "%s %d send frame err\n", __func__,__LINE__);
        ack = false;
        goto exit;
    }

exit:
    DPRINTF(AML_DEBUG_CFG80211,"%s %d, ack=%d\n", __func__,__LINE__, ack );
    cfg80211_mgmt_tx_status(wnet_vif->vm_wdev, *cookie, params->buf, params->len, ack, GFP_KERNEL);
    return ret;
}

static int vm_cfg80211_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
    struct cfg80211_mgmt_tx_params *params, unsigned long long *cookie)
{
    int ret = 0;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) || (wnet_vif->vm_opmode == WIFINET_M_P2P_CLIENT) || (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)) {
        if (wnet_vif->vm_p2p_support) {
            ret = vm_cfg80211_mgmt_tx_p2p(wiphy, wdev, params, cookie);

        } else {
            ret = vm_cfg80211_mgmt_tx_sta(wiphy, wdev, params, cookie);
        }
    }
    pr_debug("vm_cfg80211_mgmt_tx: opmode %d, p2p_support %d, ret %d\n", wnet_vif->vm_opmode, wnet_vif->vm_p2p_support, ret);

    return ret;
}

static int
vm_cfg80211_assoc(struct wiphy *wiphy, struct net_device *dev,
                  struct cfg80211_assoc_request *req)
{
    DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s>\n", __func__, __LINE__, dev->name);
    return 0;
}

//-++



static int
vm_cfg80211_set_txq_params(struct wiphy *wiphy, struct net_device *dev,
                            struct ieee80211_txq_params *params)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac_wme_state *wme = &wnet_vif->vm_wmac->wm_wme[wnet_vif->wnet_vif_id];
    struct wmeParams *wmep = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    switch(params->ac)
    {
        case NL80211_AC_VO:
            wmep = &wme->wme_chanParams.cap_wmeParams[WME_AC_VO];
            break;
        case NL80211_AC_VI:
            wmep = &wme->wme_chanParams.cap_wmeParams[WME_AC_VI];
            break;
        case NL80211_AC_BE:
            wmep = &wme->wme_chanParams.cap_wmeParams[WME_AC_BE];
            break;
        case NL80211_AC_BK:
            wmep = &wme->wme_chanParams.cap_wmeParams[WME_AC_BK];
            break;
        default:
            ERROR_DEBUG_OUT("error ac: %d \n", params->ac);
            return 0;
    }
    wmep->wmep_aifsn = params->aifs;
    wmep->wmep_logcwmax = WME_PARAM_LOGCWMIN_GET(params->cwmin);
    wmep->wmep_logcwmin =  WME_PARAM_LOGCWMAX_GET(params->cwmax);
    wmep->wmep_txopLimit = params->txop;

    pr_debug("%s(%d) aifs:%d, cwmax:%d, cwmin:%d, txop_lmt:%d\n", __func__, __LINE__,
           wmep->wmep_aifsn,  wmep->wmep_logcwmax, wmep->wmep_logcwmin,  wmep->wmep_txopLimit);

    wme_update_ex(wifimac,wnet_vif);

    return 0;
}

static int
vm_cfg80211_set_antenna(struct wiphy *wiphy, int radio_idx, unsigned int tx_ant, unsigned int rx_ant)
{
    ERROR_DEBUG_OUT("no support yet \n");
    return -1;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
static int
vm_cfg80211_get_channel(struct wiphy *wiphy, struct wireless_dev *wdev,
                        struct cfg80211_chan_def *chandef)
#else
static int
vm_cfg80211_get_channel(struct wiphy *wiphy, struct wireless_dev *wdev,
                        unsigned int link_id, struct cfg80211_chan_def *chandef)
#endif
{
    struct wlan_net_vif *wnet_vif = NULL;
    struct wifi_mac *wifimac = NULL;
    struct wifi_channel *temp_chan = NULL;
    pr_debug("enter %s, no support yet\n", __func__);
    return -1;
    if (!chandef || !wiphy) {
       ERROR_DEBUG_OUT("chandef or wiphy null\n");
       return -1;
    }

    wnet_vif = wiphy_to_adapter(wiphy);
    if (wnet_vif->vm_wmac) {
        wifimac = wnet_vif->vm_wmac;
    } else {
        ERROR_DEBUG_OUT("vm_wmac null \n");
        return -1;
    }

    if (wnet_vif->vm_curchan) {
        temp_chan = wnet_vif->vm_curchan;
    } else if (wifimac->wm_curchan) {
        temp_chan = wifimac->wm_curchan;
    } else {
        ERROR_DEBUG_OUT("---curchan NULL\n");
        return -1;
    }
    if (temp_chan) {
        if (chandef->chan) {
            pr_debug("enter %s, %d\n", __func__, __LINE__);
         } else {
            ERROR_DEBUG_OUT("chandef->chan null \n");
         }
    } else {
        ERROR_DEBUG_OUT("curchan is null\n");
        return -1;
    }

    chan_dbg(wnet_vif->vm_wmac->wm_curchan, "x3", 4623);
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
static int
vm_cfg80211_set_wds_peer(struct wiphy *wiphy, struct net_device *dev,
                        const unsigned char *addr)
{
    ERROR_DEBUG_OUT("no support yet\n");
    return -1;
}
#endif

static int
vm_cfg80211_set_qos_map(struct wiphy *wiphy,struct net_device *dev,
                        struct cfg80211_qos_map *qos_map)
{
    ERROR_DEBUG_OUT("no support yet \n");
    return -1;
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
static int
vm_cfg80211_sched_scan_stop(struct wiphy *wiphy, struct net_device *dev,u64 reqid)
#else
static int
vm_cfg80211_sched_scan_stop(struct wiphy *wiphy, struct net_device *dev)
#endif
{
    ERROR_DEBUG_OUT("no support yet\n");
    return -1;
}

static int
vm_cfg80211_set_noack_map(struct wiphy *wiphy,struct net_device *dev,
                          unsigned short noack_map)
{

    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    pr_debug("enter %s\n", __func__);
    pr_debug("---wiphy addr:  0x%p\n", wiphy);

    if (noack_map & BE_MAP)
        wifimac->drv_priv->drv_txlist_table[HAL_WME_AC_BE].noack_flag = 1;
    else if (noack_map & BK_MAP)
        wifimac->drv_priv->drv_txlist_table[HAL_WME_AC_BK].noack_flag = 1;
    else if (noack_map & VI_MAP)
        wifimac->drv_priv->drv_txlist_table[HAL_WME_AC_VI].noack_flag = 1;
    else if (noack_map & VO_MAP)
        wifimac->drv_priv->drv_txlist_table[HAL_WME_AC_VO].noack_flag = 1;
    else
        ERROR_DEBUG_OUT("invalid noack_map\n");

    return 0;
}
static int
vm_cfg80211_start_p2p_device(struct wiphy *wiphy,struct wireless_dev *wdev)
{
#ifdef CONFIG_P2P //TODO
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wdev);
    struct wifi_mac_p2p *p2p = &pwdev_priv->p2p;

    pr_debug("%s(%d) \n", __func__, __LINE__);
    if (!vm_p2p_enabled(p2p))
    {
        vm_p2p_up(wnet_vif);
    }

    wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
#endif

    return 0;
}

static void
vm_cfg80211_stop_p2p_device(struct wiphy *wiphy, struct wireless_dev *wdev)
{
#ifdef CONFIG_P2P //TODO
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wdev);
    struct wifi_mac_p2p *p2p = &pwdev_priv->p2p;

    pr_debug("enter %s\n", __func__);
    if (vm_p2p_enabled(p2p))
        vm_p2p_down(wnet_vif);
    wifi_mac_top_sm(wnet_vif, WIFINET_S_INIT,0);
#endif
}

static int
vm_cfg80211_set_mcast_rate(struct wiphy *wiphy, struct net_device *dev,
                            int rate[IEEE80211_NUM_BANDS])
{
    ERROR_DEBUG_OUT("no support yet\n");
    return -1;
}

#if 0
static int
vm_cfg80211_start_radar_detection(struct wiphy *wiphy, struct net_device *dev,
                                  struct cfg80211_chan_def *chandef, unsigned int cac_time_ms)
{
    pr_debug("no support yet %s\n", __func__);
    return -1;
}
#endif

static int
vm_cfg80211_set_coalesce(struct wiphy *wiphy, struct cfg80211_coalesce *coalesce)
{
    ERROR_DEBUG_OUT("no support yet \n");
    return -1;
}

static int
vm_cfg80211_channel_switch(struct wiphy *wiphy, struct net_device *dev,
                           struct cfg80211_csa_settings *params)
{
    ERROR_DEBUG_OUT("no support yet \n");
    return -1;
}

static int
vm_cfg80211_get_antenna(struct wiphy *wiphy, int radio_idx, unsigned int *tx_ant, unsigned int *rx_ant)
{
    ERROR_DEBUG_OUT("no support yet \n");
    return -1;
}
#if 0
static int
vm_cfg80211_auth(struct wiphy *wiphy, struct net_device *dev,
                    struct cfg80211_auth_request *req)
{
    int rc = -EINVAL;
    int auth_value = WIFINET_AUTH_NONE;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    pr_debug("-<running> %s %d\n",__func__,__LINE__);
    pr_debug("----auth_type:%d\n", req->auth_type);
    switch (req->auth_type)
    {
        case NL80211_AUTHTYPE_OPEN_SYSTEM:
            wnet_vif->vm_flags &= ~(WIFINET_F_WPA|WIFINET_F_PRIVACY);
            auth_value = WIFINET_AUTH_OPEN;
            break;
        case NL80211_AUTHTYPE_SHARED_KEY:
            wnet_vif->vm_flags &= ~WIFINET_F_WPA;
            wnet_vif->vm_flags |= WIFINET_F_PRIVACY;
            auth_value = WIFINET_AUTH_SHARED;
            break;
        case NL80211_AUTHTYPE_FT:
            break;
        case NL80211_AUTHTYPE_NETWORK_EAP:
            wnet_vif->vm_flags &= ~WIFINET_F_WPA;
            wnet_vif->vm_flags |= WIFINET_F_PRIVACY;
            auth_value = WIFINET_AUTH_8021X;
            break;
        case NL80211_AUTHTYPE_SAE:
            //used by mesh
            break;
        case NL80211_AUTHTYPE_AUTOMATIC:
            auth_value = WIFINET_AUTH_AUTO;
            break;
        default:
            pr_err("<running> %s %d, unknown auth type\n",__func__,__LINE__);
            break;
    }
    wnet_vif->vm_mainsta->sta_authmode = auth_value;
    pr_debug("----sta_authmode:%d\n", wnet_vif->vm_mainsta->sta_authmode);

    return rc;
}
#endif
static int
vm_cfg80211_set_cqm_txe_cfg(struct wiphy *wiphy,struct net_device *dev,
                               unsigned int rate, unsigned int pkts, unsigned int intvl)
{
//TODO
    ERROR_DEBUG_OUT("not support yet \n");
    return -1;
}

static int
vm_cfg80211_set_monitor_channel(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_chan_def *chandef)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);

    if (chandef == NULL) {
        ERROR_DEBUG_OUT("chandef is null at\n");
        return -1;
    }
    pr_debug("---enter %s\n", __func__);
    chan_dbg(wnet_vif->vm_wmac->wm_curchan, "x4", 4863);
    return 0;
}
static int
vm_cfg80211_set_cqm_rssi_cfg(struct wiphy *wiphy,
					 struct net_device *dev,
					 int rssi_thold, unsigned int rssi_hyst)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);

    pr_debug("---enter %s\n", __func__);
    wnet_vif->vm_wmac->drv_priv->drv_config.cqm_rssi_thold = rssi_thold;
    wnet_vif->vm_wmac->drv_priv->drv_config.cqm_rssi_hyst = rssi_hyst;
    pr_debug("---rssi_thold:%d, rssi_hyst:%d\n", rssi_thold, rssi_hyst);

    return 0;
}

#if (KERNEL_VERSION(4, 17, 0) <= LINUX_VERSION_CODE) || (defined WPA3_PATCH)
static int vm_cfg80211_external_auth(struct wiphy *wiphy, struct net_device *dev,
    struct cfg80211_external_auth_params *params)
{
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_station *sta;

    pr_debug("%s vid:%d, action:%d, status:%d\n", __func__, wnet_vif->wnet_vif_id, params->action, params->status);
    if (params->status == WLAN_STATUS_SUCCESS) {
        pr_debug("bssid: "MAC_FMT"", MAC_ARG(params->bssid));
        pr_debug("SSID: [%s]\n", ((params->ssid.ssid_len == 0) ? "" : (char *)params->ssid.ssid));
        pr_debug("suite: 0x%08x\n", params->key_mgmt_suite);

        if (wnet_vif->vm_opmode == WIFINET_M_STA) {
            wifi_mac_top_sm(wnet_vif, WIFINET_S_ASSOC, 0);

        } else {
            sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, params->bssid, wnet_vif->wnet_vif_id);
            if (sta != NULL) {
                pr_debug("auth sta:%p\n", sta);
                wifi_mac_sta_auth(sta);
            }
        }
    }

    return 0;
}
#endif

/*Modify order of the interfaces according to the definition in linux kernel
** those commented were not supported currently in our driver.
*/
static struct cfg80211_ops vm_cfg80211_ops =
{
    .suspend = vm_cfg80211_suspend,
    .resume = vm_cfg80211_resume,

    .add_virtual_intf = vm_cfg80211_add_vif,
    .del_virtual_intf = vm_cfg80211_del_vif,
    .change_virtual_intf = vm_cfg80211_change_vif,
    .add_key = vm_cfg80211_add_key,
    .get_key = vm_cfg80211_get_key,
    .del_key = vm_cfg80211_del_key,
    .set_default_key = vm_cfg80211_config_default_key,
    .set_default_mgmt_key = vm_cfg80211_set_default_mgmt_key,
    .start_ap = vm_cfg80211_start_ap,
    .change_beacon = vm_cfg80211_change_beacon,
    .stop_ap = vm_cfg80211_stop_ap,
    .add_station = vm_cfg80211_add_station,
    .del_station = vm_cfg80211_del_station,
    .change_station = vm_cfg80211_change_station,
    .get_station = vm_cfg80211_get_station,
    .dump_station = vm_cfg80211_dump_station,//same to get station
    .change_bss = vm_cfg80211_change_bss,
    .set_txq_params = vm_cfg80211_set_txq_params,
    .set_monitor_channel = vm_cfg80211_set_monitor_channel,

    .scan = vm_cfg80211_scan,
    .assoc = vm_cfg80211_assoc,
    .connect = vm_cfg80211_connect,
    .disconnect = vm_cfg80211_disconnect,
    .join_ibss = vm_cfg80211_join_ibss,
    .leave_ibss = vm_cfg80211_leave_ibss,
    .set_mcast_rate = vm_cfg80211_set_mcast_rate,
    .set_wiphy_params = vm_cfg80211_set_wiphy_params,
    .set_tx_power = vm_cfg80211_set_tx_power,
    .get_tx_power = vm_cfg80211_get_tx_power,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    .set_wds_peer = vm_cfg80211_set_wds_peer,
#endif

    .set_bitrate_mask = vm_cfg80211_set_bitrate_mask,
    .set_pmksa = aml_cfg80211_set_pmksa,
    .del_pmksa = aml_cfg80211_del_pmksa,
    .flush_pmksa = aml_cfg80211_flush_pmksa,
    .remain_on_channel = vm_cfg80211_remain_on_channel,
    .cancel_remain_on_channel = vm_cfg80211_cancel_remain_on_channel,
    .mgmt_tx = vm_cfg80211_mgmt_tx,

    .set_power_mgmt = vm_cfg80211_set_power_mgmt,
    .set_cqm_rssi_config = vm_cfg80211_set_cqm_rssi_cfg,
    .set_cqm_txe_config = vm_cfg80211_set_cqm_txe_cfg,
    .set_antenna = vm_cfg80211_set_antenna,
    .get_antenna = vm_cfg80211_get_antenna,
    .sched_scan_stop = vm_cfg80211_sched_scan_stop,
    .set_rekey_data = vm_cfg80211_set_rekey_data,
    .set_noack_map = vm_cfg80211_set_noack_map,
    .get_channel = vm_cfg80211_get_channel,
    .start_p2p_device = vm_cfg80211_start_p2p_device,
    .stop_p2p_device = vm_cfg80211_stop_p2p_device,
    .set_coalesce = vm_cfg80211_set_coalesce,
    .channel_switch = vm_cfg80211_channel_switch,
    .set_qos_map = vm_cfg80211_set_qos_map,
#if (KERNEL_VERSION(4, 17, 0) <= LINUX_VERSION_CODE) || (defined WPA3_PATCH)
    .external_auth = vm_cfg80211_external_auth,
#endif
};

/*
* Added by Xuexing.yang@ for enable VHT features
* Date: 06/05 2017
*
*/
static void vm_cfg80211_init_vht_capab(struct ieee80211_sta_vht_cap *vht_cap, enum ieee80211_band band)
{
    pr_debug("%s(%d)\n",__func__,__LINE__);

    vht_cap->vht_supported = true;
    #if 0 /*Original added from RTL view, interpreted into kernel language*/
    vht_cap->cap =
    (3 << 0) | // MaxMPDU Len: 0/1/2/3 is 3895/7991/11454/reserved octets
    (0 << 2) | // ChanWidth: 0 is only80, 1 is 160, 2 is 160+8080, 3 is reserved
    (0 << 4) | // RxLDPC   : 0 is not support, 1 is support
    (1 << 5) | // ShortGI_for_80: 1 is support
    (0 << 6) | // ShortGI 160&8080: 0 is NOT support
    (0 << 7) | // Tx STBC : 0 is no supported
    (1 << 8) | // Rx STBC : 0 is no supported , 1 is one sps, 2/3/4 is 2-4 sps. 5/6/7 is reserved.
    (0 << 11) | // SU beamformer:
    (1 << 12) | // SU Beamformee:
    (1 << 13) | // Beamformee STS Capability: Rx SPS -1
    (0 << 16) | // Number of sounding dimensions
    (0 << 19) | // MU beamformer Capable
    (0 << 20) | // MU beamformee Capable
    (0 << 21) | // VHT TXOP PS
    (0 << 22) | // HTC-VHT Capable: variable HT control field
    (7 << 23) | // Max AMPDU length Exponent
    (0 << 26) | // VHT link Adaptation Capable
    (0 << 28) | // Rx Antenna Pattern Consistency
    (0 << 29) | // Tx Antenna Pattern Consistency
    (0 << 30);  // Reserved.
    #endif

    vht_cap->cap =  IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895 | IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991 |
                    IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 | IEEE80211_VHT_CAP_RXLDPC| IEEE80211_VHT_CAP_SHORT_GI_80 |
                    IEEE80211_VHT_CAP_RXSTBC_1 | IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
                    IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK | IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;

    vht_cap->vht_mcs.tx_highest = 390;
    vht_cap->vht_mcs.rx_highest = 390;
    vht_cap->vht_mcs.tx_mcs_map = IEEE80211_VHT_MCS_SUPPORT_0_9;// nss1 0-9
    vht_cap->vht_mcs.rx_mcs_map = IEEE80211_VHT_MCS_SUPPORT_0_9;// nss1 0-9

}
/*End of adding*/

static void vm_cfg80211_init_ht_capab(struct ieee80211_sta_ht_cap *ht_cap, enum ieee80211_band band)
{
    pr_debug("%s(%d)\n",__func__,__LINE__);

    ht_cap->ht_supported = true;

    ht_cap->cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
                  IEEE80211_HT_CAP_SGI_40 | IEEE80211_HT_CAP_SGI_20 |
                  IEEE80211_HT_CAP_LDPC_CODING | IEEE80211_HT_CAP_RX_STBC |
                  IEEE80211_HT_CAP_DSSSCCK40 | IEEE80211_HT_CAP_MAX_AMSDU;

    ht_cap->ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
    ht_cap->ampdu_density = IEEE80211_HT_MPDU_DENSITY_16;
    ht_cap->mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;

    ht_cap->mcs.rx_mask[0] = 0xFF;
    ht_cap->mcs.rx_mask[1] = 0x00;
    ht_cap->mcs.rx_mask[4] = 0x01;

    ht_cap->mcs.rx_highest = MAX_BIT_RATE_40MHZ_MCS7;
}

/**
 * @brief get the event id of the events array
 *
 * @param event     vendor event
 *
 * @return    index of events array
 */
static int
vm_get_event_id(int event)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(vendor_events); i++) {
		if (vendor_events[i].subcmd == event)
			return i;
	}

	return event_max;
}

/**
 * @brief vendor command to set parameters through nl80211
 *      to communicate with iw application
 *
 * @param wiphy       A pointer to wiphy struct
 * @param wdev     A pointer to wireless_dev struct
 * @param data     a pointer to data
 * @param  len     data length
 *
 * @return     0: success  1: fail
 * @Author     Boatman Yang(xuexing.yang@amlogic.com)
 * @Date       05/10 2017
 */
extern struct sts_cfg_data g_sts_cfg;

unsigned int irr_para_reg_addr[] ={
    RG_RECV_A3,
    RG_RECV_A4,
    RG_RECV_A5,
    RG_RECV_A6,
    RG_RECV_A7,
    RG_RECV_A8,
    RG_RECV_A9,
    RG_RECV_A10
};

extern void print_driver_version(void);
extern unsigned int efuse_manual_read(unsigned int addr);
char *rssi_result_path = "/vendor/etc/rssi.txt";
char *reg_result_path = "/data/reg.txt";

static void record_reg_value(unsigned int address, unsigned int value)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    struct kstat stat;
    mm_segment_t fs;
    int error = 0;
    struct file *fp;
    char buf[512] = {0};
    fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open(reg_result_path, O_CREAT|O_RDWR, 0644);

    if (!IS_ERR(fp)) {
            sprintf(buf, "address: 0x%08x 0x%08x\r\n",
                address,
                value);
            error = vfs_stat(reg_result_path, &stat);
            if (error) {
                filp_close(fp, NULL);
                goto err;
            }

        fp->f_pos = (int)stat.size;
        if ((int)stat.size < 0) {
            filp_close(fp, NULL);
            goto err;
        }

            vfs_write(fp, buf, strlen(buf), &fp->f_pos);
        filp_close(fp, NULL);
    }
    else {
        ERROR_DEBUG_OUT("open file %s failed.\n", reg_result_path);
    }
err:
    set_fs(fs);
#endif
}

static int reply_get_efuse_data(struct wiphy *wiphy, unsigned int offset) {
    struct sk_buff *reply;
    unsigned int data;

    data = efuse_manual_read(offset);

    pr_debug("efuse addr:%08x, efuse data is :%08x\n", offset, data);

    reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(data));
    if (!reply)
        return -ENOMEM;

    /* We use offset as attribute value. It is easiest way for
     * userspace to parse response. */
    if (nla_put_u32(reply, offset, data)) {
        kfree_skb(reply);
        return -ENOBUFS;
    }

    return cfg80211_vendor_cmd_reply(reply);
}

int vm_cfg80211_vnd_cmd_set_para(struct wiphy *wiphy, struct wireless_dev *wdev, const void *data, int data_len)
{
    int ret = 0;
    unsigned char opcode = 0;
    unsigned int usr_data_arr[8] = {0};
    unsigned int usr_data = 0;
    unsigned int usr_data_ext = 0; //for complex cmd use, for example register write
    unsigned int reg_val = 0;
    unsigned char  vmc_tmp_addr[6]= {0x00,0x01,0x02,0x03,0x04,0x05};
    unsigned char* opt_data = NULL;
    struct hw_interface *hif = hif_get_hw_interface();

    union vendor_if *vnd_if = NULL;
    struct statistic_cmd *vnd_cmd = NULL;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct net_device *netdev = wnet_vif->vm_ndev;

    int frame_num;
    int frame_len;
    unsigned int val_con = 0;
    struct _iwVendorCmdStruct *iwVendorCmd = (struct _iwVendorCmdStruct *)data;
    unsigned int i = 0;
    unsigned int reg = 0;
    unsigned int efuse_data = 0;

    if (!data)
        return -EINVAL;

    vnd_cmd = (struct statistic_cmd *)data;
    opt_data = (unsigned char *)data;
    vnd_if = (union vendor_if *)data;

    pr_debug("%s cmd 0x%x, addr 0x%x, data 0x%x\n", __func__,
        (vnd_if->vnd_reg_cmd.cmd), ntohl(vnd_if->vnd_reg_cmd.addr), ntohl(vnd_if->vnd_reg_cmd.data));

    if (data_len > 0) {
        opcode = iwVendorCmd->first_eight_byte & 0x000000ff;
        switch(data_len) {
            case 1:
                pr_debug("---Aml drv: only opcode.\n");
                break;

            case 2:
                usr_data_arr[0] = iwVendorCmd->first_eight_byte>>8 & 0x000000ff;
                usr_data = usr_data_arr[0];
                break;

            case 3:
                usr_data_arr[0] = iwVendorCmd->first_eight_byte>>8 & 0x000000ff;
                usr_data_arr[1] = iwVendorCmd->first_eight_byte>>16 & 0x000000ff;
                usr_data = usr_data_arr[0]<<8 | usr_data_arr[1];
                break;

            case 4:
                usr_data_arr[0] = iwVendorCmd->first_eight_byte>>8 & 0x000000ff;
                usr_data_arr[1] = iwVendorCmd->first_eight_byte>>16 & 0x000000ff;
                usr_data_arr[2] = iwVendorCmd->first_eight_byte>>24 & 0x000000ff;
                usr_data = usr_data_arr[0] << 16 | usr_data_arr[1] << 8 | usr_data_arr[2];
                break;

            case 5:
                usr_data_arr[0] = iwVendorCmd->first_eight_byte>>8 & 0x000000ff;
                usr_data_arr[1] = iwVendorCmd->first_eight_byte>>16 & 0x000000ff;
                usr_data_arr[2] = iwVendorCmd->first_eight_byte>>24 & 0x000000ff;
                usr_data_arr[3] = iwVendorCmd->second_eight_byte & 0x000000ff;
                usr_data = usr_data_arr[0] << 24 | usr_data_arr[1] << 16 | usr_data_arr[2] << 8 | usr_data_arr[3];
                break;

            case 7: //write vmac macaddress
                vmc_tmp_addr[0] = iwVendorCmd->first_eight_byte>>8 & 0x000000ff;
                vmc_tmp_addr[1] = iwVendorCmd->first_eight_byte>>16 & 0x000000ff;
                vmc_tmp_addr[2] = iwVendorCmd->first_eight_byte>>24 & 0x000000ff;
                vmc_tmp_addr[3] = iwVendorCmd->second_eight_byte & 0x000000ff;
                vmc_tmp_addr[4] = iwVendorCmd->second_eight_byte>>8 & 0x000000ff;
                vmc_tmp_addr[5] = iwVendorCmd->second_eight_byte>>16 & 0x000000ff;
                break;

            case 9:
                usr_data_arr[0] = iwVendorCmd->first_eight_byte>>8 & 0x000000ff;
                usr_data_arr[1] = iwVendorCmd->first_eight_byte>>16 & 0x000000ff;
                usr_data_arr[2] = iwVendorCmd->first_eight_byte>>24 & 0x000000ff;
                usr_data_arr[3] = iwVendorCmd->second_eight_byte & 0x000000ff;
                usr_data_arr[4] = iwVendorCmd->second_eight_byte>>8 & 0x000000ff;
                usr_data_arr[5] = iwVendorCmd->second_eight_byte>>16 & 0x000000ff;
                usr_data_arr[6] = iwVendorCmd->second_eight_byte>>24 & 0x000000ff;
                usr_data_arr[7] = iwVendorCmd->last_byte;
                usr_data = usr_data_arr[0] <<24 | usr_data_arr[1] <<16 | usr_data_arr[2] <<8 | usr_data_arr[3];
                usr_data_ext = usr_data_arr[4] <<24 | usr_data_arr[5] <<16 | usr_data_arr[6] <<8 | usr_data_arr[7];
                break;

            default:
                break;
        }
    }

    switch(opt_data[0]) {
        case VM_NL80211_VENDER_SUBCMD_AMSDU:
            if (!usr_data) {
                wifimac->wm_flags_ext &= ~WIFINET_FEXT_AMSDU;
                wifi_mac_config(wifimac, CHIP_PARAM_AMSDU_ENABLE, 0);

            } else {
                wifimac->wm_flags_ext |= WIFINET_FEXT_AMSDU;
                wifi_mac_config(wifimac, CHIP_PARAM_AMSDU_ENABLE, 1);
            }
            pr_debug("Cfg80211: Set amsdu done:%d\n",usr_data);
            break;

        case VM_NL80211_VENDER_SUBCMD_AMPDU:
            if (usr_data) {
                if (wifimac->wm_caps &WIFINET_C_HT) {
                    wifi_macwnet_vif_set_extflag(wnet_vif, WIFINET_FEXT_AMPDU);
                    wifi_mac_config(wifimac, CHIP_PARAM_AMPDU, 1);

                } else {
                    ERROR_DEBUG_OUT("wifi have no cap to support HT mode");
                }

            } else {
                wifi_macwnet_vif_clear_extflag(wnet_vif, WIFINET_FEXT_AMPDU);
                wifi_mac_config(wifimac, CHIP_PARAM_AMPDU, 0);
            }

            if (IS_RUNNING(netdev)) {
                wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
            }
            pr_debug("Cfg80211: Set ampdu done:%d\n",usr_data);
            break;

        case VM_NL80211_VENDER_SUBCMD_B2BNCTYPE:
            pr_debug("Cfg80211: Set encryption type done:%d\n",usr_data);
            gB2BTestCasePacket.encryp_type = usr_data;
            break;

        case VM_NL80211_VENDER_SUBCMD_B2BTKIPMIC:
            pr_debug("Cfg80211: Set tkip mic done:%d\n",usr_data);
            gB2BTestCasePacket.tkip_mic = usr_data;
            break;

        case VM_NL80211_VENDER_SUBCMD_B2BTCPCSUM:
            pr_debug("Cfg80211: Set tcp checksum done:%d\n",usr_data);
            gB2BTestCasePacket.tcpip_csum = usr_data;
            break;

        case VM_NL80211_VENDER_SUBCMD_B2BPKTSNUM:
            pr_debug("Cfg80211: Set send packets numbers done:%d\n",usr_data);
            gB2BTestCasePacket.send_frame_num = usr_data;
            break;

        case VM_NL80211_VENDER_SUBCMD_B2BTESTYPE:
            pr_debug("Cfg80211: To be send data packets and test type :%d\n",usr_data);
            pr_debug("Cfg80211: Please make sure set bssid mac address as the same with dest.\n");
            gB2BTestCasePacket.packet_type = usr_data;
            pr_debug("Cfg80211: Before calling prepare_test_hal_layer_thr_init.\n");
            prepare_test_hal_layer_thr_init(usr_data);
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_LOCAL_MAC_ADDR:
            memcpy(TrcConfMib.the_mac_address,vmc_tmp_addr,6);
            local_mac_addr_update();
            pr_debug("Cfg80211: Set local mac addr done.\n");
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_GET_LOCAL_MAC_ADDR:
            pr_debug("Cfg80211: local wnet_vif addr: "MAC_FMT"\n",MAC_ARG(TrcConfMib.the_mac_address));
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_PEER_MAC_ADDR:
            memcpy(TrcConfMib.the_desc_address,vmc_tmp_addr,6);
            memcpy(TrcConfMib.the_connect_address,vmc_tmp_addr,6);
            dst_mac_addr_update();
            pr_debug("Cfg80211: Set peer mac addr done.\n");
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_GET_PEER_MAC_ADDR:
            pr_debug("Cfg80211:  dest wnet_vif addr: "MAC_FMT"\n", MAC_ARG(TrcConfMib.the_desc_address));
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_BSSID_MAC_ADDR:
            memcpy(TrcConfMib.the_bssid,vmc_tmp_addr,6);
            pr_debug("Cfg80211: Set bssid mac addr done.\n");
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_GET_BSSID_MAC_ADDR:
            pr_debug("Cfg80211: local wnet_vif addr: "MAC_FMT"\n", MAC_ARG(TrcConfMib.the_mac_address));
            pr_debug("Cfg80211: dest wnet_vif addr: "MAC_FMT"\n", MAC_ARG(TrcConfMib.the_desc_address));
            pr_debug("Cfg80211: connect addr: "MAC_FMT"\n", MAC_ARG(TrcConfMib.the_connect_address));
            pr_debug("Cfg80211: bssid : "MAC_FMT"\n", MAC_ARG(TrcConfMib.the_bssid));
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_SET_CHL:
            {
                struct hal_channel hchan;

                gB2BTestCasePacket.channel = vnd_if->vnd_chn_bw_cmd.chn;
                hchan.cchan_num = vnd_if->vnd_chn_bw_cmd.chn;
                hchan.pchan_num = vnd_if->vnd_chn_bw_cmd.chn;
                hchan.chan_bw = vnd_if->vnd_chn_bw_cmd.bw;

                pr_debug("Cfg80211: set channel to %d. bw =%d\n",vnd_if->vnd_chn_bw_cmd.chn, vnd_if->vnd_chn_bw_cmd.bw);

                phy_set_rf_chan(&hchan, 0, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);
                break;
            }

        case VM_NL80211_VENDER_SUBCMD_DRV_B2B_PKT_LENGTH:
            gB2BTestCasePacket.pkt_length = usr_data;
            pr_debug("Cfg80211: set pkt_len(mpdu_len) = %d uninvolved (fcs,delimiter)\n",usr_data);
            break;

        case VM_NL80211_VENDER_SUBCMD_DRV_SET_SHORT_GI:
            if (aml_wifi_is_enable_rf_test())
                gB2BTestCasePacket.if_shortGI = usr_data;

            if (1 == usr_data) {
                wifimac->wm_flags_ext |= WIFINET_FEXT_SHORTGI_ENABLE;
                pr_debug("Cfg80211: enable short GI done");

            } else {
                wifimac->wm_flags_ext &= ~WIFINET_FEXT_SHORTGI_ENABLE;
                pr_debug("Cfg80211: disable short GI done.\n");
            }

            if (1 == usr_data)
                pr_debug("Cfg80211: enable short GI done.\n");
            else
                pr_debug("Cfg80211: disable short GI done.\n");
            break;

        case VM_NL80211_VENDER_SUBCMD_B2BCHLBW:
            pr_debug("Cfg80211: Set channel bandwidth done:%d\n",usr_data);
            if (aml_wifi_is_enable_rf_test())
                gB2BTestCasePacket.channel_bw = usr_data;

            if (usr_data == 2) {
                pr_debug("\n----: 80M bw\n");
            } else if (usr_data == 1) {
                pr_debug("\n----: 40M bw\n");
            } else {
                pr_debug("\n-----: 20M bw\n");
            }
            break;

        case VM_NL80211_VENDER_SUBCMD_GETREG:
            if (((usr_data >> 24) & 0xff) == 0xff ) {
#ifdef USE_T902X_RF
                reg_val = rf_i2c_read(usr_data & 0x00ffffff); //access t902x rf reg
#endif
            } else if (((usr_data >> 24) & 0xf0) == 0xf0 ) {
#ifdef USE_T902X_RF
                reg_val = rf_i2c_read(usr_data & 0xffffffff); //access t902x rf reg
#endif
            } else {
                reg_val = wnet_vif->vif_ops.read_word(usr_data);
            }
            pr_debug("Drv info: Reg data=&0x%08x\n",reg_val);//do not remove me!!!!!
            record_reg_value(usr_data, reg_val);
            break;

        case VM_NL80211_VENDER_SUBCMD_SETREG:
            pr_debug("Cfg80211: Reg addr: val:0x%08x,val:0x%08x\n",usr_data,usr_data_ext);
            if (((usr_data >> 24) & 0xff) == 0xff) {
#ifdef USE_T902X_RF
                rf_i2c_write( usr_data & 0x00ffffff,usr_data_ext );//access t902x rf reg
#endif

            } else if (((usr_data >> 24) & 0xf0) == 0xf0) {
#ifdef USE_T902X_RF
                rf_i2c_write( usr_data & 0xffffffff,usr_data_ext );//access t902x rf reg
#endif
            } else {
                wnet_vif->vif_ops.write_word(usr_data , usr_data_ext);
            }
            break;

    case VM_NL80211_VENDER_I2C_GETREG:
#ifdef USE_T902X_RF 
        reg_val = rf_i2c_read(usr_data);
#endif

        pr_debug("[I2C read]: Reg data=&0x%08x\n",reg_val);
        break;

    case VM_NL80211_VENDER_I2C_SETREG:
#ifdef USE_T902X_RF
        rf_i2c_write(usr_data,usr_data_ext);
#endif

        pr_debug("[I2C write] I2c addr: 0x%08x, data:0x%08x\n",usr_data,usr_data_ext);
        break;

     case VM_NL80211_T9026_DUMP_RXIRR_REG:
       batch_dump_reg(wiphy, irr_para_reg_addr, 8);
     break;


    case VM_NL80211_PHY_STATISTIC:
        wifimac->drv_priv->drv_ops.phy_stc();
        break;
    case VM_NL80211_CCA_BUSY_CHECK:
        wifimac->drv_priv->drv_ops.cca_busy_check();
        break;

    case VM_NL80211_VENDER_DBG_INFO_ENABLE:
        g_dbg_info_enable = (unsigned long long)usr_data;
        pr_debug("%s(%d)dbg_en=0x%08llx\n ", __func__, __LINE__, g_dbg_info_enable);
        break;
    case VM_NL80211_VENDER_DYNAMIC_BW_CFG:
        if( usr_data != 0 ){
            wifi_mac_config(wifimac, CHIP_PARAM_DYNAMIC_BW, 1);
            pr_debug("enable dynamic bandwidth\n");
        }else{
            wifi_mac_config(wifimac, CHIP_PARAM_DYNAMIC_BW, 0);
            pr_debug("disable dynamic bandwidth\n");
        }

        break;

    case VM_NL80211_VENDER_BCN_INTERVAL:
        /* change firmware bcn intvl, JUST for power save test. */
        {
            int bcn_intvl = usr_data * 100;
            unsigned int regdata = 0;

            if (bcn_intvl == 0)
                bcn_intvl = WIFINET_BINTVAL_DEFAULT;
            pr_debug("%s:%d, bcn intvl %d\n", __func__, __LINE__, bcn_intvl);
            wifimac->drv_priv->drv_ops.Phy_beaconinit(wifimac->drv_priv,
                wnet_vif->wnet_vif_id, bcn_intvl);

            /* change max sleep time */
            if (usr_data != 0)
            {
                regdata = wnet_vif->vif_ops.read_word(RG_AON_A37);
                regdata &= ~0x00ff0000;
                regdata |= usr_data << 16;
                wnet_vif->vif_ops.write_word(RG_AON_A37, regdata);
            }
        }
        break;
    case VM_NL80211_VENDOR_GET_STA_RSSI_NOISE:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
        if (wnet_vif->vm_state == WIFINET_S_CONNECTED) {
            struct file *fp;
            struct kstat stat;
            mm_segment_t fs;
            int error = 0;
            char buf[512] = {0};
            unsigned int arr[8] = {0};

            fs = get_fs();
            set_fs(KERNEL_DS);
            fp = filp_open(rssi_result_path, O_CREAT|O_RDWR, 0644);

            if (!IS_ERR(fp)) {
                 if (wnet_vif->vm_mainsta != NULL) {
                    get_phy_stc_info(arr);

                    sprintf(buf, "att:%d, sta_avg_rssi:%d, sta_avg_bcn_rssi:%d, CP1:%d, avg_snr:%d, ",
                        opt_data[1], wnet_vif->vm_mainsta->sta_avg_rssi - 256,
                        wnet_vif->vm_mainsta->sta_avg_bcn_rssi,
                        arr[0], arr[1]);
                    sprintf(&buf[strlen(buf)], "snr_qdb:%d crc_err:%d, crc_ok:%d, noise_f:%d\r\n",
                        arr[5], arr[2], arr[3], arr[4]);

                    error = vfs_stat(rssi_result_path, &stat);
                    if (error) {
                        filp_close(fp, NULL);
                        goto err;
                    }

                    fp->f_pos = (int)stat.size;
                    if ((int)stat.size < 0) {
                        filp_close(fp, NULL);
                        goto err;
                    }
                    vfs_write(fp, buf, strlen(buf), &fp->f_pos);
                }
                filp_close(fp, NULL);
            }
            else {
                ERROR_DEBUG_OUT("open file %s failed.\n", rssi_result_path);
            }
err:
            set_fs(fs);
        }
#endif
        break;

    case VM_NL80211_VENDOR_UPDATE_WIPHY_PARAMS:
        pr_debug("Cfg80211: update_wiphy_params\n");
        pr_debug("Cfg80211: wiphy addr: 0x%p----\n", wiphy);
        vm_cfg80211_update_wiphy_params(wiphy);
        break;
    case VM_NL80211_VENDOR_SET_PREAMBLE_TYPE:
        pr_debug("Cfg80211: set preamble type\n");
        phy_set_preamble_type(usr_data);
        break;
    case VM_NL80211_VENDOR_SET_BURST:
        pr_debug("Cfg80211: set burst\n");
        wifimac->drv_priv->drv_config.cfg_burst_ack = usr_data;
        break;
    case VM_NL80211_VENDOR_SET_ACK_POLICY:
        pr_debug("Cfg80211: set ack policy\n");
        wifimac->drv_priv->drv_config.ack_policy = usr_data;
        break;
    case VM_NL80211_SENDTEST:
        frame_num = usr_data >> 16;
        frame_len = usr_data & 0xffff;
        pr_debug("Cfg80211: frame_num =%d,frame_len=%d\n", frame_num, frame_len);
        do
        {
            wifi_mac_send_testdata(wnet_vif->vm_mainsta,  frame_len);
            msleep(1);
        }
        while(frame_num--);
        break;

    case VM_NL80211_BT_REG_READ:
        reg_val = wnet_vif->vif_ops.bt_read_word(usr_data);
        pr_debug("Read BT Addr = 0x%x,data = 0x%x\n",usr_data,reg_val);
        break;

    case VM_NL80211_BT_REG_WRITE:
        wnet_vif->vif_ops.bt_write_word(usr_data,usr_data_ext);
        pr_debug("Write BT Addr = 0x%x,data = 0x%x\n",usr_data,usr_data_ext);
        break;

    case VM_NL80211_SET_LDPC:
        if( 1 == usr_data )
        {
            if (aml_wifi_is_enable_rf_test()) {
                gB2BTestCasePacket.ldpc_enable = 1;
                pr_debug("Enable tx ldpc\n");
            }

            wnet_vif->vm_mainsta->sta_vhtcap |= WIFINET_VHTCAP_RX_LDPC;
            wifimac->wm_flags |=WIFINET_F_LDPC;
            pr_debug("Enable ldpc, if need change,the action must be excute before connect ap or create ap\n");
        }
        else
        {
            if (aml_wifi_is_enable_rf_test()) {
                gB2BTestCasePacket.ldpc_enable = 0;
                pr_debug("Disable tx ldpc\n");
            }

            wnet_vif->vm_mainsta->sta_vhtcap &= ~WIFINET_VHTCAP_RX_LDPC;
            wifimac->wm_flags &=~WIFINET_F_LDPC;
            pr_debug("Disable ldpc, if need change,the action must be excute before connect ap or create ap\n");
        }
        break;

    case VM_NL80211_VENDOR_ENABLE_AUTO_RATE:
        wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_NONE;
        wnet_vif->vm_change_rate_enable = 1;
        break;

    case VM_NL80211_VENDER_SYS_INFO_STATISTIC_DEFAULT_CFG:
        sts_default_cfg(&g_sts_cfg, 0);
        break;

    case VM_NL80211_VENDER_SYS_INFO_STATISTIC_UPDATE_CFG:
        sts_update_cfg(&g_sts_cfg,data, data_len);
        break;

    case VM_NL80211_VENDER_SYS_INFO_STATISTIC_OPT:
        sts_opt_by_cfg(&g_sts_cfg, opt_data[1]);
        break;

    case VM_NL80211_VENDER_STS_BY_LAYER:
        memcpy((unsigned char*)&val_con, (unsigned char*)&opt_data[1], sizeof(unsigned int));
        wifimac->drv_priv->net_ops->wifi_mac_get_sts(wifimac, 0, ntohl(val_con));
        break;

    case VM_NL80211_VENDER_PT_RX_START:
        wnet_vif->vif_ops.pt_rx_start(opt_data[1]);
        break;

    case VM_NL80211_VENDER_PT_RX_STOP:
        wnet_vif->vif_ops.pt_rx_stop();
        break;

#ifdef WIFI_CAPTURE
    case VM_NL80211_SET_CAP_GAIN:
        pr_debug("set gain = 0x%08x\n",ntohl(vnd_if->vnd_com_cmd.data));
        dut_set_gain(ntohl(vnd_if->vnd_com_cmd.data));
        break;

   case VM_NL80211_SET_BCAP_NAME:
        pr_debug("bcap name = 0x%08x\n",ntohl(vnd_if->vnd_com_cmd.data));
        dut_set_bcap_log_name(ntohl(vnd_if->vnd_com_cmd.data));
       break;

    case VM_NL80211_VDR_SUBCMD_START_CAPTURE:
        pr_debug("Drv info: Reg addr=0x%08x\n",ntohl(vnd_if->vnd_com_cmd.data));
        dut_start_capture(ntohl(vnd_if->vnd_com_cmd.data));
        break;

    case VM_NL80211_VDR_SUBCMD_STOP_CAPTURE:
        pr_debug("Drv info: Reg addr=0x%08x\n",usr_data);
        dut_stop_capture();
        break;
#endif

    case VM_NL80211_POWER_SAVE_INTERVAL:
        if (wnet_vif->vm_pwrsave.ips_sta_psmode == WIFINET_PWRSAVE_NONE)
        {
            break;
        }
        wnet_vif->vm_pwrsave.ips_inactivitytime = (unsigned int)usr_data;

        os_timer_ex_start_period(&wnet_vif->vm_pwrsave.ips_timer_presleep, wnet_vif->vm_pwrsave.ips_inactivitytime);
        pr_debug("%s(%d) set ips_inactivitytime = 0x%08x\n ", __func__, __LINE__, wnet_vif->vm_pwrsave.ips_inactivitytime);
        break;

    case VM_NL80211_FETCH_PKT_METHOD:
        wnet_vif->vm_mainsta->sta_fetch_pkt_method = (unsigned char)usr_data;
        pr_debug("%s(%d) set sta_fetch_pkt_method = %d\n ", __func__, __LINE__, wnet_vif->vm_mainsta->sta_fetch_pkt_method);
        break;

    case VM_NL80211_PKT_FRAGMENT_THRESHOLD:
        if ((unsigned short)usr_data > 0) {
            wnet_vif->vm_fragthreshold = (unsigned short)usr_data;
        }
        pr_debug("%s(%d) set vm_fragthreshold = %d\n ", __func__, __LINE__, wnet_vif->vm_fragthreshold);
        break;

    case VM_NL80211_UAPSD_ENABLE:
        if ((unsigned char)usr_data != 0) {
            WIFINET_VMAC_UAPSD_ENABLE(wnet_vif);
            pr_debug("%s(%d) enable ap uapsd\n ", __func__, __LINE__);

        } else {
            WIFINET_VMAC_UAPSD_DISABLE(wnet_vif);
            pr_debug("%s(%d) disable ap uapsd\n ", __func__, __LINE__);
        }
        break;

    case VM_NL80211_CLK_MEASURE:
        host_clk_msr((unsigned char)(usr_data));
        pr_debug("%s(%d) host_clk_msr: %d\n ", __func__, __LINE__, (unsigned char)(usr_data));
        break;

    case VM_NL80211_AUTORATE_ENABLE:
        wnet_vif->vm_fixed_rate.rateinfo = 0;
        wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_NONE;
        wnet_vif->vm_change_rate_enable = 1;
        pr_debug("%s %d, enable autorate\n", __func__,__LINE__);
        break;

    case VM_NL80211_SDIO_DLY_ADJUST:
        pr_debug("sdio change timing %08x\n", usr_data);
        hif->hif_ops.hi_write_reg8(0x2c0, 0);

        switch (usr_data) {
            case 0:
                reg = 0x2c2;
                break;
            case 1:
                reg = 0x2c3;
                break;
            case 2:
                reg = 0x2c4;
                break;
            case 3:
                reg = 0x2c5;
                break;
            default:
                reg = 0x2c2;
                break;
        }

        pr_debug("debug reg:%08x, read:%08x\n", reg, (unsigned int)(hif->hif_ops.hi_read_reg8(reg)));
        for (i = 0; i < 32; ++i) {
            hif->hif_ops.hi_write_reg8(reg, i);

            pr_debug("0x0000b078 %08x, debug reg:%08x, i :%d, read:%08x\n",
                (unsigned int)(hif->hif_ops.hi_read_word(0x0000b078)), reg, i,  (unsigned int)(hif->hif_ops.hi_read_reg8(reg)));
            hif->hif_ops.hi_write_word(0x0000b078, 0x00000701 + i);
        }
        break;

#if defined(SU_BF) || defined(MU_BF)
    /* for beamforming test. remove it after finishing beamforming debug */
    case VM_NL80211_VENDER_BEAMFORMING:

        if ((usr_data & 0xFF) > 4)
        {
            pr_debug("support max spatial is 4 !\n");
            wifimac->max_spatial = 4;
        }
        else if ((usr_data & 0xFF) <= 0)
        {
            pr_debug("min spatial is 1 !\n");
            wifimac->max_spatial = 1;
        }
        else
            wifimac->max_spatial = usr_data & 0xFF;
        pr_debug("%s:%d, set spatial %d \n", __func__, __LINE__, wifimac->max_spatial);

        if ((((usr_data >> 8) & 0xFF) & BIT(0)) == BIT(0))
        {
            wifimac->wm_flags_ext2 |= WIFINET_VHTCAP_SU_BFMEE;
            pr_debug("%s:%d, enable su mimo\n", __func__, __LINE__);
        }
        else
        {
            wifimac->wm_flags_ext2 &= ~WIFINET_VHTCAP_SU_BFMEE;
            pr_debug("%s:%d, disable su mimo\n", __func__, __LINE__);
        }

        if ((((usr_data >> 8) & 0xFF) & BIT(1)) == BIT(1))
        {
            wifimac->wm_flags_ext2 |= WIFINET_VHTCAP_MU_BFMEE;
            pr_debug("%s:%d, enable mu mimo\n", __func__, __LINE__);
        }
        else
        {
            wifimac->wm_flags_ext2 &= ~WIFINET_VHTCAP_MU_BFMEE;
            pr_debug("%s:%d, disable mu mimo\n", __func__, __LINE__);
        }
        break;
#endif

#ifdef WIFI_CAPTURE
        case VM_NL80211_VDR_SUBCMD_BT_START_CAPTURE:
            pr_debug("Drv info: Reg addr=0x%08x\n",ntohl(vnd_if->vnd_com_cmd.data));
            dut_bt_start_capture(ntohl(vnd_if->vnd_com_cmd.data));
            break;

        case VM_NL80211_VDR_SUBCMD_BT_STOP_CAPTURE:
            pr_debug("Drv info: Reg addr=0x%08x\n",usr_data);
            dut_bt_stop_capture();
            break;
#endif

    case VM_NL80211_SCAN_TIME_IDLE:
        if (usr_data) {
            wnet_vif->vm_scan_time_idle = (unsigned char)usr_data;
            wifi_mac_set_scan_time(wnet_vif);
        }

        pr_debug("%s, vid:%d set scan_time_idle = %d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_time_idle);
        break;

    case VM_NL80211_SCAN_TIME_CONNECT:
        if (usr_data) {
            wnet_vif->vm_scan_time_connect = (unsigned char)usr_data;
            wifi_mac_set_scan_time(wnet_vif);
        }

        pr_debug("%s, vid:%d set scan_time_connect = %d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_time_connect);
        break;

    case VM_NL80211_SCAN_HANG:
        wnet_vif->vm_scan_hang = (unsigned char)usr_data;
        pr_debug("%s, vid:%d vm_scan_hang:%d\n ", __func__, wnet_vif->wnet_vif_id, wnet_vif->vm_scan_hang);
        break;

    case VM_NL80211_PRINT_VERSION:
        print_driver_version();
        pr_debug("driver version: %s\n", DRIVERVERSION);
        break;

     case VM_NL80211_SET_EN_COEX:
        pr_debug("%s, coexist en= %d\n ", __func__, usr_data);
        wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_en(usr_data);
        break;

    case VM_NL80211_SET_COEXIST_MAX_MISS_BCN_CNT:
        //pr_debug("%s, set max bcn miss cnt= %d\n ", __func__, usr_data);
        wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_max_miss_bcn(usr_data);
        break;

    case VM_NL80211_SET_COEXIST_REQ_TIMEOUT:
        wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_req_timeslice_timeout_value(usr_data);
        pr_debug("%s, set req timeout value= %d\n ", __func__, usr_data);
        break;

    case VM_NL80211_SET_COEXIST_NOT_GRANT_WEIGHT:
        wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_not_grant_weight(usr_data);
         pr_debug("%s, set coexist_not_grant_weight= %d\n ", __func__, usr_data);
        break;

    case VM_NL80211_SET_COEXIST_CONFIG:
        wifimac->drv_priv->hal_priv->hal_ops.phy_coexist_config(data, data_len);
        break;

    case VM_NL80211_SET_COEXIST_MAX_NOT_GRANT_CNT:
        wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_max_not_grant_cnt(usr_data);
        pr_debug("%s, set coexist_max_not_grant_cnt= %d\n ", __func__, usr_data);
        break;

     case VM_NL80211_SET_COEXIST_IRQ_END_TIM:
        pr_debug("%s, set SET_COEXIST_IRQ_END_TIM= 0x%8x\n ", __func__, usr_data);
        pr_debug("%s, A2DP irq end time is = %d\n ", __func__, usr_data &0xffff);
        pr_debug("%s, other traffic irq end time is = %d\n ", __func__, usr_data>>16);
        wnet_vif->vif_ops.write_word(RG_COEX_IRQ_END_TIME, usr_data);
        break;


     case VM_NL80211_SET_COEXIST_SCAN_PRI_RANGE:
         /*
            bit31-bit16 : minimal  priority
            bit15:bit0: max priority
        */
       wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_scan_priority_range(usr_data);
       break;

    case VM_NL80211_SET_COEXIST_BE_BK_NOQOS_PRI_RANGE:
       /*
            bit31-bit16 : minimal  priority
            bit15:bit0: max priority
        */
        wifimac->drv_priv->hal_priv->hal_ops.phy_set_coexist_be_bk_noqos_priority_range(usr_data);
        break;

    case VM_NL80211_GET_EFUSE_DATA:
        ret = reply_get_efuse_data(wiphy, usr_data);
        break;
    /* add host print fw log. */
    case VM_NL80211_GET_FW_LOG:
        wifimac->drv_priv->hal_priv->hal_ops.hal_set_fwlog_cmd(opt_data[1]);
        break;

    default:
        pr_err("Cfg80211 Warning:Vendor subcmd index out of range!!!\n");
        break;
    }
    return ret;
}


const struct wiphy_vendor_command vendor_commands[] = {
	{
	 .info = {.vendor_id = NL80211_ATTR_VENDOR_ID,.subcmd = NL80211_ATTR_VENDOR_SUBCMD,},
	 .flags = WIPHY_VENDOR_CMD_NEED_WDEV |
	 WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
	 .policy = VENDOR_CMD_RAW_DATA,
#endif
	 .doit = vm_cfg80211_vnd_cmd_set_para,
	 },
};

static void
vm_register_cfg80211_vnd_cmd(struct wiphy *wiphy)
{
	wiphy->vendor_commands = vendor_commands;
	wiphy->n_vendor_commands = ARRAY_SIZE(vendor_commands);
	wiphy->vendor_events = vendor_events;
	wiphy->n_vendor_events = ARRAY_SIZE(vendor_events);
}
static void vm_cfg80211_preinit_wiphy(struct wlan_net_vif *wnet_vif, struct wiphy *wiphy)
{
    wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

    wiphy->max_scan_ssids = IV_SSID_SCAN_AMOUNT;
    wiphy->max_scan_ie_len = AML_SCAN_IE_LEN_MAX;
    wiphy->max_num_pmkids = AML_MAX_NUM_PMKIDS;

    wiphy->max_remain_on_channel_duration = AML_MAX_REMAIN_ON_CHANNEL_DURATION;
    wiphy->interface_modes =    BIT(NL80211_IFTYPE_STATION)
                                | BIT(NL80211_IFTYPE_AP)
                                | BIT(NL80211_IFTYPE_MONITOR) ;

    wiphy->cipher_suites = aml_cipher_suites;
    wiphy->n_cipher_suites = ARRAY_SIZE(aml_cipher_suites);
#ifdef AML_WPA3
    wiphy->features |= NL80211_FEATURE_SAE;
#endif

    wiphy->bands[IEEE80211_BAND_2GHZ] = aml_spt_band_alloc(IEEE80211_BAND_2GHZ);
    wiphy->bands[IEEE80211_BAND_5GHZ] = aml_spt_band_alloc(IEEE80211_BAND_5GHZ);

    if (wnet_vif->vm_pwrsave.ips_sta_psmode != WIFINET_PWRSAVE_NONE)
        wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;
    else
        wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
    wiphy->mgmt_stypes = vm_cfg80211_default_mgmt_stypes;

    wiphy->software_iftypes |= BIT(NL80211_IFTYPE_MONITOR);

#if defined(CONFIG_PM) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) && \
        LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
    wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#ifdef PNO_SUPPORT
    wiphy->max_sched_scan_ssids = MAX_PNO_LIST_COUNT;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
    wiphy->max_match_sets = MAX_PNO_LIST_COUNT;
#endif
#endif
#endif

#if defined(CONFIG_PM)
    wiphy->wowlan = &wowlan_stub;
#endif

    wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
    wiphy->flags |= WIPHY_FLAG_OFFCHAN_TX ;
    wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
    wiphy->flags |= WIPHY_FLAG_OFFCHAN_TX | WIPHY_FLAG_HAVE_AP_SME;

    pr_debug("AML INFO:before register vendor cmd!!!\n ");
    vm_register_cfg80211_vnd_cmd(wiphy);
    return;
}

int wifi_mac_alloc_wdev(struct wlan_net_vif *wnet_vif, struct device *dev)
{
    int ret = -1;
    struct wireless_dev *wdev = NULL;
    struct net_device *netdev = wnet_vif->vm_ndev;
    struct vm_wdev_priv *pwdev_priv;


    wdev = (struct wireless_dev *)ZMALLOC(sizeof(struct wireless_dev),"wdev", GFP_KERNEL);
    if (!wdev) {
        ERROR_DEBUG_OUT("ERROR ENOMEM\n");
        ret = -ENOMEM;
        return ret;
    }

    /*alloc a cfg80211_register_dev and set ops for cfg80211 framework,
    the 'sizeof(struct vm_wdev_priv)' is sized for our private data. And return wireless hw descriptor. */
    wdev->wiphy = wiphy_new(&vm_cfg80211_ops, sizeof(struct vm_wdev_priv));
    if (!wdev->wiphy) {
        ERROR_DEBUG_OUT("ERROR ENOMEM\n");
        ret = -ENOMEM;
        goto out_err_new;
    }
    /*set wdev->wiphy.parent = dev */
    set_wiphy_dev(wdev->wiphy, dev);

    wnet_vif->vm_wdev = wdev;
    /*set wireless device to net device */
    netdev->ieee80211_ptr = wdev;

    /*get 'private' data area from wiphy */
    pwdev_priv = wdev_to_priv(wdev);
    pwdev_priv->pmon_ndev = NULL;
    pwdev_priv->pGo_ndev = NULL;
    pwdev_priv->ifname_mon[0] = '\0';
    pwdev_priv->ifname_go[0] = '\0';
    pwdev_priv->vm_wdev = wdev;
    pwdev_priv->wnet_vif = wnet_vif;
    DPRINTF(AML_DEBUG_ANY, "%s %d pwdev_priv->scan_request=%p wnet_vif_id=%d\n",
                __func__, __LINE__, pwdev_priv->scan_request,wnet_vif->wnet_vif_id);

    spin_lock_init(&pwdev_priv->scan_req_lock);

    OS_SPIN_LOCK(&pwdev_priv->scan_req_lock);
    pwdev_priv->scan_request = NULL;
    OS_SPIN_UNLOCK(&pwdev_priv->scan_req_lock);

    pwdev_priv->connect_request = NULL;

    spin_lock_init(&pwdev_priv->connect_req_lock);
    os_timer_ex_initialize(&pwdev_priv->connect_timeout, CFG80211_CONNECT_TIMER_OUT,
        vm_cfg80211_connect_timeout_timer, wnet_vif);

    /*set net dev for wireless device */
    wdev->netdev = netdev;
    vm_cfg80211_preinit_wiphy(wnet_vif, wdev->wiphy);

#ifdef CONFIG_P2P
    vm_p2p_attach(wnet_vif);
#endif

    /*set wireless interface type */
    wdev->iftype = drv_opmode_2_nl80211_iftype(wnet_vif->vm_opmode, wnet_vif->vm_p2p->p2p_role);

    wdev->wiphy->addresses = (struct mac_address *)wnet_vif->vm_myaddr;
    wdev->wiphy->n_addresses = 1;
    AML_OUTPUT("%p %d wdev->iftype  %d\n",
            wdev->wiphy->addresses,wdev->wiphy->n_addresses,wdev->iftype );
    /*register wireless device. */
    ret = wiphy_register(wdev->wiphy);
    aml_2g_channels_init(wdev->wiphy->bands[IEEE80211_BAND_2GHZ]->channels);
    aml_5g_channels_init(wdev->wiphy->bands[IEEE80211_BAND_5GHZ]->channels);

    if (ret < 0)
    {
        ERROR_DEBUG_OUT("ERROR register wiphy\n");
        goto out_err_register;
    }

    return ret;

out_err_register:
    wiphy_free(wdev->wiphy);

out_err_new:
    FREE((unsigned char*)wdev,"wdev");

    return ret;
}

void vm_wdev_free(struct wireless_dev *wdev, unsigned char vid)
{
    struct vm_wdev_priv *pwdev_priv;

    if (!wdev)
    {
        AML_OUTPUT("vid:%d\n", vid);
        return;
    }

    pwdev_priv = wdev_to_priv(wdev);
    os_timer_ex_del(&pwdev_priv->connect_timeout, CANCEL_SLEEP);
    AML_OUTPUT("scan abort when device remove\n");
    vm_cfg80211_indicate_scan_done(pwdev_priv, true);

    if (pwdev_priv->pmon_ndev)
    {
        if (pwdev_priv->pmon_ndev->flags & IFF_RUNNING)
        {
            pwdev_priv->pmon_ndev->flags &= ~IFF_RUNNING;
        }
        AML_OUTPUT("unregister monitor interface\n");
        if (pwdev_priv->pmon_ndev->reg_state == NETREG_REGISTERED)
            unregister_netdev(pwdev_priv->pmon_ndev);
        if (pwdev_priv->pmon_ndev->reg_state == NETREG_UNREGISTERED)
            free_netdev(pwdev_priv->pmon_ndev);
        pwdev_priv->pmon_ndev = NULL;
    }
    if (pwdev_priv->pGo_ndev)
    {
        AML_OUTPUT("unregister pGo_ndev interface\n");
        if (pwdev_priv->pGo_ndev->reg_state == NETREG_REGISTERED)
            unregister_netdev(pwdev_priv->pGo_ndev);
        if (pwdev_priv->pGo_ndev->reg_state == NETREG_UNREGISTERED)
            free_netdev(pwdev_priv->pGo_ndev);
#ifdef NEW_WIPHY
        wiphy_free(pwdev_priv->pGo_wdev->wiphy);
#endif//      #ifdef NEW_WIPHY
        FREE((unsigned char*)pwdev_priv->pGo_wdev,"pwdev_priv->pGo_wdev");
        pwdev_priv->pGo_ndev=NULL;
        //wiphy_unregister(wdev->wiphy);
    }
    wiphy_unregister(wdev->wiphy);
    aml_spt_band_free(wdev->wiphy->bands[IEEE80211_BAND_2GHZ]);
    aml_spt_band_free(wdev->wiphy->bands[IEEE80211_BAND_5GHZ]);
    wiphy_free(wdev->wiphy);
    FREE((unsigned char*)wdev,"wdev");
    AML_OUTPUT("<exit>\n");
    return;
}

int vm_cfg80211_up(struct wlan_net_vif *wnet_vif)
{
    struct ieee80211_supported_band *bands;
    struct wireless_dev *pwdev = wnet_vif->vm_wdev;
    struct wiphy *wiphy = pwdev->wiphy;

    /* start chip */
    {
        struct wifi_mac *wifimac = wnet_vif->vm_wmac;

        if (wifimac->drv_priv->drv_ops.drv_interface_enable != NULL)
            wifimac->drv_priv->drv_ops.drv_interface_enable(ENABLE, wnet_vif->wnet_vif_id);
    }

    AML_OUTPUT("<%s>\n", wnet_vif->vm_ndev->name);
    bands = wiphy->bands[IEEE80211_BAND_2GHZ];
    if (bands)
    {
        AML_OUTPUT("config 2G ht\n");
        vm_cfg80211_init_ht_capab(&bands->ht_cap, IEEE80211_BAND_2GHZ);
        vm_cfg80211_init_vht_capab(&bands->vht_cap, IEEE80211_BAND_2GHZ);
    }

    bands = wiphy->bands[IEEE80211_BAND_5GHZ];
    if (bands)
    {
        AML_OUTPUT("config 5G ht\n");
        vm_cfg80211_init_ht_capab(&bands->ht_cap, IEEE80211_BAND_5GHZ);
        vm_cfg80211_init_vht_capab(&bands->vht_cap, IEEE80211_BAND_5GHZ);
    }

#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p_support) {
        vm_p2p_up(wnet_vif);
    }
#endif

    return 0;
}

void vm_cfg80211_down(struct wlan_net_vif *wnet_vif)
{
    struct wireless_dev *pwdev = wnet_vif->vm_wdev;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(pwdev);

    DPRINTF(AML_DEBUG_CFG80211, "<%s>:%s %d\n", wnet_vif->vm_ndev->name,__func__, __LINE__);
    wnet_vif->vm_phase_flags = 0;
    vm_cfg80211_indicate_scan_done(pwdev_priv, true);

#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p_support) {
        vm_p2p_down(wnet_vif);
    }
#endif

    /* stop chip */
    {
        struct wifi_mac *wifimac = wnet_vif->vm_wmac;

        if (wifimac->drv_priv->drv_ops.drv_interface_enable != NULL)
            wifimac->drv_priv->drv_ops.drv_interface_enable(DISABLE, wnet_vif->wnet_vif_id);
    }
    return;
}

static void vm_wlan_net_vif_detach_ex(struct wlan_net_vif *wnet_vif)
{
    KASSERT(wnet_vif->vm_state == WIFINET_S_INIT, ("wnet_vif not stopped"));

    wifi_mac_scan_vdetach(wnet_vif);
    wifi_mac_com_vdetach(wnet_vif);
    wifi_mac_pwrsave_vdetach(wnet_vif);
    wifi_mac_sta_vdetach(wnet_vif);
}

static void
vm_wlan_net_vif_attach_ex(struct wlan_net_vif *wnet_vif,struct vm_wlan_net_vif_params *cp)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int vm_opmode;
    struct in_device *in_dev;
    struct in_ifaddr *ifa_v4;
    __be32 ipv4 = 0;

    pr_debug("<running> %s %d  cp->vm_param_opmode  %x \n",__func__,__LINE__,cp->vm_param_opmode);
    switch (cp->vm_param_opmode)
    {
        case WIFINET_M_STA:
            vm_opmode = cp->vm_param_opmode;
            break;
        case WIFINET_M_IBSS:
        case WIFINET_M_MONITOR:
            vm_opmode = cp->vm_param_opmode;
            break;
        case WIFINET_M_HOSTAP:
        case WIFINET_M_WDS:
            vm_opmode = WIFINET_M_HOSTAP;
            netif_carrier_on(wnet_vif->vm_ndev);
            break;
        default:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            return ;
    }

    in_dev = __in_dev_get_rtnl(wnet_vif->vm_ndev);
    if (in_dev) {
      ifa_v4 = in_dev->ifa_list;
      if (ifa_v4 != NULL)
        ipv4 = ifa_v4->ifa_local;
    }

    if (wifimac->drv_priv->drv_ops.change_interface(wifimac->drv_priv,
            wnet_vif->wnet_vif_id,wnet_vif,vm_opmode, wnet_vif->vm_myaddr, ipv4))
    {
        pr_err("Unable to add an interface for driver.\n");
        wifi_mac_free_vmac(wnet_vif);
        return ;
    }
    vm_wlan_net_vif_setup_forchvif(wifimac, wnet_vif, NULL, vm_opmode);
    wifi_mac_station_init(wnet_vif);
    pr_debug("<running> %s %d wnet_vif->vm_opmode =%d \n",__func__,__LINE__,wnet_vif->vm_opmode);
    wifi_mac_pwrsave_latevattach(wnet_vif);
}

void vm_wlan_net_vif_mode_change(struct wlan_net_vif *wnet_vif,struct vm_wlan_net_vif_params *cp)
{
    AML_OUTPUT("<running> %s %d \n",__func__,__LINE__);
    wnet_vif->wnet_vif_replaycounter++;
    if (wnet_vif->vm_ndev->flags & IFF_RUNNING) {
        /*RUNNING ,For the settings to take effect, need to restart the wifi*/
        AML_OUTPUT("<running> %s %d mode change need stop open\n",__func__,__LINE__);
        wifi_mac_stop(wnet_vif->vm_ndev);
        vm_wlan_net_vif_detach_ex(wnet_vif);
        vm_wlan_net_vif_attach_ex(wnet_vif,cp);
        wifi_mac_security_vattach(wnet_vif);
        wifi_mac_open(wnet_vif->vm_ndev);

    } else {
        AML_OUTPUT("<running> %s %d mode change not need stop open\n",__func__,__LINE__);
        /*Not RUNNING ,Just need to set and wait for the upper layer to be up*/
        vm_wlan_net_vif_detach_ex(wnet_vif);
        vm_wlan_net_vif_attach_ex(wnet_vif,cp);
        wifi_mac_security_vattach(wnet_vif);

    }
}

int wifi_mac_rm_app_ie(struct wifi_mac_app_ie_t  * app_ie)
{
    if (app_ie->ie != NULL)
    {
        NET_FREE(app_ie->ie,"wifi_mac_save_app_ie.ie");
        app_ie->ie = NULL;
        app_ie->length = 0;
    }
    return 0;
}

int wifi_mac_save_app_ie(struct wifi_mac_app_ie_t *app_ie, const unsigned char *app_buf, int app_buflen)
{
    void * ie;
    void * tmpPtr;

    if((app_buf == NULL) ||(app_buflen == 0))
    {
        wifi_mac_rm_app_ie(app_ie);
        return -1;
    }
    ie = NET_MALLOC(app_buflen, GFP_ATOMIC, "wifi_mac_save_app_ie.ie");
    if (ie == NULL)
        return -ENOMEM;

    tmpPtr = app_ie->ie;

    memcpy(ie, app_buf, app_buflen);
    app_ie->ie = ie;
    app_ie->length = app_buflen;

    /*free old IE buffer*/
    if (tmpPtr)
        FREE(tmpPtr, "wifi_mac_save_app_ie.ie");

    return 0;
}

int cipher2cap(int cipher)
{
    switch (cipher)
    {
        case WIFINET_CIPHER_WEP:
            return WIFINET_C_WEP;
        case WIFINET_CIPHER_AES_OCB:
            return WIFINET_C_AES;
        case WIFINET_CIPHER_AES_CCM:
            return WIFINET_C_AES_CCM;
        case WIFINET_CIPHER_WPI:
            return WIFINET_C_WAPI;
        case WIFINET_CIPHER_TKIP:
            return WIFINET_C_TKIP;
    }
    return 0;
}

int  netdev_setcsum( struct net_device *dev,int data)
{
    if (data)
    {
        dev->features |= NETIF_F_HW_CSUM;
        dev->hw_features |= dev->features;
    }
    else
    {
        dev->features &= ~NETIF_F_HW_CSUM;
    }
    return 0;
}


int wifi_mac_preempt_scan(struct wifi_mac *wifimac, int max_grace, int max_wait)
{
    int total_delay = 0;
    int canceled = 0, ready = 0;

    pr_debug("%s %d try to preempt scan \n", __func__, __LINE__);

    while (!ready && total_delay < max_grace + max_wait)
    {
        if ((wifimac->wm_flags & WIFINET_F_SCAN) == 0)
        {
            ready = 1;
        }
        else
        {
            if (!canceled && total_delay > max_grace)
            {
                pr_warn("%s: cancel pending scan request", __func__);
                (void) wifi_mac_cancel_scan(wifimac);
                canceled = 1;
            }
            msleep(CANCLE_STEP_MS);
            total_delay += CANCLE_STEP_MS;
        }
    }

    if (!ready)
    {
       pr_debug("%s: ","Timeout canceling current scan.");
    }

    return ready;
}


char preempt_scan(struct net_device *dev, int max_grace, int max_wait)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

#ifdef CONFIG_P2P
    if((wnet_vif->vm_p2p_support == 0) && wifimac->wm_p2p_connection_protect) {
        ERROR_DEBUG_OUT("rejected preempt scan due to p2p scan protection\n");
        return -1;
    }
#endif //#ifdef CONFIG_P2P

    wifi_mac_preempt_scan(wifimac, max_grace, max_wait);

    return 0;
}


void batch_dump_reg(struct wiphy *wiphy,unsigned int addr[], unsigned int addr_num)
{
    unsigned int i =0;
    unsigned int reg_val = 0;
    struct wlan_net_vif *wnet_vif = wiphy_to_adapter(wiphy);
    for(i = 0; i < addr_num; i++)
    {
        if(((addr[i] >> 24) & 0xff) == 0xff )
        {
#ifdef USE_T902X_RF
         reg_val = rf_i2c_read(addr[i] & 0x00ffffff); //access t902x rf reg
#endif
        }
        else if(((addr[i]  >> 24) & 0xf0) == 0xf0 )
        {
#ifdef USE_T902X_RF
            reg_val = rf_i2c_read(addr[i] & 0xffffffff); //access t902x rf reg
#endif

        }
        else
        {
            reg_val = wnet_vif->vif_ops.read_word(addr[i]);
        }

        pr_debug("addr[%d] 0x%x  data 0x%x\n", i, addr[i] , reg_val);
    }

}

static void aml_cfg80211_ch_switch_notify(struct net_device *dev,
    struct cfg80211_chan_def *chandef, unsigned int link_id)
{
#if ((defined (CONFIG_AMLOGIC_KERNEL_VERSION) && defined (AML_KERNEL_VERSION)) && (\
    (CONFIG_AMLOGIC_KERNEL_VERSION == 13515 && AML_KERNEL_VERSION >= 15)\
    || (CONFIG_AMLOGIC_KERNEL_VERSION == 14515 && AML_KERNEL_VERSION >= 12) ) )\
    || (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 9, 0))
    cfg80211_ch_switch_notify(dev, chandef, link_id, 0);
#elif defined (CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT) \
    || LINUX_VERSION_CODE >= KERNEL_VERSION(6, 9, 0)
    cfg80211_ch_switch_notify(dev, chandef, link_id);
#else
    cfg80211_ch_switch_notify(dev, chandef);
#endif
}

static void aml_cfg80211_ch_switch_started_notify(struct net_device *dev,
    struct cfg80211_chan_def *chandef, unsigned int link_id, u8 count, bool quiet)
{
#if ((defined (CONFIG_AMLOGIC_KERNEL_VERSION) && defined (AML_KERNEL_VERSION)) && (\
    (CONFIG_AMLOGIC_KERNEL_VERSION == 13515 && AML_KERNEL_VERSION >= 15)\
    || (CONFIG_AMLOGIC_KERNEL_VERSION == 14515 && AML_KERNEL_VERSION >= 12) ) )\
    || (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 9, 0))
    cfg80211_ch_switch_started_notify(dev, chandef, link_id, count, quiet, 0);
#elif defined (CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT) \
    || LINUX_VERSION_CODE >= KERNEL_VERSION(6, 9, 0)
    cfg80211_ch_switch_started_notify(dev, chandef, link_id, count, quiet);
#else
    cfg80211_ch_switch_started_notify(dev, chandef, count);
#endif
}

void vm_cfg80211_chan_switch_notify_task(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)param1;
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param2;
    unsigned char start = (unsigned char) param3;
    struct net_device *ndev = wnet_vif->vm_ndev;
    struct cfg80211_chan_def chandef = {0};
    struct wifi_channel *vmac_chan =  (struct wifi_channel *)param4;
    struct ieee80211_channel *notify_channel = NULL;
    struct wiphy *wiphy = wnet_vif->vm_wdev->wiphy;

    if (vmac_chan == WIFINET_CHAN_ERR)
    {
        ERROR_DEBUG_OUT("main_vmac_chan is WIFINET_CHAN_ERR\n");
        return;
    }

    notify_channel = ieee80211_get_channel(wiphy, wifi_mac_Ieee2mhz(vmac_chan->chan_pri_num, 0));
    if (notify_channel == NULL) {
        ERROR_DEBUG_OUT("no such pri_chan:%d\n", vmac_chan->chan_pri_num);
        return;
    } else {
        cfg80211_chandef_create(&chandef, notify_channel, NL80211_CHAN_NO_HT);

        if (vmac_chan->chan_bw == WIFINET_BWC_WIDTH20) {
            chandef.width = NL80211_CHAN_WIDTH_20;
        } else if (vmac_chan->chan_bw == WIFINET_BWC_WIDTH40) {
            chandef.width = NL80211_CHAN_WIDTH_40;
        } else if (vmac_chan->chan_bw == WIFINET_BWC_WIDTH80) {
            chandef.width = NL80211_CHAN_WIDTH_80;
        } else {
            ERROR_DEBUG_OUT("station chan_bw  do not support\n");
        }
        chandef.center_freq1 = vmac_chan->chan_cfreq1;
    }

    AML_OUTPUT("vid:%d notify new_chan:%d bw:%d freq:%d start:%d\n", wnet_vif->wnet_vif_id, vmac_chan->chan_pri_num,
        vmac_chan->chan_bw, vmac_chan->chan_cfreq1, start);

    if (start) {
        aml_cfg80211_ch_switch_started_notify(ndev, &chandef, 0, wnet_vif->vm_wmac->wm_doth_tbtt, 0);/*#define CSA_BLOCK_TX            1*/
    } else {
        aml_cfg80211_ch_switch_notify(ndev, &chandef, 0);
        memset(&(wnet_vif->csa_target.switch_chan) , 0, sizeof(struct wifi_channel));
    }

    return;
}
#endif//CONFIG_AML_CFG80211
