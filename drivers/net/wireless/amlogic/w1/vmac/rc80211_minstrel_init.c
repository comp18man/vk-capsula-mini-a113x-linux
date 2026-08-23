#ifdef AUTO_RATE_SIM
#include "type.h"
#include "opt_sim2.h"
#include "mac80211_temp.h"
#include "cfg80211.h"
struct sk_buff g_skbuffer[MAX_SKB_NUM];
#else
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/random.h>
#include <linux/ieee80211.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include "wifi_mac_com.h"
#endif

#include "osdep.h"
#include "rc80211_minstrel.h"
#include "rc80211_minstrel_ht.h"
#include "rc80211_minstrel_init.h"
#include "wifi_debug.h"
#include "wifi_mac.h"
#include "fi_sdio.h"
#include "wifi_rate_ctrl.h"
#include "wifi_pkt_desc.h"
#include "wifi_cfg80211.h"

static struct ieee80211_supported_band aml_band_24ghz = {
	.n_channels = AML_2G_CHANNELS_NUM,
	.channels = aml_2ghz_channels,
	.band = (enum nl80211_band)IEEE80211_BAND_2GHZ,
	.n_bitrates = AML_G_RATES_NUM,
	.bitrates = aml_g_rates,
	.ht_cap.cap = 0,/*Need to be initialized later*/
	.ht_cap.ht_supported = true,
};

static struct ieee80211_supported_band aml_band_5ghz = {
	.n_channels = AML_5G_CHANNELS_NUM,
	.channels = aml_5ghz_channels,
	.band = (enum nl80211_band)IEEE80211_BAND_5GHZ,
	.n_bitrates = AML_A_RATES_NUM,/*Eliminate 11b rate*/
	.bitrates = aml_a_rates,/*Eliminate 11b rate*/
	.ht_cap.cap = 0,  /*Need to be initialized later*/
	.ht_cap.ht_supported = true,
};

short rssi_threshold[3][10] = {
	{555, -70, -75, -76, -77, -82, -85, -89, -94},
	{-66, -67, -72, -73, -74, -79, -82, -86, -88},
	{-62, -64, -68, -69, -71, -76, -79, -82, -85},
};

short snr_threshold[3][10] = {
	{555, 26, 21, 20, 19, 14, 11, 8, 5},
	{27, 26, 21, 20, 19, 14, 11, 8, 6},
	{27, 26, 22, 20, 19, 14, 11, 8, 5},
};

static struct ieee80211_sta_ht_cap aml_get_ht_cap(struct aml_rate_adaptation_dev *aml_minstrel_dev, struct ieee80211_sta_ht_cap *p_ht_cap)
{
	int i;
	struct ieee80211_sta_ht_cap ht_cap = {0};
	p_ht_cap->ht_supported = 0;

	if (!(aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_ENABLED))
		return ht_cap;

	ht_cap.ht_supported = 1;
	ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
	ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_8;
	ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	ht_cap.cap |= IEEE80211_HT_CAP_DSSSCCK40;
	ht_cap.cap |= WLAN_HT_CAP_SM_PS_STATIC << IEEE80211_HT_CAP_SM_PS_SHIFT;

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_HT20_SGI)
		ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_HT40_SGI)
		ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_DYNAMIC_SMPS) {
		u32 smps;

		smps   = WLAN_HT_CAP_SM_PS_DYNAMIC;
		smps <<= IEEE80211_HT_CAP_SM_PS_SHIFT;

		ht_cap.cap |= smps;
	}

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_TX_STBC)
		ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_RX_STBC) {
		u32 stbc;

		stbc   = aml_minstrel_dev->ht_cap_info;
		stbc  &= WMI_HT_CAP_RX_STBC;
		stbc >>= WMI_HT_CAP_RX_STBC_MASK_SHIFT;
		stbc <<= IEEE80211_HT_CAP_RX_STBC_SHIFT;
		stbc  &= IEEE80211_HT_CAP_RX_STBC;

		ht_cap.cap |= stbc;
	}

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_LDPC)
		ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;

	if (aml_minstrel_dev->ht_cap_info & WMI_HT_CAP_L_SIG_TXOP_PROT)
		ht_cap.cap |= IEEE80211_HT_CAP_LSIG_TXOP_PROT;

	/* max AMSDU is implicitly taken from vht_cap_info */
	if (aml_minstrel_dev->vht_cap_info & WMI_VHT_CAP_MAX_MPDU_LEN_MASK)
		ht_cap.cap |= IEEE80211_HT_CAP_MAX_AMSDU;

	for (i = 0; i < aml_minstrel_dev->num_rf_chains; i++)
		ht_cap.mcs.rx_mask[i] = 0xFF;

	ht_cap.mcs.tx_params |= IEEE80211_HT_MCS_TX_DEFINED;

	memcpy(p_ht_cap, &ht_cap, sizeof(struct ieee80211_sta_ht_cap));
	return ht_cap;
}


static struct ieee80211_sta_vht_cap aml_create_vht_cap(struct aml_rate_adaptation_dev *aml_minstrel_dev, int rate_mode)
{
    struct ieee80211_sta_vht_cap vht_cap = {0};
    u16 mcs_map;
    int i;

    /*0: legacy rate, 1:ht rate, 2:vht rate*/
    if (rate_mode == 2) {
        vht_cap.vht_supported = 1;

    } else {
        vht_cap.vht_supported = 0;
    }
    vht_cap.cap = aml_minstrel_dev->vht_cap_info;

    mcs_map = 0;
    for (i = 0; i < 8; i++) {
        if (i < aml_minstrel_dev->num_rf_chains)
            mcs_map |= IEEE80211_VHT_MCS_SUPPORT_0_9 << (i * 2);
        else
            mcs_map |= IEEE80211_VHT_MCS_NOT_SUPPORTED << (i * 2);
    }

    vht_cap.vht_mcs.rx_mcs_map = /*cpu_to_le16*/(mcs_map);
    vht_cap.vht_mcs.tx_mcs_map = /*cpu_to_le16*/(mcs_map);

    return vht_cap;
}

static struct aml_rate_adaptation_dev  g_aml_rate_adaptation_dev;
static struct ieee80211_hw g_hw;
static struct wiphy g_wiphy;

static struct ieee80211_sta_aml g_sta;
#ifdef AUTO_RATE_SIM
    static struct ieee80211_sta_rates g_rates[4];
    int g_rate_mode = 0;	/*0: legacy rate, 1:ht rate, 2:vht rate*/
#endif

 /* allocate memory and init in alloc_sta*/
struct minstrel_ht_sta_priv *g_minstrel_ht_sta_priv = NULL;
struct minstrel_sta_info *g_minstrel_sta_info = NULL;
struct minstrel_priv *g_minstel_pri = NULL;

void aml_minstrel_attach(void)
{
    struct minstrel_rate_control_ops* p_rate_control_ops_ht = NULL;

    memset(&g_hw,0,sizeof(g_hw));
    memset(&g_wiphy,0,sizeof(g_wiphy));

    g_hw.max_rates = 4;
    g_hw.max_rate_tries = 3;
    g_hw.wiphy = &g_wiphy;
    g_hw.wiphy->bands[NL80211_BAND_2GHZ] = &aml_band_24ghz;
    g_hw.wiphy->bands[NL80211_BAND_5GHZ] = &aml_band_5ghz;
    p_rate_control_ops_ht = get_rate_control_ops_ht();
    g_minstel_pri = p_rate_control_ops_ht->alloc(&g_hw);
}

void aml_minstrel_detach(void)
{
    struct minstrel_rate_control_ops *p_rate_control_ops = NULL;

    pr_debug("%s\n", __func__);
    g_aml_rate_adaptation_dev.ht_cap_info = 0;
    p_rate_control_ops = get_rate_control_ops();
    p_rate_control_ops->free(g_minstel_pri);
    g_minstel_pri = NULL;
}

static unsigned int support_legacy_rate_init( struct wifi_station *sta ,  struct ieee80211_sta_aml *p_ieee_sta,unsigned int channel_band)
{
    int i = 0;
    unsigned int bit_val = 0;

    for( i = 0; i<sta->sta_rates.dot11_rate_num; i++){
        switch(sta->sta_rates.dot11_rate[i]&0x7f){
            case 0x02: /*1M*/
                bit_val |= 0x1;
                break;

            case 0x04:  /*2M*/
                  bit_val |= 0x2;
                break;

            case 0x0b:   /*5.5M*/
                bit_val |= 0x4;
                break;

            case 0x16: /*11M*/
                 bit_val |= 0x8;
                break;

            case 0x0c:  /*6M*/
                 bit_val |= 0x10;
                break;

            case 0x12:  /*9M*/
                  bit_val |= 0x20;
                break;

            case 0x18: /*12M*/
                bit_val |= 0x40;
                break;

            case 0x24: /*18M*/
                bit_val |= 0x80;
                break;

            case 0x30:   /*24M*/
                 bit_val |= 0x100;
                break;

            case 0x48:   /*36M*/
                bit_val |= 0x200;
                break;

            case 0x60:   /*48M*/
                bit_val |= 0x400;
                break;

            case 0x6c:   /*54M*/
                bit_val |= 0x800;
                break;
            default :
                ERROR_DEBUG_OUT("input rate error\n");
               break;
        }
    }

    if (channel_band == IEEE80211_BAND_2GHZ) {
        p_ieee_sta->supp_rates[IEEE80211_BAND_2GHZ]  = bit_val;

    } else if (channel_band == IEEE80211_BAND_5GHZ) {
        p_ieee_sta->supp_rates[IEEE80211_BAND_5GHZ] = (bit_val >> 4);

    } else {
        ERROR_DEBUG_OUT("input channel_band error\n");
    }

    AML_OUTPUT("channel_band=%d, bit_val=0x%04x\n", channel_band, bit_val);
    return 0;
}

static void aml_rate_adaptation_dev_init(struct wifi_station *sta, int rate_mode, unsigned int channel_band, struct ieee80211_sta_aml *p_ieee_sta)
{
    g_aml_rate_adaptation_dev.num_rf_chains = 1;

    g_aml_rate_adaptation_dev.ht_cap_info = 0;
    g_aml_rate_adaptation_dev.vht_cap_info = sta->sta_vhtcap;

    if (rate_mode) {
        g_aml_rate_adaptation_dev.ht_cap_info |= WMI_HT_CAP_ENABLED;

    if (sta->sta_htcap & WMI_HT_CAP_HT20_SGI)
        g_aml_rate_adaptation_dev.ht_cap_info |= WMI_HT_CAP_HT20_SGI;

    if (sta->sta_htcap & WMI_HT_CAP_HT40_SGI)
        g_aml_rate_adaptation_dev.ht_cap_info |= WMI_HT_CAP_HT40_SGI;

    if (sta->sta_htcap & WMI_HT_CAP_DYNAMIC_SMPS)
        g_aml_rate_adaptation_dev.ht_cap_info |= WMI_HT_CAP_DYNAMIC_SMPS;

    if (sta->sta_htcap & WMI_HT_CAP_LDPC)
        g_aml_rate_adaptation_dev.ht_cap_info |= WMI_HT_CAP_LDPC;
    }

    if (channel_band == IEEE80211_BAND_2GHZ) {
        int i = 0;
        g_aml_rate_adaptation_dev.sband = &aml_band_24ghz;

        pr_debug("support rate start\n");
        for (i = 0; i < sta->sta_rates.dot11_rate_num; i++)
        {
            pr_debug("%02x  ",sta->sta_rates.dot11_rate[i]);
        }
        pr_debug("\n");
    } else {
        g_aml_rate_adaptation_dev.sband = &aml_band_5ghz;
        g_aml_rate_adaptation_dev.sband->vht_cap = aml_create_vht_cap(&g_aml_rate_adaptation_dev, rate_mode);
    }
}

static unsigned char get_fitable_bw(struct wifi_station *sta) {
    unsigned char bw;

    if (sta->sta_avg_bcn_rssi < sta->sta_wmac->wm_signal_power_bw_change_thresh_narrow) {
        bw = CHAN_BW_20M;

    } else if (sta->sta_avg_bcn_rssi < sta->sta_wmac->wm_signal_power_bw_change_thresh_wide) {
        bw = CHAN_BW_40M;

    } else {
        bw = CHAN_BW_80M;
    }

    if (bw <= sta->sta_chbw) {
        return bw;
    }

    return sta->sta_chbw;
}

static unsigned char get_fitable_mcs_rate(struct wifi_station *sta, unsigned char bw) {
    int avg_rssi = 0;
    unsigned char max_rate_rssi = 0;
    unsigned char max_rate_snr = 0;
    unsigned char max_rate = 0;
    char rssi_offset = 6;
    char snr_offset = 0;

    if (sta->sta_wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
        avg_rssi = translate_to_dbm(sta->sta_avg_rssi);
    else
        avg_rssi = sta->sta_avg_bcn_rssi;

    if((aml_wifi_get_platform_verid() == 1) || (aml_wifi_get_platform_verid() == 2)) {
        /*this is for gva only*/
        rssi_offset = 10;
    }

    if (avg_rssi >= rssi_threshold[bw][0] + rssi_offset) {
        max_rate_rssi = 9;

    } else if (avg_rssi >= rssi_threshold[bw][1] + rssi_offset) {
        max_rate_rssi = 8;

    } else if (avg_rssi >= rssi_threshold[bw][2] + rssi_offset) {
        max_rate_rssi = 7;

    } else if (avg_rssi >= rssi_threshold[bw][3] + rssi_offset) {
        max_rate_rssi = 6;

    } else if (avg_rssi >= rssi_threshold[bw][4] + rssi_offset) {
        max_rate_rssi = 5;

    } else if (avg_rssi >= rssi_threshold[bw][5] + rssi_offset) {
        max_rate_rssi = 4;

    } else if (avg_rssi >= rssi_threshold[bw][6] + rssi_offset) {
        max_rate_rssi = 3;

    } else if (avg_rssi >= rssi_threshold[bw][7] + rssi_offset) {
        max_rate_rssi = 2;

    } else if (avg_rssi >= rssi_threshold[bw][8] + rssi_offset) {
        max_rate_rssi = 1;

    } else {
        max_rate_rssi = 0;
    }

    //max rate according snr
    if (sta->sta_avg_snr >= snr_threshold[bw][0] + snr_offset) {
        max_rate_snr = 9;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][1] + snr_offset) {
        max_rate_snr = 8;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][2] + snr_offset) {
        max_rate_snr = 7;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][3] + snr_offset) {
        max_rate_snr = 6;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][4] + snr_offset) {
        max_rate_snr = 5;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][5] + snr_offset) {
        max_rate_snr = 4;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][6] + snr_offset) {
        max_rate_snr = 3;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][7] + snr_offset) {
        max_rate_snr = 2;

    } else if (sta->sta_avg_snr >= snr_threshold[bw][8] + snr_offset) {
        max_rate_snr = 1;

    } else {
        max_rate_snr = 0;
    }

    max_rate = max_rate_rssi;
    if (max_rate > max_rate_snr)
        max_rate = max_rate_snr;

    return max_rate;
}

void aml_minstrel_init(
#ifdef AUTO_RATE_SIM
    void
#else
    void *p_sta
#endif
)
{
    struct minstrel_rate_control_ops *p_rate_control_ops = NULL;
    struct minstrel_rate_control_ops *p_rate_control_ops_ht = NULL;
    struct ieee80211_sta_aml *p_ieee_sta = NULL;
    struct wifi_station *sta  = (struct wifi_station *)p_sta;
    struct minstrel_ht_sta_priv *p_minstrel_ht_sta_priv = NULL;
    struct minstrel_sta_info *p_minstrel_sta_info = NULL;
    unsigned int channel_band = IEEE80211_BAND_5GHZ;
    bool mcs_rate_support = true;
    int rate_mode = 0;   /*0: legacy rate, 1:ht rate, 2:vht rate*/
    unsigned char fitable_bw;

#ifdef AUTO_RATE_SIM
    memset(&g_rates, 0, sizeof(g_rates));
    rate_mode = g_rate_mode;
    g_sta.smps_mode = IEEE80211_SMPS_OFF;
    g_sta.rates = g_rates;
    p_ieee_sta = &g_sta;
#else
    p_ieee_sta = &(sta->ieee_sta);
    p_ieee_sta->smps_mode = IEEE80211_SMPS_OFF;
    p_ieee_sta->rates = &(sta->sta_ieee_rates);
    p_ieee_sta->bandwidth = (enum ieee80211_sta_rx_bandwidth)sta->sta_chbw;
    AML_OUTPUT("bw=%d, sta:%p,p_ieee_sta=%p\n", p_ieee_sta->bandwidth, sta,p_ieee_sta);
    if (sta->sta_wnet_vif->vm_curchan == NULL) {
        ERROR_DEBUG_OUT("vm_curchan is NULL, just return\n");
        return;
    }

    if (sta->sta_flags & WIFINET_NODE_VHT) {
        rate_mode = 2;
    } else if ( sta->sta_flags & WIFINET_NODE_HT) {
        rate_mode = 1;
    }

    if (WIFINET_IS_CHAN_2GHZ(sta->sta_wnet_vif->vm_curchan)) {
        channel_band = IEEE80211_BAND_2GHZ;
    } else {
        channel_band = IEEE80211_BAND_5GHZ;
    }
#endif

    /*0: legacy rate, 1:ht rate, 2:vht rate*/
    if (rate_mode) {
        mcs_rate_support = true;

    } else {
        mcs_rate_support = false;
    }

    AML_OUTPUT("channel_band=%d, rate_mode=%d\n", channel_band, rate_mode);
    support_legacy_rate_init(sta, p_ieee_sta, channel_band);

    aml_rate_adaptation_dev_init(sta, rate_mode, channel_band, p_ieee_sta);

    if (channel_band == IEEE80211_BAND_5GHZ) {
        p_ieee_sta->vht_cap = g_aml_rate_adaptation_dev.sband->vht_cap;
        AML_OUTPUT("vht_supported = %d\n", p_ieee_sta->vht_cap.vht_supported);
    }

    aml_get_ht_cap(&g_aml_rate_adaptation_dev, &(g_aml_rate_adaptation_dev.sband->ht_cap));
    p_ieee_sta->ht_cap = g_aml_rate_adaptation_dev.sband->ht_cap;

    AML_OUTPUT("ht_supported:%d, minstel_pri=%p, p_ieee_sta=%p\n", p_ieee_sta->ht_cap.ht_supported, g_minstel_pri, p_ieee_sta);
    p_rate_control_ops = get_rate_control_ops();
    p_rate_control_ops_ht = get_rate_control_ops_ht();
    if (mcs_rate_support) {
        p_minstrel_ht_sta_priv = p_rate_control_ops_ht->alloc_sta(g_minstel_pri, p_ieee_sta, GFP_ATOMIC);
        p_rate_control_ops_ht->rate_init(g_minstel_pri, g_aml_rate_adaptation_dev.sband, p_ieee_sta, p_minstrel_ht_sta_priv);
        fitable_bw = get_fitable_bw(sta);
        minstrel_init_start_stats(g_minstel_pri, p_minstrel_ht_sta_priv, get_fitable_mcs_rate(sta, fitable_bw), fitable_bw);

    } else {
        p_minstrel_sta_info = p_rate_control_ops->alloc_sta(g_minstel_pri, p_ieee_sta, GFP_ATOMIC);
        p_rate_control_ops->rate_init(g_minstel_pri, g_aml_rate_adaptation_dev.sband, p_ieee_sta, p_minstrel_sta_info);
    }

#ifdef AUTO_RATE_SIM
    g_minstrel_ht_sta_priv = p_minstrel_ht_sta_priv;
    g_minstrel_sta_info = p_minstrel_sta_info;
#else
    sta->sta_minstrel_ht_priv = p_minstrel_ht_sta_priv;
    sta->sta_minstrel_info = p_minstrel_sta_info;
    sta->minstrel_init_flag = 1;

    if (mcs_rate_support) {
        AML_OUTPUT("sta:%p, sta_minstrel_ht_priv:%p\n", sta, sta->sta_minstrel_ht_priv);
    } else {
        AML_OUTPUT("sta:%p, sta_minstrel_info:%p\n", sta, sta->sta_minstrel_info);
    }
#endif
}

void aml_minstrel_deinit(void *p_sta)
{
    struct wifi_station *sta = (struct wifi_station *)p_sta;
    struct minstrel_rate_control_ops* p_rate_control_ops = NULL;
    struct minstrel_rate_control_ops *p_rate_control_ops_ht = NULL;
    pr_debug("%s:%04x ", __func__, sta->sta_flags);

    if ((sta->sta_flags & WIFINET_NODE_VHT) || (sta->sta_flags & WIFINET_NODE_HT)) {
        p_rate_control_ops_ht = get_rate_control_ops_ht();
        pr_debug("ht free:%p\n", sta->sta_minstrel_ht_priv);
        p_rate_control_ops_ht->free_sta(sta->sta_minstrel_ht_priv);
        sta->sta_minstrel_ht_priv = NULL;

    } else {
        p_rate_control_ops = get_rate_control_ops();
        pr_debug("free:%p\n", sta->sta_minstrel_info);
        p_rate_control_ops->free_sta(sta->sta_minstrel_info);
        sta->sta_minstrel_info = NULL;
    }
    sta->minstrel_init_flag = 0;
}

static void rate_control_fill_sta_table(struct ieee80211_sta_aml *sta,
    struct ieee80211_tx_info *info, struct ieee80211_tx_rate *rates, int max_rates)
{
    struct ieee80211_sta_rates *ratetbl = NULL;
    int i;

    ratetbl = (sta->rates);

    /* Fill remaining rate slots with data from the sta rate table. */
    max_rates = MIN(max_rates, IEEE80211_TX_RATE_TABLE_SIZE);
    for (i = 0; i < max_rates; i++) {
        if ((i < ARRAY_SIZE(info->control.rates)) && (info->control.rates[i].idx >= 0) && info->control.rates[i].count) {
            if (rates != info->control.rates)
                rates[i] = info->control.rates[i];

        } else if (ratetbl) {
            rates[i].idx = ratetbl->rate[i].idx;
            rates[i].flags = ratetbl->rate[i].flags;
            rates[i].count = 2;//ratetbl->rate[i].count;

        } else {
            rates[i].idx = -1;
            rates[i].count = 0;
        }

        if (rates[i].idx < 0 || !rates[i].count)
            break;
    }

    rates[3].idx = -1;
    rates[3].count = 0;
    rates[3].flags  = 0;
}

static int check_is_rate_fitable(struct wifi_station *sta, struct ieee80211_tx_info *info, void *priv_sta) {
    struct minstrel_ht_sta_priv *msp = priv_sta;
    struct minstrel_ht_sta *mi = &msp->ht;
    int max_rate = 0;
    int i;
    unsigned char fitable_bw = 0;
    unsigned char bw = 0;
    int rate_index = -1;

    for (i = 0; i < IEEE80211_TX_RATE_TABLE_SIZE; i++) {
        if ((info->control.rates[i].idx >= 0) && (info->control.rates[i].count)) {
            bw = info->control.rates[i].flags & IEEE80211_TX_RC_80_MHZ_WIDTH ? BW_80
                : info->control.rates[i].flags & IEEE80211_TX_RC_40_MHZ_WIDTH ? BW_40 : BW_20;
            rate_index = info->control.rates[i].idx;

        } else {
            continue;
        }
    }

    if (rate_index == -1)
        return -1;

    fitable_bw = get_fitable_bw(sta);
    if (bw < fitable_bw) {
        AML_PRINT(AML_DBG_MODULES_RATE_CTR, "bandwidth too low, no need to sample. bw:%d, fitable_bw:%d\n", bw, fitable_bw);
        return -1;
    }

    max_rate = get_fitable_mcs_rate(sta, bw);
    if (max_rate < rate_index) {
        AML_PRINT(AML_DBG_MODULES_RATE_CTR, "snr or rssi not fit, rssi:%d, snr:%d, max_rate:%d, rate_index:%d\n",
             sta->sta_avg_bcn_rssi, sta->sta_avg_snr, max_rate, rate_index);
        minstrel_clear_unfitable_rate_stats(mi, max_rate);
        return -1;

    } else {
        return 0;
    }
}

static int minstrel_rate_index_to_vendor_rate_code(int minstrel_rate_idx, struct ieee80211_sta_aml *p_ieee80211_sta)
{
    enum ieee80211_band band = (enum ieee80211_band)g_aml_rate_adaptation_dev.sband->band;

    if (p_ieee80211_sta->vht_cap.vht_supported && ((minstrel_rate_idx >= 0) && (minstrel_rate_idx <= 9))) {
        return WIFINET_RATE_VHT_MCS + minstrel_rate_idx;

    } else if (p_ieee80211_sta->ht_cap.ht_supported && ((minstrel_rate_idx >= 0) && (minstrel_rate_idx <= 7))) {
        return WIFINET_RATE_MCS + minstrel_rate_idx;

    } else {
        if (band == IEEE80211_BAND_5GHZ) {
            /*because 5G band haven't 11b rate ,so need plus 4*/
            return minstrel_rate_idx += 4;

        } else {
            return  minstrel_rate_idx;
        }
    }

    AML_OUTPUT("rate convert error\n");
    return 0;
}

static unsigned int protocol_rate_to_vendor_rate(unsigned int protocol_rate)
{

    //For 11b: (0x82 -0x80) * 500K =  1M
    //For 11g:  0xc * 500k = 6M
    unsigned int ret = 0;
    //DPRINTF(AML_DEBUG_RATE,"rate 0x%x %s(%d)\n", protocol_rate, __func__, __LINE__);
    switch(protocol_rate)
    {
        case  0x02:// 1M
            ret = WIFI_11B_1M;
            break;
        case   0x04:// 2M
             ret = WIFI_11B_2M;
            break;
        case   0x0b://5.5M
            ret = WIFI_11B_5M;
            break;
        case   0x16://11M
             ret = WIFI_11B_11M;
            break;
        case   0x0c://6M
             ret = WIFI_11G_6M;
            break;
        case   0x12://9M
             ret = WIFI_11G_9M;
            break;
        case   0x18://12M
            ret = WIFI_11G_12M;
            break;
        case   0x24: //18M
            ret = WIFI_11G_18M;
            break;
        case    0x30: //24M
            ret = WIFI_11G_24M;
            break;
        case   0x48:// 36M
            ret = 9;
            break;
        case    0x60:// 48M
            ret = WIFI_11G_48M;
            break;
        case    0x6c:// 54M
             ret = WIFI_11G_54M ;
            break;
        default:
            ERROR_DEBUG_OUT("protocol rate to vendor rate convert errore protocol_rate = 0x%x \n", protocol_rate);
            ret =  0;
            break;

    }

     return ret;
}


unsigned char minstrel_find_rate(
    struct aml_ratecontrol ratectrl[]
#ifndef AUTO_RATE_SIM
,
   void *p_sta
#endif
)
{
    struct ieee80211_tx_info tx_info;
    struct ieee80211_tx_info *info = &tx_info;
    struct wifi_station *sta  = (struct wifi_station *)p_sta ;
    int i;
    struct minstrel_rate_control_ops* p_rate_control_ops = NULL;
    void *priv_sta = NULL;
    int mcs_rate = 0;
    struct ieee80211_sta_aml *p_ieee_sta;
    struct minstrel_ht_sta_priv *p_minstrel_ht_sta_priv = NULL;
    struct minstrel_sta_info *p_minstrel_sta_info = NULL;

#ifdef AUTO_RATE_SIM
    mcs_rate = g_rate_mode;
    p_ieee_sta = &g_sta;
    p_minstrel_ht_sta_priv = g_minstrel_ht_sta_priv;
    p_minstrel_sta_info = g_minstrel_sta_info;
#else
    p_ieee_sta = &(sta->ieee_sta);
    p_minstrel_ht_sta_priv = sta->sta_minstrel_ht_priv;
    p_minstrel_sta_info = sta->sta_minstrel_info;

    if (!sta->minstrel_init_flag) {
        ERROR_DEBUG_OUT("minstrel not init, sta:%p\n", sta);
        return 0;
    }

    if ((sta->sta_flags & WIFINET_NODE_HT) || (sta->sta_flags & WIFINET_NODE_VHT)) {
        mcs_rate = 1;
    }

    if (sta->sta_wnet_vif->vm_fixed_rate.mode == WIFINET_FIXED_RATE_MCS) {
        if ((sta->sta_wnet_vif->vm_fixed_rate.rateinfo) & 0x80) {
            g_minstel_pri->fixed_rate_idx = sta->sta_wnet_vif->vm_fixed_rate.rateinfo;
        } else {
            g_minstel_pri->fixed_rate_idx = protocol_rate_to_vendor_rate(sta->sta_wnet_vif->vm_fixed_rate.rateinfo);
        }

    } else {
        g_minstel_pri->fixed_rate_idx = ((u32) -1);
    }
#endif

    memset(&tx_info, 0,sizeof(struct ieee80211_tx_info));
    for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
        info->control.rates[i].idx = -1;
    }

    if (g_minstel_pri->fixed_rate_idx != ((u32) -1)) {
        for (i = 0; i < IEEE80211_TX_MAX_RATES-1; i++) {
            ratectrl[i].vendor_rate_code = g_minstel_pri->fixed_rate_idx;
            ratectrl[i].rate_index = g_minstel_pri->fixed_rate_idx&0xf;
            info->control.rates[i].idx = ratectrl[i].rate_index;
            ratectrl[i].flags |= HAL_RATECTRL_USE_FIXED_RATE;
            ratectrl[i].bw = (sta->sta_chbw < WIFINET_BWC_WIDTH80) ? sta->sta_chbw : IS_HT_RATE(ratectrl[i].vendor_rate_code) ? WIFINET_BWC_WIDTH40 : sta->sta_chbw;
            ratectrl[i].trynum = 2;
            ratectrl[i].maxampdulen = max_4ms_framelen[0][HT_RC_2_MCS(ratectrl[i].vendor_rate_code )];
        }

        sta->sta_vendor_bw = ratectrl[0].bw;
        sta->sta_vendor_rate_code = ratectrl[0].vendor_rate_code;

        ratectrl[0].trynum = 2;
        ratectrl[1].trynum = 2;
        ratectrl[2].trynum = 3;

        return 1;
    }

    if (mcs_rate > 0) {
        p_rate_control_ops =  get_rate_control_ops_ht();
        priv_sta = p_minstrel_ht_sta_priv;

    } else {
        p_rate_control_ops =  get_rate_control_ops();
        priv_sta = p_minstrel_sta_info;
    }

    if (priv_sta == NULL) {
        AML_OUTPUT("sta->sta_flags=%08x, sta:%p\n", sta->sta_flags, sta);
    }

    p_rate_control_ops->get_rate(g_minstel_pri, p_ieee_sta, priv_sta, info);
    if (mcs_rate && check_is_rate_fitable(sta, info, priv_sta)) {
        memset(&tx_info, 0,sizeof(struct ieee80211_tx_info));
        for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
            info->control.rates[i].idx = -1;
        }
    }

    rate_control_fill_sta_table(p_ieee_sta, info, info->control.rates, ARRAY_SIZE(info->control.rates) - 1);
    for (i = 0; i < ARRAY_SIZE(info->control.rates); i++) {
        ratectrl[i].rate_index = info->control.rates[i].idx;
        ratectrl[i].shortgi_en = info->control.rates[i].flags & IEEE80211_TX_RC_SHORT_GI ? 1: 0;
        ratectrl[i].vendor_rate_code = minstrel_rate_index_to_vendor_rate_code(ratectrl[i].rate_index, p_ieee_sta);
        ratectrl[i].trynum = info->control.rates[i].count;
        ratectrl[i].flags = info->control.rates[i].flags;
        ratectrl[i].bw = info->control.rates[i].flags & IEEE80211_TX_RC_80_MHZ_WIDTH ? BW_80
            : info->control.rates[i].flags & IEEE80211_TX_RC_40_MHZ_WIDTH ? BW_40 : BW_20;

        if (IS_MCS_RATE(ratectrl[i].vendor_rate_code)) {
            ratectrl[i].maxampdulen = max_4ms_framelen[0][HT_RC_2_MCS(ratectrl[i].vendor_rate_code)];
        }

        if (info->control.rates[i].idx < 0) {
            continue;
        }
        //AML_OUTPUT("ratectrl[%d].rate_index =%d, vendor_rate_code =0x%x, maxampdulen=%d, flags=%x\n", i,
        //    ratectrl[i].rate_index,ratectrl[i].vendor_rate_code, ratectrl[i].maxampdulen, info->control.rates[i].flags);
    }

    if (info->flags & IEEE80211_TX_CTL_RATE_CTRL_PROBE) {
        ratectrl[0].flags |= HAL_RATECTRL_USE_SAMPLE_RATE;
    }

    ratectrl[0].trynum = 2;
    ratectrl[1].trynum = 2;
    ratectrl[2].trynum = 96;

    sta->sta_vendor_bw = ratectrl[0].bw;
    sta->sta_vendor_rate_code = ratectrl[0].vendor_rate_code;

    return 1;
}

void minstrel_tx_complete(
    struct aml_ratecontrol *rc
#ifndef AUTO_RATE_SIM
, void *p_sta
#endif
)
{
    void *priv_sta = NULL;
    struct wifi_station *sta = (struct wifi_station *)p_sta;
    struct minstrel_rate_control_ops *p_rate_control_ops = NULL;
    struct minstrel_ht_sta_priv *p_minstrel_ht_sta_priv = NULL;
    struct minstrel_sta_info *p_minstrel_sta_info = NULL;
    struct ieee80211_tx_info info;
    struct ieee80211_tx_rate *ar = info.status.rates;
    int i = 0;
    int mcs_rate = 0;

#ifdef AUTO_RATE_SIM
    mcs_rate = g_rate_mode;
    p_minstrel_ht_sta_priv = g_minstrel_ht_sta_priv;
    p_minstrel_sta_info  =   g_minstrel_sta_info;
#else
    p_minstrel_ht_sta_priv = sta->sta_minstrel_ht_priv;
    p_minstrel_sta_info = sta->sta_minstrel_info;

    if (!sta->minstrel_init_flag) {
        ERROR_DEBUG_OUT("minstrel not init, sta:%p\n", sta);
        return;
    }

    if ((sta->sta_flags & WIFINET_NODE_HT) || (sta->sta_flags & WIFINET_NODE_VHT)) {
        mcs_rate = 1;
    }
#endif


    memset(&info, 0, sizeof(struct ieee80211_tx_info));
    if (rc[0].flags & HAL_RATECTRL_TX_SEND_SUCCESS) {
        info.flags |= IEEE80211_TX_STAT_ACK;
        rc[0].flags &= ~HAL_RATECTRL_TX_SEND_SUCCESS;
    }

    for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
        ar[i].idx = -1;
        ar[i].count = 0;
        ar[i].flags = 0;

        if (rc[i].trynum != 0) {//not use if trynum is 0
            ar[i].count = rc[i].trynum;
            ar[i].idx   = rc[i].rate_index;
            ar[i].flags = rc[i].flags;
            /*only used in vht debug*/
            //ar[i].flags |= IEEE80211_TX_RC_VHT_MCS;
        }
    }

    if (mcs_rate > 0) {
        p_rate_control_ops =  get_rate_control_ops_ht();
        if (p_minstrel_ht_sta_priv == NULL) {
            return;
        }
        priv_sta = p_minstrel_ht_sta_priv;

    } else {
        p_rate_control_ops =  get_rate_control_ops();
        if (p_minstrel_sta_info == NULL) {
            return;
        }
        priv_sta = p_minstrel_sta_info;
    }

    p_rate_control_ops->tx_status(g_minstel_pri, g_aml_rate_adaptation_dev.sband,  priv_sta, &info);
}

static void  minstrel_set_sta_bandwidth( int bw )
{
	g_sta.bandwidth = bw;
}

