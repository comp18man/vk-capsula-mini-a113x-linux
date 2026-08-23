/*****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *              Driver interface header file
 * Author : Boatman Yang(xuexing.yang@amlogic.com)
 *
 * Date:    20160901
 ****************************************************************************************
 */
#ifndef _DRV_HAL_THR_DRIVERIF_H
#define _DRV_HAL_THR_DRIVERIF_H

#include "wifi_pt_init.h"
#include "wifi_pt_network.h"

#define THR_BIT(n)    (1U << (n))

#define THR_WIFI_MORE_BUFFER            THR_BIT(0)
#define THR_WIFI_FIRST_BUFFER           THR_BIT(1)
#define THR_WIFI_MORE_AGG               THR_BIT(2)

#define LIST_HEAD_INIT(name) { &(name), &(name) }
//#define LIST_HEAD(name) struct thr_list_head name = LIST_HEAD_INIT(name)
#define INIT_LIST_HEAD(ptr) do { \
                (ptr)->next = (ptr); (ptr)->prev = (ptr); \
        } while (0)

#ifdef _LINUX_LIST_H
#define thr_list_head list_head
#endif

#define OS_LIST			struct thr_list_head
#define OS_LIST_INIT		INIT_LIST_HEAD
#define OS_DEL_HEAD	list_del

#define MAX_SKB_NUM                1000
#define HI_TXPRIVD_NUM_PER_TID     (128)
#define HI_TXPRIVD_SIZE_PER_TID    (HI_TXPRIVD_NUM_PER_TID * sizeof(struct HI_TX_PRIV_HDR))
#define HI_TXPRIVD_NUM_ALL_TID     (MAX_TID * HI_TXPRIVD_NUM_PER_TID)
#define HI_TXPRIVD_SIZE_ALL_TID    (MAX_TID * HI_TXPRIVD_NUM_PER_TID * sizeof(struct HI_TX_PRIV_HDR))

#define HI_RXPRIVD_NUM_ALL_TID     (255)
#define HI_RXPRIVD_SIZE_ALL_TID    (HI_RXPRIVD_NUM_ALL_TID * sizeof(struct HI_RxDescripter))
#define DRIVER_TXPRIV_DESC_LEN     (8+HI_TXPRIVD_NUM_ALL_TID * sizeof(struct _HI_TxPrivDescripter_chain ) )
#define DRIVER_SKBBUFER_LEN        (MAX_SKB_NUM * sizeof(struct sk_buff) )
#define  THR_OS_DDR2DMAAddress(__t)  ((void*)__t)

#ifndef _LINUX_LIST_H
struct thr_list_head {
        struct thr_list_head *next, *prev;
};
#endif

struct _HI_TxPrivDescripter_chain {
        OS_LIST  workList;
        struct hi_tx_priv_hdr HI_TxPriv;
        unsigned char *buffer;
} ;

struct _HI_AGG_TxDescripter_chain {
        OS_LIST workList;
        struct hi_agg_tx_desc HI_AGG_TxD;
} ;

struct _WIFI_htframe {
        unsigned short	    FrameCtrl;
        unsigned char		Duration[2];
        unsigned char		Address1[6];
        unsigned char		Address2[6];
        unsigned char		Address3[6];//WIFI_ADDR_LEN
        unsigned char		SeqCtrl[2];
        unsigned char		QosCtrl[2];
        unsigned char       HtCtrl[4];
};


#ifdef SIM
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({			\
                const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
                (type *)( (char *)__mptr - offsetof(type,member) );})



static __inline void prefetch(const void *x)
{
        ;
}
static __inline void prefetchw(const void *x)
{
        ;
}

#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __list_add_new(struct thr_list_head *new_head,
                                struct thr_list_head *prev,
                                struct thr_list_head *next)
{
        next->prev = new_head;
        new_head->next = next;
        new_head->prev = prev;
        prev->next = new_head;
}

/**
 * list_add - add a new_head entry
 * @new_head: new_head entry to be added
 * @head: list head to add it after
 *
 * Insert a new_head entry after the specified head.
 * This is good for implementing stacks.
 */
static __inline void list_add(struct thr_list_head *new_head, struct thr_list_head *head)
{
        __list_add(new_head, head, head->next);
}


/**
 * list_add_tail - add a new_head entry
 * @new_head: new_head entry to be added
 * @head: list head to add it before
 *
 * Insert a new_head entry before the specified head.
 * This is useful for implementing queues.
 */
void list_add_tail_new(struct thr_list_head *new_head, struct thr_list_head *head)
{
        __list_add_new(new_head, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline void __list_del(struct thr_list_head * prev, struct thr_list_head * next)
{
        next->prev = prev;
        prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static __inline void list_del(struct thr_list_head *entry)
{
        __list_del(entry->prev, entry->next);
        entry->next = (struct thr_list_head *)LIST_POISON1;
        entry->prev = (struct thr_list_head *)LIST_POISON2;
}

/**
 * list_replace - replace old entry by new_head one
 * @old : the element to be replaced
 * @new_head : the new_head element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static __inline void list_replace(struct thr_list_head *old,
                                  struct thr_list_head *new_head)
{
        new_head->next = old->next;
        new_head->next->prev = new_head;
        new_head->prev = old->prev;
        new_head->prev->next = new_head;
}

static __inline void list_replace_init(struct thr_list_head *old,
                                       struct thr_list_head *new_head)
{
        list_replace(old, new_head);
        INIT_LIST_HEAD(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static __inline void list_del_init(struct thr_list_head *entry)
{
        __list_del(entry->prev, entry->next);
        INIT_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static __inline void list_move(struct thr_list_head *list, struct thr_list_head *head)
{
        __list_del(list->prev, list->next);
        list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static __inline void list_move_tail(struct thr_list_head *list,
                                    struct thr_list_head *head)
{
        __list_del(list->prev, list->next);
        list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static __inline int list_is_last(const struct thr_list_head *list,
                                 const struct thr_list_head *head)
{
        return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static __inline int list_empty(const struct thr_list_head *head)
{
        return head->next == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static __inline int list_empty_careful(const struct thr_list_head *head)
{
        struct thr_list_head *next = head->next;
        return (next == head) && (next == head->prev);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static __inline int list_is_singular(const struct thr_list_head *head)
{
        return !list_empty(head) && (head->next == head->prev);
}

static __inline void __list_cut_position(struct thr_list_head *list,
                struct thr_list_head *head, struct thr_list_head *entry)
{
        struct thr_list_head *new_first = entry->next;
        list->next = head->next;
        list->next->prev = list;
        list->prev = entry;
        entry->next = list;
        head->next = new_first;
        new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new_head list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *      and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static __inline void list_cut_position(struct thr_list_head *list,
                                       struct thr_list_head *head, struct thr_list_head *entry)
{
        if (list_empty(head))
                return;
        if (list_is_singular(head) &&
                        (head->next != entry && head != entry))
                return;
        if (entry == head)
                INIT_LIST_HEAD(list);
        else
                __list_cut_position(list, head, entry);
}

static __inline void __list_splice(const struct thr_list_head *list,
                                   struct thr_list_head *prev,
                                   struct thr_list_head *next)
{
        struct thr_list_head *first = list->next;
        struct thr_list_head *last = list->prev;

        first->prev = prev;
        prev->next = first;

        last->next = next;
        next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new_head list to add.
 * @head: the place to add it in the first list.
 */
static __inline void list_splice(const struct thr_list_head *list,
                                 struct thr_list_head *head)
{
        if (!list_empty(list))
                __list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new_head list to add.
 * @head: the place to add it in the first list.
 */
static __inline void list_splice_tail(struct thr_list_head *list,
                                      struct thr_list_head *head)
{
        if (!list_empty(list))
                __list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new_head list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static __inline void list_splice_init(struct thr_list_head *list,
                                      struct thr_list_head *head)
{
        if (!list_empty(list)) {
                __list_splice(list, head, head->next);
                INIT_LIST_HEAD(list);
        }
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new_head list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static __inline void list_splice_tail_init(struct thr_list_head *list, struct thr_list_head *head)
{
        if (!list_empty(list)) {
                __list_splice(list, head->prev, head);
                INIT_LIST_HEAD(list);
        }
}

/**
 * list_entry - get the struct for this entry
 * @ptr:        the &struct list_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:        the list head to take the element from.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
        list_entry((ptr)->next, type, member)

/**
 * list_for_each        -       iterate over a list
 * @pos:        the &struct list_head to use as a loop cursor.
 * @head:       the head for your list.
 */
#define list_for_each(pos, head) \
        for (pos = (head)->next; prefetch(pos->next), pos != (head); \
                        pos = pos->next)

/**
 * __list_for_each      -       iterate over a list
 * @pos:        the &struct list_head to use as a loop cursor.
 * @head:       the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev   -       iterate over a list backwards
 * @pos:        the &struct list_head to use as a loop cursor.
 * @head:       the head for your list.
 */
#define list_for_each_prev(pos, head) \
        for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
                        pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:        the &struct list_head to use as a loop cursor.
 * @n:          another &struct list_head to use as temporary storage
 * @head:       the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                        pos = n, n = pos->next)

#endif

#define OS_ADD_TAIL 	list_add_tail_new

struct _HI_AGG_TxDescripter_chain* Driver_GetAGG(void);
unsigned char Driver_FillAgg(struct _HI_AGG_TxDescripter_chain* HI_AGG_TxDp);
void Driver_FreeAGG(struct _HI_AGG_TxDescripter_chain * HI_AGG_TxDp);
int  Driver_IsTxPrivEnough(int num);
void Driver_mic_error_event(void *  drv_priv,const void * wh,unsigned char * sa,unsigned char vmac_id);
//void Driver_mic_error_event(void *  drv_priv,unsigned char bGroupPacket ,unsigned char *addr,unsigned char vmac_id);
//void Driver_intr_rx_handle(void *  drv_priv,struct sk_buff *skb,unsigned char Rssi,unsigned char RxRate);
void Driver_intr_rx_handle(void *  drv_prv,struct sk_buff *skb,unsigned char Rssi,unsigned char RxRate,
        unsigned char bw, unsigned char channel,  unsigned char aggr, unsigned char wnet_vif_id,unsigned char keyid);
void Driver_intr_dtim_send(void *dev,unsigned char vid);
void Driver_intr_bcn_send(void *dev,unsigned char vid);
void Driver_intr_ba_recv(void *dev,unsigned char vid);
void drv_intr_bt_info_change(void * dpriv, unsigned char wnet_vif_id,unsigned char bt_lk_change);
void Driver_intr_tx_handle(void *  drv_priv,struct txdonestatus* tx_done_status,SYS_TYPE callback, unsigned char queue_id);
void Driver_ListInit(struct _HI_AGG_TxDescripter_chain  *HI_AGG_TxDp );
struct _HI_TxPrivDescripter_chain *  Driver_GetTxPriv(void);
int Do_HI_AGG_TxPriv_TYPE_AMSDU(struct _HI_TxPrivDescripter_chain* HI_TxPriv[],unsigned char* skbptr[],int length[],int packetNum);
void Do_HI_AGG_TxPriv_TYPE_AMPDU(struct _HI_TxPrivDescripter_chain* HI_TxPriv[],unsigned char* buffer[],int length[],int packetNum);
void Do_HI_AGG_TxPriv_TYPE_COMMO(struct _HI_TxPrivDescripter_chain* HI_TxPriv,unsigned char* buffer,int length,int packetNum);
void Driver_CreatTxPriv(void);

#endif
