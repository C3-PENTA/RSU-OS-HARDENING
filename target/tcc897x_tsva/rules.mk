LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/tcc_shared

PLATFORM := tcc897x

#------------------------------------------------------------------
# Define board revision
# 0x1000 : TCC8970_D3164_2CS_V0.1 - DDR3 2048MB(32Bit),2015.04.28
# 0x1000 : TCC8971_D3164_2CS_V1.0 - DDR3 2048MB(32Bit),2015.09.07

HW_REV=0x1000

#==================================================================
# Chipset Information
#==================================================================

#==================================================================
# System Setting
#==================================================================

#------------------------------------------------------------------
# BASE Address
#------------------------------------------------------------------

#	0x80000000 |------------------------------|
#	0x80000100 |------------------------------|
#	           |             ATAG             |
#	0x80008000 |------------------------------|
#	           |         Kernel Image         |
#	0x81000000 |------------------------------|
#	           |           ramdisk            |
#	0x82000000 |------------------------------|
#	           |        Secure Kernel         |
#	0x83000000 |------------------------------|
#	           |         Device Tree          |
#	           | ~  ~  ~  ~  ~  ~  ~  ~  ~  ~ |
#	           |------------------------------| PMAP_BASE
#	           |        FB_VIDEO_BASE         |
#	0x88000000 |------------------------------|
#	           |     LK Bootloader Image      |
#	           |------------------------------|
#	           |          heap area           |
#	0x89000000 |------------------------------|
#	           |             dma              |
#	0x89800000 |------------------------------|
#	0x8C000000 |------------------------------|
#	           |       QB Scratch area        |
#	0x8C400000 |------------------------------|
#	           |           Scratch            |
#	           | ~  ~  ~  ~  ~  ~  ~  ~  ~  ~ |
#	           |------------------------------|

BASE_ADDR        := 0x80000000

# kernel parts
TAGS_ADDR        := BASE_ADDR+0x00000100
KERNEL_ADDR      := BASE_ADDR+0x00008000
RAMDISK_ADDR     := BASE_ADDR+0x02000000
SKERNEL_ADDR     := BASE_ADDR+0x03000000
DTB_ADDR         := BASE_ADDR+0x04000000

# lk bootloader parts
MEMBASE          := 0x88000000		# kernel's mem reserve area (PMAP)
MEMSIZE          := 0x01000000		# 16MB
DMA_SIZE         := 0x00800000		# 8MB
QB_SCRATCH_ADDR  := MEMBASE+0x04000000	# 2MB
SCRATCH_ADDR     := MEMBASE+0x04400000

#==================================================================
# SDRAM Setting
#==================================================================
#------------------------------------------------------------------
# SDRAM CONTROLLER TYPE
#------------------------------------------------------------------
#TCC_MEM_TYPE := DRAM_LPDDR2
TCC_MEM_TYPE := DRAM_DDR3

#------------------------------------------------------------------
# Define memory bus width
#------------------------------------------------------------------
#DEFINES += CONFIG_DRAM_16BIT_USED
DEFINES += CONFIG_DRAM_32BIT_USED

#------------------------------------------------------------------
# Define memory size in MB
#------------------------------------------------------------------
TCC_MEM_SIZE := 1024
#TCC_MEM_SIZE := 2048

DEFINES += TSVA_MEMORY_TEST
DEFINES += TSVA_DDR3_CL14
#DEFINES += TSVA_BOOT_CONFIG_CHECK

#------------------------------------------------------------------
# SDRAM DDR3 Config
#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR3)
	#parameter setting
#	DEFINES += CONFIG_DDR3_1866
#	DEFINES += CONFIG_DDR3_1600
#	DEFINES += CONFIG_DDR3_1333H
#	DEFINES += CONFIG_DDR3_1066
#	DEFINES += CONFIG_DDR3_800E

	ifneq ($(filter $(HW_REV),0x1000),)
		DEFINES += CONFIG_DDR3_MODULE_512MB
		DEFINES += CONFIG_DDR3_1866
		DEFINES += CONFIG_DDR3_IF_16
		DEFINES += CONFIG_DDR3_2CS
	endif
	DEFINES += DRAM_DDR3
endif

DEFINES += DEFAULT_DISPLAY_LCD
#DEFINES += DEFAULT_DISPLAY_HDMI
DEFINES += DISPLAY_SPLASH_SCREEN=1
#DEFINES += DISPLAY_SPLASH_SCREEN_DIRECT=1
#DEFINES += DISPLAY_TYPE_MIPI=1
#DEFINES += AT070TN93      # 800x480
#DEFINES += FLD0800	  # 1024x600 
#DEFINES += ED090NA	   # 1280x800
DEFINES += HONDA_TSVA  # 800x480
DEFINES += IMSI_1024x600_TO_800x480_LCD
#DEFINES += DISPLAY_TYPE_DSI6G=1

TCC_SPLASH_USE := 1
#------------------------------------------------------------------
# for enable early camera(used cortex-m4)
#------------------------------------------------------------------
#USE_CM4_EARLY_CAM := 1
#EARLY_CAM_MODULE := tvp5150
#EARLY_CAM_MODULE := tw9900
#EARLY_CAM_MODULE := adv7182
#EARLY_CAM_MODULE := tw9921

ifeq ($(USE_CM4_EARLY_CAM),1)
	DEFINES += USE_CM4_EARLY_CAM
	DEFINES += CONFIG_VIDEO_TCCXXX_V4L_DEVICE
	MODULES += dev/camera
#	DEFINES += CONFIG_TCC_PARKING_GUIDE_LINE
ifeq ($(EARLY_CAM_MODULE),tw9921)
	DEFINES += CONFIG_CM4_EARLY_LVDS
endif
endif

#------------------------------------------------------------------
# Keypad Type
#------------------------------------------------------------------
KEYS_USE_GPIO_KEYPAD := 0
#KEYS_USE_ADC_KEYPAD := 1
MODULES += \
	dev/pmic/rt5028 \
	lib/ptable \
	lib/libfdt

# //+[TCCQB] Add QuickBoot Scratch Area.
DEFINES += \
	MEMSIZE=$(MEMSIZE) \
	MEMBASE=$(MEMBASE) \
	BASE_ADDR=$(BASE_ADDR) \
	TAGS_ADDR=$(TAGS_ADDR) \
	KERNEL_ADDR=$(KERNEL_ADDR) \
	RAMDISK_ADDR=$(RAMDISK_ADDR) \
	QB_SCRATCH_ADDR=$(QB_SCRATCH_ADDR) \
	SCRATCH_ADDR=$(SCRATCH_ADDR) \
	SKERNEL_ADDR=$(SKERNEL_ADDR) \
	DTB_ADDR=$(DTB_ADDR)\
	TCC_MEM_SIZE=$(TCC_MEM_SIZE) \
	HW_REV=$(HW_REV)
# //-[TCCQB]
# //

#------------------------------------------------------------------
OBJS += \
	$(LOCAL_DIR)/clock.o \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/gpio.o \
	$(LOCAL_DIR)/target_display.o \
	#$(LOCAL_DIR)/atags.o

DEFINES += RT5028_PMIC
#--------------------------------------------------------------
# for USB 3.0 Device
#--------------------------------------------------------------
TCC_USB_30_USE := 0

ifeq ($(TCC_USB_30_USE), 1)
  DEFINES += TCC_USB_30_USE
endif

#==================================================================
# U-Boot Features
# check ./u-boot/u-boot.rom
#==================================================================
#UBOOT_RECOVERY_USE := 1

DEFINES += LCDC0_USE

