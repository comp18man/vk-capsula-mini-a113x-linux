#ifndef __WIFI_MAC_RX_STATUS_H__
#define __WIFI_MAC_RX_STATUS_H__

struct wifi_mac_rx_status
{
    int rs_flags;
    int rs_rssi;
    int rs_datarate;
    int rs_wnet_vif_id;
    int rs_channel;
    int rs_keyid;
    int rs_vendor_rate;
    int rs_bw;

    union
    {
        unsigned char data[8];
        u_int64_t tsf;
    } rs_tstamp;
};

#endif//__WIFI_MAC_RX_STATUS_H__
