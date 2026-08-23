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

#ifndef _NET80211_IEEE80211_CRYPTO_H_
#define _NET80211_IEEE80211_CRYPTO_H_

#include "wifi_drv_if.h"


#define WIFINET_KEYBUF_SIZE 16
#define WIFINET_MICBUF_SIZE (8+8)
#define WIFINET_TID_SIZE  17

#define michael_block(l, r) \
        do {                \
                r ^= rotl(l, 17);   \
                l += r;         \
                r ^= xswap(l);      \
                l += r;         \
                r ^= rotl(l, 3);    \
                l += r;         \
                r ^= rotr(l, 2);    \
                l += r;         \
        } while (0)

struct wifi_mac_security;

#define WIFINET_KEY_XMIT 0x01
#define WIFINET_KEY_RECV 0x02
#define WIFINET_KEY_GROUP 0x04
#define WIFINET_KEY_SWMIC 0x20
#define WIFINET_KEY_PERSTA 0x80
#define WIFINET_KEY_NOFREE 0x40

struct wifi_mac_key
{
    unsigned char wk_keylen;
    unsigned char wk_flags;
    unsigned char wk_key_type;
    unsigned char wk_valid;
    unsigned short wk_keyix;
    unsigned char wk_key[WIFINET_KEYBUF_SIZE + WIFINET_MICBUF_SIZE];

#define wk_txmic wk_key + WIFINET_KEYBUF_SIZE+0
#define wk_rxmic wk_key + WIFINET_KEYBUF_SIZE+8

    unsigned char wk_keyrsc[WIFINET_TID_SIZE];
    u_int64_t wk_keytsc;
    const struct wifi_mac_security *wk_cipher;
    void *wk_private;
    unsigned short wk_clearkeyix;
};
#define WIFINET_KEY_COMMON      \
        (WIFINET_KEY_XMIT | WIFINET_KEY_RECV | WIFINET_KEY_GROUP)

#define WIFINET_CIPHER_WEP 0
#define WIFINET_CIPHER_TKIP 1
#define WIFINET_CIPHER_AES_OCB 2
#define WIFINET_CIPHER_AES_CCM 3
#define WIFINET_CIPHER_WPI 4
#define WIFINET_CIPHER_AES_CMAC 5
#define WIFINET_CIPHER_AES_PSK_256 6
#define WIFINET_CIPHER_AES_SAE 8
#define WIFINET_CIPHER_NONE 9
#define WIFINET_CIPHER_MAX      (WIFINET_CIPHER_NONE+1)
#define WIFINET_KEYIX_NONE  ((unsigned char) -1)

struct wifi_mac_rekey_data
{
    unsigned char  kek[16];
    unsigned char  kck[16];
    unsigned char  replay_counter[8];
};

#if defined(__KERNEL__) || defined(_KERNEL)

struct wifi_mac;
struct wlan_net_vif;
struct wifi_station;
struct sk_buff;

void wifi_mac_security_attach(struct wifi_mac *);
void wifi_mac_security_detach(struct wifi_mac *);
void wifi_mac_security_vattach(struct wlan_net_vif *);
void wifi_mac_security_vdetach(struct wlan_net_vif *);
int wifi_mac_security_req(struct wlan_net_vif *, int cipher, int flags, struct wifi_mac_key *);

int wifi_mac_sec_delt_key(struct wlan_net_vif *, struct wifi_mac_key *, struct wifi_station *);
int wifi_mac_security_setkey(struct wlan_net_vif *, struct wifi_mac_key *,
    const unsigned char macaddr[WIFINET_ADDR_LEN], struct wifi_station *);
int wifi_mac_security_set_rekey_data(struct wlan_net_vif *wnet_vif, struct wifi_mac_rekey_data rekey_data);
void wifi_mac_security_delt_default_key(struct wlan_net_vif *);

struct wifi_mac_security
{
    const char *wm_name;
    unsigned int wm_cipher;
    unsigned int wm_header;
    unsigned int wm_trailer;
    unsigned int wm_miclen;
    void*   (*wm_attach)(struct wlan_net_vif *, struct wifi_mac_key *);
    void    (*wm_detach)(struct wifi_mac_key *);
    int     (*wm_setkey)(struct wifi_mac_key *);
    int     (*wm_encap)(struct wifi_mac_key *, struct sk_buff *, unsigned char keyid);
    int     (*wm_decap)(struct wifi_mac_key *, struct sk_buff *, int);
    int     (*wm_enmic)(struct wifi_mac_key *, struct sk_buff *, int);
    int     (*wm_demic)(struct wifi_mac_key *, struct sk_buff *, int, int);
};
extern  const struct wifi_mac_security wifi_mac_cipher_none;
extern  const struct wifi_mac_security wifi_mac_cipher_wpi;
extern  const struct wifi_mac_security wifi_mac_cipher_wep;
extern  const struct wifi_mac_security wifi_mac_cipher_tkip;
extern  const struct wifi_mac_security wifi_mac_cipher_ccmp;

int wifi_mac_security_available(unsigned int cipher);
struct wifi_mac_key *wifi_mac_security_encap(struct wifi_station *, struct sk_buff *);
struct wifi_mac_key *wifi_mac_security_decap(struct wifi_station *, struct sk_buff *, int);

static __inline int wifi_mac_security_demic(struct wlan_net_vif *wnet_vif, struct wifi_mac_key *k,
    struct sk_buff *skb, int hdrlen, int force)
{
    const struct wifi_mac_security *cip = k->wk_cipher;
    return (cip->wm_miclen > 0 ? cip->wm_demic(k, skb, hdrlen, force) : 1);
}

static __inline int wifi_mac_security_enmic(struct wlan_net_vif *wnet_vif,
    struct wifi_mac_key *k, struct sk_buff *skb, int force)
{
    const struct wifi_mac_security *cip = k->wk_cipher;
    return (cip->wm_miclen > 0 ? cip->wm_enmic(k, skb, force) : 1);
}

static __inline void wifi_mac_security_resetkey(struct wlan_net_vif *wnet_vif,
    struct wifi_mac_key *k, unsigned short ix)
{
    k->wk_cipher = &wifi_mac_cipher_none;;
    k->wk_private = k->wk_cipher->wm_attach(wnet_vif, k);
    k->wk_keyix = ix;
    k->wk_flags = WIFINET_KEY_XMIT | WIFINET_KEY_RECV;
}

void wifi_mac_notify_mic_fail(struct wlan_net_vif *, const struct wifi_frame *, unsigned int key_index);
#endif /* defined(__KERNEL__) || defined(_KERNEL) */

int wifi_mac_key_delete(struct wlan_net_vif *wnet_vif, const struct wifi_mac_key *k, struct wifi_station *stanfo);
int wifi_mac_key_set(struct wlan_net_vif *wnet_vif,  const struct wifi_mac_key *k, const unsigned char peermac[WIFINET_ADDR_LEN]);
int wifi_mac_keyset_tkip(struct wifi_mac *wifimac, unsigned char wnet_vif_id, const struct wifi_mac_key *k, struct hal_key_val *hk,
    const unsigned char mac[WIFINET_ADDR_LEN]);

#endif /* _NET80211_IEEE80211_CRYPTO_H_ */
