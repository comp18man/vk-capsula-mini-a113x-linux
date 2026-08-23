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
#include "wifi_hal_com.h"
#include <linux/namei.h>
#include "wifi_common.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
static int openFile(struct file **fpp, const char *path, int flag, int mode)
{
    struct file *fp;

    fp = filp_open(path, flag, mode);
    if (IS_ERR(fp)) {
        *fpp = NULL;
        return PTR_ERR(fp);
    } else {
        *fpp = fp;
        return 0;
    }
}
#endif

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp)
{
    filp_close(fp, NULL);
    return 0;
}

static int readFile(struct file *fp, char *buf, int len)
{
    int rlen = 0, sum = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    if (!(fp->f_mode & FMODE_CAN_READ)) {
#else
    if (!fp->f_op || !fp->f_op->read) {
#endif
        return -EPERM;
    }

    while (sum < len) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        rlen = kernel_read(fp, buf + sum, len - sum, &fp->f_pos);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
        rlen = __vfs_read(fp, buf + sum, len - sum, &fp->f_pos);
#else
        rlen = fp->f_op->read(fp, buf + sum, len - sum, &fp->f_pos);
#endif
#endif
        if (rlen > 0) {
            sum += rlen;
        } else if (0 != rlen) {
            return rlen;
        } else {
            break;
        }
    }

    return sum;

}

static int writeFile(struct file *fp, char *buf, int len)
{
    int wlen = 0, sum = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    if (!(fp->f_mode & FMODE_CAN_WRITE)) {
#else
    if (!fp->f_op || !fp->f_op->write) {
#endif
        return -EPERM;
    }

    while (sum < len) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        wlen = kernel_write(fp, buf + sum, len - sum, &fp->f_pos);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
        wlen = __vfs_write(fp, buf + sum, len - sum, &fp->f_pos);
#else
        wlen = fp->f_op->write(fp, buf + sum, len - sum, &fp->f_pos);
#endif
#endif
        if (wlen > 0) {
            sum += wlen;
        } else if (0 != wlen) {
            return wlen;
        } else {
            break;
        }
    }
    return sum;
}

/*
* Test if the specific @param path is a file and readable
* If readable, @param sz is got
* @param path the path of the file to test
* @return Linux specific error code
*/
int isFileReadable(const char *path, u32 *sz)
{
    int ret = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    struct file *fp;
    mm_segment_t oldfs;
    char buf;

    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        ret = PTR_ERR(fp);
    } else {
        oldfs = get_fs();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
        set_fs(KERNEL_DS);
#else
        set_fs(get_ds());
#endif
        if (1 != readFile(fp, &buf, 1)) {
            ret = PTR_ERR(fp);
        }

        if (ret == 0 && sz) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
            *sz = i_size_read(fp->f_path.dentry->d_inode);
#else
            *sz = i_size_read(fp->f_dentry->d_inode);
#endif
        }

        set_fs(oldfs);
        filp_close(fp, NULL);
    }
#endif
    return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(const char *path, u8 *buf, u32 sz)
{

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    int ret = -1;
    mm_segment_t oldfs;
    struct file *fp;

    if (path && buf) {
        ret = openFile(&fp, path, O_RDONLY, 0);
        if (0 == ret) {
            pr_debug("openFile path:%s fp=%p\n", path , fp);
            oldfs = get_fs();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
            set_fs(KERNEL_DS);
#else
            set_fs(get_ds());
#endif
            ret = readFile(fp, buf, sz);
            set_fs(oldfs);
            closeFile(fp);

            pr_debug("readFile, ret:%d\n", ret);
        } else {
            ERROR_DEBUG_OUT("openFile path:%s Fail, ret:%d\n", path, ret);
        }
    } else {
        ERROR_DEBUG_OUT("NULL pointer\n");
        ret = -EINVAL;
    }
    return ret;
#else
    return 0;
#endif
}

/*
* Open the file with @param path and write @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(const char *path, u8 *buf, u32 sz)
{
    int ret = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
    mm_segment_t oldfs;
    struct file *fp;

    if (path && buf) {
        ret = openFile(&fp, path, O_CREAT | O_WRONLY, 0666);
        if (0 == ret) {
            pr_debug("openFile path:%s fp=%p\n", path , fp);
            oldfs = get_fs();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
            set_fs(KERNEL_DS);
#else
            set_fs(get_ds());
#endif
            ret = writeFile(fp, buf, sz);
            set_fs(oldfs);
            closeFile(fp);
            pr_debug("writeFile, ret:%d\n", ret);
        } else {
            ERROR_DEBUG_OUT("openFile path:%s Fail, ret:%d\n", path, ret);
        }
    } else {
        ERROR_DEBUG_OUT("NULL pointer\n");
        ret =  -EINVAL;
    }
#endif
    return ret;
}

/*
* Test if the specific @param path is a file and readable
* @param path the path of the file to test
* @return true or false
*/
int aml_is_file_readable(const char *path)
{
    if (isFileReadable(path, NULL) == 0) {
        return true;
    } else {
        return false;
    }
}

/*
* Test if the specific @param path is a file and readable.
* If readable, @param sz is got
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int aml_is_file_readable_with_size(const char *path, u32 *sz)
{
    if (isFileReadable(path, sz) == 0) {
        return true;
    } else {
        return false;
    }
}

/*
* Test if the specific @param path is a readable file with valid size.
* If readable, @param sz is got
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int aml_readable_file_sz_chk(const char *path, u32 sz)
{
    u32 fsz;

    if (aml_is_file_readable_with_size(path, &fsz) == false) {
        return false;
    }

    if (fsz > sz) {
        return false;
    }

    return true;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int aml_retrieve_from_file(const char *path, u8 *buf, u32 sz)
{
    int ret = retriveFromFile(path, buf, sz);
    return ret >= 0 ? ret : 0;
}

/*
* Open the file with @param path and write @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int aml_store_to_file(const char *path, u8 *buf, u32 sz)
{
    int ret = storeToFile(path, buf, sz);
    return ret >= 0 ? ret : 0;
}

/**
* IsHexDigit -
*
* Return TRUE if chTmp is represent for hex digit
* FALSE otherwise.
*/
bool aml_char_is_hex_digit(char chTmp)
{
    if ((chTmp >= '0' && chTmp <= '9') ||
        (chTmp >= 'a' && chTmp <= 'f') ||
        (chTmp >= 'A' && chTmp <= 'F')) {
        return true;
    } else {
        return false;
    }
}


u32 aml_read_macaddr_from_file(const char *path, u8 *buf)
{
    u32 i;
    u8 temp[3];
    u32 ret = false;

    u8 file_data[FILE_DATA_LEN];
    u32 read_size;
    u8 addr[ETH_ALEN];

    read_size = aml_retrieve_from_file(path, file_data, FILE_DATA_LEN);
    if (read_size != FILE_DATA_LEN) {
        pr_err("%s read from %s fail\n", __func__, path);
        goto exit;
    }

    temp[2] = 0; /* end of string '\0' */

    for (i = 0 ; i < ETH_ALEN ; i++) {
        if (aml_char_is_hex_digit(file_data[PARSE_DIGIT_BASE + i * 3]) == false
            || aml_char_is_hex_digit(file_data[PARSE_DIGIT_BASE + i * 3 + 1]) == false) {
            pr_err("%s invalid 8-bit hex format for address offset:%u\n", __func__, i);
            goto exit;
        }

        if (i < ETH_ALEN - 1 && file_data[PARSE_DIGIT_BASE + i * 3 + 2] != ':') {
            pr_err("%s invalid separator after address offset:%u\n", __func__, i);
            goto exit;
        }

        temp[0] = file_data[PARSE_DIGIT_BASE + i * 3];
        temp[1] = file_data[PARSE_DIGIT_BASE + i * 3 + 1];
        if (sscanf(temp, "%hhx", &addr[i]) != 1) {
            pr_err("%s sscanf fail for address offset:0x%03x\n", __func__, i);
            goto exit;
        }
    }
    memset(buf, '\0', ETH_ALEN);
    memcpy(buf, addr, ETH_ALEN);

    ret = true;

exit:
    return ret;
}
