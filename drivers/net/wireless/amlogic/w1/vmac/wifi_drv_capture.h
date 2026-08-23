#ifndef __WIFI_DEV_CAPTURE_H
#define __WIFI_DEV_CAPTURE_H
#include "wifi_hal_com.h"

#define TESTBUSBUF_LEN  0x8000

int dut_start_capture(unsigned int value);
int   dut_stop_capture(void);

//0:use sw stop   1:use hw stop
int  dut_stop_tbus_to_get_sram(struct file *filep, int stop_ctrl, int save_file);
unsigned char dut_set_reg_frag(int address, int bit_end, int bit_start, int value);
int dut_bt_start_capture(unsigned int value );
int dut_bt_stop_capture(void);
unsigned int dut_get_reg_frag(int address, int high_bit, int low_bit);
int dut_v32_tx(unsigned int *outbuffer, unsigned int *inbufer, const unsigned long long bitnum);
int dut_dump_data(unsigned int addr, unsigned char *data, int len);
void dut_set_bcap_log_name(unsigned int value);
void dut_set_gain(unsigned int value);
int iwp_stop_tbus_to_get_sram(unsigned char *buf);

#endif
