# top level project rules for the tcsb_tcc896x_evm project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

include $(LOCAL_DIR)/tcc896x_evm.mk

################################################
## SECURE BOOT
################################################
DEFINES += TSBM_ENABLE
TSBM_INCLUDE := 1

ifeq ($(TZTEE),1)
TRUSTZONE_ENABLE := 1
endif
