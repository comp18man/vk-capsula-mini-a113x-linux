#ifndef _HI_SDIO_H_
#define _HI_SDIO_H_

#include "wifi_sdio.h"

#define HIF_RD_MAC_REG_MAX (15)
#define HIF_RD_PHY_REG_MAX (10)


//irq
#define COEX_INFO_BT_LK_INFO          BEACON_RECV_OK_VID0
#define COEX_BT_EXIST_INF0              BEACON_RECV_OK_VID1


/*shijie.chen add, need add to register define file */
#define BCN_READABLE  BIT(31)
#define BCN_LEN_MASK  (BIT(17)|BIT(18)|BIT(19))

struct hif_reg_info{
      unsigned int addr;
      unsigned char* prt;
};

enum hif_suspend_state
{
    HIF_SUSPEND_STATE_NONE,
    HIF_SUSPEND_STATE_WOW,
    HIF_SUSPEND_STATE_DEEPSLEEP,
};

/*1 is  rtl support addr wrapper around, else 0 is not support. */
#define RTL_WRAPPER_AROUND_ADDR     1
/*sdio max block count when we use scatter/gather list.*/
#define SDIO_MAX_BLK_CNT    511
/*enable sdio update host rd/wr pointer by enable corresponding bit in
    SDIO MISC CTRL or SDIO MISC CTRL2 */
/* when 1, wrptr is updated by sdio rtl(func4). when 0, is updated by host via send cmd53 */
#define SDIO_UPDATE_HOST_WRPTR     0
/* when 1, rdptr is updated by sdio rtl(func6). when 0, is updated by host via send cmd53 */
#define SDIO_UPDATE_HOST_RDPTR      1

#define SG_NUM_MAX 16
#define SG_WRITE 1 //MMC_DATA_WRITE
#define SG_FRAME_MAX (1 * SG_NUM_MAX)
/* max page num while process a sdio write operation*/
#define SG_PAGE_MAX 80
#define FUNC4_BLKSIZE PAGE_LEN
#define FUNC6_BLKSIZE PAGE_LEN
#define BLKSIZE PAGE_LEN

#define HIF_READ        0x00000001
#define HIF_WRITE       0x00000002
#define HIF_SYNCHRONOUS  0x00000010
#define HIF_ASYNCHRONOUS 0x00000020

#define SDIO_MAXSG_SIZE	32
#define SDIO_MAX_SG_ENTRIES	(SDIO_MAXSG_SIZE+2)

#if defined (HAL_FPGA_VER)
struct amlw_hif_scatter_item {
	struct sk_buff * skbbuf;
	int len;
	int page_num;
	void * packet;
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
#endif

struct amlw_hif_ops {
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
#if defined (HAL_FPGA_VER)
    int (*hi_enable_scat)(void);
    void (*hi_cleanup_scat)(void);
    struct amlw_hif_scatter_req * (*hi_get_scatreq)(void);
    int (*hi_scat_rw)(struct scatterlist *sg_list, unsigned int sg_num, unsigned int blkcnt,
        unsigned char func_num, unsigned int addr, unsigned char write);
    int (*hi_send_frame)(struct amlw_hif_scatter_req *scat_req);
#elif defined (HAL_SIM_VER)
    void (*hi_send_frame)(unsigned char* buf, unsigned char* addr, SYS_TYPE len);
#endif

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

void hi_clear_irq_status(unsigned int data);
unsigned int hi_get_irq_status(void);
unsigned char hi_set_cmd(unsigned char *pdata,unsigned int len);
unsigned char hi_get_cmd(unsigned char *pdata,unsigned int len);

unsigned int hi_get_device_id(void);
unsigned int hi_get_vendor_id(void);
unsigned int hi_get_fw_version(void);

void hi_top_task( unsigned long data);
void hi_soft_tx_irq(void);


void hif_init_ops(void);

void hi_soft_rx_irq(struct hal_private *hal_priv, unsigned int rx_fw_ptr);
void hi_soft_rx_bcn(struct hal_private *hal_priv, unsigned char vid);

void hi_cfg_firmware(void);
void show_macframe (unsigned char *start, unsigned char len);
void set_wifi_baudrate (unsigned int apb_clk);

void hif_pt_rx_start(unsigned int qos);
void hif_pt_rx_stop(void);

#if defined (HAL_FPGA_VER)
struct tx_status_list {
    struct list_head status_list;
    struct list_head free_list;
    spinlock_t status_list_lock;
};

struct tx_status_node {
    struct list_head list_node;
    union
    {
        struct txdonestatus txstatus;
        struct tx_nulldata_status tx_null_status;
    }tx_status;
};
int hi_tx_frame(struct hi_tx_desc **pTxDPape, unsigned int mpdu_num, unsigned int page_num);
int hi_sdio_setup_scat_data(struct scatterlist *sg_list, struct hi_tx_desc **pTxDPape,
    unsigned int sg_num, unsigned char func_num);

/*txok status process */
int tx_status_list_init(struct tx_status_list *tx_status_list, int num);
void tx_status_node_free(struct tx_status_node *txok_status_node,
    struct tx_status_list *tx_status_list);
void tx_status_node_enqueue(struct tx_status_node *txok_status_node,
    struct tx_status_list *tx_status_list);
struct tx_status_node * tx_status_node_dequeue(struct tx_status_list *tx_status_list);
struct tx_status_node * tx_status_node_alloc(struct tx_status_list *tx_status_list);
void hi_irq_task(struct hal_private *hal_priv);
void hi_rx_fifo_init(void);

#elif defined (HAL_SIM_VER)
int hi_tx_frame( struct hi_tx_desc* pTxDPape, unsigned int blk_num, unsigned int page_num);
int hal_calc_block_in_mpdu (int mpdulen);
#endif 
unsigned int  MAC_RD_REG(unsigned int addr);
void MAC_WR_REG(unsigned int addr,unsigned int data);

void b2b_rx_throughput_calc(HW_RxDescripter_bit *RxPrivHdr);

#endif //_HI_SDIO_H_
