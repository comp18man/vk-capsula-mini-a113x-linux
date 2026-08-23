/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac amsdu module
 *
 *
 ****************************************************************************************
 */
#ifndef _IEEE80211_AMSDU_H_
#define _IEEE80211_AMSDU_H_

#define WIFINET_AMSDU_LOCK_INIT(_wm) spin_lock_init(&(_wm)->wm_amsdu_txq_lock)
#define WIFINET_AMSDU_LOCK_DESTROY(_wm)
#define WIFINET_AMSDU_LOCK(_wm) OS_SPIN_LOCK(&(_wm)->wm_amsdu_txq_lock)
#define WIFINET_AMSDU_UNLOCK(_wm) OS_SPIN_UNLOCK(&(_wm)->wm_amsdu_txq_lock)

#define WIFINET_AMSDU_TASKLET_LOCK_INIT(_ic) spin_lock_init(&(_ic)->wm_amsdu_tasklet_lock)
#define WIFINET_AMSDU_TASKLET_LOCK(_ic) OS_SPIN_LOCK(&(_ic)->wm_amsdu_tasklet_lock)
#define WIFINET_AMSDU_TASKLET_UNLOCK(_ic) OS_SPIN_UNLOCK(&(_ic)->wm_amsdu_tasklet_lock)


struct wifi_mac_msdu_node
{
    struct list_head msdu_node;
    struct sk_buff *skbbuf;
};

struct wifi_mac_amsdu_tx
{
    struct sk_buff *amsdu_tx_buf;
    struct list_head amsdu_qelem;
    unsigned int b_work;
    int amsdunum;
    int framelen;
    struct list_head msdu_list;
    /*record the time when the buffer first msdu in ' msdu_tmp_buf ' */
    unsigned long in_time;
    struct sk_buff *msdu_tmp_buf[DEFAULT_TXAMSDU_SUB_MAX_BW80 * 2];
    int tid;
    unsigned char vid;
};

struct wifi_mac_amsdu
{
    struct wifi_mac_amsdu_tx amsdutx[WME_NUM_TID];
    unsigned int amsdu_max_length ;
    unsigned int amsdu_max_sub;
};

#define AMSDU_SIZE_3839 3839
#define AMSDU_SIZE_7935 7935
#define VHT_MPDU_SIZE_3895 3895
#define VHT_MPDU_SIZE_7991 7991
#define VHT_MPDU_SIZE_11451 11451

#define AMSDU_MAX_SUBFRM_LEN 2304
#define AMSDU_BUFFER_HEADROOM 64
#define AMSDU_MAX_BUFFER_SIZE (4000)
#define AMSDU_MAX_LEN (AMSDU_MAX_BUFFER_SIZE - AMSDU_BUFFER_HEADROOM)

#define AMSDU_MAX_LIVE_TIME 10 /* ms */
#define MAX_MSDU_CNT ((HI_AGG_TXD_NUM_PER_QUEUE + WIFI_MAX_TXFRAME/WIFI_MAX_TID) * 4)

#define AMSDU_MAX_BUFFER_SIZE_BW80 (7935)
#define AMSDU_MAX_LEN_BW80 (AMSDU_MAX_BUFFER_SIZE_BW80 - AMSDU_BUFFER_HEADROOM)
#define AMSDU_SUBFRAME_TOTAL_LEN(len) (roundup((len)+LLC_SNAPFRAMELEN, 4))

int wifi_mac_txamsdu_task(void *arg);
int  wifi_mac_amsdu_attach(struct wifi_mac *wifimac);
void wifi_mac_amsdu_detach(struct wifi_mac *wifimac);
int  wifi_mac_alloc_amsdu_node(struct wifi_mac *wifimac, unsigned char vid, struct wifi_station *ansta);
void wifi_mac_amsdu_nsta_free(struct wifi_mac *wifimac, struct wifi_station *ansta);
struct sk_buff *wifi_mac_amsdu_send( struct sk_buff *skbbuf);
struct sk_buff *wifi_mac_amsdu_aggr(struct wifi_mac *wifimac, struct wifi_mac_amsdu_tx *amsdutx);
struct sk_buff *wifi_mac_amsdu_ex( struct sk_buff *skbbuf);
void wifi_mac_amsdu_complete_wbuf( struct sk_buff *skbbuf);
void wifi_mac_txamsdu_free_all(struct wifi_mac *wifimac, unsigned char vid);

#endif /* _IEEE80211_AMSDU_H_ */
