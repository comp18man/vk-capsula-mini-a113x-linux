#include <linux/mmc/sdio_func.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>    /* udelay */
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/moduleparam.h>
#include <linux/irqreturn.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h> /* pr_debug() */
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/gpio.h> //mach
#include <linux/timer.h>
#include <linux/string.h>

extern void sdio_reinit(void);
extern void amlwifi_set_sdio_host_clk(int clk);
extern void set_usb_bt_power(int is_on);

#define PRINT(...)      do {pr_debug("w1_sdio->");pr_debug( __VA_ARGS__ );}while(0)
#ifndef ASSERT
#define ASSERT(exp) do{    \
                if (!(exp)) {   \
                        pr_err("=>=>=>=>=>assert %s,%d\n",__func__,__LINE__);   \
                        /*BUG();        while(1);   */  \
                }                       \
        } while (0);
#endif

#define ERROR_DEBUG_OUT(format,...) do {    \
                 pr_err("FUNCTION: %s LINE: %d:"format"",__FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)


#define CHIP_BT_PMU_REG_BASE               (0xf03000)
#define RG_BT_PMU_A17                             (CHIP_BT_PMU_REG_BASE + 0x44)
#define RG_BT_PMU_A18                             (CHIP_BT_PMU_REG_BASE + 0x48)
#define RG_BT_PMU_A20                             (CHIP_BT_PMU_REG_BASE + 0x50)
#define RG_BT_PMU_A22                             (CHIP_BT_PMU_REG_BASE + 0x58)

#define W1_PRODUCT_AMLOGIC  0x8888
#define W1_VENDOR_AMLOGIC  0x8888

//sdio manufacturer code, usually vendor ID, 'a'=0x61, 'm'=0x6d
#define W1_VENDOR_AMLOGIC_EFUSE ('a'|('m'<<8))
//sdio manufacturer info, usually product ID
#define W1_PRODUCT_AMLOGIC_EFUSE (0x9007)

#define WIFI_SDIO_IF    (0x5000)

/* APB domain, checksum error status, checksum enable, frame flag bypass*/
#define RG_SDIO_IF_MISC_CTRL (WIFI_SDIO_IF+0x80)
#define RG_SDIO_IF_MISC_CTRL2 (WIFI_SDIO_IF+0x84)
#define RG_SCFG_FUNC5_BADDR_A (0x8150)
#define SDIO_ADDR_MASK (128*1024-1)
#define SDIO_OPMODE_INCREMENT 1
#define SDIO_OPMODE_FIXED 0
#define SDIO_WRITE 1
#define SDIO_READ 0
#define SDIOH_API_RC_SUCCESS (0x00)
#define SDIOH_API_RC_FAIL (0x01)

typedef unsigned long SYS_TYPE;

/* Stand sdio function number from 0~7
** Func1: For register operation, cmd52
** Func2: For sram operation,cmd53
** Func3: For writer CMD,cmd53,irq fw arc
** Func4: For Tx/Rx operaton,cmd53
** Func5: For BT use only currently
*/
enum SDIO_STD_FUNNUM {
	SDIO_FUNC0=0,
	SDIO_FUNC1,
	SDIO_FUNC2,
	SDIO_FUNC3,
	SDIO_FUNC4,
	SDIO_FUNC5,
	SDIO_FUNC6,
	SDIO_FUNC7,
};

#define FUNCNUM_SDIO_LAST SDIO_FUNC7
#define SDIO_FUNCNUM_MAX (FUNCNUM_SDIO_LAST+1)
#define OS_LOCK spinlock_t
#define RXFRAME_MAXLEN 4096

#define SDIO_MAXSG_SIZE	32
#define SDIO_MAX_SG_ENTRIES	(SDIO_MAXSG_SIZE+2)

#define FUNC4_BLKSIZE 512

struct amlw_hif_scatter_item {
    struct sk_buff *skbbuf;
    int len;
    int page_num;
    void *packet;
};

struct amlw_hif_scatter_req {
	/* address for the read/write operation */
	unsigned int addr;
	/* request flags */
	unsigned int req;
	/* total length of entire transfer */
	unsigned int len;

	void (*complete) (struct sk_buff *);

	bool free;
	int result;
	int scat_count;

	struct scatterlist sgentries[SDIO_MAX_SG_ENTRIES];
	struct amlw_hif_scatter_item scat_list[SDIO_MAX_SG_ENTRIES];
};

struct amlw1_hwif_sdio {
    struct sdio_func * sdio_func_if[SDIO_FUNCNUM_MAX];
    bool scatter_enabled;

    /* protects access to scat_req */
    OS_LOCK scat_lock;

    /* scatter request list head */
    struct amlw_hif_scatter_req *scat_req;
};

struct amlw1_hif_ops {
    int (*hi_bottom_write8)(unsigned char  func_num, int addr, unsigned char data);
    unsigned char (*hi_bottom_read8)(unsigned char  func_num, int addr);
    int (*hi_bottom_read)(unsigned char func_num, int addr, void *buf, size_t len, int incr_addr);
    int (*hi_bottom_write)(unsigned char func_num, int addr, void *buf, size_t len, int incr_addr);

    unsigned char (*hi_read8_func0)(unsigned long sram_addr);
    void (*hi_write8_func0)(unsigned long sram_addr, unsigned char sramdata);

    unsigned long (*hi_read_reg8)(unsigned long sram_addr);
    void (*hi_write_reg8)(unsigned long sram_addr, unsigned long sramdata);
    unsigned long (*hi_read_reg32)(unsigned long sram_addr);
    int (*hi_write_reg32)(unsigned long sram_addr, unsigned long sramdata);

    void (*hi_write_cmd)(unsigned long sram_addr, unsigned long sramdata);
    void (*hi_write_sram)(unsigned char*buf, unsigned char* addr, SYS_TYPE len);
    void (*hi_read_sram)(unsigned char* buf, unsigned char* addr, SYS_TYPE len);
    void (*hi_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*hi_read_word)(unsigned int addr);

    void (*hi_rcv_frame)(unsigned char* buf, unsigned char* addr, SYS_TYPE len);

    int (*hi_enable_scat)(void);
    void (*hi_cleanup_scat)(void);
    struct amlw_hif_scatter_req * (*hi_get_scatreq)(void);
    int (*hi_scat_rw)(struct scatterlist *sg_list, unsigned int sg_num, unsigned int blkcnt,
        unsigned char func_num, unsigned int addr, unsigned char write);
    int (*hi_send_frame)(struct amlw_hif_scatter_req *scat_req);

    /*bt use*/
    void (*bt_hi_write_sram)(unsigned char* buf, unsigned char* addr, SYS_TYPE len);
    void (*bt_hi_read_sram)(unsigned char* buf, unsigned char* addr, SYS_TYPE len);
    void (*bt_hi_write_word)(unsigned int addr,unsigned int data);
    unsigned int (*bt_hi_read_word)(unsigned int addr);

    void (*hif_get_sts)(unsigned int op_code, unsigned int ctrl_code);
    void (*hif_pt_rx_start)(unsigned int qos);
    void (*hif_pt_rx_stop)(void);

    int (*hif_suspend)(unsigned int suspend_enable);
};

void w1_aml_wifi_sdio_power_lock(void);
void w1_aml_wifi_sdio_power_unlock(void);
void *w1_aml_mem_prealloc(int section, unsigned long size);
