LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LK_TOP_DIR)/platform/tcc_shared/include

OBJS += \
	$(LOCAL_DIR)/clock_tests.o
