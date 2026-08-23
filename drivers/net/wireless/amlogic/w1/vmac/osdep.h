#ifndef _DRV_LINUX_OSDEP_H
#define _DRV_LINUX_OSDEP_H

#include <linux/version.h>
#include <generated/autoconf.h>

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/wait.h>

#include <linux/workqueue.h>        //work_struct

#include <asm/byteorder.h>
//#include <asm/scatterlist.h>
#include <linux/scatterlist.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <linux/wait.h>
#include <net/cfg80211.h>
#include <net/rtnetlink.h>
#include <linux/nl80211.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/ctype.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

#include "wifi_debug.h"
#include "wifi_drv_timer.h"

#define _LITTLE_ENDIAN  1234   
#define _BIG_ENDIAN 4321    
#define _BYTE_ORDER _LITTLE_ENDIAN

/*
 * Byte Order stuff
 */
#define le16toh(_x) le16_to_cpu(_x)
#define htole16(_x) cpu_to_le16(_x)
#define le32toh(_x) le32_to_cpu(_x)
#define htole32(_x) cpu_to_le32(_x)
#define be32toh(_x) be32_to_cpu(_x)
#define htobe32(_x) cpu_to_be32(_x)

/*
 * Normal Delay functions. Time specified in microseconds.
 */
#define OS_DELAY(_us) udelay(_us)
#define OS_SLEEP(_us) do {                  \
                set_current_state(TASK_INTERRUPTIBLE);  \
                schedule_timeout((_us) * HZ / 1000000); \
        } while (0)

/*
 * System time interface
 */

static __inline unsigned long
OS_GET_TIMESTAMP(void)
{
    return ((jiffies / HZ) * 1000) + (jiffies % HZ) * (1000 / HZ);
}

#ifdef DEBUG_MALLOC
extern int kmalloc_count;
extern int kfree_count;

struct mem_statistic
{
    char name[64];
    int count;
};

extern struct mem_statistic m_pn_buf[64];
extern struct mem_statistic f_pn_buf[64];


static __inline int find_name(char *name, struct mem_statistic *buf, unsigned int len) {
    unsigned int i;

    for (i = 0; i < kmalloc_count; ++i) {
        if (!memcmp(buf[i].name, name, len)) {
            return i;
        }
    }

    return -1;
}

static __inline void  *ZMALLOC(int a, char *name, unsigned gfp)
{
    short i;
    void *tmp = kzalloc(a, gfp);

    i = find_name(name, &m_pn_buf[0], strlen(name));
    if (i == -1) {
        memcpy(m_pn_buf[kmalloc_count].name, name, strlen(name));
        m_pn_buf[kmalloc_count++].count++;
    } else {
        m_pn_buf[i].count++;
    }

    pr_debug("Zmalloc %p:%s\n", tmp, name);
    if (kmalloc_count >= 64) {
        pr_err("amlogic WIFI:<ERROR>Array Overflow, please make sure array "
            "length big enough in debug malloc process\n");
    }
    return tmp;
}

static __inline void FREE(void *a, char *name)
{
    short i;

    kfree(a);

    i = find_name(name, &f_pn_buf[0], strlen(name));
    if (i == -1) {
        memcpy(f_pn_buf[kfree_count].name, name, strlen(name));
        f_pn_buf[kfree_count++].count++;
    } else {
        f_pn_buf[i].count++;
    }

    pr_debug("kfree %p:%s\n", a, name);
    if (kfree_count >= 64) {
        ERROR_DEBUG_OUT("amlogic WIFI:<ERROR>Array Overflow, please make sure array "
            "length big enough in debug malloc process\n");
    }
}

#else
#ifdef HAL_SIM_VER

#define MALLOC(a) malloc(a)
#define memset memset
#define FREE     free
#define FRINT    io_printf
#define OS_LIST  CO_LIST_HDR

#else

#define ZMALLOC(size, name, gfp) kzalloc(size, gfp)
#define FREE(a, name) kfree(a)

#endif
#endif

#define NET_MALLOC(_size, _gfp, _str) ZMALLOC(ALIGN(_size + 1, 128), _str, _gfp)
#define NET_FREE(_p, name) do { if ((_p) != NULL) FREE(_p, name); /* pr_debug("<running> %s %d kfree %p \n",__func__,__LINE__,_p); */(_p) = NULL;} while(0)

#define CIRCLE_Add_One(_data,_total) do {\
                if((_data)== (_total)-1)        \
                        (_data) = 0;            \
                else                    \
                        (_data)++;          \
        }while(0)
        
#define CIRCLE_Sub_One(_data,_total) do {\
                if((_data)== 0)     \
                        (_data) = (_total)-1;           \
                else                    \
                        (_data)--;          \
        }while(0)
        

#define CIRCLE_Subtract2(a,b,l) (((a) > (b)) ? ((a) - (b)):((l)+(a)-(b)))
#define CIRCLE_Addition2(a,b,l) (((a) +(b)< (l)) ?  ((a) + (b)):((a)+(b)-(l)))


#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a)         (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MIN
#define MIN(a, b)               ((a) < (b) ? a : b)
#endif

#ifndef MAX
#define MAX(a, b)               ((a) > (b) ? a : b)
#endif

#ifndef NBBY
#define NBBY    8           /* number of bits/byte */
#endif

#ifndef roundup
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

#ifndef howmany
#define howmany(x, y)   (((x)+((y)-1))/(y))
#endif

#ifndef offsetof
#define offsetof(type, field)   ((size_t)(&((type *)0)->field))
#endif

/* Bit map related macros. */
#define setbit(a,i) ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i) ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)  ((a)[(i)/NBBY] & (1<<((i)%NBBY)))

#define DRV_SUPPORT_STA
#ifndef DRV_SUPPORT_STA
#define __packed   __attribute__((__packed__))
#endif
#undef DRV_SUPPORT_STA

#define __offsetof(t,m) offsetof(t,m)

#ifndef ALIGNED_POINTER
#define ALIGNED_POINTER(p,t)    1
#endif

#if defined (SYSTEM32)
    typedef unsigned int  SYS_TYPE;
#elif defined (SYSTEM64)
    typedef unsigned long SYS_TYPE;
#else
    #err "SYSTEM TYPE ERR"
#endif


#define IN
#define OUT
#ifdef BIT
#undef  BIT

#define BIT(n)    (1UL << (n))
#endif


#ifndef BITS_TO_LONGS /* Older kernels define this already */
    #define BITS_PER_BYTE           8
#if defined (HAL_FPGA_VER)
    #define BITS_TO_LONGS(nr)   	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#elif defined (HAL_SIM_VER)
    #define BITS_TO_LONGS(nr)	(nr/32 +1)
#endif
#endif


#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
#define vlan_tx_tag_present(__skb)   skb_vlan_tag_present(__skb)
#define vlan_tx_tag_get(__skb)  skb_vlan_tag_get(__skb)
#define vlan_tx_tag_get_id(__skb)  skb_vlan_tag_get_id(__skb)
//#define skb_vlan_tag_get_prio(__skb)	  skb_vlan_tag_get_prio(__skb)


#define IRQF_DISABLED IRQF_ONESHOT
/*
 * wireless hardware capability structures
 */

/**
 * enum ieee80211_band - supported frequency bands
 *
 * The bands are assigned this way because the supported
 * bitrates differ in these bands.
 *
 * @IEEE80211_BAND_2GHZ: 2.4GHz ISM band
 * @IEEE80211_BAND_5GHZ: around 5GHz band (4.9-5.7)
 * @IEEE80211_BAND_60GHZ: around 60 GHz band (58.32 - 64.80 GHz)
 * @IEEE80211_NUM_BANDS: number of defined bands
 */
enum ieee80211_band {
	IEEE80211_BAND_2GHZ = NL80211_BAND_2GHZ,
	IEEE80211_BAND_5GHZ = NL80211_BAND_5GHZ,
	IEEE80211_BAND_60GHZ = NL80211_BAND_60GHZ,

	/* keep last */
	IEEE80211_NUM_BANDS
};

#define strnicmp  strncasecmp

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 251)
static inline void
dev_addr_mod(struct net_device *dev, unsigned int offset,
	     const u8 *addr, size_t len)
{
       memcpy(&dev->dev_addr[offset], addr, len);
}
#endif
#endif
#endif
