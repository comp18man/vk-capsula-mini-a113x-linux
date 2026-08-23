/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *  driver layer configuration function
 *
 *
 ****************************************************************************************
 */
#include "wifi_drv_config.h"
#include "wifi_drv_main.h"
#include "wifi_drv_if.h"
#include "wifi_debug.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#endif

int drv_set_config(void * dev, enum cip_param_id id, int data)
{
    struct drv_private *drv_priv = ( struct drv_private *)dev;
    struct hal_private* hal_priv = hal_get_priv();
    
    pr_debug("%s(%d) id 0x%x data 0x%x\n", __func__, __LINE__, id, data);

    switch (id) {
        case CHIP_PARAM_TXCHAINMASK:
            drv_priv->drv_config.cfg_txchainmask = data;
            break;

        case CHIP_PARAM_RXCHAINMASK:
            drv_priv->drv_config.cfg_rxchainmask = data;
            break;

        case CHIP_PARAM_TXCHAINMASKLEGACY:
            drv_priv->drv_config.cfg_txchainmasklegacy = data;
            break;

        case CHIP_PARAM_RXCHAINMASKLEGACY:
            drv_priv->drv_config.cfg_rxchainmasklegacy = data;
            break;

        case CHIP_PARAM_CHAINMASK_SEL:
            drv_priv->drv_config.cfg_chainmask_sel = data;
            break;

        case CHIP_PARAM_AMPDU:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            drv_priv->drv_config.cfg_txaggr = data;
            drv_priv->drv_config.cfg_rxaggr = data;
            break;

        case CHIP_PARAM_AMPDU_RX:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            drv_priv->drv_config.cfg_rxaggr = data;
            break;

        case CHIP_PARAM_AMPDU_LIMIT:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            drv_priv->drv_config.cfg_ampdu_limit = data;
            break;

        case CHIP_PARAM_AMPDU_SUBFRAMES:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            drv_priv->drv_config.cfg_ampdu_subframes = data;
            break;

        case CHIP_PARAM_AGGR_PROT:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            drv_priv->drv_config.cfg_aggr_prot = data;
            break;

        case CHIP_PARAM_TXPOWER_LIMIT:
            drv_priv->drv_config.cfg_txpowlimit = data;
            break;

        case CHIP_PARAM_BURST_ACK:
            pr_debug("<running> %s %d CHIP_PARAM_BURST_ACK\n",__func__,__LINE__);
            drv_priv->drv_config.cfg_burst_ack = data;
            break;

        case CHIP_PARAM_ACK_POLICY:
            pr_debug("<running> %s %d CHIP_PARAM_ACK_POLICY\n",__func__,__LINE__);
            drv_priv->drv_config.cfg_ampduackpolicy= data;
            break;

        case CHIP_PARAM_40MSUPPORT:
            drv_priv->drv_config.cfg_40Msupport= data;
            break;

        case CHIP_PARAM_AMSDU_ENABLE:
            drv_priv->drv_config.cfg_txamsdu = data;
            break;

        case CHIP_PARAM_USE_EAP_LOWEST_RATE:
            drv_priv->drv_config.cfg_eap_lowest_rate = data;
            break;

        case CHIP_PARAM_RETRY_LIMIT:
            drv_priv->drv_config.cfg_retrynum = data;
            drv_hal_setretrynum( data);
            break;

        case CHIP_PARAM_HTSUPPORT:
            drv_priv->drv_config.cfg_htsupport= data;
            break;
			
	  case CHIP_PARAM_VHTSUPPORT:
              drv_priv->drv_config.cfg_vhtsupport= data;
	  	break;

        case CHIP_PARAM_DISABLE_RATECONTROL:
            drv_priv->drv_config.cfg_disratecontrol= data;
            break;

        case CHIP_PARAM_AMPDU_ONE_FRAME:
            drv_priv->drv_config.cfg_ampdu_oneframe= data;
            break;

        case CHIP_PARAM_TXLIVETIME:
            drv_priv->drv_config.cfg_ampdu_livetime= data;
            drv_hal_txlivetime(data);
            break;

        case  CHIP_PARAM_HWCSUM:
            drv_priv->drv_config.cfg_checksumoffload= data;
            break;

        case  CHIP_PARAM_HWTKIPMIC:
            drv_priv->drv_config.cfg_tkipmicsupport= data;
            break;

        case CHIP_PARAM_DEBUG:
            aml_debug= data;
            break;

        case CHIP_PARAM_DBG_RXERR_RESET:
            hal_priv->hal_ops.Hal_RxerrRst_trigger(data);
            break;

        case CHIP_PARAM_RTC_ENABLE:
            drv_priv->drv_config.cfg_rtcenable  = data;
            break;

        case CHIP_PARAM_SHORTPREAMBLE:
            pr_debug("%s(%d),Before set short preamble.\n\n",__func__,__LINE__);
            drv_priv->drv_config.cfg_shortpreamble = data;
            break;

         case CHIP_PARAM_DYNAMIC_BW:
            drv_priv->drv_config.cfg_dynamic_bw = data;
            break;

         case CHIP_PARAM_EAT_COUNT:
            drv_priv->drv_config.cfg_eat_count_max = data;
            break;

         case CHIP_PARAM_AGGR_THRESH:
            drv_priv->drv_config.cfg_aggr_thresh = data;
            break;

         case CHIP_PARAM_HRTIMER_INTERVAL:
            drv_priv->drv_config.cfg_hrtimer_interval = data;
            break;

        default:
            return (-1);
    }

    return (0);
}

int drv_get_config(void *dev, enum cip_param_id id)
{
    struct drv_private *drv_priv = (struct drv_private *)dev;
    int supported = 0;

    switch (id) {
        case CHIP_PARAM_TXCHAINMASK:
            supported = drv_priv->drv_config.cfg_txchainmask;
            break;

        case CHIP_PARAM_RXCHAINMASK:
            supported = drv_priv->drv_config.cfg_rxchainmask;
            break;

        case CHIP_PARAM_TXCHAINMASKLEGACY:
            supported = drv_priv->drv_config.cfg_txchainmasklegacy;
            break;

        case CHIP_PARAM_RXCHAINMASKLEGACY:
            supported = drv_priv->drv_config.cfg_rxchainmasklegacy;
            break;

        case CHIP_PARAM_CHAINMASK_SEL:
            supported = drv_priv->drv_config.cfg_chainmask_sel;
            break;

        case CHIP_PARAM_AMPDU:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            supported = drv_priv->drv_config.cfg_txaggr;
            break;

        case CHIP_PARAM_AMPDU_RX:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            supported = drv_priv->drv_config.cfg_rxaggr;
            break;

        case CHIP_PARAM_AMPDU_LIMIT:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            supported = drv_priv->drv_config.cfg_ampdu_limit;
            break;

        case CHIP_PARAM_AMPDU_SUBFRAMES:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            supported = drv_priv->drv_config.cfg_ampdu_subframes;
            break;

        case CHIP_PARAM_AGGR_PROT:
            pr_debug("<running> %s %d \n",__func__,__LINE__);
            supported = drv_priv->drv_config.cfg_aggr_prot;
            break;

        case CHIP_PARAM_TXPOWER_LIMIT:
            supported = drv_priv->drv_config.cfg_txpowlimit;
            break;

        case CHIP_PARAM_BURST_ACK:
            supported = drv_priv->drv_config.cfg_burst_ack;
            break;

        case CHIP_PARAM_ACK_POLICY:
            supported = drv_priv->drv_config.cfg_ampduackpolicy;
            break;

        case CHIP_PARAM_40MSUPPORT:
            supported = drv_priv->drv_config.cfg_40Msupport;
            break;

        case CHIP_PARAM_USE_EAP_LOWEST_RATE:
            supported = drv_priv->drv_config.cfg_eap_lowest_rate;
            break;

        case CHIP_PARAM_HWCSUM:
            supported = drv_priv->drv_config.cfg_checksumoffload;
            break;

        case CHIP_PARAM_HWTKIPMIC:
            supported = drv_priv->drv_config.cfg_tkipmicsupport;
            break;

        case CHIP_PARAM_WMM:
            supported = drv_priv->drv_config.cfg_haswme;
            break;

        case CHIP_PARAM_HT:
            supported = drv_priv->drv_config.cfg_htsupport;
            break;

        case CHIP_PARAM_VHT:
            supported = drv_priv->drv_config.cfg_vhtsupport;
            break;

        case CHIP_PARAM_UAPSD:
            supported = drv_priv->drv_config.cfg_uapsdsupported;
            break;

        case CHIP_PARAM_DS:
            supported =  drv_priv->drv_config.cfg_dssupport;
            break;

        case CHIP_PARAM_CIPHER_WEP:
            supported =  drv_priv->drv_config.cfg_wepsupport;
            break;

        case CHIP_PARAM_CIPHER_AES_CCM:
            supported =  drv_priv->drv_config.cfg_aessupport;
            break;

        case CHIP_PARAM_CIPHER_WPI:
            supported =  drv_priv->drv_config.cfg_wapisupport;
            break;

        case CHIP_PARAM_CIPHER_TKIP:
            supported =  drv_priv->drv_config.cfg_tkipsupport;
            break;

        case CHIP_PARAM_DISRATECONTROL:
            supported =  drv_priv->drv_config.cfg_disratecontrol;
            break;

        case CHIP_PARAM_BWAUTOCONTROL:
            supported = drv_priv->drv_config.cfg_bw_ctrl;
            break;

        case CHIP_PARAM_DEBUG:
            supported = aml_debug;
            break;

        case CHIP_PARAM_EAT_COUNT:
            supported = drv_priv->drv_config.cfg_eat_count_max;
            break;

        case CHIP_PARAM_AGGR_THRESH:
            supported = drv_priv->drv_config.cfg_aggr_thresh;
            break;

        case CHIP_PARAM_HRTIMER_INTERVAL:
            supported = drv_priv->drv_config.cfg_hrtimer_interval;
            break;

        default:
            supported = 0;
            break;
    }

    return supported;
}

static unsigned int process_drv_cfg_content(char *varbuf, unsigned int len)
{
    char *dp;
    bool findNewline;
    int column;
    unsigned int buf_len, n;
    unsigned int pad = 0;

    dp = varbuf;
    findNewline = false;
    column = 0;

    for (n = 0; n < len; n++) {
        if (varbuf[n] == '\r')
            continue;

        if (findNewline && varbuf[n] != '\n')
            continue;
        findNewline = false;
        if (varbuf[n] == '#') {
            findNewline = true;
            continue;
        }
        if (varbuf[n] == '\n') {
            if (column == 0)
                continue;
            *dp++ = 0;
            column = 0;
            continue;
        }
        *dp++ = varbuf[n];
        column++;
    }
    buf_len = (unsigned int)(dp - varbuf);
    if (buf_len % 4) {
        pad = 4 - buf_len % 4;
        if (pad && (buf_len + pad <= len)) {
            buf_len += pad;
        }
    }

    while (dp < varbuf + n)
        *dp++ = 0;

    return buf_len;
}

extern unsigned char get_s16_item(char *varbuf, int len, char *item, short *item_value);
extern unsigned char get_s8_item(char *varbuf, int len, char *item, char *item_value);
static unsigned char parse_drv_cfg_param(char *varbuf, int len)
{
    struct drv_private *drv_priv = drv_get_drv_priv();

    get_s8_item(varbuf, len, "cfg_band", &drv_priv->drv_config.cfg_band);
    //get_s16_item(varbuf, len, "cali_config", cali_config);
    AML_OUTPUT("======>>>>>> drv_config cfg_band = %d\n", drv_priv->drv_config.cfg_band);

    return 0;
}


int drv_cfg_load_from_file(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    struct kstat stat;
    mm_segment_t fs;
    int error = 0;
    struct file *fp;
    int size, len;
    char *content =  NULL;


    char conf_path[30] = "/vendor/etc/wifi/w1";
    unsigned char cfg_file[100];

    sprintf(cfg_file, "%s/aml_wifi_drv_cfg_%d.conf", conf_path, 0);

    fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open(cfg_file, O_RDONLY, 0);

    if (IS_ERR(fp)) {
        fp = NULL;
        goto err;
    }

    error = vfs_stat(cfg_file, &stat);
    if (error) {
        filp_close(fp, NULL);
        goto err;
    }

    size = (int)stat.size;
    if (size <= 0) {
        filp_close(fp, NULL);
        goto err;
    }

    content = ZMALLOC(size, "aml_drv_cfg", GFP_KERNEL);

    if (content == NULL) {
        filp_close(fp, NULL);
        goto err;
    }

    if (vfs_read(fp, content, size, &fp->f_pos) != size) {
        FREE(content, "aml_drv_cfg");
        filp_close(fp, NULL);
        goto err;
    }

    len = process_drv_cfg_content(content, size);
    parse_drv_cfg_param(content, len);

    FREE(content, "aml_drv_cfg");
    filp_close(fp, NULL);
    set_fs(fs);
#endif
    return 0;
err:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    set_fs(fs);
#endif
    return 1;
}
