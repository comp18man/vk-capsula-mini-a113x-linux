/*
 * This file is an implementation of the SampleRate algorithm
 * in "Bit-rate Selection in Wireless Networks"
 * (http://www.pdos.lcs.mit.edu/papers/jbicket-ms.ps)
 *
 * SampleRate chooses the bit-rate it predicts will provide the most
 * throughput based on estimates of the expected per-packet
 * transmission time for each bit-rate.  SampleRate periodically sends
 * packets at bit-rates other than the current one to estimate when
 * another bit-rate will provide better performance. SampleRate
 * switches to another bit-rate when its estimated per-packet
 * transmission time becomes smaller than the current bit-rate's.
 * SampleRate reduces the number of bit-rates it must sample by
 * eliminating those that could not perform better than the one
 * currently being used.  SampleRate also stops probing at a bit-rate
 * if it experiences several successive losses.
 *
 * The difference between the algorithm in the thesis and the one in this
 * file is that the one in this file uses a ewma instead of a window.
 *
 * Also, this implementation tracks the average transmission time for
 * a few different packet sizes independently for each link.
 */

#include "wifi_rate_ctrl.h"
#include "rc80211_minstrel_init.h"

int max_4ms_framelen[MCS_VHT160_SGI +1][32] =
{
    [MCS_HT20] = {
        3212,  6432,  9648,  12864,  19300,  25736,  28952,  28952,
        28952,  28952, 19280, 25708,  38568,  51424,  57852,  64280,
        9628,  19260, 28896, 38528,  57792,  65532,  65532,  65532,
        12828, 25656, 38488, 51320,  65532,  65532,  65532,  65532,
    },
    [MCS_HT20_SGI] = {
        3572,  7144,  10720,  14296,  21444,  28596,  32172,  35744,
        7140,  14284, 21428,  28568,  42856,  57144,  64288,  65532,
        10700, 21408, 32112,  42816,  64228,  65532,  65532,  65532,
        14256, 28516, 42780,  57040,  65532,  65532,  65532,  65532,
    },
    [MCS_HT40] = {
        6680,  13360,  20044,  26724,  40092,  53456,  60140,  65532,
        13348, 26700,  40052,  53400,  65532,  65532,  65532,  65532,
        20004, 40008,  60016,  65532,  65532,  65532,  65532,  65532,
        26644, 53292,  65532,  65532,  65532,  65532,  65532,  65532,
    },
    [MCS_HT40_SGI] = {
        7420,  14844,  22272,  29696,  44544,  59396,  65532,  65532,
        14832, 29668,  44504,  59340,  65532,  65532,  65532,  65532,
        22232, 44464,  65532,  65532,  65532,  65532,  65532,  65532,
        29616, 59232,  65532,  65532,  65532,  65532,  65532,  65532,
    },
    [  MCS_VHT80] = {
	20036 - DELT, 40072 - DELT, 60108 -DELT, 80145-DELT, 120217-DELT,  // MCS0 ~4
	169290-DELT, 180326-DELT, 200362-DELT, 240435-DELT, 267150-DELT, // MCS5 ~ 9
	0,0,0,0,0,
	0,0,0,0,0,
	0,0,0,0,0,
	0,0,0,0,0,
	0,0,
	}
};

////////////////////////////////////////////////////////////////////
// ratecontrol module
////////////////////////////////////////////////////////////////////
struct ratecontrol_ops minstrel_ops =
{
    .name              = "minstrel",
    .rate_attach       = aml_minstrel_attach,
    .rate_detach       = aml_minstrel_detach,
    .rate_newassoc     = aml_minstrel_init,
    .rate_disassoc     = aml_minstrel_deinit,
    .rate_findrate     = minstrel_find_rate,
    .rate_tx_complete  = minstrel_tx_complete,
};

