/*
 ****************************************************************************************
 *
 * Copyright (C) Amlogic 2020-2024
 *
 * Project: file read write
 *
 * Description:
 *     file read write fun
 *
 *
 ****************************************************************************************
 */
#ifndef _WIFI_COMMON_H_
#define _WIFI_COMMON_H_

#define FILE_DATA_LEN 48
#define PARSE_DIGIT_BASE 30
#ifdef CONFIG_AML_USE_STATIC_BUF
void *wifi_mem_prealloc(int section, unsigned long size);
#endif

int aml_is_file_readable(const char *path);
int aml_is_file_readable_with_size(const char *path, u32 *sz);
int aml_readable_file_sz_chk(const char *path, u32 sz);
int aml_retrieve_from_file(const char *path, u8 *buf, u32 sz);
int isFileReadable(const char *path, u32 *sz);
int aml_store_to_file(const char *path, u8 *buf, u32 sz);
u32 aml_read_macaddr_from_file(const char *path, u8 *buf);
bool aml_char_is_hex_digit(char chTmp);

#endif
