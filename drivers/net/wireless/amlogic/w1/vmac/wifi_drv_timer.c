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

#include "wifi_mac_com.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static void os_timer_ex_handler(struct timer_list *timer_arg)
#else
static void os_timer_ex_handler(unsigned long timer_arg)
#endif
{
    struct os_timer_ext    *timer_object;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    timer_object = from_timer(timer_object, timer_arg, os_timer);
#else
    timer_object = (struct os_timer_ext *)(timer_arg);
#endif
    /*
     * Acquire the synchronization object.
     * This is used to synchronize with routine drv_cancelTimer.
     */
    if (cmpxchg(&(timer_object->timer_lock), 0, 1) == 1)
    {
        return;
    }
    //here, timer_lock=1
    // if timer is being cancelled, do not run handler, and do not rearm timer.
    if (timer_object->cancel_flag)
    {
        // release the synchronization object
        (void) cmpxchg(&(timer_object->timer_lock), 1, 0);
        return;
    }

    // run timer handler
    if (timer_object->timer_handler(timer_object->context) == OS_TIMER_REARMED)
    {
        // rearm timer only if handler function returned 0
        OS_SET_TIMER(&timer_object->os_timer, timer_object->timer_period);
    }
    else
    {
        // timer not rearmed - no longer active
        timer_object->active_flag = 0;
    }

    // release the synchronization object
    (void) cmpxchg(&(timer_object->timer_lock), 1, 0);
    //here, timer_lock=0
}


unsigned char os_timer_ex_initialize(struct os_timer_ext *timer_object,
    unsigned int timer_period, timer_handler_func timer_handler, void *context)
{
    ASSERT(timer_object != NULL);

    if (timer_object != NULL)
    {
        OS_INIT_TIMER(&timer_object->os_timer, os_timer_ex_handler, timer_object);

        timer_object->timer_lock = 0;
        timer_object->active_flag = 0;
        timer_object->cancel_flag = 0;
        timer_object->timer_period = timer_period;
        timer_object->context = context;
        timer_object->timer_handler = timer_handler;
        return 1;
    }

    return 0;
}

void os_set_timer_period (struct os_timer_ext* timer_object, unsigned int timer_period)
{
    timer_object->timer_period  = timer_period;
}

/* may lower than timer */
unsigned char os_timer_ex_start (struct os_timer_ext* timer_object)
{
    // mark timer as active
    timer_object->active_flag = 1;
    timer_object->cancel_flag = 0;

    // arm timer for the first time
    OS_SET_TIMER(&timer_object->os_timer, timer_object->timer_period);

    return 1;
}

unsigned char os_timer_ex_start_period (struct os_timer_ext* timer_object, unsigned int timer_period)
{
    os_set_timer_period(timer_object, timer_period);
    return os_timer_ex_start(timer_object);
}

/* timer handler cannot cancle himself */
unsigned char os_timer_ex_cancel (struct os_timer_ext* timer_object, enum timer_flags flags)
{
    int         tick_counter = 0;
    unsigned char    canceled     = 1;

    // indicate timer is being cancelled
    timer_object->cancel_flag = 1;

    // nothing to do if timer not active.
    if (! timer_object->active_flag)
    {
        return 1;
    }

    // try to acquire LED synchronization object
    //if timer_lock == 0, set 1 and just continue
    //else, wait to become 0, set 1, and continue
    while (cmpxchg(&(timer_object->timer_lock), 0, 1) == 1)
    {
        if (tick_counter++ > 1000)      // no more than 10ms
        {
            break;
        }

        if (flags == CANCEL_NO_SLEEP)
        {
            OS_DELAY(10);   // busy wait; can be executed at IRQL <= DISPATCH_LEVEL
        }
        else
        {
            OS_SLEEP(10);   // sleep; can only be executed at IRQL < DISPATCH_LEVEL
        }
    }
    //here, normally, timer_lock should be 1

    if (! OS_CANCEL_TIMER(&timer_object->os_timer))
    {
        canceled = 0;
        ERROR_DEBUG_OUT("OS_CANCEL_TIMER failed!!\n");
    }

    timer_object->active_flag = 0;

    // release synchronization object
    (void) cmpxchg(&(timer_object->timer_lock), 1, 0);
    //here, timer_lock=0

    return canceled;
}

unsigned char os_timer_ex_active (struct os_timer_ext* timer_object)
{
    return (timer_object->active_flag);
}

unsigned char os_timer_ex_del (struct os_timer_ext* timer_object, enum timer_flags flags)
{
    unsigned char    canceled     = 1;

    ASSERT(timer_object != NULL);
    if (timer_object == NULL)
    {
        return 0;
    }
    if (os_timer_ex_active(timer_object))
    {
        canceled = os_timer_ex_cancel(timer_object, flags);
    }
    return canceled;
}
