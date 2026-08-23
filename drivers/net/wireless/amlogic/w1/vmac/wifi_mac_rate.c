#include "wifi_mac_com.h"

void wifi_mac_ap_set_basic_rates(struct wlan_net_vif *wnet_vif, enum wifi_mac_macmode macmode)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned char need_set_11b = 0;
    unsigned char need_set_11g = 0;

    switch (macmode) {
        case WIFINET_MODE_11B:
            need_set_11b = 1;
            break;

        case WIFINET_MODE_11G:
        case WIFINET_MODE_11GN:
        case WIFINET_MODE_11GNAC:
            need_set_11g =1;
            break;

        case WIFINET_MODE_11BG:
        case WIFINET_MODE_11BGN:
            need_set_11b = 1;
            need_set_11g =1;
            break;

        default:
            return;
    }

    if (need_set_11b) {
        wifimac->wm_11b_rates.dot11_rate[0] = 0x82;
        wifimac->wm_11b_rates.dot11_rate[1] = 0x84;
        wifimac->wm_11b_rates.dot11_rate[2] = 0x0b;
        wifimac->wm_11b_rates.dot11_rate[3] = 0x16;
    }

    if (need_set_11g) {
        wifimac->wm_11g_rates.dot11_rate[0] = 0x8c;
        wifimac->wm_11g_rates.dot11_rate[1] = 0x12;
        wifimac->wm_11g_rates.dot11_rate[2] = 0x98;
        wifimac->wm_11g_rates.dot11_rate[3] = 0x24;
        wifimac->wm_11g_rates.dot11_rate[4] = 0xb0;
        wifimac->wm_11g_rates.dot11_rate[5] = 0x48;
        wifimac->wm_11g_rates.dot11_rate[6] = 0x60;
        wifimac->wm_11g_rates.dot11_rate[7] = 0x6c;
    }
}

void wifi_mac_set_legacy_rates(struct wifi_mac_rateset *rs, struct wlan_net_vif *wnet_vif)
{
    unsigned char nxrates;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_rateset *srs;

    DPRINTF(AML_DEBUG_CONNECT, "%s(%d): phy mode=%d\n", __func__, __LINE__, wnet_vif->vm_mac_mode);
    KASSERT(wnet_vif->vm_mac_mode < WIFINET_MODE_MAX, ("invalid mode %u", wnet_vif->vm_mac_mode));

    srs = &wnet_vif->vm_legacy_rates;
    memset(srs, 0, sizeof(struct wifi_mac_rateset));

    if (WIFINET_MODE_11B == wnet_vif->vm_mac_mode) {
        srs->dot11_rate_num = wifimac->wm_11b_rates.dot11_rate_num;
        memcpy(srs->dot11_rate, wifimac->wm_11b_rates.dot11_rate, srs->dot11_rate_num);

    } else if ((WIFINET_MODE_11G == wnet_vif->vm_mac_mode)
        || (WIFINET_MODE_11GN == wnet_vif->vm_mac_mode)
        || (WIFINET_MODE_11GNAC == wnet_vif->vm_mac_mode)) {
        srs->dot11_rate_num = wifimac->wm_11g_rates.dot11_rate_num;
        memcpy(srs->dot11_rate, wifimac->wm_11g_rates.dot11_rate, srs->dot11_rate_num);

    } else if ((WIFINET_MODE_11BG == wnet_vif->vm_mac_mode)
        || (WIFINET_MODE_11BGN == wnet_vif->vm_mac_mode)) {
        srs->dot11_rate_num = wifimac->wm_11b_rates.dot11_rate_num;
        memcpy(srs->dot11_rate, wifimac->wm_11b_rates.dot11_rate, srs->dot11_rate_num);

        nxrates = wifimac->wm_11g_rates.dot11_rate_num;
        if (srs->dot11_rate_num + wifimac->wm_11g_rates.dot11_rate_num > WIFINET_RATE_MAXSIZE) {
            nxrates = WIFINET_RATE_MAXSIZE - srs->dot11_rate_num;
        }

        memcpy(srs->dot11_rate + srs->dot11_rate_num, wifimac->wm_11g_rates.dot11_rate, nxrates);
        srs->dot11_rate_num += nxrates;

    } else {
        return;
    }
}

static void wifi_mac_check_ap_basic_rate(unsigned char *rate, unsigned char *ap_rate, unsigned char rate_num, enum wifi_mac_macmode macmode)
{
    int i, j, index;
    const struct wifi_mac_rateset basic_rate[] =
    {
        { 4, { 0x82, 0x84, 0x8b, 0x96} },
        { 8, { 0x8c, 0x92, 0x98, 0xa4, 0xb0, 0xc8, 0xe0, 0xec} },
    };

    if (rate_num == 0) {
        return;
    }

    switch (macmode) {
        case WIFINET_MODE_11B:
            index = 0;
            break;

        case WIFINET_MODE_11G:
            index = 1;
            break;

        default:
            return;
    }

    for (i = 0; i < basic_rate[index].dot11_rate_num; i++) {
        for (j = 0; j < rate_num; j++) {
            if (basic_rate[index].dot11_rate[i] == ap_rate[j]) {
                rate[i] = basic_rate[index].dot11_rate[i];
                break;
            }
        }
    }
    return;
}

void wifi_mac_sta_set_basic_rates(struct wlan_net_vif *wnet_vif, char *si_rates, char *si_exrates)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    wifimac->wm_11b_rates.dot11_rate[0] = 0x02;
    wifimac->wm_11b_rates.dot11_rate[1] = 0x04;
    wifimac->wm_11b_rates.dot11_rate[2] = 0x0b;
    wifimac->wm_11b_rates.dot11_rate[3] = 0x16;

    wifimac->wm_11g_rates.dot11_rate[0] = 0x0c;
    wifimac->wm_11g_rates.dot11_rate[1] = 0x12;
    wifimac->wm_11g_rates.dot11_rate[2] = 0x18;
    wifimac->wm_11g_rates.dot11_rate[3] = 0x24;
    wifimac->wm_11g_rates.dot11_rate[4] = 0x30;
    wifimac->wm_11g_rates.dot11_rate[5] = 0x48;
    wifimac->wm_11g_rates.dot11_rate[6] = 0x60;
    wifimac->wm_11g_rates.dot11_rate[7] = 0x6c;

    if (wnet_vif->vm_opmode == WIFINET_M_STA) {
        if ((WIFINET_MODE_11B == wnet_vif->vm_mac_mode)) {
            if (si_rates[1] != 0) {
                wifi_mac_check_ap_basic_rate(wifimac->wm_11b_rates.dot11_rate, si_rates + 2, si_rates[1], WIFINET_MODE_11B);

            } else {
                wifimac->wm_11b_rates.dot11_rate[0] = 0x82;
                wifimac->wm_11b_rates.dot11_rate[1] = 0x84;
            }

        } else if ((WIFINET_MODE_11G == wnet_vif->vm_mac_mode) || (WIFINET_MODE_11GN == wnet_vif->vm_mac_mode)
            || (WIFINET_MODE_11GNAC == wnet_vif->vm_mac_mode)) {
            if (si_exrates[1] == 0) {
                if (si_rates[1] != 0) {
                    wifi_mac_check_ap_basic_rate(wifimac->wm_11g_rates.dot11_rate, si_rates + 2, si_rates[1], WIFINET_MODE_11G);

                } else {
                    wifimac->wm_11g_rates.dot11_rate[0] = 0x8c;
                    wifimac->wm_11g_rates.dot11_rate[2] = 0x98;
                    wifimac->wm_11g_rates.dot11_rate[4] = 0xb0;
                }

            } else {
                if (si_rates[1] != 0) {
                    memcpy(si_exrates + 2 + si_exrates[1], si_rates + 2, si_rates[1]);
                    si_exrates[1] += si_rates[1];
                }

                wifi_mac_check_ap_basic_rate(wifimac->wm_11g_rates.dot11_rate, si_exrates + 2, si_exrates[1], WIFINET_MODE_11G);
            }

        } else if ((WIFINET_MODE_11BG == wnet_vif->vm_mac_mode)
            || (WIFINET_MODE_11BGN == wnet_vif->vm_mac_mode)) {
            if (si_rates[1] != 0 || si_exrates[1] != 0) {
                if (si_rates[1] != 0) {
                    memcpy(si_exrates + 2 + si_exrates[1], si_rates + 2, si_rates[1]);
                    si_exrates[1] += si_rates[1];
                }

                wifi_mac_check_ap_basic_rate(wifimac->wm_11b_rates.dot11_rate, si_exrates + 2, si_exrates[1], WIFINET_MODE_11B);
                wifi_mac_check_ap_basic_rate(wifimac->wm_11g_rates.dot11_rate, si_exrates + 2, si_exrates[1], WIFINET_MODE_11G);

            } else {
                wifimac->wm_11b_rates.dot11_rate[0] = 0x82;
                wifimac->wm_11b_rates.dot11_rate[1] = 0x84;

                wifimac->wm_11g_rates.dot11_rate[0] = 0x8c;
                wifimac->wm_11g_rates.dot11_rate[2] = 0x98;
                wifimac->wm_11g_rates.dot11_rate[4] = 0xb0;
            }
        } else {
            return;
        }
    }

    return;
}

int check_rate(struct wlan_net_vif *wnet_vif, const struct wifi_scan_info *scaninfo)
{
    int i, j;
    int tmpflags =0;
    unsigned char nxrates;
    struct wifi_mac_rateset *srs;
    struct wifi_mac_rateset rrs;

    srs = &wnet_vif->vm_legacy_rates;
    memset(&rrs, 0, sizeof(rrs));
    rrs.dot11_rate_num = scaninfo->SI_rates[1];
    memcpy(rrs.dot11_rate, scaninfo->SI_rates + 2, rrs.dot11_rate_num);

    if (scaninfo->SI_exrates[1] > 0)
    {
        nxrates = scaninfo->SI_exrates[1];
        if (rrs.dot11_rate_num + nxrates > WIFINET_RATE_MAXSIZE)
        {
            nxrates = WIFINET_RATE_MAXSIZE - rrs.dot11_rate_num;
        }

        memcpy(rrs.dot11_rate + rrs.dot11_rate_num, scaninfo->SI_exrates+2, nxrates);
        rrs.dot11_rate_num += nxrates;
    }

    for (i = 0; i < rrs.dot11_rate_num; i++)
    {
        if (rrs.dot11_rate[i] & WIFINET_RATE_BASIC)
        {
            for (j = 0; j < srs->dot11_rate_num; j++)
            {
                if (WIFINET_GET_RATE_VAL(rrs.dot11_rate[i]) == WIFINET_GET_RATE_VAL(srs->dot11_rate[j]))
                {
                    tmpflags = 1;
                    break;
                }
            }
        }
    }

    return 1;
}


int check_ht_rate(struct wlan_net_vif *wnet_vif, const struct wifi_scan_info *scaninfo)
{
    int i, j, k, mac_mode;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_rateset *srs;
    struct wifi_mac_rateset rrs;
    struct wifi_mac_ie_htcap_cmn *htcap;
    struct wifi_mac_ie_htinfo_cmn *htinfo;

    mac_mode = (int)wifi_mac_get_sta_mode((struct wifi_scan_info *)scaninfo);

    k = 0;
    srs = &wifimac->wm_sup_ht_rates;
    memset(&rrs, 0, sizeof(rrs));

    if ((scaninfo->SI_htcap_ie[1] != 0) && (scaninfo->SI_htinfo_ie[1] != 0)) {
        htcap = &((struct wifi_mac_ie_htcap *)scaninfo->SI_htcap_ie)->hc_ie;
        htinfo = &((struct wifi_mac_ie_htinfo *)scaninfo->SI_htinfo_ie)->hi_ie;

        /* only support Spatial Stream 1 (mcs 0~7)*/
        for (i = 0; i < 8; i++) {
            if (htcap->hc_mcsset[i / 8] & (1 << (i % 8))) {
                rrs.dot11_rate[k++] = i | ((htinfo->hi_basicmcsset[i/8] & (1 << (i%8))) ? WIFINET_RATE_BASIC : 0);
            }

            if (k == WIFINET_RATE_MAXSIZE) {
                break;
            }
        }
    }
    rrs.dot11_rate_num = k;

    for (i = 0; i < rrs.dot11_rate_num; i++) {
        if (rrs.dot11_rate[i] & WIFINET_RATE_BASIC) {
            for (j = 0; j < srs->dot11_rate_num; j++) {
                if (WIFINET_GET_RATE_VAL(rrs.dot11_rate[i]) == WIFINET_GET_RATE_VAL(srs->dot11_rate[j])) {
                    break;
                }
            }

            if (j == srs->dot11_rate_num) {
                DPRINTF(AML_DEBUG_SCAN, "%s(%d) error\n", __func__, __LINE__);
                return 0;
            }
        }
    }

    return 1;
}


int wifi_mac_setup_rates(struct wifi_station *sta, const unsigned char *rates, const unsigned char *xrates, int flags)
{
    struct wifi_mac_rateset *rs = &sta->sta_rates;
    struct wifi_mac_rateset rrs, *irs;

    memset(&rrs, 0, sizeof(rrs));
    memset(rs, 0, sizeof(*rs));

    if (rates) {
        rrs.dot11_rate_num = rates[1];
        /* rrs save rates of peer STA/AP */
        memcpy(rrs.dot11_rate, rates + 2, rrs.dot11_rate_num);
    }

    if (xrates != NULL)
    {
        unsigned char nxrates;

        nxrates = xrates[1];
        if (rrs.dot11_rate_num + nxrates > WIFINET_RATE_MAXSIZE)
        {
            nxrates = WIFINET_RATE_MAXSIZE - rrs.dot11_rate_num;
            WIFINET_DPRINTF_STA( AML_DEBUG_RATE, sta, "extended rate set too large; only using %u of %u rates\n", nxrates, xrates[1]);
        }
        memcpy(rrs.dot11_rate + rrs.dot11_rate_num, xrates+2, nxrates);
        rrs.dot11_rate_num += nxrates;
    }
    /* irs pointer local legacy rates*/
    irs = &sta->sta_wnet_vif->vm_legacy_rates;

    if (flags & WIFINET_F_DOSORT)
    {
        /* sort rate array */
        wifi_mac_sort_rate(&rrs);
    }

    if (flags & WIFINET_F_DOXSECT)
    {
        /* get rate intersection of peer and local and saved in rs*/
        wifi_mac_xsect_rate(irs, &rrs, rs);
    }
    else
    {
        memcpy(rs, &rrs, sizeof(rrs));
    }

    /* check if rate intersection include static rate we have fixed */
    if (flags & WIFINET_F_DOFRATE)
        if (!wifi_mac_fixed_rate_check(sta, rs))
            return 0;

    if (flags & WIFINET_F_DOBRS)
        if (!wifi_mac_brs_rate_check(&(sta->sta_wnet_vif->vm_mainsta->sta_rates), rs))
            return 0;

    if (flags & WIFINET_F_DOSORT)
        wifi_mac_sort_rate(rs);

    return 1;
}

static int wifi_mac_mcs_to_numstreams(int mcs)
{
    int numstreams = 0;

    if ((mcs <= 7) || (mcs == 32))
        numstreams = 1;

    if (((mcs >= 8) && (mcs <= 15)) || ((mcs >= 33) && (mcs <= 38)))
        numstreams = 2;

    if (((mcs >= 16) && (mcs <= 23)) || ((mcs >= 39) && (mcs <= 52)))
        numstreams = 3;
    if (((mcs >= 24) && (mcs <= 31)) || ((mcs >= 53) && (mcs <= 76)))
        numstreams = 4;

    return numstreams;
}


int wifi_mac_setup_ht_rates(struct wifi_station *sta, unsigned char *ie,int flags)
{
    int i,j;
    int numstreams;
    struct wifi_mac_ie_htcap *xhtcap =(struct wifi_mac_ie_htcap *)ie;
    struct wifi_mac_ie_htcap_cmn *htcap = NULL;
    struct wifi_mac_rateset *rs = &sta->sta_htrates;
    struct wifi_mac_rateset rrs, *irs;
    struct wifi_mac *wifimac = sta->sta_wmac;

    j = 0;
    numstreams = 0;
    memset(&rrs, 0, sizeof(rrs));
    memset(rs, 0, sizeof(*rs));

    if (xhtcap != NULL)
    {
        htcap = &xhtcap->hc_ie;

        for (i=0; i < WIFINET_HT_RATE_SIZE; i++)
        {
            if (htcap->hc_mcsset[i/8] & (1<<(i%8)) && (i < 7))
            {
                rrs.dot11_rate[j++] = i;
                if (numstreams < wifi_mac_mcs_to_numstreams(i))
                    numstreams = wifi_mac_mcs_to_numstreams(i);
            }

            if (j == WIFINET_RATE_MAXSIZE)
            {
                WIFINET_DPRINTF_STA(AML_DEBUG_RATE, sta, \
                                    "ht extended rate set too large; only using %u rates", j);
                break;
            }
        }
    }

    rrs.dot11_rate_num = j;
    irs = &wifimac->wm_sup_ht_rates;

    /*sort rate for rate from assoc req frames */
    if (flags & WIFINET_F_DOSORT)
        wifi_mac_sort_rate(&rrs);

    /*get the same rate from assoc req and local, and save */
    if (flags & WIFINET_F_DOXSECT)
        wifi_mac_xsect_rate(irs, &rrs, rs);
    else
    {
        memcpy(rs, &rrs, sizeof(rrs));
    }

    if (flags & WIFINET_F_DOFRATE)
        if (!wifi_mac_fixed_rate_check(sta, rs))
            return 0;

    if (flags & WIFINET_F_DOBRS)
        if (!wifi_mac_brs_rate_check(irs, rs))
            return 0;
    return 1;
}


void wifi_mac_setup_basic_ht_rates(struct wifi_station *sta, unsigned char *ie)
{
    int i,j;
    struct wifi_mac_ie_htinfo *xhtinfo =(struct wifi_mac_ie_htinfo *)ie;
    struct wifi_mac_ie_htinfo_cmn *htinfo = &xhtinfo->hi_ie;
    struct wifi_mac_rateset *rs = &sta->sta_htrates;

    if (rs->dot11_rate_num)
    {
        for (i = 0; i < WIFINET_HT_RATE_SIZE; i++)
        {
            if (htinfo->hi_basicmcsset[i/8] & (1 << (i % 8)))
            {
                for (j = 0; j < rs->dot11_rate_num; j++)
                {
                    if (WIFINET_GET_RATE_VAL(rs->dot11_rate[j]) == i)
                    {
                        rs->dot11_rate[j] |= WIFINET_RATE_BASIC;
                    }
                }
            }
        }
    }
    else
    {
        WIFINET_DPRINTF_STA(AML_DEBUG_RATE, sta, "ht rate set %s;", "empty");
    }
}



static void wifi_mac_get_vht_rates(unsigned short map, struct wifi_mac_vht_rate_s *vht)
{
    int i = 0;
    unsigned char n; 

    memset(vht,0,sizeof(struct wifi_mac_vht_rate_s));

    /* eg. 111111111111 10 10,  spatial 1 -2 support mcs0-9,
        so 'n < 3' means a spatial */
    for ( ; i < MAX_VHT_STREAMS; i++) 
    {
        n  = map & 0x03;
        vht->rates[i] = n;
        if (n < 3) 
        {
            vht->num_streams++;
        }  
        map = map >> 2;
    }
}

static unsigned short wifi_mac_get_vht_rate_map(struct wifi_mac_vht_rate_s *vht)
{
    int i = 0;
    unsigned short map = 0; 

    for ( ; i < MAX_VHT_STREAMS; i++) 
    {
        map |= ((vht->rates[i]) << (i*2));
    }
    return(map);
}


static void  wifi_mac_get_vht_intersect_rates( struct wifi_mac_vht_rate_s *irs,
                  struct wifi_mac_vht_rate_s *srs, struct wifi_mac_vht_rate_s *drs)
{
    int i = 0;
    irs->num_streams = MIN(srs->num_streams, drs->num_streams);

    /* initialize the rates for all streams with unsupported val (0x3) */
    memset(irs->rates,0x3, MAX_VHT_STREAMS);

    for ( ; i < irs->num_streams; i++ ) 
    {
        irs->rates[i] = MIN(srs->rates[i], drs->rates[i]);
    }
}

static int  wifi_mac_vht_basic_rate_check( struct wifi_mac_vht_rate_s *irs, struct wifi_mac_vht_rate_s *brs)
{
    int i = 0;

    if (irs->num_streams < brs->num_streams) 
    {
        return 0;
    }

    for ( ; i < brs->num_streams; i++) 
    {
        if ( irs->rates[i] <  brs->rates[i] )
        {
            return 0;
        }
    }

    return 1;
}



int wifi_mac_setup_vht_rates(struct wifi_station *sta,
                         unsigned char *ie,
                         int flags)
{
    struct wifi_mac_vht_rate_s tsrs,rsrs,brs,rx_rrs, tx_rrs, tx_irs, rx_irs; 
    int i = 0;

    /* Our Supported Tx rates,*/
    wifi_mac_get_vht_rates(sta->sta_wnet_vif->vm_vhtcap_max_mcs.tx_mcs_set.mcs_map, &tsrs);
    DPRINTF(AML_DEBUG_RATE,"Our Supported Tx rates,num_streams=%d\n", tsrs.num_streams);
    for( i=0; i<tsrs.num_streams; i++)
    {
        DPRINTF(AML_DEBUG_RATE,"i=%d,rates=0x%x\n ", i, tsrs.rates[i]);
    }
    
    /* Our Supported Rx rates, */
    wifi_mac_get_vht_rates(sta->sta_wnet_vif->vm_vhtcap_max_mcs.rx_mcs_set.mcs_map, &rsrs);
    DPRINTF(AML_DEBUG_RATE,"Our Supported Rx rates,num_streams=%d\n", rsrs.num_streams);
    for( i=0; i<rsrs.num_streams; i++)
    {
        DPRINTF(AML_DEBUG_RATE,"i=%d,rates=0x%x\n ", i, rsrs.rates[i]);
    }
    
    /* Our Basic rates */
    wifi_mac_get_vht_rates(sta->sta_wnet_vif->vm_vhtop_basic_mcs, &brs);
    DPRINTF(AML_DEBUG_RATE,"Our Basic rates, num_streams=%d\n",brs.num_streams);
    for( i=0; i<brs.num_streams; i++)
    {
        DPRINTF(AML_DEBUG_RATE,"i=%d,rates=0x%x\n ", i, brs.rates[i]);
    }

    
    /* Received Rx Rates in (re)assoc req/resp - vht parse had already copied the info to sta */
    wifi_mac_get_vht_rates(sta->sta_rx_vhtrates, &rx_rrs);
    DPRINTF(AML_DEBUG_RATE,"ap support Rx Rates, num_streams=%d\n",  rx_rrs.num_streams);
    for( i=0; i<rx_rrs.num_streams; i++)
    {
        DPRINTF(AML_DEBUG_RATE,"i=%d,rates=0x%x\n ", i, rx_rrs.rates[i]);
    }
    
    /* Received Tx Rates in (re)assoc req/resp - vht parse has already copied this info to sta */
    wifi_mac_get_vht_rates(sta->sta_tx_vhtrates, &tx_rrs);
    DPRINTF(AML_DEBUG_RATE,"ap support tx Rates num_streams=%d\n", tx_rrs.num_streams );
    for( i=0; i<tx_rrs.num_streams; i++)
    {
        DPRINTF(AML_DEBUG_RATE,"i=%d,rates=0x%x\n ", i, tx_rrs.rates[i]);
    }

    /* Intersection (SRC TX & DST RX) and (SRC RX & DST TX) */ 
    if (flags & WIFINET_F_DOXSECT) 
    {
        wifi_mac_get_vht_intersect_rates(&tx_irs, &rx_rrs, &tsrs);
        wifi_mac_get_vht_intersect_rates(&rx_irs, &tx_rrs, &rsrs);
    }
    else
    { 
        memcpy( &tx_irs, &rx_rrs,  sizeof(struct wifi_mac_vht_rate_s) );
        memcpy( &rx_irs, &tx_rrs,  sizeof(struct wifi_mac_vht_rate_s) );
    }

    DPRINTF(AML_DEBUG_RATE,"Negotiated Tx VHT rates num_streams=%d\n",tx_irs.num_streams);
    for( i=0; i<tx_irs.num_streams; i++)
    {
        DPRINTF(AML_DEBUG_RATE,"i=%d,rates=0x%x\n ", i, tx_irs.rates[i]);
    }

    if( tx_irs.rates[tx_irs.num_streams-1] == VHT_MCS0_MC9 )
    {
        for( i = 0; i<10; i++ )
        {
            /*set mcs0 - mcs9 */
            sta->sta_vhtrates.dot11_rate[i] = i;
            DPRINTF(AML_DEBUG_RATE,"i =%d,dot11_rate=0x%x\n", i, sta->sta_vhtrates.dot11_rate[i]);
        }
        sta->sta_vhtrates.dot11_rate_num = 10;  
    }
    else if( tx_irs.rates[tx_irs.num_streams-1] == VHT_MCS0_MC8 )
    {
         for( i = 0; i<9; i++ )
        {
            /*set mcs0 - mcs8*/
            sta->sta_vhtrates.dot11_rate[i] = i;
            DPRINTF(AML_DEBUG_RATE,"i =%d,dot11_rate=0x%x\n", i, sta->sta_vhtrates.dot11_rate[i]);
        }
        sta->sta_vhtrates.dot11_rate_num = 9;  
    
    }
    else if( tx_irs.rates[tx_irs.num_streams-1] == VHT_MCS0_MC7 )
    {
        for( i = 0; i<8; i++ )
        {
            sta->sta_vhtrates.dot11_rate[i] = i;
            DPRINTF(AML_DEBUG_RATE,"i =%d,dot11_rate=0x%x\n", i, sta->sta_vhtrates.dot11_rate[i]);
        }
        sta->sta_vhtrates.dot11_rate_num = 8;  
    }
    else
    {
        DPRINTF(AML_DEBUG_RATE,"Negotiated Tx VHT rates error\n");
         for( i = 0; i<8; i++ )
        {
            sta->sta_vhtrates.dot11_rate[i] = i;
            DPRINTF(AML_DEBUG_RATE,"i =%d,dot11_rate=0x%x\n", i, sta->sta_vhtrates.dot11_rate[i]);
        }
        sta->sta_vhtrates.dot11_rate_num = 8; 
        
    }

    /* Rate control needs intersection of SRC TX rates and DST RX rates */
    sta->sta_streams = tx_irs.num_streams;
    sta->sta_tx_vhtrates = wifi_mac_get_vht_rate_map(&tx_irs);
    sta->sta_rx_vhtrates = wifi_mac_get_vht_rate_map(&rx_irs);

    if (flags & WIFINET_F_DOBRS) 
    {
        if (!wifi_mac_vht_basic_rate_check(&tx_irs, &brs)) 
        {
             DPRINTF(AML_DEBUG_RATE,"%s: Mismatch in Tx basic rate set\n", __func__);
             return 0;
        }

        if (!wifi_mac_vht_basic_rate_check(&rx_irs, &brs))
        {
              DPRINTF(AML_DEBUG_RATE,"%s: Mismatch in Rx basic rate set\n", __func__);
             return 0;
        }
 
    }
    
    DPRINTF(AML_DEBUG_RATE,"Negotiated: txrates = %x rxrates = %x\n", sta->sta_tx_vhtrates, sta->sta_rx_vhtrates);
    return 1;

}

void wifi_mac_vht_rate_init(struct wlan_net_vif *wnet_vif, unsigned short mcs_map,
                      unsigned short max_datarate, unsigned short basic_mcs)
{
    /* Set the VHT Supported MCS subset and highest data rate*/
    wnet_vif->vm_vhtcap_max_mcs.rx_mcs_set.data_rate =
        wnet_vif->vm_vhtcap_max_mcs.tx_mcs_set.data_rate = max_datarate;
    wnet_vif->vm_vhtcap_max_mcs.rx_mcs_set.mcs_map =
        wnet_vif->vm_vhtcap_max_mcs.tx_mcs_set.mcs_map = mcs_map;

    /* Set up the VHT Basic MCS rate set. Use only the relevant 16 bits */
    wnet_vif->vm_vhtop_basic_mcs = basic_mcs;
}

int wifi_mac_rate_vattach (struct wlan_net_vif *wnet_vif)
{
    wnet_vif->vm_fixed_rate.rateinfo = 0xff;
    wnet_vif->vm_fixed_rate.mode = WIFINET_FIXED_RATE_NONE;
    wnet_vif->vm_change_rate_enable = 1;
    return 0;
}

struct ratecontrol_ops *ratectrl[RATE_ALGORITHM_NUMS] =
{
    &minstrel_ops,
};


void wifi_mac_rate_ratmod_attach(void *drv)
{
    int i;
    char *def_ratectrl_name = "minstrel";
    struct drv_private *drv_priv = (struct drv_private *)drv;

    for (i = 0; i < RATE_ALGORITHM_NUMS; i++)
    {
        if (!strcmp(def_ratectrl_name, ratectrl[i]->name))
        {
            drv_priv->ratctrl_ops = ratectrl[i];
            drv_priv->ratctrl_ops->rate_attach(  );
        }
    }
}


void wifi_mac_rate_ratmod_detach(void *drv)
{
    struct drv_private *drv_priv = (struct drv_private *)drv;

    drv_priv->ratctrl_ops->rate_detach();
    drv_priv->ratctrl_ops = NULL;
}


void wifi_mac_rate_newassoc(struct wifi_station *sta, int isnew)
{
    struct drv_private *drv_priv = ((struct aml_driver_nsta *)sta->drv_sta)->sta_drv_priv;
    drv_priv->ratctrl_ops->rate_newassoc(sta);
}

void wifi_mac_rate_disassoc(struct wifi_station *sta)
{
    struct drv_private *drv_priv = ((struct aml_driver_nsta *)sta->drv_sta)->sta_drv_priv;
    DRV_MINSTREL_LOCK(drv_priv);
    drv_priv->ratctrl_ops->rate_disassoc(sta);
    DRV_MINSTREL_UNLOCK(drv_priv);
}

void wifi_mac_rate_init(void * ieee,
                    enum wifi_mac_macmode mode,
                    const struct drv_rate_table *rt)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    struct wifi_mac_rateset *rs;
    unsigned char i, maxrates;
    unsigned char ht_rate_num = 0, vht_rate_num = 0, g_rate_index = 0;
    unsigned char b_rate_index = 0;

    if (mode >= WIFINET_MODE_MAX)
    {
        DPRINTF( AML_DEBUG_ANY, "%s: unsupported mode %u\n",
                 __func__, mode);
        return;
    }

    if (rt->rateCount > WIFINET_RATE_MAXSIZE)
    {
        DPRINTF( AML_DEBUG_ANY,"%s: rate table too small (%u > %u)\n",
            __func__, rt->rateCount, WIFINET_RATE_MAXSIZE);
        maxrates = WIFINET_RATE_MAXSIZE;
    }
    else
    {
        maxrates = rt->rateCount;
    }

    for (i = 0; i < maxrates; i++)
    {
        if (rt->info[i].valid == HAL_NOSUPPORT)
            continue;

        switch (rt->info[i].phy)
        {
            case WIFINET_T_VHT:
                /* vht rates */
                rs = &wifimac->wm_sup_vht_rates;
                rs->dot11_rate[vht_rate_num++] = rt->info[i].dot11Rate;
                rs->dot11_rate_num = vht_rate_num;
                break;

            case WIFINET_T_HT:
                /* ht rates */
                rs = &wifimac->wm_sup_ht_rates;
                rs->dot11_rate[ht_rate_num++] = rt->info[i].dot11Rate;
                rs->dot11_rate_num = ht_rate_num;
                break;

            case WOFDM:
                /* save 11g rate for p2p GO and vht mode*/
                rs = &wifimac->wm_11g_rates;
                rs->dot11_rate[g_rate_index++] = rt->info[i].dot11Rate;
                rs->dot11_rate_num = g_rate_index;
                break;

            case CCK:
                /* save 11b rate */
                rs = &wifimac->wm_11b_rates;
                rs->dot11_rate[b_rate_index++] = rt->info[i].dot11Rate;
                rs->dot11_rate_num = b_rate_index;
                break;

            default:
                break;
        }
    }

    AML_OUTPUT("maxrates %d mode %d \n", maxrates,mode);
}

int wifi_mac_iserp_rateset(struct wifi_mac *wifimac, struct wifi_mac_rateset *rs)
{
    static const int rates[] = { 2, 4, 11, 22, 12, 24, 48 };
    int i, j;

    if (rs->dot11_rate_num < ARRAY_LENGTH(rates))
        return 0;
    for (i = 0; i < ARRAY_LENGTH(rates); i++)
    {
        for (j = 0; j < rs->dot11_rate_num; j++)
        {
            int r = WIFINET_GET_RATE_VAL(rs->dot11_rate[j] );
            if (rates[i] == r)
                goto next;
            if (r > rates[i])
                return 0;
        }
        return 0;
    next:
        ;
    }
    return 1;
}

void wifi_mac_sort_rate(struct wifi_mac_rateset *rs)
{
    int i, j;
    unsigned char r;
    for (i = 0; i < rs->dot11_rate_num; i++ )
    {
        for (j = i + 1; j < rs->dot11_rate_num; j++)
        {
            if (WIFINET_GET_RATE_VAL(rs->dot11_rate[i]) > WIFINET_GET_RATE_VAL(rs->dot11_rate[j]))
            {
                r = rs->dot11_rate[i];
                rs->dot11_rate[i] = rs->dot11_rate[j];
                rs->dot11_rate[j] = r;
            }
        }
    }
}

void wifi_mac_xsect_rate(struct wifi_mac_rateset *rs1, struct wifi_mac_rateset *rs2, struct wifi_mac_rateset *srs)
{
    int i, j, k;

    k = 0;
    for (i = 0; i < rs1->dot11_rate_num; i++)
    {
        for (j = 0; j < rs2->dot11_rate_num; j++)
        {
            if (WIFINET_GET_RATE_VAL(rs1->dot11_rate[i]) == WIFINET_GET_RATE_VAL(rs2->dot11_rate[j]))
            {
                srs->dot11_rate[k++] = rs1->dot11_rate[i];
                break;
            }
        }
    }
    srs->dot11_rate_num = k;
}

int wifi_mac_brs_rate_check(struct wifi_mac_rateset *srs, struct wifi_mac_rateset *nrs)
{
    int i, j;
    for (i = 0; i < srs->dot11_rate_num; i++)
    {
        if (srs->dot11_rate[i] & WIFINET_RATE_BASIC)
        {
            for (j = 0; j < nrs->dot11_rate_num; j++)
            {
                if (WIFINET_GET_RATE_VAL(srs->dot11_rate[i]) == WIFINET_GET_RATE_VAL(nrs->dot11_rate[j]))
                {
                    break;
                }
            }
            if (j == nrs->dot11_rate_num)
            {
                return 0;
            }
        }
    }
    return 1;
}

int wifi_mac_fixed_rate_check(struct wifi_station *sta, struct wifi_mac_rateset *nrs)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int i;

    if ((wnet_vif->vm_fixed_rate.mode == WIFINET_FIXED_RATE_NONE)
        || (wnet_vif->vm_fixed_rate.mode == WIFINET_FIXED_RATE_MCS))
        return 1;

    for (i = 0; i < nrs->dot11_rate_num; i++)
    {
        if ( WIFINET_GET_RATE_VAL( nrs->dot11_rate[i] ) ==
            WIFINET_GET_RATE_VAL( wnet_vif->vm_fixed_rate.rateinfo ))
            return 1;
    }
    return 0;
}
