#include "wifi_mac_com.h"
#include "crypto/aes_wrap.h"
#include "crypto/wlancrypto_wrap.h"

#ifdef AML_WPA3

extern unsigned int fi_ahb_read(unsigned int addr);
extern void fi_ahb_write(unsigned int addr,unsigned int data);

void wifi_mac_disable_hw_mgmt_decrypt(void) {
    unsigned int reg_val;

    pr_debug("%s\n", __func__);
    reg_val = fi_ahb_read(0x0a000308);
    fi_ahb_write(0x0a000308, reg_val | BIT(28));

    reg_val = fi_ahb_read(0x0a000314);
    fi_ahb_write(0x0a000314, reg_val | BIT(28));

    reg_val = fi_ahb_read(0x0a000320);
    fi_ahb_write(0x0a000320, reg_val | BIT(28));

    reg_val = fi_ahb_read(0x0a00032c);
    fi_ahb_write(0x0a00032c, reg_val | BIT(28));

    reg_val = fi_ahb_read(0x0a000338);
    fi_ahb_write(0x0a000338, reg_val | BIT(28));

    reg_val = fi_ahb_read(0x0a000340);
    fi_ahb_write(0x0a000340, reg_val | BIT(28));

    reg_val = fi_ahb_read(0x0a000354);
    fi_ahb_write(0x0a000354, reg_val | BIT(28));
}

static unsigned char _bip_ccmp_protect(const unsigned char *key, size_t key_len,
    const unsigned char *data, size_t data_len, unsigned char *mic)
{
    unsigned char res = 1;

    if (key_len == 16) {
        if (omac1_aes_128(key, data, data_len, mic)) {
            res = 0;
            pr_err("%s : omac1_aes_128 fail!", __func__);
        }

    } else if (key_len == 32) {
        if (omac1_aes_256(key, data, data_len, mic)) {
            res = 0;
            pr_err("%s : omac1_aes_256 fail!", __func__);
        }

    } else {
        pr_warn("%s : key_len not match!", __func__);
        res = 0;
    }

    return  res;
}

static unsigned char aml_calculate_bip_mic(unsigned char gmcs, unsigned char *whdr_pos, int len,
    const unsigned char *key, const unsigned char *data, size_t data_len, unsigned char *mic)
{
    unsigned char res = 1;

    if (gmcs == WIFINET_CIPHER_AES_CMAC) {
        if (_bip_ccmp_protect(key, 16, data, data_len, mic) == 0) {
            res = 0;
            pr_err("%s : _bip_ccmp_protect(128) fail!", __func__);
        }

    } else {
        res = 0;
        pr_err("%s : unsupport dot11wCipher !\n", __func__);
    }

    return res;
}

static void wifi_mac_dump_pkt(void *pkt, unsigned short pkt_len) {
    int pp;
    int len = pkt_len;
    unsigned char *frm = (unsigned char *)pkt;

    pr_debug("pktlen = %d\n", pkt_len);
    for (pp = 0; pp < pkt_len; pp++) {
        if ((len > 6) && (pp < 4)) {
            pr_debug("%02x:%02x:%02x:%02x", frm[pp], frm[pp + 1], frm[pp + 2], frm[pp + 3]);
            len -= 4;
            pp += 3;

        } else if (len > 6) {
            pr_debug("%02x:%02x:%02x:%02x:%02x:%02x",
                frm[pp], frm[pp + 1], frm[pp + 2], frm[pp + 3], frm[pp + 4], frm[pp + 5]);
            len -= 6;
            pp += 5;

        } else {
            pr_debug("%02x ", frm[pp]);
        }
    }
    pr_debug("\n");
}

int wifi_mac_parse_mmie(struct wifi_station *sta, struct sk_buff *skb, unsigned char subtype)
{
    unsigned char *bip_aad;
    unsigned char *mme;
    unsigned char *frm;
    unsigned int res = 0;
    unsigned int ori_len;
    unsigned short pkt_keyid = 0;
    unsigned long pkt_ipn = 0;
    unsigned long *local_ipn = 0;
    struct wifi_frame *wh;
    unsigned char mic[16];
    unsigned char mic_len;
    unsigned char mme_offset;
    unsigned short pkt_len = skb->len;
    unsigned char hdr_len = sizeof(struct wifi_frame);
    unsigned char i;

    if (sta->pmf_key.wk_key_type == WIFINET_CIPHER_AES_CMAC) {
        mic_len = 8;

    } else {
        mic_len = 16;
    }

    frm = skb->data;
    wh = (struct wifi_frame *)os_skb_data(skb);

    #ifdef PMF_PKT_PRINT
        /* dump the packet content before decrypt */
        pr_debug("before parse:%d\n", skb->len);
        wifi_mac_dump_pkt(frm, skb->len);
    #endif


    if ((pkt_len < hdr_len) || (pkt_len - hdr_len < mic_len)) {
        ERROR_DEBUG_OUT("pkt_len too small\n");
        return -1;
    }

    mme_offset = (mic_len == 8) ? 18 : 26;
    mme = frm + pkt_len - mme_offset;

    if (*mme != MME_IE_ID) {
        ERROR_DEBUG_OUT("not mmie:%d\n", *mme);
        return -1;
    }

    /* copy key index */
    memcpy(&pkt_keyid, mme + 2, 2);
    pkt_keyid = le16_to_cpu(pkt_keyid);
    if (pkt_keyid != sta->pmf_key.wk_keyix) {
        ERROR_DEBUG_OUT("BIP key index error!\n");
        return -1;
    }

    /* save packet number */
    memcpy(&pkt_ipn, mme + 4, sizeof(unsigned long));
    pkt_ipn = le64_to_cpu(pkt_ipn);
    local_ipn = (unsigned long *)(sta->pmf_key.wk_keyrsc);
    /* BIP packet number should bigger than previous BIP packet */
    if (pkt_ipn <= *local_ipn) { /* wrap around? */
        ERROR_DEBUG_OUT("replay BIP packet, pkt_ipn%08x, local_ipn:%08x\n", pkt_ipn, *local_ipn);
        return -1;
    }

    ori_len = pkt_len - hdr_len + BIP_AAD_SIZE;
    bip_aad = ZMALLOC(ori_len, "bip_add", GFP_ATOMIC);
    if (bip_aad == NULL) {
        ERROR_DEBUG_OUT("BIP AAD allocate fail\n");
        return -1;
    }

    /* save the frame body + MME (w/o mic) */
    memcpy(bip_aad + BIP_AAD_SIZE, frm + hdr_len, pkt_len - hdr_len - mic_len);

    /* construct AAD, copy frame control field */
    memcpy(bip_aad, &wh->i_fc[0], 2);
    clear_retry(bip_aad);
    clear_pwr_mgt(bip_aad);
    clear_more_data(bip_aad);
    /* construct AAD, copy address 1 to address 3 */
    memcpy(bip_aad + 2, wh->i_addr1, 6);
    memcpy(bip_aad + 8, wh->i_addr2, 6);
    memcpy(bip_aad + 14, wh->i_addr3, 6);

    pr_debug("%s bip_add before\n", __func__);
    for (i = 0; i < ori_len; ++i) {
        pr_debug("%02x", bip_aad[i]);
    }
    pr_debug("\n");

    if (aml_calculate_bip_mic(sta->pmf_key.wk_key_type, frm,
        pkt_len, sta->pmf_key.wk_key, bip_aad, ori_len, mic) == 0)
        goto bip_exit;

    /* MIC field should be last 8 bytes of packet (packet without FCS) */
    if (memcmp(mic, wh + pkt_len - mic_len, mic_len)) {
        *local_ipn = pkt_ipn;
        res = 1;
    } else {
        ERROR_DEBUG_OUT("BIP MIC error!\n");
    }

    #ifdef PMF_PKT_PRINT
        /* dump the packet content before decrypt */
        pr_debug("after parse:%d\n", skb->len);
        wifi_mac_dump_pkt(frm, skb->len);
    #endif


bip_exit:
    FREE(bip_aad, "bip_aad");
    return res;
}

static int aml_aes_decrypt(struct wifi_station *sta, struct sk_buff *skb, unsigned char key_index)
{
    unsigned char *frm;
    unsigned char *key;
    struct ieee80211_hdr *hdr;
    unsigned char hdr_len;
    unsigned char key_len;
    unsigned char *plain = NULL;
    size_t plain_len = 0;

    frm = skb->data;
    hdr = (struct ieee80211_hdr *)os_skb_data(skb);
    hdr_len = sizeof(struct wifi_frame);
    key_len = sta->sta_ucastkey.wk_keylen;
    key = sta->sta_ucastkey.wk_key;

    if (key_len == 16) { /* 128 bits */
        plain = ccmp_decrypt(key, hdr, frm + hdr_len, /* PN + enc_data + MIC */
            skb->len - hdr_len, /* PN + enc_data + MIC */ &plain_len);

    } else if (key_len == 32) { /* 256 bits */
        plain = ccmp_256_decrypt(key, hdr, frm + hdr_len, /* PN + enc_data + MIC */
            skb->len - hdr_len, /* PN + enc_data + MIC */&plain_len);
    }

    if (plain == NULL) {
        ERROR_DEBUG_OUT("fail\n");
        return 1;
    }

    /* Copy @plain back to @frame and free @plain */
    memcpy(frm + hdr_len + 8, plain, plain_len);
    FREE(plain, "plain");

    return 0;
}

static int aml_pkt_decrypt(struct wifi_station *sta, struct sk_buff *skb)
{
    int res = 0;
    unsigned char hdr_len = sizeof(struct wifi_frame);
    unsigned char *iv = skb->data + hdr_len;
    unsigned char key_index = (((iv[3]) >> 6) & 0x3);

    switch (sta->sta_rsn.rsn_ucastcipher) {
        case WIFINET_CIPHER_AES_OCB:
        case WIFINET_CIPHER_AES_CCM:
        case WIFINET_CIPHER_AES_CMAC:
            res = aml_aes_decrypt(sta, skb, key_index);
            break;
        default:
            break;
    }

    return res;
}

unsigned char is_robust_management_frame(struct wifi_frame *wh)
{
    unsigned char type, subtype;
    unsigned char category;

    type = wh->i_fc[0] & WIFINET_FC0_TYPE_MASK;
    subtype = wh->i_fc[0] & WIFINET_FC0_SUBTYPE_MASK;

    if (type != WIFINET_FC0_TYPE_MGT) {
        return 0;
    }
    if (subtype == WIFINET_FC0_SUBTYPE_ACTION) {
        category = *((unsigned char *)wh + sizeof(struct wifi_frame));
    }

    if ((subtype != WIFINET_FC0_SUBTYPE_DISASSOC) && (subtype != WIFINET_FC0_SUBTYPE_DEAUTH)
        && (subtype != WIFINET_FC0_SUBTYPE_ACTION)) {
        return 0;
    }

    if (subtype == WIFINET_FC0_SUBTYPE_ACTION) {
        if (CATEGORY_IS_NON_ROBUST(category))
            return 0;
    }

    return 1;
}

int wifi_mac_unprotected_mgmt_pkt_handle(struct wifi_station *sta, struct sk_buff *skb, unsigned char subtype)
{
    struct wifi_frame *wh;
    unsigned char *frm;
    int reason = 0;
    unsigned char is_bmc;
    unsigned char category;
    struct wifi_station *remote_sta = NULL;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;

    wh = (struct wifi_frame *)os_skb_data(skb);
    frm = (unsigned char *)&wh[1];
    is_bmc = (WIFINET_IS_MULTICAST(wh->i_addr1));

    //pr_debug("%s recv :%d\n", __func__, subtype);
    switch (wnet_vif->vm_opmode) {
        case WIFINET_M_STA:
            if ((is_bmc) && (subtype != WIFINET_FC0_SUBTYPE_BEACON)) {
                return 0;
            }

            if ((subtype == WIFINET_FC0_SUBTYPE_DEAUTH) || (subtype == WIFINET_FC0_SUBTYPE_DISASSOC)) {
                if (!(sta->sta_flags_ext & WIFINET_NODE_MFP_CONFIRM_DEAUTH)) {
                    reason = le16toh(*(unsigned short *)frm);
                    pr_debug("%s recv deauthenticate (reason %d)\n", __func__, reason);

                    wifi_mac_send_sa_query(sta, WIFINET_ACTION_SA_QUERY_REQ, sta->sa_query_seq++);
                    os_timer_ex_start_period(&wnet_vif->vm_actsend, 100);
                    sta->sta_flags_ext |= WIFINET_NODE_MFP_CONFIRM_DEAUTH;
                }

            } else if (subtype == WIFINET_FC0_SUBTYPE_ACTION) {
                category = *((unsigned char *)wh + sizeof(struct wifi_frame));
                if (CATEGORY_IS_NON_ROBUST(category)) {
                    return 1;

                } else {
                    return 0;
                }

            } else {
                return 1;
            }
            break;

        case WIFINET_M_HOSTAP:
            remote_sta = wifi_mac_find_tx_sta(wnet_vif, wh->i_addr2);
            if ((remote_sta == NULL) || ((remote_sta != NULL) && (remote_sta->sta_associd == 0))) {
                return 1;
            }

            if (subtype == WIFINET_FC0_SUBTYPE_AUTH) {
                reason = (WIFINET_STATUS_REFUSED_TEMPORARILY << 16);
                wifi_mac_send_mgmt(remote_sta, WIFINET_FC0_SUBTYPE_AUTH, (void *)&reason);

            } else if (subtype == WIFINET_FC0_SUBTYPE_ASSOC_REQ) {
                reason = WIFINET_STATUS_REFUSED_TEMPORARILY;
                wifi_mac_send_mgmt(remote_sta, WIFINET_FC0_SUBTYPE_ASSOC_RESP, (void *)&reason);

            } else if (subtype == WIFINET_FC0_SUBTYPE_REASSOC_REQ) {
                reason = WIFINET_STATUS_REFUSED_TEMPORARILY;
                wifi_mac_send_mgmt(remote_sta, WIFINET_FC0_SUBTYPE_REASSOC_RESP, (void *)&reason);

            } else {
                return 0;
            }

            if (!(remote_sta->sta_flags_ext & WIFINET_NODE_MFP_CONFIRM_DEAUTH)) {
                remote_sta->sta_flags_ext |= WIFINET_NODE_MFP_CONFIRM_DEAUTH;
                wifi_mac_send_sa_query(remote_sta, WIFINET_ACTION_SA_QUERY_REQ, remote_sta->sa_query_seq++);
                os_timer_ex_start_period(&wnet_vif->vm_actsend, 100);
            }
            break;

        default:
            break;
    }

    return 0;
}

int wifi_mac_mgmt_validate(struct wifi_station *sta, struct sk_buff *skb, unsigned char subtype) {
    unsigned char *frm;
    struct wifi_frame *wh;
    unsigned char is_bmc;
    unsigned short data_len;
    unsigned char iv_len;
    unsigned char icv_len;
    unsigned char *mgmt_data;
    unsigned char hdr_len;
    unsigned short decrypt_res;

    frm = skb->data;
    wh = (struct wifi_frame *)os_skb_data(skb);
    hdr_len = sizeof(struct wifi_frame);
    is_bmc = (WIFINET_IS_MULTICAST(wh->i_addr1));

    if (!is_robust_management_frame(wh)) {
        return 1;
    }

    /* cases to decrypt mgmt frame */
    /* set iv and icv length */
    SET_ICE_IV_LEN(&iv_len, &icv_len, sta->sta_rsn.rsn_ucastcipher);

    /* actual management data frame body */
    data_len = skb->len - hdr_len - iv_len - icv_len;
    mgmt_data = ZMALLOC(data_len, "mgmt_data", GFP_ATOMIC);
    if (mgmt_data == NULL) {
        ERROR_DEBUG_OUT("mgmt_data alloc fail\n");
        goto fail;
    }

    #ifdef PMF_PKT_PRINT
        /* dump the packet content before decrypt */
        pr_debug("rsn_ucastcipher:%d\n", sta->sta_rsn.rsn_ucastcipher);
        wifi_mac_dump_pkt(frm, skb->len);
    #endif

    decrypt_res = aml_pkt_decrypt(sta, skb);
    if (decrypt_res) {
        ERROR_DEBUG_OUT("decrypt mgmt pkt fail\n");
        FREE(mgmt_data, "mgmt_data");
        goto fail;
    }
    /* save actual management data frame body */
    memcpy(mgmt_data, frm + hdr_len + iv_len, data_len);
    /* overwrite the iv field */
    memcpy(frm + hdr_len, mgmt_data, data_len);
    /* remove the iv and icv length */
    skb->len = skb->len - iv_len - icv_len;
    FREE(mgmt_data, "mgmt_data");

    #ifdef PMF_PKT_PRINT
        /* print packet content after decryption */
        pr_debug("after decrypt\n");
        wifi_mac_dump_pkt(frm, skb->len);
    #endif

    if (is_bmc) {
        pr_debug("parse mmie for broadcast mgmt pkt\n");
        if (wifi_mac_parse_mmie(sta, skb, subtype) != 1) {
            goto fail;
        }
    }

    return 0;

fail:
    return 1;
}

int wifi_mac_add_mmie(struct wifi_station *sta, struct sk_buff *skb) {
    unsigned char *frm;
    struct wifi_frame *wh;
    unsigned short ori_len;
    unsigned char mme_cont[MME_IE_LENGTH] = {0};
    unsigned char mme_clen;
    unsigned char *mgmt_body;
    unsigned char *frm_end;
    unsigned char *bip_aad;
    unsigned short pkt_len_before_add_ie = skb->data_len;
    unsigned short frame_body_len;

    frm = skb->data;
    wh = (struct wifi_frame *)os_skb_data(skb);
    frame_body_len = skb->len - sizeof(struct wifi_frame);
    ori_len = BIP_AAD_SIZE + frame_body_len;

    bip_aad = ZMALLOC(ori_len, "bip_add", GFP_ATOMIC);
    if (bip_aad == NULL) {
        ERROR_DEBUG_OUT("bip alloc fail\n");
        return 1;
    }
    pr_debug("%s len:%d, frame_body_len:%d\n", __func__, skb->len, frame_body_len);

    {
        /* broadcast robust mgmt frame, using BIP */
        unsigned char mic[16] = {0};

        /* IGTK key is not install ex: mesh MFP without IGTK */
        if (sta->pmf_key.wk_valid != 1) {
            FREE(bip_aad, "bip_aad");
            return 1;
        }

        if (sta->pmf_key.wk_key_type == WIFINET_CIPHER_AES_CMAC) {
            mme_clen = 16;

        } else {
            mme_clen = 24;
        }

        mgmt_body = frm + sizeof(struct wifi_frame);
        frm_end = frm + skb->len - mme_clen - 2;

        #ifdef PMF_PKT_PRINT
            /* dump the packet content before decrypt */
            pr_debug("before add aad:%d\n", skb->len);
            wifi_mac_dump_pkt(frm, skb->len);
        #endif

        mme_cont[0] = MME_IE_ID;
        mme_cont[1] = mme_clen;
        /* octent 0 and 1 is key index ,BIP keyid is 4 or 5, LSB only need octent 0 */
        mme_cont[2] = sta->pmf_key.wk_keyix;

        /* increase PN and apply to packet */
        sta->pmf_key.wk_keyrsc[5]++;
        memcpy(&mme_cont[4], &sta->pmf_key.wk_keyrsc[0], 6);

        /* add MME IE with MIC all zero, MME string doesn't include element id and length */
        memcpy(frm_end, mme_cont, mme_clen + 2);

        /* construct AAD, copy frame control field */
        memcpy(bip_aad, &wh->i_fc[0], 2);
        clear_retry(bip_aad);
        clear_pwr_mgt(bip_aad);
        clear_more_data(bip_aad);
        /* construct AAD, copy address 1 to address 3 */
        memcpy(bip_aad + 2, wh->i_addr1, 6);
        memcpy(bip_aad + 8, wh->i_addr2, 6);
        memcpy(bip_aad + 14, wh->i_addr3, 6);
        /* copy management frame body */
        memcpy(bip_aad + BIP_AAD_SIZE, mgmt_body, frame_body_len);

        /* dump total packet include MME with zero MIC */
        #if 0
            pr_debug("bip_aad:%d\n", ori_len);
            wifi_mac_dump_pkt(bip_add, ori_len);
        #endif

        /* calculate mic */
        if (aml_calculate_bip_mic(sta->pmf_key.wk_key_type, (unsigned char *)wh, pkt_len_before_add_ie,
            sta->pmf_key.wk_key, bip_aad, ori_len, mic) == 0)
                goto add_fail;

        /* dump calculated mic result */
        if (0) {
            int i;
            pr_debug("Calculated mic result: ");
            for (i = 0; i < 16; i++)
                pr_debug(" %02x ", mic[i]);
            pr_debug("\n");
        }

        frm_end = frm + skb->len;
        /* copy right BIP mic value, total is 128bits, we use the 0~63 bits */
        if (sta->pmf_key.wk_key_type == WIFINET_CIPHER_AES_CMAC)
            memcpy(frm_end - 8, mic, 8);
        else
            memcpy(frm_end - 16, mic, 16);

        #ifdef PMF_PKT_PRINT
            pr_debug("after add mic:%d\n", skb->len);
            wifi_mac_dump_pkt(frm, skb->len);
        #endif
    }

    FREE(bip_aad, "bip_aad");
    return 0;

add_fail:
    FREE(bip_aad, "bip_aad");
    return 1;
}

static int _aml_ccmp_encrypt(struct wifi_mac_key *key, unsigned char *pn, unsigned int hdrlen, unsigned char *frame, uint plen)
{
    unsigned char *enc = NULL;
    size_t enc_len = 0;

    if (key->wk_keylen == 16) { /* 128 bits */
        enc = ccmp_encrypt(key->wk_key, frame, hdrlen + plen, hdrlen,
            (hdrlen == 26) ? (frame + hdrlen - 2) : NULL, pn, key->wk_keyix, &enc_len);

    } else if (key->wk_keylen == 32) { /* 256 bits */
        enc = ccmp_256_encrypt(key->wk_key, frame, hdrlen + plen, hdrlen,
            (hdrlen == 26) ? (frame + hdrlen - 2) : NULL, pn, key->wk_keyix, &enc_len);
    }

    if (enc == NULL) {
        ERROR_DEBUG_OUT("Failed to encrypt CCMP(%d) frame", key->wk_keylen);
        return 0;
    }

    /* Copy @enc back to @frame and free @enc */
    memcpy(frame, enc, enc_len);
    FREE(enc, "enc");

    return 1;
}

int wifi_mac_sw_encrypt(struct wifi_station *sta, struct sk_buff *skb) {
    int res = 0;
    unsigned char hdr_len;
    unsigned char key_index;
    unsigned char is_bmc;
    unsigned char iv_len;
    unsigned char icv_len;
    unsigned short staid;
    unsigned char *frm = NULL;
    struct wifi_mac_key *key = NULL;
    unsigned char *pn = NULL;
    struct wifi_frame *wh = NULL;
    unsigned long long *PN = NULL;
    struct wlan_net_vif *wnet_vif = sta->sta_wnet_vif;
    struct hal_private *hal_priv = hal_get_priv();

    frm = skb->data;
    wh = (struct wifi_frame *)os_skb_data(skb);
    hdr_len = sizeof(struct wifi_frame);
    is_bmc = (WIFINET_IS_MULTICAST(wh->i_addr1));
    key_index = wnet_vif->vm_def_txkey;

    if (is_bmc) {
        key = &wnet_vif->vm_nw_keys[key_index];
        pn = (unsigned char *)hal_priv->mRepCnt[sta->wnet_vif_id].txPN;

    } else {
        key = &sta->sta_ucastkey;
        staid = (sta->sta_wnet_vif->vm_opmode == WIFINET_M_STA ) ? 1 : (sta->sta_associd & (~0xc000));
        pn = (unsigned char *)hal_priv->uRepCnt[sta->wnet_vif_id][staid].txPN[0];
    }

    #ifdef PMF_PKT_PRINT
        /* dump the packet content before decrypt */
        pr_debug("before encrypt rsn_ucastcipher:%d\n", sta->sta_rsn.rsn_ucastcipher);
        wifi_mac_dump_pkt(frm, skb->len);
    #endif

    SET_ICE_IV_LEN(&iv_len, &icv_len, key->wk_key_type);
    os_skb_put(skb, iv_len + icv_len);

    switch (key->wk_key_type) {
        case WIFINET_CIPHER_AES_OCB:
        case WIFINET_CIPHER_AES_CCM:
        case WIFINET_CIPHER_AES_CMAC:
            res = _aml_ccmp_encrypt(key, pn, hdr_len, frm, (skb->len - hdr_len - iv_len - icv_len)/*data len*/);
            break;

        default:
            break;
    }

    PN = (unsigned long long *)pn;
    (*PN)++;
    //pr_debug("PN is %016x, pn[0]:%02x, pn[7]:%02x\n", *PN, pn[0], pn[7]);

    #ifdef PMF_PKT_PRINT
        /* dump the packet content before decrypt */
        pr_debug("after encrypt:\n");
        wifi_mac_dump_pkt(frm, skb->len);
    #endif

    return res;
}

int wifi_mac_send_sa_query(struct wifi_station *sta, unsigned char action, unsigned short seq) {
    struct wifi_mac_action_mgt_args arg;

    arg.category = AML_CATEGORY_SA_QUERY;
    arg.action = action;
    arg.arg1 = seq;

    pr_debug("%s sta:%p, action:%d\n", __func__, sta, action);
    wifi_mac_send_actionframe(sta->sta_wnet_vif, sta, &arg);
    return 0;
}

#endif//AML_WPA3
