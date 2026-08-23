# SPDX-License-Identifier: GPL-2.0
#
# Amlogic W1 (W155S1 / AW3155) SDIO Wi-Fi, from the CoreELEC w1-aml tree.
# Objects listed rather than globbed, feature switches as -D rather than
# Kconfig symbols so they cannot collide with the kernel namespace.
#
# Dropped: aml_static_buf.o wants linux/amlogic/wlan_plat.h from the vendor
# BSP; print_version.o was generated at build time and would define
# print_driver_version() a second time.

# Order matters. Built-in, both module_inits become device_initcalls run in
# link order; vlsicomm expects w1_w1_sdio_driver_insmoded already set, and
# link it first and you get "Driver 'aml_w1_sdio' is already registered".
obj-$(CONFIG_W1_AML) += aml_sdio.o
obj-$(CONFIG_W1_AML) += vlsicomm.o

aml_sdio-y := vmac/w1_sdio/w1_sdio.o

vlsicomm-y += vmac/aml_compat.o
vlsicomm-y += vmac/rc80211_minstrel.o
vlsicomm-y += vmac/rc80211_minstrel_ht.o
vlsicomm-y += vmac/rc80211_minstrel_init.o
vlsicomm-y += vmac/wifi_android.o
vlsicomm-y += vmac/wifi_cfg80211.o
vlsicomm-y += vmac/wifi_clock_measure.o
vlsicomm-y += vmac/wifi_cmd_func.o
vlsicomm-y += vmac/wifi_common.o
vlsicomm-y += vmac/wifi_debug.o
vlsicomm-y += vmac/wifi_drv_capture.o
vlsicomm-y += vmac/wifi_drv_config.o
vlsicomm-y += vmac/wifi_drv_if.o
vlsicomm-y += vmac/wifi_drv_main.o
vlsicomm-y += vmac/wifi_drv_power.o
vlsicomm-y += vmac/wifi_drv_recv.o
vlsicomm-y += vmac/wifi_drv_reg_ops.o
vlsicomm-y += vmac/wifi_drv_statistic.o
vlsicomm-y += vmac/wifi_drv_timer.o
vlsicomm-y += vmac/wifi_drv_uapsd.o
vlsicomm-y += vmac/wifi_drv_xmit.o
vlsicomm-y += vmac/wifi_hal.o
vlsicomm-y += vmac/wifi_hal_cmd.o
vlsicomm-y += vmac/wifi_hal_platform.o
vlsicomm-y += vmac/wifi_hal_txdesc.o
vlsicomm-y += vmac/wifi_hif.o
vlsicomm-y += vmac/wifi_iwpriv_cmd.o
vlsicomm-y += vmac/wifi_mac_acl.o
vlsicomm-y += vmac/wifi_mac_action.o
vlsicomm-y += vmac/wifi_mac_amsdu.o
vlsicomm-y += vmac/wifi_mac_beacon.o
vlsicomm-y += vmac/wifi_mac_chan.o
vlsicomm-y += vmac/wifi_mac_concurrent.o
vlsicomm-y += vmac/wifi_mac_encrypt.o
vlsicomm-y += vmac/wifi_mac_if.o
vlsicomm-y += vmac/wifi_mac_monitor.o
vlsicomm-y += vmac/wifi_mac_p2p.o
vlsicomm-y += vmac/wifi_mac_pmf.o
vlsicomm-y += vmac/wifi_mac_power.o
vlsicomm-y += vmac/wifi_mac_rate.o
vlsicomm-y += vmac/wifi_mac_recv.o
vlsicomm-y += vmac/wifi_mac_sae.o
vlsicomm-y += vmac/wifi_mac_scan.o
vlsicomm-y += vmac/wifi_mac_sta.o
vlsicomm-y += vmac/wifi_mac_xmit.o
vlsicomm-y += vmac/wifi_phy.o
vlsicomm-y += vmac/wifi_pkt_desc.o
vlsicomm-y += vmac/wifi_pt_if.o
vlsicomm-y += vmac/wifi_pt_init.o
vlsicomm-y += vmac/wifi_pt_network.o
vlsicomm-y += vmac/wifi_rate_ctrl.o
vlsicomm-y += vmac/wifi_sdio.o
vlsicomm-y += vmac/wifi_share_fifo.o
vlsicomm-y += vmac/wifi_skbbuf.o
vlsicomm-y += vmac/crypto/aes-ccm.o
vlsicomm-y += vmac/crypto/aes-ctr.o
vlsicomm-y += vmac/crypto/aes-gcm.o
vlsicomm-y += vmac/crypto/aes-internal.o
vlsicomm-y += vmac/crypto/aes-internal-enc.o
vlsicomm-y += vmac/crypto/aes-omac1.o
vlsicomm-y += vmac/crypto/aes-siv.o
vlsicomm-y += vmac/crypto/aml_crypto_wrap.o
vlsicomm-y += vmac/crypto/ccmp.o
vlsicomm-y += vmac/crypto/sha256.o
vlsicomm-y += vmac/crypto/sha256-internal.o
vlsicomm-y += vmac/crypto/sha256-prf.o

ccflags-y += -I$(src)/vmac
ccflags-y += -I$(src)/common
ccflags-y += -I$(src)/vmac/compat_include
ccflags-y += -include $(src)/vmac/aml_compat.h

# SDIO, cfg80211 with P2P, RF calibration from firmware, capture.
# MAC_SUPPORT stays off, and cannot be turned on: wifi_get_mac() is only
# declared and WIFIMAC_PATH is not defined at all. It covers the efuse read as
# well as the file one, so that half is lifted out of the ifdef instead.
ccflags-y += -DSYSTEM64 -DPROJECT_W1 -DHAL_FPGA_VER
ccflags-y += -DCONFIG_SDIO_IF -DFPGA -DSDIO_BUILD_IN -DUSE_T902X_RF
ccflags-y += -DCONFIG_AML_CFG80211 -DCONFIG_P2P -DCONFIG_WFD
ccflags-y += -DWITH_RF -DRF_T9026 -DFW_RF_CALIBRATION
ccflags-y += -DCONFIG_WAPI -DAML_WPA3 -DCONFIG_CONCURRENT_MODE -DSRAM_CONCURRENT
ccflags-y += -DINTER_BEACON -DDYNAMIC_BW -DPHY_TEST_FUNC -DCTS_VERIFIER_GAS
ccflags-y += -DWIFI_CAPTURE

# Both must be defined, not just left out: the sources select with
# "#if (USE_SDIO_IRQ==1) ... #elif (USE_GPIO_IRQ==1)", so undefined compiles
# neither branch and the driver loads deaf. This board is wired for in-band.
ccflags-y += -DUSE_SDIO_IRQ=1 -DUSE_GPIO_IRQ=0

# Same shape: opt_all.h defines the WAKE_LOCK macros only inside
# "#if defined(CONFIG_HAS_WAKELOCK)", in both branches, so leaving it out
# turns the call sites into undefined symbols at link time. Safe here, the
# parts that need Android's API are also gated on LINUX_VERSION <= 3.14.29.
ccflags-y += -DCONFIG_HAS_WAKELOCK

# NOT_AMLOGIC_PLATFORM would kill the pr_debug flood (opt_all.h maps
# pr_debug to pr_info, ~1000 sites) but also switches code paths the
# reference port never exercised. Quieten from the command line instead.
# ccflags-y += -DNOT_AMLOGIC_PLATFORM

# What the vendor Makefile silenced. Cheaper than patching 66 files.
ccflags-y += -Wno-enum-conversion -Wno-strict-prototypes -Wno-int-conversion
ccflags-y += -Wno-implicit-function-declaration -Wno-misleading-indentation
ccflags-y += -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-function
ccflags-y += -Wno-unused-variable -Wno-format

# gcc 14 made return-mismatch, int-conversion, incompatible-pointer-types
# and implicit-function-declaration errors by default; this code predates
# that. Real breakage still stops the build - wrong argument counts and
# duplicate storage classes are hard errors -Wno-error does not touch.
ccflags-y += -Wno-error

# Enormous stack frames. The vendor raised this via KBUILD_CFLAGS_MODULE,
# which does nothing for a built-in object.
ccflags-remove-y += -Wframe-larger-than=$(CONFIG_FRAME_WARN)
ccflags-y += -Wframe-larger-than=8192
