#include "aml_crypto_wrap.h"

int os_memcmp_const(const void *a, const void *b, size_t len)
{
    const u8 *aa = a;
    const u8 *bb = b;
    size_t i;
    u8 res;

    for (res = 0, i = 0; i < len; i++)
        res |= aa[i] ^ bb[i];

    return res;
}

void* os_memdup(const void *src, u32 sz)
{
    void *r = os_malloc(sz);

    if (r && src)
        memcpy(r, src, sz);
    return r;
}

size_t os_strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}

void forced_memzero(void *ptr, size_t len)
{
    memset(ptr, 0, len);
}

void bin_clear_free(void *bin, size_t len)
{
    if (bin) {
        memset(bin, 0, len);
        kfree(bin);
    }
}

void wpa_printf(int level, const char *fmt, ...)
{
#define MSG_LEN 100
    va_list args;
    u8 buf[MSG_LEN] = { 0 };
    int err;

    va_start(args, fmt);
    err = vsnprintf(buf, MSG_LEN, fmt, args);
    va_end(args);

    pr_info("%s", buf);
#undef MSG_LEN
}

void wpa_hexdump(int level, const char *title, const void *buf, size_t len)
{
}

void wpa_hexdump_key(int level, const char *title, const void *buf, size_t len)
{
}
