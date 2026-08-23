/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer p2p  module
 *
 *
 ****************************************************************************************
 */

#include "wifi_mac_com.h"


#ifdef CONFIG_P2P
unsigned char p2p_public_action_tmp[][32]=
{
    "P2P_GO_NEGO_REQ",
    "P2P_GO_NEGO_RESP",
    "P2P_GO_NEGO_CONF",
    "P2P_INVITE_REQ",
    "P2P_INVITE_RESP",
    "P2P_DEVDISC_REQ",
    "P2P_DEVDISC_RESP",
    "P2P_PROVISION_DISC_REQ",
    "P2P_PROVISION_DISC_RESP"
};
unsigned char p2p_action_tmp[][32]=
{
    "P2P_NOTICE_OF_ABSENCE",
    "P2P_PRESENCE_REQUEST",
    "P2P_PRESENCE_RESPONSE",
    "P2P_GO_DISC_REQUEST"
};

static int
vm_p2p_build_noa_attrib (struct wifi_mac_p2p *p2p ,unsigned char  *p_noa_attrib)
{
    int noa_attrib_len = 0;
    unsigned char  *ptr = p_noa_attrib;

    ptr[0] = P2P_ATTR_NOTICE_OF_ABSENCE;
    noa_attrib_len += 3;
    ptr += noa_attrib_len;

    ptr[0] = p2p->noa_index;
    noa_attrib_len++;
    ptr++;

    ptr[0] = p2p->ctw_opps_u.ctw_opps_u8;
    noa_attrib_len++;
    ptr++;

    if (p2p->noa.count)
    {
        ptr[0] = p2p->noa.count;
        noa_attrib_len ++;
        ptr++;

        WRITE_32L(ptr, p2p->noa.duration);
        noa_attrib_len += 4;
        ptr += 4;

        WRITE_32L(ptr, p2p->noa.interval);
        noa_attrib_len += 4;
        ptr += 4;

        WRITE_32L(ptr, p2p->noa.start);
        noa_attrib_len += 4;
        ptr += 4;
    }
    WRITE_16L(p_noa_attrib + 1, noa_attrib_len - 3);

    return noa_attrib_len;
}

static int
vm_p2p_build_p2p_ie_with_noa_attrib (struct wifi_mac_p2p *p2p, unsigned char *p_p2p_ie)
{
    int p2p_len=0;

    p_p2p_ie[0] = WIFINET_ELEMID_VENDOR;
    p2p_len = 2;

    WRITE_32B(p_p2p_ie + p2p_len, P2P_OUI_BE);
    p2p_len += 4;

    p2p_len += vm_p2p_build_noa_attrib(p2p, p_p2p_ie + p2p_len);

    p_p2p_ie[1] = p2p_len - 2;
    return p2p_len;
}

int vm_p2p_update_noa_ie (struct wifi_mac_p2p *p2p)
{
    int ret = 0;
    struct wifi_mac_app_ie_t *noa_beacon_ie = &p2p->noa_app_ie[WIFINET_APPIE_FRAME_BEACON];
    struct wifi_mac_app_ie_t *noa_proberesp_ie = &p2p->noa_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP];
    int p2p_len = 0;
    unsigned char p2p_ie[WIFINET_MAX_WDF_IE] = {0};

    if (!((p2p->noa.count == 0) && (p2p->ctw_opps_u.ctw_opps_s.opps == 0)))
    {
        p2p_len = vm_p2p_build_p2p_ie_with_noa_attrib(p2p, p2p_ie);
    }
    else
    {
       AML_PRINT(AML_DBG_MODULES_P2P,"clear noa ie\n");
    }
    wifi_mac_save_app_ie(noa_beacon_ie, p2p_ie, p2p_len);
    ret = vm_p2p_update_beacon_app_ie(p2p->wnet_vif);
    wifi_mac_save_app_ie(noa_proberesp_ie, p2p_ie, p2p_len);
    return ret;

}

int vm_p2p_noa_start(struct wifi_mac_p2p *p2p, const struct p2p_noa *noa)
{
    struct p2p_noa *l_noa = &(p2p->noa);
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned short beacon_interval = wnet_vif->vm_bcn_intval;
    static unsigned short ps_trigger_timeout_backup = 0;

    if(!memcmp(l_noa, noa, sizeof(struct p2p_noa)))
    {
        return P2P_NOA_IGNORE;
    }
    memcpy(l_noa, noa, sizeof(struct p2p_noa));
    p2p->p2p_flag &= (~(P2P_NOA_START_MATCH_BEACON_HI|P2P_NOA_END_MATCH_BEACON_HI));
    p2p->HiP2pNoaCountNow = 0;
    if (p2p->p2p_flag & P2P_NOA_START_FLAG_HI)
    {

        p2p->p2p_flag &= (~P2P_NOA_START_FLAG_HI);
        wifimac->drv_priv->drv_ops.drv_hal_set_p2p_noa_enable(wifimac->drv_priv,wnet_vif->wnet_vif_id, 0, 0, 0, 0, 0);
        wnet_vif->vm_pstxqueue_flags &= (~WIFINET_PSQUEUE_NOA);
        wifi_mac_buffer_txq_send_pre(wnet_vif);
        if (ps_trigger_timeout_backup)
        {
            wnet_vif->vm_pwrsave.ips_ps_trigger_timeout = ps_trigger_timeout_backup;
            ps_trigger_timeout_backup = 0;
        }
    }
    AML_PRINT(AML_DBG_MODULES_P2P,"beacon_interval=%d count=%d duration=%d interval=%d start=%d\n",
              beacon_interval, l_noa->count,
             l_noa->duration, l_noa->interval, l_noa->start);
    if (l_noa->count)
    {
        p2p->p2p_flag |= P2P_NOA_START_FLAG_HI;

        if ((l_noa->count != 1) &&((l_noa->duration < P2P_NOA_DURATION_MIN) ||
            (l_noa->duration + P2P_NOA_DURATION_MIN >= l_noa->interval) ||
            (l_noa->interval > 3 * beacon_interval * TU_DURATION)))
        {
            DPRINTF(AML_DEBUG_WARNING, "%s %d duration=%d  interval=%d beacon_interval=%d, opps ignore\n",
                    __func__, __LINE__, l_noa->duration, l_noa->interval, beacon_interval);
            return P2P_NOA_IGNORE;
        }
        p2p->HiP2pNoaCountNow = (l_noa->count << 1);

        if (wnet_vif->vm_pwrsave.ips_ps_trigger_timeout > ((l_noa->interval-l_noa->duration)/1000))
        {
            ps_trigger_timeout_backup = wnet_vif->vm_pwrsave.ips_ps_trigger_timeout;
            wnet_vif->vm_pwrsave.ips_ps_trigger_timeout = ((l_noa->interval-l_noa->duration)/1000);
        }
    }
    else
    {
        return P2P_NOA_IGNORE;
    }

    if (l_noa->interval == beacon_interval*1000)
    {
        l_noa->interval = beacon_interval*TU_DURATION;
    }

    if (l_noa->count && !l_noa->start)
    {
        unsigned long long tsf = wifimac->drv_priv->drv_ops.drv_hal_get_tsf(wifimac->drv_priv, wnet_vif->wnet_vif_id);
        unsigned int ltsf;
        unsigned int next_tbtt;

        ltsf = (unsigned int)tsf;
        next_tbtt = roundup(ltsf, beacon_interval*TU_DURATION);
        l_noa->start = next_tbtt + (l_noa->interval - l_noa->duration);
        AML_PRINT(AML_DBG_MODULES_P2P,"tsf=%u %u\n",
            (unsigned int)(tsf>>32), ltsf);
    }

    if ((l_noa->count == NET80211_P2P_SCHED_REPEAT) &&
        (l_noa->interval == beacon_interval*TU_DURATION))
    {
        if (!(l_noa->start % (beacon_interval*TU_DURATION)))
        {
            p2p->p2p_flag |= P2P_NOA_START_MATCH_BEACON_HI;
        }
        else if (!((l_noa->start+l_noa->duration) % (beacon_interval*TU_DURATION)))
        {
            p2p->p2p_flag |= P2P_NOA_END_MATCH_BEACON_HI;
        }
    }
    AML_PRINT(AML_DBG_MODULES_P2P,"interval=%d start=%u flag=0x%x\n",
             l_noa->interval, l_noa->start, p2p->p2p_flag);
    wifimac->drv_priv->drv_ops.drv_hal_set_p2p_noa_enable(wifimac->drv_priv,wnet_vif->wnet_vif_id, \
            l_noa->duration, l_noa->interval, l_noa->start, l_noa->count, p2p->p2p_flag);
    return 0;
}

int vm_p2p_opps_start(struct wifi_mac_p2p *p2p, const union type_ctw_opps_u *ctw_opps_u)
{
    struct type_ctw_opps_s *l_ctw_opps_s = &(p2p->ctw_opps_u.ctw_opps_s);
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned short beacon_interval = wnet_vif->vm_bcn_intval;
    static unsigned short ps_waitbeacon_timeout_backup = 0;

    if (p2p->ctw_opps_u.ctw_opps_u8 == ctw_opps_u->ctw_opps_u8)
    {
        return P2P_OPPS_IGNORE;
    }
    p2p->ctw_opps_u.ctw_opps_u8 = ctw_opps_u->ctw_opps_u8;
    p2p->p2p_flag &= (~P2P_OPPPS_CWEND_FLAG_HI);
    if (p2p->p2p_flag & P2P_OPPPS_START_FLAG_HI)
    {

        p2p->p2p_flag &= ~P2P_OPPPS_START_FLAG_HI;
        wifimac->drv_priv->drv_ops.drv_hal_set_p2p_opps_cwend_enable(wifimac->drv_priv,wnet_vif->wnet_vif_id, 0);
        wnet_vif->vm_pstxqueue_flags &= (~WIFINET_PSQUEUE_OPPS);
        wifi_mac_buffer_txq_send_pre(wnet_vif);
        if (ps_waitbeacon_timeout_backup)
        {
            wnet_vif->vm_pwrsave.ips_ps_waitbeacon_timeout = ps_waitbeacon_timeout_backup;
            ps_waitbeacon_timeout_backup = 0;
        }
    }

    AML_PRINT(AML_DBG_MODULES_P2P,"beacon_interval=%d ctw_opps=0x%x ctw 0x%x\n",
            beacon_interval, l_ctw_opps_s->opps,l_ctw_opps_s->ctw);

    if (!((wifi_mac_pwrsave_is_sta_sleeping(wnet_vif) ==0)
            || (wifi_mac_pwrsave_if_ap_can_opps(wnet_vif) ==0)))
    {
        p2p->noa_index = 0;
        p2p->ctw_opps_u.ctw_opps_u8 = 0;
        AML_PRINT(AML_DBG_MODULES_P2P,"mode=%d cannot opps\n",
                 wnet_vif->vm_opmode);
        return P2P_OPPS_IGNORE;
    }

    if (l_ctw_opps_s->opps && l_ctw_opps_s->ctw)
    {
        unsigned char ctw = l_ctw_opps_s->ctw;
        p2p->p2p_flag |= P2P_OPPPS_START_FLAG_HI;

        if ((ctw < P2P_OPPS_CTW_MIN) || (ctw+P2P_OPPS_CTW_MIN > beacon_interval))
        {
            DPRINTF(AML_DEBUG_WARNING,
                    "%s %d ctw=%d  beacon_interval=%d, opps ignore\n",
                    __func__, __LINE__, ctw, beacon_interval);
            return P2P_OPPS_IGNORE;
        }
        AML_PRINT(AML_DBG_MODULES_P2P,"wait_bcn_timer %d ctwindow %d\n",
                wnet_vif->vm_pwrsave.ips_ps_waitbeacon_timeout,
                (l_ctw_opps_s->ctw*TU_DURATION)/1000);

        if (wnet_vif->vm_pwrsave.ips_ps_waitbeacon_timeout
            > ((l_ctw_opps_s->ctw * TU_DURATION) /1000))
        {
            ps_waitbeacon_timeout_backup = wnet_vif->vm_pwrsave.ips_ps_waitbeacon_timeout;
            wnet_vif->vm_pwrsave.ips_ps_waitbeacon_timeout =
                    ((l_ctw_opps_s->ctw * TU_DURATION) / 1000);
        }
    }

    if (l_ctw_opps_s->opps && l_ctw_opps_s->ctw)
    {
        wifimac->drv_priv->drv_ops.drv_hal_set_p2p_opps_cwend_enable(wifimac->drv_priv,
                                        wnet_vif->wnet_vif_id, l_ctw_opps_s->ctw);
    }
    return 0;
}

int vm_p2p_client_cancel_opps (struct wifi_mac_p2p *p2p)
{
    union type_ctw_opps_u ctw_opps_u;

    memset(&ctw_opps_u,0,sizeof(union type_ctw_opps_u));
    p2p->noa_index = 0;
    vm_p2p_opps_start(p2p, &ctw_opps_u);

    return 0;
}


int vm_p2p_go_cancel_opps (struct wifi_mac_p2p *p2p)
{
    vm_p2p_client_cancel_opps(p2p);
    vm_p2p_update_noa_ie(p2p);
    return 0;
}

int vm_p2p_client_cancel_noa (struct wifi_mac_p2p *p2p)
{
    struct p2p_noa noa = {0};

    p2p->noa_index = 0;
    vm_p2p_noa_start(p2p, &noa);

    return 0;
}

int vm_p2p_go_cancel_noa (struct wifi_mac_p2p *p2p)
{
    vm_p2p_client_cancel_noa(p2p);
    vm_p2p_update_noa_ie(p2p);
    return 0;
}

static int
vm_p2p_get_noa_param (const unsigned char *pnoa,
                            union type_ctw_opps_u *o_ctw_opps,
                            struct p2p_noa *o_noa, unsigned char *o_index)
{
    union type_ctw_opps_u ctw_opps_u;
    struct p2p_noa noa = {0};
    unsigned char index;
    unsigned char count = 0;

    const unsigned char *p= pnoa;
    unsigned short len;

    memset(&ctw_opps_u,0,sizeof(union type_ctw_opps_u));
    if (!pnoa)
    {
        return -1;
    }

    p++;

    len = READ_16L(p);
    p+=2;

    index = *p;
    p++;

    ctw_opps_u.ctw_opps_u8 = *p;
    p++;

    if (ctw_opps_u.ctw_opps_s.opps && !(ctw_opps_u.ctw_opps_s.ctw))
    {
        DPRINTF(AML_DEBUG_ERROR, "%s %d noa opps=%d ctw=%d error\n",
                __func__, __LINE__, ctw_opps_u.ctw_opps_s.opps, ctw_opps_u.ctw_opps_s.ctw);
        return -1;
    }

    if (len > 2)
    {
        if (len != 15)
        {
            DPRINTF(AML_DEBUG_ERROR, "%s noa len=%d error\n", __func__, len);
            return -1;
        }
        noa.count = *p;
        p++;

        noa.duration = READ_32L(p);
        p+=4;

        noa.interval = READ_32L(p);
        p+=4;

        noa.start = READ_32L(p);
        p+=4;

        if (!noa.count)
        {
            DPRINTF(AML_DEBUG_ERROR, "%s noa count=%d error\n", __func__, count);
            return -1;
        }

        memcpy(o_noa, &noa, sizeof(struct p2p_noa));
    }

    *o_index = index;
    o_ctw_opps->ctw_opps_u8 = ctw_opps_u.ctw_opps_u8;

    return 0;
}

int vm_p2p_client_parse_noa_ie (struct wifi_mac_p2p *p2p, const unsigned char *pnoa)
{
    union type_ctw_opps_u ctw_opps_u;
    unsigned char index = 0;
    struct p2p_noa noa;

    memset(&noa,0,sizeof(struct p2p_noa));
    memset(&ctw_opps_u,0,sizeof(union type_ctw_opps_u));

    if (pnoa)
    {
        do
        {
            if (vm_p2p_get_noa_param(pnoa, &ctw_opps_u, &noa, &index) < 0)
            {
                break;
            }

            if (p2p->p2p_flag & P2P_NOA_INDEX_INIT_FLAG)
            {
                p2p->p2p_flag &= (~P2P_NOA_INDEX_INIT_FLAG);
                p2p->noa_index = index;
                AML_PRINT(AML_DBG_MODULES_P2P,"first GO noa index=%d\n", index);
            }
            else if (p2p->noa_index != index)
            {
                AML_PRINT(AML_DBG_MODULES_P2P,"index=%d->%d\n", p2p->noa_index, index);
                p2p->noa_index = index;
            }
            else
            {
                AML_PRINT(AML_DBG_MODULES_P2P, "noa start err\n");
                break;
            }

            if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_GO_COMPLETE) {
                AML_PRINT(AML_DBG_MODULES_P2P,"ctw_opps_u8:%d\n", ctw_opps_u.ctw_opps_u8);
                vm_p2p_noa_start(p2p, &noa);
                vm_p2p_opps_start(p2p, &ctw_opps_u);
            }
        }
        while(0);
    }
    else  if (!(p2p->p2p_flag & P2P_NOA_INDEX_INIT_FLAG))
    {
        AML_PRINT(AML_DBG_MODULES_P2P,"GO cancel noa ie\n");
        p2p->p2p_flag |= P2P_NOA_INDEX_INIT_FLAG;
        vm_p2p_client_cancel_opps(p2p);
        vm_p2p_client_cancel_noa(p2p);
    }

    return 0;
}


static unsigned int p2p_noa_start_time_cond (unsigned int tsf, unsigned int start_time)
{
    unsigned int ret = 0;

    if (tsf < start_time)
    {
        if (start_time -tsf < P2P_NOA_START_TIME_WRAP)
        {
            ret = P2P_NOA_START_IN_FUTURE;
        }
        else
        {
            ret = P2P_NOA_START_IN_PAST|P2P_NOA_START_NEED_WRAP;
        }
    }
    else
    {
        if (tsf -start_time < P2P_NOA_START_TIME_WRAP)
        {
            ret = P2P_NOA_START_IN_PAST;
        }
        else
        {
            ret = P2P_NOA_START_IN_FUTURE|P2P_NOA_START_NEED_WRAP;
        }
    }
    return ret;
}

int vm_p2p_update_noa_count_start (struct wifi_mac_p2p *p2p)
{
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;

#if 0
    unsigned int total_len  = wnet_vif->app_ie[WIFINET_APPIE_FRAME_BEACON].length;
    unsigned char *frm = wnet_vif->app_ie[WIFINET_APPIE_FRAME_BEACON].ie;
    unsigned char *p = frm, *p2p_ie_with_noa=NULL, *noa_ie=NULL;
    unsigned char len = 0, tmplen;

    union type_ctw_opps_u ctw_opps_u = {0};
    struct p2p_noa noa = {0};
    unsigned char index = 0;

    while (len < total_len)
    {
        p2p_ie_with_noa = vm_get_p2p_ie(p, total_len-len, NULL, &tmplen);
        if (p2p_ie_with_noa)
        {
            noa_ie = vm_p2p_get_p2pie_attrib(p2p_ie_with_noa, P2P_ATTR_NOTICE_OF_ABSENCE);
            if (noa_ie)
            {
                if (vm_p2p_get_noa_param(noa_ie, &ctw_opps_u, &noa, &index) < 0)
                {
                    break;
                }
                DPRINTF(AML_DEBUG_P2P, "%s %d found noa in our beacon\n", __func__, __LINE__);
                break;
            }
            else
            {
                len += (tmplen + 2);
                p += len;
                continue;
            }
        }
        else
        {
            break;
        }
    }
#endif
    if (p2p->noa.count)
    {
        struct wifi_mac *wifimac = wnet_vif->vm_wmac;
        unsigned long long tsf = wifimac->drv_priv->drv_ops.drv_hal_get_tsf(wifimac->drv_priv, wnet_vif->wnet_vif_id);
        unsigned int next_tbtt;

        next_tbtt = (unsigned int)tsf + wnet_vif->vm_bcn_intval*TU_DURATION;

        if (p2p_noa_start_time_cond(next_tbtt, p2p->noa.start+P2P_NOA_START_TIME_WRAP)
            & P2P_NOA_START_IN_PAST)
        {
            pr_debug("%s(%d) noa_len %d, count=%d\n",__func__,__LINE__,
                wnet_vif->app_ie[WIFINET_APPIE_FRAME_BEACON].length, p2p->noa.count);
            wnet_vif->vm_p2p->noa_index ++;
            p2p->noa.start += P2P_NOA_START_TIME_WRAP;
            vm_p2p_update_noa_ie(p2p);
        }
    }
    return 0;
}

#ifdef CONFIG_WFD

static int vm_wfd_initial(struct wifi_mac_p2p *p2p)
{
    int res = true;
    struct wifi_mac_wfd_info *pwfdinfo = &p2p->wfd_info;
    int index = 0;

    pwfdinfo->rtsp_ctrlport = 554;
    pwfdinfo->peer_rtsp_ctrlport = 0;
    pwfdinfo->wfd_enable = false;
    pwfdinfo->wfd_device_type = WFD_DEVINFO_PSINK;

    pwfdinfo->peer_session_avail = true;
    pwfdinfo->wfd_pc = false;
    pwfdinfo->max_thruput = 150;

    memset( pwfdinfo->ip_address, 0x00, 4 );
    memset( pwfdinfo->peer_ip_address, 0x00, 4 );
    for(index=0; index<WIFINET_APPIE_NUM_OF_FRAME; index++)
    {
        wifi_mac_rm_app_ie(&p2p->wfd_app_ie[index]);
    }

    return res;

}

unsigned char *vm_get_wfd_ie(unsigned char *in_ie,
                                                unsigned int in_len,
                                                unsigned char *wfd_ie,
                                                unsigned int *wfd_ielen)
{
    unsigned int cnt = 0;
    unsigned char *wfd_ie_ptr = NULL;
    unsigned char eid;

    AML_PRINT(AML_DBG_MODULES_P2P,"in_len=%d\n", in_len);


    ASSERT(wfd_ielen != NULL);
    *wfd_ielen = 0;

    if (!in_ie || in_len<=0)
        return wfd_ie_ptr;

    while (cnt<in_len)
    {
        eid = in_ie[cnt];

        if ( ( eid == WIFINET_ELEMID_VENDOR ) && iswfdoui(&in_ie[cnt]) )
        {
            wfd_ie_ptr = &in_ie[cnt];
            *wfd_ielen = saveie(wfd_ie,wfd_ie_ptr);
            return wfd_ie_ptr;
        }
        else
        {
            cnt += in_ie[ cnt + 1 ] +2; //goto next
        }
    }
    return NULL;

}

int vm_wfd_build_assocbssid_attrib(struct wlan_net_vif *wnet_vif,unsigned char  *wfdie)
{
    unsigned int wfdielen =0;

    wfdie[ wfdielen++ ] = WFD_ATTR_ASSOC_BSSID;
    WRITE_16B(wfdie + wfdielen, WIFINET_ADDR_LEN);
    wfdielen += 2;
    if ( wnet_vif->vm_state == WIFINET_S_CONNECTED )
    {
        if (wnet_vif->vm_mainsta != NULL)
        {
            memcpy( wfdie , wnet_vif->vm_mainsta->sta_bssid, WIFINET_ADDR_LEN );
        }
        else
        {
            memcpy( wfdie , wnet_vif->vm_des_bssid, WIFINET_ADDR_LEN );
        }
    }
    else
    {
        memcpy( wfdie , wnet_vif->vm_des_bssid, WIFINET_ADDR_LEN );
    }
    wfdielen += WIFINET_ADDR_LEN;

    return wfdielen;
}

static int vm_wfd_build_sinkinfor_attrib(unsigned char  *wfdie)
{
    unsigned int wfdielen =0;

    wfdie[ wfdielen++ ] = WFD_ATTR_COUPLED_SINK_INFO;

    WRITE_16B(wfdie + wfdielen, 0x0001);
    wfdielen += 2;

    wfdie[ wfdielen++ ] = 0;
    return wfdielen;
}

static int
vm_wfd_build_devinfo_attrib(struct wifi_mac_wfd_info *pwfdinfo,
                                        unsigned char  *wfdie,unsigned short devinfo)
{
    unsigned int wfdielen =0;
    wfdie[ wfdielen++ ] = WFD_ATTR_DEVICE_INFO;
    WRITE_16B(wfdie + wfdielen, 0x0006);
    wfdielen += 2;

    WRITE_16B(wfdie + wfdielen, pwfdinfo->wfd_device_type |devinfo);

    wfdielen += 2;

    WRITE_16B(wfdie + wfdielen, pwfdinfo->rtsp_ctrlport );
    wfdielen += 2;

    WRITE_16B(wfdie + wfdielen, pwfdinfo->max_thruput);
    wfdielen += 2;
    return wfdielen;
}



static int
vm_wfd_build_sessioninfo_attrib(struct wifi_mac_p2p *p2p ,unsigned char  *wfdie)
{
    unsigned int wfdielen =0;

    if ( NET80211_P2P_ROLE_GO == p2p->p2p_role )
    {
        wfdie[ wfdielen++ ] = WFD_ATTR_SESSION_INFO;

        WRITE_16B(wfdie + wfdielen, 0x0000);
        wfdielen += 2;
    }
    return wfdielen;
}

unsigned int vm_wfd_add_beacon_ie(struct wifi_mac_p2p *p2p, unsigned char *frm)
{
    unsigned char  *wfdie = frm;
    unsigned int  wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    unsigned short devinfo = 0;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");

    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;

    wfdielen = 2;
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;


    if ( NET80211_P2P_ROLE_GO == p2p->p2p_role )
    {
        if (  wnet_vif->vm_sta_assoc>0 )
        {
            devinfo =WFD_DEVINFO_WSD;
        }
        else
        {
            devinfo = WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD ;
        }
    }
    else
    {
        devinfo= WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD;
    }


    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);


    wfdie[1] = wfdielen-2;

    return wfdielen;

}

unsigned int vm_wfd_add_probereq_ie(struct wifi_mac_p2p *p2p, unsigned char *frm)
{
    unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;

    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    //  WFD OUI
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;


    if (pwfdinfo->wfd_tdls_enable == true)
    {
        devinfo =    WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD |WFD_DEVINFO_PC_TDLS;
    }
    else
    {
        devinfo =    WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD ;
    }

    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);


    wfdie[1] = wfdielen-2;

    return wfdielen;

}

unsigned int vm_wfd_add_probersp_ie(struct wifi_mac_p2p *p2p, unsigned char *frm)
{
    unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int  wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;

    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;


    if ( pwfdinfo->session_available == true )
    {
        if ( NET80211_P2P_ROLE_GO == p2p->p2p_role )
        {
            if (  wnet_vif->vm_sta_assoc>0 )
            {
                if ( pwfdinfo->wfd_tdls_enable )
                {
                    devinfo= WFD_DEVINFO_WSD | WFD_DEVINFO_PC_TDLS;
                }
                else
                {
                    devinfo= WFD_DEVINFO_WSD;
                }
            }
            else
            {
                if ( pwfdinfo->wfd_tdls_enable )
                {
                    devinfo=WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD | WFD_DEVINFO_PC_TDLS ;
                }
                else
                {
                    devinfo= WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD ;
                }
            }
        }
        else
        {
            if ( pwfdinfo->wfd_tdls_enable )
            {
                devinfo = WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD | WFD_DEVINFO_PC_TDLS ;
            }
            else
            {

                devinfo = WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD ;
            }
        }
    }
    else
    {
        if ( pwfdinfo->wfd_tdls_enable )
        {
            devinfo = WFD_DEVINFO_WSD |WFD_DEVINFO_PC_TDLS ;
        }
        else
        {
            devinfo =WFD_DEVINFO_WSD ;
        }
    }
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdielen += vm_wfd_build_sessioninfo_attrib(p2p,wfdie + wfdielen);


    wfdie[1] = wfdielen-2;

    return wfdielen;

}

unsigned int vm_wfd_add_assocreq_ie(struct wifi_mac_p2p *p2p, unsigned char *frm)
{
    unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int  wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    //  WFD OUI
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;

    devinfo = WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD;

    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;
}

unsigned int vm_wfd_add_assocrsp_ie(struct wifi_mac_p2p *p2p, unsigned char *frm)
{
    unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");

    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;

    devinfo = WFD_DEVINFO_SESSION_AVAIL | WFD_DEVINFO_WSD ;

    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);


    wfdie[1] = wfdielen-2;

    return wfdielen;
}

static unsigned int
vm_wfd_add_nego_req_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;

    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;
    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;

    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;
}

static unsigned int
vm_wfd_add_nego_resp_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;

    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;

}

static unsigned int
vm_wfd_add_nego_confirm_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;

    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);


    wfdie[1] = wfdielen-2;

    return wfdielen;

}

static unsigned int
vm_wfd_add_invitation_req_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;

    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");

    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    //  WFD OUI
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;


    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);


    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdielen += vm_wfd_build_sessioninfo_attrib(p2p,wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;

}

static unsigned int
vm_wfd_add_invitation_resp_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;

    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;


    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdielen += vm_wfd_build_sessioninfo_attrib(p2p,wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;

}

static unsigned int
vm_wfd_add_provdisc_req_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    //  WFD OUI
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;


    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;

}

static unsigned int
vm_wfd_add_provdisc_resp_ie(struct wifi_mac_p2p *p2p, unsigned char *wfdie)
{
    //unsigned char  *wfdie = frm;
    unsigned short devinfo = 0;
    unsigned int wfdielen = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");


    //memset(frm, 0, _len);
    wfdie[0] = WIFINET_ELEMID_VENDOR;
    wfdielen = 2;
    WRITE_32B(wfdie + wfdielen, WFD_OUI_BE);
    wfdielen+=4;

    devinfo = WFD_DEVINFO_WSD | WFD_DEVINFO_SESSION_AVAIL;
    wfdielen += vm_wfd_build_devinfo_attrib(pwfdinfo,wfdie + wfdielen,devinfo);

    wfdielen += vm_wfd_build_assocbssid_attrib(wnet_vif,wfdie + wfdielen);

    wfdielen += vm_wfd_build_sinkinfor_attrib(wfdie + wfdielen);

    wfdie[1] = wfdielen-2;

    return wfdielen;

}


unsigned int vm_wfd_add_ie(struct wlan_net_vif *wnet_vif,
                                                const unsigned char *frm, unsigned int len)
{
    unsigned int wfdielen = 0;
    unsigned char  *wfdie = (unsigned char *)(frm + len);
    struct wifi_mac_p2p_pub_act_frame *p2p_pub_act = NULL;
    struct wifi_mac_p2p_action_frame *p2p_act = NULL ;
    struct wifi_mac_p2p *p2p =  wnet_vif->vm_p2p;
    struct wifi_mac_wfd_info   *pwfdinfo = &p2p->wfd_info;

    p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)(frm + sizeof(struct wifi_frame));
    p2p_act = (struct wifi_mac_p2p_action_frame *)(frm + sizeof(struct wifi_frame));

    if (pwfdinfo->wfd_enable == false)
    {
        AML_PRINT(AML_DBG_MODULES_P2P,"wfd not enable \n");
        return len;
    }
    AML_PRINT(AML_DBG_MODULES_P2P,"ACT_P2P, category=%x action=%x, dialog_token %x\n",
            p2p_pub_act->category, p2p_pub_act->action,p2p_pub_act->dialog_token);


    switch (p2p_pub_act->category)
    {
        case AML_CATEGORY_PUBLIC:
        {
            if ( p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P)
            {
                AML_PRINT(AML_DBG_MODULES_P2P,"p2p_public_action %s \n",
                        p2p_public_action_tmp[p2p_pub_act->subtype]);
                switch (p2p_pub_act->subtype)
                {
                    case P2P_GO_NEGO_REQ:
                    {
                        wfdielen =vm_wfd_add_nego_req_ie( p2p, wfdie);
                        break;
                    }
                    case P2P_GO_NEGO_RESP:
                    {
                        wfdielen =vm_wfd_add_nego_resp_ie(p2p, wfdie);
                        break;
                    }
                    case P2P_GO_NEGO_CONF:
                    {
                        wfdielen =vm_wfd_add_nego_confirm_ie(p2p, wfdie);
                        break;
                    }
                    case P2P_INVITE_REQ:
                    {
                        wfdielen =vm_wfd_add_invitation_req_ie(p2p, wfdie);
                        break;
                    }
                    case P2P_INVITE_RESP:
                    {
                        wfdielen =vm_wfd_add_invitation_resp_ie(p2p, wfdie);
                        break;
                    }
                    case P2P_DEVDISC_REQ:
                        break;
                    case P2P_DEVDISC_RESP:
                        break;
                    case P2P_PROVISION_DISC_REQ:
                    {
                        wfdielen =vm_wfd_add_provdisc_req_ie(p2p, wfdie);
                        break;
                    }
                    case P2P_PROVISION_DISC_RESP:
                    {
                        wfdielen =vm_wfd_add_provdisc_resp_ie(p2p, wfdie);
                        break;
                    }
                    default:

                        break;
                }
            }
            else
            {
                AML_PRINT(AML_DBG_MODULES_P2P,"p2p_public_action not p2p action\n");
            }
        }
        break;

        case AML_CATEGORY_P2P:
        {
            switch (p2p_act->subtype)
            {
                case P2P_NOA:
                case P2P_PRESENCE_REQ:
                case P2P_PRESENCE_RESP:
                case P2P_GO_DISC_REQ:
                    AML_PRINT(AML_DBG_MODULES_P2P,"p2p_public_action %s \n",
                            p2p_action_tmp[p2p_pub_act->subtype]);
                    break;
                default:
                    AML_PRINT(AML_DBG_MODULES_P2P,"ACTION_CAT subtype %d UNKNOWN\n",
                            p2p_act->subtype);
                    break;
            }
        }
        break;
        default:
        {
           AML_PRINT(AML_DBG_MODULES_P2P,"not p2p action frame category=%d\n",
                    p2p_pub_act->category);
        }
        break;
    }
    return (len+wfdielen);
}


#endif //CONFIG_WFD
 int vm_p2p_bsschan_in_chanList(struct wifi_mac_p2p *p2p,
                                        const unsigned char *frm, unsigned int len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned char *pattr_temp ;
    int i =0;
    int attr_contentlen = 0;
    unsigned char *pattr = NULL;
    struct p2p_ie_chan_list_attr *chan_list_attr = NULL;
    struct p2p_ie_chanlist_attr_chan_entry * chan_entry = NULL;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR)
        || (wifi_mac_is_wm_running(wifimac) == false))
    {
        return true;
    }


    p2p_ie = vm_get_p2p_ie( frm, len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_CHANNEL_LIST)) != NULL)
        {
            chan_list_attr = (struct p2p_ie_chan_list_attr *)(pattr-3);
            chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)&chan_list_attr->chan_entry;
            attr_contentlen = chan_list_attr->len;
            ASSERT(attr_contentlen <p2p_ielen);
            pattr_temp = (unsigned char *)&chan_list_attr->chan_entry;
            attr_contentlen -= 3;

            while (attr_contentlen >= (chan_entry->num + 2))
            {
                for(i=0; i < chan_entry->num; i++)
                {
                    if(chan_entry->chan_list[i] == main_vmac_chan->chan_pri_num)
                    {
                        return true;
                    }
                }
                pattr_temp += chan_entry->num+2;
                attr_contentlen -= (chan_entry->num+2);
                chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)pattr_temp;
            }
        }

        if(len  <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                            len -(p2p_ie -frm + p2p_ielen),
                            NULL, &p2p_ielen);

    }

    return false;
}

static unsigned char vm_get_p2pie_oper_class(struct wifi_mac_p2p *p2p)
{
    int i = 0, j = 0, k = 0, index = 0, mark = 0;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;
    unsigned char peer_oper_class[16];
    int self_oper_class_num = p2p->self_chan_list.oper_class_num;
    int peer_oper_class_num = p2p->peer_chan_list.oper_class_num;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR) ||(wifi_mac_is_wm_running(wifimac) == false))
    {
        return 0;
    }
    if (!self_oper_class_num) {
        mark = 1;
    }
    for (i = 0; i < self_oper_class_num; i++)
    {
        for (j = 0; j < p2p->self_chan_list.opera_class_and_chan[i].per_chan_num; j++) {
            if (p2p->self_chan_list.opera_class_and_chan[i].operating_class == main_vmac_chan->global_operating_class &&
                p2p->self_chan_list.opera_class_and_chan[i].chan_pri_num[j] == main_vmac_chan->chan_pri_num) {
                mark = 1;
                break;
            }
        }
    }
    for (i = 0; i < peer_oper_class_num; i++)
    {
        for (j = 0; j < p2p->peer_chan_list.opera_class_and_chan[i].per_chan_num; j++) {
            if (p2p->peer_chan_list.opera_class_and_chan[i].operating_class == main_vmac_chan->global_operating_class &&
                p2p->peer_chan_list.opera_class_and_chan[i].chan_pri_num[j] == main_vmac_chan->chan_pri_num && mark == 1) {
                return main_vmac_chan->global_operating_class;
            }
        }
    }

    for (i = 0; i < peer_oper_class_num; i++)
    {
        for (j = 0; j < p2p->peer_chan_list.opera_class_and_chan[i].per_chan_num; j++)
        {
            if (p2p->peer_chan_list.opera_class_and_chan[i].chan_pri_num[j] == main_vmac_chan->chan_pri_num) {
                if (!self_oper_class_num) {
                    return p2p->peer_chan_list.opera_class_and_chan[i].operating_class;
                }
                peer_oper_class[index++] = p2p->peer_chan_list.opera_class_and_chan[i].operating_class;
            }
        }
    }

    for (i = 0; i < self_oper_class_num; i++)
    {
        for (j = 0; j < p2p->self_chan_list.opera_class_and_chan[i].per_chan_num; j++)
        {
            if (p2p->self_chan_list.opera_class_and_chan[i].chan_pri_num[j] == main_vmac_chan->chan_pri_num) {
                for (k = 0; k < index; k++)
                {
                    if (peer_oper_class[k] == p2p->self_chan_list.opera_class_and_chan[i].operating_class) {
                        return p2p->self_chan_list.opera_class_and_chan[i].operating_class;
                    }
                }
            }
        }
    }
    return 0;
}

static void vm_get_p2pie_device_info_addr(struct wifi_mac_p2p *p2p , const unsigned char *frm, unsigned int *len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned int  ielt_len = *len -8;
    unsigned char *pattr = NULL;

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
      if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_DEVICE_INFO)) != NULL)
        {
            snprintf(p2p->peer_dev_addr, MAX_MAC_BUF_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
               *pattr, *(pattr + 1), *(pattr + 2), *(pattr + 3), *(pattr + 4), *(pattr + 5));
            return;
        }

        if (ielt_len  <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                              ielt_len -(p2p_ie -frm + p2p_ielen),
                              NULL, &p2p_ielen);
    }

}


static void vm_get_p2pie_listenchannel(struct wifi_mac_p2p *p2p , const unsigned char *frm, unsigned int *len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned int  ielt_len = *len -8;
    unsigned char *pattr = NULL;
    struct p2p_ie_listen_chan_attr *listen_chan_attr = NULL;

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
      if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_LISTEN_CHANNEL)) != NULL)
        {
            listen_chan_attr = (struct p2p_ie_listen_chan_attr *)(pattr-3);
            p2p->peer_listen_channel = listen_chan_attr->channel;
            return;
        }

        if (ielt_len  <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                              ielt_len -(p2p_ie -frm + p2p_ielen),
                              NULL, &p2p_ielen);
    }

}

unsigned char *vm_p2p_get_p2pie_noa_ie(struct wifi_mac_p2p *p2p ,const unsigned char *frm, unsigned int *len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned int  ielt_len = *len - 7;
    unsigned char *pattr = NULL;

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_NOTICE_OF_ABSENCE)) != NULL)
        {
            return (pattr-3);
        }

        if (ielt_len  <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                              ielt_len -(p2p_ie -frm + p2p_ielen),
                              NULL, &p2p_ielen);
    }
    return NULL;
}


static void vm_change_p2pie_channelList(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len)
{
    const unsigned char *p2p_ie;
    unsigned int p2p_ielen;
    unsigned char *pattr_temp ;
    int i = 0;
    int attr_contentlen = 0;
    unsigned char *pattr = NULL;
    unsigned char *p2p_ie_temp = NULL;
    unsigned int ielt_len = *len -8;
    unsigned int last_len = 0;
    unsigned int diff_len = 0;
    struct p2p_ie_chan_list_attr *chan_list_attr = NULL;
    struct p2p_ie_chanlist_attr_chan_entry *chan_entry = NULL;

    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR) || (wifi_mac_is_wm_running(wifimac) == false))
    {
        return;
    }

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        p2p_ie_temp = (unsigned char *)p2p_ie;
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_CHANNEL_LIST)) != NULL)
        {
            chan_list_attr = (struct p2p_ie_chan_list_attr *)(pattr - 3);
            chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)&chan_list_attr->chan_entry;

            attr_contentlen = chan_list_attr->len;
            ASSERT(attr_contentlen <p2p_ielen);
            if (attr_contentlen < 6)
            {
                pattr_temp = (unsigned char *)&chan_list_attr->chan_entry;
                attr_contentlen -= 3;

                while (attr_contentlen >= (chan_entry->num + 2))
                {
                    for (i = 0; i < chan_entry->num; i++)
                        chan_entry->chan_list[i] = main_vmac_chan->chan_pri_num;

                    pattr_temp += chan_entry->num+2;
                    attr_contentlen -= (chan_entry->num+2);
                    chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)pattr_temp;
                }
            }
            else
            {
                last_len = ielt_len - (pattr - p2p_ie) - attr_contentlen - sizeof(struct wifi_frame);
                chan_list_attr->len = 0x6;
                chan_entry->chan_list[0] = main_vmac_chan->chan_pri_num;
                chan_entry->num = 0x1;
                chan_entry->operat_class = vm_get_p2pie_oper_class(p2p);
                diff_len = attr_contentlen - chan_list_attr->len;
                *len -= diff_len;
                *(p2p_ie_temp + 1) = p2p_ielen - diff_len - 2;
                memcpy(pattr + chan_list_attr->len, pattr + attr_contentlen, last_len);
            }
        }

        if (ielt_len <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        //Get the next P2P IE
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                            ielt_len -(p2p_ie -frm + p2p_ielen),
                            NULL, &p2p_ielen);
    }
}

void vm_set_p2pie_channelList(struct wifi_mac_p2p *p2p ,
                            const unsigned char *frm, unsigned int len,int channel)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned char *pattr_temp ;
    int i =0;
    int attr_contentlen = 0;
    unsigned char *pattr = NULL;
    struct p2p_ie_chan_list_attr *chan_list_attr = NULL;
    struct p2p_ie_chanlist_attr_chan_entry * chan_entry = NULL;

    AML_PRINT(AML_DBG_MODULES_P2P,"channel = %d\n", channel);

    p2p_ie = vm_get_p2p_ie( frm, len, NULL, &p2p_ielen );

    while ( p2p_ie != NULL)
    {
        if((pattr=vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_CHANNEL_LIST))!=NULL)
        {
            chan_list_attr = (struct p2p_ie_chan_list_attr *)(pattr-3);
            chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)&chan_list_attr->chan_entry;

            attr_contentlen = chan_list_attr->len;
            ASSERT(attr_contentlen <p2p_ielen);
            pattr_temp = (unsigned char *)&chan_list_attr->chan_entry;
            attr_contentlen -= 3;

            while (attr_contentlen >= (chan_entry->num + 2))
            {
                for(i=0; i<chan_entry->num; i++)
                {
                    chan_entry->chan_list[i]= channel;
                }

                pattr_temp += chan_entry->num+2;
                attr_contentlen -= (chan_entry->num+2);
                chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)pattr_temp;
            }
        }

        if(len  <= 2+(p2p_ie -frm + p2p_ielen))
            break;
        //Get the next P2P IE
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                            len -(p2p_ie -frm + p2p_ielen),
                            NULL, &p2p_ielen);

    }

}

static void
vm_change_p2pie_operchannel(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned char *pattr = NULL;
    unsigned int ielt_len = *len - 8;
    struct p2p_ie_operate_chan_attr *operate_chan_attr = NULL;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR) ||(wifi_mac_is_wm_running(wifimac) == false))
    {
        return;
    }

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_OPERATING_CHANNEL)) != NULL)
        {
            operate_chan_attr = (struct p2p_ie_operate_chan_attr *)(pattr - 3);
            operate_chan_attr->operat_class = vm_get_p2pie_oper_class(p2p);
            operate_chan_attr->channel = main_vmac_chan->chan_pri_num;
            AML_PRINT(AML_DBG_MODULES_P2P, "channel=%d\n", operate_chan_attr->channel);
        }

        if (ielt_len <= 2 + (p2p_ie -frm + p2p_ielen))
            break;

        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,  ielt_len -(p2p_ie -frm + p2p_ielen), NULL, &p2p_ielen);
    }
}

void vm_change_p2pie_listenchannel(struct wifi_mac_p2p *p2p , const unsigned char *frm, unsigned int len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned char *pattr = NULL;
    struct p2p_ie_listen_chan_attr *listen_chan_attr = NULL;
    struct wifi_mac *wifimac = NULL;

    if (!(p2p->wnet_vif) || (p2p->wnet_vif->vm_curchan == WIFINET_CHAN_ERR)
        ||(wifi_mac_is_wm_running(wifimac)==false))
    {
        return;
    }
    wifimac = p2p->wnet_vif->vm_wmac;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");
    p2p_ie = vm_get_p2p_ie( frm, len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        //DPRINTF(AML_DEBUG_P2P,"%s %d \n", __func__, __LINE__);

        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_LISTEN_CHANNEL))!=NULL)
        {
            listen_chan_attr = (struct p2p_ie_listen_chan_attr *)(pattr-3);
            listen_chan_attr->channel = p2p->wnet_vif->vm_curchan->chan_pri_num;
            AML_PRINT(AML_DBG_MODULES_P2P,"channel = %d\n",
                    p2p->wnet_vif->vm_curchan->chan_pri_num);
        }

        if(len  <= 2+(p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                    len -(p2p_ie -frm + p2p_ielen),
                    NULL, &p2p_ielen);

    }

}

void vm_change_p2pie_channel(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len)
{
    vm_change_p2pie_operchannel(p2p,frm,len);
    vm_change_p2pie_channelList(p2p,frm,len);
}

const unsigned char *
vm_get_p2p_ie(const unsigned char *in_ie, unsigned int in_len,
    unsigned char *p2p_ie, unsigned int *p2p_ielen)
{
    unsigned int cnt = 0;
    unsigned char eid;

    AML_PRINT(AML_DBG_MODULES_P2P,"in_len=%d\n", in_len);


    ASSERT(p2p_ielen != NULL);
    *p2p_ielen = 0;

    if (!in_ie || in_len<=0)
        return NULL;

    while (cnt<in_len)
    {
        eid = in_ie[cnt];

        if ( ( eid == WIFINET_ELEMID_VENDOR ) && isp2poui(&in_ie[cnt]) )
        {
            *p2p_ielen = saveie(p2p_ie,&in_ie[cnt]);
            return &in_ie[cnt];
        }
        else
        {
            cnt += in_ie[ cnt + 1 ] +2;
        }
    }
    return NULL;

}

static void
vm_change_p2pie_go_intent(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len, unsigned char tx, unsigned char p2p_action_type)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned char *pattr = NULL;
    unsigned int ielt_len = *len - 8;
    unsigned int  intent_val;
    struct p2p_ie_intent_attr *intent_attr = NULL;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR) ||(wifi_mac_is_wm_running(wifimac) == false))
    {
        return;
    }

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_GROUP_OWNER_INTENT)) != NULL)
        {
            intent_attr = (struct p2p_ie_intent_attr *)(pattr - 3);
            intent_val = intent_attr->go_intent >> 1;
            if (tx && p2p_action_type == P2P_GO_NEGO_REQ) {
                if (intent_val < 15) {
                    p2p->change_intent_flag = 1;
                    intent_attr->go_intent &= ~0x1e;//11110
                }
            }
            else if (!tx && p2p_action_type == P2P_GO_NEGO_RESP) {
                if (p2p->change_intent_flag) {
                    intent_attr->go_intent |= 0x1e;
                }
            }
            else if (!tx && p2p_action_type == P2P_GO_NEGO_REQ) {
                if (p2p->change_intent_flag) {
                    intent_attr->go_intent |= 0x1e;
                }
            }
            else if (tx && p2p_action_type == P2P_GO_NEGO_RESP) {
                if (intent_val < 15) {
                    intent_attr->go_intent &= ~0x1e;
                }
            }
            AML_PRINT(AML_DBG_MODULES_P2P, "p2p_action_type:%d, go_intent=%d\n", p2p_action_type, intent_attr->go_intent);
        }

        if (ielt_len <= 2 + (p2p_ie -frm + p2p_ielen))
            break;

        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,  ielt_len -(p2p_ie -frm + p2p_ielen), NULL, &p2p_ielen);
    }
}

unsigned char *vm_p2p_get_p2pie_attrib(const void *frm, unsigned char element_id)
{
    struct wifi_mac_p2p_ie *ie = NULL;
    unsigned short len = 0;
    unsigned char *subel;
    unsigned char subelt_id;
    unsigned short subelt_len;

    //DPRINTF(AML_DEBUG_P2P,"%s\n", __func__);
    if (!frm)
    {
        ERROR_DEBUG_OUT("frm=NULL error\n");
        return NULL;
    }

    ie = (struct wifi_mac_p2p_ie*) frm;
    len = ie->len;

    subel = ie->subelts;
    len -= 4;

    while (len >= 3)
    {
        subelt_id = *subel;
        subel += 1;
        len -= 1;

        subelt_len = *subel++;
        subelt_len |= *subel++ << 8;

        len -= 2;

        if (len < subelt_len)
        {
            pr_err("w1: %s: subelt_len is greater than len: attr span several "
                   "IEs. But this case is not supported. "
                   "subelt_len = %x, len = %x\n",
                   __func__, subelt_len, len);
            return NULL;
        }

        len -= subelt_len;

        if (subelt_id == element_id)
        {
            return subel;
        }

        subel += subelt_len;
    }

    return NULL;
}

unsigned char *vm_p2p_get_p2pie_attrib_with_id(void *frm, unsigned char element_id)
{
    unsigned char *p2pie_attrib_without_id = vm_p2p_get_p2pie_attrib(frm, element_id);
    unsigned char *p2pie_attrib_with_id = NULL;

    if (p2pie_attrib_without_id)
    {
        p2pie_attrib_with_id = p2pie_attrib_without_id - 3;
    }
    return p2pie_attrib_with_id;
}

void vm_p2p_print_attr(const void *frm)
{
    struct wifi_mac_p2p_ie *ie = NULL;
    signed short len = 0;
    unsigned char *subel;
    unsigned char subelt_id;
    unsigned short subelt_len;
    unsigned short i;

    if (!frm)
    {
        ERROR_DEBUG_OUT("frm = NULL error\n");
        return;
    }
    if (READ_32B((unsigned char *)(frm + 2)) == WFD_OUI_BE)
    {
        frm += *(unsigned char *)(frm + 1) + 2;
        if (!frm || (*(unsigned char *)(frm) != WIFINET_ELEMID_VENDOR))
        {
            ERROR_DEBUG_OUT("frm = NULL error\n");
            return;
        }
    }

    ie = (struct wifi_mac_p2p_ie*) frm;
    len = ie->len;

    subel = ie->subelts;
    len -= 4;

    while (len >= 3)
    {
        subelt_id = *subel;
        subel += 1;
        len -= 1;

        subelt_len = *subel++;
        subelt_len |= *subel++ << 8;

        len -= 2;
        len -= subelt_len;
        AML_PRINT(AML_DBG_MODULES_P2P, "chris p2p ie: %02x:%04x: ", subelt_id, subelt_len);
        for (i = 0; i < subelt_len; ++i) {
            AML_PRINT(AML_DBG_MODULES_P2P, "%02x ", *(subel + i));
        }
        AML_PRINT(AML_DBG_MODULES_P2P, "\n");

        subel += subelt_len;
    }

    return;
}

#if 0
void vm_p2p_print_buf(const void *frm, int len)
{
    int i = 0;
    unsigned char *ie = (unsigned char *)frm;
    for (i = 0; i < len; i++)
    {
        pr_debug("%02x:", *(ie + i));
        if (i % 10 == 0)
            pr_debug("\n");
    }
}
#endif

static void vm_get_p2pie_channelList(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len, unsigned char tx)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned int  ielt_len = *len -8;
    unsigned char *pattr_temp ;
    int i = 0, index = 0;
    int attr_contentlen = 0;
    unsigned char *pattr = NULL;
    struct p2p_ie_chan_list_attr *chan_list_attr = NULL;
    struct p2p_ie_chanlist_attr_chan_entry * chan_entry = NULL;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR)
        || (wifi_mac_is_wm_running(wifimac) == false))
    {
        return;
    }

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_CHANNEL_LIST)) != NULL)
        {
            chan_list_attr = (struct p2p_ie_chan_list_attr *)(pattr-3);
            chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)&chan_list_attr->chan_entry;
            attr_contentlen = chan_list_attr->len;
            ASSERT(attr_contentlen <p2p_ielen);
            pattr_temp = (unsigned char *)&chan_list_attr->chan_entry;
            attr_contentlen -= 3;
            while (attr_contentlen >= (chan_entry->num + 2))
            {
                if (tx) {
                    p2p->self_chan_list.opera_class_and_chan[index].operating_class = chan_entry->operat_class;
                    for (i=0; i < chan_entry->num; i++)
                    {
                        p2p->self_chan_list.opera_class_and_chan[index].chan_pri_num[i] = chan_entry->chan_list[i];
                        p2p->self_chan_list.opera_class_and_chan[index].per_chan_num = chan_entry->num;
                    }
                }else {
                    p2p->peer_chan_list.opera_class_and_chan[index].operating_class = chan_entry->operat_class;
                    for (i=0; i < chan_entry->num; i++)
                    {
                        p2p->peer_chan_list.opera_class_and_chan[index].chan_pri_num[i] = chan_entry->chan_list[i];
                        p2p->peer_chan_list.opera_class_and_chan[index].per_chan_num = chan_entry->num;
                    }
                }
                pattr_temp += chan_entry->num+2;
                attr_contentlen -= (chan_entry->num+2);
                chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)pattr_temp;
                index++;
            }
        }

        if (ielt_len  <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                              ielt_len -(p2p_ie -frm + p2p_ielen),
                              NULL, &p2p_ielen);
    }

    if (tx) {
        p2p->self_chan_list.oper_class_num = index;
    } else {
        p2p->peer_chan_list.oper_class_num = index;
    }
    return;
}

static int vm_check_p2pie_channelList(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len)
{
    const unsigned char  *p2p_ie;
    unsigned int  p2p_ielen;
    unsigned int  ielt_len = *len -8;
    unsigned char *pattr_temp ;
    int i =0;
    int attr_contentlen = 0;
    unsigned char *pattr = NULL;
    struct p2p_ie_chan_list_attr *chan_list_attr = NULL;
    struct p2p_ie_chanlist_attr_chan_entry * chan_entry = NULL;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    struct wifi_channel *main_vmac_chan = NULL;

    main_vmac_chan = wifi_mac_get_main_vmac_channel(wifimac);
    if ((main_vmac_chan == WIFINET_CHAN_ERR)
        || (wifi_mac_is_wm_running(wifimac) == false))
    {
        return false;
    }

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  P2P_ATTR_CHANNEL_LIST)) != NULL)
        {
            chan_list_attr = (struct p2p_ie_chan_list_attr *)(pattr-3);
            chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)&chan_list_attr->chan_entry;
            attr_contentlen = chan_list_attr->len;
            ASSERT(attr_contentlen <p2p_ielen);
            pattr_temp = (unsigned char *)&chan_list_attr->chan_entry;
            attr_contentlen -= 3;
            while (attr_contentlen >= (chan_entry->num + 2))
            {
                for (i=0; i < chan_entry->num; i++)
                {
                    if (chan_entry->chan_list[i] == main_vmac_chan->chan_pri_num)
                    {
                        return true;
                    }
                }
                pattr_temp += chan_entry->num+2;
                attr_contentlen -= (chan_entry->num+2);
                chan_entry = (struct p2p_ie_chanlist_attr_chan_entry *)pattr_temp;
            }
        }

        if (ielt_len  <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                              ielt_len -(p2p_ie -frm + p2p_ielen),
                              NULL, &p2p_ielen);
    }

    return false;
}

static unsigned int vm_get_p2pie_len(struct wifi_mac_p2p *p2p, const unsigned char *frm, unsigned int *len, enum p2p_attr_id id)
{
    const unsigned char *p2p_ie;
    unsigned int p2p_ielen;
    unsigned char *pattr = NULL;
    unsigned char *p2p_ie_temp = NULL;
    unsigned int ielt_len = *len -8;

    p2p_ie = vm_get_p2p_ie( frm, ielt_len, NULL, &p2p_ielen );
    while (p2p_ie != NULL)
    {
        p2p_ie_temp = (unsigned char *)p2p_ie;
        if ((pattr = vm_p2p_get_p2pie_attrib(p2p_ie,  id)) != NULL)
        {
            return (*(pattr -2) + *(pattr -1));
        }
        if (ielt_len <= 2 + (p2p_ie -frm + p2p_ielen))
            break;
        //Get the next P2P IE
        p2p_ie = vm_get_p2p_ie(p2p_ie+p2p_ielen,
                            ielt_len -(p2p_ie -frm + p2p_ielen),
                            NULL, &p2p_ielen);
    }
    return 0;
}

void set_p2p_negotiation_status(struct wifi_station *sta, enum NET80211_P2P_NEGO_STATE status) {
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_p2p *p2p;

    if ((wnet_vif != NULL) && (wnet_vif->vm_p2p_support)) {
        p2p = wnet_vif->vm_p2p;
        p2p->p2p_negotiation_state = status;
        pr_debug("%s p2p_negotiation_state:%d\n", __func__, p2p->p2p_negotiation_state);
    }
}

unsigned char is_p2p_negotiation_complete(struct wifi_mac *wifimac) {
    struct drv_private *drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *p2p_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
    struct wifi_mac_p2p *p2p;

    if ((p2p_wnet_vif != NULL) && (p2p_wnet_vif->vm_p2p_support)) {
        p2p = p2p_wnet_vif->vm_p2p;

        if ((p2p->p2p_negotiation_state > NET80211_P2P_STATE_TX_IDLE)
            && (p2p->p2p_negotiation_state < NET80211_P2P_STATE_GO_COMPLETE)) {
            return 1;
        }
        pr_debug("%s p2p_negotiation_state:%d\n", __func__, p2p->p2p_negotiation_state);
    }

    return 0;
}

unsigned char is_need_process_p2p_action(unsigned char* buf) {
    struct wifi_mac_p2p_pub_act_frame *p2p_pub_act = NULL;
    struct wifi_mac_p2p_action_frame *p2p_act = NULL;
    unsigned ret = 0;
    char category = *(buf + sizeof(struct wifi_frame));
    p2p_act = (struct wifi_mac_p2p_action_frame *)(buf + sizeof(struct wifi_frame));
    p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)(buf + sizeof(struct wifi_frame));
    AML_PRINT(AML_DBG_MODULES_P2P, "category:%d, subtype:%d, action:%d\n", category, p2p_act->subtype, p2p_pub_act->action);
    switch(category) {
        case AML_CATEGORY_P2P:
            if (p2p_act->subtype == P2P_PRESENCE_REQ
                || p2p_act->subtype == P2P_PRESENCE_RESP
                || p2p_act->subtype == P2P_GO_DISC_REQ) {
                ret = 1;
            }
            break;
        case AML_CATEGORY_PUBLIC:
            if (p2p_pub_act->action == WIFINET_ACT_PUBLIC_GAS_RSP
                || (p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P
                && (p2p_pub_act->subtype == P2P_DEVDISC_REQ
                || p2p_pub_act->subtype == P2P_DEVDISC_RESP))) {
                ret = 1;
            }
            break;
      default:
          break;
      }

    return ret;
}


static void vm_p2p_switch_nego_state(struct wifi_mac_p2p *p2p, struct wifi_mac_p2p_pub_act_frame *frm, unsigned char tx, int *len)
{
    unsigned int chanList_len = vm_get_p2pie_len(p2p, (const unsigned char *)frm->elts, len, P2P_ATTR_CHANNEL_LIST);
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;
    int tmplen = 0;

    switch (frm->subtype) {
        case P2P_PROVISION_DISC_REQ:
        case P2P_PROVISION_DISC_RESP:
            if (tx) {
                p2p->p2p_negotiation_state = NET80211_P2P_STATE_TX_IDLE;
                p2p->change_intent_flag = 0;
                p2p->p2p_flag &= (~P2P_CHECK_TX_CHANNEL_LIST_PASS);
                p2p->p2p_flag &= (~P2P_CHECK_RX_CHANNEL_LIST_PASS);
                p2p->p2p_flag &= (~P2P_CHANGE_CHANNEL_LIST);
                p2p->peer_listen_channel = 0;
                memset(p2p->peer_dev_addr, 0, MAX_MAC_BUF_LEN);
            }
            break;

        case P2P_INVITE_REQ:
            if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_TX_IDLE) {
                vm_get_p2pie_channelList(p2p, frm->elts, len, tx);
                if (tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_TX_INVITE_REQ;
                    if (vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHECK_TX_CHANNEL_LIST_PASS;
                    }
                } else {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_RX_INVITE_REQ;
                    if (vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHECK_RX_CHANNEL_LIST_PASS;
                    }
                }
            }
            break;

        case P2P_INVITE_RESP:
            vm_get_p2pie_channelList(p2p, frm->elts, len, tx);
            if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_TX_INVITE_REQ) {
                if (!tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_RX_INVITE_RSP;
                    if ((p2p->p2p_flag & P2P_CHECK_TX_CHANNEL_LIST_PASS) && vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHANGE_CHANNEL_LIST;
                    }
                }

            } else if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_RX_INVITE_REQ) {
                if (tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_TX_INVITE_RSP;
                    if ((p2p->p2p_flag & P2P_CHECK_RX_CHANNEL_LIST_PASS) && vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHANGE_CHANNEL_LIST;
                    }
                }
            }
            if (0 == *(frm->elts + 9)) //status
            {
                p2p->p2p_negotiation_state = NET80211_P2P_STATE_GONEGO_CONF;
                if (vm_get_wfd_ie(frm->elts, *len - 8, NULL, &tmplen)) {
                    pr_debug("invite rsp set is_miracast_connect 1\n");
                    wifimac->is_miracast_connect = 1;
                }
            }else {
                p2p->p2p_negotiation_state = NET80211_P2P_STATE_TX_IDLE;
                p2p->change_intent_flag = 0;
                p2p->p2p_flag &= (~P2P_CHECK_TX_CHANNEL_LIST_PASS);
                p2p->p2p_flag &= (~P2P_CHECK_RX_CHANNEL_LIST_PASS);
                p2p->p2p_flag &= (~P2P_CHANGE_CHANNEL_LIST);
                p2p->peer_listen_channel = 0;
                memset(p2p->peer_dev_addr, 0, MAX_MAC_BUF_LEN);
            }
            break;

        case P2P_GO_NEGO_REQ:
            if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_TX_IDLE) {
                vm_get_p2pie_channelList(p2p, frm->elts, len, tx);
                if (tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_FIRST_TX_NEGO_REQ;
                    if (vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHECK_TX_CHANNEL_LIST_PASS;
                    }
                } else {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_FIRST_RX_NEGO_REQ;
                    vm_get_p2pie_listenchannel(p2p, frm->elts, len);
                    vm_get_p2pie_device_info_addr(p2p, frm->elts, len);
                    if (vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHECK_RX_CHANNEL_LIST_PASS;
                    }
                }

            } else if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_FIRST_RX_NEGO_RSP) {
                if (!tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_SECOND_RX_NEGO_REQ;
                }

            } else if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_FIRST_TX_NEGO_RSP) {
                if (tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_SECOND_TX_NEGO_REQ;
                }
            }
            break;

        case P2P_GO_NEGO_RESP:
            if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_FIRST_TX_NEGO_REQ) {
                if (!tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_FIRST_RX_NEGO_RSP;
                    vm_get_p2pie_channelList(p2p, frm->elts, len, tx);
                    if ((p2p->p2p_flag & P2P_CHECK_TX_CHANNEL_LIST_PASS) && vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                        p2p->p2p_flag |= P2P_CHANGE_CHANNEL_LIST;
                    }
                }

            } else if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_FIRST_RX_NEGO_REQ) {
                if (tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_FIRST_TX_NEGO_RSP;
                    vm_get_p2pie_channelList(p2p, frm->elts, len, tx);
                    if (p2p->p2p_flag & P2P_CHECK_RX_CHANNEL_LIST_PASS) {
                        if ((3 == chanList_len) || vm_check_p2pie_channelList(p2p, frm->elts, len)) {
                            p2p->p2p_flag |= P2P_CHANGE_CHANNEL_LIST;
                        }
                    }
                }

            } else if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_SECOND_RX_NEGO_REQ) {
                if (tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_SECOND_TX_NEGO_RSP;
                }

            } else if (p2p->p2p_negotiation_state == NET80211_P2P_STATE_SECOND_TX_NEGO_REQ) {
                if (!tx) {
                    p2p->p2p_negotiation_state = NET80211_P2P_STATE_SECOND_RX_NEGO_RSP;
                }
            }
            break;

        case P2P_GO_NEGO_CONF:
            p2p->p2p_negotiation_state = NET80211_P2P_STATE_GONEGO_CONF;
            p2p->peer_listen_channel = 0;
            memset(p2p->peer_dev_addr, 0 , MAX_MAC_BUF_LEN);
            if (0 == *(frm->elts + 9)) {
                if (vm_get_wfd_ie(frm->elts, *len - 8, NULL, &tmplen)) {
                    pr_debug("conf set is_miracast_connect 1\n");
                    wifimac->is_miracast_connect = 1;
                }
            }
            break;

        default:
            break;
    }
}

int vm_p2p_parse_negotiation_frames(struct wifi_mac_p2p *p2p,
    const unsigned char *frm, unsigned int *len, unsigned char tx)
{
    int is_p2p_frame = 0;
    struct wifi_mac_p2p_pub_act_frame *p2p_pub_act = NULL;
    struct wifi_mac_p2p_action_frame *p2p_act = NULL;
    char *printbuf = NULL;
    int buflen = 0;
    unsigned char p2p_action_type = 0xff;
    struct wifi_mac *wifimac = p2p->wnet_vif->vm_wmac;

    p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)(frm + sizeof(struct wifi_frame));
    p2p_act = (struct wifi_mac_p2p_action_frame *)(frm + sizeof(struct wifi_frame));

    printbuf = ZMALLOC(128, "printbuf", GFP_ATOMIC);
    if (printbuf == NULL) {
        DPRINTF(AML_DEBUG_ERROR,"%s %d\n", __func__, __LINE__);
        return -ENOMEM;
    }
    buflen += sprintf(printbuf+buflen, "%s ACT_P2P %s ", __func__, (tx)?"Tx":"Rx");

    switch (p2p_pub_act->category) {
        case AML_CATEGORY_PUBLIC:
            if (p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P) {
                p2p_action_type = p2p_pub_act->subtype;
                is_p2p_frame = true;
                buflen += sprintf(printbuf+buflen, "%s>%s dialog_token=%d", tx?"tx":"rx",
                    p2p_public_action_tmp[p2p_pub_act->subtype], p2p_pub_act->dialog_token);
                pr_debug("%s\n", printbuf);

            } else {
                 AML_PRINT(AML_DBG_MODULES_P2P,"p2p_public_action not p2p action\n");
            }
            break;

        case AML_CATEGORY_P2P:
            is_p2p_frame = true;
            AML_PRINT(AML_DBG_MODULES_P2P,"AML_CATEGORY_P2P:%d\n", p2p_act->subtype);
            break;

        default:
            AML_PRINT(AML_DBG_MODULES_P2P," not p2p action frame category=%d\n", p2p_pub_act->category);
            break;
    }

    if (p2p_action_type != 0xff) {
        wifimac->is_miracast_connect = 0;
        vm_p2p_switch_nego_state(p2p, p2p_pub_act, tx, len);

        switch (p2p_action_type) {
            case P2P_GO_NEGO_REQ:
            case P2P_GO_NEGO_RESP:
            case P2P_INVITE_RESP:
                if (p2p_action_type != P2P_INVITE_RESP) {
#ifndef WFA_P2P_TEST
                    vm_change_p2pie_go_intent(p2p, p2p_pub_act->elts, len, tx, p2p_action_type);
#endif
                }
                /*if (p2p->p2p_flag & P2P_CHANGE_CHANNEL_LIST) {
                    vm_change_p2pie_channel(p2p,p2p_pub_act->elts, len);
                }*/
                break;

            default:
                break;
        }
        vm_p2p_print_attr((char *)p2p_pub_act + 8);
    }

    FREE(printbuf, "printbuf");
    return is_p2p_frame;
}

void vm_p2p_scanstart(struct wiphy *wiphy, struct net_device *ndev,
                      struct cfg80211_scan_request *request)
{
    struct cfg80211_ssid *ssids = request->ssids;
    struct wifi_mac_p2p *p2p = wiphy_to_p2p(wiphy);
    unsigned int p2p_ielen =0;

    vm_p2p_cancel_remain_channel(p2p);

    if (request->n_ssids == 0)
    {
        DPRINTF(AML_DEBUG_ERROR|AML_DEBUG_SCAN,"%s  ssids->ssid == NULL ERR\n", __func__);
        return ;
    }

    if ((memcmp(ssids->ssid, P2P_WILDCARD_SSID, P2P_WILDCARD_SSID_LEN) == 0)
         && vm_get_p2p_ie((unsigned char *)request->ie, request->ie_len, NULL, &p2p_ielen))
    {
        if (!vm_p2p_enabled(p2p))
            vm_p2p_set_enable(p2p, 1);

        if (!vm_wfd_enabled(p2p))
            vm_wfd_set_enable(p2p, 1);

        vm_p2p_set_state(p2p, NET80211_P2P_STATE_SCAN);

        if (request->n_channels == SOCIAL_CHAN_CNT &&
            request->channels[0]->center_freq == SOCIAL_CHAN_1 &&
            request->channels[1]->center_freq == SOCIAL_CHAN_2 &&
            request->channels[2]->center_freq == SOCIAL_CHAN_3)
        {
            p2p->social_channel = 1;
            AML_PRINT(AML_DBG_MODULES_P2P,
                    "use social channel\n");
        }
        else
        {
            p2p->social_channel = 0;
            AML_PRINT(AML_DBG_MODULES_P2P,"not use social channel\n");
        }
    }

    if (request->ie && request->ie_len>0)
    {
        vm_p2p_set_wpsp2pie( ndev, (unsigned char *)request->ie, request->ie_len ,P2P_PROBE_REQ_IE);
    }
}

void vm_p2p_scanend(struct wifi_mac_p2p *p2p)
{
    vm_p2p_set_state(p2p, NET80211_P2P_STATE_IDLE);
    p2p->social_channel = 0;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");
}


static void vm_p2p_generate_bss_mac(unsigned char *primary_addr,
    unsigned char *out_dev_addr, unsigned char *out_interface_addr)
{
    memset(out_dev_addr, 0, sizeof(*out_dev_addr));
    memset(out_interface_addr, 0, sizeof(*out_interface_addr));

    memcpy(out_dev_addr, primary_addr, WIFINET_ADDR_LEN);
    //out_dev_addr[0] |= 0x02;


    memcpy(out_interface_addr, out_dev_addr, WIFINET_ADDR_LEN);
    out_interface_addr[4] ^= 0x80;
    AML_PRINT(AML_DBG_MODULES_P2P,"p2p_mac_addr %s\n",
            ether_sprintf(out_interface_addr));

}

unsigned int vm_p2p_rx_assocreq(struct wifi_mac_p2p *p2p,
                unsigned char *p2pie, struct wifi_station *sta)
{
    unsigned char *p2p_attr=NULL;
    unsigned short cap_attr=0;
    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");

    if (!vm_p2p_chk_p2p_role(p2p, NET80211_P2P_ROLE_GO))
    {
        AML_PRINT(AML_DBG_MODULES_P2P,"p2p role=%d\n",
                 vm_p2p_role(p2p));
        return P2P_SC_FAIL_UNABLE_TO_ACCOMMODATE;
    }

    if (p2pie == NULL)
    {

        AML_PRINT(AML_DBG_MODULES_P2P,"rx legacy sta assocreq\n");
        //return P2P_SC_FAIL_INVALID_PARAMS;
        return P2P_SC_SUCCESS;
    }

    p2p_attr = vm_p2p_get_p2pie_attrib(p2pie,P2P_ATTR_CAPABILITY);
    if (p2p_attr)
    {
         AML_PRINT(AML_DBG_MODULES_P2P,"Got  P2P_ATTR_CAPABILITY !!\n");
        cap_attr = *(unsigned short *)p2p_attr;
        sta->sta_p2p_dev_cap = le16_to_cpu(cap_attr)&0xff;
    }
    p2p_attr = vm_p2p_get_p2pie_attrib(p2pie,P2P_ATTR_DEVICE_INFO);
    if (p2p_attr)
    {
        AML_PRINT(AML_DBG_MODULES_P2P," Got P2P_ATTR_DEVICE_INFO !!\n");
        memcpy(sta->sta_p2p_dev_addr, p2p_attr, WIFINET_ADDR_LEN);
        p2p_attr += WIFINET_ADDR_LEN;
        memcpy(&sta->sta_p2p_config_methods, p2p_attr, 2);
        sta->sta_p2p_config_methods = be16_to_cpu(sta->sta_p2p_config_methods);

    }

    return P2P_SC_SUCCESS;

}


unsigned int
vm_p2p_discover_listen(struct wifi_mac_p2p *p2p, int channel, unsigned int duration_ms)
{
    AML_PRINT(AML_DBG_MODULES_P2P,"Enter Channel : %d, Duration : %d\n",
            channel, duration_ms);
    if (!vm_p2p_enabled(p2p))
    {
        vm_p2p_set_enable(p2p, 1);
    }
    p2p->wnet_vif->vm_flags |= WIFINET_F_NOSCAN;
    vm_p2p_set_state(p2p, NET80211_P2P_STATE_LISTEN);
    if (p2p->wnet_vif->vm_mainsta != NULL)
    {
        memcpy(p2p->wnet_vif->vm_mainsta->sta_essid,P2P_WILDCARD_SSID,P2P_WILDCARD_SSID_LEN);
        p2p->wnet_vif->vm_mainsta->sta_esslen = P2P_WILDCARD_SSID_LEN;
    }

    os_timer_ex_start_period(&(p2p->listen_timer),  duration_ms);
    return duration_ms;
}

void vm_p2p_cancel_remain_channel(struct wifi_mac_p2p *p2p )
{
    struct wifi_mac_p2p_pub_act_frame *p2p_pub_act = NULL;
    int cnt = 0;

    AML_PRINT(AML_DBG_MODULES_P2P, "++\n");
    p2p_pub_act = (struct wifi_mac_p2p_pub_act_frame *)(p2p->action_pkt + sizeof(struct wifi_frame));

    if (p2p_pub_act->action == WIFINET_ACT_PUBLIC_P2P) {
        while (p2p->tx_status_flag != WIFINET_TX_STATUS_SUCC
               && p2p->act_pkt_retry_count < DEFAULT_P2P_ACTION_RETRY_TIMES && cnt < 50) {
            msleep(20);
            cnt++;
        }
        AML_OUTPUT("cnt %d, flag %d, retry %d\n", cnt, p2p->tx_status_flag, p2p->act_pkt_retry_count);
    }
    if (os_timer_ex_active(&(p2p->listen_timer)))
    {
        os_timer_ex_cancel(&(p2p->listen_timer), CANCEL_SLEEP);
    }
    if (vm_p2p_is_state(p2p, NET80211_P2P_STATE_LISTEN))
    {
        vm_p2p_set_state(p2p, NET80211_P2P_STATE_IDLE);
    }

    if (p2p->wnet_vif->vm_flags & WIFINET_F_NOSCAN)
    {
        p2p->wnet_vif->vm_flags &= ~WIFINET_F_NOSCAN;
        cfg80211_remain_on_channel_expired(p2p->wnet_vif->vm_wdev, p2p->remain_on_ch_cookie,
            &p2p->remain_on_ch_channel, GFP_KERNEL);

        DPRINTF(AML_DEBUG_CFG80211, "%s %d <%s> cancel WIFINET_F_NOSCAN\n",
                __func__, __LINE__, VMAC_DEV_NAME(p2p->wnet_vif));
    }

    if (p2p->need_restore_bsschan != REASON_RESOTRE_BSSCHAN_NONE) {
        wifi_mac_restore_wnet_vif_channel(p2p->wnet_vif);
        p2p->need_restore_bsschan = REASON_RESOTRE_BSSCHAN_NONE;
    }

    if (p2p->wnet_vif->vm_wmac->wm_p2p_connection_protect) {
        p2p->wnet_vif->vm_wmac->wm_p2p_connection_protect = 0;
    }
}

static void
vm_p2p_listen_timer_expired_ex(SYS_TYPE param1,
    SYS_TYPE param2,SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac_p2p *p2p = (struct wifi_mac_p2p *) param1;
    vm_p2p_cancel_remain_channel(p2p);
}


static int vm_p2p_listen_timer_expired(void* data)
{
    struct wifi_mac_p2p *p2p = (struct wifi_mac_p2p *) data;
    struct wlan_net_vif  *wnet_vif=   p2p->wnet_vif;
    struct wifi_mac *wifimac  =   wnet_vif->vm_wmac;

    AML_PRINT(AML_DBG_MODULES_P2P,"++\n");

    wifi_mac_add_work_task(wifimac,vm_p2p_listen_timer_expired_ex, NULL,(SYS_TYPE)data,0,0,0,0 );
    return OS_TIMER_NOT_REARMED;
}

int vm_p2p_initial(struct wifi_mac_p2p *p2p)
{
    int index = 0;
    struct wlan_net_vif *wnet_vif = p2p->wnet_vif;

    pr_debug("%s\n", __func__);
#ifdef CONFIG_WFD
    vm_wfd_initial(p2p);
#endif //CONFIG_WFD
    p2p->p2p_enable = 0;
    p2p->p2p_state = NET80211_P2P_STATE_NONE;
    p2p->p2p_negotiation_state = NET80211_P2P_STATE_TX_IDLE;
    p2p->social_channel = 0;
    p2p->p2p_flag = P2P_NOA_INDEX_INIT_FLAG;
    p2p->action_dialog_token = 0;
    p2p->action_pkt_len = 0;
    p2p->action_retry_time = 0;
    p2p->work_channel = NULL;
    p2p->p2p_listen_count = 0;
    p2p->change_intent_flag = 0;
    p2p->peer_listen_channel = 0;
    p2p->ap_mode_set_noa_enable = 0;
    p2p->send_tx_status_flag = 0;
    memset(p2p->peer_dev_addr, 0, MAX_MAC_BUF_LEN);
    memset(p2p->action_pkt, 0, 256);
    memset(&p2p->self_chan_list, 0, sizeof(p2p->self_chan_list));
    memset(&p2p->peer_chan_list, 0, sizeof(p2p->self_chan_list));
    wifi_mac_reset_vmac(wnet_vif);

    for(index = 0; index < WIFINET_APPIE_NUM_OF_FRAME; index++) {
        wifi_mac_rm_app_ie(&p2p->p2p_app_ie[index]);
        wifi_mac_rm_app_ie(&p2p->noa_app_ie[index]);
    }

    return wifi_mac_rm_app_ie(&p2p->wps_beacon_ie);
}

int vm_p2p_attach(struct wlan_net_vif *wnet_vif)
{
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct wifi_mac_p2p *p2p = &pwdev_priv->p2p;
    int index = 0;

    DPRINTF(AML_DEBUG_WARNING, "%s ++\n", __func__);
    p2p->wnet_vif = wnet_vif;
    p2p->p2p_role=NET80211_P2P_ROLE_DEVICE;
    p2p->need_restore_bsschan = REASON_RESOTRE_BSSCHAN_NONE;

    vm_p2p_initial(p2p);
    concurrent_vsdb_init(wnet_vif->vm_wmac);
    wnet_vif->vm_p2p = p2p;

    for(index=0; index<WIFINET_APPIE_NUM_OF_FRAME; index++)
    {
        p2p->p2p_app_ie[index].ie = NULL;
        p2p->p2p_app_ie[index].length = 0;
#ifdef CONFIG_WFD
        p2p->wfd_app_ie[index].ie = NULL;
        p2p->wfd_app_ie[index].length = 0;
#endif //CONFIG_WFD
        p2p->noa_app_ie[index].ie = NULL;
        p2p->noa_app_ie[index].length = 0;
    }
    p2p->wps_beacon_ie.ie = NULL;
    p2p->wps_beacon_ie.length = 0;

    if (wnet_vif->vm_p2p_support) {
        wnet_vif->vm_wdev->wiphy->interface_modes |= (BIT(NL80211_IFTYPE_P2P_CLIENT) | BIT(NL80211_IFTYPE_P2P_GO));
    }
    os_timer_ex_initialize(&(p2p->listen_timer), P2P_LISTEN_TIMER_PERIOD_DEFAULT,
                           (timer_handler_func)vm_p2p_listen_timer_expired, p2p);
    return 0;
}

void vm_p2p_dettach(void *vm_wdev_priv_ptr)
{
    struct vm_wdev_priv *pwdev_priv = (struct vm_wdev_priv *)vm_wdev_priv_ptr;
    struct wifi_mac_p2p *p2p = &pwdev_priv->p2p;

    DPRINTF(AML_DEBUG_WARNING,"<exit>%s ++ \n", __func__);

    vm_p2p_cancel_remain_channel(p2p);
    os_timer_ex_del(&(p2p->listen_timer), CANCEL_SLEEP);
    vm_p2p_initial(p2p);
    p2p->wnet_vif = NULL;

    pwdev_priv->vm_wdev->wiphy->interface_modes &=  (~(BIT(NL80211_IFTYPE_P2P_CLIENT) | BIT(NL80211_IFTYPE_P2P_GO)));
}

int vm_p2p_up(struct wlan_net_vif *wnet_vif)
{
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct wifi_mac_p2p *p2p = &pwdev_priv->p2p;

    vm_p2p_initial(p2p);
    concurrent_vsdb_init(wnet_vif->vm_wmac);

    AML_OUTPUT("<%s> wnet_vif_id=%d vm_myaddr=%s\n", VMAC_DEV_NAME(wnet_vif), wnet_vif->wnet_vif_id,
            ether_sprintf(wnet_vif->vm_myaddr));

    vm_p2p_generate_bss_mac(wnet_vif->vm_myaddr, p2p->dev_addr, p2p->interface_addr);
    if ((p2p->p2p_role == NET80211_P2P_ROLE_CLIENT) || (p2p->p2p_role == NET80211_P2P_ROLE_GO))
    {
        vm_p2p_set_enable(p2p, 1);
        if (p2p->p2p_role == NET80211_P2P_ROLE_GO) {
            p2p->p2p_negotiation_state = NET80211_P2P_STATE_GO_COMPLETE;
        }
    }
    return 0;
}

void vm_p2p_down(struct wlan_net_vif *wnet_vif)
{
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct wifi_mac_p2p *p2p = &pwdev_priv->p2p;

    AML_OUTPUT("<%s>\n", VMAC_DEV_NAME(wnet_vif));
    vm_p2p_cancel_remain_channel(p2p);
    vm_p2p_initial(p2p);
    return;
}

#endif //CONFIG_P2P
