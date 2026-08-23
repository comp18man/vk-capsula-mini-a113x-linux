#ifndef __WIFI_CSI_H
#define __WIFI_CSI_H

extern struct iw_handler_def w1_iw_handle;

typedef enum {
    CSI_FRAME_TYPE_CCK = 0,
    CSI_FRAME_TYPE_11a = 1,
    CSI_FRAME_TYPE_11n = 2,
    CSI_FRAME_TYPE_11N_GREEN_FIELD = 3,
    CSI_FRAME_TYPE_11AC = 4,
    CSI_FRAME_TYPE_11BA = 5,
    CSI_FRAME_TYPE_11AX_SU = 8,
    CSI_FRAME_TYPE_11AX_MU = 9,
    CSI_FRAME_TYPE_11AX_ER_SU = 10,
    CSI_FRAME_TYPE_11AX_TRIGGER = 11
}csi_protocol_mode_e;


typedef struct{
    short i;
    short q;
} csi_complex;


typedef struct {
    unsigned long long time_stamp;
    unsigned char mac_ra[6];
    unsigned char mac_ta[6];
    unsigned char frequency_band;
    unsigned char bw;
    char rssi;
    unsigned char protocol_mode;
    unsigned char frame_type;
    unsigned char chain_num;
    unsigned char csi_len;
    unsigned char snr;
    unsigned char primary_channel_index;
    unsigned char noise;
    unsigned char phyerr;
    unsigned char rate;
    unsigned int extra_information;
    unsigned short agc_code;
    short phase_incr;
    unsigned short channel;
    unsigned char reserved[2];
    unsigned int packet_idx;
    unsigned char csi[1024];
}csi_stream_t;

#endif
