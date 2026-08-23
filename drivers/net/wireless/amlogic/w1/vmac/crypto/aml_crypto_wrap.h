#ifndef _AML_CRYTO_WRAP_H_
#define _AML_CRYTO_WRAP_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include "wifi_mac_com.h"

#define TEST_FAIL() 0

#define os_memset memset
#define os_memcpy memcpy
#define os_memcmp memcmp
#define os_malloc(size) kmalloc(size, GFP_ATOMIC)

#define WPA_GET_BE16(a) ((u16) (((a)[0] << 8) | (a)[1]))
#define WPA_PUT_BE16(a, val)			\
	do {					\
		(a)[0] = ((u16) (val)) >> 8;	\
		(a)[1] = ((u16) (val)) & 0xff;	\
	} while (0)

#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WPA_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u16) (val)) >> 8;	\
		(a)[0] = ((u16) (val)) & 0xff;	\
	} while (0)

#define WPA_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u32) (a)[1]) << 8) | \
			 ((u32) (a)[2]))
#define WPA_PUT_BE24(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[2] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_BE32(a) ((((u32) (a)[0]) << 24) | (((u32) (a)[1]) << 16) | \
			 (((u32) (a)[2]) << 8) | ((u32) (a)[3]))
#define WPA_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_LE32(a) ((((u32) (a)[3]) << 24) | (((u32) (a)[2]) << 16) | \
			 (((u32) (a)[1]) << 8) | ((u32) (a)[0]))
#define WPA_PUT_LE32(a, val)					\
	do {							\
		(a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
			 (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
			 (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
			 (((u64) (a)[6]) << 8) | ((u64) (a)[7]))
#define WPA_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u8) (((u64) (val)) >> 56);	\
		(a)[1] = (u8) (((u64) (val)) >> 48);	\
		(a)[2] = (u8) (((u64) (val)) >> 40);	\
		(a)[3] = (u8) (((u64) (val)) >> 32);	\
		(a)[4] = (u8) (((u64) (val)) >> 24);	\
		(a)[5] = (u8) (((u64) (val)) >> 16);	\
		(a)[6] = (u8) (((u64) (val)) >> 8);	\
		(a)[7] = (u8) (((u64) (val)) & 0xff);	\
	} while (0)

#define WPA_GET_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
			 (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
			 (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
			 (((u64) (a)[1]) << 8) | ((u64) (a)[0]))
#define WPA_PUT_LE64(a, val)					\
	do {							\
		(a)[7] = (u8) ((((u64) (val)) >> 56) & 0xff);	\
		(a)[6] = (u8) ((((u64) (val)) >> 48) & 0xff);	\
		(a)[5] = (u8) ((((u64) (val)) >> 40) & 0xff);	\
		(a)[4] = (u8) ((((u64) (val)) >> 32) & 0xff);	\
		(a)[3] = (u8) ((((u64) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u64) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u64) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u64) (val)) & 0xff);		\
	} while (0)

enum {
	_MSG_EXCESSIVE_, _MSG_MSGDUMP_, _MSG_DEBUG_, _MSG_INFO_, _MSG_WARNING_, _MSG_ERROR_
};


#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#define ETH_ALEN 6
#define ETH_TYPE_LEN 2
#define PAYLOAD_TYPE_LEN 1

int os_memcmp_const(const void *a, const void *b, size_t len);
void *os_memdup(const void *src, u32 sz);
size_t os_strlen(const char *s);

void wpa_printf(int level, const char *fmt, ...);
void wpa_hexdump(int level, const char *title, const void *buf, size_t len);
void wpa_hexdump_key(int level, const char *title, const void *buf, size_t len);

void forced_memzero(void *ptr, size_t len);
void bin_clear_free(void *bin, size_t len);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
u32 crc32(const u8 *frame, size_t frame_len);
#endif
#endif//_AML_CRYTO_WRAP_H_
