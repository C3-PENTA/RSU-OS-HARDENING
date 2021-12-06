# top level project rules for the tcc896x_evm project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := tcc896x_evm

MODULES += app/aboot

DEBUG := 1
#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
#DEFINES += CRYPTO_BAM=1

#Disable thumb mode
ENABLE_THUMB := false

TCC896X := 1

ifeq ($(EMMC_BOOT),1)
DEFINES += _EMMC_BOOT=1
else
NAND_BOOT := 1
ifeq ($(NAND_BOOT),1)
DEFINES += _NAND_BOOT=1
endif
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
