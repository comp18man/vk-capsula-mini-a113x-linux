#ifdef WIFI_MAC_TIMER_REG
#else
#define WIFI_MAC_TIMER_REG


#define WIFI_MAC_TIMER_REG_BASE                   (0xa00000)

#define MAC_COUNT1                                (WIFI_MAC_TIMER_REG_BASE + 0xb0)
// Bit 6   :0      reg_txsifsb                    U     RW        default = 'h2c
// Bit 14  :8      reg_rxsifsb                    U     RW        default = 'h1c
// Bit 15          reg_rxsifsb_sel                U     RW        default = 'h0
// Bit 22  :16     reg_ccasifs                    U     RW        default = 'h28
// Bit 28  :24     reg_txstartdel                 U     RW        default = 'h2
// Bit 31  :29     reg_rxendwait                  U     RW        default = 'h3
typedef union MAC_COUNT1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txsifsb : 7;
    unsigned int rsvd_0 : 1;
    unsigned int reg_rxsifsb : 7;
    unsigned int reg_rxsifsb_sel : 1;
    unsigned int reg_ccasifs : 7;
    unsigned int rsvd_1 : 1;
    unsigned int reg_txstartdel : 5;
    unsigned int reg_rxendwait : 3;
  } b;
} MAC_COUNT1_FIELD_T;

#define MAC_COUNT2                                (WIFI_MAC_TIMER_REG_BASE + 0xb4)
// Bit 6   :0      reg_txsifsa                    U     RW        default = 'h40
// Bit 14  :8      reg_rxsifsa                    U     RW        default = 'h24
// Bit 15          reg_rxsifsa_sel                U     RW        default = 'h0
// Bit 21  :16     reg_slot                       U     RW        default = 'h9
// Bit 28  :24     reg_fiac_fw_maxdel             U     RW        default = 'hf
typedef union MAC_COUNT2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_txsifsa : 7;
    unsigned int rsvd_0 : 1;
    unsigned int reg_rxsifsa : 7;
    unsigned int reg_rxsifsa_sel : 1;
    unsigned int reg_slot : 6;
    unsigned int rsvd_1 : 2;
    unsigned int reg_fiac_fw_maxdel : 5;
    unsigned int rsvd_2 : 3;
  } b;
} MAC_COUNT2_FIELD_T;

#define MAC_COUNT3                                (WIFI_MAC_TIMER_REG_BASE + 0xb8)
// Bit 8   :0      reg_backoff_hold               U     RW        default = 'h0
// Bit 12          reg_backoff_all_hold           U     RW        default = 'h0
// Bit 17  :16     reg_txop_hold_status           U     RW        default = 'h0
typedef union MAC_COUNT3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_backoff_hold : 9;
    unsigned int rsvd_0 : 3;
    unsigned int reg_backoff_all_hold : 1;
    unsigned int rsvd_1 : 3;
    unsigned int reg_txop_hold_status : 2;
    unsigned int rsvd_2 : 14;
  } b;
} MAC_COUNT3_FIELD_T;

#define MAC_BACKOFF_CONTROL                       (WIFI_MAC_TIMER_REG_BASE + 0xbc)
// Bit 7   :0      reg_slotn_s                    U     RW        default = 'h1
// Bit 11          reg_sifsn_s                    U     RW        default = 'h1
// Bit 15          reg_backoff_way                U     RW        default = 'h1
// Bit 16          reg_ackto_way                  U     RW        default = 'h0
typedef union MAC_BACKOFF_CONTROL_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slotn_s : 8;
    unsigned int rsvd_0 : 3;
    unsigned int reg_sifsn_s : 1;
    unsigned int rsvd_1 : 3;
    unsigned int reg_backoff_way : 1;
    unsigned int reg_ackto_way : 1;
    unsigned int rsvd_2 : 15;
  } b;
} MAC_BACKOFF_CONTROL_FIELD_T;

#define MAC_ABSCNT0                               (WIFI_MAC_TIMER_REG_BASE + 0xc0)
// Bit 25  :0      reg_abscnt0                    U     RW        default = 'h0
typedef union MAC_ABSCNT0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt0 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT0_FIELD_T;

#define MAC_ABSCNT1                               (WIFI_MAC_TIMER_REG_BASE + 0xc4)
// Bit 25  :0      reg_abscnt1                    U     RW        default = 'h0
typedef union MAC_ABSCNT1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt1 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT1_FIELD_T;

#define MAC_ABSCNT2                               (WIFI_MAC_TIMER_REG_BASE + 0xc8)
// Bit 25  :0      reg_abscnt2                    U     RW        default = 'h0
typedef union MAC_ABSCNT2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt2 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT2_FIELD_T;

#define MAC_ABSCNT3                               (WIFI_MAC_TIMER_REG_BASE + 0xcc)
// Bit 25  :0      reg_abscnt3                    U     RW        default = 'h0
typedef union MAC_ABSCNT3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt3 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT3_FIELD_T;

#define MAC_ABSCNT4                               (WIFI_MAC_TIMER_REG_BASE + 0xd0)
// Bit 25  :0      reg_abscnt4                    U     RW        default = 'h0
typedef union MAC_ABSCNT4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt4 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT4_FIELD_T;

#define MAC_ABSCNT5                               (WIFI_MAC_TIMER_REG_BASE + 0xd4)
// Bit 25  :0      reg_abscnt5                    U     RW        default = 'h0
typedef union MAC_ABSCNT5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt5 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT5_FIELD_T;

#define MAC_ABSCNT6                               (WIFI_MAC_TIMER_REG_BASE + 0xd8)
// Bit 25  :0      reg_abscnt6                    U     RW        default = 'h0
typedef union MAC_ABSCNT6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt6 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT6_FIELD_T;

#define MAC_ABSCNT7                               (WIFI_MAC_TIMER_REG_BASE + 0xdc)
// Bit 25  :0      reg_abscnt7                    U     RW        default = 'h0
typedef union MAC_ABSCNT7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt7 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT7_FIELD_T;

#define MAC_ABSCNT8                               (WIFI_MAC_TIMER_REG_BASE + 0xe0)
// Bit 25  :0      reg_abscnt8                    U     RW        default = 'h0
typedef union MAC_ABSCNT8_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt8 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT8_FIELD_T;

#define MAC_ABSCNT9                               (WIFI_MAC_TIMER_REG_BASE + 0xe4)
// Bit 25  :0      reg_abscnt9                    U     RW        default = 'h0
typedef union MAC_ABSCNT9_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt9 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT9_FIELD_T;

#define MAC_ABSCNT10                              (WIFI_MAC_TIMER_REG_BASE + 0xe8)
// Bit 25  :0      reg_abscnt10                   U     RW        default = 'h0
typedef union MAC_ABSCNT10_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt10 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT10_FIELD_T;

#define MAC_ABSCNT11                              (WIFI_MAC_TIMER_REG_BASE + 0xec)
// Bit 25  :0      reg_abscnt11                   U     RW        default = 'h0
typedef union MAC_ABSCNT11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt11 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT11_FIELD_T;

#ifndef PROJECT_T9026
#define MAC_ABSCNT12                              (WIFI_MAC_TIMER_REG_BASE + 0x1d0)
// Bit 25  :0      reg_abscnt12                   U     RW        default = 'h0
typedef union MAC_ABSCNT12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt12 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT12_FIELD_T;

#define MAC_ABSCNT13                              (WIFI_MAC_TIMER_REG_BASE + 0x1d4)
// Bit 25  :0      reg_abscnt13                   U     RW        default = 'h0
typedef union MAC_ABSCNT13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt13 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT13_FIELD_T;

#define MAC_ABSCNT14                              (WIFI_MAC_TIMER_REG_BASE + 0x1d8)
// Bit 25  :0      reg_abscnt14                   U     RW        default = 'h0
typedef union MAC_ABSCNT14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt14 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT14_FIELD_T;

#define MAC_ABSCNT15                              (WIFI_MAC_TIMER_REG_BASE + 0x1dc)
// Bit 25  :0      reg_abscnt15                   U     RW        default = 'h0
typedef union MAC_ABSCNT15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_abscnt15 : 26;
    unsigned int rsvd_0 : 6;
  } b;
} MAC_ABSCNT15_FIELD_T;
#endif

#define TIMER_CONFIG                              (WIFI_MAC_TIMER_REG_BASE + 0xfc)
// Bit 3   :0      reg_tx_priority_ctrl           U     RW        default = 'hc
// Bit 9   :4      reg_tx_duration_ctrl           U     RW        default = 'h0
// Bit 10          reg_tx_request_mode            U     RW        default = 'h1
// Bit 11          reg_tx_enable_clr_sel          U     RW        default = 'h0
// Bit 15  :12     reg_tx_start_cnt_thr           U     RW        default = 'h4
// Bit 31  :16     reg_vcs_thr                    U     RW        default = 'h1000
typedef union TIMER_CONFIG_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_tx_priority_ctrl : 4;
    unsigned int reg_tx_duration_ctrl : 6;
    unsigned int reg_tx_request_mode : 1;
    unsigned int reg_tx_enable_clr_sel : 1;
    unsigned int reg_tx_start_cnt_thr : 4;
    unsigned int reg_vcs_thr : 16;
  } b;
} TIMER_CONFIG_FIELD_T;

#define MAC_TXCONTROL_NONQOS                      (WIFI_MAC_TIMER_REG_BASE + 0x100)
// Bit 9   :0      reg_slot_num_o_nonqos          U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_nonqos         U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_nonqos               U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_nonqos       U     RW        default = 'h0
// Bit 24          reg_tx_enable_nonqos           U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_nonqos        U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_nonqos         U     RW        default = 'h0
typedef union MAC_TXCONTROL_NONQOS_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_nonqos : 10;
    unsigned int reg_tx_duration_nonqos : 6;
    unsigned int reg_aifsn_nonqos : 4;
    unsigned int reg_cca_busy_mask_nonqos : 4;
    unsigned int reg_tx_enable_nonqos : 1;
    unsigned int reg_tx_bandwidth_nonqos : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_nonqos : 4;
  } b;
} MAC_TXCONTROL_NONQOS_FIELD_T;

#define MAC_TXCONTROL_ACP0                        (WIFI_MAC_TIMER_REG_BASE + 0x104)
// Bit 9   :0      reg_slot_num_o_acp0            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp0           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp0                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp0         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp0             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp0          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp0           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp0 : 10;
    unsigned int reg_tx_duration_acp0 : 6;
    unsigned int reg_aifsn_acp0 : 4;
    unsigned int reg_cca_busy_mask_acp0 : 4;
    unsigned int reg_tx_enable_acp0 : 1;
    unsigned int reg_tx_bandwidth_acp0 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp0 : 4;
  } b;
} MAC_TXCONTROL_ACP0_FIELD_T;

#define MAC_TXCONTROL_ACP1                        (WIFI_MAC_TIMER_REG_BASE + 0x108)
// Bit 9   :0      reg_slot_num_o_acp1            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp1           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp1                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp1         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp1             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp1          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp1           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp1 : 10;
    unsigned int reg_tx_duration_acp1 : 6;
    unsigned int reg_aifsn_acp1 : 4;
    unsigned int reg_cca_busy_mask_acp1 : 4;
    unsigned int reg_tx_enable_acp1 : 1;
    unsigned int reg_tx_bandwidth_acp1 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp1 : 4;
  } b;
} MAC_TXCONTROL_ACP1_FIELD_T;

#define MAC_TXCONTROL_ACP2                        (WIFI_MAC_TIMER_REG_BASE + 0x10c)
// Bit 9   :0      reg_slot_num_o_acp2            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp2           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp2                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp2         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp2             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp2          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp2           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp2 : 10;
    unsigned int reg_tx_duration_acp2 : 6;
    unsigned int reg_aifsn_acp2 : 4;
    unsigned int reg_cca_busy_mask_acp2 : 4;
    unsigned int reg_tx_enable_acp2 : 1;
    unsigned int reg_tx_bandwidth_acp2 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp2 : 4;
  } b;
} MAC_TXCONTROL_ACP2_FIELD_T;

#define MAC_TXCONTROL_ACP3                        (WIFI_MAC_TIMER_REG_BASE + 0x110)
// Bit 9   :0      reg_slot_num_o_acp3            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp3           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp3                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp3         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp3             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp3          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp3           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp3 : 10;
    unsigned int reg_tx_duration_acp3 : 6;
    unsigned int reg_aifsn_acp3 : 4;
    unsigned int reg_cca_busy_mask_acp3 : 4;
    unsigned int reg_tx_enable_acp3 : 1;
    unsigned int reg_tx_bandwidth_acp3 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp3 : 4;
  } b;
} MAC_TXCONTROL_ACP3_FIELD_T;

#define MAC_TXCONTROL_ACP4                        (WIFI_MAC_TIMER_REG_BASE + 0x114)
// Bit 9   :0      reg_slot_num_o_acp4            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp4           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp4                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp4         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp4             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp4          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp4           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp4 : 10;
    unsigned int reg_tx_duration_acp4 : 6;
    unsigned int reg_aifsn_acp4 : 4;
    unsigned int reg_cca_busy_mask_acp4 : 4;
    unsigned int reg_tx_enable_acp4 : 1;
    unsigned int reg_tx_bandwidth_acp4 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp4 : 4;
  } b;
} MAC_TXCONTROL_ACP4_FIELD_T;

#define MAC_TXCONTROL_ACP5                        (WIFI_MAC_TIMER_REG_BASE + 0x118)
// Bit 9   :0      reg_slot_num_o_acp5            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp5           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp5                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp5         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp5             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp5          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp5           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp5 : 10;
    unsigned int reg_tx_duration_acp5 : 6;
    unsigned int reg_aifsn_acp5 : 4;
    unsigned int reg_cca_busy_mask_acp5 : 4;
    unsigned int reg_tx_enable_acp5 : 1;
    unsigned int reg_tx_bandwidth_acp5 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp5 : 4;
  } b;
} MAC_TXCONTROL_ACP5_FIELD_T;

#define MAC_TXCONTROL_ACP6                        (WIFI_MAC_TIMER_REG_BASE + 0x11c)
// Bit 9   :0      reg_slot_num_o_acp6            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp6           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp6                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp6         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp6             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp6          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp6           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP6_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp6 : 10;
    unsigned int reg_tx_duration_acp6 : 6;
    unsigned int reg_aifsn_acp6 : 4;
    unsigned int reg_cca_busy_mask_acp6 : 4;
    unsigned int reg_tx_enable_acp6 : 1;
    unsigned int reg_tx_bandwidth_acp6 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp6 : 4;
  } b;
} MAC_TXCONTROL_ACP6_FIELD_T;

#define MAC_TXCONTROL_ACP7                        (WIFI_MAC_TIMER_REG_BASE + 0x120)
// Bit 9   :0      reg_slot_num_o_acp7            U     RW        default = 'h0
// Bit 15  :10     reg_tx_duration_acp7           U     RW        default = 'h8
// Bit 19  :16     reg_aifsn_acp7                 U     RW        default = 'h0
// Bit 23  :20     reg_cca_busy_mask_acp7         U     RW        default = 'h0
// Bit 24          reg_tx_enable_acp7             U     RW        default = 'h0
// Bit 26  :25     reg_tx_bandwidth_acp7          U     RW        default = 'h0
// Bit 31  :28     reg_tx_priority_acp7           U     RW        default = 'h0
typedef union MAC_TXCONTROL_ACP7_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_slot_num_o_acp7 : 10;
    unsigned int reg_tx_duration_acp7 : 6;
    unsigned int reg_aifsn_acp7 : 4;
    unsigned int reg_cca_busy_mask_acp7 : 4;
    unsigned int reg_tx_enable_acp7 : 1;
    unsigned int reg_tx_bandwidth_acp7 : 2;
    unsigned int rsvd_0 : 1;
    unsigned int reg_tx_priority_acp7 : 4;
  } b;
} MAC_TXCONTROL_ACP7_FIELD_T;

#define MAC_VCSUPDATE                             (WIFI_MAC_TIMER_REG_BASE + 0x128)
// Bit 25  :0      reg_vcs_time                   U     RW        default = 'h0
// Bit 26          reg_vcs_enb                    U     RW        default = 'h0
// Bit 28  :27     reg_vcs_source                 U     RW        default = 'h0
// Bit 30          ro_vcs_enable                  U     RO        default = 'h0
// Bit 31          reg_vcs_up_enb                 U     RW        default = 'h0
typedef union MAC_VCSUPDATE_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_vcs_time : 26;
    unsigned int reg_vcs_enb : 1;
    unsigned int reg_vcs_source : 2;
    unsigned int rsvd_0 : 1;
    unsigned int ro_vcs_enable : 1;
    unsigned int reg_vcs_up_enb : 1;
  } b;
} MAC_VCSUPDATE_FIELD_T;

#define MAC_TIME_HIGH                             (WIFI_MAC_TIMER_REG_BASE + 0x12c)
// Bit 31  :0      reg_mac_time_high              U     RO        default = 'h0
typedef union MAC_TIME_HIGH_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_time_high : 32;
  } b;
} MAC_TIME_HIGH_FIELD_T;

#define MAC_TIME_LOW                              (WIFI_MAC_TIMER_REG_BASE + 0x130)
// Bit 31  :0      reg_mac_time_low               U     RO        default = 'h0
typedef union MAC_TIME_LOW_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_mac_time_low : 32;
  } b;
} MAC_TIME_LOW_FIELD_T;

#endif

