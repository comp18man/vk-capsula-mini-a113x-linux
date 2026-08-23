#ifndef __PATCH_FI_CMD__
#define __PATCH_FI_CMD__

#include "fi_sdio.h"
#include "fi_cmd.h"

#define CHANNEL_SWITCH_CMD  0x25
#define DPD_MEMORY_CMD  0x26
#define FWLOG_MODE_CMD  0x27
#define CALI_PARAM_CMD  0x28
#define PHY_INTERFACE_CMD  0x29
#define WF2G_TXPWR_PARAM_CMD  0x2b
#define WF5G_TXPWR_PARAM_CMD  0x2c
#define TXPWR_CFFC_CFG_CMD 0x2d

#define TSSI_5G_CAL_NUM 4

enum ver_id_enum
{
    VERSION_DEFAULT = 0,
    VERSION_GVA = 1,
    VERSION_GVA_MRT = 2,
};

enum patch_fw_event_type
{
    CHANNEL_SWITCH_EVENT = (TBTT_EVENT + 1),
    DPD_CALIBRATION_EVENT,
    TX_ERROR_EVENT,
    FWLOG_PRINT_EVENT,
    COEX_EVENT,
};

/* channel switch flag */
enum {
    CHANNEL_RESTORE_FLAG = BIT(0),
    CHANNEL_CONNECT_FLAG = BIT(1),
    CHANNEL_NO_EVENT_FLAG = BIT(2),
    CHANNEL_RSSI_FLAG = BIT(3),
    CHANNEL_RSSI_PWR_FLAG = BIT(4),
};

typedef struct Channel_Switch
{
    unsigned char Cmd;
    unsigned char vid;
    unsigned char flag;
    unsigned char bw;
    unsigned char rssi;
    unsigned char res[3];
    unsigned int channel;
    /* center freq value */
    unsigned int pri_chan;
}Channel_Switch;

typedef struct DPD_Memory_Download
{
    unsigned char Cmd;
    unsigned char vid;
    unsigned char reserved[6];
} DPD_Memory_Download;

typedef struct Fwlog_Mode_Control
{
    unsigned char Cmd;
    unsigned char mode; //0x00:fw  0x01:host
    unsigned char reserved[6];
} Fwlog_Mode_Control;

// event
typedef struct channel_switch_event
{
    struct fw_event_basic_info basic_info;
    unsigned int channel;
    unsigned int bw;
    unsigned int done;
} channel_switch_event;

typedef struct tx_error_event
{
    struct fw_event_basic_info basic_info;
    unsigned int error_type;
    unsigned int frame_type;
} tx_error_event;

typedef struct txtssi_ratio_limit_param
{
    unsigned char enable;
    unsigned char wf2g_ratio_limit;
    unsigned char wf5g_ratio_limit;
    unsigned char reserved;
} txtssi_ratio_limit_param;

typedef struct digital_gain_limit_param
{
    unsigned char enable;
    unsigned char min_2g;
    unsigned char max_2g;
    unsigned char min_5g;
    unsigned char max_5g;
    unsigned char resv[3];
} digital_gain_limit_param;

typedef struct Cali_Param
{
    unsigned char Cmd;
    unsigned int version;
    unsigned short cali_config;
    unsigned char freq_offset;
    unsigned char htemp_freq_offset;
    unsigned char cca_ed_det;
    unsigned char tssi_2g_offset;
    unsigned char wf2g_spur_rmen;
    unsigned char tssi_5g_offset[TSSI_5G_CAL_NUM];
    unsigned char rf_num;
    unsigned char wftx_pwrtbl_en;
    unsigned char resv[1];
    txtssi_ratio_limit_param txtssi_ratio_limit;
    digital_gain_limit_param digital_gain_limit;
    unsigned short spur_freq;
    unsigned short platform_versionid;
} Cali_Param;

typedef struct WF2G_Txpwr_Param
{
    unsigned char Cmd;
    unsigned char reserved[3];
    unsigned char wf2g_pwr_tbl[2][16];
} WF2G_Txpwr_Param;

typedef struct WF5G_Txpwr_Param
{
    unsigned char Cmd;
    unsigned char reserved[3];
    unsigned char wf5g_pwr_tbl[3][16];
} WF5G_Txpwr_Param;

typedef struct Phy_Interface_Param
{
    unsigned char Cmd;
    unsigned char vid;
    unsigned char interface_enable;
    unsigned char reserve[5];
} Phy_Interface_Param;

typedef struct fwlog_print_event
{
    struct fw_event_basic_info basic_info;
} fwlog_print_event;

typedef struct Txpwr_Cffc_Cfg_Param
{
    unsigned char Cmd;
    unsigned char coefficient[57];
    unsigned char reserve[2];
    unsigned char band[4];
} Txpwr_Cffc_Cfg_Param;

#endif
