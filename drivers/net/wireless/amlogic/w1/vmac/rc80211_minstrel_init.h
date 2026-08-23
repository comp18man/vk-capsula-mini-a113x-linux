#ifndef RC80211_MINSTREL_INIT_H
#define RC80211_MINSTREL_INIT_H

#include "linux/types.h"

struct aml_rate_adaptation_dev {
    struct ieee80211_supported_band *sband;
    struct cfg80211_chan_def *chandef;
    struct minstrel_priv *p_minstrel_priv;
    u32 ht_cap_info;
    u32 vht_cap_info;
    u32 num_rf_chains;
};

/* HT Capabilities*/
#define WMI_HT_CAP_RX_STBC_MASK_SHIFT     4
#define WMI_HT_CAP_ENABLED                0x0001   /* HT Enabled/ disabled */
#define WMI_HT_CAP_TX_STBC                0x0002   /* B1 TX STBC */
#define WMI_HT_CAP_DYNAMIC_SMPS           0x0004   /* Dynamic MIMO powersave */
#define WMI_HT_CAP_HT20_SGI               0x0020   /* Short Guard Interval with HT20 */
#define WMI_HT_CAP_HT40_SGI               0x0040
#define WMI_HT_CAP_L_SIG_TXOP_PROT        0x0080   /* L-SIG TXOP Protection */
#define WMI_HT_CAP_LDPC                   0x0200   /* LDPC supported */
#define WMI_HT_CAP_RX_STBC                0x0C00   /* B10-B11 RX STBC */
#define WMI_HT_CAP_MPDU_DENSITY           0x7000   /* MPDU Density */

/*
 * WMI_VHT_CAP_* these maps to ieee 802.11ac vht capability information
 * field. The fields not defined here are not supported, or reserved.
 * Do not change these masks and if you have to add new one follow the
 * bitmask as specified by 802.11ac draft.
 */

#define WMI_VHT_CAP_MAX_MPDU_LEN_MASK            0x00000003
#define WMI_VHT_CAP_RX_LDPC                      0x00000010
#define WMI_VHT_CAP_SGI_80MHZ                    0x00000020
#define WMI_VHT_CAP_TX_STBC                      0x00000080
#define WMI_VHT_CAP_RX_STBC_MASK                 0x00000300
#define WMI_VHT_CAP_MAX_AMPDU_LEN_EXP            0x03800000
#define WMI_VHT_CAP_RX_FIXED_ANT                 0x10000000
#define WMI_VHT_CAP_TX_FIXED_ANT                 0x20000000

/* The following also refer for max HT AMSDU */
#define WMI_VHT_CAP_MAX_MPDU_LEN_11454           0x00000002

/*
 * Interested readers refer to Rx/Tx MCS Map definition as defined in
 * 802.11ac
 */
#define WMI_VHT_MAX_SUPP_RATE_MASK           0x1fff0000

#ifdef AUTO_RATE_SIM
#define RATETAB_ENT(_rate, _rateid, _flags) {   \
	.bitrate    = (_rate),                  \
	.flags      = (_flags),                 \
	.hw_value   = (_rateid),                \
}

#define CHAN2G(_channel, _freq, _flags) {   \
	.band           = IEEE80211_BAND_2GHZ,  \
	.hw_value       = (_channel),           \
	.center_freq    = (_freq),              \
	.flags          = (_flags),             \
	.max_antenna_gain   = 0,                \
	.max_power      = 30,                   \
}

#define CHAN5G(_channel, _flags) {		    \
	.band           = IEEE80211_BAND_5GHZ,      \
	.hw_value       = (_channel),               \
	.center_freq    = 5000 + (5 * (_channel)),  \
	.flags          = (_flags),                 \
	.max_antenna_gain   = 0,                    \
	.max_power      = 30,                       \
}
#endif  
#define aml_legacy_rates_size    12
#define aml_g_htcap IEEE80211_HT_CAP_SGI_20
#define aml_a_htcap (IEEE80211_HT_CAP_SUP_WIDTH_20_40 | \
			IEEE80211_HT_CAP_SGI_20		 | \
			IEEE80211_HT_CAP_SGI_40)

void aml_minstrel_attach( void);
void aml_minstrel_detach(void);

void aml_minstrel_init( 
#ifdef AUTO_RATE_SIM    
    void
#else    
    void *p_sta
#endif     
);
void aml_minstrel_deinit(void *p_sta);

unsigned char minstrel_find_rate(
    struct aml_ratecontrol ratectrl[]
#ifndef AUTO_RATE_SIM
,
   void *p_sta
#endif 
);
void minstrel_tx_complete( 
    struct aml_ratecontrol *rc
#ifndef AUTO_RATE_SIM
,
   void *p_sta
#endif 
);

#endif
