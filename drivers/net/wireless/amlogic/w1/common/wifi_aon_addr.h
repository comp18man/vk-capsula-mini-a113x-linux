#ifndef __ADDR_AON_H__
#define __ADDR_AON_H__


//#define WIFI_AON (0xf800)
#define WIFI_AON (0x00F00000)

// pmu status
#define PMU_PWR_OFF       0x0
#define PMU_PWR_XOSC      0x1
#define PMU_XOSC_WAIT     0x2
#define PMU_XOSC_DPLL     0x3
#define PMU_DPLL_WAIT     0x4
#define PMU_DPLL_ACT      0x5
#define PMU_ACT_MODE      0x6
#define PMU_ACT_SLEEP     0x7
#define PMU_SLEEP_MODE    0x8
#define PMU_SLEEP_WAKE    0x9
#define PMU_WAKE_WAIT     0xa
#define PMU_WAKE_XOSC     0xb

#define WIFI_RG_AON_CFG4  (WIFI_AON + 0x78)
#define WIFI_RG_AON_CFG5  (WIFI_AON + 0x7C)
#endif
