/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer encrypt/decrypt module
 *
 *
 ****************************************************************************************
 */


#include "wifi_mac_com.h"

static const struct wifi_mac_security *security_type[WIFINET_CIPHER_MAX];
static int wifi_mac_security_delkey(struct wlan_net_vif *, struct wifi_mac_key *, struct wifi_station *);

void wifi_mac_security_detach(struct wifi_mac *wifimac)
{
}

void wifi_mac_security_vattach(struct wlan_net_vif *wnet_vif)
{
    int i;

    wnet_vif->vm_def_txkey = WIFINET_KEYIX_NONE;
    for (i = 0; i < WIFINET_WEP_NKID; i++)
        wifi_mac_security_resetkey(wnet_vif, &wnet_vif->vm_nw_keys[i], WIFINET_KEYIX_NONE);

    wnet_vif->vif_ops.vm_key_update_begin = NULL;
    wnet_vif->vif_ops.vm_key_update_end = NULL;
}

void wifi_mac_security_vdetach(struct wlan_net_vif *wnet_vif)
{
    wifi_mac_security_delt_default_key(wnet_vif);
}

int wifi_mac_security_available(unsigned int cipher)
{
    return cipher < WIFINET_CIPHER_MAX && security_type[cipher] != NULL;
}

int wifi_mac_security_req(struct wlan_net_vif *wnet_vif, int cipher, int flags, struct wifi_mac_key *key)
{
    const struct wifi_mac_security *cip;
    void *keyctx;
    int oflags;

    if (cipher >= WIFINET_CIPHER_MAX) {
        wnet_vif->vif_sts.sts_key_type_err++;
        pr_debug("<running> %s cipher:%d\n",__func__, cipher);
        return 0;
    }

    cip = security_type[cipher];
    if (cip == NULL) {
        WIFINET_DPRINTF(AML_DEBUG_ANY, "Err:  security_type=%d  module is out of range \n", cipher);
        return 0;
    }

    WIFINET_DPRINTF(AML_DEBUG_DEBUG," k->wk_keyix = %d new %s ,old %s\n", key->wk_keyix, cip->wm_name, key->wk_cipher->wm_name);
    oflags = key->wk_flags;
    flags &= WIFINET_KEY_COMMON;

    if (cipher == WIFINET_CIPHER_TKIP) {
        flags |= WIFINET_KEY_SWMIC;
    }

    if (key->wk_cipher != cip || key->wk_flags != flags) {
        key->wk_flags = flags;
        keyctx = cip->wm_attach(wnet_vif, key);
        if (keyctx == NULL) {
            key->wk_flags = oflags;
            WIFINET_DPRINTF(AML_DEBUG_DEBUG,"  cip->wm_attach FAIL , %s key drop \n",key->wk_cipher->wm_name);
            return 0;
        }
        key->wk_cipher->wm_detach(key);
        key->wk_cipher = cip;
        key->wk_private = keyctx;
    }
    key->wk_flags = flags;
    return 1;
}

static int wifi_mac_security_delkey(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *key, struct wifi_station *sta)
{
    unsigned short key_index;

    KASSERT(key->wk_cipher != NULL, ("No cipher!"));
    DPRINTF(AML_DEBUG_KEY, "%s: delete key sta %p\n", __func__,sta);

    key_index = key->wk_keyix;
    if (key_index != WIFINET_KEYIX_NONE) {
        wifi_mac_key_delete(wnet_vif, key, sta);
    }
    key->wk_cipher->wm_detach(key);
    memset(key, 0, sizeof(*key));

    wifi_mac_security_resetkey(wnet_vif, key, WIFINET_KEYIX_NONE);
    return 1;
}

int wifi_mac_sec_delt_key(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *key, struct wifi_station *sta)
{
    int status;

    wifi_mac_KeyUpdateBegin(wnet_vif);
    status = wifi_mac_security_delkey(wnet_vif, key, sta);
    wifi_mac_KeyUpdateEnd(wnet_vif);

    return status;
}

void wifi_mac_security_delt_default_key(struct wlan_net_vif *wnet_vif)
{
    int i;

    wifi_mac_KeyUpdateBegin(wnet_vif);
    for (i = 0; i < WIFINET_WEP_NKID; i++)
        (void) wifi_mac_security_delkey(wnet_vif, &wnet_vif->vm_nw_keys[i], wnet_vif->vm_mainsta);
    wifi_mac_KeyUpdateEnd(wnet_vif);
}

int wifi_mac_security_setkey(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *key,
    const unsigned char macaddr[WIFINET_ADDR_LEN], struct wifi_station *sta)
{
    const struct wifi_mac_security *cip = key->wk_cipher;
    int ret;

    KASSERT(cip != NULL, ("No cipher!"));

    if (!cip->wm_setkey(key)) {
        wnet_vif->vif_sts.sts_key_drop++;
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        return 0;
    }

    if (key->wk_keyix == WIFINET_KEYIX_NONE) {
        wnet_vif->vif_sts.sts_key_id_err++;
        pr_debug("<running> %s %d \n",__func__,__LINE__);
        return 0;
    }

    ret = wifi_mac_key_set(wnet_vif, key, macaddr);
    return ret;
}

int wifi_mac_security_set_rekey_data(struct wlan_net_vif *wnet_vif, struct wifi_mac_rekey_data rekey_data)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    return (wifimac->drv_priv->drv_ops.rekey_data_set(wifimac->drv_priv, wnet_vif->wnet_vif_id, (void *)(&rekey_data)) != 0);
}

struct wifi_mac_key *wifi_mac_security_encap(struct wifi_station *sta, struct sk_buff *skb)
{
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_key *k;
    struct wifi_frame *wh;
    const struct wifi_mac_security *cip;
    unsigned char keyid;

    wh = (struct wifi_frame *)os_skb_data(skb);
    if (WIFINET_IS_MULTICAST(wh->i_addr1) ||
        sta->sta_ucastkey.wk_cipher == &wifi_mac_cipher_none)
    {
        if (wnet_vif->vm_def_txkey == WIFINET_KEYIX_NONE)
        {
            WIFINET_DPRINTF_MACADDR( AML_DEBUG_KEY,
                wh->i_addr1, "no default transmit key (%s) deftxkey %u", __func__, wnet_vif->vm_def_txkey);
            wnet_vif->vif_sts.sts_tx_key_err++;
            return NULL;
        }
        keyid = wnet_vif->vm_def_txkey;
        k = &wnet_vif->vm_nw_keys[wnet_vif->vm_def_txkey];
    }
    else
    {
        keyid = 0;
        k = &sta->sta_ucastkey;
    }
    cip = k->wk_cipher;
    if (os_skb_hdrspace(skb) < cip->wm_header)
    {
        WIFINET_DPRINTF_MACADDR( AML_DEBUG_KEY, wh->i_addr1,
            "%s: malformed packet for cipher %s; headroom %u", __func__, cip->wm_name, os_skb_hdrspace(skb));
        wnet_vif->vif_sts.sts_headroom_err++;
        return NULL;
    }
    return (cip->wm_encap(k, skb, keyid<<6) ? k : NULL);
}

struct wifi_mac_key *
wifi_mac_security_decap(struct wifi_station *sta, struct sk_buff *skb, int hdrlen)
{

    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct wifi_mac_key *k;
    struct wifi_frame *wh;
    const struct wifi_mac_security *cip;
    // const unsigned char *vm_p;
    unsigned char keyid;

    if (os_skb_get_pktlen(skb) < sizeof(struct wifi_frame) )
    {
        WIFINET_DPRINTF_STA( AML_DEBUG_ANY, sta,
                             "%s: WEP data frame too short, len %u",
                             __func__, os_skb_get_pktlen(skb));
        wnet_vif->vif_sts.sts_rx_too_short++;
        return NULL;
    }

    wh = (struct wifi_frame *) os_skb_data(skb);
    if (WIFINET_IS_MULTICAST(wh->i_addr1) ||
        sta->sta_ucastkey.wk_cipher == &wifi_mac_cipher_none)
    {
        if ((wnet_vif->vm_def_txkey == 1) || (wnet_vif->vm_def_txkey == 2))
            keyid = wnet_vif->vm_def_txkey;
        else
            keyid = 1;
        k = &wnet_vif->vm_nw_keys[keyid];
    }
    else
    {
        k = &sta->sta_ucastkey;
    }

    cip = k->wk_cipher;
    if (k->wk_keyix == WIFINET_KEYIX_NONE)
    {
        return (NULL);
    }
    else
    {
        struct wifi_skb_callback *cb = (struct wifi_skb_callback *)skb->cb;

        if (os_skb_get_pktlen(skb) < (sizeof(struct wifi_frame)+  k->wk_cipher->wm_miclen) )
        {
            WIFINET_DPRINTF_STA( AML_DEBUG_ANY, sta,
                                 "%s: WEP data frame too short, len %u",
                                 __func__, os_skb_get_pktlen(skb));
            wnet_vif->vif_sts.sts_rx_too_short++;
            return NULL;
        }
        cb->sta = sta;

        return (cip->wm_decap(k, skb, hdrlen) ? k : NULL);
    }
}


#ifndef _BYTE_ORDER
#error "Don't know native byte order"
#endif



static void *
wep_attach(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *k)
{
    return wnet_vif;
}

static void
wep_detach(struct wifi_mac_key *k)
{
}

static int
wep_setkey(struct wifi_mac_key *k)
{
    return 1;
}


static int
wep_encap(struct wifi_mac_key *k, struct sk_buff *skb, unsigned char keyid)
{
    return 1;
}

static int
wep_enmic(struct wifi_mac_key *k, struct sk_buff *skb, int force)
{
    return 1;
}

static int
wep_decap(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen)
{
    return 1;
}

static int
wep_demic(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen, int force)
{
    return 1;
}

static void *
wpi_attach(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *k)
{
    return wnet_vif;
}

static void
wpi_detach(struct wifi_mac_key *k)
{
    (void) k;
}

static int
wpi_setkey(struct wifi_mac_key *k)
{
    (void) k;
    return 1;
}
static int
wpi_encap(struct wifi_mac_key *k, struct sk_buff *skb, unsigned char keyid)
{
    return 1;
}

static int
wpi_decap(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen)
{
    return 1;
}

static int
wpi_enmic(struct wifi_mac_key *k, struct sk_buff *skb, int force)
{
    return 1;
}

static int
wpi_demic(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen, int force)
{
    return 1;
}

static void *
ccmp_attach(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *k)
{
    return wnet_vif;
}

static void
ccmp_detach(struct wifi_mac_key *k)
{
}

static int
ccmp_setkey(struct wifi_mac_key *k)
{
    return 1;
}

static int
ccmp_encap(struct wifi_mac_key *k, struct sk_buff *skb, unsigned char keyid)
{
    return 1;
}

static int
ccmp_enmic(struct wifi_mac_key *k, struct sk_buff *skb, int force)
{
    return 1;
}

static int
ccmp_decap(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen)
{
    return 1;
}


static int
ccmp_demic(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen, int force)
{
    return 1;
}

static void *
none_attach(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *k)
{
    return wnet_vif;
}

static void
none_detach(struct wifi_mac_key *k)
{
    (void) k;
}

static int
none_setkey(struct wifi_mac_key *k)
{
    (void) k;
    return 1;
}

static int
none_encap(struct wifi_mac_key *k, struct sk_buff *skb, unsigned char keyid)
{
    // struct WIFINET_VMAC *vmac = k->wk_private;


    return 0;
}

static int
none_decap(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen)
{
    // struct WIFINET_VMAC *vmac = k->wk_private;


    return 0;
}

static int
none_enmic(struct wifi_mac_key *k, struct sk_buff *skb, int force)
{
    // struct WIFINET_VMAC *vmac = k->wk_private;

    return 0;
}

static int
none_demic(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen, int force)
{
    // struct WIFINET_VMAC *vmac = k->wk_private;

    return 0;
}

static  void michael_mic(const unsigned char *key,
                         struct sk_buff *, unsigned int off, size_t data_len,
                         unsigned char mic[WIFINET_WEP_MICLEN]);

static void *
tkip_attach(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *k)
{
    return wnet_vif;
}

static void
tkip_detach(struct wifi_mac_key *k)
{
}

static int
tkip_setkey(struct wifi_mac_key *k)
{
    if (k->wk_keylen != (128/NBBY))
    {
        return 0;
    }
    k->wk_keytsc = 1;
    return 1;
}
static int
tkip_encap(struct wifi_mac_key *k, struct sk_buff *skb, unsigned char keyid)
{
    struct wlan_net_vif *wnet_vif = k->wk_private;

    if (wnet_vif->vm_flags & WIFINET_F_COUNTERM)
    {

        return 0;
    }
    k->wk_keytsc++;

    return 1;

}

static int
tkip_enmic(struct wifi_mac_key *k, struct sk_buff *skb0, int force)
{
    struct wlan_net_vif *wnet_vif = k->wk_private;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    if (force || (k->wk_flags & WIFINET_KEY_SWMIC))
    {
        struct wifi_frame *wh =
            (struct wifi_frame *) skb0->data;
        int hdrlen;
        struct sk_buff *skb;
        size_t data_len;
        uint8_t mic[WIFINET_WEP_MICLEN] = { 0 };

        wnet_vif->vif_sts.sts_tx_tkip_sw_mic_err++;

        skb = skb0;
        data_len = os_skb_get_pktlen(skb);
        while (os_skb_next(skb) != NULL)
        {
            skb = os_skb_next(skb);
            data_len += os_skb_get_pktlen(skb);
        }
        if (skb_tailroom(skb) < k->wk_cipher->wm_miclen)
        {

            WIFINET_DPRINTF_MACADDR( AML_DEBUG_KEY,
                                     wh->i_addr1,
                                     "No room for Michael MIC, tailroom %u",
                                     skb_tailroom(skb));
            ERROR_DEBUG_OUT("No room for Michael MIC, tailroom \n");

            return 0;
        }

        hdrlen = wifi_mac_hdrspace(wifimac, wh);
        if ((wifimac->wm_caps & WIFINET_C_TKIPMIC)&&(!force))
        {
            M_FLAG_SET(skb,M_TKIPMICHW);
        }
        else
        {
            michael_mic(k->wk_txmic,skb0, hdrlen, data_len - hdrlen, mic);

        }
        memcpy(os_skb_put(skb, k->wk_cipher->wm_miclen), mic, k->wk_cipher->wm_miclen);
    }
    return 1;
}


static int
tkip_decap(struct wifi_mac_key *k, struct sk_buff *skb, int hdrlen)
{

    return 1;
}


static int
tkip_demic(struct wifi_mac_key *k, struct sk_buff *skb0, int hdrlen, int force)
{

    struct wlan_net_vif *wnet_vif = k->wk_private;
    // struct WIFINET_MAC *wifimac = vmac->vm_wmpriv;
    struct sk_buff *skb;
    size_t pktlen;
    struct wifi_frame *wh ;

    if ((k->wk_flags & WIFINET_KEY_SWMIC) && force)
    {
        unsigned char mic[WIFINET_WEP_MICLEN];
        unsigned char mic0[WIFINET_WEP_MICLEN];
        skb = skb0;
        pktlen = os_skb_get_pktlen(skb);
        while (os_skb_next(skb) != NULL)
        {
            skb = os_skb_next(skb);
            pktlen += os_skb_get_pktlen(skb);
        }
        wh = (struct wifi_frame *) skb0->data;

        wnet_vif->vif_sts.sts_rx_tkip_sw_mic_err++;
        // pr_debug("<running> %s %d \n",__func__,__LINE__);
        //dump_memory_internal(k->wk_key, WIFINET_KEYBUF_SIZE+WIFINET_MICBUF_SIZE);
        michael_mic(k->wk_rxmic,
                    skb0, hdrlen, pktlen - (hdrlen + k->wk_cipher->wm_miclen),
                    mic);
        memcpy(mic0, os_skb_data(skb) + os_skb_get_pktlen(skb) - k->wk_cipher->wm_miclen,
               k->wk_cipher->wm_miclen);
        if (memcmp(mic, mic0, k->wk_cipher->wm_miclen))
        {
            wifi_mac_notify_mic_fail(wnet_vif, wh, k->wk_keyix);
            WIFINET_DPRINTF( AML_DEBUG_KEY,"%s  mic fail drop\n",ether_sprintf(wh->i_addr2));
            return 0;
        }
        os_skb_trim(skb, os_skb_get_pktlen(skb) - k->wk_cipher->wm_miclen);
    }

    return 1;
}


static __inline unsigned int rotl(unsigned int val, int bits)
{
    return (val << bits) | (val >> (32 - bits));
}


static __inline unsigned int rotr(unsigned int val, int bits)
{
    return (val >> bits) | (val << (32 - bits));
}


static __inline unsigned int xswap(unsigned int val)
{
    return ((val & 0x00ff00ff) << 8) | ((val & 0xff00ff00) >> 8);
}





static __inline unsigned int get_le32_split(unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3)
{
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

static __inline unsigned int get_le32(const unsigned char *p)
{
    return get_le32_split(p[0], p[1], p[2], p[3]);
}


static __inline void put_le32(unsigned char *p, unsigned int v)
{
    p[0] = v;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;
}

static void
michael_mic_hdr(const struct wifi_frame *wh0, unsigned char hdr[16])
{
    const struct WIFINET_S_FRAME_ADDR4 *wh =
        (const struct WIFINET_S_FRAME_ADDR4 *) wh0;

    switch (wh->i_fc[1] & WIFINET_FC1_DIR_MASK)
    {
        case WIFINET_FC1_DIR_NODS:
            WIFINET_ADDR_COPY(hdr, wh->i_addr1);
            WIFINET_ADDR_COPY(hdr + WIFINET_ADDR_LEN, wh->i_addr2);
            break;
        case WIFINET_FC1_DIR_TODS:
            WIFINET_ADDR_COPY(hdr, wh->i_addr3);
            WIFINET_ADDR_COPY(hdr + WIFINET_ADDR_LEN, wh->i_addr2);
            break;
        case WIFINET_FC1_DIR_FROMDS:
            WIFINET_ADDR_COPY(hdr, wh->i_addr1);
            WIFINET_ADDR_COPY(hdr + WIFINET_ADDR_LEN, wh->i_addr3);
            break;
        case WIFINET_FC1_DIR_DSTODS:
            WIFINET_ADDR_COPY(hdr, wh->i_addr3);
            WIFINET_ADDR_COPY(hdr + WIFINET_ADDR_LEN, wh->i_addr4);
            break;
    }

    if (wh->i_fc[0] & WIFINET_FC0_SUBTYPE_QOS)
    {
        const struct wifi_qos_frame *qwh =
            (const struct wifi_qos_frame *) wh;
        hdr[12] = qwh->i_qos[0] & WIFINET_QOS_TID;
    }
    else
        hdr[12] = 0;
    hdr[13] = hdr[14] = hdr[15] = 0;
}

static void michael_mic(const unsigned char *key, struct sk_buff *skb, unsigned int off, size_t data_len,
    unsigned char mic[WIFINET_WEP_MICLEN])
{
    uint8_t hdr[16];
    unsigned int l, r;
    const uint8_t *data;
    unsigned int space;

    michael_mic_hdr((struct wifi_frame *) os_skb_data(skb), hdr);

    l = get_le32(key);
    r = get_le32(key + 4);

    l ^= get_le32(hdr);
    michael_block(l, r);
    l ^= get_le32(&hdr[4]);
    michael_block(l, r);
    l ^= get_le32(&hdr[8]);
    michael_block(l, r);
    l ^= get_le32(&hdr[12]);
    michael_block(l, r);

    data = os_skb_data(skb) + off;
    space = os_skb_get_pktlen(skb) - off;
    for (;;)
    {
        if (space > data_len)
            space = data_len;
        while (space >= sizeof(uint32_t))
        {
            l ^= get_le32(data);
            michael_block(l, r);
            data += sizeof(uint32_t), space -= sizeof(uint32_t);
            data_len -= sizeof(uint32_t);
        }
        if (data_len < sizeof(uint32_t))
            break;
        skb = os_skb_next(skb);
        if (skb == NULL)
        {
            KASSERT(0, ("out of data, data_len %zu\n", data_len));
            break;
        }
        if (space != 0)
        {
            const uint8_t *data_next;
            data_next = os_skb_data(skb);
            KASSERT(os_skb_get_pktlen(skb) >= sizeof(uint32_t) - space,
                    ("not enough data in following buffer, "
                     "skb len %u need %zu\n", os_skb_get_pktlen(skb),
                     sizeof(uint32_t) - space));
            switch (space)
            {
                case 1:
                    l ^= get_le32_split(data[0], data_next[0],
                                        data_next[1], data_next[2]);
                    data = data_next + 3;
                    space = os_skb_get_pktlen(skb) - 3;
                    break;
                case 2:
                    l ^= get_le32_split(data[0], data[1],
                                        data_next[0], data_next[1]);
                    data = data_next + 2;
                    space = os_skb_get_pktlen(skb) - 2;
                    break;
                case 3:
                    l ^= get_le32_split(data[0], data[1],
                                        data[2], data_next[0]);
                    data = data_next + 1;
                    space = os_skb_get_pktlen(skb) - 1;
                    break;
            }
            michael_block(l, r);
            data_len -= sizeof(uint32_t);
        }
        else
        {
            data = os_skb_data(skb);
            space = os_skb_get_pktlen(skb);
        }
    }
    switch (data_len)
    {
        case 0:
            l ^= get_le32_split(0x5a, 0, 0, 0);
            break;
        case 1:
            l ^= get_le32_split(data[0], 0x5a, 0, 0);
            break;
        case 2:
            l ^= get_le32_split(data[0], data[1], 0x5a, 0);
            break;
        case 3:
            l ^= get_le32_split(data[0], data[1], data[2], 0x5a);
            break;
    }
    michael_block(l, r);

    michael_block(l, r);

    put_le32(mic, l);
    put_le32(mic + 4, r);
}

const struct wifi_mac_security wifi_mac_cipher_none =
{
    .wm_name    = "NONE",
    .wm_cipher  = WIFINET_CIPHER_NONE,
    .wm_header  = 0,
    .wm_trailer = 0,
    .wm_miclen  = 0,
    .wm_attach  = none_attach,
    .wm_detach  = none_detach,
    .wm_setkey  = none_setkey,
    .wm_encap   = none_encap,
    .wm_decap   = none_decap,
    .wm_enmic   = none_enmic,
    .wm_demic   = none_demic,
};

const struct wifi_mac_security wifi_mac_cipher_wpi =
{
    .wm_name    = "wpi",
    .wm_cipher  = WIFINET_CIPHER_WPI,
    .wm_header  = WIFINET_WAPI_EXHDR_LEN,
    .wm_trailer = WIFINET_WAPI_MIC_LEN,
    .wm_miclen  = 0,
    .wm_attach  = wpi_attach,
    .wm_detach  = wpi_detach,
    .wm_setkey  = wpi_setkey,
    .wm_encap   = wpi_encap,
    .wm_decap   = wpi_decap,
    .wm_enmic   = wpi_enmic,
    .wm_demic   = wpi_demic,
};

const struct wifi_mac_security wifi_mac_cipher_ccmp =
{
    .wm_name    = "AES-CCM",
    .wm_cipher  = WIFINET_CIPHER_AES_CCM,
    .wm_header  = WIFINET_WEP_IVLEN + WIFINET_WEP_KIDLEN + WIFINET_WEP_EXTIVLEN,
    .wm_trailer = WIFINET_WEP_MICLEN,
    .wm_miclen  = 0,
    .wm_attach  = ccmp_attach,
    .wm_detach  = ccmp_detach,
    .wm_setkey  = ccmp_setkey,
    .wm_encap   = ccmp_encap,
    .wm_decap   = ccmp_decap,
    .wm_enmic   = ccmp_enmic,
    .wm_demic   = ccmp_demic,
};

const struct wifi_mac_security wifi_mac_cipher_cmac =
{
    .wm_name    = "AES-CMAC",
    .wm_cipher  = WIFINET_CIPHER_AES_CMAC,
    .wm_header  = WIFINET_WEP_IVLEN + WIFINET_WEP_KIDLEN + WIFINET_WEP_EXTIVLEN,
    .wm_trailer = WIFINET_WEP_MICLEN,
    .wm_miclen  = 0,
    .wm_attach  = ccmp_attach,
    .wm_detach  = ccmp_detach,
    .wm_setkey  = ccmp_setkey,
    .wm_encap   = ccmp_encap,
    .wm_decap   = ccmp_decap,
    .wm_enmic   = ccmp_enmic,
    .wm_demic   = ccmp_demic,
};

const struct wifi_mac_security wifi_mac_cipher_wep =
{
    .wm_name    = "WEP",
    .wm_cipher  = WIFINET_CIPHER_WEP,
    .wm_header  = WIFINET_WEP_IVLEN + WIFINET_WEP_KIDLEN,
    .wm_trailer = WIFINET_WEP_CRCLEN,
    .wm_miclen  = 0,
    .wm_attach  = wep_attach,
    .wm_detach  = wep_detach,
    .wm_setkey  = wep_setkey,
    .wm_encap   = wep_encap,
    .wm_decap   = wep_decap,
    .wm_enmic   = wep_enmic,
    .wm_demic   = wep_demic,
};

const struct wifi_mac_security wifi_mac_cipher_tkip  =
{
    .wm_name    = "TKIP",
    .wm_cipher  = WIFINET_CIPHER_TKIP,
    .wm_header  = WIFINET_WEP_IVLEN + WIFINET_WEP_KIDLEN +
    WIFINET_WEP_EXTIVLEN,
    .wm_trailer = WIFINET_WEP_CRCLEN,
    .wm_miclen  = 0, //WIFINET_WEP_MICLEN,
    .wm_attach  = tkip_attach,
    .wm_detach  = tkip_detach,
    .wm_setkey  = tkip_setkey,
    .wm_encap   = tkip_encap,
    .wm_decap   = tkip_decap,
    .wm_enmic   = tkip_enmic,
    .wm_demic   = tkip_demic,
};

int wifi_mac_key_delete(struct wlan_net_vif *wnet_vif, const struct wifi_mac_key *k,
    struct wifi_station *stanfo)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    int staid = 0;
    DPRINTF(AML_DEBUG_WARNING, "%s: delete key stanfo->sta_associd:%d\n", __func__, stanfo->sta_associd);

    staid = stanfo->sta_associd & 0xff;
    wifimac->drv_priv->drv_ops.key_delete(wifimac->drv_priv,wnet_vif->wnet_vif_id, k->wk_keyix, staid);
    return 1;
}

int wifi_mac_key_set(struct wlan_net_vif *wnet_vif, const struct wifi_mac_key *k,
    const unsigned char peermac[WIFINET_ADDR_LEN])
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    static const unsigned char ciphermap[] =
    {
        HAL_KEY_TYPE_WEP,
        HAL_KEY_TYPE_TKIP,
        HAL_KEY_TYPE_AES,
        HAL_KEY_TYPE_AES,
        HAL_KEY_TYPE_WPI,
        HAL_KEY_TYPE_CLEAR,
    };
    const struct wifi_mac_security *cip = k->wk_cipher;
    unsigned char gmac[WIFINET_ADDR_LEN];
    const unsigned char *mac = NULL;
    struct hal_key_val hk;
    int opmode, status = 1;

    DPRINTF(AML_DEBUG_KEY, "<%s> %s %d \n",wnet_vif->vm_ndev->name, __func__,__LINE__);
    ASSERT(cip != NULL);
    if (cip == NULL)
        return 0;

    if (k->wk_keyix == WIFINET_KEYIX_NONE)
        return 0;

    opmode = wnet_vif->vm_opmode;
    memset(&hk, 0, sizeof(hk));
    hk.kv_pad = opmode;

    KASSERT(cip->wm_cipher < (sizeof(ciphermap)/sizeof(ciphermap[0])), ("invalid cipher type %u", cip->wm_cipher));
    hk.kv_type = ciphermap[cip->wm_cipher];
    hk.kv_len = k->wk_keylen;
    memcpy(hk.kv_val, k->wk_key, k->wk_keylen);

    wnet_vif->current_keytype = cip->wm_cipher;
    if (k->wk_flags & WIFINET_KEY_GROUP) {
        DPRINTF(AML_DEBUG_KEY, "<%s> %s %d set group key \n", wnet_vif->vm_ndev->name,__func__,__LINE__);

        switch (opmode) {
            case WIFINET_M_STA:
                WIFINET_ADDR_COPY(gmac, peermac);
                gmac[0] |= 0x01;
                mac = gmac;
                if (hk.kv_type == HAL_KEY_TYPE_WEP) {
                    struct wifi_station *sta;
                    sta = (wnet_vif->vm_mainsta);
                    memcpy(&sta->sta_ucastkey,k,sizeof(struct wifi_mac_key));
                    sta->sta_ucastkey.wk_flags |= WIFINET_KEY_NOFREE;
                }
                break;

            case WIFINET_M_IBSS:
                DPRINTF(AML_DEBUG_KEY, "<%s> %s %d \n",wnet_vif->vm_ndev->name, __func__,__LINE__);

                if (k->wk_flags & WIFINET_KEY_RECV) {
                    if (k->wk_flags & WIFINET_KEY_PERSTA) {
                        ASSERT(k->wk_keyix >= WIFINET_WEP_NKID);
                        DPRINTF(AML_DEBUG_KEY, "<running> %s %d \n",__func__,__LINE__);
                        WIFINET_ADDR_COPY(gmac, peermac);
                        gmac[0] |= 0x01;
                        mac = gmac;

                    } else {
                        DPRINTF( AML_DEBUG_KEY, "<running> %s %d \n",__func__,__LINE__);
                        WIFINET_ADDR_COPY(gmac, peermac);
                        gmac[0] |= 0x01;
                        mac = gmac;
                    }

                } else if (k->wk_flags & WIFINET_KEY_XMIT) {
                    WIFINET_ADDR_COPY(gmac, peermac);
                    gmac[0] |= 0x01;
                    mac = gmac;

                } else {
                    ASSERT(0);
                    status = 0;
                    goto done;
                }
                break;

            case WIFINET_M_HOSTAP:
                if (hk.kv_type == HAL_KEY_TYPE_WEP) {
                    struct wifi_station *sta;
                    sta = (wnet_vif->vm_mainsta);
                    memcpy(&sta->sta_ucastkey,k,sizeof(struct wifi_mac_key));
                    sta->sta_ucastkey.wk_flags |= WIFINET_KEY_NOFREE;
                }
                WIFINET_ADDR_COPY(gmac, peermac);
                gmac[0] |= 0x01;
                mac = gmac;
                break;

            default:
                ASSERT(0);
                break;
        }
    } else {
        DPRINTF(AML_DEBUG_KEY, "<running> %s %d k->wk_keyix = %d\n",
                __func__,__LINE__,k->wk_keyix);
        //ASSERT(k->wk_keyix <= WIFINET_WEP_NKID);
        if ((hk.kv_type == HAL_KEY_TYPE_WEP)||(opmode == WIFINET_M_IBSS)) {
            WIFINET_ADDR_COPY(gmac, peermac);
            gmac[0] |= 0x01;
            mac = gmac;

        } else {
            mac = peermac;
        }
    }

    if (hk.kv_type == HAL_KEY_TYPE_TKIP) {
        status = wifi_mac_keyset_tkip(wifimac,wnet_vif->wnet_vif_id, k, &hk, mac);

    } else {
        DPRINTF( AML_DEBUG_KEY, "<%s> %s %d \n",wnet_vif->vm_ndev->name,__func__,__LINE__);
        status = (wifimac->drv_priv->drv_ops.key_set(wifimac->drv_priv,wnet_vif->wnet_vif_id,
                    k->wk_keyix, &hk, mac) != 0);
        DPRINTF(AML_DEBUG_KEY, "<running> %s %d %d\n",__func__,__LINE__,status);
    }

    if ((mac != NULL) && (cip->wm_cipher == WIFINET_CIPHER_TKIP) && ((k->wk_flags & WIFINET_KEY_GROUP) == 0))
    {
        struct wifi_station *sta;
        if (opmode == WIFINET_M_STA)
        {
            sta = wnet_vif->vm_mainsta;
        }
        else
            sta = wifi_mac_get_sta(&wnet_vif->vm_sta_tbl, mac,wnet_vif->wnet_vif_id);
    }

done:
    return status;
}

int wifi_mac_keyset_tkip(struct wifi_mac *wifimac, unsigned char wnet_vif_id,
    const struct wifi_mac_key *k, struct hal_key_val *hk, const unsigned char mac[WIFINET_ADDR_LEN])
{
    memcpy(hk->kv_mic, k->wk_rxmic, sizeof(hk->kv_mic));
    memcpy(hk->kv_txmic, k->wk_txmic, sizeof(hk->kv_txmic));
    DPRINTF( AML_DEBUG_KEY, "<running> %s %d \n",__func__,__LINE__);
    return (wifimac->drv_priv->drv_ops.key_set(wifimac->drv_priv,
                wnet_vif_id,k->wk_keyix, hk, mac));
}

void wifi_mac_security_attach(struct wifi_mac *wifimac)
{
    security_type[WIFINET_CIPHER_NONE] = &wifi_mac_cipher_none;
    security_type[WIFINET_CIPHER_WPI] = &wifi_mac_cipher_wpi;
    security_type[WIFINET_CIPHER_AES_CCM] = &wifi_mac_cipher_ccmp;
    security_type[WIFINET_CIPHER_AES_CMAC] = &wifi_mac_cipher_cmac;
    security_type[WIFINET_CIPHER_WEP] = &wifi_mac_cipher_wep;
    security_type[WIFINET_CIPHER_TKIP] = &wifi_mac_cipher_tkip;
}

