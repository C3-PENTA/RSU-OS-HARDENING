LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LOCAL_DIR)/platform/tcc_shared/include

OBJS += \
	$(LOCAL_DIR)/sensor_if.o	\
	$(LOCAL_DIR)/camera.o
	
ifeq ($(EARLY_CAM_MODULE),tvp5150)
OBJS += \
	$(LOCAL_DIR)/tvp5150.o
DEFINES += EARLY_CAM_MODULE_TVP5150
endif

ifeq ($(EARLY_CAM_MODULE),adv7182)
OBJS += \
	$(LOCAL_DIR)/adv7182.o
DEFINES += EARLY_CAM_MODULE_ADV7182
endif

ifeq ($(EARLY_CAM_MODULE),tw9921)
OBJS += \
	$(LOCAL_DIR)/tw9921.o
DEFINES += EARLY_CAM_MODULE_TW9921
endif

