#ifndef _DRV__RATE_SAMPLE_H
#define _DRV__RATE_SAMPLE_H

#ifndef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#endif

#define DRV_MGT_TXMAXTRY 4/* max number of trynum for management and control frames */

struct aml_ratecontrol
{
    signed char rate_index;
    unsigned char vendor_rate_code;/* hardware rate code */
    unsigned char trynum;
    unsigned char shortgi_en;
    unsigned char bw;
    unsigned short flags;
    unsigned int maxampdulen;
};

#define WIFINET_TX_ERROR 0x01
struct wifi_mac_tx_status
{
    int ts_flags;
    unsigned char ts_status;
    int ts_ratekbps;
    unsigned int ts_tstamp;
    unsigned short ts_seqnum;
    unsigned short ts_ackrssi;
    unsigned char ts_ratecode;
    unsigned char ts_rateindex;
    unsigned char ts_shortretry;
    unsigned char ts_longretry;
    unsigned char ts_finaltsi;
};

struct drv_rate_table
{
    int rateCount;/* NB: for proper padding */
    unsigned char dot11rate_to_idx[256];/* back mapping */
    struct
    {
        unsigned char valid;/* valid for rate control use */
        unsigned char phy;/* CCK/OFDM/XR */
        unsigned int rateKbps;/* transfer rate in kbs */
        unsigned char vendor_rate_code;/* rate for h/w descriptors */
        unsigned char shortPreamble;/* mask for enabling short * preamble in CCK rate code */
        unsigned char dot11Rate;/* value for supported rates* info element of MLME */
        unsigned char controlRate;/* index of next lower basic* rate; used for dur. calcs */
        unsigned short lpAckDuration;/* long preamble ACK duration */
        unsigned short spAckDuration;/* short preamble ACK duration*/
    } info[102];
};

enum
{
    HAL_NOSUPPORT = 0x0,/* NB: lots of code assumes false is zero */
    HAL_SUPPORT  = 0x1,
    HAL_RATECTRL  =0x2,
};

enum
{
    MCS_HT20,
    MCS_HT20_SGI,
    MCS_HT40,
    MCS_HT40_SGI,
    MCS_VHT80,
    MCS_VHT80_SGI,
    MCS_VHT160,
    MCS_VHT160_SGI
};

/***  aml txdesc***/
#define BITS_PER_BYTE			8
#define OFDM_PLCP_BITS                  22
#define L_STF                           8
#define L_LTF                           8
#define L_SIG                           4
#define HT_SIG                          8
#define HT_GF_STF                       8
#define HT_LTF1                         8
#define HT_STF                          4
#define HT_LTF_STREAM1                  4
#define VHT_SIG_A                       8
#define VHT_STF                         4
#define VHT_LTF                         4
#define VHT_SIG_B                       4
#define HT_LTF(_ns)         (4 * (_ns))
#define MCS_2_STREAMS(_mcs)             ((((_mcs)&0x78)>>3)+1)
#define SYMBOL_TIME(_ns)                ((_ns) << 2)            // ns * 4 us
#define SYMBOL_TIME_HALFGI(_ns)         (((_ns) * 18 + 4) / 5)  // ns * 3.6 us
#define NUM_SYMBOLS_PER_USEC(_usec)     (_usec >> 2)
#define NUM_SYMBOLS_PER_USEC_HALFGI(_usec) (((_usec*5)-4)/18)
#define SIGNAL_EXTENSION_VALUE          6              /* Value of the signal extension          */


/* 802.11n related timing definitions */
#define HT_RC_2_MCS(_rc)    ((_rc) & 0xf)
#define HT_RC_2_STREAMS(_rc)    ((((_rc) & 0x78) >> 3) + 1)

#define OFDM_SIFS_TIME      16
#define OFDM_PREAMBLE_TIME  20
#define OFDM_PLCP_BITS      22
#define OFDM_SYMBOL_TIME    4
#define DELT (0)

// rate control modules
struct ratecontrol_ops
{
    char name[16];
    void (*rate_attach)(void);
    void (*rate_detach)(void);
    void (*rate_newassoc)(void *p_sta);
    void (*rate_disassoc)(void *p_sta);
    unsigned char (*rate_findrate)(struct aml_ratecontrol ratectrl[], void *p_sta);
    void (*rate_tx_complete)(struct aml_ratecontrol *rc, void *p_sta);
};

extern struct drv_rate_table amluno_11bgnac_table;
extern struct ratecontrol_ops minstrel_ops;
extern int max_4ms_framelen[MCS_VHT160_SGI +1][32];

#endif /* _DRV__RATE_SAMPLE_H */
