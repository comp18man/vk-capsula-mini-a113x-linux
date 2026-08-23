/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 mac  layer Software
 *
 * Description:
 *     wifi_mac layer interface function/interface use by driver layer
 *
 *
 ****************************************************************************************
 */

#ifndef _WIFI_MAC_ARP_H_
#define _WIFI_MAC_ARP_H_

#define WIFI_ARP_GET_AP_IP BIT(0)

#define IPV4_LEN 4
#define ETHERNET_HW_TYPE 1
#define ARP_HW_ADDR_LEN 6
#define ARP_PROTO_ADDR_LEN 4
#define ARP_REQUEST 1
#define ARP_RESPONSE 2

typedef struct wifi_arp_pkt {
    unsigned short hw_type;
    unsigned short proto_type;
    unsigned char hw_len;
    unsigned char proto_len;
    unsigned short op;
    unsigned char src_mac_addr[WIFINET_ADDR_LEN];
    unsigned char src_ip_addr[IPV4_LEN];
    unsigned char dst_mac_addr[WIFINET_ADDR_LEN];
    unsigned char dst_ip_addr[IPV4_LEN];
} wifi_arp_pkt;

typedef struct wifi_mac_iphdr {
    unsigned char version;
    unsigned char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    unsigned int saddr;
    unsigned int daddr;
} wifi_ip_header;

typedef struct wifi_udp_pkt {
    unsigned short src_port;
    unsigned short dst_port;
    unsigned short len;
    unsigned short checksum;
    unsigned char data[0];
} wifi_udp_pkt;


#endif//_WIFI_MAC_ARP_H_

