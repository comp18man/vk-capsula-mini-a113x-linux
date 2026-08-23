/*****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     Storage to save configuration when init test for hal layer throughput
 * Author : Boatman Yang(xuexing.yang@amlogic.com)
 *
 * Date:    20160901
 ****************************************************************************************
 */

#ifndef _DRV_HAL_THR_CONF_H
#define _DRV_HAL_THR_CONF_H

#define STA1_VMAC0_KEY_LEN  	32

#define STA1_VMAC0_IBSS				0

#define DP_SEC_KEY_AUTH 1
#define DP_SEC_KEY_SUPP 0

#define TYPE_COMMON                  1  //send common packet, mpdu->ack->mpdu->ack...,every mpdu need catch the channel
#define TYPE_AMPDU                   2  // send ampdu packet
#define TYPE_AMSDU                   3  // send amsdu packet
#define TYPE_AMSDU_AMPDU             4  // send ampdu packet with amsdu packet included
#define TYPE_COMMON_NOACK            5  //mpdu->mpdu->mpdu...
#define TYPE_BURST_ACK               6  //send common packet until one txop used up
#define TYPE_BURST_BA                7
#define TYPE_STOP_TX                 8

#define TYPE_MANAGE 0
/*The following 1 item modified by xman for hal thr test*/
#define STA1_VMAC0_SEND_TYPE  TYPE_BURST_BA //TYPE_AMPDU, Boatman modify for hal lay throughput test.

#define STA1_VMAC0_SEND_RATE		0x85//0x85 for mcs5, 0x84 for mcs4,original 0x87
#define  STA1_VMAC0_SEND_40M        0 //0: 20M,1:40,2:80M channel bandwidth

#define STA1_VMAC0_AGG_NUM			2

#define WIFI_IS_GREENFIELD          BIT(5)

#define STA2_VMAC1_SEND_TID_CTRL  		1//use STA2_VMAC1_SEND_TID value ,if 0 ,send frame form all tid 0~7
#define STA2_VMAC1_SEND_TID  			3      //send frame for which tid, 5 for video and 6 for voice
#define STA2_VMAC1_SEND_FRAME_NUM  		1500//send frame num for test,original 100, xman modified.
#define STA2_VMAC1_RX_FRAME_DUMP  		1//dump rx frame data
#define STA1_VMAC0_SEND_LEN			    1200//need 10~1500

//reserve param 1

#define STA2_VMAC1_TKIPMIC_HW		0//if tkip txmic key rx frame data
#define STA2_TCPIP_CHECKSUM			0




#endif