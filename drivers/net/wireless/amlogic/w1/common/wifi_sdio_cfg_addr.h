#ifndef __WIFI_SDIO_CFG_ADDR_H__
#define __WIFI_SDIO_CFG_ADDR_H__

//Base ADDR
// for statistic host module
#define MOD_VADDR (0xF000)  
#define HOST_SW_VADDR (MOD_VADDR + 0x0)

// bit2,bit3 keep r/w ptr after amlogic's self-definition reset
#define RG_SCFG_RX_EN (0x74)
#define RG_SCFG_SRAM_FUNC (0x8040)
#define RG_SCFG_REG_FUNC (0x8044)
#define RG_SCFG_EEPROM_FUNC (0x8048)
#define RG_SCFG_SRAM_FUNC6 (0x8160)
#define RG_SCFG_FUNC3_BADDR_A (0x8130)
#define RG_SCFG_FUNC5_BADDR_A (0x8150)
/* select reset sdio keep sdio slave in selected state*/
#define RG_SCFG_SELECT_RST (0x9000)

// sdio pmu
#define RG_SDIO_PMU_WAKE   (0x220)
/* host sleep req bit13-bit120, bit13-bit15 are mapped with
bit5-bit7 of 0x221, bit16-bit20 are mapped with bit0-bit4 of 0x222 */
#define RG_SDIO_PMU_HOST_REQ   (0x221)
/* bit 0-3 pmu status, bit 4 wf power en, bit 5-7 fw status (get
    fw status from RG_AON_A30[bit3-0] )*/
#define RG_SDIO_PMU_STATUS (0x23c)

#ifdef PROJECT_T9026
#define FW_SLEEP  BIT(5) //host, t9026, flag in RG_SDIO_PMU_STATUS
#define HOST_SLEEP_REQ  BIT(6) //host, t9026, flag in RG_SDIO_PMU_STATUS
#else
#define FW_SLEEP  BIT(4) //host, w1, flag in RG_SDIO_PMU_STATUS
#define HOST_SLEEP_REQ  BIT(5) //host, w1, flag in RG_SDIO_PMU_HOST_REQ
#endif
#endif

