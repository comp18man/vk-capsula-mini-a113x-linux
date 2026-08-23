#ifndef __WIFI_I2C_ADDR_H__
#define __WIFI_I2C_ADDR_H__

#define WIFI_I2C          (0x4800)

#define WIFI_I2C_M0_CONTROL_REG (WIFI_I2C + (0x00 << 2))
#define WIFI_I2C_M0_SLAVE_ADDR (WIFI_I2C + (0x01 << 2))
#define WIFI_I2C_M0_TOKEN_LIST0 (WIFI_I2C + (0x02 << 2))
#define WIFI_I2C_M0_TOKEN_LIST1 (WIFI_I2C + (0x03 << 2))
#define WIFI_I2C_M0_WDATA_REG0 (WIFI_I2C + (0x04 << 2))
#define WIFI_I2C_M0_WDATA_REG1 (WIFI_I2C + (0x05 << 2))
#define WIFI_I2C_M0_RDATA_REG0 (WIFI_I2C + (0x06 << 2))

#endif
