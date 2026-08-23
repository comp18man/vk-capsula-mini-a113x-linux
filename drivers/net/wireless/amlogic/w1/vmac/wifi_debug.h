#ifndef _AML_DEBUG_H_
#define _AML_DEBUG_H_

#include <linux/bitops.h>
#include <linux/kernel.h>

#ifndef BIT
#define BIT(n)    (1UL << (n))
#endif //BIT

#define  CTRL_BYTE
#define LINEBYTE 32
#define ASCII_IN  1

enum
{
    AML_DEBUG_ACL              = BIT(0),   /* reset module  */
    AML_DEBUG_XMIT      = BIT(1),   /* xmit ampdu */       //AML_DEBUG_TX_AMPDU==>AML_DEBUG_XMIT
    AML_DEBUG_KEY               = BIT(2),   /* key module  */
    AML_DEBUG_STATE     = BIT(3),   /* state module  */
    AML_DEBUG_RATE              = BIT(4),   /* rate control */
    AML_DEBUG_RECV          = BIT(5),   /* rx module */
    AML_DEBUG_P2P           = BIT(6),   /* p2p module */
    AML_DEBUG_CFG80211      = BIT(7),   /* cfg80211 module */
    AML_DEBUG_SCAN              = BIT(8),   /* scan module */
    AML_DEBUG_LOCK  = BIT(9),   /*  */
    AML_DEBUG_INIT              = BIT(10),   /* initial module */
    AML_DEBUG_ROAM              = BIT(11),   /* exit module */
    AML_DEBUG_NODE              = BIT(12),   /* nsta module */
    AML_DEBUG_ANDROID               = BIT(13),   /* android module */
    AML_DEBUG_ACTION            = BIT(14),   /* channel module */
    AML_DEBUG_IOCTL             = BIT(15),   /* ioctl module */
    AML_DEBUG_CONNECT               = BIT(16),   /* Connect Operating */
    AML_DEBUG_TIMER             = BIT(17),   /* TIMER Debug */
    AML_DEBUG_ADDBA             = BIT(18),   /* ADDBA Debug */
    AML_DEBUG_NETDEV            = BIT(19),   /* netdev module */
    AML_DEBUG_HAL               = BIT(20),   /* hal module  */
    AML_DEBUG_BEACON           = BIT(21),   /* beacon module  */
    AML_DEBUG_UAPSD             = BIT(22),   /* uapsd */
    AML_DEBUG_BWC               = BIT(23),   /* channel bw control */
    AML_DEBUG_ELEMID      = BIT(24),   /* elemid */
    AML_DEBUG_PWR_SAVE      = BIT(25),   /* PS Poll and PS save */
    AML_DEBUG_DEBUG     = BIT(26),   /*  debug */
    AML_DEBUG_INFO          = BIT(27),   /*  info */
    AML_DEBUG_WARNING       = BIT(28),   /*  warning */
    AML_DEBUG_ERROR             = BIT(29),   /*  errors */
    AML_DEBUG_WME          =BIT(30),
    AML_DEBUG_DOTH         =BIT(31),
    AML_DEBUG_ANY               = 0xffffffff
};

enum
{
    AML_DEBUG_VHT		  = BIT(0),
    AML_DEBUG_VHT_ANY       = 0xffffffff
};
/*
** Define the "default" debug mask
*/
//#define AML_DEBUG_LEVEL   AML_DEBUG_BWC         //40M
#define AML_DEBUG_LEVEL (AML_DEBUG_STATE|AML_DEBUG_ERROR|AML_DEBUG_WARNING|AML_DEBUG_INIT|\
                            AML_DEBUG_CFG80211)


#define DBG_HAL_THR_ENTER()          //pr_debug("---xman debug---:%s ++ \n",__FUNCTION__);
#define DBG_HAL_THR_EXIT()           //pr_debug("---xman debug---:%s -- \n",__FUNCTION__);


extern int aml_debug;
extern unsigned long long g_dbg_info_enable;
extern unsigned long long g_dbg_modules;

#define DPRINTF( _m,  ...) do {         \
                if (aml_debug& (_m))                \
                        pr_debug(__VA_ARGS__);      \
        } while (0)

enum
{
    AML_DBG_OFF = 0,
    AML_DBG_ON = 1,
};

enum
{
    AML_DBG_MODULES_P2P = BIT(0),         /* p2p */
    AML_DBG_MODULES_RATE_CTR = BIT(1),    /* minstrel rate */
    AML_DBG_MODULES_TX = BIT(2),    /* tx */
    AML_DBG_MODULES_HAL_TX = BIT(3),    /* hal_tx */
    AML_DBG_MODULES_TX_ERROR = BIT(4),
    AML_DBG_MODULES_SCAN = BIT(5),
    AML_DEBUG_MODULES_ALL = 0xffffffffffffffff,
};

#define AML_PRINT( _m,format,...) do {         \
                if (g_dbg_modules & (_m)) \
                { \
                    if(_m == AML_DBG_MODULES_P2P) \
                        pr_debug("[p2p] <%s> %d "format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
                    else if(_m == AML_DBG_MODULES_RATE_CTR) \
                        pr_debug("[mi_rate] <%s> %d "format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
                    else if(_m == AML_DBG_MODULES_TX) \
                        pr_debug("[TX] <%s> %d "format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
                    else if(_m == AML_DBG_MODULES_TX_ERROR) \
                        pr_debug("[TX_ERROR] <%s> %d "format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
                    else if(_m == AML_DBG_MODULES_SCAN) \
                        pr_debug("[SCAN] <%s> %d "format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
                }    \
        } while (0)

#define ERROR_DEBUG_OUT(format,...) do {    \
                 pr_err("FUNCTION: %s LINE: %d:"format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define AML_OUTPUT(format,...) do {    \
                 pr_debug("<%s> %d:"format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)


#include "wifi_pt_init.h"
extern struct _B2B_Platform_Conf gB2BPlatformConf;


#define DEBUG_LOCK

#ifdef DEBUG_LOCK
#define OS_SPIN_LOCK_IRQ(a, b)        {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p, #a ++\n",__func__,__LINE__,(a));spin_lock_irqsave((a), (b));}
#define OS_SPIN_UNLOCK_IRQ(a, b)   {if (aml_debug& (AML_DEBUG_LOCK)) pr_debug("%s,%d,%p,#a --\n",__func__,__LINE__,(a));spin_unlock_irqrestore((a), (b));}


#define OS_SPIN_LOCK_BH(a)        {if (aml_debug& (AML_DEBUG_LOCK)) pr_debug("%s,%d,%p,#a ++\n",__func__,__LINE__,(a));spin_lock_bh((a));}
#define OS_SPIN_UNLOCK_BH(a)   {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a --\n",__func__,__LINE__,(a));spin_unlock_bh((a));}


#define OS_SPIN_LOCK(a)        {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a ++\n",__func__,__LINE__,(a));spin_lock((a));}
#define OS_SPIN_UNLOCK(a)   {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a --\n",__func__,__LINE__,(a));spin_unlock((a));}


#define OS_WRITE_LOCK(a)        {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a ++\n",__func__,__LINE__,(a));write_lock((a));}
#define OS_WRITE_UNLOCK(a)   {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a --\n",__func__,__LINE__,(a));write_unlock((a));}

#define OS_WRITE_LOCK_BH(a)        {if (aml_debug& (AML_DEBUG_LOCK)) pr_debug("%s,%d,%p,#a ++\n",__func__,__LINE__,(a));write_lock_bh((a));}
#define OS_WRITE_UNLOCK_BH(a)   {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a --\n",__func__,__LINE__,(a));write_unlock_bh((a));}

#define OS_WRITE_LOCK_IRQ(a, b)        {if (aml_debug& (AML_DEBUG_LOCK))  pr_debug("%s,%d,%p,#a ++\n",__func__,__LINE__,(a));write_lock_irqsave((a), (b));}
#define OS_WRITE_UNLOCK_IRQ(a, b)   {if (aml_debug& (AML_DEBUG_LOCK)) pr_debug("%s,%d,%p,#a --\n",__func__,__LINE__,(a));write_unlock_irqrestore((a), (b));}

#define OS_MUTEX_LOCK(a)        {/*pr_debug("%s,%d,%p, #a ++\n",__func__,__LINE__,(a));*/mutex_lock(a);}
#define OS_MUTEX_UNLOCK(a)        {/*pr_debug("%s,%d,%p, #a --\n",__func__,__LINE__,(a));*/mutex_unlock(a);}

#else
#define OS_SPIN_LOCK_IRQ(a, b)        {spin_lock_irqsave((a), (b));}
#define OS_SPIN_UNLOCK_IRQ(a, b)   {spin_unlock_irqrestore((a), (b));}


#define OS_SPIN_LOCK_BH(a)        {spin_lock_bh((a));}
#define OS_SPIN_UNLOCK_BH(a)   {spin_unlock_bh((a));}


#define OS_SPIN_LOCK(a)        {spin_lock((a));}
#define OS_SPIN_UNLOCK(a)   {spin_unlock((a));}


#define OS_WRITE_LOCK(a)        {write_lock((a));}
#define OS_WRITE_UNLOCK(a)   {write_unlock((a));}

#define OS_WRITE_LOCK_BH(a)        {write_lock_bh((a));}
#define OS_WRITE_UNLOCK_BH(a)   {write_unlock_bh((a));}

#define OS_WRITE_LOCK_IRQ(a, b)        {write_lock_irqsave((a), (b));}
#define OS_WRITE_UNLOCK_IRQ(a, b)   {write_unlock_irqrestore((a), (b));}


#define OS_MUTEX_LOCK(a)        {mutex_lock(a);}
#define OS_MUTEX_UNLOCK(a)        {mutex_unlock(a);}

#endif

#if defined (FPGA) ||defined (CHIP)
#define PRINT(...)      do {pr_debug( __VA_ARGS__ );}while(0)
#define PRINT_ERR(...)      do {pr_err( __VA_ARGS__ );}while(0)
#define PUTC( character )   pr_debug("%c" ,character )
#define PUTX8(size, value)    pr_debug("%02x", value);
#define PUTU8(value)            pr_debug("%u", value);
#define PUTS( ... )    pr_debug(__VA_ARGS__)
#define PUTU( number)   pr_debug("%d\n",number)
#define DBG_ENTER()           //pr_debug("--->%s ++ \n",__FUNCTION__);
#define DBG_EXIT()              //pr_debug("--->%s -- \n",__FUNCTION__);
#endif


#ifndef ASSERT
#define ASSERT(exp) do{    \
                if (!(exp)) {   \
                        pr_err("=>=>=>=>=>assert %s,%d\n",__func__,__LINE__);   \
                        /*BUG();        while(1);   */  \
                }                       \
        } while (0);
#endif

#ifdef __KERNEL__
#include <asm/page.h>

#define KASSERT(exp, msg) do {          \
                if (unlikely(!(exp))) {         \
                        pr_err msg ;         \
                        BUG();              \
                }                   \
        } while (0)

#endif /* __KERNEL__ */

void address_print( unsigned char* address );
void IPv4_address_print( unsigned char* address );
void dump_memory_internal(unsigned char *data,int len);
void address_read( unsigned char* cursor, unsigned char* address );

 unsigned short READ_16L( const unsigned char* address );
 void WRITE_16L( unsigned char* address, unsigned short value );
unsigned int READ_32L( const unsigned char* address );
void WRITE_32L( unsigned char* address, unsigned int value );

unsigned short READ_16B( const unsigned char* address );
void WRITE_16B( unsigned char* address, unsigned short value );
unsigned int READ_32B( const unsigned char* address );
void WRITE_32B( unsigned char* address, unsigned int value );

void ie_dbg(unsigned char *ie ) ;

#endif 
