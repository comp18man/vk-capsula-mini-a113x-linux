#ifndef __WIFI_COEX_ADDR_H__
#define __WIFI_COEX_ADDR_H__

#define WIFI_COEX  (0x00f06000)

#define  RG_COEX_PRIORITY_M6  (WIFI_COEX + 0xbc)
#define  RG_COEX_PRD_NUM_M6  (WIFI_COEX + 0xc0)
#define  RG_COEX_ACT_NUM_M6  (WIFI_COEX + 0xc4)
#define  RG_COEX_OFFSET_NUM_M6  (WIFI_COEX + 0xc8)
#define  RG_COEX_INT_OFFSET_M6  (WIFI_COEX + 0xcc)
#define  RG_COEX_INI2_OFFSET_M6  (WIFI_COEX + 0xd0)

#define  RG_COEX_HS5W_MANUAL0  (WIFI_COEX + 0x4)
#define  RG_COEX_HS5W_MANUAL1  (WIFI_COEX + 0x8)
#define  RG_COEX_HS5W_MANUAL2  (WIFI_COEX + 0xc)
#define  RG_COEX_HS5W_MANUAL3  (WIFI_COEX + 0x10)
#define  RG_COEX_HS5W_MANUAL4  (WIFI_COEX + 0x14)

#define  RG_COEX_HS5W_CTRL0  (WIFI_COEX + 0x18)
#define  RG_COEX_HS5W_CTRL1  (WIFI_COEX + 0x1c)

#define  RG_COEX_RF_STABLE_CTRL  (WIFI_COEX + 0x20)
#define  RG_COEX_BT_OWNER_CTRL   (WIFI_COEX + 0x24)
#define  RG_COEX_WF_OWNER_CTRL       (WIFI_COEX + 0x28)

#define  RG_COEX_BT_LOGIC_INFO    (WIFI_COEX + 0x200)
#define  COEX_EN_ESCO                (0x00000010)
#define  RG_COEX_CFG1                (WIFI_COEX + 0x19c)
#define  RG_COEX_IRQ_END_TIME   (WIFI_COEX + 0x304)
#define  RG_COEX_WF_IRQ_MSK          (WIFI_COEX + 0x39c)
#define  RG_COEX_BT_IRQ_MSK          (WIFI_COEX + 0x3a0)

#endif

