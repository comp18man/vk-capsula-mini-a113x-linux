/* Compatibility shims for building this 4.9-era vendor driver on 6.x kernels.
 * Force-included for every translation unit via ccflags-y.
 */
#ifndef _AML_COMPAT_H_
#define _AML_COMPAT_H_

#include <linux/version.h>
#include <linux/random.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
/* prandom_bytes() was folded into get_random_bytes() */
#define prandom_bytes(buf, len) get_random_bytes((buf), (len))
#endif


#include <linux/timer.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
/* del_timer()/del_timer_sync() were renamed */
#define del_timer(t)      timer_delete(t)
#define del_timer_sync(t) timer_delete_sync(t)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
/* from_timer() became timer_container_of() */
#define from_timer(var, callback_timer, timer_fieldname) \
    timer_container_of(var, callback_timer, timer_fieldname)
#endif

#include <linux/kthread.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
/* complete_and_exit() moved into the kthread namespace */
#define complete_and_exit(c, code) kthread_complete_and_exit(c, code)
#endif

/* Dropped from cfg80211; it only suppressed a disconnect on regdomain change. */
#ifndef REGULATORY_IGNORE_STALE_KICKOFF
#define REGULATORY_IGNORE_STALE_KICKOFF 0
#endif

#endif /* _AML_COMPAT_H_ */
