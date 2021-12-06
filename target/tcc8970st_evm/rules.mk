LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/tcc_shared

PLATFORM := tcc897x

#===========================================================================
# Define board revision
#===========================================================================
# 4th 4bits      / 3rd 4bits  / 2nd 4bits           / 1st 4bits
# (Board)        / (chipset)  / (Memory)            / (Wi-Fi & etc)
# --------------------------------------------------------------------------
# STB   :6       /TCC8960 :0  / 512MB(16bitx2):0    /Broadcom:0~7
# Stick :7       /TCC8963 :1  /1024MB(16bitx2):1    /0-TCM3800
# EVM   :8       /TCC8975 :2  /2048MB( 8bitx4):2    /1-TCM3830
#                /TCC8976 :3                        /2-BCM4354
#                /TCC8970 :4                        /3-BCM43241
# --------------------------------------------------------------------------
# 0x6420 : STB   /TCC8970     /DDR3 2048MB( 8bitx4) /Broadcom(TCM3800)-eMMC
# 0x6220 : STB   /TCC8975     /DDR3 2048MB( 8bitx4) /Broadcom(TCM3800)-eMMC
# 0x6211 : STB   /TCC8975     /DDR3 1024MB(16bitx2) /Broadcom(TCM3830)-eMMC
# 0x7210 : Stick /TCC8975     /DDR3 1024MB(16bitx2) /Broadcom(TCM3800)-eMMC
# 0x7212 : Stick /TCC8975     /DDR3 1024MB(16bitx2) /Broadcom(BCM4354)-eMMC
# 0x7213 : Stick /TCC8975     /DDR3 1024MB(16bitx2) /Broadcom(BCM43241)-eMMC
# 0x8210 : EVM   /TCC8975     /DDR3 1024MB(16bitx2) /NONE
#---------------------------------------------------------------------------

#HW_REV=0x6420
HW_REV=0x6220
#HW_REV=0x6211
#HW_REV=0x7210
#HW_REV=0x7212
#HW_REV=0x7213
#HW_REV=0x8210


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
#	0x8A000000 |------------------------------|
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
DMA_SIZE         := 0x01000000		# 16MB
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
ifneq ($(filter $(HW_REV),0x6211 0x7210 0x7212 0x7213 0x8210),)
TCC_MEM_SIZE := 1024
endif
ifneq ($(filter $(HW_REV),0x6420 0x6220),)
TCC_MEM_SIZE := 2048
endif

#------------------------------------------------------------------
# SDRAM DDR3 Vender
#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR3)
	#parameter setting	
#	DEFINES += CONFIG_DDR3_1866		    
#	DEFINES += CONFIG_DDR3_1600		    
#	DEFINES += CONFIG_DDR3_1333H		    
#	DEFINES += CONFIG_DDR3_1066		    
#	DEFINES += CONFIG_DDR3_800E
DEFINES += CONFIG_DDR3_MODULE_512MB
DEFINES += CONFIG_DDR3_1CS	
DEFINES += CONFIG_DDR3_1866

	ifneq ($(filter $(HW_REV),0x6211 0x7210 0x7212 0x7213 0x8210),)
		DEFINES += CONFIG_DDR3_IF_16
	endif
	
	ifneq ($(filter $(HW_REV),0x6420 0x6220),)
        DEFINES += CONFIG_DDR3_IF_8
	endif

 	DEFINES += DRAM_DDR3
endif


#==================================================================
# Target Board Setting
#==================================================================
#------------------------------------------------------------------
# Define if target board is STB
#------------------------------------------------------------------
TARGET_BOARD_STB := true
ifeq ($(TARGET_BOARD_STB),true)
 DEFINES += TARGET_BOARD_STB
 DEFINES += BOARD_TCC897X_STB_DEMO
endif

#------------------------------------------------------------------
# Define target board
#------------------------------------------------------------------
ifneq ($(filter $(HW_REV),0x6420),)
 DEFINES += TARGET_TCC8970_STB
endif

ifneq ($(filter $(HW_REV),0x6211 0x6220),)
 DEFINES += TARGET_TCC8975_STB
endif

ifneq ($(filter $(HW_REV),0x7210 0x7212 0x7213),)
 DEFINES += TARGET_TCC8975_STICK
endif

ifneq ($(filter $(HW_REV),0x8210),)
 DEFINES += TARGET_TCC8975_EVM
endif

# Defines Default Display
ifneq ($(filter $(HW_REV),0x8210),)
DEFINES += DEFAULT_DISPLAY_HDMI
DEFINES += HDMI_1280X720
else
#DEFINES += DEFAULT_DISPLAY_HDMI
#DEFINES += DEFAULT_DISPLAY_COMPOSITE
#DEFINES += DEFAULT_DISPLAY_COMPONENT
DEFINES += DEFAULT_DISPLAY_OUTPUT_DUAL
DEFINES += HDMI_1280X720
endif

# Defines Display Type
DEFINES += DISPLAY_STB_NORMAL
#DEFINES += DISPLAY_STB_AUTO_HDMI_CVBS
#DEFINES += DISPLAY_STB_ATTACH_HDMI_CVBS
#DEFINES += DISPLAY_STB_ATTACH_DUAL_AUTO

# Define for component/composite output
ifneq ($(filter $(HW_REV),0x6420),)
 DEFINES += DISPLAY_SUPPORT_COMPOSITE
 DEFINES += DISPLAY_SUPPORT_COMPONENT
 DEFINES += COMPONENT_CHIP_THS8200
endif

# Define Default Splash
DEFINES += DISPLAY_SPLASH_SCREEN=1
DEFINES += DISPLAY_SPLASH_SCREEN_DIRECT=1

#TCC_SPLASH_USE := 1

# Define scaler use for displaying logo
#DEFINES += DISPLAY_SCALER_USE

#------------------------------------------------------------------
# Keypad Type
#------------------------------------------------------------------
KEYS_USE_GPIO_KEYPAD := 1
#KEYS_USE_ADC_KEYPAD := 1

#------------------------------------------------------------------
#Define Factory Reset
#------------------------------------------------------------------
ifeq ($(TARGET_BOARD_STB),true)
DEFINES += TCC_FACTORY_RESET_SUPPORT
endif

#------------------------------------------------------------------
# Defines Remocon
#------------------------------------------------------------------
#BOOTING_BY_REMOTE_KEY := true
#RECOVERY_BY_REMOTE_KEY := true
#START_FWDN_BY_REMOTE_KEY := true

ifeq ($(BOOTING_BY_REMOTE_KEY), true)
DEFINES += BOOTING_BY_REMOTE_KEY
endif
ifeq ($(RECOVERY_BY_REMOTE_KEY), true)
DEFINES += RECOVERY_BY_REMOTE_KEY
endif
ifeq ($(START_FWDN_BY_REMOTE_KEY), true)
DEFINES += START_FWDN_BY_REMOTE_KEY
endif

ifneq ($(filter true,$(BOOTING_BY_REMOTE_KEY) $(RECOVERY_BY_REMOTE_KEY) $(START_FWDN_BY_REMOTE_KEY)),)
#DEFINES += CONFIG_CS_2000_IR_X_CANVAS
DEFINES += CONFIG_YAOJIN_IR

#DEFINES += PBUS_DIVIDE_CLOCK          # only tcc892x
DEFINES += PBUS_DIVIDE_CLOCK_WITH_XTIN # only tcc893x
endif

MODULES += \
	dev/keys \
    	dev/pmic/rt8088 \
	dev/pmic/da9062 \
	dev/gpio/pca953x \
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
	$(LOCAL_DIR)/keypad.o \
	$(LOCAL_DIR)/gpio.o \
	$(LOCAL_DIR)/target_display.o \
	$(LOCAL_DIR)/atags.o

#DEFINES += RT8088_DISCR
DEFINES += DA9062_PMIC

DEFINES += TCC_PCA953X_USE

TCC_HDMI_USE := 1
TCC_CHIP_REV :=1

ifeq ($(TCC_HDMI_USE), 1)
  DEFINES += TCC_HDMI_USE
  DEFINES += TCC_HDMI_USE_XIN_24MHZ
  DEFINES += TCC_HDMI_DRIVER_V1_4
  BOARD_HDMI_V1_4 := 1
endif

#--------------------------------------------------------------
# for USB 3.0 Device
#--------------------------------------------------------------
TCC_USB_30_USE := 0

ifeq ($(TCC_USB_30_USE), 1)
  DEFINES += TCC_USB_30_USE
endif
