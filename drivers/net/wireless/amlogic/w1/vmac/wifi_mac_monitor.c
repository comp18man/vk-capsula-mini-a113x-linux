/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     monitor interface
 *
 *
 ****************************************************************************************
 */

#include "wifi_mac_com.h"
#include "wifi_hal_com.h"

static int vm_cfg80211_monitor_if_open(struct net_device *ndev)
{
    int ret = 0;
    DPRINTF(AML_DEBUG_CFG80211,"%s <%s>\n", __func__, DEV_NAME(ndev));
    return ret;
}
static int vm_cfg80211_monitor_if_close(struct net_device *ndev)
{
    int ret = 0;
    DPRINTF(AML_DEBUG_CFG80211,"%s <%s>\n", __func__, DEV_NAME(ndev));
    return ret;
}

static int vm_cfg80211_monitor_if_xmit_entry(struct sk_buff *skb, struct net_device *ndev)
{
    int ret = 0;
    int rtap_len;
    int dot11_hdr_len = 24;
    int snap_len = 6;
    unsigned char *pdata;
    unsigned short frame_ctl = 0;
    unsigned char src_mac_addr[6];
    unsigned char dst_mac_addr[6];
    struct wifi_frame *wh;
    struct wlan_net_vif *wnet_vif = vm_netdev_priv(ndev);
    struct ieee80211_radiotap_header *rtap_hdr;

    DPRINTF(AML_DEBUG_CFG80211,"<%s>:%s ++\n", wnet_vif->vm_ndev->name,__func__);
#if 1
    if (unlikely(os_skb_get_pktlen(skb) < sizeof(struct ieee80211_radiotap_header)))
        goto fail;

    rtap_hdr = (struct ieee80211_radiotap_header *)os_skb_data(skb);
    if (unlikely(rtap_hdr->it_version))
        goto fail;
    rtap_len = ieee80211_get_radiotap_len(os_skb_data(skb));
    if (unlikely(os_skb_get_pktlen(skb) < rtap_len))
        goto fail;
    if (rtap_len != 14)
    {
        DPRINTF(AML_DEBUG_CFG80211,"radiotap len (should be 14): %d\n", rtap_len);
        goto fail;
    }
    os_skb_pull(skb, rtap_len);
    wh = (struct wifi_frame *)os_skb_data(skb);

    if (WIFINET_IS_ACTION(wh))
    {
        DPRINTF(AML_DEBUG_CFG80211, "%s, do: scan_abort\n", __func__);

        preempt_scan(wnet_vif->vm_ndev, 100, 100);
    }


    frame_ctl = *(unsigned short*)wh->i_fc; // need to check twice
    if ((frame_ctl & IEEE80211_FCTL_FTYPE) == IEEE80211_FTYPE_DATA)
    {

        dot11_hdr_len= wifi_mac_hdrsize(wh);
        memcpy(dst_mac_addr, wh->i_addr1, sizeof(dst_mac_addr));
        memcpy(src_mac_addr, wh->i_addr2, sizeof(src_mac_addr));
        os_skb_pull(skb, dot11_hdr_len + snap_len - sizeof(src_mac_addr) * 2);
        pdata = (unsigned char*)os_skb_data(skb);
        memcpy(pdata, dst_mac_addr, sizeof(dst_mac_addr));
        memcpy(pdata + sizeof(dst_mac_addr), src_mac_addr, sizeof(src_mac_addr));
        // DPRINTF(AML_DEBUG_CFG80211,"should be eapol packet\n");
        DPRINTF(AML_DEBUG_CFG80211, "<running> %s %d should be eapol packet\n",__func__,__LINE__);

        ret =  wifi_mac_hardstart(skb, wnet_vif->vm_ndev);
        return ret;
    }
    else if ((frame_ctl & (IEEE80211_FCTL_FTYPE|IEEE80211_FCTL_STYPE)) == cpu_to_le16(IEEE80211_STYPE_ACTION))
    {
#ifdef CONFIG_P2P
        if (vm_p2p_parse_negotiation_frames(wnet_vif->vm_p2p, os_skb_data(skb), &os_skb_get_pktlen(skb), true)== false)
        {
            DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_ERROR, "%s %d print frame err\n", __func__,__LINE__);
            goto fail;
        }
#endif
        if ( vm_cfg80211_send_mgmt(wnet_vif,os_skb_data(skb),  os_skb_get_pktlen(skb)) != 0)
        {
            DPRINTF(AML_DEBUG_CFG80211|AML_DEBUG_ERROR, "%s %d send frame err\n", __func__,__LINE__);
            goto fail;
        }
    }
    else
    {
        DPRINTF(AML_DEBUG_CFG80211,"frame_ctl=0x%x\n", frame_ctl & (IEEE80211_FCTL_FTYPE|IEEE80211_FCTL_STYPE));
    }
#endif
fail:
    wifi_mac_free_skb(skb);
    return 0;
}
static void vm_cfg80211_monitor_if_destructor(struct net_device *ndev)
{
    struct wlan_net_vif *wnet_vif = vm_netdev_priv(ndev);

    DPRINTF(AML_DEBUG_ANY,"<%s>:<%s>:%s\n", wnet_vif->vm_ndev->name,ndev->name,__func__);

    if (ndev->ieee80211_ptr)
        FREE((unsigned char *)ndev->ieee80211_ptr,"mwdev");
    
    free_netdev(ndev);
}
static int vm_cfg80211_go_if_xmit_entry(struct sk_buff *skb, struct net_device *ndev)
{
    struct wlan_net_vif *wnet_vif = vm_netdev_priv(ndev);

    DPRINTF(AML_DEBUG_ANY,"<%s>:<%s>:%s\n", wnet_vif->vm_ndev->name,ndev->name,__func__);

    dump_memory_internal(skb->data,32);
    return wifi_mac_hardstart(skb, wnet_vif->vm_ndev);
}

static int vm_cfg80211_monitor_set_mac_addr(struct net_device *ndev, void *addr)
{
    int ret = 0;
    DPRINTF(AML_DEBUG_CFG80211,"%s,<%s>:mac<%s> \n", __func__,ndev->name,ether_sprintf(ndev->dev_addr));
    return ret;
}
static const struct net_device_ops vm_cfg80211_monitor_if_ops =
{
    .ndo_open = vm_cfg80211_monitor_if_open,
    .ndo_stop = vm_cfg80211_monitor_if_close,
    .ndo_start_xmit = vm_cfg80211_monitor_if_xmit_entry,
    .ndo_set_mac_address = vm_cfg80211_monitor_set_mac_addr,
};
struct net_device *vm_cfg80211_add_monitor_if(struct wlan_net_vif *wnet_vif ,
const char *name)

{
    int ret = 0;
    struct net_device* ndev = NULL;
    struct vm_wdev_priv *pwdev_priv = wdev_to_priv(wnet_vif->vm_wdev);
    struct vm_netdev_priv_indicator *pnpi;
    struct wireless_dev* wdev = NULL;
    DPRINTF(AML_DEBUG_CFG80211,"%s <%s>\n", __func__, name);
    if (!name )
    {
        ret = -EINVAL;
        goto out;
    }
    if ((strnicmp(name, pwdev_priv->ifname_mon, strlen(name)) ==0)
        && pwdev_priv->pmon_ndev)
    {
        ndev = pwdev_priv->pmon_ndev;
        DPRINTF(AML_DEBUG_CFG80211,"%s, monitor interface(%s) has existed\n", __func__, name);
        goto out;
    }
    ndev = alloc_etherdev(sizeof(struct vm_netdev_priv_indicator));
    if (!ndev)
    {
        ret = -ENOMEM;
        goto out;
    }

    ndev->type = ARPHRD_IEEE80211_RADIOTAP;
    strncpy(ndev->name, name, IFNAMSIZ);
    ndev->name[IFNAMSIZ - 1] = 0;
    dev_addr_mod(ndev, 0, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);
    ndev->hard_header_len = DEFAULT_HARD_HDR_LEN;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    ndev->priv_destructor = vm_cfg80211_monitor_if_destructor;
#else
    ndev->destructor = vm_cfg80211_monitor_if_destructor;
#endif
    if (wnet_vif->vm_wmac->wm_caps & WIFINET_C_HWCSUM)
    {
        //ndev->features |= (NETIF_F_HW_CSUM |NETIF_F_IP_CSUM);
        netdev_setcsum(ndev,1);
    }
    ndev->netdev_ops = &vm_cfg80211_monitor_if_ops;
    pnpi = netdev_priv(ndev);
    pnpi->priv = wnet_vif;
    pnpi->sizeof_priv = sizeof(wnet_vif);
    /*  wdev */
    wdev = (struct wireless_dev *)ZMALLOC(sizeof(struct wireless_dev),"mwdev", GFP_KERNEL);
    if (!wdev)
    {
        ERROR_DEBUG_OUT("MALLOC wireless_dev fail\n");
        ret = -ENOMEM;
        goto out;
    }

    wdev->wiphy = wnet_vif->vm_wdev->wiphy;
    wdev->netdev = ndev;
    wdev->iftype = NL80211_IFTYPE_MONITOR;
    ndev->ieee80211_ptr = wdev;
    ret = register_netdevice(ndev);
    if (ret)
    {
        goto out;
    }
    pwdev_priv->pmon_ndev = ndev;
    memcpy(pwdev_priv->ifname_mon, name, IFNAMSIZ+1);
    return  ndev;

out:
    if (ret && ndev)
    {
        free_netdev(ndev);
        ndev = NULL;
    }
    ERROR_DEBUG_OUT("ndev=%p, pmon_ndev=%p, ret=%d\n", ndev, pwdev_priv->pmon_ndev, ret);
    return ndev;
}

static int __vm_cfg80211_ioctl(struct net_device *ndev, void __user *data, int cmd)
{
    struct wlan_net_vif *wnet_vif = vm_netdev_priv(ndev);
    
    DPRINTF(AML_DEBUG_IOCTL, "%s %d cmd 0x%x\n", __func__, __LINE__, cmd);
    switch (cmd)
    {
        case SIOCANDROID_PRIV:
            return  aml_android_priv_cmd(wnet_vif, data, cmd);
            break;
    }

    return -1;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static int vm_cfg80211_ioctl_wrapper(struct net_device *ndev, struct ifreq *ifr,
        void __user *data, int cmd)
{
    return __vm_cfg80211_ioctl(ndev, data, cmd);
}

#else

static int vm_cfg80211_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{
    return __vm_cfg80211_ioctl(ndev, ifr->ifr_data, cmd);
}

#endif

static const struct net_device_ops vm_cfg80211_go_if_ops =
{
    .ndo_open = vm_cfg80211_monitor_if_open,
    .ndo_stop = vm_cfg80211_monitor_if_close,
    .ndo_start_xmit = vm_cfg80211_go_if_xmit_entry,
    .ndo_set_mac_address = vm_cfg80211_monitor_set_mac_addr,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    .ndo_siocdevprivate         = vm_cfg80211_ioctl_wrapper,
#else
    .ndo_do_ioctl               = vm_cfg80211_ioctl,
#endif
};
struct net_device *vm_cfg80211_add_p2p_go_if(struct wlan_net_vif *wnet_vif ,
const char *name, enum nl80211_iftype type)
{
    int ret = 0;
    struct net_device* ndev = NULL;
    struct vm_wdev_priv *pwdev_priv = NULL;
    struct vm_netdev_priv_indicator *pnpi;
    // struct WIFINET_MAC *wifimac = vmac->vm_wmpriv;
    struct wireless_dev *vwdev;
    enum wifi_mac_opmode networkType, oldnetworkType;

    DPRINTF(AML_DEBUG_CFG80211, "%s %s,type %d\n", __func__,name,type);
    if (!name )
    {
        ret = -EINVAL;
        goto out;
    }
    ndev = alloc_etherdev(sizeof(struct vm_netdev_priv_indicator));
    if (!ndev)
    {
        ret = -ENOMEM;
        goto out;
    }
    vwdev = (struct wireless_dev *)ZMALLOC(sizeof(struct wireless_dev),"vwdev", GFP_KERNEL);
    if (!vwdev)
    {
        AML_OUTPUT("ERROR ENOMEM\n");
        ret = -ENOMEM;
        return NULL;
    }

#ifdef NEW_WIPHY
    vwdev->wiphy = wiphy_new(&vm_cfg80211_ops, sizeof(struct vm_wdev_priv));
    if (!vwdev->wiphy)
    {
        AML_OUTPUT("ERROR ENOMEM\n");
        ret = -ENOMEM;
        return NULL;

    }
    AML_PRINT(AML_DBG_MODULES_P2P, "%s,type %d\n", name,type);

    set_wiphy_dev(vwdev->wiphy, ndev );
    vwdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_P2P_GO)|BIT(NL80211_IFTYPE_P2P_CLIENT);

    vwdev->wiphy->cipher_suites = aml_cipher_suites;
    vwdev->wiphy->n_cipher_suites = ARRAY_SIZE(aml_cipher_suites);

    vwdev->wiphy->bands[IEEE80211_BAND_2GHZ] = aml_spt_band_alloc(IEEE80211_BAND_2GHZ);
    vwdev->wiphy->bands[IEEE80211_BAND_5GHZ] = aml_spt_band_alloc(IEEE80211_BAND_5GHZ);

    if (wnet_vif->vm_pwrsave.ips_sta_psmode != WIFINET_PWRSAVE_NONE)
        vwdev->wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;
    else
        vwdev->wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
    vwdev->wiphy->mgmt_stypes = vm_cfg80211_default_mgmt_stypes;
    vwdev->wiphy->addresses = wnet_vif->vm_myaddr;
    vwdev->wiphy->n_addresses = 1;
    pwdev_priv = wdev_to_priv(vwdev);
    pwdev_priv->pmon_ndev = NULL;
    pwdev_priv->pGo_ndev = NULL;
    pwdev_priv->ifname_mon[0] = '\0';
    pwdev_priv->ifname_go[0] = '\0';
    pwdev_priv->vm_wdev = vwdev;
    pwdev_priv->wnet_vif = wnet_vif;
    pwdev_priv->scan_request = NULL;
    spin_lock_init(&pwdev_priv->scan_req_lock);
    pwdev_priv->connect_request = NULL;
    AML_PRINT(AML_DBG_MODULES_P2P, "%s,type %d\n", name,type);

    spin_lock_init(&pwdev_priv->connect_req_lock);
    os_timer_ex_initialize(&pwdev_priv->connect_timeout, CFG80211_CONNECT_TIMER_OUT,
                           vm_cfg80211_connect_timeout_timer, wnet_vif);

    AML_OUTPUT("<running>\n");
    ret = wiphy_register(vwdev->wiphy);
    if (ret < 0)
    {
        AML_OUTPUT("ERROR register wiphy\n");
        ret = -ENOMEM;
        return NULL;
    }
    AML_PRINT(AML_DBG_MODULES_P2P,"%s,type %d\n", name,type);
#else
    vwdev->wiphy =    wnet_vif->vm_wdev->wiphy;
    pwdev_priv = wdev_to_priv(vwdev);
#endif
    vwdev->iftype = type;
    ndev->ieee80211_ptr = vwdev;
    vwdev->netdev = ndev;
    AML_PRINT(AML_DBG_MODULES_P2P,"%s,type %d\n",name,type);
    //ndev->type = ARPHRD_ETHER;
    strncpy(ndev->name, name, IFNAMSIZ);
    ndev->name[IFNAMSIZ - 1] = 0;
    ndev->netdev_ops = &vm_cfg80211_go_if_ops;

    pnpi = netdev_priv(ndev);
    pnpi->priv = wnet_vif;
    pnpi->sizeof_priv = sizeof(wnet_vif);
    dev_addr_mod(ndev, 0, wnet_vif->vm_myaddr, WIFINET_ADDR_LEN);

    AML_PRINT(AML_DBG_MODULES_P2P,"%s,type %d\n",name,type);

    ret = register_netdevice(ndev);
    if (ret)
    {
        goto out;
    }
    pwdev_priv->pGo_ndev = ndev;
    pwdev_priv->pGo_wdev = vwdev;
    memcpy(pwdev_priv->ifname_go, name, IFNAMSIZ+1);

    AML_PRINT(AML_DBG_MODULES_P2P,"%s,type %d\n",name,type);

    networkType = nl80211_iftype_2_drv_opmode(type);
    if (networkType < 0)
    {
        goto out;
    }

    oldnetworkType = wnet_vif->vm_opmode;
    if (oldnetworkType != networkType)
    {
        struct wifi_mac *wifimac = wnet_vif->vm_wmac;
        DPRINTF(AML_DEBUG_CFG80211, "%s %d\n", __func__, __LINE__);
        preempt_scan(wnet_vif->vm_ndev, 100, 100);
        wnet_vif->wnet_vif_replaycounter++;
        //wifi_mac_stop(vmac->vm_dev);
        wnet_vif->vm_state = WIFINET_S_INIT;
        wifi_mac_scan_vdetach(wnet_vif);
        if (wifimac->drv_priv->drv_ops.change_interface(wifimac->drv_priv,
                                                        wnet_vif->wnet_vif_id,
                                                        wnet_vif,
                                                        (enum hal_op_mode)networkType,
                                                        wnet_vif->vm_myaddr, 0))
        {
            ERROR_DEBUG_OUT("Unable to add an interface for driver.\n");
            wifi_mac_free_vmac(wnet_vif);
            return NULL;
        }
        wnet_vif->vm_opmode = networkType;
        wifi_mac_scan_vattach(wnet_vif);
        //wifi_mac_vmac_setup_forchvif(wifimac, vmac, NULL, networkType);
        if (wnet_vif->vm_opmode == WIFINET_M_HOSTAP)
        {
            if (wnet_vif->vm_max_aid == 0)
                wnet_vif->vm_max_aid = WIFINET_AID_DEF;
            else if (wnet_vif->vm_max_aid > WIFINET_AID_MAX)
                wnet_vif->vm_max_aid = WIFINET_AID_MAX;
        }
        wifi_mac_open(wnet_vif->vm_ndev);
#ifdef  CONFIG_P2P
        if (wnet_vif->vm_p2p_support)
        {
            vm_p2p_set_role(wnet_vif->vm_p2p, NL80211_IFTYPE_2_p2p_role(type));
            DPRINTF(AML_DEBUG_CFG80211, "%s %d new p2p_role=%d\n", __func__, __LINE__, vm_p2p_role(wnet_vif->vm_p2p));
        }
#endif

        if (WIFINET_M_HOSTAP == networkType)
        {
            DPRINTF(AML_DEBUG_CFG80211, "%s %d mode change over\n", __func__, __LINE__);
            wait_for_ap_run(wnet_vif, 5000, 100);
            DPRINTF(AML_DEBUG_CFG80211, "%s %d wait_for_ap_run over\n", __func__, __LINE__);
        }
        wnet_vif->vm_ndev->ieee80211_ptr->iftype = type;
    }
out:
    if (ret && ndev)
    {
        free_netdev(ndev);
        ndev = NULL;
    }

    AML_PRINT(AML_DBG_MODULES_P2P,"%s,type %d, ndev=%p, ret=%d\n",name,type, ndev, ret);
    return ndev;
}

