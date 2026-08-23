/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer send frame  module
 *
 *
 ****************************************************************************************
 */
#include "wifi_mac_com.h"
#include "wifi_mac_sae.h"
#include "wifi_mac_action.h"
#include <linux/inetdevice.h>
#include "wifi_cmd_func.h"

static int wifi_mac_classify(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct ether_header *eh = (struct ether_header *) os_skb_data(skb);
    int v_wme_ac=0, d_wme_ac=0, v_pri = 0;
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *)(skb->cb);
    struct vlan_ethhdr *veth = (struct vlan_ethhdr *)os_skb_data(skb);
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info*) os_skb_cb(skb);

    memset(txinfo, 0,sizeof(struct wifi_mac_tx_info));
    os_skb_set_priority(skb,WME_AC_BE);
    cb->u_tid = 0;

    if (!(sta->sta_flags & WIFINET_NODE_QOS))
        return 0;

    if (sta->sta_vlan != 0 && vlan_tx_tag_present(skb))
    {
        unsigned int tag=0;

        if (wnet_vif->vm_vlgrp == NULL)
        {
            WIFINET_NODE_STAT(sta, tx_novlantag);
            return 1;
        }

        if (((tag = vlan_tx_tag_get(skb)) & VLAN_VID_MASK) != (sta->sta_vlan & VLAN_VID_MASK))
        {
            WIFINET_NODE_STAT(sta, tx_vlanmismatch);
            return 1;
        }

        if (sta->sta_flags & WIFINET_NODE_QOS)
        {
            v_pri = (tag >> VLAN_PRI_SHIFT) & VLAN_PRI_MASK;
            v_wme_ac = TID_TO_WME_AC(v_pri);
        }
    }
    else
    {
        if (veth->h_vlan_proto == __constant_htons(ETH_P_8021Q))
        {
            v_pri = (veth->h_vlan_TCI >> VLAN_PRI_SHIFT) & VLAN_PRI_MASK;
            v_wme_ac = TID_TO_WME_AC(v_pri);
        }
    }

    if (eh->ether_type == __constant_htons(ETHERTYPE_IP))
    {
        const struct iphdr *ip = (struct iphdr *)(os_skb_data(skb) + sizeof (struct ether_header));

        cb->u_tid = (ip->tos & (~INET_ECN_MASK)) >> IP_PRI_SHIFT;
        d_wme_ac = TID_TO_WME_AC(cb->u_tid);
        os_skb_set_priority(skb,d_wme_ac);
    }

    if (v_wme_ac > d_wme_ac)
    {
        cb->u_tid = v_pri;
        pr_debug("<running> %s %d cb->u_tid = %d\n",__func__,__LINE__,cb->u_tid);
        os_skb_set_priority(skb,v_wme_ac);
    }

    if (wnet_vif->vm_opmode == WIFINET_M_STA)
    {
        struct wifi_mac *wifimac = sta->sta_wmac;

        while ((os_skb_get_priority(skb) != WME_AC_BK)
            && wifimac->wm_wme[wnet_vif->wnet_vif_id].wme_chanParams.cap_wmeParams[os_skb_get_priority(skb)].wmep_acm)
        {
            switch (os_skb_get_priority(skb))
            {
                case WME_AC_BE:
                    os_skb_set_priority(skb,WME_AC_BK);
                    break;
                case WME_AC_VI:
                    os_skb_set_priority(skb,WME_AC_BE);
                    break;
                case WME_AC_VO:
                    os_skb_set_priority(skb,WME_AC_VI);
                    break;
                default:
                    os_skb_set_priority(skb,WME_AC_BK);
                    break;
            }

            cb->u_tid = WME_AC_TO_TID(os_skb_get_priority(skb));
        }
    }

    txinfo->tid_index =  os_skb_get_tid(skb);
    //pr_debug("%s tid_index:%d\n", __func__, txinfo->tid_index);

    return 0;
}

static int is_rtsp_play(char *buf)
{
    return (buf + 3) && (*(buf + 3) | *(buf + 2) << 8 | *(buf + 1) << 16 | *buf << 24) == 0x504c4159;
}

static int is_rtsp_play_session(char *buf)
{
    return (buf + 3) && (*(buf + 3) | *(buf + 2) << 8 | *(buf + 1) << 16 | *buf << 24) == 0x53657373;
}


static void wifi_mac_get_rtsp_session(char* rtsp, struct wifi_mac_p2p* p2p)
{
    char *buf = rtsp;
    char id_buf[8];
    memset(p2p->wfd_session_id, 0, MAC_WFD_SESSION_LEN);
    if (is_rtsp_play(buf)) {
        while (buf) {
            if (is_rtsp_play_session(buf)) {
                break;
            }
            buf++;
        }
        buf += 9;
        while (*buf != 0x0d) {
            memset(id_buf, 0, 8);
            sprintf(id_buf, "%d", (*buf - 0x30));
            strcat(p2p->wfd_session_id, id_buf);
            buf++;
        }
        pr_debug("wfd_session_id=%s\n", p2p->wfd_session_id);
    }
}

void wifi_mac_xmit_pkt_parse(struct sk_buff *skb, struct wifi_mac *wifimac)
{
    struct ether_header *eh = (struct ether_header *)((unsigned char *)os_skb_data(skb));
    struct iphdr *iphdrp = (struct iphdr *)((unsigned char *)eh + sizeof(struct ether_header));
    struct tcphdr *th = (struct tcphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
    struct udphdr *uh = (struct udphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skb->cb;
    struct wifi_station *sta = cb->sta;
    struct wifi_mac_pkt_info *mac_pkt_info = &txinfo->ptxdesc->drv_pkt_info.pkt_info[0];

    unsigned char j = 0;
    char *rtsp = NULL;
    unsigned char *dhcp_p;
    unsigned short offset;
    unsigned short offset_max;

    static unsigned char start_flag = 0;
    static unsigned long in_time;
    static unsigned long tcp_tx_payload_total = 0;
    unsigned long tcp_tx_payload = 0;
    unsigned char tid_index = 0;

    if ((eh->ether_type == __constant_htons(ETHERTYPE_PAE))
        ||(eh->ether_type == __constant_htons(ETHERTYPE_WPI))) {
        unsigned char *p_eap = (unsigned char *)(eh+1);

        mac_pkt_info->b_eap = 1;
        if (p_eap[1] == 3 && p_eap[4] == 2) {
            AML_OUTPUT("send eapol frame,len:%d, key_info:%04x\n", (p_eap[2]<<8)|p_eap[3], (p_eap[5]<<8)|p_eap[6]);
        }

    } else if (eh->ether_type == __constant_htons(ETHERTYPE_IP)) {
        if (iphdrp->protocol == IPPROTO_TCP) {
            AML_PRINT(AML_DBG_MODULES_TX, "tcp src:%d, dst:%d, seq:%u, tcp ack:%u\n",  __constant_htons(th->source),  __constant_htons(th->dest),
                __constant_htonl(th->seq), __constant_htonl(th->ack_seq));

            rtsp = (char *)th + 32;
            if (rtsp && wifimac->is_miracast_connect) {
                wifi_mac_get_rtsp_session(rtsp, sta->sta_wnet_vif->vm_p2p);
            }

            if ((!th->fin) && (!th->syn)) {
                mac_pkt_info->b_tcp = 1;
                mac_pkt_info->b_tcp_push = 1;
            }

            tcp_tx_payload = (unsigned long)(skb->data + skb->len) - (unsigned long)((unsigned char *)th + (th->doff << 2));
            if ((th->ack) && (!th->psh) && (!th->fin) && (!th->syn)) {
                if (!tcp_tx_payload) {
                    mac_pkt_info->b_tcp_ack = 1;
                    if (th->doff == 5)
                        mac_pkt_info->b_tcp_ack_del = 1;
                }
            }

            if (!start_flag) {
                start_flag = 1;
                in_time = jiffies;
            }

            tcp_tx_payload_total += tcp_tx_payload;

            if (time_after(jiffies, in_time + HZ)) {
                sta->sta_wnet_vif->vm_tx_speed = tcp_tx_payload_total >> 17;
                start_flag = 0;
                tcp_tx_payload_total = 0;
            }

            wifimac->drv_priv->drv_config.cfg_ampdu_subframes = DEFAULT_TXAMPDU_SUB_MAX;

            mac_pkt_info->tcp_seqnum = th->seq;
            mac_pkt_info->tcp_ack_seqnum = th->ack_seq;
            mac_pkt_info->tcp_src_port = th->source;
            mac_pkt_info->tcp_dst_port = th->dest;

        } else if (iphdrp->protocol == IPPROTO_UDP) {
            if (((uh->source == 0x4400) && (uh->dest == 0x4300))
                || ((uh->source == 0x4300) && (uh->dest == 0x4400))) {
                if (sta->connect_status == CONNECT_DHCP_GET_ACK) {
                    return;
                }

                dhcp_p = (unsigned char *)((unsigned char *)uh + 8);
                offset = 240;
                offset_max = os_skb_get_pktlen(skb);

                while ((dhcp_p[offset] != 255) && (offset < offset_max)) {
                    if (dhcp_p[offset] == 53) {
                        AML_OUTPUT("dhcp send status : %d\n", dhcp_p[offset + 2]);
                        break;
                    }

                    offset += dhcp_p[offset + 1] + 2;
                }

                mac_pkt_info->b_dhcp = 1;

            } else {
                for (j = 0; j < udp_cnt; j++) {
                    if ((uh->source == __constant_htons(aml_udp_info[j].src_port)) && (uh->dest == __constant_htons(aml_udp_info[j].dst_port)) && (aml_udp_info[j].out)) {
                        if (aml_udp_info[j].dst_ip == __constant_htonl(iphdrp->daddr)) {
                            aml_udp_info[j].tx += 1;
                        }
                    }
                }
            }
        }

    } else if (eh->ether_type == __constant_htons(ETHERTYPE_ARP)) {
        mac_pkt_info->b_arp = 1;
    }

    if (mac_pkt_info->b_arp || mac_pkt_info->b_dhcp || mac_pkt_info->b_eap) {
        for (tid_index = 1; tid_index < WME_NUM_TID; tid_index++) {
            if (!wifimac->drv_priv->drv_ops.aggr_tid_query(wifimac->drv_priv, sta->drv_sta, tid_index)) {
                cb->u_tid = tid_index;
                break;
            }
        }
        if (tid_index >= WME_NUM_TID) {
            AML_OUTPUT("all tid with ba session, set arp dhcp eap tid 2\n");
            cb->u_tid = 2;
        }
    }
}

int wifi_mac_is_allow_send(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, struct sk_buff *skb)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    unsigned short qlen_real;
    unsigned short pending_cnt;
    unsigned char tid_index = os_skb_get_tid(skb);
    struct aml_driver_nsta *drv_sta = sta->drv_sta;
    struct drv_tx_scoreboard *tid = DRV_GET_TIDTXINFO(drv_sta, tid_index);

    if (tid->tid_tx_buff_sending) {
        wnet_vif->vm_devstats.tx_dropped++;
        wnet_vif->vif_sts.sts_tx_tid_drop_bf_in_msdu[os_skb_get_tid(skb)]++;
        return NETDEV_TX_BUSY;
    }

    if (wnet_vif->vm_phase_flags & PHASE_TX_BUFF_QUEUE) {
        wnet_vif->vm_devstats.tx_dropped++;
        return NETDEV_TX_BUSY;
    }
    /*check the txlist_x threshhold: been sent to hal and to be sent to hal*/
    /* check msdu pending num*/
    if (wifimac->drv_priv->drv_ops.txlist_isfull(wifimac->drv_priv, wifimac->wm_ac2q[os_skb_get_priority(skb)], (skb), sta->drv_sta)
        || (wifimac->msdu_cnt[os_skb_get_tid(skb)] > MAX_MSDU_CNT)) {
        wnet_vif->vm_devstats.tx_dropped++;
        wnet_vif->vif_sts.sts_tx_tid_drop_bf_in_msdu[os_skb_get_tid(skb)]++;
        return NETDEV_TX_BUSY;
    }

    if (wifimac->wm_flags & WIFINET_F_SCAN) {
        qlen_real = WIFINET_SAVEQ_QLEN(&wnet_vif->vm_tx_buffer_queue);
        pending_cnt = wifimac->drv_priv->drv_ops.drv_tx_pending_pkt(wifimac->drv_priv);

        if (qlen_real + pending_cnt > 200) {
            //pr_debug("qlen_real:%d, pending_cnt:%d\n", qlen_real, pending_cnt);
            wnet_vif->vm_devstats.tx_dropped++;
            wnet_vif->vif_sts.sts_tx_tid_drop_bf_in_msdu[os_skb_get_tid(skb)]++;
            return NETDEV_TX_BUSY;
        }
    }

    return 0;
}

static unsigned short wifi_mac_checksum_calc(unsigned short len_udp, unsigned char * src_addr,
    unsigned char *dest_addr, unsigned char type, unsigned char *buff)
{
    unsigned short padd = 0;
    unsigned short word16;
    unsigned int sum;
    int i = 0;
    if (len_udp % 2 != 0 )
    {
        padd = 1;
        buff[len_udp] = 0;
    }

    sum = 0;
    for (i = 0; i < len_udp + padd; i = i + 2)
    {
        word16 = ((buff[i] << 8) &0xFF00) + (buff[i + 1] & 0xFF);
        sum = sum + (unsigned long)word16;
    }

    for (i = 0; i < 4; i = i + 2)
    {
        word16 = ((src_addr[i] << 8) & 0xFF00) + (src_addr[i + 1] & 0xFF);
        sum = sum + word16;
    }
    for (i = 0; i < 4; i = i + 2)
    {
        word16 = ((dest_addr[i] << 8) & 0xFF00) + (dest_addr[i + 1] & 0xFF);
        sum = sum + word16;
    }
    sum = sum + type + len_udp;

    while (sum>>16)
        sum = (sum & 0xFFFF)+(sum >> 16);

    sum = ~sum;

    return ((unsigned short) sum);
}


int wifi_mac_udp_csum(struct sk_buff *skb)
{
    struct iphdr *iphdrp = (struct iphdr *)((unsigned char *)os_skb_data(skb)+sizeof(struct ether_header));
    struct udphdr *uh = (struct udphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));

    uh->check = 0;
    uh->check = wifi_mac_checksum_calc(__constant_htons(uh->len),(unsigned char *) &iphdrp->saddr,(unsigned char *)&iphdrp->daddr, 17, (unsigned char *)uh);
    uh->check = __constant_htons(uh->check);
    return 0;
}

static int wifi_mac_tcp_csum(struct sk_buff *skb)
{
    struct iphdr *iphdrp = (struct iphdr *)((unsigned char *)os_skb_data(skb)+sizeof(struct ether_header));
    struct tcphdr *th = (struct tcphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
    unsigned short data_len = __constant_htons(iphdrp->tot_len) - iphdrp->ihl * 4;

    th->check = 0;
    th->check = wifi_mac_checksum_calc(data_len, (unsigned char *) &iphdrp->saddr, (unsigned char *)&iphdrp->daddr, 6, (unsigned char *)th);
    th->check = __constant_htons(th->check);
    return 0;
}

netdev_tx_t wifi_mac_hardstart(struct sk_buff *skb, struct net_device *dev)
{
    struct wlan_net_vif *wnet_vif = netdev_priv(dev);
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_station *sta = NULL;
    struct ether_header *eh = NULL;
    struct iphdr *iphdrp = NULL;
    unsigned char hw_calculate_flag = 0;
    struct wifi_mac_tx_info *txinfo;
    struct drv_txdesc *ptxdesc = NULL;
    unsigned char error = 0;

    if (aml_wifi_is_enable_rf_test())
        goto bad;

    if (skb != NULL) {
        eh = (struct ether_header *)os_skb_data(skb);
        os_skb_count_alloc(skb);
    } else {
        error = 1;
        goto bad;
    }

    txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);

    if (NET80211_IF_RUN_UP(wifimac) == 0) {
        error = 2;
        goto bad;
    }

    if (wnet_vif->vm_state != WIFINET_S_CONNECTED || (wnet_vif->vm_phase_flags & PHASE_DISCONNECTING)) {
        error = 3;
        goto bad;
    }

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wnet_vif->vm_flags_ext & WIFINET_C_STA_FORWARD)) {
        if (WIFINET_ADDR_EQ(eh->ether_shost, dev->dev_addr)) {
            if (eh->ether_type ==__constant_htons( ETHERTYPE_PAE)) {
                WIFINET_ADDR_COPY(eh->ether_shost,wnet_vif->vm_myaddr);

            } else {
                error = 4;
                goto bad;
            }

        } else {
            error = 5;
            goto bad;
        }
    }

    // find a sta by mac address
    sta = wifi_mac_find_tx_sta(wnet_vif, eh->ether_dhost);
    if ((sta == NULL) || ((wnet_vif->vm_opmode == WIFINET_M_HOSTAP) && (sta->is_disconnecting == 1))) {
        error = 6;
        goto bad;
    }

    if ((wnet_vif->vm_opmode != WIFINET_M_IBSS) && (sta->sta_associd == 0) && (sta != wnet_vif->vm_mainsta)) {
        error = 7;
        goto bad;
    }

    if (wifi_mac_classify(sta, skb)) {
        error = 8;
        goto bad;
    }

    if (sta->sta_channel_switch_mode == 1) {
        error = 9;
        goto bad;
    }

    if (wifi_mac_is_allow_send(wnet_vif, sta, skb)) {
        error = 10;
        goto bad;
    }

    ptxdesc = wifi_mac_alloc_txdesc(wifimac);
    if (ptxdesc == NULL) {
        error = 11;
        goto bad;
    }

    wnet_vif->vif_sts.sts_tx_total_msdu++;
    wnet_vif->vif_sts.sts_tx_tid_in_msdu[os_skb_get_tid(skb)]++;

    wnet_vif->vm_devstats.tx_packets++;
    wnet_vif->vm_devstats.tx_bytes += os_skb_get_pktlen(skb);

    if (skb->ip_summed == CHECKSUM_PARTIAL) {
        if (wifimac->wm_caps & WIFINET_C_HWCSUM) {
            iphdrp = (struct iphdr *)((unsigned char *)os_skb_data(skb) + sizeof(struct ether_header));

            if (eh->ether_type == __constant_htons(ETHERTYPE_IP)) {
                M_FLAG_SET(skb, M_CHECKSUMHW);

                if (iphdrp->protocol == IPPROTO_TCP) {
                    wnet_vif->vif_sts.sts_tx_tcp_msdu++;

                } else if (iphdrp->protocol == IPPROTO_UDP) {
                    wnet_vif->vif_sts.sts_tx_udp_msdu++;
                }
                hw_calculate_flag = 1;

            } else if(eh->ether_type == __constant_htons(ETH_P_ARP)) {
                wnet_vif->vif_sts.sts_tx_arp_msdu++;

            } else if (eh->ether_type == __constant_htons(ETHERTYPE_IPV6)) {
                //TO DO
            }
        }

        if (!hw_calculate_flag) {
            //call kernel calculate
            skb_checksum_help(skb);
        }
    }

    AML_PRINT(AML_DBG_MODULES_TX, "vid:%d, eh->ether_type:%04x\n", wnet_vif->wnet_vif_id, eh->ether_type);
    txinfo->cb.sta = sta;
    txinfo->ptxdesc = ptxdesc;
    txinfo->ptxdesc->drv_pkt_info.pkt_info_count = 1;
    wifi_mac_xmit_pkt_parse(skb, wifimac);

    wnet_vif->vif_sts.sts_tx_out_msdu++;
    wnet_vif->vif_sts.sts_tx_tid_out_msdu[os_skb_get_tid(skb)]++;

    M_FLAG_KEEP_ONLY(skb, M_CHECKSUMHW);
    wifi_mac_pwrsave_txpre(skb);
    skb->dev = wnet_vif->vm_ndev;

    return wifi_mac_tx_send(skb);
bad:


    AML_PRINT(AML_DBG_MODULES_TX_ERROR, "vid:%d, error:%d\n", wnet_vif->wnet_vif_id, error);
    if (error > 9) {
        return NETDEV_TX_BUSY;

    } else {
        if (skb != NULL) {
            wnet_vif->vif_sts.sts_tx_drop_msdu++;
            wnet_vif->vm_devstats.tx_dropped++;
            wnet_vif->vif_sts.sts_tx_tid_drop_msdu[os_skb_get_tid(skb)]++;
            os_skb_free(skb);
        }
        return 0;
    }
}

void wifi_mac_send_setup(struct wlan_net_vif *wnet_vif,
    struct wifi_station *sta, struct wifi_frame *wh, int type, const unsigned char sa[WIFINET_ADDR_LEN],
    const unsigned char da[WIFINET_ADDR_LEN], const unsigned char bssid[WIFINET_ADDR_LEN])
{
    wh->i_fc[0] = WIFINET_FC0_VERSION_0 | type;
    if ((type & WIFINET_FC0_TYPE_MASK) == WIFINET_FC0_TYPE_DATA)
    {
        switch (wnet_vif->vm_opmode)
        {
            case WIFINET_M_STA:
                wh->i_fc[1] = WIFINET_FC1_DIR_TODS;
                WIFINET_ADDR_COPY(wh->i_addr1, bssid);
                WIFINET_ADDR_COPY(wh->i_addr2, sa);
                WIFINET_ADDR_COPY(wh->i_addr3, da);
                break;
            case WIFINET_M_IBSS:
                wh->i_fc[1] = WIFINET_FC1_DIR_NODS;
                WIFINET_ADDR_COPY(wh->i_addr1, da);
                WIFINET_ADDR_COPY(wh->i_addr2, sa);
                WIFINET_ADDR_COPY(wh->i_addr3, bssid);
                break;
            case WIFINET_M_HOSTAP:
                wh->i_fc[1] = WIFINET_FC1_DIR_FROMDS;
                WIFINET_ADDR_COPY(wh->i_addr1, da);
                WIFINET_ADDR_COPY(wh->i_addr2, bssid);
                WIFINET_ADDR_COPY(wh->i_addr3, sa);
                break;
            case WIFINET_M_WDS:
                wh->i_fc[1] = WIFINET_FC1_DIR_DSTODS;
                WIFINET_ADDR_COPY(wh->i_addr1, bssid);
                WIFINET_ADDR_COPY(wh->i_addr2, wnet_vif->vm_myaddr);
                WIFINET_ADDR_COPY(wh->i_addr3, da);
                WIFINET_ADDR_COPY(WH4(wh)->i_addr4, sa);
                break;
            case WIFINET_M_MONITOR:
                break;
            case WIFINET_M_P2P_CLIENT:
                break;
            case WIFINET_M_P2P_DEV:
                break;
            case WIFINET_M_P2P_GO:
                break;
        }
    }
    else
    {
        wh->i_fc[1] = WIFINET_FC1_DIR_NODS;
        WIFINET_ADDR_COPY(wh->i_addr1, da);
        WIFINET_ADDR_COPY(wh->i_addr2, sa);
        WIFINET_ADDR_COPY(wh->i_addr3, bssid);
    }
    *(unsigned short *)&wh->i_dur[0] = 0;
    *(unsigned short *)wh->i_seq = 0;
}

static void
wifi_mac_mgmt_csa_action_broadcast_output(struct wifi_station *sta, struct sk_buff *skb, int type)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wifi_frame *wh;
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skb->cb;

    KASSERT(sta != NULL, ("null nsta"));

    cb->sta = sta;

    wh = (struct wifi_frame *)os_skb_push(skb, sizeof(struct wifi_frame));
    wifi_mac_send_setup(wnet_vif, sta, wh, WIFINET_FC0_TYPE_MGT | type,
        wnet_vif->vm_myaddr, BROADCAST_ADDRESS, sta->sta_bssid);

    if ((cb->flags & M_LINK0) != 0 && sta->sta_challenge != NULL)
    {
        cb->flags &= ~M_LINK0;
        DPRINTF(AML_DEBUG_CONNECT, "recv addr is [%02x:%02x:%02x:%02x:%02x:%02x]\n",
            wh->i_addr1[0], wh->i_addr1[1], wh->i_addr1[2],
            wh->i_addr1[3], wh->i_addr1[4], wh->i_addr1[5]);
        wh->i_fc[1] |= WIFINET_FC1_WEP;
    }

    if (WIFINET_VMAC_IS_SLEEPING(sta->sta_wnet_vif))
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;

    WIFINET_NODE_STAT(sta, tx_mgmt);
    wifi_mac_tx_mgmt_frm(wifimac, skb);
}

static void
wifi_mac_mgmt_output(struct wifi_station *sta, struct sk_buff *skb, int type)
{
    struct wlan_net_vif *wnet_vif;
    struct wifi_mac *wifimac;
    struct wifi_frame *wh;
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skb->cb;

    KASSERT(sta != NULL, ("null nsta"));

    wnet_vif = sta->sta_wnet_vif;
    wifimac = sta->sta_wmac;
    cb->sta = sta;

    wh = (struct wifi_frame *)os_skb_push(skb, sizeof(struct wifi_frame));
    wifi_mac_send_setup(wnet_vif, sta, wh, WIFINET_FC0_TYPE_MGT | type,
        wnet_vif->vm_myaddr, sta->sta_macaddr, sta->sta_bssid);

    if ((cb->flags & M_LINK0) != 0 && sta->sta_challenge != NULL)
    {
        cb->flags &= ~M_LINK0;
        WIFINET_DPRINTF_MACADDR(AML_DEBUG_CONNECT, wh->i_addr1, "encrypting frame (%s)", __func__);
        wh->i_fc[1] |= WIFINET_FC1_WEP;
    }

    if (WIFINET_VMAC_IS_SLEEPING(sta->sta_wnet_vif))
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;

    WIFINET_NODE_STAT(sta, tx_mgmt);
    wifi_mac_tx_mgmt_frm(wifimac, skb);
}

static void
wifi_mac_mgmt_probe_resp_output(struct wifi_station *sta,
                                struct sk_buff *skb,
                                unsigned char *macaddr)
{
    struct wlan_net_vif *wnet_vif;
    struct wifi_mac *wifimac;
    struct wifi_frame *wh;
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skb->cb;
    unsigned char * bssid = NULL;
    unsigned char * sa = NULL;

    KASSERT(sta != NULL, ("null nsta"));

    wnet_vif = sta->sta_wnet_vif;
    wifimac = sta->sta_wmac;
    cb->sta = sta;

    wh = (struct wifi_frame *)os_skb_push(skb, sizeof(struct wifi_frame));
#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p_support
        && ((wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_DEVICE) || (wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_CLIENT)))
    {
        bssid = wnet_vif->vm_p2p->dev_addr;
        sa = wnet_vif->vm_p2p->dev_addr;
    }
    else
#endif
    {
        //CONFIG_P2P
        bssid = sta->sta_bssid;
        sa = wnet_vif->vm_myaddr;
    }
    WIFINET_DPRINTF(AML_DEBUG_XMIT,"p2p send proberesp da=%s\n ", ether_sprintf(macaddr));
    WIFINET_DPRINTF(AML_DEBUG_XMIT,"p2p send proberesp bssid=%s\n ", ether_sprintf(bssid));
    wifi_mac_send_setup(wnet_vif, sta, wh, WIFINET_FC0_TYPE_MGT | WIFINET_FC0_SUBTYPE_PROBE_RESP, sa,macaddr, bssid);

    if ((cb->flags & M_LINK0) != 0 && sta->sta_challenge != NULL)
    {
        cb->flags &= ~M_LINK0;
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_CONNECT, wh->i_addr1, "encrypting frame (%s)", __func__);
        wh->i_fc[1] |= WIFINET_FC1_WEP;
    }

    if (WIFINET_VMAC_IS_SLEEPING(sta->sta_wnet_vif))
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;

    WIFINET_NODE_STAT(sta, tx_mgmt);

    wifi_mac_tx_mgmt_frm(wifimac, skb);
}

int wifi_mac_send_nulldata_for_ap(struct wifi_station *sta, unsigned char pwr_save,
        unsigned char pwr_flag, unsigned char qos, int ac)
{
    struct sk_buff *skb;
    unsigned char pkt_len = 0;
    unsigned char subtype = 0;
    int error = 0;
    struct wlan_net_vif *wnet_vif;
    struct wifi_mac *wifimac;
    struct wifi_mac_tx_info *txinfo = NULL;
    struct wifi_frame *wh;
    struct wifi_skb_callback *cb = NULL;

    KASSERT(sta != NULL, ("null nsta"));

    wnet_vif = sta->sta_wnet_vif;
    wifimac = sta->sta_wmac;

    if (qos) {
        pkt_len = sizeof(struct wifi_qos_frame) + FCS_LEN;
        subtype = WIFINET_FC0_SUBTYPE_QOS_NULL;
    } else {
        pkt_len = sizeof(struct wifi_frame) + FCS_LEN;;
        subtype = WIFINET_FC0_SUBTYPE_NODATA;
    }

    skb = wifi_mac_get_mgmt_frm(wnet_vif->vm_wmac, pkt_len);
    if (skb == NULL) {
        WIFINET_DPRINTF_STA(AML_DEBUG_ERROR, sta, "%s: Unable to allocate frame", __func__);
        return ENOMEM;
    }

    cb = (struct wifi_skb_callback *)skb->cb;
    cb->sta = sta;

    if (qos) {
        wh = (struct wifi_frame *)os_skb_push(skb, sizeof(struct wifi_qos_frame));
    } else {
        wh = (struct wifi_frame *)os_skb_push(skb, sizeof(struct wifi_frame));
    }

    wifi_mac_send_setup(wnet_vif, sta, wh, WIFINET_FC0_TYPE_DATA | subtype,
        wnet_vif->vm_myaddr, sta->sta_macaddr, sta->sta_bssid);

    os_skb_set_priority(skb,ac);

    txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);

    ASSERT(sizeof(struct wifi_mac_tx_info) <= OS_SKB_CB_MAXLEN);
    memset(&txinfo->wnet_vif_id, 0, sizeof(struct wifi_mac_tx_info)-sizeof(struct wifi_skb_callback)-sizeof(struct drv_txdesc *));
    wifi_mac_pwrsave_txpre(skb);
#if 0
    txinfo->b_pwr_flag = pwr_flag;
#endif
    error = wifi_mac_build_txinfo(wifimac, skb, txinfo);
    if (!error) {
        error = wifimac->drv_priv->drv_ops.tx_start(wifimac->drv_priv, skb);
        return error;
    }
    return error;
}

int wifi_mac_send_nulldata(struct wifi_station *sta, unsigned char pwr_save,
        unsigned char pwr_flag, unsigned char qos, int ac)
{
    struct wlan_net_vif *wnet_vif;
    struct wifi_mac *wifimac;
    int ret = -1, len = 0;
    struct NullDataCmd null_data;

    if (sta == NULL) {
        return -1;
    }

    wnet_vif = sta->sta_wnet_vif;
    wifimac = sta->sta_wmac;

    memset(&null_data, 0, sizeof(struct NullDataCmd));

    null_data.vid = wnet_vif->wnet_vif_id;
    null_data.pwr_save= pwr_save;
    null_data.pwr_flag = pwr_flag;
    null_data.staid = (wnet_vif->vm_opmode == WIFINET_M_STA) ? 1 : sta->sta_associd;
    if (wifimac->drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH20)
    {
        null_data.bw = CHAN_BW_20M;
    }
    else if(wifimac->drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH40)
    {
        null_data.bw = (sta->sta_chbw == WIFINET_BWC_WIDTH20) ? CHAN_BW_20M : CHAN_BW_40M;
    }
    else if(wifimac->drv_priv->drv_curchannel.chan_bw == WIFINET_BWC_WIDTH80)
    {
        null_data.bw = (sta->sta_chbw == WIFINET_BWC_WIDTH20) ? CHAN_BW_20M :
            ((sta->sta_chbw == WIFINET_BWC_WIDTH40) ? CHAN_BW_40M : CHAN_BW_80M);
    }

    if((sta->sta_flags & WIFINET_NODE_HT) || (sta->sta_flags & WIFINET_NODE_VHT))
    {
        null_data.rate = WIFI_11G_6M;
    }
    else
    {
        null_data.rate = WIFI_11B_1M;
        null_data.bw = CHAN_BW_20M;
    }

    if (qos == 0)
    {
        null_data.qos = 0;
        null_data.tid = 0;
        len = sizeof(struct wifi_frame) + FCS_LEN;
    }
    else
    {
        null_data.qos = qos;
        null_data.tid = WME_AC_TO_TID(ac);
        len = sizeof(struct wifi_qos_frame) + FCS_LEN;
        /*AP check it . */
        if (WME_UAPSD_NODE_AC_CAN_TRIGGER(ac, sta))
        {
            null_data.eosp = 1;
        }
    }
    memcpy(&null_data.dest_addr, sta->sta_macaddr, WIFINET_ADDR_LEN);
    null_data.sn = (sta->sta_txseqs[0] & 0xfff);
    sta->sta_txseqs[0]++;
    if ((wifi_mac_pwrsave_is_wnet_vif_sleeping(wnet_vif) == 0)
        && (pwr_save == 0))
    {
         //if vmac is in sleep, but ps=0 in frame, so need to wakeup first
        wifi_mac_pwrsave_wakeup_for_tx(wnet_vif);
    }
    ret = wifimac->drv_priv->drv_ops.drv_send_null_data(wifimac->drv_priv, null_data, len);
    return 0;
}

int wifi_mac_send_qosnulldata(struct wifi_station *sta, int ac)
{
    unsigned char ps = 0;

    if (wifi_mac_pwrsave_is_sta_sleeping(sta->sta_wnet_vif) == 0)
    {
        ps = 1;
    }
    wifi_mac_send_nulldata(sta, ps/*ps*/, 0/*ps packet flag */, 1/*qos*/, ac);
    return 0;
}

int wifi_mac_send_testdata(struct wifi_station *sta, int length)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct sk_buff *skb;
    struct wifi_skb_callback *cb;
    struct wifi_qos_frame *qwh;

#if 1 // debug for sdio modified data
    int len = length & 0xffff;
    int senddata = 0xaa;
#endif

    skb = wifi_mac_alloc_skb(wifimac, sizeof(struct wifi_qos_frame)+len+sizeof(struct hi_tx_desc)); //2016.12.21 fix
    if (skb == NULL)
    {
        DPRINTF(AML_DEBUG_WARNING, "WARNING:: %s %d, alloc skb fail \n",__func__,__LINE__);
        wnet_vif->vif_sts.sts_tx_no_buf++;
        return -ENOMEM;
    }

    memset(skb->data + sizeof(struct wifi_qos_frame), senddata, len);
    cb = (struct wifi_skb_callback *)skb->cb;
    cb->sta = sta;
    cb->flags = 0;
    cb->hdrsize = sizeof (struct wifi_frame);

    os_skb_set_priority(skb,WME_AC_BE);
    qwh = (struct wifi_qos_frame *) os_skb_put(skb, sizeof(struct wifi_qos_frame)+len);
#if 1
    wifi_mac_send_setup(wnet_vif, sta, (struct wifi_frame *)qwh,
                       WIFINET_FC0_TYPE_MGT|WIFINET_FC0_SUBTYPE_REASSOC_RESP,
                       wnet_vif->vm_myaddr,
                       BROADCAST_ADDRESS,
                       sta->sta_bssid);
#else
    {
        unsigned char dstmac[WIFINET_ADDR_LEN] =  {0x00,0x11,0x11,0x11,0x11,0x11};
        unsigned char bssidmac[WIFINET_ADDR_LEN] =  {0x00,0x22,0x22,0x22,0x22,0x22};
        wifi_mac_send_setup(wnet_vif, sta, (struct wifi_frame *)qwh,
                           WIFINET_FC0_TYPE_DATA|WIFINET_FC0_SUBTYPE_QOS,
                           wnet_vif->vm_myaddr, /* SA */
                           dstmac, /* DA */
                           bssidmac);
    }
#endif
    WIFINET_NODE_STAT(sta, tx_data);

    wifi_mac_tx_mgmt_frm(wifimac, skb);
    return 0;
}

struct sk_buff*
wifi_mac_skbhdr_adjust(struct wlan_net_vif *wnet_vif, int hdrsize,
    struct wifi_mac_key *key, struct sk_buff *skb, int ismulticast)
{
    int is_amsdu = M_FLAG_GET(skb, M_AMSDU);
    int need_headroom = (is_amsdu ? 0 : LLC_SNAPFRAMELEN) + hdrsize + WIFINET_ADDR_LEN;
    int need_tailroom = 0;
    struct sk_buff *pskb = skb;

    if (key != NULL)
    {
        const struct wifi_mac_security *cip = key->wk_cipher;

        if (cip->wm_cipher == WIFINET_CIPHER_TKIP)
            need_tailroom += cip->wm_miclen;
    }

    if ((os_skb_get_tailroom(pskb) < need_tailroom) || (os_skb_hdrspace(pskb) < need_headroom))
    {
        pskb = os_skb_copy_expand(skb, need_headroom+32, need_tailroom+32, GFP_ATOMIC, pskb);
        if (pskb == NULL)
        {
            ERROR_DEBUG_OUT("alloc skb fail.\n");
        }
        os_skb_free(skb);
    }
    return pskb;
}


static __inline struct wifi_mac_key *
wifi_mac_security_getucastkey(struct wlan_net_vif *wnet_vif, struct wifi_station *sta)
{
    if (KEY_UNDEFINED(sta->sta_ucastkey))
    {
        if (wnet_vif->vm_def_txkey == WIFINET_KEYIX_NONE ||
            KEY_UNDEFINED(wnet_vif->vm_nw_keys[wnet_vif->vm_def_txkey]))
            return NULL;
        return &wnet_vif->vm_nw_keys[wnet_vif->vm_def_txkey];
    }
    else
    {
        return &sta->sta_ucastkey;
    }
}

static __inline struct wifi_mac_key *
wifi_mac_security_getmcastkey(struct wlan_net_vif *wnet_vif, struct wifi_station *sta)
{
    if (wnet_vif->vm_def_txkey == WIFINET_KEYIX_NONE ||
        KEY_UNDEFINED(wnet_vif->vm_nw_keys[wnet_vif->vm_def_txkey]))
        return NULL;
    return &wnet_vif->vm_nw_keys[wnet_vif->vm_def_txkey];
}

static unsigned short calculate_checksum(unsigned short *buffer, int len)
{
    unsigned int cksum = 0;
    unsigned short *p = buffer;
    int size = (len >> 1) + (len & 0x1);

    while (size-- > 0) {
        cksum += *p++;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (unsigned short) (~cksum);
}

static void calculate_checksum_for_fragment_pkt(struct sk_buff *skb, struct ether_header eh, char hdrsize) {
    unsigned short is_hw_calculate = M_FLAG_GET(skb, M_CHECKSUMHW);
    unsigned char ip_pkt_offset;
    unsigned short *ip_hdr;
    unsigned short *tcp_hdr;
    unsigned short *udp_hdr;

    ip_pkt_offset = hdrsize + 8;

    // process tcp hardware checksum case
    if (is_hw_calculate) {
        if (eh.ether_type == __constant_htons(ETHERTYPE_IP)) {
            struct iphdr *iphdrp = (struct iphdr *)((unsigned char *)os_skb_data(skb) + ip_pkt_offset);
            struct tcphdr *th = (struct tcphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
            struct udphdr *uh = (struct udphdr *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));

            //pr_debug("chris ip_pkt_offset:%d, hdrsize:%d, iphdrp->ihl:%d\n", ip_pkt_offset, hdrsize, iphdrp->ihl);
            iphdrp->check = 0;
            ip_hdr = (unsigned short *)((unsigned char *)os_skb_data(skb) + ip_pkt_offset);
            iphdrp->check = calculate_checksum(ip_hdr, iphdrp->ihl * 4);

            if (iphdrp->protocol== IPPROTO_TCP) {
                th->check = 0;
                tcp_hdr = (unsigned short *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
                th->check = calculate_checksum(tcp_hdr, iphdrp->tot_len - iphdrp->ihl * 4);

            } else if(iphdrp->protocol== IPPROTO_UDP) {
                uh->check = 0;
                udp_hdr = (unsigned short *)((unsigned char *)iphdrp + (iphdrp->ihl << 2));
                uh->check = calculate_checksum(udp_hdr, uh->len);
            }

            M_FLAG_CLR(skb,M_CHECKSUMHW);
        } else if (eh.ether_type ==__constant_htons(ETHERTYPE_IPV6)) {
            //TODO
        }
    }
}

struct sk_buff *wifi_mac_encap(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct ether_header eh;
    struct wifi_frame *wh, *twh;
    struct wifi_mac_key *key;
    struct llc *llc;
    int hdrsize, datalen, addqos;
    int hdrsize_nopad;
    struct sk_buff *framelist = NULL;
    struct sk_buff *tskb;
    int fragcnt = 1;
    int pdusize = 0;
    int ismulticast=0;
    int use4addr=0;
    int addhtc=0;
    struct wifi_skb_callback *cb = (struct wifi_skb_callback *)(skb->cb);
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);
    int is_amsdu = M_FLAG_GET(skb, M_AMSDU);
    struct wifi_mac_pkt_info *mac_pkt_info = &txinfo->ptxdesc->drv_pkt_info.pkt_info[0];

    memcpy(&eh, os_skb_data(skb), sizeof(struct ether_header));
    os_skb_pull(skb, sizeof(struct ether_header));  //stripped ethernet header

    if (wnet_vif->vm_flags & WIFINET_F_PRIVACY)
    {
        if (wnet_vif->vm_opmode == WIFINET_M_STA || !WIFINET_IS_MULTICAST(eh.ether_dhost))
            key = wifi_mac_security_getucastkey(wnet_vif, sta);
        else
            key = wifi_mac_security_getmcastkey(wnet_vif, sta);

        if (key == NULL && !mac_pkt_info->b_eap)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_KEY, eh.ether_dhost, "no default transmit key (%s) deftxkey %u",
                __func__, wnet_vif->vm_def_txkey);
            wnet_vif->vif_sts.sts_tx_key_err++;
        }
    }
    else
    {
        key = NULL;
    }

    if (sta->sta_flags & WIFINET_NODE_QOS)
    {
        hdrsize = sizeof(struct wifi_qos_frame);
        addqos = 1;
    }
    else
    {
        hdrsize = sizeof(struct wifi_frame);
        addqos = 0;
    }

    switch (wnet_vif->vm_opmode)
    {
        case WIFINET_M_IBSS:
            ismulticast = WIFINET_IS_MULTICAST(eh.ether_dhost);
            break;
        case WIFINET_M_WDS:
            hdrsize += WIFINET_ADDR_LEN;
            use4addr=1;
            ismulticast = WIFINET_IS_MULTICAST(sta->sta_macaddr);
            break;
        case WIFINET_M_HOSTAP:
            if (!WIFINET_ADDR_EQ(eh.ether_dhost, sta->sta_macaddr)
                && (!WIFINET_IS_MULTICAST(eh.ether_dhost)))
            {
                hdrsize += WIFINET_ADDR_LEN;
                use4addr=1;
                ismulticast = WIFINET_IS_MULTICAST(sta->sta_macaddr);
            }
            else
            {
                ismulticast = WIFINET_IS_MULTICAST(eh.ether_dhost);
            }
            break;
        case WIFINET_M_STA:
            if ((!WIFINET_ADDR_EQ(eh.ether_shost, wnet_vif->vm_myaddr))
                &&(wnet_vif->vm_flags_ext & WIFINET_FEXT_WDS))
            {
                hdrsize += WIFINET_ADDR_LEN;
                use4addr=1;
                ismulticast = WIFINET_IS_MULTICAST(sta->sta_macaddr);
                if (WIFINET_IS_MULTICAST(eh.ether_dhost))
                {
                    struct wifi_station_tbl *nt;
                    struct wifi_station *sta_wds=NULL;
                    nt = &wnet_vif->vm_sta_tbl;
                    sta_wds = wifi_mac_find_wds_sta(nt,eh.ether_shost);
                    if (!sta_wds)
                    {
                        wifi_mac_add_wds_addr(nt, sta, eh.ether_shost);
                    }
                }
            }
            else
            {
                ismulticast = WIFINET_IS_MULTICAST(sta->sta_bssid);
            }
            break;
        default:
            break;
    }

    if (use4addr)
    {
        sta->sta_flags |= WIFINET_NODE_WDS;
    }

    if ((sta->sta_flags_ext&WIFINET_NODE_RDG)&&(wnet_vif->vm_flags &WIFINET_F_RDG))   // no use
    {
        addhtc =1;
        hdrsize +=4;
    }

    hdrsize_nopad = hdrsize;
    if (wifimac->wm_flags & WIFINET_F_DATAPAD)
        hdrsize = roundup(hdrsize, sizeof(unsigned int));

    skb = wifi_mac_skbhdr_adjust(wnet_vif, hdrsize, key, skb, ismulticast);
    if (skb == NULL)
    {
        /*expand skb fail and can't fill 80211 header, free old skb and return */
        return NULL;
    }
    cb = (struct wifi_skb_callback *)(skb->cb);

    llc = (struct llc *)os_skb_data(skb);
    if ( (!(os_skb_get_pktlen(skb) >= LLC_SNAPFRAMELEN &&
            llc->llc_dsap == LLC_SNAP_LSAP &&
            llc->llc_ssap == LLC_SNAP_LSAP &&
            llc->llc_control == LLC_UI))
         && (!is_amsdu ) )
    {
        llc = (struct llc *)os_skb_push(skb, LLC_SNAPFRAMELEN);    //added LLC SNAP
        llc->llc_dsap = llc->llc_ssap = LLC_SNAP_LSAP;
        llc->llc_control = LLC_UI;
        llc->llc_snap.org_code[0] = 0;
        llc->llc_snap.org_code[1] = 0;
        llc->llc_snap.org_code[2] = 0;
        llc->llc_snap.ether_type = eh.ether_type;
    }

    datalen = os_skb_get_pktlen(skb);
    cb->hdrsize=hdrsize;
    wh = (struct wifi_frame *)os_skb_push(skb, hdrsize);  //added 802.11 header
    wh->i_fc[0] = WIFINET_FC0_VERSION_0 | WIFINET_FC0_TYPE_DATA;
    wh->i_dur[0] = 0;
    wh->i_dur[1] = 0;
    if (use4addr)
    {
        wh->i_fc[1] = WIFINET_FC1_DIR_DSTODS;
        WIFINET_ADDR_COPY(wh->i_addr1, sta->sta_macaddr);
        WIFINET_ADDR_COPY(wh->i_addr2, wnet_vif->vm_myaddr);
        WIFINET_ADDR_COPY(wh->i_addr3, eh.ether_dhost);
        WIFINET_ADDR_COPY(WH4(wh)->i_addr4, eh.ether_shost);
    }
    else
    {
        switch (wnet_vif->vm_opmode)
        {
            case WIFINET_M_IBSS:
                wh->i_fc[1] = WIFINET_FC1_DIR_NODS;
                WIFINET_ADDR_COPY(wh->i_addr1, eh.ether_dhost);
                WIFINET_ADDR_COPY(wh->i_addr2, eh.ether_shost);

                WIFINET_ADDR_COPY(wh->i_addr3, wnet_vif->vm_mainsta->sta_bssid);
                break;
            case WIFINET_M_STA:
                wh->i_fc[1] = WIFINET_FC1_DIR_TODS;
                WIFINET_ADDR_COPY(wh->i_addr1, sta->sta_bssid);
                WIFINET_ADDR_COPY(wh->i_addr2, eh.ether_shost);
                WIFINET_ADDR_COPY(wh->i_addr3, eh.ether_dhost);
                break;
            case WIFINET_M_HOSTAP:
                wh->i_fc[1] = WIFINET_FC1_DIR_FROMDS;
                WIFINET_ADDR_COPY(wh->i_addr1, eh.ether_dhost);
                WIFINET_ADDR_COPY(wh->i_addr2, sta->sta_bssid);
                WIFINET_ADDR_COPY(wh->i_addr3, eh.ether_shost);
                if (M_PWR_SAV_GET(skb))
                {
                    if (WIFINET_SAVEQ_QLEN(&(sta->sta_pstxqueue)))
                    {
                        wh->i_fc[1] |= WIFINET_FC1_MORE_DATA;
                    }
                }

                if (M_FLAG_GET(skb, M_UAPSD))
                {
                    wh->i_fc[1] |= WIFINET_FC1_MORE_DATA;
                }

                break;
            case WIFINET_M_WDS:
                wh->i_fc[1] = WIFINET_FC1_DIR_DSTODS;
                WIFINET_ADDR_COPY(wh->i_addr1, sta->sta_macaddr);
                WIFINET_ADDR_COPY(wh->i_addr2, wnet_vif->vm_myaddr);
                WIFINET_ADDR_COPY(wh->i_addr3, eh.ether_dhost);
                WIFINET_ADDR_COPY(WH4(wh)->i_addr4, eh.ether_shost);
                break;
            case WIFINET_M_MONITOR:
                goto bad;
            case WIFINET_M_P2P_GO:
            case WIFINET_M_P2P_DEV:
            case WIFINET_M_P2P_CLIENT:
                goto bad;
        }
    }

    if (WIFINET_VMAC_IS_SLEEPING(wnet_vif))
    {
        wh->i_fc[1] |= WIFINET_FC1_PWR_MGT;
    }

    if (addqos)
    {
        struct wifi_qos_frame *qwh;
        struct WIFINET_S_FRAME_QOS_ADDR4 *wds;
        unsigned char *qos;
        int tid;

        if (use4addr) {
            wds = (struct WIFINET_S_FRAME_QOS_ADDR4 *)wh;
            qos = &wds->i_qos[0];
            wds->i_fc[0] |= WIFINET_FC0_SUBTYPE_QOS;
        } else {
            qwh = (struct wifi_qos_frame *)wh;
            qos = &qwh->i_qos[0];
            qwh->i_fc[0] |= WIFINET_FC0_SUBTYPE_QOS;
        }

        tid = cb->u_tid;
        qos[0] = tid & WIFINET_QOS_TID;
        if (wifimac->wm_wme[wnet_vif->wnet_vif_id].wme_wmeChanParams.cap_wmeParams[os_skb_get_priority(skb)].wmep_noackPolicy)
        {
            qos[0] |= (1 << WIFINET_QOS_ACKPOLICY_S) & WIFINET_QOS_ACKPOLICY;
        }

        if (is_amsdu)
        {
            qos[0] |= (1 << WIFINET_QOS_AMSDU_S) & WIFINET_QOS_AMSDU;
        }

        qos[1] = 0;
    }

    if (addhtc)
    {
        struct wifi_qos_htc_frame *twh = (struct wifi_qos_htc_frame*) wh;
        wh->i_fc[1] |= WIFINET_FC1_ORDER;
        twh->i_htc[0] = 0;
        twh->i_htc[1] = 0;
        twh->i_htc[2] = 0;
        twh->i_htc[3] = 0;
    }

    if (os_skb_get_pktlen(skb) > wnet_vif->vm_fragthreshold
        && (!WIFINET_IS_MULTICAST(wh->i_addr1)) && (!is_amsdu))
    {
        int pktlen, skbcnt, tailsize, ciphdrsize;
        struct wifi_mac_security *cip;
        struct wifi_skb_callback     *tcb;

        calculate_checksum_for_fragment_pkt(skb, eh, hdrsize);

        pktlen = os_skb_get_pktlen(skb);
        ciphdrsize = 0;
        tailsize = WIFINET_CRC_LEN;

        if (key != NULL)
        {
            cip = (struct wifi_mac_security *) key->wk_cipher;
            ciphdrsize = cip->wm_header;
            tailsize += (cip->wm_trailer + cip->wm_miclen);
        }

        pdusize = wnet_vif->vm_fragthreshold - (hdrsize_nopad + ciphdrsize);
        fragcnt = ((pktlen - (hdrsize_nopad + ciphdrsize)) / pdusize) +
            (((pktlen - (hdrsize_nopad + ciphdrsize)) % pdusize == 0) ? 0 : 1);

        for (skbcnt = 1; skbcnt < fragcnt; ++skbcnt)
        {
            tskb = wifi_mac_alloc_skb(wifimac, hdrsize + ciphdrsize + pdusize + tailsize);
            if (tskb == NULL)
                break;

            tskb->next = framelist;
            framelist = tskb;

            tcb = (struct wifi_skb_callback *)tskb->cb;
            tcb->sta = sta;
            tcb->flags = cb->flags;
            tcb->u_tid = cb->u_tid;
            tcb->hdrsize = cb->hdrsize;
        }

        if (skbcnt != fragcnt)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_DEBUG, eh.ether_dhost, "%s", "skbcnt != fragcnt, discard frame");
            goto bad;
        }
    }

    if (key != NULL)
    {
        if ((eh.ether_type != __constant_htons(ETHERTYPE_PAE) && eh.ether_type != __constant_htons(ETHERTYPE_WPI))
            || ((wnet_vif->vm_flags & WIFINET_F_WPA)
            && (wnet_vif->vm_opmode == WIFINET_M_STA ? !KEY_UNDEFINED(*key) : !KEY_UNDEFINED(sta->sta_ucastkey))))
        {
            int force_swmic = (fragcnt > 1) ? 1 : 0;    //only fragment need to force sw mic (for tkip, in crypto internal code)

            wh->i_fc[1] |= WIFINET_FC1_WEP;

            if (!wifi_mac_security_enmic(wnet_vif, key, skb, force_swmic))
            {
                WIFINET_DPRINTF_MACADDR( AML_DEBUG_XMIT, eh.ether_dhost, "%s", "enmic failed, discard frame");
                goto bad;
            }
        }
    }

    *(unsigned short *)wh->i_seq = 0;
    if (fragcnt > 1)
    {
        int fragnum, offset, pdulen;
        void *pdu;

        fragnum = 0;
        wh = twh = (struct wifi_frame *) os_skb_data(skb);

        wh->i_fc[1] |= WIFINET_FC1_MORE_FRAG;

        *(unsigned short *)&wh->i_seq[0] = htole16(sta->sta_txseqs[cb->u_tid] << WIFINET_SEQ_SEQ_SHIFT);
        sta->sta_txseqs[cb->u_tid]++;

        *(unsigned short *)&wh->i_seq[0] |= htole16((fragnum & WIFINET_SEQ_FRAG_MASK) << WIFINET_SEQ_FRAG_SHIFT);
        ++fragnum;

        offset = hdrsize + pdusize;
        datalen = (os_skb_get_pktlen(skb) - hdrsize) - pdusize;

        WIFINET_NODE_STAT(sta, tx_data);
        WIFINET_NODE_STAT_ADD(sta, tx_bytes, pdusize);

        for (tskb = framelist; tskb != NULL; tskb = tskb->next)
        {
            twh = (struct wifi_frame *) os_skb_put(tskb, hdrsize);
            memcpy((void *) twh, (void *) wh, hdrsize);

            *(unsigned short *)&twh->i_seq[0] |= htole16((fragnum & WIFINET_SEQ_FRAG_MASK) << WIFINET_SEQ_FRAG_SHIFT);
            ++fragnum;

            if (pdusize <= datalen)
                pdulen = pdusize;
            else
                pdulen = datalen;
            pdu = os_skb_put(tskb, pdulen);
            memcpy(pdu, (void *) os_skb_data(skb)+offset, pdulen);

            offset += pdusize;
            datalen -= pdusize;

            WIFINET_NODE_STAT(sta, tx_data);
            WIFINET_NODE_STAT_ADD(sta, tx_bytes, pdulen);
        }

        twh->i_fc[1] &= ~WIFINET_FC1_MORE_FRAG;
        os_skb_trim(skb,hdrsize + pdusize);
        os_skb_next(skb) = framelist;
    }
    else
    {
        WIFINET_NODE_STAT(sta, tx_data);
        WIFINET_NODE_STAT_ADD(sta, tx_bytes, datalen);
    }

    return skb;
bad:
    if (framelist != NULL)
    {
        struct sk_buff *temp;

        tskb = framelist;
        while (tskb)
        {
            temp = tskb->next;
            tskb->next = NULL;
            wifi_mac_free_skb(tskb);
            tskb = temp;
        }
    }

    if (skb != NULL)
    {
        wifi_mac_free_skb(skb);
    }
    return NULL;

}

unsigned char *
wifi_mac_add_rates(unsigned char *frm, const struct wifi_mac_rateset *rs)
{
    int nrates;

    *frm++ = WIFINET_ELEMID_RATES;
    nrates = rs->dot11_rate_num;
    if (nrates > WIFINET_RATE_SIZE)
        nrates = WIFINET_RATE_SIZE;
    *frm++ = nrates;
    memcpy(frm, rs->dot11_rate, nrates);

    return frm + nrates;
}

unsigned char *
wifi_mac_add_xrates(unsigned char *frm, const struct wifi_mac_rateset *rs)
{

    if (rs->dot11_rate_num > WIFINET_RATE_SIZE)
    {
        int nrates = rs->dot11_rate_num - WIFINET_RATE_SIZE;
        *frm++ = WIFINET_ELEMID_XRATES;
        *frm++ = nrates;
        memcpy(frm, rs->dot11_rate + WIFINET_RATE_SIZE, nrates);

        frm += nrates;
    }
    return frm;
}

static unsigned char *
wifi_mac_add_ssid(unsigned char *frm, const unsigned char *ssid, unsigned int len)
{
    *frm++ = WIFINET_ELEMID_SSID;
    *frm++ = len;
    memcpy(frm, ssid, len);
    return frm + len;
}

unsigned char *
wifi_mac_add_erp(unsigned char *frm, struct wifi_mac *wifimac)
{
    unsigned char erp;

    *frm++ = WIFINET_ELEMID_ERP;
    *frm++ = 1;
    erp = 0;
    if (wifimac->wm_nonerpsta != 0)
        erp |= WIFINET_ERP_NON_ERP_PRESENT;

    //if (wifimac->wm_flags & WIFINET_F_USEPROT)
    //    erp |= WIFINET_ERP_USE_PROTECTION;

    if (wifimac->wm_flags & WIFINET_F_USEBARKER)
        erp |= WIFINET_ERP_LONG_PREAMBLE;
    *frm++ = erp;
    return frm;
}

unsigned char *
wifi_mac_add_country(unsigned char *frm, struct wifi_mac *wifimac)
{
    memcpy(frm, (unsigned char *)&wifimac->wm_countryinfo, wifimac->wm_countryinfo.country_len + 2);
    frm +=  wifimac->wm_countryinfo.country_len + 2;
    return frm;
}

static unsigned char *
wifi_mac_setup_wpa_ie(struct wlan_net_vif *wnet_vif, unsigned char *ie)
{
    static const unsigned char oui[4] = { WPA_OUI_BYTES, WPA_OUI_TYPE };
    static const unsigned char cipher_suite[][4] =
    {
        { WPA_OUI_BYTES, WPA_CSE_WEP40 },
        { WPA_OUI_BYTES, WPA_CSE_TKIP },
        { 0x00, 0x00, 0x00, 0x00 },
        { WPA_OUI_BYTES, WPA_CSE_CCMP },
        { 0x00, 0x00, 0x00, 0x00 },
        { WPA_OUI_BYTES, WPA_CSE_NULL },
    };
    static const unsigned char wep104_suite[4] = { WPA_OUI_BYTES, WPA_CSE_WEP104 };
    static const unsigned char key_mgt_unspec[4] = { WPA_OUI_BYTES, WPA_ASE_8021X_UNSPEC };
    static const unsigned char key_mgt_psk[4] = { WPA_OUI_BYTES, WPA_ASE_8021X_PSK };
    const struct wifi_mac_Rsnparms *rsn = &wnet_vif->vm_mainsta->sta_rsn;
    unsigned char *frm = ie;
    unsigned char *selcnt;

    *frm++ = WIFINET_ELEMID_VENDOR;
    *frm++ = 0;
    memcpy(frm, oui, sizeof(oui));
    frm += sizeof(oui);
    ADDSHORT(frm, WPA_VERSION);

    if (rsn->rsn_mcastcipher == WIFINET_CIPHER_WEP &&
        rsn->rsn_mcastkeylen >= 13)
        ADDSELECTOR(frm, wep104_suite);
    else
        ADDSELECTOR(frm, cipher_suite[rsn->rsn_mcastcipher]);

    selcnt = frm;
    ADDSHORT(frm, 0);
    if (rsn->rsn_ucastcipherset & (1 << WIFINET_CIPHER_AES_CCM))
    {
        selcnt[0]++;
        ADDSELECTOR(frm, cipher_suite[WIFINET_CIPHER_AES_CCM]);
    }
    if (rsn->rsn_ucastcipherset & (1 << WIFINET_CIPHER_TKIP))
    {
        selcnt[0]++;
        ADDSELECTOR(frm, cipher_suite[WIFINET_CIPHER_TKIP]);
    }
    selcnt = frm;
    ADDSHORT(frm, 0);
    if (rsn->rsn_keymgmtset & (1 << WPA_ASE_8021X_UNSPEC)) {
        selcnt[0]++;
        ADDSELECTOR(frm, key_mgt_unspec);
    }

    if (rsn->rsn_keymgmtset & (1 << WPA_ASE_8021X_PSK)) {
        selcnt[0]++;
        ADDSELECTOR(frm, key_mgt_psk);
    }

    if (rsn->sa_valid != 0) {
        frm[0] = rsn->gtksa;
        frm[1] = rsn->ptksa;
        frm += 2;
    }

    ie[1] = frm - ie - 2;
    KASSERT(ie[1]+2 <= sizeof(struct wifi_mac_ie_wpa),
            ("WPA IE too big, %u > %zu", ie[1]+2, sizeof(struct wifi_mac_ie_wpa)));
    return frm;
}

static unsigned char *
wifi_mac_setup_rsn_ie(struct wlan_net_vif *wnet_vif, unsigned char *ie)
{
    static const unsigned char cipher_suite[][4] =
    {
        { RSN_OUI_BYTES, RSN_CSE_WEP40 },
        { RSN_OUI_BYTES, RSN_CSE_TKIP },
        { RSN_OUI_BYTES, RSN_CSE_WRAP },
        { RSN_OUI_BYTES, RSN_CSE_CCMP },
        { 0x00, 0x00, 0x00, 0x00 },
        { RSN_OUI_BYTES, RSN_CSE_NULL },
    };
    static const unsigned char wep104_suite[4] = {RSN_OUI_BYTES, RSN_CSE_WEP104};
    static const unsigned char key_mgt_unspec[4] = {RSN_OUI_BYTES, RSN_ASE_8021X_UNSPEC};
    static const unsigned char key_mgt_psk[4] = {RSN_OUI_BYTES, RSN_ASE_8021X_PSK};
    static const unsigned char key_mgt_psk_sha256[4] = {RSN_OUI_BYTES, RSN_ASE_8021X_PSK_SHA256};
    static const unsigned char key_mgt_psk_sae[4] = {RSN_OUI_BYTES, RSN_ASE_8021X_SAE};

    const struct wifi_mac_Rsnparms *rsn = &wnet_vif->vm_mainsta->sta_rsn;
    unsigned char *frm = ie;
    unsigned char *selcnt;

    *frm++ = WIFINET_ELEMID_RSN;
    *frm++ = 0;
    ADDSHORT(frm, RSN_VERSION);

    if ((rsn->rsn_mcastcipher == WIFINET_CIPHER_WEP) && (rsn->rsn_mcastkeylen >= 13))
        ADDSELECTOR(frm, wep104_suite);
    else
        ADDSELECTOR(frm, cipher_suite[rsn->rsn_mcastcipher]);

    selcnt = frm;
    ADDSHORT(frm, 0);
    if (rsn->rsn_ucastcipherset & (1 << WIFINET_CIPHER_AES_CCM)) {
        selcnt[0]++;
        ADDSELECTOR(frm, cipher_suite[WIFINET_CIPHER_AES_CCM]);
    }

    if (rsn->rsn_ucastcipherset & (1 << WIFINET_CIPHER_TKIP)) {
        selcnt[0]++;
        ADDSELECTOR(frm, cipher_suite[WIFINET_CIPHER_TKIP]);
    }

    selcnt = frm;
    ADDSHORT(frm, 0);
    if (rsn->rsn_keymgmtset & (1 << WPA_ASE_8021X_UNSPEC)) {
        selcnt[0]++;
        ADDSELECTOR(frm, key_mgt_unspec);
    }

    if (rsn->rsn_keymgmtset & (1 << WPA_ASE_8021X_PSK)) {
        selcnt[0]++;
        ADDSELECTOR(frm, key_mgt_psk);
    }

    if (rsn->rsn_keymgmtset & (1 << RSN_ASE_8021X_PSK_SHA256)) {
        selcnt[0]++;
        ADDSELECTOR(frm, key_mgt_psk_sha256);
    }

    if (rsn->rsn_keymgmtset & (1 << RSN_ASE_8021X_SAE)) {
        selcnt[0]++;
        ADDSELECTOR(frm, key_mgt_psk_sae);
    }

    ADDSHORT(frm, rsn->rsn_caps);

    ie[1] = frm - ie - 2;
    KASSERT(ie[1]+2 <= sizeof(struct wifi_mac_ie_wpa),
        ("RSN IE too big, %u > %zu", ie[1] + 2, sizeof(struct wifi_mac_ie_wpa)));

    return frm;

}

unsigned char *
wifi_mac_add_wpa(unsigned char *frm, struct wlan_net_vif *wnet_vif)
{

    KASSERT(wnet_vif->vm_flags & WIFINET_F_WPA, ("no WPA/RSN!"));
    if (wnet_vif->vm_flags & WIFINET_F_WPA2)
        frm = wifi_mac_setup_rsn_ie(wnet_vif, frm);

    if (wnet_vif->vm_flags & WIFINET_F_WPA1)
        frm = wifi_mac_setup_wpa_ie(wnet_vif, frm);

    return frm;
}

/*just be used by assoc req frame, because of no edca parameters. */
static unsigned char *
wifi_mac_add_wme(unsigned char *frm, struct wifi_station *sta)
{
    static const unsigned char oui[4] = { WME_OUI_BYTES, WME_OUI_TYPE };
    struct wifi_mac_ie_wme *ie = (struct wifi_mac_ie_wme *) frm;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_wme_state *wme = &sta->sta_wmac->wm_wme[wnet_vif->wnet_vif_id];

    *frm++ = WIFINET_ELEMID_VENDOR;
    *frm++ = 0;
    memcpy(frm, oui, sizeof(oui));
    frm += sizeof(oui);
    *frm++ = WME_INFO_OUI_SUBTYPE;
    *frm++ = WME_VERSION;
    switch (wnet_vif->vm_opmode)
    {
        case WIFINET_M_HOSTAP:
            *frm = wme->wme_bssChanParams.cap_info_count;
            if (WIFINET_VMAC_UAPSD_ENABLED(wnet_vif))
            {
                *frm |= WME_CAPINFO_UAPSD_EN;
            }
            frm++;
            break;
        case WIFINET_M_STA:
            *frm++ = wnet_vif->vm_uapsdinfo;
            break;
        default:
            *frm++ = 0;
            break;
    }

    ie->wme_len = frm - &ie->wme_oui[0];

    return frm;
}

unsigned char *
wifi_mac_add_wme_param(unsigned char *frm,
    struct wifi_mac_wme_state *wme,int uapsd_enable)
{
    static const unsigned char oui[4] = { WME_OUI_BYTES, WME_OUI_TYPE };
    struct wifi_mac_wme_param *ie = (struct wifi_mac_wme_param *) frm;
    int i;

    *frm++ = WIFINET_ELEMID_VENDOR;
    *frm++ = 0;
    memcpy(frm, oui, sizeof(oui));
    frm += sizeof(oui);
    *frm++ = WME_PARAM_OUI_SUBTYPE;
    *frm++ = WME_VERSION;
    *frm = wme->wme_bssChanParams.cap_info_count;
    if (uapsd_enable)
    {
        /*set uapsd  and max sp */
        *frm |= WME_CAPINFO_UAPSD_EN;
    }
    frm++;
    *frm++ = 0;
    for (i = 0; i < WME_NUM_AC; i++)
    {
        const struct wmeParams *ac =
                &wme->wme_bssChanParams.cap_wmeParams[i];
        *frm++ = SM(i, WME_PARAM_ACI)
                 | SM(ac->wmep_acm, WME_PARAM_ACM)
                 | SM(ac->wmep_aifsn, WME_PARAM_AIFSN)
                 ;
        *frm++ = SM(ac->wmep_logcwmax, WME_PARAM_LOGCWMAX)
                 | SM(ac->wmep_logcwmin, WME_PARAM_LOGCWMIN)
                 ;
        ADDSHORT(frm, ac->wmep_txopLimit);
    }

    ie->param_len = frm - &ie->param_oui[0];

    return frm;
}


static unsigned char *
wifi_mac_add_doth(unsigned char *frm, struct wlan_net_vif *wnet_vif)
{
    *frm++ = WIFINET_ELEMID_PWRCAP;
    *frm++ = 2;
    *frm++ = wnet_vif->vm_curchan->chan_minpower;
    *frm++ = wnet_vif->vm_curchan->chan_maxpower;

    *frm++ = WIFINET_ELEMID_SUPPCHAN;
    *frm++ = WIFINET_SUPPCHAN_LEN;
    memcpy(frm, wnet_vif->vm_wmac->wm_chan_avail, WIFINET_SUPPCHAN_LEN);

    return frm + WIFINET_SUPPCHAN_LEN;
}


static void
wifi_mac_set_htrates(unsigned char *supp_mcs, struct wifi_mac *wifimac)
{
    unsigned char tx_streams = wifimac->wm_tx_streams;
    unsigned char rx_streams = wifimac->wm_rx_streams;
    supp_mcs[WIFINET_TX_MCS_OFFSET] &= ~WIFINET_TX_MCS_SET;

    if (tx_streams != rx_streams)
    {
        supp_mcs[WIFINET_TX_MCS_OFFSET] |= WIFINET_TX_MCS_SET_DEFINED;
        supp_mcs[WIFINET_TX_MCS_OFFSET] |= WIFINET_TX_RX_MCS_SET_NOT_EQUAL;

        switch (tx_streams)
        {
            case 2:
                supp_mcs[WIFINET_TX_MCS_OFFSET] |= WIFINET_TX_2_SPATIAL_STREAMS;
                break;
            case 3:
                supp_mcs[WIFINET_TX_MCS_OFFSET] |= WIFINET_TX_3_SPATIAL_STREAMS;
                break;
            case 4:
                supp_mcs[WIFINET_TX_MCS_OFFSET] |= WIFINET_TX_4_SPATIAL_STREAMS;
                break;
        }
    }

    switch (rx_streams)
    {
        case 1:
            supp_mcs[WIFINET_RX_MCS_SINGLE_STREAM_BYTE_OFFSET] = WIFINET_RX_MCS_ALL_NSTREAM_RATES;
            break;
        case 2:
        default:
            supp_mcs[WIFINET_RX_MCS_SINGLE_STREAM_BYTE_OFFSET] = WIFINET_RX_MCS_ALL_NSTREAM_RATES;
            supp_mcs[WIFINET_RX_MCS_DUAL_STREAM_BYTE_OFFSET] = WIFINET_RX_MCS_ALL_NSTREAM_RATES;
    }
}

static void
wifi_mac_set_basic_htrates(unsigned char *frm, const struct wifi_mac_rateset *rs)
{
    int i;
    int nrates;

    nrates = rs->dot11_rate_num;
    if (nrates > WIFINET_HT_RATE_SIZE)
        nrates = WIFINET_HT_RATE_SIZE;

    for (i=0; i < nrates; i++)
    {
        if ((WIFINET_GET_RATE_VAL(rs->dot11_rate[i] ) < WIFINET_HT_RATE_SIZE) &&
            (rs->dot11_rate[i] & WIFINET_RATE_BASIC))
            *(frm + WIFINET_GET_RATE_VAL(rs->dot11_rate[i]) / 8) |= 1 << (WIFINET_GET_RATE_VAL(rs->dot11_rate[i]) % 8);
    }
}

unsigned char *wifi_mac_add_extcap(unsigned char *frm,struct wifi_station *sta)
{
    struct wifi_mac_ie_ext_cap *ie = (struct wifi_mac_ie_ext_cap *) frm;

    memset(ie, 0, sizeof(struct wifi_mac_ie_ext_cap));
    ie->elem_id = WIFINET_ELEMID_EXTCAP;
    ie->elem_len = sizeof(struct wifi_mac_ie_ext_cap) - 2;
    ie->ext_capflags = 1;
    ie->ext3_capflags[3] = 1 << 6;

    return frm + sizeof(struct wifi_mac_ie_ext_cap);
}

unsigned char *wifi_mac_add_timeout_interval(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac_ie_timeout *ie = (struct wifi_mac_ie_timeout *)frm;

    memset(ie, 0, sizeof(struct wifi_mac_ie_timeout));
    ie->elem_id = WIFINET_ELEMID_TIMEOUT;
    ie->elem_len = sizeof(struct wifi_mac_ie_timeout) - 2;
    ie->timeout_interval_type = ASSOC_COMEBACK_TIME;
    ie->timeout_val = 1000;

    return frm + sizeof(struct wifi_mac_ie_timeout);
}

unsigned char *
wifi_mac_add_obss_scan(unsigned char *frm,struct wifi_station *sta)
{
    // struct WIFINET_VMAC *vmac = sta->sta_vmac;
    struct wifi_mac_ie_obss_scan *ie = (struct wifi_mac_ie_obss_scan *)frm;

    memset(ie, 0, sizeof(struct wifi_mac_ie_obss_scan));
    ie->elem_id = WIFINET_ELEMID_OBSS_SCAN;
    ie->elem_len = sizeof(struct wifi_mac_ie_obss_scan) - 2;
    ie->scan_passive_dwell = htole16(WIFINET_OBSS_SCAN_PASSIVE_DWELL_DEF);
    ie->bss_cw_trigger_scan_interval= htole16(WIFINET_OBSS_SCAN_INTERVAL_DEF);
    ie->scan_active_dwell = htole16(WIFINET_OBSS_SCAN_ACTIVE_DWELL_DEF);
    ie->scan_passive_total = htole16(WIFINET_OBSS_SCAN_PASSIVE_TOTAL_DEF);
    ie->scan_active_total = htole16(WIFINET_OBSS_SCAN_ACTIVE_TOTAL_DEF);
    ie->scan_thresh = htole16(WIFINET_OBSS_SCAN_THRESH_DEF);
    ie->scan_delay = htole16(WIFINET_OBSS_SCAN_DELAY_DEF);
    return frm + sizeof (struct wifi_mac_ie_obss_scan);
}

unsigned char *
wifi_mac_add_htcap(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_ie_htcap_cmn *ie;
    int htcaplen;
    unsigned short htcap;
    struct wifi_mac_ie_htcap *htcap_ie = (struct wifi_mac_ie_htcap *)frm;

    htcap_ie->hc_id = WIFINET_ELEMID_HTCAP;
    htcap_ie->hc_len = sizeof(struct wifi_mac_ie_htcap) - 2;
    ie = &htcap_ie->hc_ie;
    htcaplen = sizeof(struct wifi_mac_ie_htcap);
    htcap = wnet_vif->vm_htcap;

    if(wifimac->wm_flags_ext & WIFINET_FEXT_SHORTGI_ENABLE) {
        htcap |= WIFINET_HTCAP_C_SHORTGI20;
    }

    if ((wnet_vif->vm_curchan != NULL) && (wnet_vif->vm_curchan->chan_bw >= WIFINET_BWC_WIDTH40)) {
        if(wifimac->wm_flags_ext & WIFINET_FEXT_SHORTGI_ENABLE)
            htcap |= WIFINET_HTCAP_C_SHORTGI40;

    } else {
        htcap &= ~WIFINET_HTCAP_SUPPORTCBW40;
    }

    if(wifimac->wm_flags & WIFINET_F_LDPC) {
        /*if our support rx ldpc,  add support rx ldpc capability to ht capability information element */
        htcap |= WIFINET_HTCAP_LDPC;
    }

    htcap |= (wnet_vif->vm_ht40_intolerant) ? WIFINET_HTCAP_40M_INTOLERANT : 0;
    ie->hc_cap = htole16(htcap);
    ie->hc_maxampdu = wifimac->wm_maxampdu;
    ie->hc_mpdudensity = wifimac->wm_mpdudensity;
    ie->hc_reserved = 0;

    memset(ie->hc_mcsset, 0, sizeof(ie->hc_mcsset));
    wifi_mac_set_htrates(ie->hc_mcsset, wifimac);

    if (wnet_vif->vm_flags &WIFINET_F_RDG) {
        ie->hc_extcap |= WIFINET_HTCAP_EXTC_HTC_SUPPORT |WIFINET_HTCAP_EXTC_RDG_RESPONDER;

    } else {
        ie->hc_extcap  = 0;
    }
    ie->hc_txbf = 0;
    ie->hc_antenna = 0;
    return frm + htcaplen;
}

static void print_ht_opt_information(unsigned char* ht_opt)
{
    if (ht_opt[0] != WIFINET_ELEMID_HTINFO)
    {
        pr_warn("%s(%d), Not HT OP IE:%d.\n\n",__func__,__LINE__,ht_opt[0]);
        return;
    }

    /*Print Primary Channel*/
    pr_debug("%s(%d),primary channel:%d.\n",__func__,__LINE__,ht_opt[2]);

    /*Second channel offset parsing*/
    switch( ht_opt[3] & 0x03 )
    {
        case 1:
            pr_debug("%s(%d),Secondary Channel is ABOVE the primary channel.\n",__func__,__LINE__);
            break;
        case 3:
            pr_debug("%s(%d),Secondary Channel is BELOW the primary channel.\n",__func__,__LINE__);
            break;
        case 0:
            pr_debug("%s(%d),No Secondary Channel is present.\n",__func__,__LINE__);
            break;
        default:
            pr_warn("%s(%d),Reserved value.\n",__func__,__LINE__);
            break;
    }

    /*STA Channel Width parsing*/
    if ( ht_opt[3] & 0x03 )
    {
       pr_debug("%s(%d),HT_OP:Allow use of any channel width in the supported channel width set.\n",__func__,__LINE__);
    }
    else
    {
       pr_debug("%s(%d),HT_OP:20M channel bandwidth.\n",__func__,__LINE__);
    }

    return;
}
unsigned char *
wifi_mac_add_htinfo(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wifi_mac_ie_htinfo_cmn *ie;
    int htinfolen;

    struct wifi_mac_ie_htinfo *htinfo = (struct wifi_mac_ie_htinfo *)frm;

    htinfo->hi_id = WIFINET_ELEMID_HTINFO;
    htinfo->hi_len = sizeof(struct wifi_mac_ie_htinfo) - 2;

    ie = &htinfo->hi_ie;
    htinfolen = sizeof(struct wifi_mac_ie_htinfo);

    memset(ie, 0, sizeof(struct wifi_mac_ie_htinfo_cmn));

    ie->hi_ctrlchannel = wifi_mac_chan2ieee(wifimac, wifimac->wm_curchan);
    ie->hi_extchoff = sta->sta_wnet_vif->scnd_chn_offset;
    ie->hi_txchwidth = ((sta->sta_wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH40)
        || (sta->sta_wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH80))
        ? WIFINET_HTINFO_TXWIDTH_2040 : WIFINET_HTINFO_TXWIDTH_20;

    /*802.11-2016.pdf   9.4.2.57 HT Operation element  new add*/
    //ie->hi_chan_center_freq_seg2 = wifimac->wm_curchan->chan_cfreq1;

    ie->hi_nongfpresent = 1; /*Nongreenfield HT STAs Present*/

    ie->hi_opmode = wifimac->wm_ht_prot_sm;
    if ((wifimac->wm_ht_prot_sm == WIFINET_HTINFO_OPMODE_MIXED_PROT_OPT)
        || (wifimac->wm_ht_prot_sm == WIFINET_HTINFO_OPMODE_MIXED_PROT_ALL)) {
        ie->hi_obssnonhtpresent = WIFINET_HTINFO_OBBSS_NONHT_PRESENT;
        ie->hi_rifsmode = WIFINET_HTINFO_RIFSMODE_PROHIBITED;

    } else {
        ie->hi_obssnonhtpresent = WIFINET_HTINFO_OBBSS_NONHT_NOT_PRESENT;
        ie->hi_rifsmode = WIFINET_HTINFO_RIFSMODE_ALLOWED;
    }

    memset(ie->hi_basicmcsset, 0, sizeof(ie->hi_basicmcsset));
    wifi_mac_set_basic_htrates(ie->hi_basicmcsset, &sta->sta_htrates);

    return frm + htinfolen;
}

#if 0
void wifi_mac_set_vht_rates( struct wifi_mac  *wifimac, struct wlan_net_vif *wnet_vif )

{
    unsigned char tx_streams = wifimac->wm_tx_streams;
    unsigned char  rx_streams =wifimac->wm_rx_streams;
    unsigned char bcc_11ng_256qam_20mhz_s = 0;

    bcc_11ng_256qam_20mhz_s = IEEE80211_IS_CHAN_11NG(ic->ic_curchan) &&
                                ieee80211_vap_256qam_is_set(wnet_vif) &&
                                !ieee80211_vap_ldpc_is_set(wnet_vif) &&
                                (ic_cw_width == WIFINET_BWC_WIDTH20);
    /* Adjust supported rate set based on txchainmask */
    switch (tx_streams) {
        default:
            /* Default to single stream */
        case 1:
             /*MCS9 is not supported for BCC, NSS=1,2 in 20Mhz */
            if(bcc_11ng_256qam_20mhz_s) {
                 wifimac->wm_vhtcap_max_mcs.tx_mcs_set.mcs_map = VHT_MCSMAP_NSS1_MCS0_8; /* MCS 0-8 */
            }
            else {
                 wifimac->wm_vhtcap_max_mcs.tx_mcs_set.mcs_map = VHT_MCSMAP_NSS1_MCS0_9; /* MCS 0-9 */
            }
            if (wnet_vif->iv_vht_tx_mcsmap) {
                wifimac->ic_vhtcap_max_mcs.tx_mcs_set.mcs_map = (wnet_vif->iv_vht_tx_mcsmap | VHT_MCSMAP_NSS1_MASK);
            }
        break;

        case 2:
            /* Dual stream */
             /*MCS9 is not supported for BCC, NSS=1,2 in 20Mhz */
            if(bcc_11ng_256qam_20mhz_s) {
                 wifimac->ic_vhtcap_max_mcs.tx_mcs_set.mcs_map = VHT_MCSMAP_NSS2_MCS0_8; /* MCS 0-8 */
            }
            else {
                 wifimac->ic_vhtcap_max_mcs.tx_mcs_set.mcs_map = VHT_MCSMAP_NSS2_MCS0_9; /* MCS 0-9 */
            }
            if (wnet_vif->iv_vht_tx_mcsmap) {
                wifimac->ic_vhtcap_max_mcs.tx_mcs_set.mcs_map = (wnet_vif->iv_vht_tx_mcsmap | VHT_MCSMAP_NSS2_MASK);
            }
        break;

        case 3:
            /* Tri stream */
            wifimac->ic_vhtcap_max_mcs.tx_mcs_set.mcs_map = VHT_MCSMAP_NSS3_MCS0_9; /* MCS 0-9 */
            if (wnet_vif->iv_vht_tx_mcsmap) {
                wifimac->ic_vhtcap_max_mcs.tx_mcs_set.mcs_map = (wnet_vif->iv_vht_tx_mcsmap | VHT_MCSMAP_NSS3_MASK);
            }
        break;
    }

    /* Adjust rx rates based on the rx chainmask */
    switch (rx_streams) {
        default:
            /* Default to single stream */
        case 1:
             /*MCS9 is not supported for BCC, NSS=1,2 in 20Mhz */
            if(bcc_11ng_256qam_20mhz_s) {
                 wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = VHT_MCSMAP_NSS1_MCS0_8; /* MCS 0-8 */
            }
            else {
                 wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = VHT_MCSMAP_NSS1_MCS0_9;
            }
            if (wnet_vif->iv_vht_rx_mcsmap) {
                wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = (wnet_vif->iv_vht_rx_mcsmap | VHT_MCSMAP_NSS1_MASK);
            }
        break;

        case 2:
            /* Dual stream */
             /*MCS9 is not supported for BCC, NSS=1,2 in 20Mhz */
            if(bcc_11ng_256qam_20mhz_s) {
                 wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = VHT_MCSMAP_NSS2_MCS0_8; /* MCS 0-8 */
            }
            else {
                 wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = VHT_MCSMAP_NSS2_MCS0_9;
            }
            if (wnet_vif->iv_vht_rx_mcsmap) {
                wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = (wnet_vif->iv_vht_rx_mcsmap | VHT_MCSMAP_NSS2_MASK);
            }
        break;

        case 3:
            /* Tri stream */
            wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = VHT_MCSMAP_NSS3_MCS0_9;
            if (wnet_vif->iv_vht_rx_mcsmap) {
                wifimac->ic_vhtcap_max_mcs.rx_mcs_set.mcs_map = (wnet_vif->iv_vht_rx_mcsmap | VHT_MCSMAP_NSS3_MASK);
            }
        break;
    }
}
#endif

unsigned char *
wifi_mac_add_vht_cap(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    int vht_cap_len = sizeof(struct wifi_mac_ie_vht_cap);
    unsigned char tx_streams = wifimac->wm_tx_streams;    /* Default to max 1 tx spatial stream */
    unsigned char rx_streams = wifimac->wm_rx_streams;    /* Default to max 1 rx spatial stream */
    struct wifi_mac_ie_vht_cap *vht_cap_ie = ( struct wifi_mac_ie_vht_cap *)frm;

    ASSERT(frm);

    memset(vht_cap_ie,0, sizeof(struct wifi_mac_ie_vht_cap));
    vht_cap_ie->elem_id = WIFINET_ELEMID_VHTCAP;
    vht_cap_ie->elem_len = sizeof(struct wifi_mac_ie_vht_cap) - 2;

    if (wifimac->wm_flags_ext & WIFINET_FEXT_SHORTGI_ENABLE) {
        SET_VHT_CAP_80M_SGI( vht_cap_ie->vht_cap_info, VHT_CAP_SUPPORT_80M_SGI );

    } else {
        sta->sta_vhtcap &= ~WIFINET_VHTCAP_SHORTGI_80;
    }

    if ((wnet_vif->vm_ldpc & WIFINET_HTCAP_C_LDPC_RX) && (wifimac->wm_flags& WIFINET_F_LDPC)) {
        SET_VHT_CAP_RX_LPDC( vht_cap_ie->vht_cap_info, VHT_CAP_RX_LDPC_SUPPORTED );

    } else if((wnet_vif->vm_tx_stbc) && (tx_streams >= 1)) {
        SET_VHT_CAP_TX_STBC( vht_cap_ie->vht_cap_info, VHT_CAP_NOT_SUPPORT_TX_STBC );
    }

    if ((wnet_vif->vm_rx_stbc) && (rx_streams >= 1)) {
        SET_VHT_CAP_RX_STBC( vht_cap_ie->vht_cap_info, VHT_CAP_RX_STBC_MAX_1SS );
    }
#ifdef SU_BF
    if (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_SU_BFMEE)
    {
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
            SET_VHT_CAP_SU_BFMER(vht_cap_ie->vht_cap_info, VHT_CAP_SUPPORT_SU_BFMER);
        else
            SET_VHT_CAP_SU_BFMEE(vht_cap_ie->vht_cap_info, VHT_CAP_SUPPORT_SU_BFMEE);
    }
    else
#endif
    {
        SET_VHT_CAP_SU_BFMER(vht_cap_ie->vht_cap_info, VHT_CAP_NOT_SUPPORT_SU_BFMER);
        SET_VHT_CAP_SU_BFMEE(vht_cap_ie->vht_cap_info, VHT_CAP_NOT_SUPPORT_SU_BFMEE);
    }

    SET_VHT_CAP_SND_DMS_NUM(vht_cap_ie->vht_cap_info, 0);
#ifdef MU_BF
    if (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MU_BFMEE)
    {
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
            SET_VHT_CAP_MU_BFMER(vht_cap_ie->vht_cap_info, VHT_CAP_SUPPORT_MU_BFMER);
        else
        {
            SET_VHT_CAP_MU_BFMEE(vht_cap_ie->vht_cap_info, VHT_CAP_SUPPORT_MU_BFMEE);
        }
    }
    else
#endif
    {
        SET_VHT_CAP_MU_BFMER(vht_cap_ie->vht_cap_info, VHT_CAP_NOT_SUPPORT_MU_BFMER);
        SET_VHT_CAP_MU_BFMEE(vht_cap_ie->vht_cap_info, VHT_CAP_NOT_SUPPORT_MU_BFMEE);
    }

#if defined(SU_BF) || defined(MU_BF)
    if ((wifimac->wm_flags_ext2 & WIFINET_VHTCAP_SU_BFMEE) || (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MU_BFMEE))
        SET_VHT_CAP_BFMEE_STS(vht_cap_ie->vht_cap_info, wifimac->max_spatial - 1);
    else
#endif
        SET_VHT_CAP_BFMEE_STS(vht_cap_ie->vht_cap_info, 0);

    SET_VHT_CAP_TXOP_PS(vht_cap_ie->vht_cap_info, VHT_CAP_TXOP_PS_NOT_SUPPORT);
    SET_VHT_CAP_HTC_VHT(vht_cap_ie->vht_cap_info, VHT_CAP_NOT_SUPPORT_HTCVHT);
    SET_VHT_CAP_MAX_APMDU_LEN_EXP(vht_cap_ie->vht_cap_info,  MAX_VHT_AMPDU_32K);
    SET_VHT_CAP_LK_ADP(vht_cap_ie->vht_cap_info, VHT_CAP_LK_ADP_NO_FB);
    SET_VHT_CAP_RX_ATN_CONSTN(vht_cap_ie->vht_cap_info, VHT_CAP_RX_ATN_CONSTN_NOT_CHANGE);
    SET_VHT_CAP_TX_ATN_CONSTN(vht_cap_ie->vht_cap_info, VHT_CAP_TX_ATN_CONSTN_NOT_CHANGE);
    SET_VHT_CAP_RSV(vht_cap_ie->vht_cap_info, 0);
    SET_VHT_CAP_MAX_MPDU(vht_cap_ie->vht_cap_info, VHT_CAP_MAX_MPDU_LEN_7991);

    /*filled  max mpdu length capability */
    if (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MAX_MPDU_LEN_11454) {
        SET_VHT_CAP_MAX_MPDU(vht_cap_ie->vht_cap_info,   VHT_CAP_MAX_MPDU_LEN_11454);

    } else if (wifimac->wm_flags_ext2 & WIFINET_VHTCAP_MAX_MPDU_LEN_7991) {
        SET_VHT_CAP_MAX_MPDU(vht_cap_ie->vht_cap_info,   VHT_CAP_MAX_MPDU_LEN_7991);

    } else {
        SET_VHT_CAP_MAX_MPDU(vht_cap_ie->vht_cap_info,   VHT_CAP_MAX_MPDU_LEN_3895);
    }

    /* Fill in the VHT MCS info */
    //ieee80211_set_vht_rates(ic,vap);
    vht_cap_ie->rx_mcs_map =  VHT_MCSMAP_NSS1_MCS0_9;//__swapb16(wifimac->wm_vhtcap_max_mcs.rx_mcs_set.mcs_map);
    vht_cap_ie->rx_high_data_rate = 0x186;//__swapb16(wifimac->wm_vhtcap_max_mcs.rx_mcs_set.data_rate);
    vht_cap_ie->tx_mcs_map = VHT_MCSMAP_NSS1_MCS0_9;//__swapb16(wifimac->wm_vhtcap_max_mcs.tx_mcs_set.mcs_map);
    vht_cap_ie->tx_high_data_rate = 0x186;//__swapb16(wifimac->wm_vhtcap_max_mcs.tx_mcs_set.data_rate);

    return frm + vht_cap_len;
}

static void print_vht_opt_information(unsigned char* vht_opt)
{
    if (vht_opt[0] != WIFINET_ELEMID_VHTOP)
    {
        pr_debug("%s(%d), Not VHT OP IE:%d.\n\n",__func__,__LINE__,vht_opt[0]);
        return;
    }
    if (vht_opt[2] & 0x3 )/*Channel Width,1:80M;0:20M or 40M BSS bandwidth*/
    {
        pr_debug("%s(%d),Channel bandwidth: 80M.\n",__func__,__LINE__);
    }
    else
    {
        pr_debug("%s(%d),Channel bandwidth: 20M or 40M.\n",__func__,__LINE__);
    }

    /*Print vht_oper_centr_freq_seg0/1_idx*/
    pr_debug("%s(%d),vht_oper_centr_freq_seg0_idx:%d.\n",__func__,__LINE__,vht_opt[3]);
    pr_debug("%s(%d),vht_oper_centr_freq_seg1_idx:%d.\n",__func__,__LINE__,vht_opt[4]);

    return;
}

unsigned char*wifi_mac_add_vht_opt(unsigned char *frm, struct wifi_station *sta, unsigned int sub_type)
{
    struct wifi_mac_ie_vht_opt *vhtop = (struct wifi_mac_ie_vht_opt *)frm;

    int vht_opt_len = sizeof(struct wifi_mac_ie_vht_opt);
    //default xsect_bw should be 20M.
    enum wifi_mac_bwc_width xsect_bw = WIFINET_BWC_WIDTH20;

    ASSERT(frm);

    vhtop->elem_id = WIFINET_ELEMID_VHTOP;
    vhtop->elem_len = sizeof(struct wifi_mac_ie_vht_opt) - 2;

    //use the biggest bw when broadcast your capability in beacon or prob req as AP
    if ((sub_type == WIFINET_FC0_SUBTYPE_PROBE_RESP) || (sub_type == WIFINET_FC0_SUBTYPE_BEACON)) {
        xsect_bw = sta->sta_wnet_vif->vm_bandwidth;
    }
    /*use the proper bw when negotiate your capability in associate response frame as AP*/
    else if (sub_type == WIFINET_FC0_SUBTYPE_ASSOC_RESP) {
        xsect_bw = (sta->sta_wnet_vif->vm_bandwidth > sta->sta_chbw) ? sta->sta_chbw : sta->sta_wnet_vif->vm_bandwidth;

    } else {
        ERROR_DEBUG_OUT("Not supported sub_type:0x%x\n ", sub_type);
    }

    vhtop->vht_op_chwidth = (xsect_bw >= WIFINET_BWC_WIDTH80) ? 1 : 0;

    if (sta->sta_wnet_vif->vm_curchan != NULL) {
        vhtop->vht_op_ch_freq_seg1 = wifi_mac_mhz2chan(sta->sta_wnet_vif->vm_curchan->chan_cfreq1);

    } else {
        ERROR_DEBUG_OUT("vm_curchan is NULL!!!, vm_macmode:%d\n", sta->sta_wnet_vif->vm_mac_mode);
        vhtop->vht_op_ch_freq_seg1 = 0;
    }
    vhtop->vht_op_ch_freq_seg2 = 0;

    /* Note: This is applicable only for 80+80Mhz mode */
   // vhtop->vht_op_ch_freq_seg2 = vmac->iv_bsschan->ic_vhtop_ch_freq_seg2;   //Not  implemented yet

    /* Fill in the VHT Basic MCS set */
    vhtop->vhtop_basic_mcs_set =  htole16(sta->sta_wnet_vif->vm_vhtop_basic_mcs);

    DPRINTF(AML_DEBUG_BWC,"%s(%d) sub_type:0x%x, self_bw: 0x%x, peer_bw:0x%x, xsect_bw:0x%x,"
        "bw_in_frm: 0x%x, vht_op_ch_freq_seg1:%d, vht_op_ch_freq_seg2:%d\n",
        __func__, __LINE__, sub_type,  sta->sta_wnet_vif->vm_bandwidth, sta->sta_chbw, xsect_bw,
        vhtop->vht_op_chwidth, vhtop->vht_op_ch_freq_seg1, vhtop->vht_op_ch_freq_seg2);

    return frm + vht_opt_len;
}

unsigned char *wifi_mac_add_vht_txpw(unsigned char *frm, struct wifi_station * sta)
{
    struct wifi_mac_ie_vht_txpwr_env *ie_vht_txpwr_env = (struct wifi_mac_ie_vht_txpwr_env *)frm;
    int vht_txpw_env_len = sizeof(struct wifi_mac_ie_vht_txpwr_env);

    ASSERT(frm);
    ie_vht_txpwr_env->elem_id = WIFINET_ELEMID_VHT_TX_PWR_ENVLP;
    ie_vht_txpwr_env->elem_len = vht_txpw_env_len -2;
    SET_LMAX_TXPW_CNT(ie_vht_txpwr_env->txpwr_info, 0);
    SET_LMAX_TXPW_UNIT_INTP_CNT(ie_vht_txpwr_env->txpwr_info, 0);

    memset(ie_vht_txpwr_env->local_max_txpwr, 0, LC_MAX_TXPW_LEN);

    return frm + vht_txpw_env_len;
}

unsigned char *wifi_mac_add_vht_ch_sw_wrp(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac_ie_vht_ch_sw_wrp* ie_vht_ch_sw_wrp = (struct wifi_mac_ie_vht_ch_sw_wrp*)frm;
    int vht_ch_sw_wrp_len = sizeof(struct wifi_mac_ie_vht_ch_sw_wrp);
    ASSERT(frm);

    ie_vht_ch_sw_wrp->elem_id = WIFINET_ELEMID_CHAN_SWITCH_WRAP;
    ie_vht_ch_sw_wrp->elem_len = vht_ch_sw_wrp_len -2;

    memset(ie_vht_ch_sw_wrp->new_country_sub_ie, 0 ,  SUB_IE_MAX_LEN);
    memset(ie_vht_ch_sw_wrp->wide_bw_ch_sw_sub_ie, 0 , SUB_IE_MAX_LEN);
    memset(ie_vht_ch_sw_wrp->new_vht_tx_pw_sub_ie, 0, SUB_IE_MAX_LEN);

    return frm + vht_ch_sw_wrp_len;
}

unsigned char *wifi_mac_add_vht_ch_sw_an(unsigned char *frm, struct wifi_station *sta)
{
    int vht_ch_sw_an_len = sizeof(struct wifi_mac_channelswitch_ie);
    struct wifi_mac_channelswitch_ie *ie_vht_ch_sw_an = (struct wifi_mac_channelswitch_ie*)frm;

    ASSERT(frm);
    ie_vht_ch_sw_an->ie = WIFINET_ELEMID_CHANSWITCHANN;
    ie_vht_ch_sw_an->len = vht_ch_sw_an_len - 2;
    ie_vht_ch_sw_an->switchmode = 0;
    ie_vht_ch_sw_an->newchannel = 149;
    ie_vht_ch_sw_an->tbttcount = 1;
    return frm + vht_ch_sw_an_len;
}

unsigned char *wifi_mac_add_vht_ext_bss_ld(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac_ie_vht_ext_bss_ld * ie_vht_ext_bss_ld = (struct wifi_mac_ie_vht_ext_bss_ld *)frm;
    int vht_ext_bss_ld_len = sizeof(struct wifi_mac_ie_vht_ext_bss_ld);
    ASSERT(frm);

    ie_vht_ext_bss_ld->elem_id = WIFINET_ELEMID_EXT_BSS_LOAD;
    ie_vht_ext_bss_ld->elem_len = vht_ext_bss_ld_len - 2;

    ie_vht_ext_bss_ld->mu_mimo_sta_cnt = 0;
    ie_vht_ext_bss_ld->obs_scnd_20mhz = 0;
    ie_vht_ext_bss_ld->obs_scnd_40mhz = 0;
    ie_vht_ext_bss_ld->obs_scnd_80mhz = 0;

    return frm + vht_ext_bss_ld_len;
}

unsigned char *wifi_mac_add_vht_quiet_ch(unsigned char *frm, struct wifi_station *sta)
{
    struct wifi_mac_ie_vht_quiet_chn*  ie_vht_quiet_chn = (struct wifi_mac_ie_vht_quiet_chn*)frm;
    int vht_quiet_ch_len  = sizeof(struct wifi_mac_ie_vht_quiet_chn);
    ASSERT(frm);

    ie_vht_quiet_chn->elem_id = WIFINET_ELEMID_QUIET_CHANNEL;
    if (sta->sta_wnet_vif->vm_opmode == WIFINET_M_HOSTAP) {
        ie_vht_quiet_chn->elem_len = vht_quiet_ch_len -2;
        ie_vht_quiet_chn->ap_quiet_md = EN_AP_MODE_IN_QUIET_IE;

        ie_vht_quiet_chn->quiet_cnt = 0;
        ie_vht_quiet_chn->quiet_prd = 0;
        ie_vht_quiet_chn->quiet_drt = 0;
        ie_vht_quiet_chn->quiet_offset = 0;
        return frm + vht_quiet_ch_len;

    } else/*in no sta mode, there is only mode =0 in quit ie */{
        ie_vht_quiet_chn->ap_quiet_md = DIS_AP_MODE_IN_QUIET_IE;
        ie_vht_quiet_chn->elem_len = 1;	 // ie mode
        return frm + 1 + 1 + 1; // ie_id + ie_len + opmode in no AP mode
    }
}

unsigned char *wifi_mac_add_vht_op_md_ntf(unsigned char *frm, struct wifi_station *sta)
{
    int vht_op_md_ntf_len = sizeof(struct wifi_mac_ie_vht_opt_md_ntf);
    struct wifi_mac_ie_vht_opt_md_ntf *vht_opt_md_ntf = (struct wifi_mac_ie_vht_opt_md_ntf*)frm;

    ASSERT(frm);
    vht_opt_md_ntf->elem_id = WIFINET_ELEMID_OP_MODE_NOTIFY;
    vht_opt_md_ntf->elem_len = vht_op_md_ntf_len - 2;
    vht_opt_md_ntf->opt_mode = 0;
    return frm + vht_op_md_ntf_len;
}

unsigned char *
wifi_mac_add_vendor_ie(unsigned char *frm, struct wifi_mac *wifimac, unsigned char index)
{
    *frm++ = WIFINET_ELEMID_VENDOR;
    *frm++ = wifimac->wm_vendorinfo[index].len;
    memcpy(frm, &wifimac->wm_vendorinfo[index].buf, wifimac->wm_vendorinfo[index].len);

    return frm + wifimac->wm_vendorinfo[index].len;
}

int wifi_mac_send_probereq(struct wifi_station *sta, const unsigned char sa[WIFINET_ADDR_LEN],
    const unsigned char da[WIFINET_ADDR_LEN], const unsigned char bssid[WIFINET_ADDR_LEN],
    const unsigned char *ssid, size_t ssidlen, const void *optie, size_t optielen)
{
    struct wlan_net_vif *wnet_vif;
    struct wifi_mac *wifimac;
    struct wifi_frame *wh;
    struct wifi_skb_callback *cb;
    struct sk_buff *skb;
    unsigned char *frm;
    unsigned char *t_cursor;

    //int htcaplen;
    int skb_len;
    if (sta == NULL) {
        return -EINVAL;
    }
    wnet_vif = sta->sta_wnet_vif;
    wifimac = sta->sta_wmac;

    skb_len = 2 + WIFINET_NWID_LEN
                + 2 + WIFINET_RATE_SIZE
                + 2 + (WIFINET_RATE_MAXSIZE - WIFINET_RATE_SIZE)
                + sizeof(struct wifi_mac_ie_htcap) //htcaplen
                + wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ].length
                + sizeof(struct wifi_mac_ie_vht_cap);

    skb = wifi_mac_get_mgmt_frm(wifimac, skb_len);
    if (skb == NULL) {
        wnet_vif->vif_sts.sts_tx_no_buf++;
        return ENOMEM;
    }
    frm = os_skb_put(skb, skb_len);

    t_cursor = frm;
    frm = wifi_mac_add_ssid(frm, ssid, ssidlen);
    frm = wifi_mac_add_rates(frm, &wnet_vif->vm_legacy_rates);
    frm = wifi_mac_add_xrates(frm, &wnet_vif->vm_legacy_rates);

    if (wnet_vif->vm_mac_mode >= WIFINET_MODE_11N) {
        frm = wifi_mac_add_htcap(frm, sta);
    }

    if (wnet_vif->vm_mac_mode >= WIFINET_MODE_11AC) {
        frm = wifi_mac_add_vht_cap(frm, sta);
    }

    //p2p ie is included in app_ie[WIFINET_APPIE_FRAME_PROBE_REQ] from scan cmd,
    //not from p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_REQ]
    if (wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ].ie) {
        memcpy(frm, wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ].ie,
               wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ].length);
        frm += wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_REQ].length;
    }

    os_skb_trim(skb, frm - os_skb_data(skb));

    cb = (struct wifi_skb_callback *)skb->cb;
    cb->sta = sta;

    wh = (struct wifi_frame *)os_skb_push(skb, sizeof(struct wifi_frame));
    wifi_mac_send_setup(wnet_vif, sta, wh, WIFINET_FC0_TYPE_MGT | WIFINET_FC0_SUBTYPE_PROBE_REQ, sa, da, bssid);

    WIFINET_NODE_STAT(sta, tx_probereq);
    WIFINET_NODE_STAT(sta, tx_mgmt);

    wifi_mac_tx_mgmt_frm(wifimac, skb);
    return 0;
}

int wifi_mac_send_probe_rsp(struct wlan_net_vif  *wnet_vif,
                                                        struct wifi_station *sta,void *arg)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct sk_buff *skb;
    unsigned char *frm;
    unsigned short capinfo;
    unsigned short pkt_len;
    unsigned char  index;

    pkt_len = 8 + sizeof(unsigned short) + sizeof(unsigned short) + 2 + WIFINET_NWID_LEN + 2 + WIFINET_RATE_SIZE
        + 7 + 6 + 3 + 2 + (WIFINET_RATE_MAXSIZE - WIFINET_RATE_SIZE) + wifimac->wm_countryinfo.country_len + 2
        + 3 + (wnet_vif->vm_flags & WIFINET_F_WPA ? 2 * sizeof(struct wifi_mac_ie_wpa) : 0) + sizeof(struct wifi_mac_wme_param)
#ifdef CONFIG_P2P
        + wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length
#ifdef CONFIG_WFD
        + wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length
#endif//#ifdef CONFIG_WFD
#endif//#ifdef CONFIG_P2P
        + sizeof(struct wifi_mac_ie_htcap) + sizeof(struct wifi_mac_ie_htinfo) + sizeof (struct wifi_mac_ie_obss_scan)
        + sizeof (struct wifi_mac_ie_ext_cap) + WIFINET_APPIE_MAX + sizeof(struct wifi_mac_ie_vht_cap) + sizeof(struct wifi_mac_ie_vht_opt)
        + sizeof(struct wifi_mac_ie_vht_txpwr_env) + sizeof(struct wifi_mac_ie_vht_ch_sw_wrp) + sizeof(struct wifi_mac_vendor_ie) * VENDOR_IE_MAX;

    skb = wifi_mac_get_mgmt_frm(wifimac, pkt_len);
    if (skb == NULL)
    {
        wnet_vif->vif_sts.sts_tx_no_buf++;
        return ENOMEM;
    }
    frm = os_skb_put(skb, pkt_len);

    memset(frm, 0, 8);
    frm += 8;
    *(unsigned short *)frm = htole16(wnet_vif->vm_bcn_intval);
    frm += 2;

    //ESS=bit0, ap-sta
    //IBSS=bit1, ibss sta1, sta2
    //p2p, 0
#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_enable) {
        if (wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_GO) {
            capinfo = WIFINET_CAPINFO_ESS;

        } else {
            capinfo = 0;
        }
    }
    else
#endif//CONFIG_P2P
    {
        if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
            capinfo = WIFINET_CAPINFO_IBSS;
        else
            capinfo = WIFINET_CAPINFO_ESS;
    }

    if (wnet_vif->vm_flags & WIFINET_F_PRIVACY)
        capinfo |= WIFINET_CAPINFO_PRIVACY;

    if ((wifimac->wm_flags & WIFINET_F_SHPREAMBLE) && WIFINET_IS_CHAN_2GHZ(wifimac->wm_curchan))
        capinfo |= WIFINET_CAPINFO_SHORT_PREAMBLE;

    if (wifimac->wm_flags & WIFINET_F_SHSLOT)
        capinfo |= WIFINET_CAPINFO_SHORT_SLOTTIME;

    *(unsigned short *)frm = htole16(capinfo);
    frm += 2;

    if (wnet_vif->vm_p2p->p2p_enable) {
        if (wnet_vif->vm_p2p->p2p_flag & P2P_REQUEST_SSID) {
            frm = wifi_mac_add_ssid(frm, wnet_vif->vm_mainsta->sta_essid, wnet_vif->vm_mainsta->sta_esslen);
            wnet_vif->vm_p2p->p2p_flag &= (~P2P_REQUEST_SSID);

        } else {
            frm = wifi_mac_add_ssid(frm, P2P_WILDCARD_SSID,P2P_WILDCARD_SSID_LEN);
        }
    } else {
        frm = wifi_mac_add_ssid(frm, wnet_vif->vm_mainsta->sta_essid, wnet_vif->vm_mainsta->sta_esslen);
    }

    //p2p not support 11b rate
#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_enable)
    {
        frm = wifi_mac_add_rates(frm, &wnet_vif->vm_legacy_rates);
        frm = wifi_mac_add_xrates(frm, &wnet_vif->vm_legacy_rates);
    }
    else
#endif//CONFIG_P2P
    {
        frm = wifi_mac_add_rates(frm, &wnet_vif->vm_mainsta->sta_rates);
        frm = wifi_mac_add_xrates(frm, &wnet_vif->vm_mainsta->sta_rates);
    }

    /*5G mode ,non-support DSSS Parameter Set*/
    if( WIFINET_IS_CHAN_2GHZ(wifimac->wm_curchan) )
    {
        *frm++ = WIFINET_ELEMID_DSPARMS;
        *frm++ = 1;
        *frm++ = wifi_mac_chan2ieee(wifimac, wifimac->wm_curchan);
    }

    if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
    {
        *frm++ = WIFINET_ELEMID_IBSSPARMS;
        *frm++ = 2;
        *frm++ = 0;
        *frm++ = 0;
    }

    if ((wifimac->wm_flags & WIFINET_F_DOTH) ||
        (wifimac->wm_flags_ext & WIFINET_FEXT_COUNTRYIE))
    {
        if (wifimac->wm_countryinfo.country_len == 0)
        {
            AML_OUTPUT("country ie should init\n");
            wifi_mac_build_country_ie(wnet_vif);
        }
        frm = wifi_mac_add_country(frm, wifimac);
    }

    if (wifimac->wm_flags & WIFINET_F_DOTH)
    {
        *frm++ = WIFINET_ELEMID_PWRCNSTR;
        *frm++ = 1;
        *frm++ = WIFINET_PWRCONSTRAINT_VAL(wnet_vif);
    }

    if (WIFINET_INCLUDE_11G(wnet_vif->vm_mac_mode))
        frm = wifi_mac_add_erp(frm, wifimac);

    if (wnet_vif->vm_flags & WIFINET_F_WPA)
        frm = wifi_mac_add_wpa(frm, wnet_vif);

    if (wnet_vif->vm_flags & WIFINET_F_WME)
    {
        frm = wifi_mac_add_wme_param(frm, &wifimac->wm_wme[wnet_vif->wnet_vif_id],
                                    WIFINET_VMAC_UAPSD_ENABLED(wnet_vif));
    }

    if (sta->sta_flags & WIFINET_NODE_HT) {
        frm = wifi_mac_add_htcap(frm, sta);
        frm = wifi_mac_add_htinfo(frm, sta);

        if (!(wifimac->wm_flags_ext & WIFINET_FEXT_COEXT_DISABLE)) {
            frm = wifi_mac_add_obss_scan(frm, sta);
            frm = wifi_mac_add_extcap(frm, sta);
        }
    }

    if (wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie) {
        WIFINET_DPRINTF( AML_DEBUG_XMIT, "add wps ie ie for proberesp, len=%d\n",
            wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length);
        memcpy(frm, wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie,
            wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length);
        frm += wnet_vif->app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length;
    }

    //p2p ie from android
#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_enable && (sta->sta_p2p_ie[0] != NULL))
    {
        if (wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie)
        {
            memcpy(frm, wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie,
                   wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length);
            frm += wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length;
        }

        if (wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie)
        {
            //WIFINET_DPRINTF( WIFINET_MSG_OUTPUT, "add p2p ie for proberesp, len=%d, to mac=%s\n",
            //      vmac->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length, ether_sprintf(arg));
            memcpy(frm, wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie,
                   wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length);
            frm += wnet_vif->vm_p2p->noa_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length;
        }
    }
#endif//#ifdef CONFIG_P2P

#ifdef CONFIG_WFD
    if (/*(sta->sta_wfd_ie!= NULL) &&*/  wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie)
    {
        memcpy(frm, wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].ie,
               wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length);
        frm += wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_PROBE_RESP].length;
        //  DPRINTF(AML_DEBUG_P2P,"%s %d wfd ie\n",__func__,__LINE__);
    }
#endif//#ifdef CONFIG_WFD

    if(wifi_mac_is_vht_enable(wnet_vif))
    {
        frm = wifi_mac_add_vht_cap(frm, sta);
        if (!wnet_vif->vm_p2p_support || (wnet_vif->vm_p2p_support && wnet_vif->vm_curchan != NULL))
        {
            frm = wifi_mac_add_vht_opt(frm, sta, WIFINET_FC0_SUBTYPE_PROBE_RESP);
        }
        //frm = wifi_mac_add_vht_txpw(frm, sta);
        //frm = wifi_mac_add_vht_ch_sw_wrp(frm, sta);

        /*The following capability does not support*/
        //frm = wifi_mac_add_vht_ext_bss_ld(frm, sta);
        //frm = wifi_mac_add_vht_quiet_ch(frm, sta);
        //frm = wifi_mac_add_vht_op_md_ntf(frm, sta);
    }

    for (index = 0; index < VENDOR_IE_MAX; index++) {
        if (wifimac->wm_vendorinfo[index].ie == WIFINET_ELEMID_VENDOR
            && wifimac->wm_vendorinfo[index].len != 0) {
            frm = wifi_mac_add_vendor_ie(frm, wifimac, index);
        }
    }

    os_skb_trim(skb, frm - os_skb_data(skb));
    wifi_mac_mgmt_probe_resp_output(sta, skb, (unsigned char *)arg);
    return 0;
}

int wifi_mac_send_auth(struct wlan_net_vif *wnet_vif,struct wifi_station *sta,void *arg)
{
    struct sk_buff *skb;
    unsigned char *frm;
    int status,has_challenge,is_shared_key;

    status = *(int *)arg >> 16;
    *(int *)arg &= 0xffff;
    has_challenge = ((*(int *)arg == WIFINET_AUTH_SHARED_CHALLENGE ||
        *(int *)arg == WIFINET_AUTH_SHARED_RESPONSE) && (sta->sta_challenge != NULL));
    WIFINET_DPRINTF( AML_DEBUG_CONNECT, "status=%d seq=%d", status, *(int *)arg);

    is_shared_key = has_challenge || *(int *)arg >= WIFINET_AUTH_SHARED_RESPONSE ||
        (*(int *)arg == WIFINET_AUTH_SHARED_REQUEST && wnet_vif->vm_mainsta->sta_authmode == WIFINET_AUTH_SHARED);

    skb = wifi_mac_get_mgmt_frm(wnet_vif->vm_wmac, 3 * sizeof(unsigned short) + sizeof(struct wifi_mac_ie_timeout)
        + ((has_challenge && status == WIFINET_STATUS_SUCCESS) ? sizeof(unsigned short) + WIFINET_CHALLENGE_LEN : 0));

    if (skb == NULL)
    {
        ERROR_DEBUG_OUT("Auth: Unable to allocate mgmt frame\n");
        return ENOMEM;
    }
    frm = os_skb_put(skb, 3 * sizeof(unsigned short)
        + ((has_challenge && status == WIFINET_STATUS_SUCCESS) ? sizeof(unsigned short) + WIFINET_CHALLENGE_LEN : 0));

    ((unsigned short *)frm)[0] =(is_shared_key) ? htole16(WIFINET_AUTH_SHARED) : htole16(WIFINET_AUTH_OPEN);
    ((unsigned short *)frm)[1] = htole16(*(int *)arg);
    ((unsigned short *)frm)[2] = htole16(status);

    if (has_challenge && (status == WIFINET_STATUS_SUCCESS)) {
        ((unsigned short *)frm)[3] = htole16((WIFINET_CHALLENGE_LEN << 8) | WIFINET_ELEMID_CHALLENGE);
        memcpy(&((unsigned short *)frm)[4], sta->sta_challenge, WIFINET_CHALLENGE_LEN);

        if (*(int *)arg == WIFINET_AUTH_SHARED_RESPONSE) {
            struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skb->cb;
            WIFINET_DPRINTF_STA( AML_DEBUG_CONNECT, sta, "request encrypt frame (%s)", __func__);
            cb->flags |= M_LINK0;
        }
    }

    if (status == WIFINET_STATUS_SUCCESS) {
        WIFINET_NODE_STAT(sta, tx_auth);

    } else if (status == WIFINET_STATUS_REFUSED_TEMPORARILY) {
        frm = wifi_mac_add_timeout_interval(frm, sta);
        WIFINET_NODE_STAT(sta, tx_auth_fail);

    } else {
        WIFINET_NODE_STAT(sta, tx_auth_fail);
    }

    wifi_mac_mgmt_output(sta, skb,WIFINET_FC0_SUBTYPE_AUTH);

    if (wnet_vif->vm_opmode == WIFINET_M_STA)
    {
        DPRINTF(AML_DEBUG_TIMER,"TIMERdbg <running> %s %d \n",__func__,__LINE__);
        os_timer_ex_start_period(&wnet_vif->vm_mgtsend, DEFAULT_AUTH_RETRY_INTERVAL);
    }
    return 0;
}

int wifi_mac_send_deauth(struct wlan_net_vif  *wnet_vif, struct wifi_station *sta,void *arg)
{
    struct sk_buff *skb;
    unsigned char *frm;
    unsigned char pkt_len = sizeof(unsigned short) + WIFINET_CIPHER_AES_CMAC;

    WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "send station deauthenticate (reason %d)", *(int *)arg);

    skb = wifi_mac_get_mgmt_frm(wnet_vif->vm_wmac, pkt_len);
    if (skb == NULL) {
        WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "%s: DeAuth: Unable to allocate mgmt frame", __func__);
        return ENOMEM;
    }
    frm = os_skb_put(skb, sizeof(unsigned short));
    *(unsigned short *)frm = htole16(*(int *)arg);

    WIFINET_NODE_STAT(sta, tx_deauth);
    WIFINET_NODE_STAT_SET(sta, tx_deauth_code, *(int *)arg);

    wifi_mac_StationUnauthorize(sta);
    wifi_mac_mgmt_output(sta, skb, WIFINET_FC0_SUBTYPE_DEAUTH);
    return 0;
}

static int aml_rsn_sync_pmkid(struct wifi_station *sta, int pmkid_index)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_Rsnparms *rsn = &sta->sta_rsn;
    unsigned char *frm = wnet_vif->vm_opt_ie;
    unsigned char *rsnie = NULL;
    unsigned char rsn_ie_len = 0, pmkid_pos = 0;
    int idx = 0, rsnie_offset = 0, pmkid_count;

    while (idx < wnet_vif->vm_opt_ie_len) {
        if (frm[idx] == WIFINET_ELEMID_RSN) {
            rsnie = frm + idx;
            rsn_ie_len = frm[idx+IE_LEN_OFFSET];
            pmkid_pos = rsn->rsn_caps_offset + 2 + 2; // 2: rsn_caps_len; 2: pmkid_cnt
            rsnie_offset = rsnie - frm;
            break;
        }
        idx += (frm[idx+IE_LEN_OFFSET] + IE_HDR_LEN);
    }

    if (wnet_vif->vm_opt_ie_len <= 0 || rsnie == NULL) {
        DPRINTF(AML_DEBUG_WARNING, "RSN ie invalid\n");
        return WIFINET_REASON_IE_INVALID;
    }

    if (pmkid_index != -1) {
        if (rsn->rsn_pmkid_count == 0) {
            DPRINTF(AML_DEBUG_WARNING, "no rsn pmkid, need to add\n");
            memcpy(rsnie+pmkid_pos+WPA_PMKID_LEN,rsnie+pmkid_pos,wnet_vif->vm_opt_ie_len-rsnie_offset-pmkid_pos);
            rsnie[rsn->rsn_caps_offset + 2] = 1;
            rsnie[rsn->rsn_caps_offset + 3] = 0;
            rsn->rsn_pmkid_offset = pmkid_pos;
            memcpy(rsnie + pmkid_pos, wnet_vif->pmk_list->pmkid_cache[pmkid_index].pmkid, WPA_PMKID_LEN);
            memcpy(rsn->rsn_pmkid, wnet_vif->pmk_list->pmkid_cache[pmkid_index].pmkid, WPA_PMKID_LEN);
            rsnie[IE_LEN_OFFSET] += WPA_PMKID_LEN;
            wnet_vif->vm_opt_ie_len += WPA_PMKID_LEN;
            rsn->rsn_pmkid_count++;

        } else {
            if (memcmp(rsn->rsn_pmkid, wnet_vif->pmk_list->pmkid_cache[pmkid_index].pmkid, WPA_PMKID_LEN)) {
                DPRINTF(AML_DEBUG_CFG80211, "%s pmkid not the same:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    __func__, rsn->rsn_pmkid[0], rsn->rsn_pmkid[1], rsn->rsn_pmkid[2], rsn->rsn_pmkid[3], rsn->rsn_pmkid[4], rsn->rsn_pmkid[5], rsn->rsn_pmkid[6],
                    rsn->rsn_pmkid[7], rsn->rsn_pmkid[8], rsn->rsn_pmkid[9], rsn->rsn_pmkid[10], rsn->rsn_pmkid[11], rsn->rsn_pmkid[12], rsn->rsn_pmkid[13],
                    rsn->rsn_pmkid[14], rsn->rsn_pmkid[15]);

                memcpy(rsnie+rsn->rsn_pmkid_offset, wnet_vif->pmk_list->pmkid_cache[pmkid_index].pmkid, WPA_PMKID_LEN);
                memcpy(rsn->rsn_pmkid, wnet_vif->pmk_list->pmkid_cache[pmkid_index].pmkid, WPA_PMKID_LEN);
            } else {
                DPRINTF(AML_DEBUG_WARNING, "pmkid the same\n");
            }
        }

    } else {
        if (rsn->rsn_pmkid_count == 0) {
            DPRINTF(AML_DEBUG_WARNING, "no rsn pmkid, just return, not delete\n");
            return 0;
        }
        pmkid_count = rsnie[rsn->rsn_caps_offset + 2] | (rsnie[rsn->rsn_caps_offset + 3] << 8);
        DPRINTF(AML_DEBUG_WARNING, "delete rsn pmkid:%d\n", pmkid_count);

        rsnie[IE_LEN_OFFSET] -= pmkid_count * WPA_PMKID_LEN;
        rsnie[rsn->rsn_caps_offset + 2] = 0;
        rsnie[rsn->rsn_caps_offset + 3] = 0;
        memcpy(rsnie + pmkid_pos, rsnie + pmkid_pos + (WPA_PMKID_LEN * pmkid_count),
            wnet_vif->vm_opt_ie_len - rsnie_offset - pmkid_pos - (WPA_PMKID_LEN * pmkid_count));
        memset(rsn->rsn_pmkid, 0, WPA_PMKID_LEN);
        rsn_ie_len -= (WPA_PMKID_LEN * pmkid_count);
        wnet_vif->vm_opt_ie_len -= (WPA_PMKID_LEN * pmkid_count);
        rsn->rsn_pmkid_count -= pmkid_count;
        if (rsn->rsn_pmkid_count == 0) {
            rsn->rsn_pmkid_offset = 0;

        } else {
            ERROR_DEBUG_OUT("BUG!\n");
        }
    }

    return 0;
}

void wifi_mac_check_opt_ie(struct wlan_net_vif *wnet_vif)
{
    unsigned char *frm = wnet_vif->vm_opt_ie;
    unsigned int i = 0, j = 0;

    while(i < wnet_vif->vm_opt_ie_len) {
        if (frm[i] == WIFINET_ELEMID_SUPP_REG_CLASS) {
            j = i + frm[i+1] + 2;
            if (j < wnet_vif->vm_opt_ie_len) {
                memcpy(&frm[i], &frm[j], wnet_vif->vm_opt_ie_len - j);
            }
            wnet_vif->vm_opt_ie_len -= (frm[i+1] + 2);
            return;
        }
        i += frm[i+1] + 2;
    }
}

int wifi_mac_send_assoc_req(struct wlan_net_vif *wnet_vif, struct wifi_station *sta,int type)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct sk_buff *skb;
    unsigned char *frm;
    unsigned short capinfo;
    int pmkid_index;
    unsigned short pkt_len;

    wifi_mac_check_opt_ie(wnet_vif);

    pkt_len = sizeof(unsigned short) + sizeof(unsigned short) + WIFINET_ADDR_LEN + 2 + WIFINET_NWID_LEN + 2 + WIFINET_RATE_SIZE
        + 2 + (WIFINET_RATE_MAXSIZE - WIFINET_RATE_SIZE) + 4 + 2 + WIFINET_SUPPCHAN_LEN + sizeof(struct wifi_mac_ie_wme)
        + sizeof(struct wifi_mac_ie_htcap) + (wnet_vif->vm_opt_ie_len) + wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length
#ifdef CONFIG_P2P
        + wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length
#ifdef CONFIG_WFD
#ifndef WFA_P2P_TEST
        + wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length
#endif
#endif//#ifdef CONFIG_WFD
#endif//#ifdef CONFIG_P2P
        + sizeof(struct wifi_mac_ie_vht_cap);

    skb = wifi_mac_get_mgmt_frm(wifimac, pkt_len);
    if (skb == NULL) {
        WIFINET_DPRINTF_STA( AML_DEBUG_CONNECT, sta,
            "%s: Assoc/Reassoc Req: Unable to allocate mgmt frame", __func__);
        return ENOMEM;
    }
    frm = os_skb_put(skb, pkt_len);

    capinfo = 0;
    if (wnet_vif->vm_opmode == WIFINET_M_IBSS)
        capinfo |= WIFINET_CAPINFO_IBSS;
    else
        capinfo |= WIFINET_CAPINFO_ESS;

    if (wnet_vif->vm_flags & WIFINET_F_PRIVACY)
        capinfo |= WIFINET_CAPINFO_PRIVACY;

    if ((wifimac->wm_flags & WIFINET_F_SHPREAMBLE) && WIFINET_IS_CHAN_2GHZ(wifimac->wm_curchan))
        capinfo |= WIFINET_CAPINFO_SHORT_PREAMBLE;

    if ((sta->sta_capinfo & WIFINET_CAPINFO_SHORT_SLOTTIME) && (wifimac->wm_caps & WIFINET_C_SHSLOT))
        capinfo |= WIFINET_CAPINFO_SHORT_SLOTTIME;

    if ((sta->sta_capinfo & WIFINET_CAPINFO_SPECTRUM_MGMT))
        capinfo |= WIFINET_CAPINFO_SPECTRUM_MGMT;

    /*9.23.5 LG-SIG protection in 11ac */
    if (wifi_mac_is_vht_enable(wnet_vif)) {
        SET_HT_CAP_INFO_LGSIG_TXOP_PRCT(capinfo, HT_CAP_INFO_NOT_SPT_LGSIG_TXOP_PRCT);
    }

    *(unsigned short *)frm = htole16(capinfo);
    frm += 2;

    *(unsigned short *)frm = sta->sta_listen_intval;//htole16(wnet_if->vm_lintval);
    frm += 2;

    if (type == WIFINET_FC0_SUBTYPE_REASSOC_REQ) {
        WIFINET_ADDR_COPY(frm, wnet_vif->vm_mainsta->sta_bssid);
        frm += WIFINET_ADDR_LEN;
    }

    frm = wifi_mac_add_ssid(frm, sta->sta_essid, sta->sta_esslen);
    frm = wifi_mac_add_rates(frm, &sta->sta_rates);
    if ((wifimac->wm_flags & WIFINET_F_DOTH) && wnet_vif->vm_p2p_support)
        frm = wifi_mac_add_doth(frm, wnet_vif);

    frm = wifi_mac_add_xrates(frm, &sta->sta_rates);
    if ((wnet_vif->vm_flags & WIFINET_F_WME) && sta->sta_wme_ie != NULL) {
        /*just add qos_info in wmm ie */
        frm = wifi_mac_add_wme(frm, sta);
    }

    if (wifi_mac_is_ht_enable(sta)) {
        frm = wifi_mac_add_htcap(frm, sta);
    }

    if (wifi_mac_is_vht_enable(wnet_vif)) {
        frm = wifi_mac_add_vht_cap(frm, sta);
        frm = wifi_mac_add_extcap(frm, sta);
        // not be used so far
        // frm = wifi_mac_add_vht_op_md_ntf(frm, sta);
    }

    if (sta->sta_authmode == WIFINET_AUTH_OPEN) {
        if (sta->sta_rsn.rsn_keymgmtset & (1 << RSN_ASE_8021X_SAE)) {
            //find pmkid and compare, if not same, replace
            pmkid_index = aml_pmkid_cache_index(wnet_vif, sta->sta_bssid);
            aml_rsn_sync_pmkid(sta, pmkid_index);
        }

    } else if (sta->sta_authmode == WIFINET_AUTH_SAE) {
        if (sta->sta_rsn.rsn_keymgmtset & (1 << RSN_ASE_8021X_SAE)) {
            //find pmkid and delete
            aml_rsn_sync_pmkid(sta, -1);
        }
    }

    if (wnet_vif->vm_opt_ie_len) {
        memcpy(frm, wnet_vif->vm_opt_ie, wnet_vif->vm_opt_ie_len);
        frm += wnet_vif->vm_opt_ie_len;
    }

    if (wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].ie) {
        memcpy(frm, wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].ie,
               wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length);
        frm += wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length;
    }

#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_enable &&  wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].ie) {
        memcpy(frm, wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].ie,
               wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length);
        frm += wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length;
    }
#ifdef CONFIG_WFD
#ifndef WFA_P2P_TEST
    if (wnet_vif->vm_p2p->p2p_enable && wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].ie) {
        memcpy(frm, wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].ie,
               wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length);
        frm += wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_REQ].length;
    }
#endif
#endif//#ifdef CONFIG_WFD
#endif//#ifdef CONFIG_P2P

    os_skb_trim(skb, frm - os_skb_data(skb));
    wifi_mac_save_app_ie(&wnet_vif->assocreq_ie, os_skb_data(skb) + 4/*sizeof(capinfo+listen int)*/,os_skb_get_pktlen(skb) - 4);
    wifi_mac_mgmt_output(sta, skb, type);
    DPRINTF(AML_DEBUG_TIMER, "TIMERdbg <running> %s %d \n",__func__,__LINE__);
    os_timer_ex_start_period(&wnet_vif->vm_mgtsend, DEFAULT_MGMT_RETRY_INTERVAL);
    return 0;
}

int wifi_mac_send_assoc_rsp(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, void *arg, int type)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct sk_buff *skb;
    unsigned char *frm;
    unsigned short capinfo;
    unsigned short pkt_len;
    unsigned char  index;

    pkt_len = sizeof(capinfo) + sizeof(unsigned short) + sizeof(unsigned short) + 2 + WIFINET_RATE_SIZE + sizeof(struct wifi_mac_ie_timeout)
        + 2 + (WIFINET_RATE_MAXSIZE - WIFINET_RATE_SIZE) + sizeof(struct wifi_mac_wme_param) + sizeof(struct wifi_mac_ie_htcap)
        + sizeof(struct wifi_mac_ie_htinfo) + sizeof (struct wifi_mac_ie_obss_scan) + sizeof (struct wifi_mac_ie_ext_cap) + WIFINET_APPIE_MAX
        + sizeof(struct wifi_mac_vendor_ie) * VENDOR_IE_MAX;

    skb = wifi_mac_get_mgmt_frm(wifimac, pkt_len);
    if (skb == NULL) {
        WIFINET_DPRINTF_STA(AML_DEBUG_CONNECT, sta, "%s: Assoc/Reassoc Resp: Unable to allocate mgmt frame", __func__);
        return ENOMEM;
    }
    frm = os_skb_put(skb, pkt_len);

    capinfo = WIFINET_CAPINFO_ESS;
    if (wnet_vif->vm_flags & WIFINET_F_PRIVACY)
        capinfo |= WIFINET_CAPINFO_PRIVACY;

    if ((wifimac->wm_flags & WIFINET_F_SHPREAMBLE) && WIFINET_IS_CHAN_2GHZ(wifimac->wm_curchan))
        capinfo |= WIFINET_CAPINFO_SHORT_PREAMBLE;

    if (wifimac->wm_flags & WIFINET_F_SHSLOT)
        capinfo |= WIFINET_CAPINFO_SHORT_SLOTTIME;

    *(unsigned short *)frm = htole16(capinfo);
    frm += 2;

    *(unsigned short *)frm = htole16(*(int *)arg);
    frm += 2;

    if (*(int *)arg == WIFINET_STATUS_SUCCESS) {
        *(unsigned short *)frm = htole16(sta->sta_associd);
        WIFINET_NODE_STAT(sta, tx_assoc);
        wifimac->drv_priv->drv_ops.RegisterStationID(wifimac->drv_priv, wnet_vif->wnet_vif_id, sta->sta_associd,
            sta->sta_macaddr, sta->sta_encrypt_flag);

    } else if (*(int *)arg == WIFINET_STATUS_REFUSED_TEMPORARILY) {
        *(unsigned short *)frm = htole16(sta->sta_associd);
        WIFINET_NODE_STAT(sta, tx_assoc_fail);

    } else {
        WIFINET_NODE_STAT(sta, tx_assoc_fail);
    }
    frm += 2;

    if (*(int *)arg == WIFINET_STATUS_REFUSED_TEMPORARILY) {
        frm = wifi_mac_add_timeout_interval(frm, sta);
    }

    /*add rate ie and extend rate ie*/
    frm = wifi_mac_add_rates(frm, &sta->sta_rates);
    frm = wifi_mac_add_xrates(frm, &sta->sta_rates);

    if ((wnet_vif->vm_flags & WIFINET_F_WME) && sta->sta_wme_ie != NULL) {
        frm = wifi_mac_add_wme_param(frm, &wifimac->wm_wme[wnet_vif->wnet_vif_id],
            WIFINET_VMAC_UAPSD_ENABLED(wnet_vif));
    }

    pr_debug("%s %d sta flags: 0x%x, assoc success :%d.\n",__func__,__LINE__,sta->sta_flags, *(int *)arg);
    if (sta->sta_flags & WIFINET_NODE_HT) {
        pr_debug("%s %d will add ht cat and op for assocresp\n",__func__,__LINE__);
        frm = wifi_mac_add_htcap(frm, sta);
        frm = wifi_mac_add_htinfo(frm, sta);

        if (!(wifimac->wm_flags_ext & WIFINET_FEXT_COEXT_DISABLE)) {
            frm = wifi_mac_add_obss_scan(frm, sta);
            frm = wifi_mac_add_extcap(frm, sta);
        }
    }

    if (wifi_mac_is_vht_enable(wnet_vif) && (sta->sta_flags & WIFINET_NODE_VHT)) {
        pr_debug("%s %d will add vht cat and op for assocresp\n",__func__,__LINE__);
        frm = wifi_mac_add_vht_cap( frm, sta) ;
        frm = wifi_mac_add_vht_opt( frm, sta , WIFINET_FC0_SUBTYPE_ASSOC_RESP);
    }

    /*app_ie save wps_ie for p2p */
    if (wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].ie) {
        memcpy(frm, wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].ie,
            wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].length);
        frm += wnet_vif->app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].length;
    }

    //p2p ie from android
#ifdef CONFIG_P2P
    if (wnet_vif->vm_p2p->p2p_enable && (sta->sta_p2p_ie[0] != NULL)
        &&  wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].ie)
    {
        AML_PRINT(AML_DBG_MODULES_P2P,"add p2p ie for assocresp\n");
        memcpy(frm, wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].ie,
               wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].length);
        frm += wnet_vif->vm_p2p->p2p_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].length;
    }
#endif//#ifdef CONFIG_P2P
#ifdef CONFIG_WFD
    if (wnet_vif->vm_p2p->p2p_enable
        /*  &&  vmac->vm_p2p->wfd_info.wfd_enable*/
        && wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].ie)
    {
        memcpy(frm, wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].ie,
               wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].length);
        frm += wnet_vif->vm_p2p->wfd_app_ie[WIFINET_APPIE_FRAME_ASSOC_RESP].length;
    }
#endif//#ifdef CONFIG_WFD

    for (index = 0; index < VENDOR_IE_MAX; index++) {
        if (wifimac->wm_vendorinfo[index].ie == WIFINET_ELEMID_VENDOR
            && wifimac->wm_vendorinfo[index].len != 0) {
            frm = wifi_mac_add_vendor_ie(frm, wifimac, index);
        }
    }

    os_skb_trim(skb, frm - os_skb_data(skb));
    wifi_mac_mgmt_output(sta, skb, type);
    return 0;
}

int wifi_mac_send_disassoc(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, void *arg)
{
    struct sk_buff *skb;
    unsigned char *frm;
    unsigned char pkt_len = sizeof(unsigned short) + MME_IE_LENGTH_CMAC;

    WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "send station disassociate (reason %d)", *(int *)arg);

    skb = wifi_mac_get_mgmt_frm(wnet_vif->vm_wmac, pkt_len);
    if (skb == NULL) {
        WIFINET_DPRINTF_STA(AML_DEBUG_WARNING, sta, "%s: disassoc: Unable to allocate mgmt frame", __func__);
        return ENOMEM;
    }
    frm = os_skb_put(skb, sizeof(unsigned short));
    *(unsigned short *)frm = htole16(*(int *)arg); /* reason */

    WIFINET_NODE_STAT(sta, tx_disassoc);
    WIFINET_NODE_STAT_SET(sta, tx_disassoc_code, *(int *)arg);

    wifi_mac_mgmt_output(sta, skb, WIFINET_FC0_SUBTYPE_DISASSOC);
    return 0;
}

static void wifi_mac_get_bsscoexist_channel(unsigned char *chan_list, unsigned char *num_channels, unsigned char *num_intol)
{
    struct wifi_mac *wifi_mac = wifi_mac_get_mac_handle();
    struct wifi_mac_scan_state *ss = wifi_mac->wm_scan;
    struct scaninfo_table *st = ss->ScanTablePriv;
    struct scaninfo_entry *se;
    unsigned char *ht_ie;
    unsigned int ht_cap = 0;
    unsigned short channel = 0;
    int i = 0, chan_count = 0, intol_count = 0;

    WIFI_SCAN_SE_LIST_LOCK(st);
    list_for_each_entry(se,&st->st_entry,se_list) {
        DPRINTF(AML_DEBUG_SCAN, "<running> %s %d se%p st_entry%p \n",__func__,__LINE__,se, &st->st_entry);
        channel = se->scaninfo.SI_chan->chan_pri_num;
        if (channel > 14) {
            continue;
        }
        ht_ie = se->scaninfo.SI_htcap_ie;
        if (ht_ie && ht_ie[1] == 26) {
            ht_cap = ht_ie[2] | (ht_ie[3] << 8);
        }

        if (!ht_cap || (ht_cap & WIFINET_HTCAP_40M_INTOLERANT)) {
            if (ht_cap & WIFINET_HTCAP_40M_INTOLERANT) {
                intol_count++;
            }
            /* Check whether the channel is already considered */
            for (i = 0; i < chan_count; i++) {
                if (channel == chan_list[i]) {
                    break;
                }
            }
            if (i != chan_count) {
                continue;
            }
            chan_list[chan_count++] = channel;
        }

    }
    WIFI_SCAN_SE_LIST_UNLOCK(st);

    *num_channels = chan_count;
    *num_intol = intol_count;
}

static void wifi_mac_build_wmm_tspec_ie(struct wlan_net_vif *wnet_vif, struct wifi_mac_wmm_tspec_element *ie)
{
    struct wifi_mac_wmm_tspec_element *tspec = ie;
    char uapsd = 0;
    if (strcmp(wnet_vif->vm_wmm_ac_params.psb, "UAPSD") == 0) {
        uapsd = 1;
    }
    tspec->eid = WIFINET_ELEMID_VENDOR;
    tspec->length = sizeof(*tspec) - 2; /* reduce eid and length */
    tspec->oui[0] = 0x00;
    tspec->oui[1] = 0x50;
    tspec->oui[2] = 0xf2;
    tspec->oui_type = WMM_OUI_TYPE;
    tspec->oui_subtype = WMM_OUI_SUBTYPE_TSPEC_ELEMENT;
    tspec->version = WMM_VERSION;
    tspec->ts_info[0] = wnet_vif->vm_wmm_ac_params.tid << 1;
    tspec->ts_info[0] |= wnet_vif->vm_wmm_ac_params.direction << 5;
    tspec->ts_info[0] |= WMM_AC_ACCESS_POLICY_EDCA << 7;
    tspec->ts_info[1] = uapsd << 2;
    tspec->ts_info[1] |= wnet_vif->vm_wmm_ac_params.up << 3;
    tspec->ts_info[2] = 0;

    tspec->nominal_msdu_size = wnet_vif->vm_wmm_ac_params.size;
    if (strcmp(wnet_vif->vm_wmm_ac_params.fixed, "true") == 0) {
        tspec->nominal_msdu_size |= WMM_AC_FIXED_MSDU_SIZE;
    }
    tspec->mean_data_rate = wnet_vif->vm_wmm_ac_params.mean_data_rate;
    tspec->minimum_phy_rate = wnet_vif->vm_wmm_ac_params.phyrate;
    tspec->surplus_bandwidth_allowance = wnet_vif->vm_wmm_ac_params.sba;

}

static int wifi_mac_up_to_ac(struct wlan_net_vif *wnet_vif)
{
    int ac = 0;
    if (wnet_vif->vm_wmm_ac_params.up == 0 || wnet_vif->vm_wmm_ac_params.up == 1) {
        ac = WME_AC_BE;
    } else if (wnet_vif->vm_wmm_ac_params.up == 2 || wnet_vif->vm_wmm_ac_params.up == 3) {
        ac = WME_AC_BK;
    } else if (wnet_vif->vm_wmm_ac_params.up == 4 || wnet_vif->vm_wmm_ac_params.up == 5) {
        ac = WME_AC_VI;
    } else if (wnet_vif->vm_wmm_ac_params.up == 6 || wnet_vif->vm_wmm_ac_params.up == 7) {
        ac = WME_AC_VO;
    }
    return ac;
}

static unsigned char  wmm_ac_get_tid(const struct wifi_mac_wmm_tspec_element tspec)
{
    return (tspec.ts_info[0] >> 1) & 0x0f;
}


static unsigned char wmm_ac_find_tid(struct wlan_net_vif *wnet_vif, unsigned char tid, unsigned char *dir)
{
    unsigned char ac;
    unsigned char idx;
    for (ac = 0; ac < WME_AC_NUM; ac++) {
        for (idx = 0; idx < TS_DIR_IDX_COUNT; idx++) {
            if (wmm_ac_get_tid(wnet_vif->tspecs[ac][idx]) == tid) {
                if (dir) {
                    *dir = idx;
                }
                return ac;
            }
        }
    }

    return -1;
}

int wifi_mac_send_actionframe(struct wlan_net_vif *wnet_vif, struct wifi_station *sta, void *arg)
{
    struct sk_buff *skb = NULL;
    unsigned char *frm;
    struct wifi_mac_action_ht_txchwidth *txchwidth=NULL;
    struct wifi_mac_action_ba_addbarequest *addbarequest=NULL;
    struct wifi_mac_action_ba_addbaresponse *addbaresponse=NULL;
    struct wifi_mac_action_ba_delba *delba=NULL;
    struct wifi_mac_ba_parameterset baparamset;
    struct wifi_mac_ba_seqctrl basequencectrl;
    struct wifi_mac_action_mgt_args *actionargs=NULL;
    struct wifi_mac_delba_parameterset delbaparamset;
    struct wifi_mac_action_bss_coex_frame *bsscoex = NULL;
    struct wifi_mac_action_addts_req_frame *addts_req = NULL;
    struct wifi_mac_action_delts_req_frame *delts_req = NULL;
    struct wifi_mac_wmm_tspec_element wmm_tspec;
    struct wifi_mac_action_sa_query *sa_query = NULL;
    struct drv_rx_scoreboard *RxTidState;
    unsigned short batimeout;
    unsigned char category, action, tid_index;
    unsigned short statuscode;
    unsigned short reasoncode;
    unsigned short buffersize;
    unsigned char chan_list[P2P_MAX_CHANNELS];
    unsigned char num_channels = 0, num_intol = 0,ac = 0,dir = 0;
    struct wifi_mac *wifi_mac = wifi_mac_get_mac_handle();
    int result = 0;

    actionargs = (struct wifi_mac_action_mgt_args *)arg;
    category = actionargs->category;
    action = actionargs->action;

    skb = wifi_mac_get_mgmt_frm(wifi_mac, MAX_ACTION_LEN);
    if (skb == NULL) {
        wnet_vif->vif_sts.sts_tx_no_buf++;
        return -ENOMEM;
    }

    switch (category)
    {
        case AML_CATEGORY_QOS:
            WIFINET_DPRINTF_STA(AML_DEBUG_ACTION, sta, "%s: QoS action mgt frames not supported", __func__);
            wnet_vif->vif_sts.sts_tx_unsprt_mngt++;
            result = EINVAL;
            break;

        case AML_CATEGORY_BACK:
            tid_index = actionargs->arg1;
            switch (action)
            {
                case WIFINET_ACTION_BA_ADDBA_REQUEST:
                    frm = os_skb_put(skb, sizeof(struct wifi_mac_action_ba_addbarequest));

                    DPRINTF(AML_DEBUG_ADDBA,"<running> %s %d addba req rq_dialogtoken =%d \n",__func__,__LINE__,tid_index + 1);
                    addbarequest = (struct wifi_mac_action_ba_addbarequest *)frm;
                    addbarequest->rq_header.ia_category = AML_CATEGORY_BACK;
                    addbarequest->rq_header.ia_action = action;
                    addbarequest->rq_dialogtoken = 9;
                    buffersize = actionargs->arg2;
                    wifi_mac_addba_req_setup(sta, tid_index, &baparamset, &batimeout, &basequencectrl, buffersize);

                    *(unsigned short *)&addbarequest->rq_baparamset = htole16(*(unsigned short *)&baparamset);
                    addbarequest->rq_batimeout = htole16(batimeout);
                    *(unsigned short *)&addbarequest->rq_basequencectrl = htole16(*(unsigned short *)&basequencectrl);
                    DPRINTF(AML_DEBUG_ADDBA, "%s: ADDBA request action mgt frame. TID %d, buffer size %d tid %d rq_baparamset tid %d  \n",
                            __func__, tid_index, baparamset.buffersize,baparamset.tid,addbarequest->rq_baparamset.tid);
                    break;

                case WIFINET_ACTION_BA_ADDBA_RESPONSE:
                    if (tid_index < WME_NUM_TID) {
                        frm = os_skb_put(skb, sizeof(struct wifi_mac_action_ba_addbaresponse));

                        addbaresponse = (struct wifi_mac_action_ba_addbaresponse *)frm;
                        addbaresponse->rs_header.ia_category = AML_CATEGORY_BACK;
                        addbaresponse->rs_header.ia_action = action;
                        RxTidState  = &((struct aml_driver_nsta *)sta->drv_sta)->rx_scb[tid_index];

                        addbaresponse->rs_dialogtoken = RxTidState->dialogtoken;
                        statuscode = RxTidState->statuscode;
                        baparamset = RxTidState->baparamset;
                        batimeout = RxTidState->batimeout;

                        wifi_mac_addba_rsp_setup(sta, tid_index);
                        *(unsigned short *)&addbaresponse->rs_baparamset = htole16(*(unsigned short *)&baparamset);
                        //dump_memory_internal(&baparamset,sizeof(struct wifi_mac_ba_parameterset));
                        addbaresponse->rs_batimeout = htole16(batimeout);
                        addbaresponse->rs_statuscode = htole16(statuscode);

                        if (wifi_mac->wm_bt_en) {
                            if (wifi_mac->wm_esco_en) {
                                addbaresponse->rs_baparamset.buffersize = DEFAULT_TXAMPDU_SUB_MAX_COEX_ESCO;

                            } else {
                                addbaresponse->rs_baparamset.buffersize = DEFAULT_TXAMPDU_SUB_MAX_COEX;
                            }
                        }

                        //AMSDU is supported on the opposite end
                        if (actionargs->arg2) {
                            addbaresponse->rs_baparamset.buffersize = 16;
                            DPRINTF(AML_DEBUG_ADDBA, "%s: ADDBA response. opposite support AMSDU, reduce buffer size to %d\n",
                                __func__, addbaresponse->rs_baparamset.buffersize);
                        }

                        DPRINTF(AML_DEBUG_WARNING, "%s: ADDBA response action mgt frame. TID %d, buffer size %d, status %d \n",
                            __func__, tid_index, baparamset.buffersize, statuscode);
                    } else {
                        result = EINVAL;
                        ERROR_DEBUG_OUT("ADDBA response TID %d exceeds the max value %d\n", tid_index, WME_NUM_TID);
                    }
                    break;

                case WIFINET_ACTION_BA_DELBA:
                    frm = os_skb_put(skb, sizeof(struct wifi_mac_action_ba_delba));
                    delba = (struct wifi_mac_action_ba_delba *)frm;
                    delba->dl_header.ia_category = AML_CATEGORY_BACK;
                    delba->dl_header.ia_action = action;

                    delbaparamset.reserved0 = 0;
                    delbaparamset.initiator = actionargs->arg2;
                    delbaparamset.tid = tid_index;
                    reasoncode = actionargs->arg3;
                    *(unsigned short *)&delba->dl_delbaparamset = htole16(*(unsigned short *)&delbaparamset);
                    delba->dl_reasoncode = htole16(reasoncode);
                    DPRINTF(AML_DEBUG_WARNING, "%s: DELBA action mgt frame. TID %d, initiator %d, reason %d\n",
                            __func__, tid_index, delbaparamset.initiator, reasoncode);
                    break;

                default:
                    WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta, "%s: invalid BA action mgt frame", __func__);
                    wnet_vif->vif_sts.sts_tx_unsprt_mngt++;
                    result = EINVAL;
                    break;
            }
            break;

        case AML_CATEGORY_HT:
            switch (action)
            {
                case WIFINET_ACTION_HT_TXCHWIDTH:
                    frm = os_skb_put(skb, sizeof(struct wifi_mac_action_ht_txchwidth));
                    WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta, "%s: HT txchwidth action mgt frame. Width %d",
                        __func__, wnet_vif->vm_bandwidth);
                    txchwidth = (struct wifi_mac_action_ht_txchwidth *)frm;
                    txchwidth->at_header.ia_category = AML_CATEGORY_HT;
                    txchwidth->at_header.ia_action = WIFINET_ACTION_HT_TXCHWIDTH;
                    txchwidth->at_chwidth =  (wnet_vif->vm_bandwidth == WIFINET_BWC_WIDTH40) ?
                                             WIFINET_A_HT_TXCHWIDTH_2040 : WIFINET_A_HT_TXCHWIDTH_20;
                    break;

                case WIFINET_ACTION_HT_SMPOWERSAVE:
                    WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta, "%s: HT SM pwr save action mgt frame not implemented", __func__);
                    wnet_vif->vif_sts.sts_tx_unsprt_mngt++;
                    result = EINVAL;
                    break;

                default:
                    WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta, "%s: invalid HT action mgt frame", __func__);
                    wnet_vif->vif_sts.sts_tx_unsprt_mngt++;
                    result = EINVAL;
                    break;
            }
            break;

        case AML_CATEGORY_PUBLIC:
            switch (action) {
                case WIFINET_ACT_PUBLIC_BSSCOEXIST:
                    wifi_mac_get_bsscoexist_channel(chan_list, &num_channels, &num_intol);
                    pr_debug("%s:%d, num_channels=%d, num_intol=%d.\n", __func__, __LINE__, num_channels, num_intol);

                    frm = os_skb_put(skb, sizeof(struct wifi_mac_action_bss_coex_frame) + num_channels);
                    bsscoex = (struct wifi_mac_action_bss_coex_frame *)frm;
                    bsscoex->ac_header.ia_category = AML_CATEGORY_PUBLIC;
                    bsscoex->ac_header.ia_action   = WIFINET_ACT_PUBLIC_BSSCOEXIST;
                    bsscoex->coex.elem_id = WIFINET_ELEMID_2040_COEXT;
                    bsscoex->coex.elem_len = 1;
                    if (num_intol) {
                        bsscoex->coex.ht20_width_req = 1;
                    }
                    bsscoex->chan_report.elem_id = WIFINET_ELEMID_2040_INTOL;
                    bsscoex->chan_report.elem_len = num_channels + 1;
                    bsscoex->chan_report.reg_class = 0;
                    if (num_channels) {
                        memcpy(bsscoex->chan_report.chan_list, chan_list, num_channels);
                    }
                    break;

               default:
                   WIFINET_DPRINTF_STA( AML_DEBUG_ACTION, sta, "%s: invalid public action mgt frame", __func__);
                   result = EINVAL;
                   break;
            }
            break;

        case AML_CATEGORY_SA_QUERY:
            frm = os_skb_put(skb, sizeof(struct wifi_mac_action_sa_query));
            sa_query = (struct wifi_mac_action_sa_query *)frm;
            sa_query->sa_header.ia_category = AML_CATEGORY_SA_QUERY;
            sa_query->sa_header.ia_action = action;
            sa_query->sa_transaction_identifier = actionargs->arg1;
            break;

        case AML_CATEGORY_WMM:
            switch (action) {
                case WIFINET_ACTION_ADDTS_REQ:
                    frm = os_skb_put(skb, sizeof(struct wifi_mac_action_addts_req_frame));
                    addts_req = (struct wifi_mac_action_addts_req_frame *)frm;
                    addts_req->ac_header.ia_category = AML_CATEGORY_WMM;
                    addts_req->ac_header.ia_action = WIFINET_ACTION_ADDTS_REQ;
                    addts_req->dialog_token = wnet_vif->vm_wmm_ac_params.dialog_token;
                    addts_req->status_code = 0;
                    wifi_mac_build_wmm_tspec_ie(wnet_vif, &wmm_tspec);
                    memcpy(&addts_req->wmm_tspec_element, &wmm_tspec, sizeof(struct wifi_mac_wmm_tspec_element));
                    ac = wifi_mac_up_to_ac(wnet_vif);
                    memcpy(&wnet_vif->tspecs[ac][wnet_vif->vm_wmm_ac_params.direction], &wmm_tspec, sizeof(struct wifi_mac_wmm_tspec_element));
                    break;

                case WIFINET_ACTION_DELTS:
                    frm = os_skb_put(skb, sizeof(struct wifi_mac_action_delts_req_frame));
                    tid_index = actionargs->arg1;
                    delts_req = (struct wifi_mac_action_delts_req_frame *)frm;
                    delts_req->ac_header.ia_category = AML_CATEGORY_WMM;
                    delts_req->ac_header.ia_action = WIFINET_ACTION_DELTS;
                    delts_req->dialog_token = wnet_vif->vm_wmm_ac_params.dialog_token;
                    delts_req->status_code = 0;
                    ac = wmm_ac_find_tid(wnet_vif, tid_index, &dir);
                    if(ac < WME_AC_NUM) {
                        memcpy(&delts_req->wmm_tspec_element, &wnet_vif->tspecs[ac][dir], sizeof(struct wifi_mac_wmm_tspec_element));
                        memset(&wnet_vif->tspecs[ac][dir], 0, sizeof(struct wifi_mac_wmm_tspec_element));
                    }
                    break;

                default:
                    WIFINET_DPRINTF_STA(AML_DEBUG_ACTION, sta, "%s: invalid public action mgt frame", __func__);
                    result = EINVAL;
                    break;
            }
        break;

        default:
            WIFINET_DPRINTF_STA(AML_DEBUG_ACTION, sta, "%s: action mgt frame has invalid category %d", __func__, category);
            wnet_vif->vif_sts.sts_tx_unsprt_mngt++;
            result = EINVAL;
            break;
    }

    if (result) {
        os_skb_free(skb);

    } else {
        wifi_mac_mgmt_output(sta, skb, WIFINET_FC0_SUBTYPE_ACTION);
    }

    return result;
}

int wifi_mac_send_mgmt(struct wifi_station *sta, int type, void *arg)
{
    struct wlan_net_vif *wnet_vif;
    int ret;
    unsigned char retry_count;

    KASSERT(sta != NULL, ("null nsta"));

    wnet_vif = sta->sta_wnet_vif;

    if (wnet_vif->vm_curchan == NULL && !wnet_vif->vm_p2p_support) {
        ERROR_DEBUG_OUT("vm_curchan is NULL, just return\n");
        ret = EINVAL;
        return ret;
    }

    switch (type) {
        case WIFINET_FC0_SUBTYPE_PROBE_RESP:
            ret = wifi_mac_send_probe_rsp(wnet_vif,sta,arg);
            break;

        case WIFINET_FC0_SUBTYPE_AUTH:
            ret = wifi_mac_send_auth(wnet_vif,sta,arg);
            break;

        case WIFINET_FC0_SUBTYPE_DEAUTH:
            if (sta->sta_flags_ext & WIFINET_NODE_MFP) {
                for (retry_count = 0; retry_count < 2; ++retry_count) {
                    ret = wifi_mac_send_deauth(wnet_vif,sta,arg);
                }
            } else {
                ret = wifi_mac_send_deauth(wnet_vif,sta,arg);
            }
            break;

        case WIFINET_FC0_SUBTYPE_ASSOC_REQ:
        case WIFINET_FC0_SUBTYPE_REASSOC_REQ:
            ret = wifi_mac_send_assoc_req(wnet_vif,sta,type);
            break;

        case WIFINET_FC0_SUBTYPE_ASSOC_RESP:
        case WIFINET_FC0_SUBTYPE_REASSOC_RESP:
            ret = wifi_mac_send_assoc_rsp(wnet_vif,sta,arg,type);
            break;

        case WIFINET_FC0_SUBTYPE_DISASSOC:
            if (sta->sta_flags_ext & WIFINET_NODE_MFP) {
                for (retry_count = 0; retry_count < 2; ++retry_count) {
                    ret = wifi_mac_send_disassoc(wnet_vif,sta,arg);
                }
            } else {
                ret = wifi_mac_send_disassoc(wnet_vif,sta,arg);
            }
            break;

        case WIFINET_FC0_SUBTYPE_ACTION:
            ret = wifi_mac_send_actionframe(wnet_vif,sta,arg);
            break;

        default:
            WIFINET_DPRINTF_STA( AML_DEBUG_ANY, sta, "invalid mgmt frame type %u", type);
            ret = EINVAL;
            wnet_vif->vif_sts.sts_tx_unsprt_mngt++;
            break;
    }

    return ret;
}

int wifi_mac_send_arp_req(struct wlan_net_vif *wnet_vif) {
    struct sk_buff *skb = NULL;
    struct ether_header *eh = NULL;
    wifi_arp_pkt *arp = NULL;
    unsigned char pkt_len = 42;
    struct wifi_station *sta = wnet_vif->vm_mainsta;
    struct in_device *in_dev;
    struct in_ifaddr *ifa_v4;
    __be32 ipv4;
    __be32 *sip;
    unsigned char i = 0;

    if (wnet_vif->vm_state != WIFINET_S_CONNECTED) {
        ERROR_DEBUG_OUT("not connected\n");
        return 0;
    }

    /* get ipv4 addr */
    in_dev = __in_dev_get_rtnl(wnet_vif->vm_ndev);
    if (!in_dev) {
        ERROR_DEBUG_OUT("get own ip dev error\n");
        return 0;
    }

    ifa_v4 = in_dev->ifa_list;
    if (!ifa_v4) {
        ERROR_DEBUG_OUT("get own ip list error\n");
        return 0;
    }
    memset(&ipv4, 0, sizeof(ipv4));
    ipv4 = ifa_v4->ifa_local;

    pr_debug("wifi_mac_send_arp_req, ipv4:%08x\n", ipv4);
    for (i = 1; i < 15; ++i) {
        skb = os_skb_alloc(pkt_len + 64 + HI_TXDESC_DATAOFFSET);
        if (skb == NULL) {
            ERROR_DEBUG_OUT("fail to alloc frm\n");
            return -ENOMEM;
        }
        skb_reserve(skb, HI_TXDESC_DATAOFFSET + 32);
        eh = (struct ether_header *)(os_skb_put(skb, pkt_len));

        memcpy(eh->ether_dhost, sta->sta_bssid, WIFINET_ADDR_LEN);
        memcpy(eh->ether_shost, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
        eh->ether_type = 0x0608;

        arp = (wifi_arp_pkt *)((unsigned char *)eh + 14);
        arp->hw_type = __constant_htons(ETHERNET_HW_TYPE);
        arp->proto_type = 0x0008;
        arp->hw_len = ARP_HW_ADDR_LEN;
        arp->proto_len = ARP_PROTO_ADDR_LEN;
        arp->op = __constant_htons(ARP_REQUEST);
        memcpy(arp->src_mac_addr, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
        sip = (__be32 *)(arp->src_ip_addr);
        *sip = ipv4;
        memset(arp->dst_mac_addr, 0, WIFINET_ADDR_LEN);
        memcpy(arp->dst_ip_addr, arp->src_ip_addr, IPV4_LEN);
        arp->dst_ip_addr[3] = i;
        wifi_mac_hardstart(skb, wnet_vif->vm_ndev);
    }

    return 1;
}

static int wifi_mac_send_udp_pkt(struct wlan_net_vif *wnet_vif) {
    struct sk_buff *skb = NULL;
    struct ether_header *eh = NULL;
    wifi_ip_header *ip_hdr = NULL;
    wifi_udp_pkt *udp_pkt = NULL;
    unsigned int pkt_len = 0;
    struct wifi_station *sta = wnet_vif->vm_mainsta;
    unsigned char mcast_dst[WIFINET_ADDR_LEN] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x05 };
    //unsigned char marvell_dst[WIFINET_ADDR_LEN] = { 0x00, 0x50, 0x43, 0x22, 0x33, 0x6e};
    unsigned char intel_dst[WIFINET_ADDR_LEN] = { 0xac, 0xfd, 0xce, 0x8a, 0x78, 0x7f};
    int i = 0;
    if (wnet_vif->vm_state != WIFINET_S_CONNECTED) {
        ERROR_DEBUG_OUT("not connected\n");
        return 0;
    }
    for (i = 0; i < udp_cnt; i++) {
        pkt_len = aml_udp_info[i].pkt_len + 14 + 20 + 8;
        skb = os_skb_alloc(pkt_len + 64 + HI_TXDESC_DATAOFFSET);
        if (skb == NULL) {
            ERROR_DEBUG_OUT("fail to alloc frm\n");
            return -ENOMEM;
        }
        skb_reserve(skb, HI_TXDESC_DATAOFFSET + 32);
        eh = (struct ether_header *)(os_skb_put(skb, pkt_len));

        if (aml_udp_info[i].dst_ip == 0xe0000005) {
            memcpy(eh->ether_dhost, mcast_dst, WIFINET_ADDR_LEN);

        } else if (wnet_vif->vm_opmode == WIFINET_M_STA) {
            memcpy(eh->ether_dhost, sta->sta_bssid, WIFINET_ADDR_LEN);

        } else {
            memcpy(eh->ether_dhost, intel_dst, WIFINET_ADDR_LEN);
        }
        memcpy(eh->ether_shost, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
        eh->ether_type = 0x0008;

        ip_hdr = (wifi_ip_header *)((unsigned char *)eh + 14);
        ip_hdr->version = 0x45;
        ip_hdr->tos = 0x00;
        ip_hdr->tot_len = __constant_htons(aml_udp_info[i].pkt_len + 20 + 8);
        ip_hdr->id = aml_udp_info[i].seq++;
        ip_hdr->frag_off = 0x0040;
        ip_hdr->ttl = 0x40;
        ip_hdr->protocol = 0x11;
        ip_hdr->saddr = __constant_htonl(aml_udp_info[i].src_ip);
        ip_hdr->daddr = __constant_htonl(aml_udp_info[i].dst_ip);

        udp_pkt = (wifi_udp_pkt *)((unsigned char *)ip_hdr + 20);
        udp_pkt->src_port = __constant_htons(aml_udp_info[i].src_port);
        udp_pkt->dst_port = __constant_htons(aml_udp_info[i].dst_port);
        udp_pkt->len =  __constant_htons(aml_udp_info[i].pkt_len + 8);
        udp_pkt->checksum = 0;

        wifi_mac_hardstart(skb, wnet_vif->vm_ndev);
    }
    return 0;
}
extern unsigned short
wifi_mac_eth_type_trans(struct sk_buff *skb, struct net_device *dev);

int wifi_mac_set_arp_rsp(struct wlan_net_vif *wnet_vif) {
    struct sk_buff *skb = NULL;
    struct ether_header *eh = NULL;
    wifi_arp_pkt *arp = NULL;
    unsigned char pkt_len = 42;
    struct in_device *in_dev;
    struct in_ifaddr *ifa_v4;
    __be32 ipv4;
    __be32 *dip;
    unsigned char i = 0;
    unsigned char addr[6] = {0, 0, 0, 0, 0, 0};
    struct net_device *dev = wnet_vif->vm_ndev;

    if (wnet_vif->vm_state != WIFINET_S_CONNECTED) {
        ERROR_DEBUG_OUT("not connected\n");
        return 0;
    }

    /* get ipv4 addr */
    in_dev = __in_dev_get_rtnl(wnet_vif->vm_ndev);
    if (!in_dev) {
        ERROR_DEBUG_OUT("get own ip dev error\n");
        return 0;
    }

    ifa_v4 = in_dev->ifa_list;
    if (!ifa_v4) {
        ERROR_DEBUG_OUT("get own ip list error\n");
        return 0;
    }
    memset(&ipv4, 0, sizeof(ipv4));
    ipv4 = ifa_v4->ifa_local;

    pr_debug("wifi_mac_set_arp_rsp rx, ipv4:%08x\n", ipv4);
    for (i = 1; i < 3; ++i) {
        skb = os_skb_alloc(pkt_len);
        if (skb == NULL) {
            ERROR_DEBUG_OUT("fail to alloc frm\n");
            return -ENOMEM;
        }

        eh = (struct ether_header *)(os_skb_put(skb, pkt_len));

        memcpy(eh->ether_dhost, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
        memcpy(eh->ether_shost, wnet_vif->vm_arp_rx.src_mac_addr, WIFINET_ADDR_LEN);
        eh->ether_type = 0x0608;

        arp = (wifi_arp_pkt *)((unsigned char *)eh + 14);
        arp->hw_type = __constant_htons(ETHERNET_HW_TYPE);
        arp->proto_type = 0x0008;
        arp->hw_len = ARP_HW_ADDR_LEN;
        arp->proto_len = ARP_PROTO_ADDR_LEN;
        if (wnet_vif->vm_arp_rx.out == 0) {
            arp->op = __constant_htons(ARP_REQUEST);
            memcpy(arp->dst_mac_addr, addr, WIFINET_ADDR_LEN);
        } else if (wnet_vif->vm_arp_rx.out == 1) {
            arp->op = __constant_htons(ARP_RESPONSE);
            memcpy(arp->dst_mac_addr, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
        }
        memcpy(arp->src_mac_addr, wnet_vif->vm_arp_rx.src_mac_addr, WIFINET_ADDR_LEN);
        memcpy(arp->src_ip_addr, wnet_vif->vm_arp_rx.src_ip_addr, IPV4_LEN);
        dip = (__be32 *)(arp->dst_ip_addr);
        *dip = ipv4;

        skb->ip_summed = CHECKSUM_UNNECESSARY;
        if (!ALIGNED_POINTER(os_skb_data(skb) + sizeof(*eh), unsigned int)) {
            struct sk_buff *n;
            n = os_skb_copy(skb, GFP_ATOMIC, n);
            os_skb_free(skb);
            if (n == NULL)
               return 0;
            skb = n;
            eh = (struct ether_header *) os_skb_data(skb);
        }


        skb->dev = dev;
        skb->protocol = wifi_mac_eth_type_trans(skb, dev);

        netif_rx(skb);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
        dev->last_rx = jiffies;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0) */

    }

    return 1;
}


int wifi_mac_udp_send_timeout_ex(void *arg)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)arg;
    unsigned char i;

    for (i = 0; i < 1; ++i) {
        wifi_mac_send_udp_pkt(wnet_vif);
    }

    if (!aml_udp_timer.udp_timer_stop) {
        os_timer_ex_start(&aml_udp_timer.udp_send_timeout);
    }
    return OS_TIMER_NOT_REARMED;
}

void
wifi_mac_notify_queue_status(struct wifi_mac *wifimac, unsigned int qqcnt)
{
    if (qqcnt == 0)
        wifi_mac_pwrsave_notify_txq_empty(wifimac);
}

void wifi_mac_complete_wbuf(struct sk_buff *skbbuf, int errcode)
{
    struct wifi_station *sta = NULL;

    if (skbbuf != NULL)
        sta = os_skb_get_nsta(skbbuf);
    else
        return;

    if (errcode == 0) {
        wifi_mac_free_skb(skbbuf);
    }
}

unsigned char *
wifi_mac_add_wide_bandwidth_subie(unsigned char *frm,struct wifi_station *sta)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_channel *switch_chan = &(wnet_vif->csa_target.switch_chan);

    if (WIFINET_IS_CHAN_ERR(switch_chan)) {
        ERROR_DEBUG_OUT("NULL chan\n");
        return frm;
    }

    *frm++ = WIFINET_ELEMID_WIDE_BAND_CHAN_SWITCH;
    *frm++ = 3;
    if (switch_chan->chan_bw == WIFINET_BWC_WIDTH80) {
        *frm++ = 1;
    } else {
        /* for 40m bw*/
        *frm++ = 0;
    }
    *frm++ = wifi_mac_mhz2chan(switch_chan->chan_cfreq1);
    *frm++ = 0;
    return frm;
}

unsigned char *
wifi_mac_add_chansw_wrapper(unsigned char *frm,struct wifi_station *sta)
{

    *frm++ = WIFINET_ELEMID_CHAN_SWITCH_WRAP;
    *frm++ = WIFINET_WIDEBANDCHANSW_BYTES;
    return wifi_mac_add_wide_bandwidth_subie(frm,sta);
}

unsigned char *
wifi_mac_add_extended_chanswitch(unsigned char *frm,struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_channel *switch_chan = &(wnet_vif->csa_target.switch_chan);

    *frm++ = WIFINET_ELEMID_EXTCHANSWITCHANN;
    *frm++ = 4;
    *frm++ = 1;
    *frm++ = switch_chan->global_operating_class;
    *frm++ = wifimac->wm_doth_channel;
    *frm++ = wifimac->wm_doth_tbtt;
    return frm;
}

unsigned char *
wifi_mac_add_chanswitch(unsigned char *frm,struct wifi_station *sta)
{
    struct wifi_mac *wifimac = sta->sta_wmac;

    *frm++ = WIFINET_ELEMID_CHANSWITCHANN;
    *frm++ = 3;
    *frm++ = 1;
    *frm++ = wifimac->wm_doth_channel;
    *frm++ = wifimac->wm_doth_tbtt;
    return frm;
}

void wifi_mac_csa_send_action_task(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)param1;
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param2;
    struct cfg80211_chan_def *chan_def_ptr = (struct cfg80211_chan_def *)param3;
    struct cfg80211_chan_def chan_def = {0};


    memcpy(&chan_def, chan_def_ptr, sizeof(struct cfg80211_chan_def));
    wifi_mac_csa_send_action(wifimac, wnet_vif, wnet_vif->vm_mainsta, chan_def);
    AML_OUTPUT("target_chan:%d, cur_chan:%d, wnet_vif->csa_count:%d\n", chan_def.chan->hw_value, wifimac->wm_curchan->chan_pri_num, wnet_vif->csa_count);

    return;
}

int wifi_mac_csa_send_action(struct wifi_mac *wifimac, struct wlan_net_vif *wnet_vif, struct wifi_station *sta, struct cfg80211_chan_def chan_def)
{
    struct sk_buff *skb;
    int ret = 0;
    struct wifi_mac_csa_data* csa_data_p = NULL;

    skb = wifi_mac_get_mgmt_frm(wifimac, MAX_ACTION_LEN);
    if (skb == NULL) {
        wnet_vif->vif_sts.sts_tx_no_buf++;
        return -ENOMEM;
    }

    csa_data_p = (struct wifi_mac_csa_data*)os_skb_put(skb, sizeof(struct wifi_mac_csa_data));
    csa_data_p->extend_csa.ac_header.ia_category = AML_CATEGORY_PUBLIC;
    csa_data_p->extend_csa.ac_header.ia_action = WIFINET_ACT_PUBLIC_CSA; //ACT_PUBLIC_EXT_CHL_SWITCH
    csa_data_p->extend_csa.csa.chan_switch_mode = CSA_BLOCK_TX;
    csa_data_p->extend_csa.csa.new_operation_class = wifi_mac_get_operation_class(chan_def);
    csa_data_p->extend_csa.csa.new_chan_num = ieee80211_frequency_to_channel(chan_def.chan->center_freq);
    csa_data_p->extend_csa.csa.chan_switch_count = wnet_vif->csa_count--;
    csa_data_p->csa_wrapper.ie = WLAN_EID_WIDE_BW_CHANNEL_SWITCH;
    csa_data_p->csa_wrapper.len = 3;
    /* New channel width */
    switch (chan_def.width) {
        case NL80211_CHAN_WIDTH_80:
            csa_data_p->csa_wrapper.info[0] = IEEE80211_VHT_CHANWIDTH_80MHZ;
        break;
        case NL80211_CHAN_WIDTH_160:
            csa_data_p->csa_wrapper.info[0] = IEEE80211_VHT_CHANWIDTH_160MHZ;
        break;
        case NL80211_CHAN_WIDTH_80P80:
            csa_data_p->csa_wrapper.info[0] = IEEE80211_VHT_CHANWIDTH_80P80MHZ;
        break;
        default:
            csa_data_p->csa_wrapper.info[0] = IEEE80211_VHT_CHANWIDTH_USE_HT;
    }
    /* new center frequency segment 0 */
    csa_data_p->csa_wrapper.info[1] = ieee80211_frequency_to_channel(chan_def.center_freq1);
    /* new center frequency segment 1 */
    if (chan_def.center_freq2)
        csa_data_p->csa_wrapper.info[2] = ieee80211_frequency_to_channel(chan_def.center_freq2);
    else
        csa_data_p->csa_wrapper.info[2] = 0;

    wifi_mac_mgmt_csa_action_broadcast_output(sta, skb, WIFINET_FC0_SUBTYPE_ACTION);

    return ret;
}
