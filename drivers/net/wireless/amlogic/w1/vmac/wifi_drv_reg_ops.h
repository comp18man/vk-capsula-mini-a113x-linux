#ifndef _WIFI_DRV_REG_OPS_H_
#define _WIFI_DRV_REG_OPS_H_

/*********************************************************************/
/***************************** I2C Module ***************************/
/******************************************************************/
#ifdef    DRIVER_FOR_BT //for I2C BASE FOR ACCESS BT RF
#define  I2C_REG_BASEADDR     (0x00250800)
#else                           //for I2C BASE FOR ACCESS WIFI RF
#define  I2C_REG_BASEADDR     (0x00004800)
#endif

#define  I2C_CONTROL_REG        (I2C_REG_BASEADDR + 0x00)
#define  I2C_SLAVE_ADDR           (I2C_REG_BASEADDR + 0x04)
#define  I2C_TOKEN_LIST_REG0  (I2C_REG_BASEADDR + 0x08)
#define  I2C_TOKEN_LIST_REG1  (I2C_REG_BASEADDR + 0x0c)
#define  I2C_TOKEN_WDATA_REG0 (I2C_REG_BASEADDR + 0x10)
#define  I2C_TOKEN_WDATA_REG1 (I2C_REG_BASEADDR + 0x14)
#define  I2C_TOKEN_RDATA_REG0 (I2C_REG_BASEADDR + 0x18)
#define  I2C_TOKEN_RDATA_REG1 (I2C_REG_BASEADDR + 0x1c)

#define  I2C_CLOCK_OFFSET  (12)
#define  I2C_END               0x0
#define  I2C_START             0x1
#define  I2C_SLAVE_ADDR_WRITE  0x2
#define  I2C_SLAVE_ADDR_READ   0x3
#define  I2C_DATA              0x4
#define  I2C_DATA_LAST         0x5
#define  I2C_STOP              0x6

unsigned int rf_i2c_read(unsigned int reg_addr);
void rf_i2c_write(unsigned int reg_addr, unsigned int data);
int new_set_reg(unsigned int address,unsigned int data);
int new_get_reg(unsigned int address,unsigned int *data);
void i2c_set_reg_fragment(unsigned int addr,unsigned int bit_end,
unsigned int bit_start,unsigned int value);
unsigned int i2c_get_reg_fragment(int address, int bit_end, int bit_start);
extern unsigned int efuse_manual_read(unsigned int addr);
extern void efuse_manual_write(unsigned int bit, unsigned int addr);

#endif
