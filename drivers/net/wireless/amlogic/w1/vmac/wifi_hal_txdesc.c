/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2010-2014
 *
 * Project: 11N 80211 HAL  layer Software
 *
 * Description:
 *   firmware and hal interface descripter bulid module
 *
 *
 ****************************************************************************************
 */
#ifdef HAL_SIM_VER
#ifdef FW_NAME
namespace FW_NAME
{
#endif
#endif

#include "wifi_hal_cmd.h"
#include "wifi_hal_com.h"
#include "wifi_hal.h"
#include "wifi_hal_txdesc.h"

#ifdef HAL_FPGA_VER
#include "wifi_mac.h"
#endif

// Private data structures
//some packets are sent by hi, some packets are sent by fi, so both need the rate control tables
static struct rate_control our_rate_controls[ WIFI_11BG_MAX +1];
static struct ht_mcs_control our_htmcs_controls[ WIFI_11N_MAX+1];
static struct vht_mcs_control our_vhtmcs_controls[ WIFI_11AC_MAX+1];
//short preamble
static const unsigned short our_ack_time_short[WIFI_11BG_MAX+1]=
{
    314,/*PHY_RATE_1_LONG*/
    162,/*PHY_RATE_2_SHORT*/
    127,/*PHY_RATE_5_SHORT*/
    117,/*PHY_RATE_11_SHORT*/
    60, /*PHY_RATE_6   = 0xB,*/
    60, /*PHY_RATE_9   = 0xF,*/
    48, /*PHY_RATE_12  = 0xA,*/
    48, /*PHY_RATE_18  = 0xE,*/
    44, /*PHY_RATE_24  = 0x9,*/
    44, /*PHY_RATE_36  = 0xD,*/
    44, /*PHY_RATE_48  = 0x8,*/
    44, /*PHY_RATE_54  = 0xC,*/
};
//long preamble
static const unsigned short our_ack_time_11b_long[]=
{
    314,/*PHY_RATE_1_LONG*/
    258,/*PHY_RATE_2_LONG*/
    223,/*PHY_RATE_5_LONG*/
    213,/*PHY_RATE_11_LONG*/
};

static const unsigned short our_ba_txtime_short[WIFI_11BG_MAX+1] =
{
    362 , /*rateid=0*/
    234 , /*rateid=1*/
    153 , /*rateid=2*/
    130 , /*rateid=3*/
    84 , /*rateid=4*/
    68 , /*rateid=5*/
    60 , /*rateid=6*/
    52 , /*rateid=7*/
    48 , /*rateid=8*/
    44 , /*rateid=9*/
    44 , /*rateid=10*/
    44 , /*rateid=11*/
};

static const unsigned short  our_ba_txtime_11b_long[4] =
{
    458 ,  /*rateid=0*/
    330 ,  /*rateid=1*/
    249 ,  /*rateid=2*/
    226 ,  /*rateid=3*/
};

static const unsigned short ht_bits_per_symbol[WIFI_11N_MAX+1][2] =
{
    /* 20MHz 40MHz */
    {    26,   54 },     //  0: BPSK
    {    52,  108 },     //  1: QPSK 1/2
    {    78,  162 },     //  2: QPSK 3/4
    {   104,  216 },     //  3: 16-QAM 1/2
    {   156,  324 },     //  4: 16-QAM 3/4
    {   208,  432 },     //  5: 64-QAM 2/3
    {   234,  486 },     //  6: 64-QAM 3/4
    {   260,  540 },     //  7: 64-QAM 5/6
};

static const unsigned short vht_bits_per_symbol[WIFI_11AC_MAX+1][4] =
{
    /* 20MHz 40MHz        80M      160M                          */
    {    26,     54,            117,     234},     //  0: BPSK
    {    52,     108,          234,     468},     //  1: QPSK 1/2
    {    78,     162,          351,     702},     //  2: QPSK 3/4
    {    104,   216,          468,     936 },     //  3: 16-QAM 1/2
    {    156,   324,          702,     1404},     //  4: 16-QAM 3/4
    {    208,   432,          936,     1872},     //  5: 64-QAM 2/3
    {    234,   486,          1053,   2106 },     //  6: 64-QAM 3/4
    {    260,   540,          1170,   2340},     //  7: 64-QAM 5/6
    {    312,   648,          1404,   2808},     //  8: BPSK
    {    0,      720,          1560,   3120},     //  9: QPSK 1/2

};

static const unsigned char ht_ba_txtime[WIFI_11N_MAX+1][2/*b_shortGI*/][2/*b_40M*/]=
{
    { {80 ,60  } , {76 ,58  } , }, /*mcs0*/
    { {60 ,48  } , {58 ,47  } , }, /*mcs1*/
    { {52 ,44  } , {51 ,44  } , }, /*mcs2*/
    { {48 ,44  } , {47 ,44  } , }, /*mcs3*/
    { {44 ,40  } , {44 ,40  } , }, /*mcs4*/
    { {44 ,40  } , {44 ,40  } , }, /*mcs5*/
    { {44 ,40  } , {44 ,40  } , }, /*mcs6*/
    { {44 ,40  } , {44 ,40  } , }, /*mcs7*/
};

static const unsigned char vht_ba_txtime[WIFI_11AC_MAX+1]=
{
    68, /*mcs0*/
    44, /*mcs1*/
    44, /*mcs2*/
    32, /*mcs3*/
    32, /*mcs4*/
    32, /*mcs5*/
    32, /*mcs6*/
    32, /*mcs7*/
    32, /*mcs8*/
    32, /*mcs9*/
};

static const unsigned char ht_ack_txtime[WIFI_11N_MAX+1][2][2] =
{
    { {60 ,48  } , {58 ,47  } , }, /*mcs0*/
    { {48 ,44  } , {47 ,44  } , }, /*mcs1*/
    { {44 ,40  } , {44 ,40  } , }, /*mcs2*/
    { {44 ,40  } , {44 ,40  } , }, /*mcs3*/
    { {40 ,40  } , {40 ,40  } , }, /*mcs4*/
    { {40 ,40  } , {40 ,40  } , }, /*mcs5*/
    { {40 ,40  } , {40 ,40  } , }, /*mcs6*/
    { {40 ,40  } , {40 ,40  } , }, /*mcs7*/
};

struct wifi_cfg_mib wifi_conf_mib;


unsigned char tmp_dot11_preamble_type;
//
// Public operations
//
void hal_tx_desc_init(void)
{
    struct rate_control* control;
    struct ht_mcs_control* htmcs_control;
    struct vht_mcs_control* vhtmcs_control;
    //
    // FIXME: fill this table based on beacon and probe response
    //
    DBG_ENTER();

    memset(&wifi_conf_mib,0,sizeof(wifi_conf_mib));
    wifi_conf_mib.dot11FragmentationThreshold = 2000;
    wifi_conf_mib.dot11PreambleType =PREAMBLE_LONG;
    memset(&wifi_conf_mib.cmddata[0],-1, MAX_HI_CMD*sizeof(wifi_conf_mib.cmddata[0]));

    control = &our_rate_controls[ WIFI_11B_1M];
    control->bit_time   = 262144;// = 2^18 / 1
    control->rts_rate   = WIFI_11B_1M;
    control->ack_rate  = WIFI_11B_1M;
    control->tx_power = 0;

    control = &our_rate_controls[ WIFI_11B_2M ];
    control->bit_time   = 131072;// = 2^18 / 2
    control->rts_rate   = WIFI_11B_1M;
    control->ack_rate  = WIFI_11B_1M;
    control->tx_power = 0;

    control = &our_rate_controls[ WIFI_11B_5M ];
    control->bit_time = 47662;// = 2^18/5.5 = 47662.545454...
    control->rts_rate = WIFI_11B_5M;
    control->ack_rate = WIFI_11B_5M;
    control->tx_power = 1;

    control = &our_rate_controls[ WIFI_11B_11M ];
    control->bit_time = 23831;// = 2^18/11 = 23831.272727...
    control->rts_rate = WIFI_11B_5M;
    control->ack_rate = WIFI_11B_5M;
    control->tx_power = 1;

    control = &our_rate_controls[ WIFI_11G_6M ];
    control->bit_time = 43690;// = 2^18/6 = 43690.666...
    control->rts_rate = WIFI_11G_6M;
    control->ack_rate = WIFI_11G_6M;
    control->tx_power = 2;

    control = &our_rate_controls[ WIFI_11G_9M ];
    control->bit_time = 29127;// = 2^18/9 = 29127.111...
    control->rts_rate = WIFI_11G_6M;
    control->ack_rate = WIFI_11G_6M;
    control->tx_power = 2;

    control = &our_rate_controls[ WIFI_11G_12M ];
    control->bit_time = 21845;// = 2^18/12 = 21845.333...
    control->rts_rate = WIFI_11G_12M;
    control->ack_rate = WIFI_11G_12M;
    control->tx_power = 2;

    control = &our_rate_controls[ WIFI_11G_18M ];
    control->bit_time = 14563;// = 2^18/18 = 14563.555...
    control->rts_rate = WIFI_11G_12M;
    control->ack_rate = WIFI_11G_12M;
    control->tx_power = 3;

    control = &our_rate_controls[ WIFI_11G_24M ];
    control->bit_time = 10922;// = 2^18/24 = 10922.666...
    control->rts_rate = WIFI_11G_12M;
    control->ack_rate = WIFI_11G_12M;
    control->tx_power = 3;

    control = &our_rate_controls[ WIFI_11G_36M ];
    control->bit_time = 7281;// = 2^18/36 = 7281.777...
    control->rts_rate = WIFI_11G_24M;
    control->ack_rate = WIFI_11G_24M;
    control->tx_power = 3;

    control = &our_rate_controls[ WIFI_11G_48M ];
    control->bit_time = 5461;// = 2^18/48 = 5461.333...
    control->rts_rate = WIFI_11G_24M;
    control->ack_rate = WIFI_11G_24M;
    control->tx_power = 4;

    control = &our_rate_controls[ WIFI_11G_54M ];
    control->bit_time = 4854;// = 2^18/54 = 4854.518518518...
    control->rts_rate = WIFI_11G_24M;
    control->ack_rate = WIFI_11G_24M;
    control->tx_power = 5;
    //
    ///mcs
    //
    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS0)];
    htmcs_control->rts_rate = WIFI_11G_6M;
    htmcs_control->ack_rate = WIFI_11G_6M;
    htmcs_control->tx_power = 6;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS1)];
    htmcs_control->rts_rate = WIFI_11G_6M;
    htmcs_control->ack_rate = WIFI_11G_6M;
    htmcs_control->tx_power = 6;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS2)];
    htmcs_control->rts_rate = WIFI_11G_6M;
    htmcs_control->ack_rate = WIFI_11G_6M;
    htmcs_control->tx_power = 6;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS3)];
    htmcs_control->rts_rate = WIFI_11G_12M;
    htmcs_control->ack_rate = WIFI_11G_12M;
    htmcs_control->tx_power = 7;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS4)];
    htmcs_control->rts_rate = WIFI_11G_12M;
    htmcs_control->ack_rate = WIFI_11G_12M;
    htmcs_control->tx_power = 7;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS5)];
    htmcs_control->rts_rate = WIFI_11G_12M;
    htmcs_control->ack_rate = WIFI_11G_12M;
    htmcs_control->tx_power = 7;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS6)];
    htmcs_control->rts_rate = WIFI_11G_24M;
    htmcs_control->ack_rate = WIFI_11G_24M;
    htmcs_control->tx_power = 8;

    htmcs_control = &our_htmcs_controls[GET_HT_MCS(WIFI_11N_MCS7)];
    htmcs_control->rts_rate = WIFI_11G_24M;
    htmcs_control->ack_rate = WIFI_11G_24M;
    htmcs_control->tx_power = 9;

    //vht mcs
    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS0)];
    vhtmcs_control->rts_rate = WIFI_11G_6M;
    vhtmcs_control->ack_rate = WIFI_11G_6M;
    vhtmcs_control->tx_power = 10;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS1)];
    vhtmcs_control->rts_rate = WIFI_11G_6M;
    vhtmcs_control->ack_rate = WIFI_11G_6M;
    vhtmcs_control->tx_power = 10;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS2)];
    vhtmcs_control->rts_rate = WIFI_11G_6M;
    vhtmcs_control->ack_rate = WIFI_11G_6M;
    vhtmcs_control->tx_power = 10;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS3)];
    vhtmcs_control->rts_rate = WIFI_11G_12M;
    vhtmcs_control->ack_rate = WIFI_11G_12M;
    vhtmcs_control->tx_power = 11;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS4)];
    vhtmcs_control->rts_rate = WIFI_11G_12M;
    vhtmcs_control->ack_rate = WIFI_11G_12M;
    vhtmcs_control->tx_power = 11;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS5)];
    vhtmcs_control->rts_rate = WIFI_11G_12M;
    vhtmcs_control->ack_rate = WIFI_11G_12M;
    vhtmcs_control->tx_power = 11;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS6)];
    vhtmcs_control->rts_rate = WIFI_11G_24M;
    vhtmcs_control->ack_rate = WIFI_11G_24M;
    vhtmcs_control->tx_power = 12;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS7)];
    vhtmcs_control->rts_rate = WIFI_11G_24M;
    vhtmcs_control->ack_rate = WIFI_11G_24M;
    vhtmcs_control->tx_power = 13;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS8)];
    vhtmcs_control->rts_rate = WIFI_11G_24M;
    vhtmcs_control->ack_rate = WIFI_11G_24M;
    vhtmcs_control->tx_power = 14;

    vhtmcs_control = &our_vhtmcs_controls[GET_VHT_MCS(WIFI_11AC_MCS9)];
    vhtmcs_control->rts_rate = WIFI_11G_24M;
    vhtmcs_control->ack_rate = WIFI_11G_24M;
    vhtmcs_control->tx_power = 15;

    DBG_EXIT();
}

unsigned int hal_tx_desc_nonht_mode(unsigned char date_rate ,unsigned char channel_bw)
{
    if ((date_rate == WIFI_11B_1M)||(date_rate == WIFI_11B_2M))
    {
        return ERP_DSSS;
    }
    else if ((date_rate == WIFI_11B_11M)||(date_rate == WIFI_11B_5M))
    {
        return ERP_CCK;
    }

    if ((channel_bw == CHAN_BW_40M ||channel_bw == CHAN_BW_80M ) &&
        !IS_MCS_RATE(date_rate) )
    {
        return NON_HT_DUP_OFDM;
    }

    return ERP_OFDM;
}

unsigned char hal_tx_desc_get_power( unsigned char rate )
{
    if (IS_HT_RATE(rate))
    {
        return  our_htmcs_controls[ GET_HT_MCS(rate) ].tx_power;
    }
    else if (IS_VHT_RATE(rate))
    {
        return  our_vhtmcs_controls[ GET_HT_MCS(rate) ].tx_power;
    }
    else
    {
        return   our_rate_controls[ rate ].tx_power;
    }

}

static inline unsigned char Hal_TxDescriptor_GetPreamble( unsigned char rate,unsigned char preambletype )
{
    return ( IS_OFMD_RATE(rate) ? 20: ((preambletype==PREAMBLE_SHORT)?96:192) );
}

static unsigned char Hal_TxDescriptor_GetAckRate( unsigned char data_rate )
{
    if (IS_HT_RATE(data_rate) )
    {
        data_rate = GET_HT_MCS(data_rate);
        ASSERT( data_rate <= WIFI_11N_MAX );
        return our_htmcs_controls[ data_rate ].ack_rate;
    }
    else if (IS_VHT_RATE(data_rate) )
    {
        data_rate = GET_VHT_MCS(data_rate);
        ASSERT( data_rate <= WIFI_11AC_MAX );
        return our_vhtmcs_controls[ data_rate ].ack_rate;
    }
    else
    {
        ASSERT( data_rate <= WIFI_11BG_MAX );
        return our_rate_controls[ data_rate ].ack_rate;
    }
}

static unsigned char Hal_TxDescriptor_GetRtsRate( unsigned char data_rate )
{
    if (IS_HT_RATE(data_rate) )
    {
        data_rate = GET_HT_MCS(data_rate);
        ASSERT( data_rate <= WIFI_11N_MAX );
        return our_htmcs_controls[ data_rate ].rts_rate;
    }
    else if (IS_VHT_RATE(data_rate) )
    {
        data_rate = GET_VHT_MCS(data_rate);
        ASSERT( data_rate <= WIFI_11AC_MAX );
        return our_vhtmcs_controls[ data_rate ].rts_rate;
    }
    else
    {
        ASSERT( data_rate <= WIFI_11BG_MAX );
        return our_rate_controls[ data_rate ].rts_rate;
    }
}

static inline unsigned char Hal_TxDescriptor_HT_GetPreamble(unsigned char mcs ,unsigned char green)
{

    /*
     unsigned char streams = 0;

    addup duration for legacy/ht training and signal fields

    streams =   MCS_2_STREAMS(GET_MCS(mcs));//we only support streams = 1
    return (!green)?( L_STF + L_LTF + L_SIG + HT_SIG + HT_STF + HT_LTF(streams))
        :( HT_SIG + HT_GF_STF+ HT_LTF1+ HT_LTF(streams));

    */
    //we only support streams = 1?
    return (green)?( HT_SIG + HT_GF_STF+ HT_LTF1+ HT_LTF_STREAM1)
        :( L_STF + L_LTF + L_SIG + HT_SIG + HT_STF + HT_LTF_STREAM1);
}

static inline unsigned char Hal_TxDescriptor_VHT_GetPreamble(unsigned char mcs ,unsigned char green)
{

    /*
     unsigned char streams = 0;

    addup duration for legacy/ht training and signal fields

    streams =   MCS_2_STREAMS(GET_MCS(mcs));//we only support streams = 1
    return (!green)?( L_STF + L_LTF + L_SIG + HT_SIG + HT_STF + HT_LTF(streams))
        :( HT_SIG + HT_GF_STF+ HT_LTF1+ HT_LTF(streams));

    */
    //we only support streams = 1
    //return (green)?( HT_SIG + HT_GF_STF+ HT_LTF1+ HT_LTF_STREAM1)
    //    :( L_STF + L_LTF + L_SIG + HT_SIG + HT_STF + HT_LTF_STREAM1);
    green = 0;
    //(green)?( HT_SIG + HT_GF_STF+ HT_LTF1)
    return (green + L_STF + L_LTF + L_SIG + VHT_SIG_A  + VHT_STF +VHT_LTF+VHT_SIG_B);
}

/*+SIFS+TXTIME(pclp+data+signalextension)*/
static unsigned short Hal_TxDescriptor_HT_GetAckTime( unsigned char ackmcs,
    unsigned char b_shortGI,unsigned char b_rifs,unsigned char b_40M)
{
    /*
            return (b_rifs?2:(b_5G?16:10) )                                    //sifs
                   +hal_txdescriptor_ht_gettxTime(ackmcs,14,b_shortGI,b_40M,0)//ack txtime
                   +(b_5G?0:SIGNAL_EXTENSION_VALUE);//ack frame Signal Extension

    */
    ackmcs = GET_HT_MCS(ackmcs);
    return (b_rifs?2:16) + ht_ack_txtime[ackmcs][b_shortGI][b_40M];//ack txtime not have Signal Extension
}

/*+SIFS+TXTIME(pclp+data+signalextension)*/
static __inline unsigned short Hal_TxDescriptor_HT_GetBATime( unsigned char ackmcs,
    unsigned char b_shortGI,unsigned char b_rifs,unsigned char b_40M)
{
    /* ht_ba_txtime
            return (b_rifs?2:(b_5G?16:10) )                                    //sifs
                   +hal_txdescriptor_ht_gettxTime(ackmcs,14,b_shortGI,b_40M,0)//ack txtime
                   +(b_5G?0:SIGNAL_EXTENSION_VALUE);//ack frame Signal Extension
    */
    ackmcs = GET_HT_MCS(ackmcs);
    return (b_rifs?2:16)                                   //sifs +ack frame Signal Extension
           +ht_ba_txtime[ackmcs][b_shortGI][b_40M];//ack txtime not have Signal Extension
}

/*get ht frame tx time ,don't contain Signal Extension*/
static unsigned short hal_txdescriptor_ht_gettxTime(unsigned char mcs ,unsigned short pktlen,
    unsigned char shortGI,unsigned char bw,unsigned char green)
{
    unsigned int nbits = 0;
    unsigned short duration,nsymbits,nsymbols;

    mcs = GET_HT_MCS(mcs);
    if (bw > BW_40) {
        bw = BW_40;
    }

    /*
     * find number of symbols: PLCP + data
     */
    nbits = (pktlen << 3) + OFDM_PLCP_BITS;
    nsymbits = ht_bits_per_symbol[mcs][bw];
    nsymbols = (nbits + nsymbits - 1) / nsymbits;
    if (!shortGI)
        duration = SYMBOL_TIME(nsymbols);
    else
        duration = SYMBOL_TIME_HALFGI(nsymbols);

    duration += Hal_TxDescriptor_HT_GetPreamble(mcs,green);
    //duration + = Signal Extension;/* Value of the signal extension      SIGNAL_EXTENSION_VALUE    */

    return duration;
}

/*get vht frame tx time ,don't contain Signal Extension*/
static unsigned short Hal_TxDescriptor_VHT_GetTxTime(unsigned char mcs ,unsigned short pktlen,
    unsigned char shortGI, unsigned char bandwidth, unsigned char green)
{
    unsigned int nbits = 0;
    unsigned short duration,nsymbits,nsymbols;

    mcs = GET_VHT_MCS(mcs);
    /*
    * find number of symbols: PLCP + data
    */
    nbits = (pktlen << 3) + OFDM_PLCP_BITS;
    nsymbits = vht_bits_per_symbol[mcs][bandwidth];

    if (nsymbits == 0) {
        pr_debug("Hal_TxDescriptor_VHT_GetTxTime warming nsymbits=%d, bw=%d, mcs=%d\n ", nsymbits, bandwidth, mcs);
        nsymbols = 1;

    } else {
        nsymbols = (nbits + nsymbits - 1) / nsymbits;
    }

    if (!shortGI)
        duration = SYMBOL_TIME(nsymbols);
    else
        duration = SYMBOL_TIME_HALFGI(nsymbols);

    duration += Hal_TxDescriptor_VHT_GetPreamble(mcs,green);
    //duration + = Signal Extension;/* Value of the signal extension      SIGNAL_EXTENSION_VALUE    */

    return duration;
}

__INLINE unsigned short Hal_TxDescriptor_HT_GetDataTime(unsigned char mcs,
    unsigned short pktlen,unsigned char b_40M)
{

    unsigned int nbits=0;
    unsigned short duration,nsymbits,nsymbols;

    mcs = GET_HT_MCS(mcs);
    if (b_40M > BW_40) {
        b_40M = BW_40;
    }
    /*
    * find number of symbols: data
    */
    nbits = (pktlen << 3);
    nsymbits = ht_bits_per_symbol[mcs][b_40M];
    nsymbols = (nbits + nsymbits - 1) / nsymbits;
    duration = SYMBOL_TIME(nsymbols);

    return duration;
}

static unsigned short Hal_TxDescriptor_GetDuration( unsigned char rate, unsigned short length )
{
    unsigned short duration =0;
    length *= 8;

    if ( IS_OFMD_RATE(rate)  )
    {
        length += 16 + 6;// Add service and tail bits for OFDM
    }

    duration = ( our_rate_controls[ rate ].bit_time * length + ( ( 1 << 18 ) - 1 ) ) >> 18;

    if ( IS_OFMD_RATE(rate)  )
    {
        duration = ( duration + 3 ) & ~0x3;// Round up to multiple of 4us symbol time
    }

    return duration;
}
/*get legacy  frame tx time*/
static unsigned short Hal_TxDescriptor_GetLegacyTxTime(unsigned char data_rate,
    unsigned short pktlen,unsigned char preambletype )
{
    return Hal_TxDescriptor_GetPreamble( data_rate,preambletype )// data frame
           + Hal_TxDescriptor_GetDuration( data_rate, pktlen );
}

/*
L_LENGTH = (((TXTIME Signal Extension)  20)/4)*3-3
*/
unsigned short hal_tx_desc_get_len(unsigned char rate ,unsigned short pktlen,
        unsigned char shortGI,unsigned char bw,unsigned char green)
{
        if (IS_HT_RATE(rate))//ht
                return ((hal_txdescriptor_ht_gettxTime(rate,pktlen,shortGI,bw,green) - 20)>>2)*3-3 ;
        else if (IS_VHT_RATE(rate)) {
            return ((Hal_TxDescriptor_VHT_GetTxTime(rate,pktlen,shortGI,bw,green) - 20)>>2)*3-3 ;
        }else
                return pktlen;

}

static unsigned short Hal_TxDescriptor_GetAckTimeout( unsigned char data_rate,unsigned char preambletype)
{
    if (IS_HT_RATE(data_rate) || IS_VHT_RATE(data_rate)) {
        return 80 + PHY_TEST;
    } else {
        return 20 + PHY_TEST;
    }
}


/*+SIGNAL_EXTENSION_VALUE+TXTIME(pclp+data+signalextension)*/
static unsigned short Hal_TxDescriptor_GetAckTime(
    unsigned char data_rate ,
    unsigned char preambletype,
    unsigned char b_shortGI,
    unsigned char b_rifs,
    unsigned char b_40M)
{
    unsigned char AckRate = Hal_TxDescriptor_GetAckRate(data_rate);
    if (IS_HT_RATE(AckRate) )
    {
        return Hal_TxDescriptor_HT_GetAckTime(AckRate, b_shortGI, b_rifs,b_40M);
    }
    else if (IS_VHT_RATE(AckRate) )
    {
        return Hal_TxDescriptor_HT_GetAckTime(AckRate, b_shortGI, b_rifs,b_40M);
    }
    else
    {
        if ((preambletype == PREAMBLE_SHORT)||IS_OFMD_RATE(data_rate))
        {
            return our_ack_time_short[AckRate]+PHY_TEST;
        }
        else
        {
            return our_ack_time_11b_long[AckRate]+PHY_TEST;
        }
    }
}

static unsigned short Hal_TxDescriptor_GetBATime(
        unsigned char data_rate,
        unsigned char preambletype,
        unsigned char b_shortGI,
        unsigned char b_rifs,
        unsigned char      bw)
{
    unsigned char ackrate = Hal_TxDescriptor_GetAckRate(data_rate);

    if (IS_HT_RATE(ackrate))
    {
        ackrate = GET_HT_MCS(ackrate);
        return (b_rifs?2:16)//sifs +ack frame Signal Extension
               +ht_ba_txtime[ackrate][b_shortGI][bw]+PHY_TEST;//ack txtime not have Signal Extension
    }
    else if (IS_VHT_RATE(ackrate))
    {
        ackrate = GET_VHT_MCS(ackrate);
        return (b_rifs?2:16)//sifs +ack frame Signal Extension
               +vht_ba_txtime[ackrate]+PHY_TEST;//ack txtime not have Signal Extension
    }
    else
    {
        if ((preambletype == PREAMBLE_SHORT)||IS_OFMD_RATE(data_rate))
        {
            return    our_ba_txtime_short[ackrate] +PHY_TEST;//block ack tx time+SIFS
        }
        else
        {
            return   our_ba_txtime_11b_long[ackrate]+PHY_TEST ; //block ack tx time +SIFS
        }
    }
}

static unsigned int Hal_TxDescriptor_GetTxTime(struct hi_agg_tx_desc* HiTxDesc)
{
        //fixme :need to add signal extern
        if (IS_HT_RATE(DESC_RATE)) {
                return  hal_txdescriptor_ht_gettxTime(DESC_RATE,
                                                      DESC_LEN,
                                                      DESC_IS_SHORTGI,
                                                      DESC_BANDWIDTH,
                                                      DESC_IS_GREEN);
        }
        else if (IS_VHT_RATE(DESC_RATE)) {
                return  Hal_TxDescriptor_VHT_GetTxTime(DESC_RATE,
                                                      DESC_LEN,
                                                      DESC_IS_SHORTGI,
                                                      DESC_BANDWIDTH,
                                                      DESC_IS_GREEN);
        }
        else {
                return Hal_TxDescriptor_GetLegacyTxTime(DESC_RATE,
                                                        DESC_LEN,
                                                        DESC_PREMBLETYPE);
        }
}

__INLINE unsigned int Hal_TxDescriptor_GetTxTimeBA(struct hi_agg_tx_desc* HiTxDesc)
{
        return Hal_TxDescriptor_GetTxTime( HiTxDesc)
               +Hal_TxDescriptor_GetBATime(DESC_RATE,DESC_PREMBLETYPE,DESC_IS_SHORTGI,DESC_RIFS,  DESC_BANDWIDTH);
}

__INLINE unsigned int Hal_TxDescriptor_GetCtsTime(struct hi_agg_tx_desc* HiTxDesc)
{
        if (IS_HT_RATE(DESC_RATE)) {
                return 16 // SIFS
                       + hal_txdescriptor_ht_gettxTime( DESC_RATE, DESC_LEN,
                            DESC_IS_SHORTGI, DESC_BANDWIDTH, DESC_IS_GREEN) // data frame
                       + Hal_TxDescriptor_GetAckTime(DESC_RATE, PREAMBLE_SHORT,
                            DESC_IS_SHORTGI, DESC_RIFS, DESC_BANDWIDTH);// ack + SIFS
        }
        else if (IS_VHT_RATE(DESC_RATE)) {
                return 16  // SIFS
                       + Hal_TxDescriptor_VHT_GetTxTime( DESC_RATE, DESC_LEN,
                            DESC_IS_SHORTGI, DESC_BANDWIDTH, DESC_IS_GREEN) // data frame
                       + Hal_TxDescriptor_GetAckTime(DESC_RATE, PREAMBLE_SHORT,
                            DESC_IS_SHORTGI, DESC_RIFS, DESC_BANDWIDTH); // ack + SIFS
        }
        else {
                ASSERT( DESC_RATE <= WIFI_11BG_MAX );
                if ((DESC_PREMBLETYPE == PREAMBLE_SHORT)||IS_OFMD_RATE(DESC_RATE)) {
                        return ( IS_OFMD_RATE(DESC_RATE) ?  16 :10 )          // SIFS
                               + Hal_TxDescriptor_GetPreamble( DESC_RATE,DESC_PREMBLETYPE ) // data frame
                               + Hal_TxDescriptor_GetDuration( DESC_RATE, DESC_LEN )
                               + our_ack_time_short[our_rate_controls[ DESC_RATE ].ack_rate] // CTS + SIFS
                               ;
                }
                else {
                        return (10 )          // SIFS
                               + Hal_TxDescriptor_GetPreamble( DESC_RATE,DESC_PREMBLETYPE ) // data frame
                               + Hal_TxDescriptor_GetDuration( DESC_RATE, DESC_LEN )
                               + our_ack_time_11b_long[our_rate_controls[ DESC_RATE ].ack_rate] // CTS + SIFS
                               ;
                }
        }
}

unsigned char hal_mac_frame_type( unsigned int frame_control, unsigned int type )
{
    return (( frame_control & ( FRAME_TYPE_MASK | FRAME_SUBTYPE_MASK ) ) == type);
}

void assign_tx_desc_pn(unsigned char is_bc, unsigned char vid,
    unsigned char sta_id, struct hi_tx_desc *tx_page, unsigned char encrypt_type) {
    struct hal_private *hal_priv = hal_get_priv();
    unsigned long long *PN = NULL;

    if (encrypt_type == WIFI_NO_WEP) {
        return;
    }

    PN_LOCK();
    if (is_bc) {
        PN = (unsigned long long *)hal_priv->mRepCnt[vid].txPN;
    } else {
        PN = (unsigned long long *)hal_priv->uRepCnt[vid][sta_id].txPN[TX_UNICAST_REPCNT_ID];
    }
    //pr_debug("zy:pn=0x%x\n",*PN);
    switch (encrypt_type) {
        case WIFI_TKIP:
            memcpy(&tx_page->TxOption.PN[0],(unsigned char *)PN, 8);
            *PN = (*PN) + 1;
            break;

        case WIFI_AES:
            memcpy(&tx_page->TxOption.PN[0],(unsigned char *)PN,8);
            *PN = (*PN) + 1;
            break;

        case WIFI_WPI:
            memcpy(&tx_page->TxOption.PN[0], (unsigned char *)PN, MAX_PN_LEN);
            if (is_bc) {
                hal_wpi_pn_self_plus(PN);

            } else {
                hal_wpi_pn_self_plus_plus(PN);
            }
            break;

        default:
            break;
    }
    PN_UNLOCK();
}


void hal_tx_desc_build(struct hi_agg_tx_desc* HiTxDesc,
    struct hi_tx_priv_hdr* HI_TxPriv,struct hi_tx_desc *pTxDPape)
{
    struct hw_tx_vector_bits * TxVector_Bit= (struct hw_tx_vector_bits * )&pTxDPape->TxVector;
    unsigned short bw  = ( HiTxDesc->FLAG & WIFI_CHANNEL_BW_MASK)>>WIFI_CHANNEL_BW_OFFSET;
    struct wifi_qos_frame *wh = NULL;
    unsigned char is_bc;

    //pr_debug("%s(%d) bw 0x%x\n", __func__, __LINE__, bw);

    tmp_dot11_preamble_type = (DESC_RATE==WIFI_11B_1M) ? PREAMBLE_LONG:wifi_conf_mib.dot11PreambleType;

    TxVector_Bit->tv_reserved0 = 0;
    TxVector_Bit->tv_fw_duration_valid = 0;
    TxVector_Bit->tv_rty_flag = 0;

    if ((DESC_FLAG & (WIFI_IS_BLOCKACK | WIFI_IS_NOACK)) == (WIFI_IS_BLOCKACK | WIFI_IS_NOACK )) {
        TxVector_Bit->tv_ack_policy =BLOCKACK;
    }else if (DESC_FLAG& WIFI_IS_NOACK) {
        TxVector_Bit->tv_ack_policy = NOACK;
    }else {
        TxVector_Bit->tv_ack_policy =  ACK;
    }

    wh = ( struct wifi_qos_frame *)pTxDPape->txdata;
    if ((IS_VHT_RATE( DESC_RATE)) && !(HiTxDesc->FLAG & WIFI_IS_AGGR)) {
        TxVector_Bit->tv_single_ampdu = ((pTxDPape->MPDUBufFlag & (HW_LAST_AGG_FLAG|HW_FIRST_AGG_FLAG))
                                                        == (HW_LAST_AGG_FLAG|HW_FIRST_AGG_FLAG)); //:1,
        if (TxVector_Bit->tv_single_ampdu) {
            TxVector_Bit->tv_ack_policy =  ACK;
            wh->i_qos[0] &= ~( 0x3<<5 );  //change to Normal ack
        }

    } else {
        TxVector_Bit->tv_single_ampdu = 0;
    }

    TxVector_Bit->tv_wnet_vif_id = DESC_VID;
    TxVector_Bit->tv_ack_to_en = ((DESC_FLAG & WIFI_IS_NOACK) == 0);
    TxVector_Bit->tv_ack_timeout = Hal_TxDescriptor_GetAckTimeout(DESC_RATE,DESC_PREMBLETYPE);

    TxVector_Bit->tv_ht.htbit.tv_format = hw_rate2tv_format(DESC_RATE);
    TxVector_Bit->tv_ht.htbit.tv_nonht_mod  = (hal_tx_desc_nonht_mode(DESC_RATE,bw));
    TxVector_Bit->tv_ht.htbit.tv_tx_pwr = hal_tx_desc_get_power(DESC_RATE);
    TxVector_Bit->tv_ht.htbit.tv_tx_rate = DESC_RATE;
    TxVector_Bit->tv_ht.htbit.tv_Channel_BW = bw&0x3;
    TxVector_Bit->tv_ht.htbit.tv_preamble_type = DESC_PREMBLETYPE;
    TxVector_Bit->tv_ht.htbit.tv_GI_type = (!!(DESC_FLAG & WIFI_IS_SHORTGI));
    TxVector_Bit->tv_ht.htbit.tv_antenna_set = 0;

    TxVector_Bit->tv_L_length = hal_tx_desc_get_len(DESC_RATE,DESC_LEN,
        DESC_IS_SHORTGI,DESC_BANDWIDTH,DESC_IS_GREEN);
    TxVector_Bit->tv_sounding = 0;

    if (IS_VHT_RATE( DESC_RATE) && !(HiTxDesc->FLAG & WIFI_IS_AGGR)) {
        TxVector_Bit->tv_length = DESC_LEN + 4; //add ampdu delimiter length
        TxVector_Bit->tv_ampdu_flag = 1;
    }
    else {
        TxVector_Bit->tv_length = DESC_LEN;
        TxVector_Bit->tv_ampdu_flag = DESC_IS_AMPDU;
    }

#ifdef TX_STBC_TEST
    TxVector_Bit->tv_STBC = 1;
#else
    TxVector_Bit->tv_STBC = 0;
#endif
#if defined (HAL_FPGA_VER)
    TxVector_Bit->tv_FEC_coding = (hw_rate2tv_format(DESC_RATE) != 0) ?
        (!!(HiTxDesc->FLAG2&TX_DESCRIPTER_ENABLE_TX_LDPC)) : 0;
#elif defined (HAL_SIM_VER)
    if (TxVector_Bit->tv_ht.htbit.tv_format == NON_HT) {
        /*non-ht bcc coding , 11g/11b rate*/
        TxVector_Bit->tv_FEC_coding = 0;
    }else{
        /*ht ldpc coding , 11n/11ac rate*/
#ifdef FEC_CODE_TEST
        TxVector_Bit->tv_FEC_coding = 1;
#else
        TxVector_Bit->tv_FEC_coding = hal_priv->fec_coding & BIT(0);
#endif
    }
    PRINT("tv_FEC_coding = %d\n", TxVector_Bit->tv_FEC_coding);
#endif
    TxVector_Bit->tv_num_exten_ss = 0;
    TxVector_Bit->tv_num_tx= 1;
    TxVector_Bit->tv_expansion_mat_type= 1;
    TxVector_Bit->tv_no_sig_extn= ((DESC_RATE>WIFI_11B_11M)?0:1);

    TxVector_Bit->tv_txop_time = 0;
    /* all ampdu is burst ,firmware current implement. */
    TxVector_Bit->tv_burst_en = ((!!(DESC_FLAG &WIFI_IS_BURST)) | (!!(WIFI_IS_BLOCKACK&DESC_FLAG )));
    TxVector_Bit->tv_SequenceNum=HI_TxPriv->Seqnum<<SEQUENCE_CONTROL_SEQUENCE_SHIFT;
    TxVector_Bit->tv_FrameControl = HI_TxPriv->FrameControl;

    /*
        tv_fw_duration_valid:Indicate duration from fw is valid,
        hw won't rewrite it again, only PS-POLL frame set to 1 currently
    */
    if ( hal_mac_frame_type(TxVector_Bit->tv_FrameControl,
        FRAME_TYPE_CONTROL | FRAME_SUBTYPE_PS_POLL))
    {
        TxVector_Bit->tv_fw_duration_valid = 1;
    }
    else
    {
        TxVector_Bit->tv_fw_duration_valid = 0;
    }

    TxVector_Bit->tv_sq_valid= 0;
    TxVector_Bit->tv_fc_valid = 1;
    TxVector_Bit->tv_du_valid= 1;
    TxVector_Bit->tv_beacon_flag =!! (HiTxDesc->FLAG2 & TX_DESCRIPTER_BEACON);

    TxVector_Bit->tv_tid_num =0;
    TxVector_Bit->tv_control_wrapper=0;
    TxVector_Bit->tv_llength_hw_calc = 0;
    TxVector_Bit->tv_htc_modify=0;

    //mac3 add
    TxVector_Bit->tv_txcsum_enbale = 1;

    if (HiTxDesc->FLAG & WIFI_IS_PMF)
        TxVector_Bit->tv_encrypted_disable = 1;
    else
        TxVector_Bit->tv_encrypted_disable = 0;

    TxVector_Bit->tv_tkip_mic_enable = 1;
    TxVector_Bit->tv_dyn_bw_nonht = 0;
    TxVector_Bit->tv_txop_ps_not = 0;
    TxVector_Bit->tv_reserved2 = 0; //:10;
    TxVector_Bit->tv_usr_postion = 0;
    TxVector_Bit->tv_group_id = 0;
    TxVector_Bit->tv_partial_id = 0;
    TxVector_Bit->tv_num_users = 0;
    TxVector_Bit->tv_num_sts = 0;
    TxVector_Bit->tv_ch_bw_nonht = SW_CBW20;
    TxVector_Bit->tv_ht.htbit.tv_cfend_flag = 0;
    TxVector_Bit->tv_ht.htbit.tv_bar_flag = 0;
    TxVector_Bit->tv_ht.htbit.tv_beamformed = 0;
    TxVector_Bit->tv_ht.htbit.tv_rts_flag = 0;
    TxVector_Bit->tv_ht.htbit.tv_cts_flag = 0;

    if (HiTxDesc->FLAG&WIFI_IS_DYNAMIC_BW) {
        TxVector_Bit->tv_dyn_bw_nonht = 1;
        TxVector_Bit->tv_ht.htbit.tv_check_cca_bw = 1;
        TxVector_Bit->tv_ch_bw_nonht = bw & 0x3;
    }

    is_bc = ((HiTxDesc->FLAG & WIFI_IS_Group) ? 1 : 0);
    pTxDPape->TxOption.is_bc = is_bc;
    pTxDPape->TxOption.key_type = HiTxDesc->EncryptType;
    pTxDPape->TxOption.KeyIdex = HiTxDesc->KeyId;
    pTxDPape->TxPriv.AggrLen = HiTxDesc->AGGR_len;
    pTxDPape->TxPriv.aggr_page_num = HiTxDesc->aggr_page_num;
    //if have immediate rsp ,exchange txtime=FrameTxTime +SIFS +ACKTxTime
    if (HiTxDesc->FLAG &WIFI_IS_BLOCKACK)
    {
        if (!(HiTxDesc->FLAG &WIFI_IS_NOACK)) // implicit  inmediate block ack, bar-ba
            pTxDPape->TxPriv.FrameExchDur = Hal_TxDescriptor_GetTxTimeBA(HiTxDesc);
        else  //normal block ack
            pTxDPape->TxPriv.FrameExchDur = Hal_TxDescriptor_GetTxTime(HiTxDesc);
    }
    else
    {
        if (!(HiTxDesc->FLAG &WIFI_IS_NOACK))//normal ack
            pTxDPape->TxPriv.FrameExchDur = Hal_TxDescriptor_GetCtsTime(HiTxDesc);
        else // no ack
            pTxDPape->TxPriv.FrameExchDur =  Hal_TxDescriptor_GetTxTime(HiTxDesc);
    }
    //PRINT("pTxDPape->TxPriv.FrameExchDur %x \n",pTxDPape->TxPriv.FrameExchDur);

    pTxDPape->TxPriv.Flag = HiTxDesc->FLAG;
    pTxDPape->TxPriv.Flag2= HiTxDesc->FLAG2;
    pTxDPape->TxPriv.TID= HiTxDesc->TID;
    pTxDPape->TxPriv.hostcallbackid = 0;
    pTxDPape->TxPriv.StaId= HiTxDesc->StaId;
    pTxDPape->TxPriv.TxCurrentRate= HiTxDesc->CurrentRate;
    pTxDPape->TxPriv.TxRate[0]= HiTxDesc->CurrentRate;
    pTxDPape->TxPriv.TxRate[1]= HiTxDesc->TxTryRate1;
    pTxDPape->TxPriv.TxRate[2]= HiTxDesc->TxTryRate2;
    pTxDPape->TxPriv.TxRate[3]= HiTxDesc->TxTryRate3;
    pTxDPape->TxPriv.TxRetry[0]= HiTxDesc->TxTryNum0;
    pTxDPape->TxPriv.TxRetry[1]= HiTxDesc->TxTryNum1+pTxDPape->TxPriv.TxRetry[0];
    pTxDPape->TxPriv.TxRetry[2]= HiTxDesc->TxTryNum2+pTxDPape->TxPriv.TxRetry[1];
    pTxDPape->TxPriv.TxRetry[3]= HiTxDesc->TxTryNum3+pTxDPape->TxPriv.TxRetry[2];
    pTxDPape->TxPriv.hostcallbackid =0;

    pTxDPape->TxPriv.AggrNum= HiTxDesc->MpduNum;
    pTxDPape->TxPriv.AggrLen= HiTxDesc->AGGR_len;
    pTxDPape->TxPriv.SN= HI_TxPriv->Seqnum;
    pTxDPape->TxPriv.txretrynum=0;
    pTxDPape->TxPriv.txsretrynum=0;
    pTxDPape->TxPriv.pTxInfo=0;
    pTxDPape->TxPriv.vid =HiTxDesc->vid;
    if (HiTxDesc->FLAG2 & TX_DESCRIPTER_P2P_PS_NOA_TRIGRSP) {
        pTxDPape->TxPriv.HiP2pNoaCountNow = HiTxDesc->HiP2pNoaCountNow;
    }

 //if frame is broadcast, initialize TxPriv.txstatus to TX_DESCRIPTOR_STATUS_SUCCESS
    if ((pTxDPape->TxPriv.Flag & WIFI_IS_Group) == WIFI_IS_Group) {
        pTxDPape->TxPriv.txstatus = TX_DESCRIPTOR_STATUS_SUCCESS;
    }
}

void hal_tx_desc_build_sub(struct hi_tx_priv_hdr* HI_TxPriv,
    struct hi_tx_desc *pTxDPape, struct hi_tx_desc *pFirstTxDPape)
{
    unsigned char is_bc;
    ASSERT(HI_TxPriv);
    ASSERT(pTxDPape);
    ASSERT(pFirstTxDPape);

    //duplicate the first tx-vector for the others subframes here:
    //it works as  hal_tx_desc_build(..)
    memcpy(pTxDPape,pFirstTxDPape,HI_TXDESC_DATAOFFSET);

    pTxDPape->TxPriv.PageNum = hal_calc_mpdu_page(HI_TxPriv->MPDULEN);
    pTxDPape->MPDUBufFlag = 0;
    pTxDPape->MPDUBufFlag = HW_FIRST_MPDUBUF_FLAG|HW_LAST_MPDUBUF_FLAG;
    pTxDPape->MPDUBufFlag |= HW_MPDU_LEN_SET(HI_TxPriv->MPDULEN);
    pTxDPape->MPDUBufFlag |= HW_BUFFER_LEN_SET(HI_TxPriv->Delimiter);
    if ((HI_TxPriv->Flag & WIFI_MORE_AGG)==0) {
        pTxDPape->MPDUBufFlag |= HW_LAST_AGG_FLAG;
    }
    if (pTxDPape->TxPriv.PageNum == 1) {
        pTxDPape->MPDUBufFlag |= HW_LAST_MPDUBUF_FLAG;
    }

    is_bc = ((pFirstTxDPape->TxPriv.Flag & WIFI_IS_Group) ? 1 : 0);
    pTxDPape->TxOption.is_bc = is_bc;
    pTxDPape->TxOption.key_type = HI_TxPriv->EncryptType;
    pTxDPape->TxOption.KeyIdex = pFirstTxDPape->TxOption.KeyIdex;

}


unsigned int max_send_packet_len(unsigned char rate,unsigned char bw, unsigned char short_gi, unsigned char streams)
{
    unsigned int preamble_time = 0;
    unsigned int mcs = 0;
    unsigned int max_symbol_number = 0;
    unsigned int max_data_field_tx_time = 0;

    if( IS_HT_RATE(rate) )
    {
        preamble_time = Hal_TxDescriptor_HT_GetPreamble(rate, 0);
    }
    else if ( IS_VHT_RATE(rate) )
    {
        preamble_time = Hal_TxDescriptor_VHT_GetPreamble(rate, 0);
    }
    else
    {
        ERROR_DEBUG_OUT("rate error, rate=0x%x\n", rate);
        return 0;
    }

     max_data_field_tx_time = 4095 -  preamble_time;
    // pr_debug("preamble_time =%d\n", preamble_time);
     if( !short_gi )
     {
        max_symbol_number = max_data_field_tx_time>>2;   /*long gi every symbol time is 4 us*/
     }
     else
     {
        max_symbol_number = max_data_field_tx_time*10/36; /*short gi  every ofdm symbol time is 3.6us*/
     }

    //pr_debug("max_symbol_number =%d\n", max_symbol_number);
    mcs = GET_MCS(rate);
    if ((mcs > 9) || (bw > 2))
    {
        ERROR_DEBUG_OUT("mcs or bw error, rate=0x%x, bw=%d\n", rate, bw);
        return 0;
    }

    if ((mcs == 9)  && (bw == 0))
    {
        /*vht 20M bandwidth have not mcs9 rate*/
        mcs = 8;
    }

    if (IS_HT_RATE(rate))
    {
        if (bw > BW_40) {
            bw = BW_40;
        }

        if (mcs > 7) {
            mcs = 7;
        }

        //pr_debug("mcs =%d, bw=%d, NDBPS=%d\n", mcs, bw,ht_bits_per_symbol[mcs][bw]);
        return ht_bits_per_symbol[mcs][bw]*max_symbol_number >> 3;
    }
    else
    {
        //pr_debug("mcs =%d, bw=%d, NDBPS=%d\n", mcs, bw,vht_bits_per_symbol[mcs][bw]);
        return vht_bits_per_symbol[mcs][bw]*max_symbol_number>>3;
    }
}

char hw_rate2tv_format(unsigned char rate)
{
    if (IS_VHT_RATE(rate))
    {
        return VHT;
    }
    else if (IS_HT_RATE(rate))
    {
        return HT_MF;
    }
    else
    {
        return NON_HT;
    }
}

#ifdef HAL_SIM_VER
#ifdef FW_NAME
}
#endif
#endif
