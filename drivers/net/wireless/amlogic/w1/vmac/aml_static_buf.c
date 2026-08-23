/*
 * drivers/amlogic/wifi/aml_static_buf.c
 *
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

//#define pr_fmt(fmt)	"Wifi: %s: " fmt, __func__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "wifi_common.h"
#include <linux/skbuff.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
#include <linux/wlan_plat.h>
#else
#include <linux/amlogic/wlan_plat.h>
#endif

#ifdef NOT_AMLOGIC_PLATFORM
extern void *w1_aml_mem_prealloc(int section, unsigned long size);
#else
extern void *bcmdhd_mem_prealloc(int section, unsigned long size);
#endif
enum aml_prealloc_index {
    AML_RX_FIFO = 0,
    AML_TX_DESC_BUF = 1
};
#define AML_RX   11
#define AML_TX   20
#define AML_RX_FIFO_SIZE   (1290 * 1024)
#define AML_TX_DESC_BUF_SIZE            (256 * 1024)

void *wifi_mem_prealloc(int section, unsigned long size)
{
    pr_info("sectoin %d, size %ld\n", section, size);
    if (section == AML_RX_FIFO ) {
        if (size > AML_RX_FIFO_SIZE) {
            pr_err("request AML_RX_FIFO (%lu) > %d\n",
               size, AML_RX_FIFO_SIZE);
            return NULL;
        }

#ifdef NOT_AMLOGIC_PLATFORM
        return w1_aml_mem_prealloc(AML_RX, size);
#else
        return bcmdhd_mem_prealloc(AML_RX, size);
#endif
    }
    if (section == AML_TX_DESC_BUF) {
        if (size > AML_TX_DESC_BUF_SIZE) {
            pr_err("request AML_TX_DESC_BUF(%lu) > %d\n",
                size, AML_TX_DESC_BUF_SIZE);
            return NULL;
        }

#ifdef NOT_AMLOGIC_PLATFORM
        return w1_aml_mem_prealloc(AML_TX, size);;
#else
        return bcmdhd_mem_prealloc(AML_TX, size);;
#endif
    }
    return NULL;
}

