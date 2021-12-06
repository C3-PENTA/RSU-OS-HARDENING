# top level project rules for the tcc893x_evm project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := tcc893x_evm

MODULES += app/aboot

DEBUG := 1
#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
#DEFINES += CRYPTO_BAM=1
#DEFINES += ABOOT_IGNORE_BOOT_HEADER_ADDRS=1

#Disable thumb mode
ENABLE_THUMB := false

TCC893X := 1

DEFINES += ABOOT_FORCE_KERNEL_ADDR=0x80008000
DEFINES += ABOOT_FORCE_RAMDISK_ADDR=0x81000000
DEFINES += ABOOT_FORCE_TAGS_ADDR=0x81e00000

EMMC_BOOT := 1

ifeq ($(EMMC_BOOT),1)
DEFINES += _EMMC_BOOT=1
endif

ifeq ($(TZOW),1)
TZOW_ENABLE := 1
endif

ifeq ($(HDEN),1)
HIDDEN_ENABLE := 1
endif

ifeq ($(GPHEN),1)
GPHD_ENABLE := 1
endif
