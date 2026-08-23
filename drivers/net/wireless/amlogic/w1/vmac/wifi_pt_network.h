/*****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 driver  layer Software
 *
 * Description:
 *     Initialization network related APIs for driver hal layer throughput test
 * Author : Boatman Yang(xuexing.yang@amlogic.com)
 *
 * Date:    20160901
 ****************************************************************************************
 */
#ifndef _DRV_HAL_THR_NETWORK_H
#define _DRV_HAL_THR_NETWORK_H


enum {
        NET_MESSAGE_ARP_REQUEST,
        NET_MESSAGE_ARP_REPLY,
        NET_MESSAGE_PING_REQUEST,
        NET_MESSAGE_PING_REPLY,
        NET_MESSAGE_UDP,
        NET_SIM_CHECK,
	NET_TCP_IP,
};

struct _Network {
        //
        // Addresses
        //
        unsigned char* src_mac_address;
        unsigned char*  dst_mac_address;
        unsigned int src_ip_address;
        unsigned int dst_ip_address;

        //
        // Tx parameters
        //
        unsigned int tx_timestamp;
        unsigned short tx_ip_sequence;
        unsigned short tx_ping_sequence;
        unsigned char  tx_rate;

        unsigned char  state;

        //
        // Total byte count in a period
        //
        unsigned int timer_period;
        unsigned int rx_byte_count;
        unsigned int tx_byte_count;
        unsigned int tx_packet_count;
        unsigned int tx_retry_count;
        unsigned int tx_rtsretry_count;
        unsigned int tx_ackrssi_count;

        //
        // Event counters
        //
        unsigned int tx_arp_request_count;
        unsigned int tx_arp_reply_count;
        unsigned int tx_ping_request_count;
        unsigned int tx_ping_reply_count;
        unsigned int tx_udp_count;
        unsigned int tx_fail_count;
        unsigned int tx_pool_error_count;

        unsigned int rx_arp_request_count;
        unsigned int rx_arp_reply_count;
        unsigned int rx_ping_request_count;
        unsigned int rx_ping_reply_count;
        unsigned int rx_udp_count;
        unsigned int rx_bad_ethertype_count;
        unsigned int rx_bad_ip_address_count;
        unsigned int rx_bad_ip_protocol_count;
        unsigned int rx_bad_icmp_type_count;
        unsigned int rx_beacon_count;
        unsigned int net_connect;
} ;

//unsigned char unsigned char,unsigned short-->unsigned short, unsigned int--->unsigned int
//unsigned short Net_WriteArp( unsigned char* start, unsigned short opcode ,unsigned char vmac_id)
void Net_Task(void);
unsigned char * Net_GetNextPacketDes(void);
unsigned short MacFrame_GetHeaderSize(unsigned int frame_control);
void Net_Receive( unsigned char* packet ,unsigned short len,unsigned char rssi);
void prepare_test_hal_layer_thr_init(int usrtesttype);

#endif
