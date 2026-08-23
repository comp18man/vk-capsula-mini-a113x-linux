#include "wifi_mac_com.h"
#include "wifi_hal_cmd.h"

#define ARR_SIZE_OF(arr) (sizeof(arr)/sizeof(arr[0]))
#define CHAN_2_FREQ_5G(chan) ((chan) * 5 + 5000)
#define CHAN_2_FREQ_2G(chan) ((chan) * 5 + 2407)
#define CHAN_2_FREQ_14(chan) ((chan) * 5 + 2414)

#define IS_5G_BAND1(chan) ((chan) >= 36 && (chan) <= 48)
#define IS_5G_BAND2(chan) ((chan) >= 52 && (chan) <= 64)
#define IS_5G_BAND3(chan) ((chan) >= 100 && (chan) <= 144)
#define IS_5G_BAND4(chan) ((chan) >= 149 && (chan) <= 177)

#define DFS_5G_B1  0x00000001
#define DFS_5G_B2  0x00000002
#define DFS_5G_B3  0x00000004
#define DFS_5G_B4  0x00000008
#define PASSIVE_2G_12_14 0x00000010

struct class_chan_set global_chan_set [256] =
{
    {81, 20, 13, {
                     {CHAN_2_FREQ_2G(1), WIFINET_CHAN_2GHZ, 1, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(2), WIFINET_CHAN_2GHZ, 2, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(3), WIFINET_CHAN_2GHZ, 3, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(4), WIFINET_CHAN_2GHZ, 4, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(5), WIFINET_CHAN_2GHZ, 5, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(6), WIFINET_CHAN_2GHZ, 6, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(7), WIFINET_CHAN_2GHZ, 7, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(8), WIFINET_CHAN_2GHZ, 8, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(9), WIFINET_CHAN_2GHZ, 9, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(10), WIFINET_CHAN_2GHZ, 10, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(11), WIFINET_CHAN_2GHZ, 11, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(12), WIFINET_CHAN_2GHZ, 12, 4, 2, WIFINET_BWC_WIDTH20, 81},
                     {CHAN_2_FREQ_2G(13), WIFINET_CHAN_2GHZ, 13, 4, 2, WIFINET_BWC_WIDTH20, 81}
                 }
    }, //class 81
    {82, 20, 1, {
                    {CHAN_2_FREQ_14(14), WIFINET_CHAN_2GHZ, 14, 4, 2, WIFINET_BWC_WIDTH20, 82}
                }
    },//82
    {83, 40, 9, {
                    {CHAN_2_FREQ_2G(1 + 2), WIFINET_CHAN_2GHZ, 1, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(2 + 2), WIFINET_CHAN_2GHZ, 2, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(3 + 2), WIFINET_CHAN_2GHZ, 3, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(4 + 2), WIFINET_CHAN_2GHZ, 4, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(5 + 2), WIFINET_CHAN_2GHZ, 5, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(6 + 2), WIFINET_CHAN_2GHZ, 6, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(7 + 2), WIFINET_CHAN_2GHZ, 7, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(8 + 2), WIFINET_CHAN_2GHZ, 8, 4, 2, WIFINET_BWC_WIDTH40, 83},
                    {CHAN_2_FREQ_2G(9 + 2), WIFINET_CHAN_2GHZ, 9, 4, 2, WIFINET_BWC_WIDTH40, 83}
                }
    },//83
    {84, 40, 9, {
                    {CHAN_2_FREQ_2G(5 - 2), WIFINET_CHAN_2GHZ, 5, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(6 - 2), WIFINET_CHAN_2GHZ, 6, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(7 - 2), WIFINET_CHAN_2GHZ, 7, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(8 - 2), WIFINET_CHAN_2GHZ, 8, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(9 - 2), WIFINET_CHAN_2GHZ, 9, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(10 - 2), WIFINET_CHAN_2GHZ, 10, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(11 - 2), WIFINET_CHAN_2GHZ, 11, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(12 - 2), WIFINET_CHAN_2GHZ, 12, 4, 2, WIFINET_BWC_WIDTH40, 84},
                    {CHAN_2_FREQ_2G(13 - 2), WIFINET_CHAN_2GHZ, 13, 4, 2, WIFINET_BWC_WIDTH40, 84}
                }
    },//84
    {112, 20, 3, {
                     {CHAN_2_FREQ_5G(8), WIFINET_CHAN_5GHZ, 8, 4, 2, WIFINET_BWC_WIDTH20, 112},
                     {CHAN_2_FREQ_5G(12), WIFINET_CHAN_5GHZ, 12, 4, 2, WIFINET_BWC_WIDTH20, 112},
                     {CHAN_2_FREQ_5G(16) , WIFINET_CHAN_5GHZ, 16, 4, 2, WIFINET_BWC_WIDTH20, 112}
                 }
    },//112
    {115, 20, 4, {
                     {CHAN_2_FREQ_5G(36), WIFINET_CHAN_5GHZ, 36, 4, 2, WIFINET_BWC_WIDTH20, 115},
                     {CHAN_2_FREQ_5G(40), WIFINET_CHAN_5GHZ, 40, 4, 2, WIFINET_BWC_WIDTH20, 115},
                     {CHAN_2_FREQ_5G(44), WIFINET_CHAN_5GHZ, 44, 4, 2, WIFINET_BWC_WIDTH20, 115},
                     {CHAN_2_FREQ_5G(48), WIFINET_CHAN_5GHZ, 48, 4, 2, WIFINET_BWC_WIDTH20, 115}
                 }
    },//115
    {116, 40, 2, {
                     {CHAN_2_FREQ_5G(36 + 2), WIFINET_CHAN_5GHZ, 36, 4, 2, WIFINET_BWC_WIDTH40, 116},
                     {CHAN_2_FREQ_5G(44 + 2), WIFINET_CHAN_5GHZ, 44, 4, 2, WIFINET_BWC_WIDTH40, 116}
                 }
    },//116
    {117, 40, 2, {
                     {CHAN_2_FREQ_5G(40 - 2), WIFINET_CHAN_5GHZ, 40, 4, 2, WIFINET_BWC_WIDTH40, 117},
                     {CHAN_2_FREQ_5G(48 - 2), WIFINET_CHAN_5GHZ, 48, 4, 2, WIFINET_BWC_WIDTH40, 117}
                 }
    },//117
    {118, 20, 4, {
                     {CHAN_2_FREQ_5G(52), WIFINET_CHAN_5GHZ, 52, 4, 2, WIFINET_BWC_WIDTH20, 118},
                     {CHAN_2_FREQ_5G(56), WIFINET_CHAN_5GHZ, 56, 4, 2, WIFINET_BWC_WIDTH20, 118},
                     {CHAN_2_FREQ_5G(60), WIFINET_CHAN_5GHZ, 60, 4, 2, WIFINET_BWC_WIDTH20, 118},
                     {CHAN_2_FREQ_5G(64), WIFINET_CHAN_5GHZ, 64, 4, 2, WIFINET_BWC_WIDTH20, 118}
                 }
    },//118
    {119, 40, 2, {
                     {CHAN_2_FREQ_5G(52 + 2), WIFINET_CHAN_5GHZ, 52, 4, 2, WIFINET_BWC_WIDTH40, 119},
                     {CHAN_2_FREQ_5G(60 + 2), WIFINET_CHAN_5GHZ, 60, 4, 2, WIFINET_BWC_WIDTH40, 119}
                 }
    },//119
    {120, 40, 2, {
                     {CHAN_2_FREQ_5G(56-2), WIFINET_CHAN_5GHZ, 56, 4, 2, WIFINET_BWC_WIDTH40, 120},
                     {CHAN_2_FREQ_5G(64-2), WIFINET_CHAN_5GHZ, 64, 4, 2, WIFINET_BWC_WIDTH40, 120}
                 }
    },//120
    {121, 20, 12, {
                      {CHAN_2_FREQ_5G(100), WIFINET_CHAN_5GHZ, 100, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(104), WIFINET_CHAN_5GHZ, 104, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(108), WIFINET_CHAN_5GHZ, 108, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(112), WIFINET_CHAN_5GHZ, 112, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(116), WIFINET_CHAN_5GHZ, 116, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(120), WIFINET_CHAN_5GHZ, 120, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(124), WIFINET_CHAN_5GHZ, 124, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(128), WIFINET_CHAN_5GHZ, 128, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(132), WIFINET_CHAN_5GHZ, 132, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(136), WIFINET_CHAN_5GHZ, 136, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(140), WIFINET_CHAN_5GHZ, 140, 4, 2, WIFINET_BWC_WIDTH20, 121},
                      {CHAN_2_FREQ_5G(144), WIFINET_CHAN_5GHZ, 144, 4, 2, WIFINET_BWC_WIDTH20, 121}
                  }
     },//121
     {122, 40, 6, {
                      {CHAN_2_FREQ_5G(100+2), WIFINET_CHAN_5GHZ, 100, 4, 2, WIFINET_BWC_WIDTH40, 122},
                      {CHAN_2_FREQ_5G(108+2), WIFINET_CHAN_5GHZ, 108, 4, 2, WIFINET_BWC_WIDTH40, 122},
                      {CHAN_2_FREQ_5G(116+2), WIFINET_CHAN_5GHZ, 116, 4, 2, WIFINET_BWC_WIDTH40, 122},
                      {CHAN_2_FREQ_5G(124+2), WIFINET_CHAN_5GHZ, 124, 4, 2, WIFINET_BWC_WIDTH40, 122},
                      {CHAN_2_FREQ_5G(132+2), WIFINET_CHAN_5GHZ, 132, 4, 2, WIFINET_BWC_WIDTH40, 122},
                      {CHAN_2_FREQ_5G(140+2), WIFINET_CHAN_5GHZ, 140, 4, 2, WIFINET_BWC_WIDTH40, 122}
                  }
    },//122
    {123, 40, 6, {
                     {CHAN_2_FREQ_5G(104-2), WIFINET_CHAN_5GHZ, 104, 4, 2, WIFINET_BWC_WIDTH40, 123},
                     {CHAN_2_FREQ_5G(112-2), WIFINET_CHAN_5GHZ, 112, 4, 2, WIFINET_BWC_WIDTH40, 123},
                     {CHAN_2_FREQ_5G(120-2), WIFINET_CHAN_5GHZ, 120, 4, 2, WIFINET_BWC_WIDTH40, 123},
                     {CHAN_2_FREQ_5G(128-2), WIFINET_CHAN_5GHZ, 128, 4, 2, WIFINET_BWC_WIDTH40, 123},
                     {CHAN_2_FREQ_5G(136-2), WIFINET_CHAN_5GHZ, 136, 4, 2, WIFINET_BWC_WIDTH40, 123},
                     {CHAN_2_FREQ_5G(144-2), WIFINET_CHAN_5GHZ, 144, 4, 2, WIFINET_BWC_WIDTH40, 123}
                 }
    },//123
    {124, 20, 4, {
                     {CHAN_2_FREQ_5G(149), WIFINET_CHAN_5GHZ, 149, 4, 2, WIFINET_BWC_WIDTH20, 124},
                     {CHAN_2_FREQ_5G(153), WIFINET_CHAN_5GHZ, 153, 4, 2, WIFINET_BWC_WIDTH20, 124},
                     {CHAN_2_FREQ_5G(157), WIFINET_CHAN_5GHZ, 157, 4, 2, WIFINET_BWC_WIDTH20, 124},
                     {CHAN_2_FREQ_5G(161), WIFINET_CHAN_5GHZ, 161, 4, 2, WIFINET_BWC_WIDTH20, 124}
                 }
    },//124
    {125, 20, 5, {
                     {CHAN_2_FREQ_5G(149), WIFINET_CHAN_5GHZ, 149, 4, 2, WIFINET_BWC_WIDTH20, 125},
                     {CHAN_2_FREQ_5G(153), WIFINET_CHAN_5GHZ, 153, 4, 2, WIFINET_BWC_WIDTH20, 125},
                     {CHAN_2_FREQ_5G(157), WIFINET_CHAN_5GHZ, 157, 4, 2, WIFINET_BWC_WIDTH20, 125},
                     {CHAN_2_FREQ_5G(161), WIFINET_CHAN_5GHZ, 161, 4, 2, WIFINET_BWC_WIDTH20, 125},
                     {CHAN_2_FREQ_5G(165), WIFINET_CHAN_5GHZ, 165, 4, 2, WIFINET_BWC_WIDTH20, 125}
                 }
    },//125
    {126, 40, 2, {
                     {CHAN_2_FREQ_5G(149+2), WIFINET_CHAN_5GHZ, 149, 4, 2, WIFINET_BWC_WIDTH40, 126},
                     {CHAN_2_FREQ_5G(157+2), WIFINET_CHAN_5GHZ, 157, 4, 2, WIFINET_BWC_WIDTH40, 126}
                 }
    },//126
    {127, 40, 2, {
                     {CHAN_2_FREQ_5G(153-2), WIFINET_CHAN_5GHZ, 153, 4, 2, WIFINET_BWC_WIDTH40, 127},
                     {CHAN_2_FREQ_5G(161-2), WIFINET_CHAN_5GHZ, 161, 4, 2, WIFINET_BWC_WIDTH40, 127}
                 }
    },//127
    {128, 80, 24, {
                      {CHAN_2_FREQ_5G(42), WIFINET_CHAN_5GHZ, 36, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(42), WIFINET_CHAN_5GHZ, 40, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(42), WIFINET_CHAN_5GHZ, 44, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(42), WIFINET_CHAN_5GHZ, 48, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(58), WIFINET_CHAN_5GHZ, 52, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(58), WIFINET_CHAN_5GHZ, 56, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(58), WIFINET_CHAN_5GHZ, 60, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(58), WIFINET_CHAN_5GHZ, 64, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(106), WIFINET_CHAN_5GHZ, 100, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(106), WIFINET_CHAN_5GHZ, 104, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(106), WIFINET_CHAN_5GHZ, 108, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(106), WIFINET_CHAN_5GHZ, 112, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(122), WIFINET_CHAN_5GHZ, 116, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(122), WIFINET_CHAN_5GHZ, 120, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(122), WIFINET_CHAN_5GHZ, 124, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(122), WIFINET_CHAN_5GHZ, 128, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(138), WIFINET_CHAN_5GHZ, 132, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(138), WIFINET_CHAN_5GHZ, 136, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(138), WIFINET_CHAN_5GHZ, 140, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(138), WIFINET_CHAN_5GHZ, 144, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(155), WIFINET_CHAN_5GHZ, 149, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(155), WIFINET_CHAN_5GHZ, 153, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(155), WIFINET_CHAN_5GHZ, 157, 4, 2, WIFINET_BWC_WIDTH80, 128},
                      {CHAN_2_FREQ_5G(155), WIFINET_CHAN_5GHZ, 161, 4, 2, WIFINET_BWC_WIDTH80, 128}
                  }
    }//128
};

struct country_na_freq_set country_na_freq_plan_list [] =
{
    //0x00 :CN
    {1, {
            {128, {CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(138), 0}}
        }
    },
    //0x01 :US
    {3, {
            {81, {CHAN_2_FREQ_2G(12), CHAN_2_FREQ_2G(13), 0}},
            {83, {CHAN_2_FREQ_2G(8 + 2), CHAN_2_FREQ_2G(9 + 2), 0}},
            {84, {CHAN_2_FREQ_2G(12 - 2), CHAN_2_FREQ_2G(13 - 2), 0}}
        }
    },
    //0x02 :E.R/JP/FR
    {4, {
            {121, {CHAN_2_FREQ_5G(144), 0}},
            {122, {CHAN_2_FREQ_5G(140 + 2), 0}},
            {123, {CHAN_2_FREQ_5G(144 - 2), 0}},
            {128, {CHAN_2_FREQ_5G(138), CHAN_2_FREQ_5G(155), 0}}
        }
    },
    //0x03 :ISR
    {1, {
            {128, {CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(138), CHAN_2_FREQ_5G(155), 0}}
        }
    },
    //0x04 :MEXI
    {4, {
            {81, {CHAN_2_FREQ_2G(12), CHAN_2_FREQ_2G(13), 0}},
            {83, {CHAN_2_FREQ_2G(8 + 2), CHAN_2_FREQ_2G(9 + 2), 0}},
            {84, {CHAN_2_FREQ_2G(12 - 2), CHAN_2_FREQ_2G(13 - 2), 0}},
            {128, {CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122),CHAN_2_FREQ_5G(138), 0}}
         }
    },
    //0x05 :CA
    {7, {
            {81, {CHAN_2_FREQ_2G(12), CHAN_2_FREQ_2G(13), 0}},
            {83, {CHAN_2_FREQ_2G(8 + 2), CHAN_2_FREQ_2G(9 + 2), 0}},
            {84, {CHAN_2_FREQ_2G(12 - 2), CHAN_2_FREQ_2G(13 - 2), 0}},
            {121, {CHAN_2_FREQ_5G(120), CHAN_2_FREQ_5G(124), CHAN_2_FREQ_5G(128), CHAN_2_FREQ_5G(144), 0}},
            {122, {CHAN_2_FREQ_5G(116 + 2), CHAN_2_FREQ_5G(124 + 2), CHAN_2_FREQ_5G(140 + 2), 0}},
            {123, {CHAN_2_FREQ_5G(120 - 2), CHAN_2_FREQ_5G(128 - 2), CHAN_2_FREQ_5G(144 - 2), 0}},
            {128, {CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(138), 0}}
        }
    },
    //0x06 :IN
    {4, {
            {121, {CHAN_2_FREQ_5G(100), CHAN_2_FREQ_5G(104), CHAN_2_FREQ_5G(108), CHAN_2_FREQ_5G(112), CHAN_2_FREQ_5G(116)}},
            {122, {CHAN_2_FREQ_5G(100+2), CHAN_2_FREQ_5G(108+2), CHAN_2_FREQ_5G(116+2), 0}},
            {123, {CHAN_2_FREQ_5G(104-2), CHAN_2_FREQ_5G(112-2), CHAN_2_FREQ_5G(120-2), 0}},
            {128, {CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122), 0}}
        }
    },
    //0x07 :AU
    {4, {
            {121, {CHAN_2_FREQ_5G(120), CHAN_2_FREQ_5G(124), CHAN_2_FREQ_5G(128), CHAN_2_FREQ_5G(144), 0}},
            {122, {CHAN_2_FREQ_5G(116 + 2), CHAN_2_FREQ_5G(124 + 2), CHAN_2_FREQ_5G(140 + 2), 0}},
            {123, {CHAN_2_FREQ_5G(120 - 2), CHAN_2_FREQ_5G(128 - 2), CHAN_2_FREQ_5G(144 - 2), 0}},
            {128, {CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(138), 0}}
        }
    },
    //0x08 :BR
    {4, {
            {121, {CHAN_2_FREQ_5G(144), 0}},
            {122, {CHAN_2_FREQ_5G(140 + 2), 0}},
            {123, {CHAN_2_FREQ_5G(144 - 2), 0}},
            {128, {CHAN_2_FREQ_5G(138), 0}}
        }
    },
    //0x09 :RU
    {4, {
            {121, {CHAN_2_FREQ_5G(100), CHAN_2_FREQ_5G(104), CHAN_2_FREQ_5G(108), CHAN_2_FREQ_5G(112), CHAN_2_FREQ_5G(116), CHAN_2_FREQ_5G(120),CHAN_2_FREQ_5G(124), CHAN_2_FREQ_5G(128)}},
            {122, {CHAN_2_FREQ_5G(100+2), CHAN_2_FREQ_5G(108+2), CHAN_2_FREQ_5G(116+2), CHAN_2_FREQ_5G(124+2),0}},
            {123, {CHAN_2_FREQ_5G(104-2), CHAN_2_FREQ_5G(112-2), CHAN_2_FREQ_5G(120-2), CHAN_2_FREQ_5G(128-2),0}},
            {128, {CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122), 0}}
        }
    },
    //0x0a :ID
    {1, {
            {128, {CHAN_2_FREQ_5G(42),CHAN_2_FREQ_5G(58),CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(138), 0}}
        }
    },
    //0x0b :TW
    {10, {
            {81, {CHAN_2_FREQ_2G(12), CHAN_2_FREQ_2G(13), 0}},
            {83, {CHAN_2_FREQ_2G(8 + 2), CHAN_2_FREQ_2G(9 + 2), 0}},
            {84, {CHAN_2_FREQ_2G(12 - 2), CHAN_2_FREQ_2G(13 - 2), 0}},
            {118, {CHAN_2_FREQ_5G(52),0}},
            {119, {CHAN_2_FREQ_5G(52+2),0}},
            {120, {CHAN_2_FREQ_5G(56-2),0}},
            {121, {CHAN_2_FREQ_5G(120), CHAN_2_FREQ_5G(124), CHAN_2_FREQ_5G(128), CHAN_2_FREQ_5G(144), 0}},
            {122, {CHAN_2_FREQ_5G(116+2), CHAN_2_FREQ_5G(124+2),CHAN_2_FREQ_5G(140 + 2),0}},
            {123, {CHAN_2_FREQ_5G(120-2), CHAN_2_FREQ_5G(128-2),CHAN_2_FREQ_5G(144-2),0}},
            {128, {CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(138), 0}}
        }
    },
    //0x0c :AE
    {1, {
            {121, {CHAN_2_FREQ_5G(144), 0}}
        }
    },
    //0x0d :AR
    {4, {
            {121, {CHAN_2_FREQ_5G(120), CHAN_2_FREQ_5G(124), CHAN_2_FREQ_5G(128), 0}},
            {122, {CHAN_2_FREQ_5G(116 + 2), CHAN_2_FREQ_5G(124 + 2), 0}},
            {123, {CHAN_2_FREQ_5G(120 - 2), CHAN_2_FREQ_5G(128 - 2), 0}},
            {128, {CHAN_2_FREQ_5G(122), 0}}
        }
    },
    //0x0e :KZ
    {4, {
            {121, {CHAN_2_FREQ_5G(100), CHAN_2_FREQ_5G(104), CHAN_2_FREQ_5G(108), CHAN_2_FREQ_5G(112), CHAN_2_FREQ_5G(116), CHAN_2_FREQ_5G(120), CHAN_2_FREQ_5G(124), CHAN_2_FREQ_5G(128)}},
            {122, {CHAN_2_FREQ_5G(100+2), CHAN_2_FREQ_5G(108+2), CHAN_2_FREQ_5G(116+2), CHAN_2_FREQ_5G(124+2), 0}},
            {123, {CHAN_2_FREQ_5G(104-2), CHAN_2_FREQ_5G(112-2), CHAN_2_FREQ_5G(120-2), CHAN_2_FREQ_5G(128-2), 0}},
            {128, {CHAN_2_FREQ_5G(106), CHAN_2_FREQ_5G(122), CHAN_2_FREQ_5G(155), 0}}
        }
    },

};

struct tx_power_plan tx_power_plan_list[] = {
    {57, {0, 0, 0, 0}, {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, //TX_POWER_DEFAULT
    {57, {0, 0, 0, 0}, {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, //TX_POWER_CE
    {57, {0, 0, 0, 0}, {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, //TX_POWER_FCC
    {57, {0, 0, 0, 0}, {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, //TX_POWER_ARIB
    {57, {0, 0, 0, 0}, {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, //TX_POWER_SRRC
    {57, {0, 0, 0, 0}, {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, //TX_POWER_ANATEL
};

struct country_chan_plan country_chan_plan_list[] = {
    /* 0x00 */ {17, {81,82,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128}, 0xff, DFS_5G_B2|DFS_5G_B3|PASSIVE_2G_12_14, TX_POWER_DEFAULT}, //Worldwide
    /* 0x01 */ {13, {81,83,84,115,116,117,118,119,120,125,126,127,128,0,0,0,0},       0x00, DFS_5G_B2,           TX_POWER_SRRC}, //China
    /* 0x02 */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x01, DFS_5G_B2|DFS_5G_B3, TX_POWER_FCC}, //United States of America
    /* 0x03 */ {14, {81,83,84,115,116,117,118,119,120,121,122,123,125,128,0,0,0},     0x02, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //Europe
    /* 0x04 */ {17, {81,83,84,115,116,117,118,119,120,121,122,123,124,125,126,127,128}, 0x01, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //France
    /* 0x05 */ {14, {81,82,83,84,115,116,117,118,119,120,121,122,123,128,0,0,0},      0x02, DFS_5G_B2|DFS_5G_B3, TX_POWER_ARIB}, //Japan
    /* 0x06 */ {10, {81,83,84,115,116,117,118,119,120,128,0,0,0,0,0,0,0},             0x03, DFS_5G_B2,           TX_POWER_CE}, //Israel
    /* 0x07 */ {13, {81,83,84,115,116,117,118,119,120,125,126,127,128,0,0,0,0},       0x04, DFS_5G_B2,           TX_POWER_FCC}, //Mexico
    /* 0x08 */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x05, DFS_5G_B2|DFS_5G_B3, TX_POWER_DEFAULT}, //Canada
    /* 0x09 */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x06, DFS_5G_B2|DFS_5G_B3, TX_POWER_DEFAULT}, //India
    /* 0x0A */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x07, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //Australia
    /* 0x0B */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0xff, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //NewZealand
    /* 0x0C */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x08, DFS_5G_B2|DFS_5G_B3, TX_POWER_ANATEL}, //Brazil
    /* 0x0D */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x09, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //RU
    /* 0x0E */ {13, {81,83,84,115,116,117,118,119,120,125,126,127,128,0,0,0,0},       0x00, 0,                   TX_POWER_CE}, //Indonesia
    /* 0x0f */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0xff, DFS_5G_B1|DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //South Korea
    /* 0x10 */ {4,  {81,83,84,125,0,0,0,0,0,0,0,0,0,0,0,0,0},                         0xff, 0,                   TX_POWER_FCC}, //Peru
    /* 0x11 */ {13, {81,83,84,118,119,120,121,122,123,125,126,127,128,0,0,0,0},       0x0b, DFS_5G_B2|DFS_5G_B3, TX_POWER_FCC}, //Taiwan
    /* 0x12 */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0xff, DFS_5G_B1|DFS_5G_B2, TX_POWER_CE}, //Thailand
    /* 0x13 */ {6,  {81,83,84,115,118,121,0,0,0,0,0,0,0,0,0,0,0},                     0x0c, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //United Arab Emirates
    /* 0x14 */ {15, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x0d, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //Argentina
    /* 0x15 */ {6,  {81,83,84,115,118,125,0,0,0,0,0,0,0,0,0,0,0},                     0xff, DFS_5G_B2,           TX_POWER_CE}, //Bahrain
    /* 0x16 */ {3,  {81,83,84,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                           0xff, 0,                   TX_POWER_CE}, //Costa Rica
    /* 0x17 */ {5,  {81,83,84,115,118,0,0,0,0,0,0,0,0,0,0,0,0},                       0xff, DFS_5G_B2,           TX_POWER_CE}, //Armenia
    /* 0x18 */ {4,  {81,83,84,115,0,0,0,0,0,0,0,0,0,0,0,0,0},                         0xff, 0,                   TX_POWER_CE}, //Jordan
    /* 0x19 */ {7,  {81,83,84,124,126,127,128,0,0,0,0,0,0,0,0,0,0},                   0x0a, 0,                   TX_POWER_CE}, //North Korea
    /* 0x1A */ {7,  {81,83,84,115,118,121,125,0,0,0,0,0,0,0,0,0,0},                   0x0c, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //Sri Lanka
    /* 0x1B */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0x08, 0,                   TX_POWER_CE}, //Ukraine
    /* 0x1C */ {16, {81,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128,0}, 0xff, DFS_5G_B2|DFS_5G_B3|DFS_5G_B4, TX_POWER_CE}, //Great Britain
    /* 0x1D */ {13, {81,83,84,115,116,117,118,119,120,121,122,123,128,0,0,0,0},       0x0e, DFS_5G_B2|DFS_5G_B3, TX_POWER_CE}, //Kazakhstan
    /* 0x1E */ {17, {81,82,83,84,115,116,117,118,119,120,121,122,123,125,126,127,128},0xff, 0,                   TX_POWER_DEFAULT}, //Global

};

struct country_chan_mapping  country_chan_mapping_list[] = {
    {"WW", 0x00}, /* Worldwide */
    {"GO", 0x1E}, /* Global */
    {"AD", 0x04}, /* Andorra */
    {"AE", 0x13}, /* United Arab Emirates */
    {"AF", 0x00}, /* Afghanistan */
    {"AG", 0x02}, /* Antigua & Barbuda */
    {"AI", 0x04}, /* Anguilla(UK) */
    {"AL", 0x04}, /* Albania */
    {"AM", 0x17}, /* Armenia */
    {"AN", 0x02}, /* Netherlands Antilles */
    {"AO", 0x00}, /* Angola */
    {"AQ", 0x04}, /* Antarctica */
    {"AR", 0x14}, /* Argentina */
    {"AS", 0x02}, /* American Samoa */
    {"AT", 0x04}, /* Austria */
    {"AU", 0x0A}, /* Australia */
    {"AW", 0x02}, /* Aruba */
    {"AZ", 0x06}, /* Azerbaijan */
    {"BA", 0x00}, /* Bosnia & Herzegovina */
    {"BB", 0x02}, /* Barbados */
    {"BD", 0x04}, /* Bangladesh */
    {"BE", 0x04}, /* Belgium */
    {"BF", 0x04}, /* Burkina Faso */
    {"BG", 0x06}, /* Bulgaria */
    {"BH", 0x15}, /* Bahrain */
    {"BI", 0x04}, /* Burundi */
    {"BJ", 0x04}, /* Benin */
    {"BM", 0x02}, /* Bermuda (UK) */
    {"BN", 0x0E}, /* Brunei */
    {"BO", 0x0E}, /* Bolivia */
    {"BR", 0x0C}, /* Brazil */
    {"BS", 0x02}, /* Bahamas */
    {"BT", 0x04}, /* Bhutan */
    {"BV", 0x04}, /* Bouvet Island (Norway) */
    {"BW", 0x00}, /* Botswana */
    {"BY", 0x04}, /* Belarus */
    {"BZ", 0x0E}, /* Belize */
    {"CA", 0x08}, /* Canada */
    {"CC", 0x04}, /* Cocos (Keeling) Islands (Australia) */
    {"CD", 0x04}, /* Congo, Republic of the */
    {"CF", 0x04}, /* Central African Republic */
    {"CG", 0x04}, /* Congo, Democratic Republic of the. Zaire */
    {"CH", 0x04}, /* Switzerland */
    {"CI", 0x00}, /* Cote d'Ivoire */
    {"CK", 0x04}, /* Cook Islands */
    {"CL", 0x01}, /* Chile */
    {"CM", 0x04}, /* Cameroon */
    {"CN", 0x01}, /* China */
    {"CO", 0x07}, /* Colombia */
    {"CR", 0x16}, /* Costa Rica */
    {"CV", 0x04}, /* Cape Verde */
    {"CX", 0x0A}, /* Christmas Island (Australia) */
    {"CY", 0x04}, /* Cyprus */
    {"CZ", 0x04}, /* Czech Republic */
    {"DE", 0x04}, /* Germany */
    {"DJ", 0x04}, /* Djibouti */
    {"DK", 0x04}, /* Denmark */
    {"DM", 0x02}, /* Dominica */
    {"DO", 0x07}, /* Dominican Republic */
    {"DZ", 0x03}, /* Algeria */
    {"EC", 0x0C}, /* Ecuador */
    {"EE", 0x04}, /* Estonia */
    {"EG", 0x17}, /* Egypt */
    {"EH", 0x00}, /* Western Sahara */
    {"ER", 0x04}, /* Eritrea */
    {"ES", 0x04}, /* Spain, Canary Islands, Ceuta, Melilla */
    {"ET", 0x04}, /* Ethiopia */
    {"EU", 0x03}, /* Europe */
    {"FI", 0x04}, /* Finland */
    {"FJ", 0x02}, /* Fiji */
    {"FK", 0x04}, /* Falkland Islands (Islas Malvinas) (UK) */
    {"FM", 0x02}, /* Micronesia, Federated States of (USA) */
    {"FO", 0x04}, /* Faroe Islands (Denmark) */
    {"FR", 0x04}, /* France */
    {"GA", 0x04}, /* Gabon */
    {"GB", 0x1C}, /* Great Britain (United Kingdom; England) */
    {"GD", 0x02}, /* Grenada */
    {"GE", 0x04}, /* Georgia */
    {"GF", 0x04}, /* French Guiana */
    {"GG", 0x04}, /* Guernsey (UK) */
    {"GH", 0x04}, /* Ghana */
    {"GI", 0x04}, /* Gibraltar (UK) */
    {"GL", 0x04}, /* Greenland (Denmark) */
    {"GM", 0x04}, /* Gambia */
    {"GN", 0x04}, /* Guinea */
    {"GP", 0x04}, /* Guadeloupe (France) */
    {"GQ", 0x04}, /* Equatorial Guinea */
    {"GR", 0x04}, /* Greece */
    {"GS", 0x04}, /* South Georgia and the Sandwich Islands (UK) */
    {"GT", 0x07}, /* Guatemala */
    {"GU", 0x02}, /* Guam (USA) */
    {"GW", 0x04}, /* Guinea-Bissau */
    {"GY", 0x00}, /* Guyana */
    {"HK", 0x0C}, /* Hong Kong */
    {"HM", 0x0A}, /* Heard and McDonald Islands (Australia) */
    {"HN", 0x0C}, /* Honduras */
    {"HR", 0x04}, /* Croatia */
    {"HT", 0x02}, /* Haiti */
    {"HU", 0x04}, /* Hungary */
    {"ID", 0x0e}, /* Indonesia */
    {"IE", 0x04}, /* Ireland */
    {"IL", 0x06}, /* Israel */
    {"IM", 0x04}, /* Isle of Man (UK) */
    {"IN", 0x09}, /* India */
    {"IO", 0x04}, /* British Indian Ocean Territory (UK) */
    {"IQ", 0x01}, /* Iraq */
    {"IR", 0x06}, /* Iran */
    {"IS", 0x04}, /* Iceland */
    {"IT", 0x04}, /* Italy */
    {"JE", 0x04}, /* Jersey (UK) */
    {"JM", 0x00}, /* Jamaica */
    {"JO", 0x18}, /* Jordan */
    {"JP", 0x05}, /* Japan- Telec */
    {"KE", 0x00}, /* Kenya */
    {"KG", 0x04}, /* Kyrgyzstan */
    {"KH", 0x04}, /* Cambodia */
    {"KI", 0x04}, /* Kiribati */
    {"KM", 0x04}, /* Comoros */
    {"KN", 0x02}, /* Saint Kitts and Nevis */
    {"KP", 0x19}, /* North Korea */
    {"KR", 0x0f}, /* South Korea */
    {"KW", 0x16}, /* Kuwait */
    {"KY", 0x02}, /* Cayman Islands (UK) */
    {"KZ", 0x1D}, /* Kazakhstan */
    {"LA", 0x04}, /* Laos */
    {"LB", 0x10}, /* Lebanon */
    {"LC", 0x02}, /* Saint Lucia */
    {"LI", 0x04}, /* Liechtenstein */
    {"LK", 0x1A}, /* Sri Lanka */
    {"LR", 0x04}, /* Liberia */
    {"LS", 0x04}, /* Lesotho */
    {"LT", 0x04}, /* Lithuania */
    {"LU", 0x04}, /* Luxembourg */
    {"LV", 0x04}, /* Latvia */
    {"LY", 0x04}, /* Libya */
    {"MA", 0x16}, /* Morocco */
    {"MC", 0x06}, /* Monaco */
    {"MD", 0x04}, /* Moldova */
    {"ME", 0x04}, /* Montenegro */
    {"MF", 0x02}, /* Saint Martin */
    {"MG", 0x04}, /* Madagascar */
    {"MH", 0x02}, /* Marshall Islands (USA) */
    {"MK", 0x13}, /* Republic of Macedonia (FYROM) */
    {"ML", 0x04}, /* Mali */
    {"MM", 0x04}, /* Burma (Myanmar) */
    {"MN", 0x04}, /* Mongolia */
    {"MO", 0x01}, /* Macau */
    {"MP", 0x02}, /* Northern Mariana Islands (USA) */
    {"MQ", 0x04}, /* Martinique (France) */
    {"MR", 0x04}, /* Mauritania */
    {"MS", 0x04}, /* Montserrat (UK) */
    {"MT", 0x04}, /* Malta */
    {"MU", 0x04}, /* Mauritius */
    {"MV", 0x00}, /* Maldives */
    {"MW", 0x04}, /* Malawi */
    {"MX", 0x07}, /* Mexico */
    {"MY", 0x01}, /* Malaysia */
    {"MZ", 0x04}, /* Mozambique */
    {"NA", 0x04}, /* Namibia */
    {"NC", 0x04}, /* New Caledonia */
    {"NE", 0x04}, /* Niger */
    {"NF", 0x0A}, /* Norfolk Island (Australia) */
    {"NG", 0x0C}, /* Nigeria */
    {"NI", 0x02}, /* Nicaragua */
    {"NL", 0x04}, /* Netherlands */
    {"NO", 0x04}, /* Norway */
    {"NP", 0x01}, /* Nepal */
    {"NR", 0x04}, /* Nauru */
    {"NU", 0x0B}, /* Niue */
    {"NZ", 0x0B}, /* New Zealand */
    {"OM", 0x15}, /* Oman */
    {"PA", 0x07}, /* Panama */
    {"PE", 0x10}, /* Peru */
    {"PF", 0x04}, /* French Polynesia (France) */
    {"PG", 0x00}, /* Papua New Guinea */
    {"PH", 0x0C}, /* Philippines */
    {"PK", 0x19}, /* Pakistan */
    {"PL", 0x04}, /* Poland */
    {"PM", 0x04}, /* Saint Pierre and Miquelon (France) */
    {"PR", 0x07}, /* Puerto Rico */
    {"PT", 0x04}, /* Portugal */
    {"PW", 0x02}, /* Palau */
    {"PY", 0x02}, /* Paraguay */
    {"QA", 0x0B}, /* Qatar */
    {"RE", 0x04}, /* Reunion (France) */
    {"RO", 0x04}, /* Romania */
    {"RS", 0x04}, /* Serbia, Kosovo */
    {"RU", 0x0D}, /* Russia(fac/gost), Kaliningrad */
    {"RW", 0x04}, /* Rwanda */
    {"SA", 0x01}, /* Saudi Arabia */
    {"SB", 0x04}, /* Solomon Islands */
    {"SC", 0x02}, /* Seychelles */
    {"SE", 0x04}, /* Sweden */
    {"SG", 0x0C}, /* Singapore */
    {"SH", 0x04}, /* Saint Helena (UK) */
    {"SI", 0x04}, /* Slovenia */
    {"SJ", 0x04}, /* Svalbard (Norway) */
    {"SK", 0x04}, /* Slovakia */
    {"SL", 0x04}, /* Sierra Leone */
    {"SM", 0x04}, /* San Marino */
    {"SN", 0x04}, /* Senegal */
    {"SO", 0x04}, /* Somalia */
    {"SR", 0x00}, /* Suriname */
    {"ST", 0x02}, /* Sao Tome and Principe */
    {"SV", 0x16}, /* El Salvador */
    {"SX", 0x02}, /* Sint Marteen */
    {"SY", 0x16}, /* Syria */
    {"SZ", 0x04}, /* Swaziland */
    {"TC", 0x04}, /* Turks and Caicos Islands (UK) */
    {"TD", 0x04}, /* Chad */
    {"TF", 0x04}, /* French Southern and Antarctic Lands (FR Southern Territories) */
    {"TG", 0x04}, /* Togo */
    {"TH", 0x12}, /* Thailand */
    {"TJ", 0x04}, /* Tajikistan */
    {"TK", 0x0A}, /* Tokelau */
    {"TM", 0x04}, /* Turkmenistan */
    {"TN", 0x17}, /* Tunisia */
    {"TO", 0x04}, /* Tonga */
    {"TR", 0x04}, /* Turkey, Northern Cyprus */
    {"TT", 0x0C}, /* Trinidad & Tobago */
    {"TV", 0x00}, /* Tuvalu */
    {"TW", 0x11}, /* Taiwan */
    {"TZ", 0x04}, /* Tanzania */
    {"UA", 0x1B}, /* Ukraine */
    {"UG", 0x04}, /* Uganda */
    {"US", 0x02}, /* United States of America (USA) */
    {"UY", 0x0A}, /* Uruguay */
    {"UZ", 0x06}, /* Uzbekistan */
    {"VA", 0x04}, /* Holy See (Vatican City) */
    {"VC", 0x02}, /* Saint Vincent and the Grenadines */
    {"VE", 0x0E}, /* Venezuela */
    {"VG", 0x02}, /* British Virgin Islands (UK) */
    {"VI", 0x02}, /* United States Virgin Islands (USA) */
    {"VN", 0x0C}, /* Vietnam */
    {"VU", 0x04}, /* Vanuatu */
    {"WF", 0x04}, /* Wallis and Futuna (France) */
    {"WS", 0x02}, /* Samoa */
    {"YE", 0x16}, /* Yemen */
    {"YT", 0x04}, /* Mayotte (France) */
    {"ZA", 0x0C}, /* South Africa */
    {"ZM", 0x04}, /* Zambia */
    {"ZW", 0x16}  /* Zimbabwe */
};

struct country_chan_mapping  southamerica_country[] = {
    {"AR", 0x14}, /* Argentina */
    {"BO", 0x0E}, /* Bolivia */
    {"BR", 0x0C}, /* Brazil */
    {"CL", 0x01}, /* Chile */
    {"CO", 0x07}, /* Colombia */
    {"EC", 0x0C}, /* Ecuador */
    {"FK", 0x04}, /* Falkland Islands (Islas Malvinas) (UK) */
    {"GF", 0x04}, /* French Guiana */
    {"GY", 0x00}, /* Guyana */
    {"PY", 0x02}, /* Paraguay */
    {"PE", 0x10}, /* Peru */
    {"SR", 0x00}, /* Suriname */
    {"UY", 0x0A}, /* Uruguay */
    {"VE", 0x0E}, /* Venezuela */
};

unsigned char if_southamerica_country(unsigned char *countrycode) {
    unsigned char index;

    for (index = 0; index < (sizeof(southamerica_country) / sizeof(southamerica_country[0])); index++) {
        if ((southamerica_country[index].country[0] == countrycode[0])
            && (southamerica_country[index].country[1] == countrycode[1])) {
            return index + 1;
        }
    }

    return 0;
}

static int  wifi_mac_get_pos(struct wifi_channel in[], int a, int b)
{
    int i = a;
    int j = b;
    struct wifi_channel x ;

    if (!in || a > b)
    {
        return 0;
    }

    memcpy(&x,  &in[i], sizeof(struct wifi_channel));
    while (i < j)
    {
        while (i < j && in[j].chan_pri_num >= x.chan_pri_num)
        {
            j--;
        }
        if ( i < j)
        {
            memcpy(&in[i], &in[j], sizeof(struct wifi_channel));
            i++;
        }
        while (i < j && in[i].chan_pri_num < x.chan_pri_num)
        {
            i++;
        }
        if (i < j)
        {
            memcpy(&in[j], &in[i], sizeof(struct wifi_channel));
            j--;
        }
    }

    memcpy(&in[i], &x, sizeof(x));
    return i;
}

static void wifi_mac_chan_order(struct wifi_channel in[], int a, int b)
{
    int pos = 0;
    if (a < b)
    {
        pos = wifi_mac_get_pos(in, a, b);
        wifi_mac_chan_order(in, a, pos-1);
        wifi_mac_chan_order(in, pos+1, b);
    }
}

static int wifi_mac_check_if_na_freq(int country_code, int global_class, int freq)
{
    struct country_na_freq_set *na_freq_set = NULL;
    struct country_na_freq_info *na_freq_info = NULL;
    int i = 0;

    int plan_index = country_chan_mapping_list[country_code].chplan;
    int na_freq_plan_index = country_chan_plan_list[plan_index].class_na_freq_plan;

    /*get na_freq index*/
    if (na_freq_plan_index == 0xff) {
        return 0;
    }

    na_freq_set = &country_na_freq_plan_list[na_freq_plan_index];

    /*find global class*/
    if (na_freq_set) {
        for (i = 0; i < na_freq_set->na_freq_class_num; i++) {
           if (na_freq_set->na_freq_info[i].g_operating_class == global_class) {
               na_freq_info = &na_freq_set->na_freq_info[i];
           }
        }
    }

    /*find freq*/
    if (na_freq_info) {
        for (i = 0; i < MAX_NA_FREQ_NUM; i++) {
           if (na_freq_info->na_freq[i] == freq) {
               return 1;
           }

           if (na_freq_info->na_freq[i] == 0) {
               break;
           }
       }
    }

    return 0;
}

static void  wifi_mac_select_chan_from_global(int country_code, int sub_set[], int num,struct wifi_mac *wifimac)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int channel_num = 0;
    short hash_20M[200] = {0};
    short hash_40M[200][2] = {{-1}};
    short hash_80M[200] = {-1};
    int chn_idx_20M = 0;
    int chn_idx_40M = 0;
    int chn_idx_80M = 0;

    if ((!sub_set) || (!wifimac)) {
        ERROR_DEBUG_OUT("input or memory err!\n");
        return;
    }

    memset(hash_40M, -1, sizeof(hash_40M));
    for (i = 0; i < num; i++) {
        for (j = 0; j < ARR_SIZE_OF(global_chan_set); j++) {
            if (sub_set[i] == global_chan_set[j].opt_idx) {
                for (k = 0; k < global_chan_set[j].sub_num ; k++) {
                    if (wifi_mac_check_if_na_freq(country_code, global_chan_set[j].opt_idx, global_chan_set[j].chan_sub_set[k].chan_cfreq1) == 1) {
                        continue;
                    }

                    if (global_chan_set[j].chan_sub_set[k].chan_bw == WIFINET_BWC_WIDTH20) {
                        chn_idx_20M = global_chan_set[j].chan_sub_set[k].chan_pri_num;

                        if (hash_20M[chn_idx_20M] == 0) {
                            memcpy(wifimac->wm_channels + channel_num++, &global_chan_set[j].chan_sub_set[k], sizeof(struct wifi_channel));
                            hash_20M[chn_idx_20M]++;
                        }

                    } else if (global_chan_set[j].chan_sub_set[k].chan_bw == WIFINET_BWC_WIDTH40) {
                        chn_idx_40M = global_chan_set[j].chan_sub_set[k].chan_pri_num;

                        if (hash_40M[chn_idx_40M][0] == -1) {
                            if (hash_40M[chn_idx_40M][1] == -1) {
                                memcpy(wifimac->wm_channels + channel_num++, &global_chan_set[j].chan_sub_set[k], sizeof(struct wifi_channel));
                                hash_40M[chn_idx_40M][1] = channel_num - 1;

                            } else if (hash_40M[chn_idx_40M][1] != -1) {
                                if (wifimac->wm_channels[hash_40M[chn_idx_40M][1]].chan_cfreq1 != global_chan_set[j].chan_sub_set[k].chan_cfreq1) {
                                    memcpy(wifimac->wm_channels + channel_num++, &global_chan_set[j].chan_sub_set[k], sizeof(struct wifi_channel));
                                    hash_40M[chn_idx_40M][0]++;//40M finish
                                }
                            }
                        }

                    } else if (global_chan_set[j].chan_sub_set[k].chan_bw == WIFINET_BWC_WIDTH80) {
                        chn_idx_80M = global_chan_set[j].chan_sub_set[k].chan_pri_num;

                        if (hash_80M[chn_idx_80M] == 0) {
                            memcpy(wifimac->wm_channels + channel_num++, &global_chan_set[j].chan_sub_set[k], sizeof(struct wifi_channel));
                            hash_80M[chn_idx_80M]++;
                        }
                    }
                }
            }
        }
    }

    wifimac->wm_nchans = channel_num;
    wifi_mac_chan_order(wifimac->wm_channels, 0, wifimac->wm_nchans - 1);

#if 0
    pr_debug("wifimac->wm_nchans:%d\n", wifimac->wm_nchans);
    for (i = 0; i < wifimac->wm_nchans; ++i) {
        //pr_debug("chan_cfreq1:%d\n", wifimac->wm_channels[i].chan_cfreq1);
        //pr_debug("chan_bw:%d\n", wifimac->wm_channels[i].chan_bw);
        pr_debug("chan_pri_num:%d\n", wifimac->wm_channels[i].chan_pri_num);
    }
    pr_debug("i is %d\n", i);
#endif
}

static void wifi_mac_mark_dfs_channel_ex(int country_code, struct wifi_mac *wifimac, int chan_num)
{
    int i = 0;
    int chan_pri_num = 0;
    int plan_index = country_chan_mapping_list[country_code].chplan;
    unsigned char dfs_chan_flag = country_chan_plan_list[plan_index].dfs_chan_flag;

    /* if chan mun==0 mark all dfs channel*/
    if (chan_num == 0) {
        for (i = 0; i < wifimac->wm_nchans; i++) {
            chan_pri_num = wifimac->wm_channels[i].chan_pri_num;
            if (((IS_5G_BAND1(chan_pri_num)) && (dfs_chan_flag & DFS_5G_B1))
                || ((IS_5G_BAND2(chan_pri_num)) && (dfs_chan_flag & DFS_5G_B2))
                || ((IS_5G_BAND3(chan_pri_num)) && (dfs_chan_flag & DFS_5G_B3))
                || ((IS_5G_BAND4(chan_pri_num)) && (dfs_chan_flag & DFS_5G_B4))
                || ((chan_pri_num >= 12 && chan_pri_num <= 14) && (dfs_chan_flag & PASSIVE_2G_12_14))) {
//                pr_debug("%s mark channel %d \n", __func__, chan_pri_num);
                wifimac->wm_channels[i].chan_flags |= WIFINET_CHAN_DFS;
            }
        }
    } else {
        for (i = 0; i <  wifimac->wm_nchans; i++) {
            if (wifimac->wm_channels[i].chan_pri_num == chan_num) {
//                pr_debug("%s mark channel %d \n", __func__,wifimac->wm_channels[i].chan_pri_num);
                wifimac->wm_channels[i].chan_flags |= WIFINET_CHAN_DFS;
            }
        }
    }
}

static void wifi_mac_unmark_dfs_channel_ex(int country_code, struct wifi_mac *wifimac, int chan_num)
{
    int i = 0;

    for (i = 0; i < wifimac->wm_nchans; i++) {
        if (( chan_num == 0 && (wifimac->wm_channels[i].chan_flags & WIFINET_CHAN_DFS) )
            || (chan_num != 0 && (wifimac->wm_channels[i].chan_pri_num == chan_num)) ){
//            pr_debug("%s UNmark channel %d \n", __func__,wifimac->wm_channels[i].chan_pri_num);
            wifimac->wm_channels[i].chan_flags &= ~WIFINET_CHAN_DFS;
        }
    }
}

int wifi_mac_if_dfs_channel(struct wifi_mac *wifimac, int chan_num)
{
    int i = 0;

    WIFI_CHANNEL_LOCK(wifimac);

    for (i = 0; i <  wifimac->wm_nchans; i++) {
        if (wifimac->wm_channels[i].chan_pri_num == chan_num && (wifimac->wm_channels[i].chan_flags & WIFINET_CHAN_DFS)) {
//            pr_debug("%s is dfs channel %d \n", __func__,wifimac->wm_channels[i].chan_pri_num);
            WIFI_CHANNEL_UNLOCK(wifimac);
            return 1;
        }
    }

    WIFI_CHANNEL_UNLOCK(wifimac);
    return 0;
}

void wifi_mac_unmark_dfs_channel(struct wlan_net_vif *wnet_vif, int chan_num)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private* drv_priv =  wifimac->drv_priv;
    int country_code = drv_priv->drv_config.cfg_countrycode;

    WIFI_CHANNEL_LOCK(wifimac);
    wifi_mac_unmark_dfs_channel_ex(country_code, wifimac, chan_num);
    WIFI_CHANNEL_UNLOCK (wifimac);
}

void wifi_mac_mark_dfs_channel(struct wlan_net_vif *wnet_vif, int chan_num)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private* drv_priv =  wifimac->drv_priv;
    int country_code = drv_priv->drv_config.cfg_countrycode;

    WIFI_CHANNEL_LOCK(wifimac);
    wifi_mac_mark_dfs_channel_ex(country_code, wifimac, chan_num);
    WIFI_CHANNEL_UNLOCK(wifimac);

    if ((wnet_vif->vm_opmode == WIFINET_M_STA) && (wifimac->wm_nrunning == 1)) {
        if (wifi_mac_if_dfs_channel(wifimac, wnet_vif->vm_curchan->chan_pri_num) == 1) {
            pr_debug("current channel %d is dfs channel \n", wnet_vif->vm_curchan->chan_pri_num);
            wnet_vif->vm_chan_roaming_scan_flag = 0;
            wifi_mac_top_sm(wnet_vif, WIFINET_S_SCAN, 0);
        }
    }
}

void  wifi_mac_update_chan_list_by_country(int country_code, int support_opt[], int support_num, struct wifi_mac *wifimac )
{
    if ((!support_opt)||(support_num==0))
    {
        ERROR_DEBUG_OUT("input is NULL!!!\n");
    }

    memset(wifimac->wm_channels, 0, sizeof(struct wifi_channel) * (WIFINET_CHAN_MAX * 2 + 1));
    wifimac->wm_nchans = 0;

    wifi_mac_select_chan_from_global(country_code, support_opt, support_num,wifimac);
    wifi_mac_mark_dfs_channel_ex(country_code, wifimac, 0);
}

unsigned char wifi_mac_chan_num_avail (struct wifi_mac *wifimac, unsigned char channum)
{
    if (isset(wifimac->wm_chan_avail, channum))
    {
        return true;
    }
    else
    {
        return false;
    }
}

unsigned int wifi_mac_Mhz2ieee(unsigned int freq, unsigned int flags)
{
    if (flags & WIFINET_CHAN_2GHZ)
    {
        if (freq == 2484)
            return 14;

        if (freq < 2484)
            return (freq - 2407) / 5;
        else
            return 15 + ((freq - 2512) / 20);
    }
    else if (flags & WIFINET_CHAN_5GHZ)
    {
        return (freq - 5000) / 5;
    }
    else
    {
        if (freq == 2484)
            return 14;

        if (freq < 2484)
            return (freq - 2407) / 5;

        if (freq < 5000)
            return 15 + ((freq - 2512) / 20);

        return (freq - 5000) / 5;
    }
}

/*check struct WIFINET_S_CHANNEL *c and return primary channel num */
unsigned int wifi_mac_chan2ieee(struct wifi_mac *wifimac,
const struct wifi_channel *c)
{
    if (c == NULL)
    {
        ERROR_DEBUG_OUT("amlwifi,invalid channel (NULL)\n");
        return WIFINET_CHAN_INVALUE;
    }
    return c->chan_pri_num;
}

unsigned int wifi_mac_Ieee2mhz(unsigned int chan, unsigned int flags)
{
    if (flags & WIFINET_CHAN_2GHZ)
    {
        if (chan == 14)
            return 2484;

        if (chan < 14)
            return 2407 + chan*5;
        else
            return 2512 + ((chan-15)*20);
    }
    else if (flags & WIFINET_CHAN_5GHZ)
    {
        return 5000 + (chan*5);
    }
    else
    {
        if (chan == 14)
            return 2484;

        if (chan < 14)
            return 2407 + chan*5;

        if (chan < 27)
            return 2512 + ((chan-15)*20);

        return 5000 + (chan*5);
    }
}

struct wifi_channel * wifi_mac_find_chan(struct wifi_mac *wifimac, int chan, int bw, int center_chan)
{
    struct wifi_channel *c = NULL;
    int i = 0;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++) {
        c = &wifimac->wm_channels[i];
        if ((c->chan_pri_num== chan) && (c->chan_bw == bw) && (c->chan_cfreq1 == wifi_mac_Ieee2mhz(center_chan, 0))) {
            DPRINTF(AML_DEBUG_BWC, "%s(%d): find a chan=%d, bw=%d and cntr chan=%d.\n", __func__, __LINE__, chan, bw ,center_chan);
            WIFI_CHANNEL_UNLOCK(wifimac);
            return c;
        }
    }
    WIFI_CHANNEL_UNLOCK(wifimac);
    return NULL;
}

int wifi_mac_find_80M_channel_center_chan(int chan)
{
    struct wifi_channel *c = NULL;
    int i = 0;
    int j = 0;

    for (i = 0; i < ARR_SIZE_OF(global_chan_set); i++) {
        if (global_chan_set[i].bw == 80) {
            for (j = 0; j < global_chan_set[i].sub_num; j++) {
                c = &global_chan_set[i].chan_sub_set[j];
                if (c->chan_pri_num == chan) {
                    return wifi_mac_Mhz2ieee(c->chan_cfreq1,c->chan_flags);
                }
            }
        }
    }

    return 0;
}

void wifi_mac_update_country_chan_list(struct wifi_mac *wifimac)
{
    struct wifi_channel *c;
    int i;

    memset(wifimac->wm_chan_avail, 0, sizeof(wifimac->wm_chan_avail));
    wifi_mac_get_country(wifimac, &wifimac->wm_country);

    for (i = 0; i < wifimac->wm_nchans; i++)
    {
        c = &wifimac->wm_channels[i];
        setbit(wifimac->wm_chan_avail, c->chan_pri_num);
    }
}

int wifi_mac_recv_bss_intol_channelCheck(struct wifi_mac *wifimac, struct wifi_mac_ie_intolerant_report *intol_ie)
{
    int i, j;
    unsigned char intol_chan  = 0;
    unsigned char *chan_list = &intol_ie->chan_list[0];

    if (intol_ie->elem_len <= 1 || intol_ie->elem_len >254)
        return false;

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < intol_ie->elem_len-1; i++)
    {
        intol_chan = *chan_list++;
        for (j = 0; j < wifimac->wm_nchans; j++)
        {
            if (intol_chan == wifimac->wm_channels[j].chan_pri_num)
            {
                WIFI_CHANNEL_UNLOCK(wifimac);
                return true;
            }
        }
    }
    WIFI_CHANNEL_UNLOCK(wifimac);

    return false;
}

struct wifi_channel * wifi_mac_scan_sta_get_ap_channel(struct wlan_net_vif *wnet_vif, struct wifi_mac_scan_param *sp)
{
    unsigned char *htinfoie = sp->htinfo;
    unsigned char *htcapie = sp->htcap;
    unsigned char chan = sp->chan;
    struct wifi_mac_ie_htinfo *xhtinfo = (struct wifi_mac_ie_htinfo*)htinfoie;
    struct wifi_mac_ie_htcap *xhtcap =(struct wifi_mac_ie_htcap *)htcapie;
    struct wifi_mac_ie_htcap_cmn *htcap = NULL;
    int htcapval = 0;
    struct wifi_mac_ie_htinfo_cmn *htinfo = NULL;
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_mac_ie_vht_opt *vht_opt_ie =  (struct wifi_mac_ie_vht_opt *)sp->vht_opt;
    int center_chan = 0, bw = 0;
    static struct wifi_channel *apchan =NULL;

    if (xhtinfo != NULL)
    {
        htinfo = &xhtinfo->hi_ie;
    }

    if (xhtcap != NULL) {
        htcap = &xhtcap->hc_ie;
        htcapval = le16toh(htcap->hc_cap);
    }

    DPRINTF(AML_DEBUG_BWC, "%s(%d) %s, chan %d htinfo %p,chwidth %x, htcapval 0x%x \n", __func__, __LINE__,
        ssidie_sprintf(sp->ssid), sp->chan, htinfo, htinfo?htinfo->hi_txchwidth : 0, htcapval);

    if ((htinfo != NULL) && (htinfo->hi_txchwidth == WIFINET_HTINFO_TXWIDTH_2040))
    {
        DPRINTF(AML_DEBUG_BWC, "%s pri_chn %d\n",__func__, htinfo->hi_ctrlchannel);

        switch (htinfo->hi_extchoff)
        {
            case WIFINET_HTINFO_EXTOFFSET_ABOVE:
                DPRINTF(AML_DEBUG_BWC, "%s %d  WIFINET_HTINFO_EXTOFFSET_ABOVE \n", __func__,__LINE__);
                bw = WIFINET_BWC_WIDTH40;
                center_chan = htinfo->hi_ctrlchannel + 2;
                break;

                case WIFINET_HTINFO_EXTOFFSET_BELOW:
                DPRINTF(AML_DEBUG_BWC, "%s %d  WIFINET_HTINFO_EXTOFFSET_BELOW\n", __func__,__LINE__);
                bw = WIFINET_BWC_WIDTH40;
                center_chan = htinfo->hi_ctrlchannel - 2;
                break;

            case WIFINET_HTINFO_EXTOFFSET_NA:
            default:
                DPRINTF(AML_DEBUG_BWC, "%s %d 20M only mode\n",__func__,__LINE__);
                bw = WIFINET_BWC_WIDTH20;
                center_chan = htinfo->hi_ctrlchannel;
                break;
        }
    }
    else
    {
        bw = WIFINET_BWC_WIDTH20;
        center_chan = chan;
    }

    /*check coexistence for 20MHz and 40MHz */
    if ((bw == WIFINET_BWC_WIDTH40) && (xhtcap != NULL) && !(htcapval & WIFINET_HTCAP_SUPPORTCBW40)) {
        bw = WIFINET_BWC_WIDTH20;
        center_chan = chan;
        AML_OUTPUT("htcap and htinfo information conflict\n");
    }

    if((vht_opt_ie != NULL) &&
    (vht_opt_ie->vht_op_chwidth >= WIFINET_VHTOP_CHWIDTH_80))
    {
        center_chan = vht_opt_ie->vht_op_ch_freq_seg1;
        DPRINTF(AML_DEBUG_BWC, "%s center chn %d\n",__func__, center_chan);

        switch(vht_opt_ie->vht_op_chwidth)
        {
            case WIFINET_VHTOP_CHWIDTH_80:
                bw = WIFINET_BWC_WIDTH80;
                break;
            case WIFINET_VHTOP_CHWIDTH_160:
                bw = WIFINET_BWC_WIDTH160;
                break;
            case WIFINET_VHTOP_CHWIDTH_80_80:
                bw = WIFINET_BWC_WIDTH80P80;
                break;
            default:
                bw = WIFINET_BWC_WIDTH80;
                break;
        }
    }

    DPRINTF(1, "%s chan %d, bw %d, center_chan %d \n", __func__, chan, bw, center_chan);
    apchan = wifi_mac_find_chan(wifimac, chan, bw, center_chan);
    if ((apchan == NULL) && (htinfo != NULL)) {
        if ((htinfo->hi_txchwidth == WIFINET_HTINFO_TXWIDTH_2040) && (bw == WIFINET_BWC_WIDTH40)) {
            bw = WIFINET_BWC_WIDTH20;
            center_chan = chan;
            apchan = wifi_mac_find_chan(wifimac, chan, bw, center_chan);
        }
    }
    return apchan;
}

int wifi_mac_chan_attach(struct wifi_mac *wifimac)
{
    struct wifi_channel *c;
    int i;
    KASSERT(0 < wifimac->wm_nchans && wifimac->wm_nchans < WIFINET_CHAN_MAX,
            ("invalid number of channels specified: %u", wifimac->wm_nchans));
    memset(wifimac->wm_chan_avail, 0, sizeof(wifimac->wm_chan_avail));

    WIFI_CHANNEL_LOCK(wifimac);
    for (i = 0; i < wifimac->wm_nchans; i++)
    {
        c = &wifimac->wm_channels[i];
        KASSERT(c->chan_pri_num < WIFINET_CHAN_MAX,
                ("channel with bogus ieee number %u", c->chan_pri_num));
        setbit(wifimac->wm_chan_avail, c->chan_pri_num);
    }
    WIFI_CHANNEL_UNLOCK(wifimac);

    wifimac->wm_curchan = WIFINET_CHAN_ERR;

    return 0;
}

int wifi_mac_chan_overlapping_map_init(struct wifi_mac *wifimac)
{
    unsigned char i;

    //2.4G channel count:14
    for (i = 0; i < 14; ++i) {
        wifimac->chan_overlapping_map[i].chan_index = i + 1;
    }

    //5G channel  34 - 64, count:16
    for (; i < 30; ++i) {
        wifimac->chan_overlapping_map[i].chan_index = 2 * (i - 14) + 34;
    }

    //5G channel  100 - 144 count:23
    for (; i < 53; ++i) {
        wifimac->chan_overlapping_map[i].chan_index = 2 * (i - 30) + 100;
    }

    //5G channel  149 - 165 count:9
    for (; i < 62; ++i) {
        wifimac->chan_overlapping_map[i].chan_index = 2 * (i - 53) + 149;
    }

    return 0;
}

void wifi_mac_chan_setup(void * ieee, unsigned int wMode, int countrycode_ex)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    int *support_ptr = NULL;
    int support_num = 0;
    int support_index = 0;
    int i =0;

    support_index = country_chan_mapping_list[countrycode_ex].chplan;
    support_num = country_chan_plan_list[support_index].support_class_num;
    support_ptr = country_chan_plan_list[support_index].support_class;

    pr_debug("%s(%d) country code 0x%x, support num %d\n", __func__, __LINE__, countrycode_ex, support_num);
    wifi_mac_update_chan_list_by_country(countrycode_ex,support_ptr, support_num, wifimac);

    for(i= 0; i < wifimac->wm_nchans; i++) {
        chan_dbg(&wifimac->wm_channels[i], "chan info", __LINE__);
    }
}

void wifi_mac_ChangeChannel(void * ieee, struct wifi_channel *chan, unsigned char flag, unsigned char vid, unsigned char opmode)
{
    struct wifi_mac *wifimac = NET80211_HANDLE(ieee);
    struct hal_channel hchan;

    if (chan == WIFINET_CHAN_ERR) {
        ERROR_DEBUG_OUT("chan not valid !!\n");
        return;
    }

    wifimac->wm_curchan = chan;
    hchan.channel = chan->chan_cfreq1;
    hchan.cchan_num = wifi_mac_mhz2chan(chan->chan_cfreq1);
    hchan.pchan_num = chan->chan_pri_num;
    hchan.chan_bw = chan->chan_bw;

    KASSERT(hchan.channel != 0,("bogus channel %u\n", hchan.channel));
#ifdef FW_RF_CALIBRATION
    wifimac->drv_priv->drv_ops.drv_hal_tx_frm_pause(wifimac->drv_priv, 1);
#endif
    wifimac->drv_priv->drv_ops.set_channel(wifimac->drv_priv, &hchan, flag, vid, opmode);

    if (!(wifimac->wm_flags & WIFINET_F_SCAN)) {
        DPRINTF(AML_DEBUG_WARNING, "%s: %.3d %d\n", __func__, chan->chan_pri_num, chan->chan_bw);
    }
}

void wifi_mac_set_wnet_vif_chan_ex(SYS_TYPE param1,SYS_TYPE param2, SYS_TYPE param3,
    SYS_TYPE param4,SYS_TYPE param5)
{
    struct wifi_mac *wifimac = (struct wifi_mac * )param1;
    struct wifi_channel *wnet_vif_chan = (struct wifi_channel * )param2;
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)param3;
    unsigned char channel_switch_flag = (unsigned char)param4;

    wifi_mac_ChangeChannel(wifimac, wnet_vif_chan, channel_switch_flag, wnet_vif->wnet_vif_id, wnet_vif->vm_opmode);
}

struct wifi_channel * wifi_mac_get_wm_chan (struct wifi_mac *wifimac)
{
    if (wifimac->wm_curchan != WIFINET_CHAN_ERR)
    {
        DPRINTF(AML_DEBUG_ERROR,  "wm_curchan != WIFINET_CHAN_ERR\n");
        return wifimac->wm_curchan;
    }
    else
    {
        return WIFINET_CHAN_ERR;
    }
}

int wifi_mac_set_wnet_vif_channel(struct wlan_net_vif *wnet_vif,  int chan,
                                 int bw, int center_chan, unsigned char switch_flag)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct wifi_channel * c = NULL;

    pr_debug("%s(%d)\n", __func__, __LINE__);
    c = wifi_mac_find_chan(wifimac, chan, bw, center_chan);

    if (c == NULL) {
        ERROR_DEBUG_OUT("WARNING<%s>can't support set this channel, chan %d, bw %d, c_chan %d\n",
            (wnet_vif)->vm_ndev->name, chan, bw, center_chan);
        return false;
    }

    wnet_vif->vm_curchan = c;

    DPRINTF(AML_DEBUG_ERROR, "%s(%d),chan_pri_num =%d, chan_cfreq1 =%d,chan_flags =0x%x\n",
        __func__,__LINE__, c->chan_pri_num, c->chan_cfreq1, c->chan_flags);

    wifi_mac_add_work_task(wnet_vif->vm_wmac, wifi_mac_set_wnet_vif_chan_ex, NULL,
        (SYS_TYPE)(wnet_vif->vm_wmac), (SYS_TYPE)c, (SYS_TYPE)wnet_vif, (SYS_TYPE)switch_flag, 0);

    return true;
}

struct wifi_channel *wifi_mac_get_connect_wnet_vif_channel(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *selected_wnet_vif = wnet_vif;

    if (wifimac->wm_nrunning == 1) {
        if (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        } else {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
        }
    }
    return selected_wnet_vif->vm_curchan;
}

void wifi_mac_restore_wnet_vif_channel(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;
    struct drv_private* drv_priv = wifimac->drv_priv;
    struct wlan_net_vif *selected_wnet_vif = NULL;
    struct wlan_net_vif *p2p_vmac = wifimac->drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];

    if (wifimac->wm_nrunning > 1)
    {
    #ifdef CONFIG_CONCURRENT_MODE
        if (wifimac->wm_vsdb_slot == CONCURRENT_SLOT_P2P) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];

        } else if (wifimac->wm_vsdb_slot == CONCURRENT_SLOT_STA) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];

        } else {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];

            if (selected_wnet_vif->vm_curchan->chan_bw < drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC]->vm_curchan->chan_bw) {
                selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
            }
        }
    #endif
    } else if (wifimac->wm_nrunning == 1) {
        if (drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_state == WIFINET_S_CONNECTED) {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        } else {
            selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
        }
    }

    if (selected_wnet_vif == NULL) {
        return;
    }

    if ((selected_wnet_vif->vm_curchan != WIFINET_CHAN_ERR) && (wifimac->wm_curchan != selected_wnet_vif->vm_curchan)) {
        DPRINTF(AML_DEBUG_SCAN, "%s vid:%d, prichan:%d, bw:%d\n",  __func__, selected_wnet_vif->wnet_vif_id,
            selected_wnet_vif->vm_curchan->chan_pri_num, selected_wnet_vif->vm_curchan->chan_bw);
        if (wifimac->wm_flags & WIFINET_F_SCAN
            && IS_APSTA_CONCURRENT(aml_wifi_get_con_mode())
            && concurrent_check_vmac_is_AP(wifimac)
            && (wnet_vif->vm_opmode == WIFINET_M_STA)
            && (p2p_vmac->vm_state == WIFINET_S_CONNECTED)
            && concurrent_check_is_vmac_same_pri_channel(wifimac)) {
            wifi_mac_ChangeChannel(wifimac, selected_wnet_vif->vm_curchan, 1, selected_wnet_vif->wnet_vif_id, p2p_vmac->vm_opmode);
        } else {
            wifi_mac_ChangeChannel(wifimac, selected_wnet_vif->vm_curchan, 1, selected_wnet_vif->wnet_vif_id, selected_wnet_vif->vm_opmode);
        }
    }

    wifi_mac_set_channel_rssi(wifimac, (unsigned char)(selected_wnet_vif->vm_mainsta->sta_avg_bcn_rssi));
    if ((wifimac->wm_nrunning == 2) && concurrent_check_is_vmac_same_pri_channel(wifimac)) {
        selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC];
        wifimac->drv_priv->drv_ops.drv_set_is_mother_channel(wifimac->drv_priv, selected_wnet_vif->wnet_vif_id, 1);

        selected_wnet_vif = drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC];
        wifimac->drv_priv->drv_ops.drv_set_is_mother_channel(wifimac->drv_priv, selected_wnet_vif->wnet_vif_id, 1);

    } else {
        wifimac->drv_priv->drv_ops.drv_set_is_mother_channel(wifimac->drv_priv, selected_wnet_vif->wnet_vif_id, 1);
    }

    wifi_mac_scan_notify_leave_or_back(selected_wnet_vif, 0);
    wifimac->drv_priv->drv_ops.drv_flush_normal_buffer_queue(wifimac->drv_priv, selected_wnet_vif->wnet_vif_id);
    tasklet_schedule(&wifimac->drv_priv->ampdu_tasklet);
}

static void wifi_mac_restore_wnet_vif_channel_task_ex(void * data)
{
    struct wlan_net_vif *wnet_vif = (struct wlan_net_vif *)data;
    wifi_mac_restore_wnet_vif_channel(wnet_vif);
}

void wifi_mac_restore_wnet_vif_channel_task(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = (struct wifi_mac *)(wnet_vif->vm_wmac);
    wifi_mac_add_work_task(wifimac, wifi_mac_restore_wnet_vif_channel_task_ex, NULL, (SYS_TYPE)wnet_vif,0,0,0,0);
}

struct wifi_channel *wifi_mac_get_main_vmac_channel(struct wifi_mac *wifimac)
{
    struct drv_private *drv_priv = wifimac->drv_priv;
    if (drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC]) {
        return drv_priv->drv_wnet_vif_table[NET80211_MAIN_VMAC]->vm_curchan;

    } else {
        return WIFINET_CHAN_ERR;
    }
}

struct wifi_channel *wifi_mac_get_p2p_vmac_channel(struct wifi_mac *wifimac)
{
    struct drv_private *drv_priv = wifimac->drv_priv;
    return drv_priv->drv_wnet_vif_table[NET80211_P2P_VMAC]->vm_curchan;
}

void chan_dbg(struct wifi_channel *chan, char* str, int line)
{
    if (chan == WIFINET_CHAN_ERR) {
        return;
    }

    DPRINTF(AML_DEBUG_BWC, "%s(%d)->curr: freq1 %d flags 0x%x prim %d, maxpw %d, minpw %d bw %d\n",
        str, line, chan->chan_cfreq1,chan->chan_flags,chan->chan_pri_num,
        chan->chan_maxpower,chan->chan_minpower,chan->chan_bw);
}

/* * Find the country code. */
int find_country_code(unsigned char *countryString)
{
    int i;

    if (strlen(countryString) != 2) {
        return 0xff;
    }

    for (i = 0; i < (sizeof(country_chan_mapping_list) / sizeof(country_chan_mapping_list[0])); i++) {
        if ((country_chan_mapping_list[i].country[0] == countryString[0])
            && (country_chan_mapping_list[i].country[1] == countryString[1])) {
            return i;
        }
    }

    return 0xff;        /* Not found */
}

/* * Set the country tx_power_plan. */
int wifimac_set_tx_pwr_plan(int txpoweplan)
{
    struct wifi_mac *wifimac = wifi_mac_get_mac_handle();
    struct drv_private *drv_priv = drv_get_drv_priv();

    if (txpoweplan == drv_priv->drv_config.cfg_txpoweplan) {
        pr_debug("not need to update tx power plan(%d)\n", txpoweplan);
    } else {
        drv_priv->drv_config.cfg_txpoweplan = txpoweplan;
        wifi_mac_set_tx_power_coefficient(drv_priv, wifimac->wm_curchan, txpoweplan);
    }

    return 0;
}

/* * Get the country tx_power_plan. */
unsigned char wifimac_get_tx_pwr_plan(int country_code)
{
    int chan_plan = country_chan_mapping_list[country_code].chplan;
    unsigned char tx_power_plan = 0;

    tx_power_plan = country_chan_plan_list[chan_plan].tx_power_plan;

    return tx_power_plan;
}

/* * Set the country tx_power_plan. */
int wifi_mac_set_tx_power_coefficient(struct drv_private *drv_priv, struct wifi_channel *chan, int tx_power_plan)
{
    drv_priv->drv_ops.drv_cfg_txpwr_cffc_param(chan, &tx_power_plan_list[tx_power_plan]);
    return 0;
}


int hal_cfg_txpwr_cffc_param_init(int tx_power_plan)
{
    hal_cfg_txpwr_cffc_param(NULL, (void *)&tx_power_plan_list[tx_power_plan]);

    return 0;
}


int update_tx_power_coefficient_plan(int tx_power_plan, unsigned short pwr_coefficient[])
{
    int i = 0;
    int power_plan_size = tx_power_plan_list[tx_power_plan].cffc_num;

    for (i = 0; i < power_plan_size; i++) {
        tx_power_plan_list[tx_power_plan].coefficient[i] = pwr_coefficient[i];
        if (tx_power_plan_list[tx_power_plan].coefficient[i] == 0) {
            tx_power_plan_list[tx_power_plan].coefficient[i] = 100;
        }
    }

    return 0;
}

int update_tx_power_band(int tx_power_plan, unsigned short pwr_value[])
{
    int i = 0;

    for (i = 0; i < 4; i++) {
        tx_power_plan_list[tx_power_plan].band_pwr_table[i] = pwr_value[i];
    }

    return 0;
}

unsigned char wifi_mac_p2p_home_channel_enabled(struct wlan_net_vif *wnet_vif)
{
    struct wifi_mac *wifimac = wnet_vif->vm_wmac;

    return ((wnet_vif->vm_p2p->p2p_role == NET80211_P2P_ROLE_GO)
        && (wifimac->wm_p2p_home_channel != 0));
}

bool wifi_mac_get_chandef(struct wifi_channel *vmac_chan, struct cfg80211_chan_def *chandef)
{
    unsigned int index = 0;

    if (vmac_chan == WIFINET_CHAN_ERR)
    {
        ERROR_DEBUG_OUT("vmac_chan is WIFINET_CHAN_ERR\n");
        return false;
    }

    if (vmac_chan->chan_pri_num >= 1 && vmac_chan->chan_pri_num <= 14) {
        chandef->chan = &aml_2ghz_channels[vmac_chan->chan_pri_num - 1];
    } else if (vmac_chan->chan_pri_num >= 36) {
        for (index = 0; index < AML_5G_CHANNELS_NUM; index++) {
            if (aml_5ghz_channels[index].hw_value == vmac_chan->chan_pri_num) {
                chandef->chan = &aml_5ghz_channels[index];
                break;
            }
        }
    }

    if (!chandef->chan) {
        ERROR_DEBUG_OUT("not find ieee chan pri_num:%d\n", vmac_chan->chan_pri_num);
        return false;
    }

    if (vmac_chan->chan_bw == WIFINET_BWC_WIDTH20) {
        chandef->width = NL80211_CHAN_WIDTH_20;
    } else if (vmac_chan->chan_bw == WIFINET_BWC_WIDTH40) {
        chandef->width = NL80211_CHAN_WIDTH_40;
    } else if (vmac_chan->chan_bw == WIFINET_BWC_WIDTH80) {
        chandef->width = NL80211_CHAN_WIDTH_80;
    } else {
        ERROR_DEBUG_OUT("station chan_bw  do not support\n");
    }

    chandef->center_freq1 = vmac_chan->chan_cfreq1;

    return true;
}

unsigned char wifi_mac_get_operation_class(struct cfg80211_chan_def chandef)
{
    unsigned char vht_opclass = 0;
    unsigned int freq = chandef.center_freq1;
    unsigned char ret = 0;

    if (freq >= 2412 && freq <= 2472) {
        if (chandef.width > NL80211_CHAN_WIDTH_40)
            return ret;

        /* 2.407 GHz, channels 1..13 */
        if (chandef.width == NL80211_CHAN_WIDTH_40) {
            if (freq > chandef.chan->center_freq)
                ret = 83; /* HT40+ */
            else
                ret = 84; /* HT40- */
        } else {
            ret = 81;
        }

        return ret;
    }

    if (freq == 2484) {
        /* channel 14 is only for IEEE 802.11b */
        if (chandef.width != NL80211_CHAN_WIDTH_20_NOHT)
            return ret;
        ret = 82; /* channel 14 */
        return ret;
    }

    /* 5 GHz, channels 36..48 */
    if (freq >= 5180 && freq <= 5240) {
        if (vht_opclass) {
            ret = vht_opclass;
        } else if (chandef.width == NL80211_CHAN_WIDTH_40) {
            if (freq > chandef.chan->center_freq)
                ret = 116;
            else
                ret = 117;
        } else {
            ret = 115;
        }

        return ret;
    }

    /* 5 GHz, channels 52..64 */
    if (freq >= 5260 && freq <= 5320) {
        if (vht_opclass) {
            ret = vht_opclass;
        } else if (chandef.width == NL80211_CHAN_WIDTH_40) {
            if (freq > chandef.chan->center_freq)
                ret = 119;
            else
                ret = 120;
        } else {
            ret = 118;
        }

        return ret;
    }

    /* 5 GHz, channels 100..144 */
    if (freq >= 5500 && freq <= 5720) {
        if (vht_opclass) {
            ret = vht_opclass;
        } else if (chandef.width == NL80211_CHAN_WIDTH_40) {
            if (freq > chandef.chan->center_freq)
                ret = 122;
            else
                ret = 123;
        } else {
            ret = 121;
        }

        return ret;
    }

    /* 5 GHz, channels 149..169 */
    if (freq >= 5745 && freq <= 5845) {
        if (vht_opclass) {
            ret = vht_opclass;
        } else if (chandef.width == NL80211_CHAN_WIDTH_40) {
            if (freq > chandef.chan->center_freq)
                ret = 126;
            else
                ret = 127;
        } else if (freq <= 5805) {
            ret = 124;
        } else {
            ret = 125;
        }

        return ret;
    }

    /* 56.16 GHz, channel 1..4 */
    if (freq >= 56160 + 2160 * 1 && freq <= 56160 + 2160 * 6) {
        if (chandef.width >= NL80211_CHAN_WIDTH_40)
            return ret;

        ret = 180;
        return ret;
    }

    /* not supported yet */
    return ret;
}
