#ifndef AML_SDIO_H
#define AML_SDIO_H

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

#define FUNCNUM_SDIO_LAST   	SDIO_FUNC7
#define SDIO_FUNCNUM_MAX 	        (FUNCNUM_SDIO_LAST+1)

#define SDIO_OPMODE_INCREMENT	     1
#define SDIO_OPMODE_FIXED             0

#define CMD53_ARG_BLOCK_BASIS   1

#define SDIO_WRITE			1
#define SDIO_READ			0

#define SDIO_DMA_BUFFER_SIZE    1024*8
#define SDIO_CCCR_IEN 		4
#define SDIO_CCCR_IOABORT 	6
#define SDIO_ADDR_MASK 		(128*1024-1)

enum SDIO_RW_FLAG
{
    SDIO_F_SYNCHRONOUS= BIT(0),
    SDIO_F_ASYNCHRONOUS= BIT(1),
    SDIO_F_USEDMA= BIT(2),
    SDIO_F_DIRECT= BIT(3),
};

struct hal_thr_thread_test
{
    int threadtest_loop;

    struct task_struct* work_thread;
    struct semaphore work_thread_sem;
    int work_thread_quit;
    struct completion work_thread_completion; /* thread completion */

    struct task_struct* sdio_thread;
    struct semaphore sdio_thread_sem;
    int sdio_thread_quit;
    struct completion   sdio_thread_completion; /* thread completion */
};

void Init_B2B_Resource(void);

struct amlw_hwif_sdio {
    struct sdio_func * sdio_func_if[SDIO_FUNCNUM_MAX];
    bool scatter_enabled;

    /* protects access to scat_req */
    OS_LOCK scat_lock;

    /* scatter request list head */
    struct amlw_hif_scatter_req *scat_req;
};

/* Common msglevel constants */
#define SDH_ERROR_VAL		0x0001	/* Error */
#define SDH_TRACE_VAL		0x0002	/* Trace */
#define SDH_INFO_VAL		0x0004	/* Info */
#define SDH_DEBUG_VAL		0x0008	/* Debug */
#define SDH_DATA_VAL		0x0010	/* Data */
#define SDH_CTRL_VAL		0x0020	/* Control Regs */
#define SDH_LOG_VAL		0x0040	/* Enable log */
#define SDH_DMA_VAL		0x0080	/* DMA */
#define SDH_SGLIST_VAL		0x0100	/* SgList */

#define SDIOH_API_RC_SUCCESS            (0x00)
#define SDIOH_API_RC_FAIL	               (0x01)
#define SDIOH_API_SUCCESS(status)     (status == 0)


int aml_sdio_bottom_write8(unsigned char  func_num, int addr, unsigned char data);
unsigned char aml_sdio_bottom_read8(unsigned char  func_num, int addr);
int aml_sdio_bottom_read(unsigned char  func_num, int addr, void *buf, size_t len, int incr_addr);
int aml_sdio_bottom_write(unsigned char  func_num, int addr, void *buf, size_t len, int incr_addr);

void aml_sdio_write_word(unsigned int addr,unsigned int data);
unsigned int aml_sdio_read_word(unsigned int addr);
void aml_sdio_read_sram (unsigned char *buf,unsigned char *addr,SYS_TYPE len);
void aml_sdio_write_sram (unsigned char *buf,unsigned char *addr,SYS_TYPE len);
unsigned char  aml_sdio_bottom_read8_func0(unsigned long sram_addr);
void aml_sdio_bottom_write8_func0(unsigned long sram_addr, unsigned char sramdata);
void aml_sdio_recv_frame (unsigned char *buf, unsigned char *addr, SYS_TYPE len);
void aml_sdio_send_frame (unsigned char *buf, unsigned char *addr, SYS_TYPE len);
unsigned long aml_sdio_read_reg8(unsigned long sram_addr );
void  aml_sdio_write_reg8(unsigned long sram_addr,unsigned long sramdata);
unsigned long aml_sdio_read_reg32(unsigned long sram_addr);
int  aml_sdio_write_reg32(unsigned long sram_addr,unsigned long sramdata);
void  aml_sdio_write_cmd32(unsigned long sram_addr,unsigned long sramdata);
/*For BT use only start */
void aml_bt_sdio_read_sram (unsigned char *buf,unsigned char *addr,SYS_TYPE len);
void aml_bt_sdio_write_sram (unsigned char *buf,unsigned char *addr,SYS_TYPE len);
void aml_bt_hi_write_word(unsigned int addr,unsigned int data);
unsigned int aml_bt_hi_read_word(unsigned int addr);

void aml_sdio_disable_irq(int func_n);
void aml_sdio_enable_irq(int func_n);
void aml_sdio_wake_up_int(void);

void set_reg_fragment(unsigned int addr, unsigned int bit_end,
        unsigned int bit_start,unsigned int value);
int aml_sdio_init(void);
void  aml_sdio_exit(void);
int sdio_call_task( void );
void aml_enable_wifi(void);
void aml_disable_wifi(void);


void aml_aon_write_reg(unsigned int addr,unsigned int data);
unsigned int aml_aon_read_reg(unsigned int addr);

void hif_get_sts(unsigned int op_code, unsigned int ctrl_code);
int aml_sdio_suspend(unsigned int suspend_enable);

int aml_sdio_scat_req_rw(struct amlw_hif_scatter_req *scat_req);
int aml_sdio_enable_scatter(void);
void aml_sdio_cleanup_scatter(void);
void aml_sdio_scatter_req_add(struct amlw_hif_scatter_req *scat_req);
struct amlw_hif_scatter_req *aml_sdio_scatter_req_get(void);
int aml_sdio_scat_rw(struct scatterlist *sg_list, unsigned int sg_num, unsigned int blkcnt,
        unsigned char func_num, unsigned int addr, unsigned char write);

#if defined (HAL_FPGA_VER)

#include <asm/io.h>
#include <asm/uaccess.h>
extern void amlwifi_set_sdio_host_clk(int clk);
extern void sdio_reinit(void);
void aml_sdio_reset(void);
void aml_sdio_irq_path(unsigned char b_gpio);
#if (USE_GPIO_IRQ)
int amlhal_gpio_open(struct hal_private * hal_priv);
int amlhal_gpio_close(struct hal_private * hal_priv );
#endif

#elif defined (HAL_SIM_VER)

enum sdio_sim_rw_flag{
        SDIO_RW_FLAG_READ=0,
        SDIO_RW_FLAG_WRITE,
};

unsigned short aml_sdio_read_write(struct  hw_interface * hif, SYS_TYPE addr,
    unsigned char *buf, SYS_TYPE len, unsigned char  func, unsigned char rw_flag,unsigned char  flag);
int _sdio_read_write(struct hw_interface * hif, struct sdio_rw_desc * pDesc);
int sdio_read_data(int function_no, int buf_or_fifo, unsigned long sdio_addr,
    unsigned long byte_count, unsigned char *data_buf);
int sdio_write_data(int function_no, int buf_or_fifo, unsigned long sdio_addr,
    unsigned long byte_count, unsigned char *data_buf);
int sdio_read_reg(int function_no, unsigned long sdio_register,
    unsigned char *reg_data);
int sdio_write_reg(int function_no, unsigned long sdio_register,
    unsigned char *reg_data, unsigned read_after_write_flag);
extern void sdio_read_write(int idx, int addr, svBitPackedArrRef buff, int len, int func);
#endif

#ifdef SDIO_BUILD_IN
    extern struct amlw_hif_ops w1_g_w1_hif_ops;
    extern struct amlw_hwif_sdio w1_g_w1_hwif_sdio;
    extern unsigned char (*w1_host_wake_w1_req)(void);
    extern int (*w1_host_suspend_req)(struct device* device);
    extern int (*w1_host_resume_req)(struct device* device);
    #define AML_W1_WIFI_POWER_ON 1
    #define AML_W1_WIFI_POWER_OFF 0

    #define BT_POWER_CHANGE_SHIFT 0
    #define WIFI_POWER_CHANGE_SHIFT 1

    int aml_sdio_pm_suspend(struct device *device);
    int aml_sdio_pm_resume(struct device *device);
#endif

    struct sdio_func *aml_priv_to_func(int func_n);

#endif //AML_SDIO_H
