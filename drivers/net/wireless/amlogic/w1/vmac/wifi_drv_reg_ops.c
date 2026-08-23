#include "wifi_hal_com.h"

#define  I2C_CLK_QTR   0x4  //old parameter 0x82

static void write_byte_8ba(unsigned char Bus, unsigned char SlaveAddr,
    unsigned char RegAddr, unsigned char Data)
{
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int tmp,cnt = 0;

    // Set the I2C bus to 100khz
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp = (tmp & (~(0x3FF << I2C_CLOCK_OFFSET))) | (I2C_CLK_QTR << I2C_CLOCK_OFFSET);
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    // Set the I2C Address
    hif->hif_ops.hi_write_word(I2C_SLAVE_ADDR, SlaveAddr);
    // Fill the token registers
    tmp = hif->hif_ops.hi_read_word(I2C_TOKEN_LIST_REG0);
    tmp = (I2C_END  << 16)             |
            (I2C_DATA << 12)             |    // Write Data
            (I2C_DATA << 8)              |    // Write RegAddr
            (I2C_SLAVE_ADDR_WRITE << 4)  |
            (I2C_START << 0);
    hif->hif_ops.hi_write_word(I2C_TOKEN_LIST_REG0, tmp);

    // Fill the write data registers
    hif->hif_ops.hi_write_word(I2C_TOKEN_WDATA_REG0,(Data << 8) | (RegAddr << 0));
    // Start and Wait
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp &= (~(1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);
    tmp |= ( (1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    do {
        tmp  = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);

        cnt++;
        if (cnt == 1000) {
            pr_err("-------[ERR]-----> i2c[W] err\n");
            break;
        }
    } while (tmp & (1 << 2));

}

static unsigned char read_byte_8ba(unsigned char Bus, unsigned char SlaveAddr, unsigned char RegAddr)
{
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int tmp,cnt = 0;

    // Set the I2C bus to 100khz
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp = (tmp & (~(0x3FF << I2C_CLOCK_OFFSET))) | (I2C_CLK_QTR << I2C_CLOCK_OFFSET);
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    // Set the I2C Address
    hif->hif_ops.hi_write_word(I2C_SLAVE_ADDR, SlaveAddr);
    // Fill the token registers
    tmp = hif->hif_ops.hi_read_word(I2C_TOKEN_LIST_REG0);
    tmp =   (I2C_END  << 24)             |
            (I2C_DATA_LAST << 20)        |  // this is a data read
            (I2C_SLAVE_ADDR_READ << 16)  |
            (I2C_START << 12)            |
            (I2C_DATA << 8)              |  // This is a data write
            (I2C_SLAVE_ADDR_WRITE << 4)  |
            (I2C_START << 0);
    hif->hif_ops.hi_write_word(I2C_TOKEN_LIST_REG0, tmp);

    // Fill the write data registers
    hif->hif_ops.hi_write_word(I2C_TOKEN_WDATA_REG0,(RegAddr << 0));
    // Start and Wait
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp &= (~(1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);
    tmp |= ( (1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    do {
        tmp  = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);

        cnt++;
        if (cnt == 1000) {
            pr_err("-------[ERR]-----> i2c[W] err\n");
            break;
        }
    } while (tmp & (1 << 2));

    tmp  = hif->hif_ops.hi_read_word(I2C_TOKEN_RDATA_REG0) & 0xff;
    return (unsigned char)tmp;
}


//void write_rdaddr_32ba(U8 Bus, U8 SlaveAddr, U32 TkData0, U32 TkData1)
static void write_word_32ba(unsigned char Bus, unsigned char SlaveAddr,
    unsigned int StartToken, unsigned int Data)
{
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int tmp,cnt = 0;

  // pr_debug("%s(%d) token 0x%x data 0x%x\n", __func__, __LINE__, StartToken,Data);

    // Set the I2C bus to 100khz
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp = (tmp & (~(0x3FF << I2C_CLOCK_OFFSET))) | (I2C_CLK_QTR << I2C_CLOCK_OFFSET);
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    // Set the I2C Address
    hif->hif_ops.hi_write_word(I2C_SLAVE_ADDR, SlaveAddr);
    // Fill the token registers
    tmp = hif->hif_ops.hi_read_word(I2C_TOKEN_LIST_REG0);
    tmp =   (I2C_END << 28)              |    //
            (I2C_DATA << 24)             |    //
            (I2C_DATA << 20)             |    //
            (I2C_DATA << 16)             |    //
            (I2C_DATA << 12)             |    //
            (I2C_DATA << 8)              |    //  This shall be related to start token (token = 0x04 for receiving paddr,  token = 0x00 for receiving pwdata)
            (I2C_SLAVE_ADDR_WRITE << 4)  |
            (I2C_START << 0);
    hif->hif_ops.hi_write_word(I2C_TOKEN_LIST_REG0, tmp);

    // Fill the write data registers
    hif->hif_ops.hi_write_word(I2C_TOKEN_WDATA_REG0,StartToken | (Data<<8));
    hif->hif_ops.hi_write_word(I2C_TOKEN_WDATA_REG1,(Data >> 24));
    // Start and Wait
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp &= (~(1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);
    tmp |= ( (1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    tmp = 0;
    do {
        tmp  = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);

        cnt++;
        if (cnt == 100000) {
            ERROR_DEBUG_OUT("-------[ERR]-----> i2c[W] err\n");
            break;
        }
    } while (tmp & (1 << 2));
}

static unsigned int read_word_32ba(unsigned int SlaveAddr, unsigned int RegAddr)
{
    struct hw_interface* hif = hif_get_hw_interface();
    unsigned int tmp,cnt = 0;

    // Set the I2C bus to 100khz
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp = (tmp & (~(0x3FF << I2C_CLOCK_OFFSET))) | (I2C_CLK_QTR << I2C_CLOCK_OFFSET);
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    // Set the I2C Address
    hif->hif_ops.hi_write_word(I2C_SLAVE_ADDR, SlaveAddr);
    // Fill the token registers
    tmp = hif->hif_ops.hi_read_word(I2C_TOKEN_LIST_REG0);
    tmp =    (I2C_DATA  << 28)            |
             (I2C_DATA  << 24)            |
             (I2C_DATA  << 20)            |
             (I2C_SLAVE_ADDR_READ  << 16) |  // the second burst is for READ
             (I2C_START << 12)            |  // after addr pointer has been reset to 0x00, we start a new burst for reading data out
             (I2C_DATA  << 8)             |  // This shall be related a 0x00 written into rf i2c in order to reset addr pointer to 0x00
             (I2C_SLAVE_ADDR_WRITE << 4)  |  // the first burst is for WRITE token 0x00 to reset addr pointer
             (I2C_START << 0);               // start the first burst
    hif->hif_ops.hi_write_word(I2C_TOKEN_LIST_REG0, tmp);

    tmp = hif->hif_ops.hi_read_word(I2C_TOKEN_LIST_REG1);
    tmp = (I2C_END       << 4) | (I2C_DATA_LAST << 0);
    hif->hif_ops.hi_write_word(I2C_TOKEN_LIST_REG1, tmp);

    // Fill the write data registers
    hif->hif_ops.hi_write_word(I2C_TOKEN_WDATA_REG0,RegAddr << 0);
    hif->hif_ops.hi_write_word(I2C_TOKEN_WDATA_REG1,0);
    // Start and Wait
    tmp = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);
    tmp &= (~(1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);
    tmp |= ( (1 << 0));
    hif->hif_ops.hi_write_word(I2C_CONTROL_REG, tmp);

    tmp = 0;
    do {
        tmp  = hif->hif_ops.hi_read_word(I2C_CONTROL_REG);

        cnt++;
        if (cnt == 100000) {
            ERROR_DEBUG_OUT("-------[ERR]-----> i2c[R] err\n");
            break;
        }
    } while( tmp & (1 << 2));

    tmp = hif->hif_ops.hi_read_word(I2C_TOKEN_RDATA_REG0);
    return tmp;
}

unsigned int rf_i2c_read(unsigned int reg_addr)
{
    unsigned char bus = 0;
    unsigned int slave_addr = 0x7a;
    unsigned int read_data = 0;
    unsigned int start_token;

    start_token = 0x04;

    /* 0x4-0x7 save address of read register in slave device*/
    write_word_32ba(bus, slave_addr, start_token, reg_addr);

    /* 0x0-0x3 save address of read data in slave device*/
    read_data = read_word_32ba(slave_addr, 0x0);
    return read_data;
}

void rf_i2c_write(unsigned int reg_addr, unsigned int data)
{
    unsigned char bus = 0;
    unsigned int slave_addr = 0x7a;
    unsigned int start_token;

    /* 0x0-0x3 save address of write data in slave device*/
    start_token = 0x00;  //for data to be written in
    write_word_32ba(bus, slave_addr, 0x00, data);

    //system_wait(1);

    /* 0x4-0x7 save address of write register in slave device*/
    start_token = 0x04;  //for address to be written to
    write_word_32ba(bus, slave_addr, start_token, reg_addr);

    write_byte_8ba(bus, slave_addr, 0x8, bus);
}


int new_set_reg(unsigned int address,unsigned int data)
{
    struct hal_private *HalPriv = hal_get_priv();
    if (((address >> 24) & 0xff) == 0xff ) {
#ifdef USE_T902X_RF
        rf_i2c_write( address & 0x00ffffff,data );//access t902x rf reg
#endif
    } else if (((address >> 24) & 0xf0) == 0xf0 ) {
#ifdef USE_T902X_RF
        rf_i2c_write( address & 0xffffffff,data );//access t902x rf reg
        pr_debug("%s(%d) 0x%x\n", __func__, __LINE__, data);
#endif
    } else {
        HalPriv->hif->hif_ops.hi_write_word(address, data);
    }

    return 0;
}

int new_get_reg(unsigned int address,unsigned int *data)
{
    struct hal_private *HalPriv = hal_get_priv();

    if (((address >> 24) & 0xff) == 0xff ) {
#ifdef USE_T902X_RF
        *data = rf_i2c_read(address & 0x00ffffff); //access t902x rf reg
#endif
    } else if (((address >> 24) & 0xf0) == 0xf0 ) {
#ifdef USE_T902X_RF
        *data = rf_i2c_read(address & 0xffffffff); //access t902x rf reg
         pr_debug("%s(%d) 0x%x\n", __func__, __LINE__, address);
#endif
    } else {
        *data = HalPriv->hif->hif_ops.hi_read_word(address);
    }

    return 0;
}


void i2c_set_reg_fragment(unsigned int addr,unsigned int bit_end,
                             unsigned int bit_start,unsigned int value)
{

     unsigned int tmp = 0;
     unsigned int bitwidth = 0;
     int max_value = 0;
     bitwidth = bit_end - bit_start + 1;
     max_value = (bitwidth == 32) ? -1 : (1 << (bitwidth)) - 1;

    ASSERT((bitwidth > 0)||(bit_start <= 31)||(bit_end <= 31));

    new_get_reg(addr, &tmp);
    tmp &= ~(max_value << bit_start); // clear [bit_end: bit_start]
    tmp |= ((value & max_value) << bit_start);

    new_set_reg(addr,tmp);

}


unsigned int i2c_get_reg_fragment(int address, int bit_end, int bit_start)
{
    unsigned int tmp;
    int bitwidth= bit_end-bit_start + 1;
    int max_value = (bitwidth==32)?-1: (1<<(bitwidth)) - 1;

    new_get_reg(address, &tmp);
    tmp >>= bit_start;
    tmp &= max_value;
    return (tmp);
}

unsigned int rf_read_register(unsigned int addr)
{
    unsigned int data;

    addr = addr|(0xff<<24);
    new_get_reg(addr, &data);

    return data;
}
void rf_write_register(unsigned int addr,unsigned int data)
{
    addr = addr|(0xff<<24);
    new_set_reg(addr,data);
}
unsigned int fi_ahb_read(unsigned int addr)
{
    unsigned int data;

    //addr = addr&0xffff;
    new_get_reg(addr, &data);

    return data;
}
void fi_ahb_write(unsigned int addr,unsigned int data)
{
    //addr = addr&0xffff;
    new_set_reg(addr,data);
}

void efuse_manual_write(unsigned int bit, unsigned int addr)
{

    unsigned int addr_int;
    unsigned int tmp;
    int k;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // set PD = 0
    // set sim_mask_rd_data = 1  (used to avoid reading X's during simulation)
    // set auto_wr_enable = 0, auto_rd_enable = 0
    //tmp = *(RG_EFUSE_CNTL1);
    tmp = fi_ahb_read(0xf04004);
    tmp = (tmp & ~(1 << 27)) | (0 << 27) | (1 << 31);  //bit[31] sim_mask_rd_data  //bit[27] manual_PD
    tmp = (tmp & (~(1<<24|1<<12)));                    //bit[24] auto_rd_enable    //bit[12] auto_wr_enable
    //*(RG_EFUSE_CNTL1) = tmp;
    new_set_reg(0xf04004, tmp);

    udelay(2);   // Delay at least 1100nS for proj_w1  (TSUP_PD_PS max is about 1050 nS)


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // set PS = 1  (for proj_w1, must set PS to 1 before setting CSB and PGENB to 0)
    //tmp = *(RG_EFUSE_CNTL3);
    tmp = fi_ahb_read(0xf0400c);
    tmp = (tmp & ~(1 << 27)) | (1<< 27);               //bit[27]: manual_PS
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    udelay(1);   // Delay at least 68nS for proj_w1 (TSUP_PS_CS max is about 68 ns)


    //////////////////////////////////////////////////////
    // CSB: Chip select enable(active low)
    // PGENB: Program enable(active low)
    // LOAD: High to turn ON sense amplifier and load data into latch
    //
    // set CSB = 0, set PGENB = 0, set LOAD = 0
    //
    //tmp = *(RG_EFUSE_CNTL3);
    tmp = fi_ahb_read(0xf0400c);
    tmp = (tmp & ~(1 << 31));  //bit[31]: manual_CSB
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    tmp = (tmp & ~(1 << 30));  //bit[30]: manual_PGENB
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    tmp = (tmp & ~(1 << 29));  //bit[29]: manual_LOAD
    tmp = (tmp & ~(1 << 28));  //bit[28]: manual_STROBE
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    //////////////////////////////////////////////////////
    // bit[30] = 0;
    // bit[29] = 0;
    // wire  [12:0]  manual_D_ADDR       = cntl_reg3[12:0];
    // for 1Kbit size,   manual_D_ADDR[12:0] = {bit[4:0],addr[4:0]};
    // for 512bit size,  manual_D_ADDR[12:0] = {bit[4:0],addr[3:0]};

    addr_int    = (bit << 4) | (addr & 0xF);    //512bit size cell

    //tmp = *(RG_EFUSE_CNTL3);
    tmp = fi_ahb_read(0xf0400c);
    tmp = (tmp & ~((1 << 31) | (1 << 30) | (1 << 29) | (1 << 28) | (0xFFF << 0)));
    tmp = (tmp | (addr_int << 0)) ;   //bit[12:0] :  manual_ADDR
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    udelay(1); // Delay


    //////////////////////////////////////////////////////
    // Generate a strobe:  set STROBE = 1 (bit[28] = 1)
    //tmp = *(RG_EFUSE_CNTL3);
    tmp = fi_ahb_read(0xf0400c);
    tmp = (tmp & ~(1 << 28)) | (1 << 28);  //set STROBE = 1
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    udelay(2);  //wait at least 12 uS  for STROBE
    for ( k = 0; k < 6; k++) {  //do something in order to delay more time for accurate 12uS
        //tmp = *(RG_EFUSE_CNTL3);
        tmp = fi_ahb_read(0xf0400c);
    }

    //////////////////////////////////////////////////////
    // set STROBE = 0 (bit[28] = 0)
    //tmp = *(RG_EFUSE_CNTL3);
    tmp = (tmp & ~(1 << 28));  //set STROBE = 0
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    udelay(1);

    //////////////////////////////////////////////////////
    // set LOAD = 1, set PGENB = 1, set CSB = 1
    //tmp = *(RG_EFUSE_CNTL3);
    tmp = fi_ahb_read(0xf0400c);
    tmp = tmp | (1<<29);    //bit[29]: manual_LOAD
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    tmp = tmp | (1<<30);    //bit[29]: manual_PGENB
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    tmp = tmp | (1<<31);    //bit[31]: manual_CSB
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    udelay(1);

    // set PS = 0
    //tmp = *(RG_EFUSE_CNTL3);
    tmp = fi_ahb_read(0xf0400c);
    tmp = (tmp & ~(1 << 27)) | (0 << 27);
    //*(RG_EFUSE_CNTL3) = tmp;
    new_set_reg(0xf0400c, tmp);

    udelay(1);

    // set PD = 1
    //tmp = *(RG_EFUSE_CNTL1);
    tmp = fi_ahb_read(0xf04004);
    tmp = (tmp & ~(1 << 27)) | (1 << 27);
    //*(RG_EFUSE_CNTL1) = tmp;
    new_set_reg(0xf04004, tmp);

}

unsigned int efuse_manual_read(unsigned int addr)
{
    unsigned int tmp;
    unsigned int   rdata = 0 ;
    int i = 0;

    tmp  =  0x1F1F01BE ;    //bit[10]   set  encrypt_en = 0  in order to read Efuse data
    new_set_reg (0x00F04010,  tmp);    //Efuse controller register base = 0x00F0 4000

    tmp  =  0x80000000 ;    //bit[31] = 1   bit[27] = 0   bit[24] = 0   bit[12]= 0  //bit[27] manual_PD = 0
    new_set_reg (0x00F04004,  tmp);     //Efuse controller register base = 0x00F0 4000

    udelay(2);     //Delay at least 1100 ns between PD and PS
    tmp  =  0xE0000000 ;    //default value for Efuse  0x00F0 400C register //make sure  bit[27] = 0    //bit[27]: manual_PS
    new_set_reg ( 0x00F0400C,  tmp );

    udelay(1);     //Delay at least 80 ns between PS and CSB
    tmp  =  tmp & (~(1<<31)) ;    //manual_CSB = 0
    tmp  =  tmp | (1 << 30) | (1 << 29) ;    //manual_PGENB = 1, manual_LOAD = 1
    tmp  =  tmp & (~(1<<28)) ;
    tmp  =  tmp | (addr & 0x1fff) ;   //for W1 chip,  efuse address range is 0x0~0xF
    new_set_reg (0x00F0400C,  tmp);

    tmp  =  tmp|(1<< 28) ;      //set  STROBE = 1
    new_set_reg (0x00F0400C,  tmp);

    udelay(1);     // this delay may be optional,  since I2C_Wr may take enough time
    tmp  =  tmp  &  (~(1<<28));   //set  STROBE = 0
    new_set_reg (0x00F0400C,  tmp);

    //bit[31] = 0   bit[27] = 0   bit[24] = 0   bit[12]= 0
    //bit[27] manual_PD = 0   bit[31] mask_rd_data = 0
    //bit[31] is sim_mask_rd_data  --  used to avoid reading X's during simulation
    //set bit[31] = 0 in order to read data out
    tmp  =  0x00000000 ;
    new_set_reg (0x00F04004, tmp);

    tmp  =  fi_ahb_read (0x00F0400C) ;
    for  ( i = 0;  i < 4;  i= i+1 ) {
        tmp = (tmp & (~(0x7 << 24))) | (i<<24) ;    //use i to select bytes in one 32-bit word
        new_set_reg (0x00F0400C, tmp);

        tmp  =  fi_ahb_read (0x00F0400C) ;
        tmp = (tmp >> 16) & (0xFF);
        rdata = rdata | (tmp << (8*i)) ;   //get one byte to store in 32-bit Read data
    }

    tmp  =  0xE0000000 ;    //bit[31]  manual_CSB = 1 ,   bit[30]  manual_PGENB = 1
                          //bit[29]  manual_LOAD = 1 ,   bit[27]  manual_PS = 0
    new_set_reg (0x00F0400C,  tmp);

    tmp  =  0x88000000 ;    //bit[31] = 1   bit[27] =1   bit[24] = 0   bit[12]= 0
                         //bit[27] manual_PD = 1    //set PD = 1
    new_set_reg (0x00F04004, tmp);

    return (rdata);
}
