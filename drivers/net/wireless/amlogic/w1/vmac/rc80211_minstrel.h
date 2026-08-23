/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_MINSTREL_H
#define __RC_MINSTREL_H

#include "linux/types.h"

#define EWMA_LEVEL	96	/* ewma weighting factor [/EWMA_DIV] */
#define EWMA_DIV	128
#define SAMPLE_COLUMNS	10	/* number of columns in sample table */


/* scaled fraction values */
#define MINSTREL_SCALE  12
#define MINSTREL_FRAC(val, div) (((val) << MINSTREL_SCALE) / div)
#define MINSTREL_TRUNC(val) ((val) >> MINSTREL_SCALE)

/* number of highest throughput rates to consider*/
#define MAX_THR_RATES 4

#define SAMPLE_TBL(_mi, _idx, _col) \
		_mi->sample_table[(_idx * SAMPLE_COLUMNS) + _col]

/* Maximal size of an A-MSDU that can be transported in a HT BA session */

/*
 * Perform EWMA (Exponentially Weighted Moving Average) calculation
 */
static inline int
minstrel_ewma(int old, int new, int weight)
{
	int diff, incr;

	diff = new - old;
	incr = (EWMA_DIV - weight) * diff / EWMA_DIV;

	return old + incr;
}

/*
 * Perform EWMV (Exponentially Weighted Moving Variance) calculation
 */
static inline int
minstrel_ewmv(int old_ewmv, int cur_prob, int prob_ewma, int weight)
{
	int diff, incr;

	diff = cur_prob - prob_ewma;
	incr = (EWMA_DIV - weight) * diff / EWMA_DIV;
	return weight * (old_ewmv + MINSTREL_TRUNC(diff * incr)) / EWMA_DIV;
}

struct minstrel_rate_stats {
	/* current / last sampling period attempts/success counters */
	u32 attempts, last_attempts;
	u32 success, last_success;

	/* total attempts/success counters */
	u32 att_hist, succ_hist;

	/* statistics of packet delivery probability
	 *  prob_ewma - exponential weighted moving average of prob
	 *  prob_ewmsd - exp. weighted moving standard deviation of prob */
	u16 prob_ewma;
	u16 prob_ewmv;

	/* maximum retry counts */
	u8 retry_count;
	u8 retry_count_rtscts;

	bool retry_updated;
	int tp_avg;
};


struct ieee80211_sta_aml {
	u32 supp_rates[NUM_NL80211_BANDS];
	u8 addr[ETH_ALEN];
	u16 aid;
	struct ieee80211_sta_ht_cap ht_cap;
	struct ieee80211_sta_vht_cap vht_cap;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
	struct ieee80211_sta_he_cap he_cap;
	struct ieee80211_he_6ghz_capa he_6ghz_capa;
#endif
	u16 max_rx_aggregation_subframes;
	bool wme;
	u8 uapsd_queues;
	u8 max_sp;
	u8 rx_nss;
	enum ieee80211_sta_rx_bandwidth bandwidth;
	enum ieee80211_smps_mode smps_mode;
	struct ieee80211_sta_rates __rcu *rates;
	bool tdls;
	bool tdls_initiator;
	bool mfp;
	u8 max_amsdu_subframes;

	u16 max_amsdu_len;
	bool support_p2p_ps;
	u16 max_rc_amsdu_len;
	u16 max_tid_amsdu_len[IEEE80211_NUM_TIDS];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
	struct ieee80211_sta_txpwr txpwr;
#endif

	struct ieee80211_txq *txq[IEEE80211_NUM_TIDS + 1];

	/* must be last */
	u8 drv_priv[] __aligned(sizeof(void *));
};

static inline int rate_supported_aml(struct ieee80211_sta_aml *sta,
				 enum nl80211_band band,
				 int index)
{
	return (sta == NULL || sta->supp_rates[band] & BIT(index));
}


static inline s8
rate_lowest_index_aml(struct ieee80211_supported_band *sband,
		  struct ieee80211_sta_aml *sta)
{
	int i;

	for (i = 0; i < sband->n_bitrates; i++)
		if (rate_supported_aml(sta, sband->band, i))
			return i;

	/* and return 0 (the lowest index) */
	return 0;
}



struct minstrel_rate {
	int bitrate;

	s8 rix;
	u8 retry_count_cts;
	u8 adjusted_retry_count;

	unsigned int perfect_tx_time;
	unsigned int ack_time;

	int sample_limit;

	struct minstrel_rate_stats stats;
};

struct minstrel_sta_info {
    struct ieee80211_sta_aml *sta;

    unsigned long last_stats_update;
    unsigned int sp_ack_dur;
    unsigned int rate_avg;

    unsigned int lowest_rix;

    u8 max_tp_rate[MAX_THR_RATES];
    u8 max_prob_rate;
    unsigned int total_packets;
    unsigned int total_mpdu_num;
    unsigned int sample_packets;
    int sample_deferred;

    unsigned int sample_row;
    unsigned int sample_column;

    int n_rates;
    struct minstrel_rate *r;
    bool prev_sample;

    /* sampling table */
    u8 *sample_table;

#ifdef CONFIG_MAC80211_DEBUGFS
    struct dentry *dbg_stats;
#endif
};

struct minstrel_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int cw_min;
	unsigned int cw_max;
	unsigned int max_retry;
	unsigned int segment_size;
	unsigned int update_interval;
	unsigned int lookaround_rate;
	unsigned int lookaround_rate_mrr;

	u8 cck_rates[4];
	u32 fixed_rate_idx;

};

struct minstrel_debugfs_info {
	size_t len;
	char buf[];
};

struct minstrel_rate_control_ops {
    struct module *module;
    const char *name;
    void *(*alloc)(struct ieee80211_hw *hw);
    void (*free)(void *priv);

    void *(*alloc_sta)(void *priv, struct ieee80211_sta_aml *sta, gfp_t gfp);
    void (*rate_init)(void *priv, struct ieee80211_supported_band *sband, struct ieee80211_sta_aml *sta, void *priv_sta);
    void (*rate_update)(void *priv, struct ieee80211_supported_band *sband, struct ieee80211_sta_aml *sta, void *priv_sta, u32 changed);
    void (*free_sta)(void *priv_sta);

    void (*tx_status)(void *priv, struct ieee80211_supported_band *sband, void *priv_sta,struct ieee80211_tx_info *info);
    void (*get_rate)(void *priv, struct ieee80211_sta_aml *sta, void *priv_sta, struct ieee80211_tx_info *info);
    u32 (*get_expected_throughput)(void *priv_sta);
};

extern  struct minstrel_rate_control_ops mac80211_minstrel;
/* Recalculate success probabilities and counters for a given rate using EWMA */
void minstrel_calc_rate_stats(struct minstrel_rate_stats *mrs);
int minstrel_get_tp_avg(struct minstrel_rate *mr, int prob_ewma);
struct minstrel_rate_control_ops* get_rate_control_ops(void);
int ieee80211_frame_duration(enum ieee80211_band band, size_t len, int rate, int erp, int short_preamble, int shift);

#endif
