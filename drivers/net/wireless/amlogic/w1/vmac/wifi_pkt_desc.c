#include "wifi_drv_main.h"
#include "wifi_pkt_desc.h"
#include "wifi_mac_if.h"

struct drv_txdesc *wifi_mac_alloc_txdesc(struct wifi_mac *wifimac)
{
    struct drv_txdesc *ptxdesc = NULL;
    struct hal_private *hal_priv = hal_get_priv();
    static unsigned char last_free_page_count = 0;
    static unsigned char free_page_hold_count = 0;
    static unsigned short print_count = 0;

    TX_DESC_BUF_LOCK(wifimac);
    if (list_empty(&wifimac->txdesc_freequeue)) {
        TX_DESC_BUF_UNLOCK(wifimac);
        if (!(print_count++ % 20000)) {
            DPRINTF(AML_DEBUG_WARNING, "%s no tx_desc, bhaltxdrop:%d, bhalPowerSave:%d, page:%d, drv_ps_status:%d, fw_ps_status:%d\n",
                __func__, hal_priv->bhaltxdrop, hal_priv->bhalPowerSave, hal_priv->txPageFreeNum, hal_priv->hal_drv_ps_status, hal_priv->hal_fw_ps_status);
        }

        if (last_free_page_count == hal_priv->txPageFreeNum) {
            free_page_hold_count++;

        } else {
            last_free_page_count = hal_priv->txPageFreeNum;
            free_page_hold_count = 0;
        }

        if (free_page_hold_count >= 50) {
            //pr_debug("free_page_hold_count lager than 50, maybe we can reboot here\n");
        }
        return NULL;
    }

    ptxdesc = list_first_entry(&wifimac->txdesc_freequeue, struct drv_txdesc, txdesc_queue);
    list_del_init(&ptxdesc->txdesc_queue);
    TX_DESC_BUF_UNLOCK(wifimac);

    return ptxdesc;
}

void wifi_mac_recycle_txdesc(struct sk_buff *skbbuf)
{
    struct wifi_mac_tx_info *txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skbbuf);
    struct list_head tx_queue;
    struct wifi_mac *wifimac = NULL;

    wifimac = wifi_mac_get_mac_handle();

    //pr_debug("%s ptxdesc:%p, txinfo:%p\n", __func__, txinfo->ptxdesc, txinfo);
    INIT_LIST_HEAD(&tx_queue);

    if (txinfo->ptxdesc != NULL) {
        list_add_tail(&txinfo->ptxdesc->txdesc_queue, &tx_queue);
        memset(txinfo->ptxdesc, 0, sizeof(struct drv_txdesc));

        TX_DESC_BUF_LOCK(wifimac);
        list_splice_tail_init(&tx_queue, &wifimac->txdesc_freequeue);
        TX_DESC_BUF_UNLOCK(wifimac);

        txinfo->ptxdesc = NULL;
    }
}

struct sk_buff *wifi_mac_alloc_skb(struct wifi_mac *wifimac, unsigned int size)
{
    struct sk_buff *skb = NULL;
    struct drv_txdesc *ptxdesc = NULL;
    struct wifi_mac_tx_info *txinfo = NULL;

    skb = os_skb_alloc(size + 64 + HI_TXDESC_DATAOFFSET);
    if (skb == NULL) {
        return NULL;
    }
    txinfo = (struct wifi_mac_tx_info *)os_skb_cb(skb);

    ptxdesc = wifi_mac_alloc_txdesc(wifimac);
    if (ptxdesc == NULL) {
        os_skb_free(skb);
        return NULL;
    }

    skb_reserve(skb, HI_TXDESC_DATAOFFSET + 32);
    txinfo->ptxdesc = ptxdesc;
    return skb;
}

void wifi_mac_free_skb(struct sk_buff *skb) {
    wifi_mac_recycle_txdesc(skb);
    os_skb_free(skb);
    return;
}
