#ifndef _AML_OPT_ALL_H_
#define _AML_OPT_ALL_H_
#if defined (HAL_FPGA_VER)
#include "wifi_debug.h"

#elif defined (HAL_SIM_VER)
#include "../../../ucode/host.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "macframe.h"
#include "pool.h"
#include "network.h"
#endif

extern struct hal_private g_hal_priv;

#define  OS_DMAA2DDRddress(__t)  ((void*)__t)
#define  OS_DDR2DMAAddress(__t)  ((void*)__t)

/*Start of HAL_FPGA_VER*/
#if defined (HAL_FPGA_VER)

#define OS_LOCK                	spinlock_t
#define OS_MUTEX           		struct mutex
#define OS_TQ_STRUCT               tasklet_struct
#define OS_INIT_TQUEUE(a,b,c)      tasklet_init((a), (b), (unsigned long)(c))
#define OS_SCHEDULE_TQUEUE(a)      tasklet_schedule((a))
#define OS_FREE_TQUEUE(a)          tasklet_kill((a))
#define OS_MDELAY                  msleep
#define OS_UDELAY                  udelay
#define HAL_LOCK_INIT()         	mutex_init(&g_hal_priv.hal_phy_mutex)
#define HAL_BEGIN_LOCK()            OS_MUTEX_LOCK(&g_hal_priv.hal_phy_mutex);
#define HAL_END_LOCK()              OS_MUTEX_UNLOCK(&g_hal_priv.hal_phy_mutex);
#define HAL_LOCK_DESTROY()   mutex_destroy(&g_hal_priv.hal_phy_mutex);

#define POWER_LOCK_INIT()  mutex_init(&g_hal_priv.power_mutex)
#define POWER_BEGIN_LOCK()  OS_MUTEX_LOCK(&g_hal_priv.power_mutex);
#define POWER_END_LOCK()  OS_MUTEX_UNLOCK(&g_hal_priv.power_mutex);
#define POWER_LOCK_DESTROY() mutex_destroy(&g_hal_priv.power_mutex);

#define COMMON_LOCK_INIT() spin_lock_init(&(g_hal_priv).com_spinlock)
#define COMMON_LOCK_DESTROY()
#define COMMON_LOCK() do { OS_SPIN_LOCK_IRQ(&(g_hal_priv).com_spinlock, g_hal_priv.com_spinlock_flag); } while (0)
#define COMMON_UNLOCK() do { OS_SPIN_UNLOCK_IRQ(&(g_hal_priv).com_spinlock, g_hal_priv.com_spinlock_flag); } while (0)

#define WIFI_NETIF_RECV_SKB_INIT() spin_lock_init(&(g_hal_priv).skb_spinlock)
#define WIFI_NETIF_RECV_SKB_DESTROY()
#define WIFI_NETIF_RECV_SKB_LOCK() do { OS_SPIN_LOCK_IRQ(&(g_hal_priv).skb_spinlock, g_hal_priv.skb_spinlock_flag); } while (0)
#define WIFI_NETIF_RECV_SKB_UNLOCK() do { OS_SPIN_UNLOCK_IRQ(&(g_hal_priv).skb_spinlock, g_hal_priv.skb_spinlock_flag); } while (0)

#define AML_TXLOCK_INIT() spin_lock_init(&(g_hal_priv).tx_spinlock)
#define AML_TXLOCK_DESTROY()
#define AML_TXLOCK_LOCK() OS_SPIN_LOCK_IRQ(&(g_hal_priv).tx_spinlock, (g_hal_priv).tx_spinlock_flag);
#define AML_TXLOCK_UNLOCK() OS_SPIN_UNLOCK_IRQ(&(g_hal_priv).tx_spinlock, (g_hal_priv).tx_spinlock_flag);

#define PN_LOCK_INIT() spin_lock_init(&g_hal_priv.pn_spinlock)
#define PN_LOCK_DESTROY()
#define PN_LOCK() do {OS_SPIN_LOCK_IRQ(&g_hal_priv.pn_spinlock, g_hal_priv.pn_spinlock_flag); }while(0)
#define PN_UNLOCK() do{OS_SPIN_UNLOCK_IRQ(&g_hal_priv.pn_spinlock, g_hal_priv.pn_spinlock_flag); }while(0)

#define KILL_PROC(nr, sig) \
        { \
                struct task_struct *tsk; \
                struct pid *pid;    \
                pid = find_get_pid((pid_t)nr);    \
                tsk = pid_task(pid, PIDTYPE_PID);    \
                if (tsk) send_sig(sig, tsk, 1); \
        }

#if defined(CONFIG_HAS_WAKELOCK)
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,29)
#define WAKE_LOCK_INIT( hal_priv,  index, y)
#define WAKE_LOCK(hal_priv,  index)
#define WAKE_UNLOCK(hal_priv, index)
#define WAKE_LOCK_DESTROY(hal_priv, index)
#else
#define WAKE_LOCK_INIT( hal_priv,  index, y)  wake_lock_init(&hal_priv->wakelock[index], WAKE_LOCK_SUSPEND, y);
#define WAKE_LOCK(hal_priv,  index) wake_lock(&hal_priv->wakelock[index]);
#define WAKE_UNLOCK(hal_priv, index)    wake_unlock(&hal_priv->wakelock[index]);
#define WAKE_LOCK_DESTROY(hal_priv, index) wake_lock_destroy(&hal_priv->wakelock[index]);
#endif
#endif

#endif
/*End of HAL_FPGA_VER*/

#ifndef NOT_AMLOGIC_PLATFORM
#define pr_debug(fmt, ...) pr_info(fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...)
#endif

/*Start of HAL_SIM_VER*/
#if defined (HAL_SIM_VER)

#define MAX_SKB_NUM	      1000
#define RXFRAME_MAXLEN	    4096
#define  BIT(n) (1UL<<(n))
#define ALIGN(x,a) 		(((x)+(a)-1)&~((a)-1))

#define pr_debug(...)  	do{if(FW_ID==1)io_printf("<STA_0>");else io_printf("<STA_1>");sv_time();io_printf(__VA_ARGS__);}while(0)
#define pr_crit(...) pr_debug(__VA_ARGS__)
#define pr_err(...) pr_debug(__VA_ARGS__)
#define pr_warn(...) pr_debug(__VA_ARGS__)
#define pr_info(...) pr_debug(__VA_ARGS__)
#define pr_notice(...) pr_debug(__VA_ARGS__)
#define  MODULE_LICENSE(a);
#define  module_init(a);
#define  module_exit(a);

#define OS_TQ_STRUCT 				tasklet_struct
#define OS_INIT_TQUEUE(a,b,c)		//tasklet_init((a), (b), (unsigned long)(c))
#define OS_SCHEDULE_TQUEUE(a)		hi_top_task((unsigned long)hal_get_priv() );//tasklet_schedule((a))
#define OS_FREE_TQUEUE(a)			//tasklet_kill((a))
#define OS_SKBBUF_DATA(_skb) ((_skb)->data)
#define OS_SKBBUF_LEN(_skb)	 ((_skb)->data_len)
#define OS_MDELAY(_u)				sv_delay(FW_ID, _u);
#define OS_UDELAY(_u)				OS_MDELAY(_u)

#define  OS_MUTEX					int
#define  OS_LOCK		  			int
struct tasklet_struct{
	int mm;
};

#define sdio_func  			tasklet_struct
#define semaphore  			tasklet_struct
#define completion  		       tasklet_struct
#define mutex  				tasklet_struct
#define task_struct			tasklet_struct
#define wait_queue_head_t   struct tasklet_struct

#define	HAL_LOCK_INIT()
#define	HAL_BEGIN_LOCK()
#define	HAL_END_LOCK()
#define	HAL_LOCK_DESTROY(_ic)

#define POWER_LOCK_INIT()
#define POWER_BEGIN_LOCK()
#define POWER_END_LOCK()
#define POWER_LOCK_DESTROY(_ic)

#define	COMMON_LOCK_INIT()
#define	COMMON_LOCK_DESTROY()
#define	COMMON_LOCK()
#define	COMMON_UNLOCK()

#define	AML_TXLOCK_INIT()			//spin_lock_init(&(g_hal_priv).tx_spinlock)
#define	AML_TXLOCK_DESTROY()
#define	AML_TXLOCK_LOCK() 			//spin_lock_bh(&(g_hal_priv).tx_spinlock);
#define	AML_TXLOCK_UNLOCK()		//spin_unlock_bh(&(g_hal_priv).tx_spinlock);

#ifndef _LINUX_IRQRETURN_H
#define _LINUX_IRQRETURN_H

/**
 *  * enum irqreturn
 *  * @IRQ_NONE		interrupt was not from this device
 *  * @IRQ_HANDLED		interrupt was handled by this device
 *  * @IRQ_WAKE_THREAD	handler requests to wake the handler thread
 *  */
enum irqreturn {
	IRQ_NONE,
	IRQ_HANDLED,
	IRQ_WAKE_THREAD,
};


#endif /*End of _LINUX_IRQRETURN_H */

struct net_device {
        int  device;
};

struct sk_buff {
        char			cb[48];
        unsigned char 			valid;
        unsigned int		len,data_len;
        char 			*data;
        char		        buffer[RXFRAME_MAXLEN+64];
};
#define OS_SKBBUF struct sk_buff

extern  struct sk_buff g_skbuffer[MAX_SKB_NUM];

static struct sk_buff  * OS_SKBBUF_ALLOC(int size) {
        int i = 0;
        for (i=0; i<MAX_SKB_NUM; i++) {
                if (g_skbuffer[i].valid == 0) {
                        g_skbuffer[i].valid = 1;
                        g_skbuffer[i].len =  size;
                        g_skbuffer[i].data_len = 0;
                        g_skbuffer[i].data = &g_skbuffer[i].buffer[64];
                        return &g_skbuffer[i];
                }
        }
        return NULL;
}

static void * OS_SKBBUF_FREE(struct sk_buff * skb)
{
        skb->valid = 0;
        return NULL;
}

#define dev_kfree_skb(skb)    OS_SKBBUF_FREE(skb)
/*skb_pull*/
static void * OS_SKBBUF_PULL(struct sk_buff * skb, unsigned int len )
{
        skb->data_len -= len;
        skb->data+=len;
        return skb->data;
}
/*skb_put*/
static void * OS_SKBBUF_PUT(struct sk_buff * skb, unsigned int len )
{
        skb->data_len += len;

        return skb->data+skb->data_len-len;
 //       return skb->data;
}

#ifndef BITS_TO_LONGS /* Older kernels define this already */
#define BITS_PER_BYTE           				8
#define BITS_TO_LONGS(nr)				((nr)/32)
#endif /*End of BITS_TO_LONGS*/

#define  set_bit(a,m)	((*((m)+(a)/(BITS_PER_BYTE * sizeof(unsigned long)))) \
				|= BIT((a)%(BITS_PER_BYTE * sizeof(unsigned long))))
static unsigned char test_and_clear_bit(int id,unsigned long * _map)
{

    int map_shift = id / (sizeof(unsigned long) *BITS_PER_BYTE);
    int bit_offset = id % (sizeof(unsigned long) *BITS_PER_BYTE);

    if(*(_map + map_shift) & (BIT(bit_offset)))
    {
    	*(_map  + map_shift ) &= ~BIT(bit_offset);
    	return 1;
    }
    return 0;

}

static int  find_first_zero_bit(unsigned long *_map,int max)
{
    int id = 0;
    for(id = 0; id < max; id++)
    {
    	if((*(_map + id / (sizeof(unsigned long) * BITS_PER_BYTE))
    	    & BIT(id % (sizeof(unsigned long) * BITS_PER_BYTE))) == 0)
    	{
    		return id;
    	}
    }
    return WIFI_MAX_TXFRAME;

}
static unsigned char test_bit(int id,unsigned long * _map)
{
    if(*(_map + id / (sizeof(unsigned long) * BITS_PER_BYTE))
    	& (BIT(id % (sizeof(unsigned long) * BITS_PER_BYTE))))
    {
    	return 1;
    }
    return 0;
}

extern void real_sdio_init(int vid);
#define  HI_DRIVER_INIT()		real_sdio_init(FW_ID);
#endif /*End of HAL_SIM_VER*/

#endif /*End of _AML_OPT_ALL_H_ */
