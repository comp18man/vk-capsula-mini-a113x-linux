#ifndef __ADDR_UART_H__
#define __ADDR_UART_H__


#define WIFI_UART (0x4c00)
#define RG_UART_WORK_MODE (WIFI_UART + 0x08)

struct  UART_UART_WFIFO_BITS
{
    unsigned int uart_wfifo :8,
                 reserved0  :24;
};
struct  UART_UART_RFIFO_BITS
{
    unsigned int uart_rfifo :8,
                 reserved1  :24;
};
struct  UART_CNTL_REG5_BITS
{
    unsigned int cntl_reg5 :30,
                 reserved2 :2;
};
struct  UART_RESERVED6_BITS
{
    unsigned int timeout_int_tcnt       :14,
                 reserved3              :2,
                 timeout_int_tb_sel     :1,
                 rxstart_detect_long_en :1,
                 reserved4              :14;
};
struct  UART_CNTL_REG7_BITS
{
    unsigned int cntl_reg7 :26,
                 reserved5 :6;
};
struct  UART_CNTL_REG8_BITS
{
    unsigned int cntl_reg8 :16,
                 reserved6 :16;
};
#endif
