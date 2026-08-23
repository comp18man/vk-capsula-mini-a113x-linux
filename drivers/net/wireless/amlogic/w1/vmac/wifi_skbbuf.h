#ifndef _WIFI_SKBBUF_H_
#define _WIFI_SKBBUF_H_

#include <linux/skbuff.h>

//#define SKBBUF_DEBUG

#define OS_SKB_CB_MAXLEN (48)//kernel skb -> cb len = 48
#define os_skb_data(_skb) ((_skb)->data)
#define os_skb_cb(_skb) ((_skb)->cb)
#define os_skb_get_pktlen(_skb) ((_skb)->len)
#define os_skb_get_tailroom(_skb) skb_tailroom(_skb)
#define os_skb_get_priority(_skb) ((_skb)->priority)
#define os_skb_set_priority(_skb, _p) ((_skb)->priority = (_p))
#define os_skb_hdrspace(_skb) skb_headroom(_skb)
#define os_skb_next(_skb) ((_skb)->next)

#define os_skb_push(_skb, _len) skb_push(_skb, _len)
#define os_skb_trim(_skb, _len) skb_trim(_skb,_len)
#define os_skb_pull(_skb, _len) skb_pull(_skb, _len)
#define os_skb_put(_skb, _len) skb_put(_skb, _len)

#ifdef SKBBUF_DEBUG
extern int skb_alloc_time;
extern int skb_free_time;

#define os_skb_count_alloc(skb) AML_OUTPUT("%s skb:%p, skb_alloc_time:%d\n", __func__, skb, skb_alloc_time++);
#define os_skb_count_free(skb) AML_OUTPUT("%s skb:%p, skb_free_time:%d\n", __func__, skb, skb_free_time++);

#define os_skb_alloc(len) dev_alloc_skb(ALIGN(len, 4)); \
    AML_OUTPUT("%s skb:%p, skb_alloc_time:%d\n", __func__, skb, skb_alloc_time++);
#define os_skb_copy(ori_skb, priority, skb) skb_copy(ori_skb, priority);\
    AML_OUTPUT("%s skb:%p, skb_alloc_time:%d\n", __func__, skb, skb_alloc_time++);
#define os_skb_copy_expand(ori_skb, header_len, tail_len, priority, skb) skb_copy_expand(ori_skb, header_len, tail_len, priority);\
    AML_OUTPUT("%s skb:%p, skb_alloc_time:%d\n", __func__, skb, skb_alloc_time++);

#define os_skb_free(skb) consume_skb(skb);\
    AML_OUTPUT("%s skb:%p, skb_free_time:%d\n", __func__, skb, skb_free_time++);

#else

#define os_skb_count_alloc(skb)
#define os_skb_count_free(skb)
#define os_skb_alloc(_len) dev_alloc_skb(ALIGN(_len, 4));
#define os_skb_free(skbbuf) { consume_skb(skbbuf);  skbbuf = NULL; }
#define os_skb_copy(ori_skb, priority, skb) skb_copy(ori_skb, priority);
#define os_skb_copy_expand(ori_skb, header_len, tail_len, priority, skb) skb_copy_expand(ori_skb, header_len, tail_len, priority);
#endif

#endif//_WIFI_SKBBUF_H_
