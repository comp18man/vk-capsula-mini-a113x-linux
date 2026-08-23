/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     driver layer timer function
 *
 *
 ****************************************************************************************
 */

#ifndef _OS_DEV_TIMER_H
#define _OS_DEV_TIMER_H

#include "linux/timer.h"
#include <linux/version.h>
#define OS_TIMER_NOT_REARMED 1
#define OS_TIMER_REARMED 0

typedef int (*timer_handler_func) (void *context);

struct os_timer_ext
{
    struct timer_list os_timer;            // timer object
    int32_t timer_lock;          // indicate timer handler is currently executing, or is cancling
    unsigned char cancel_flag;         // indicates timer must be cancelled
    unsigned char active_flag;         // indicates timer is running
    unsigned int timer_period;        // timer period
    void *context;             // execution context
    int (*timer_handler) (void*);               // handler function
};

enum timer_flags
{
    CANCEL_SLEEP    = 0,
    CANCEL_NO_SLEEP = 1
};

unsigned char os_timer_ex_initialize(struct os_timer_ext *timer_object, unsigned int timer_period, timer_handler_func timer_handler, void *context);
void  os_set_timer_period(struct os_timer_ext* timer_object, unsigned int timer_period);
unsigned char os_timer_ex_start(struct os_timer_ext* timer_object);
unsigned char os_timer_ex_start_period(struct os_timer_ext* timer_object, unsigned int timer_period);
unsigned char os_timer_ex_cancel(struct os_timer_ext* timer_object, enum timer_flags flags);
unsigned char os_timer_ex_active(struct os_timer_ext* timer_object);
unsigned char os_timer_ex_del(struct os_timer_ext* timer_object, enum timer_flags flags);

/*
 * Timer Interfaces. Use these macros to declare timer
 * and retrieve timer argument. This is mainly for resolving
 * different argument types for timer function in different OS.
 */
typedef void (*timer_func)(unsigned long);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#define OS_INIT_TIMER( _timer, _fn, _arg) do {  \
                timer_setup(_timer, _fn, 0);	\
        } while (0)
#else
#define OS_INIT_TIMER( _timer, _fn, _arg) do {  \
                init_timer(_timer);                                 \
                (_timer)->function = (_fn);                         \
                (_timer)->data = (unsigned long)(_arg);             \
        } while (0)
#endif

#define OS_SET_TIMER(_timer, _ms) mod_timer(_timer, jiffies + MAX(((_ms)*HZ)/1000,1))
#define OS_CANCEL_TIMER(_timer) del_timer(_timer)
#define OS_CANCEL_TIMER_SYNC(_timer) del_timer_sync(_timer)

#endif
