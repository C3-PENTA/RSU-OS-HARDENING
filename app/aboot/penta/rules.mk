LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LK_TOP_DIR)/platform/tcc_shared/include -I$(LOCAL_DIR)/..

OBJS += \
    $(LOCAL_DIR)/crc16.o \
    $(LOCAL_DIR)/sha256.o \
    $(LOCAL_DIR)/secureboot.o \
    $(LOCAL_DIR)/bootimg_secureboot.o 

