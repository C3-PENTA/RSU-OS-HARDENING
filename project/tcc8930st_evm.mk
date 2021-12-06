# top level project rules for the tcc8930st_evm project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := tcc8930st_evm

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

TCC893X := 1
NAND_BOOT := 1

ifeq ($(NAND_BOOT),1)
DEFINES += _NAND_BOOT=1
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
