LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/tcc_shared

PLATFORM := tcc893x

#==================================================================
# Define board revision
#==================================================================
# 4th 4bits/ 3rd 4bits  / 2nd 4bits       / 1st 4bits
# (Board)  / (chipset)  / (Memoty)        / (Wi-Fi & etc)
# --------------------------------------------------------
# EV    :6 / TCC8930 :0 /  512MB(16BIT):0 / Broadcom:0~7
# STB1  :7 / TCC8935 :1 / 1024MB(16BIT):1 / Realtek :4~7
# Stick :8 / TCC8935S:2 /  512MB(32BIT):2 / Other1  :8~B
#          /            / 1024MB(32BIT):3 / Other2  :C~F
# --------------------------------------------------------
# 0x6034 : EV    /TCC8930 /DDR3 1024MB(32BIT) /Realtek-NAND
# 0x7030 : STB1  /TCC8930 /DDR3 1024MB(32BIT) /Broadcom(TCM3800)-eMMC
# 0x8110 : Stick /TCC8935 /DDR3 1024MB(16BIT) /Broadcom(TCM3800)-eMMC
#------------------------------------------------------------------
#HW_REV=0x6034
HW_REV=0x7030
#HW_REV=0x8110

#==================================================================
# Chipset Information
#==================================================================
#------------------------------------------------------------------
# CHIPSET TYPE
#------------------------------------------------------------------
ifneq ($(filter $(HW_REV),0x6034 0x7030),)
 DEFINES += CONFIG_CHIP_TCC8930
else
 DEFINES += CONFIG_CHIP_TCC8935
 #DEFINES += CONFIG_CHIP_TCC8935S
endif

#==================================================================
# System Setting
#==================================================================

#------------------------------------------------------------------
# BASE Address
#------------------------------------------------------------------
BASE_ADDR        := 0x80000000
MEMBASE          := 0x82000000
MEMSIZE          := 0x01000000 # 16MB
DMA_SIZE         := 0x01000000 # 16MB

TAGS_ADDR        := BASE_ADDR+0x00000100
KERNEL_ADDR      := BASE_ADDR+0x00008000
RAMDISK_ADDR     := BASE_ADDR+0x01000000
SKERNEL_ADDR     := BASE_ADDR+0x04000000
# //+[TCCQB] QuickBoot Scratch Area & change original Scratch Area
QB_SCRATCH_ADDR  := BASE_ADDR+0x06000000 # 2MB
DTB_ADDR         := BASE_ADDR+0x06200000
SCRATCH_ADDR     := BASE_ADDR+0x06400000
# //-[TCCQB]
# //


#==================================================================
# SDRAM Setting
#==================================================================
#------------------------------------------------------------------
# SDRAM CONTROLLER TYPE
#------------------------------------------------------------------
#TCC_MEM_TYPE := DRAM_LPDDR2
#TCC_MEM_TYPE := DRAM_DDR2
TCC_MEM_TYPE := DRAM_DDR3

#------------------------------------------------------------------
# Define memory bus width
#------------------------------------------------------------------
ifneq ($(filter $(DEFINES),CONFIG_CHIP_TCC8935 CONFIG_CHIP_TCC8935S),)
 DEFINES += CONFIG_DRAM_16BIT_USED
else
 DEFINES += CONFIG_DRAM_32BIT_USED
endif

#------------------------------------------------------------------
# Define memory auto training
#------------------------------------------------------------------
ifneq ($(filter $(DEFINES),CONFIG_DRAM_32BIT_USED),)
 ifeq ($(TCC_MEM_TYPE), DRAM_DDR3)
  DEFINES += CONFIG_DRAM_AUTO_TRAINING
 endif
endif

#------------------------------------------------------------------
# Define memory size in MB
#------------------------------------------------------------------
ifneq ($(filter $(HW_REV),0x6034 0x7030 0x8110),)
 TCC_MEM_SIZE := 1024
else
 TCC_MEM_SIZE := 512
endif

ifeq ($(TCC_MEM_SIZE), 256)
 DEFINES += CONFIG_TCC_MEM_256MB
endif
ifeq ($(TCC_MEM_SIZE), 512)
 DEFINES += CONFIG_TCC_MEM_512MB
endif
ifeq ($(TCC_MEM_SIZE), 1024)
 DEFINES += CONFIG_TCC_MEM_1024MB
endif

#------------------------------------------------------------------
# SDRAM DDR2 Vender
#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR2)
 #DEFINES += CONFIG_DDR2_CT83486C1
 DEFINES += CONFIG_DDR2_HY5PS1G831CFPS6
 #DEFINES += CONFIG_DDR2_HY5PS1G1631CFPS6
 #DEFINES += CONFIG_DDR2_HXB18T2G160AF
 #DEFINES += CONFIG_DDR2_K4T1G084QF_BCE7
 #DEFINES += CONFIG_DDR2_K4T1G164QE_HCF7
 #DEFINES += CONFIG_DDR2_MT47H256M8EB25E

 DEFINES += DRAM_DDR2
endif

#------------------------------------------------------------------
# SDRAM DDR3 Vender
#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR3)
	ifneq ($(filter $(HW_REV),0x8110),)
		DEFINES += CONFIG_DDR3_H5TQ4G83AFR_PBC
	else
		#DEFINES += CONFIG_DDR3_H5TC4G83MFR_H9A
		#DEFINES += CONFIG_DDR3_H5TQ1G83BFR_H9C
		#DEFINES += CONFIG_DDR3_H5TQ2G63BFR_H9C
		#DEFINES += CONFIG_DDR3_H5TQ2G63BFR_PBC
		#DEFINES += CONFIG_DDR3_H5TQ2G83BFR_H9C
		DEFINES += CONFIG_DDR3_H5TQ2G83BFR_PBC
		#DEFINES += CONFIG_DDR3_W632GG8KB_12		
		#DEFINES += CONFIG_DDR3_IN5CB256M8BN_CG
		#DEFINES += CONFIG_DDR3_K4B1G1646E_HCH9
		#DEFINES += CONFIG_DDR3_K4B2G1646C_HCK0
		#DEFINES += CONFIG_DDR3_NT5CB256M8GN_CG
	endif

 DEFINES += DRAM_DDR3
endif

#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_LPDDR2)
 DEFINES += CONFIG_LPDDR2_K4P4G324EB_AGC1

 DEFINES += DRAM_LPDDR2
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
 DEFINES += BOARD_TCC893X_STB_DEMO
endif

#------------------------------------------------------------------
# Define target board
#------------------------------------------------------------------
ifneq ($(filter $(HW_REV),0x6034),)
 DEFINES += TARGET_TCC8930_EV
endif

ifneq ($(filter $(HW_REV), 0x7030),)
 DEFINES += TARGET_TCC8930_STB1
endif

ifneq ($(filter $(HW_REV),0x8110),)
 DEFINES += TARGET_TCC8935_STICK
endif

# Defines Default Display
ifeq ($(TARGET_BOARD_STB),true)
DEFINES += DEFAULT_DISPLAY_OUTPUT_DUAL
else
DEFINES += DEFAULT_DISPLAY_LCD
endif
ifeq ($(TARGET_BOARD_STB),true)
DEFINES += HDMI_1280X720
else
DEFINES += DISPLAY_TYPE_MIPI=1
#DEFINES += AT070TN93      # 800x480
DEFINES += FLD0800	  # 1024x600 
DEFINES += DISPLAY_TYPE_DSI6G=1
endif

# Defines Display Type
ifeq ($(TARGET_BOARD_STB),true)
 ifneq ($(filter $(HW_REV),0x6034 0x7030),)
  DEFINES += DISPLAY_STB_NORMAL
  #DEFINES += DISPLAY_STB_AUTO_HDMI_CVBS
  #DEFINES += DISPLAY_STB_ATTACH_HDMI_CVBS
  #DEFINES += DISPLAY_STB_ATTACH_DUAL_AUTO
 endif

 ifneq ($(filter $(HW_REV),0x8110),)
  DEFINES += DISPLAY_STB_NORMAL
 endif
endif

# Define for composite output
ifneq ($(filter $(HW_REV),0x6034 0x7030),)
 DEFINES += DISPLAY_SUPPORT_COMPOSITE
endif

# Define for component output
ifneq ($(filter $(HW_REV),0x6034 0x7030),)
 DEFINES += DISPLAY_SUPPORT_COMPONENT
endif

# Define for component chip
ifeq ($(TARGET_BOARD_STB),true)
DEFINES += COMPONENT_CHIP_THS8200
else
DEFINES += COMPONENT_CHIP_CS4954
endif

# Define Default Splash
DEFINES += DISPLAY_SPLASH_SCREEN=1
DEFINES += DISPLAY_SPLASH_SCREEN_DIRECT=1

TCC_SPLASH_USE := 1

# Define scaler use for displaying logo
DEFINES += DISPLAY_SCALER_USE

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
ifneq ($(filter $(DEFINES),CONFIG_CHIP_TCC8935S CONFIG_CHIP_TCC8933S CONFIG_CHIP_TCC8937S),)
else
#BOOTING_BY_REMOTE_KEY := true
RECOVERY_BY_REMOTE_KEY := true
#START_FWDN_BY_REMOTE_KEY := true
endif

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

ifeq ($(BOOTING_BY_REMOTE_KEY), true)
PM_SUSPEND_USE := 1
else
PM_SUSPEND_USE := 0
endif

MODULES += \
	dev/keys \
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

TCC_HDMI_USE := 1
TCC_CHIP_REV :=1

ifeq ($(TCC_HDMI_USE), 1)
  DEFINES += TCC_HDMI_USE
  DEFINES += TCC_HDMI_USE_XIN_24MHZ

  ifneq ($(filter $(DEFINES),CONFIG_CHIP_TCC8935S CONFIG_CHIP_TCC8933S CONFIG_CHIP_TCC8937S),)
    DEFINES += TCC_HDMI_DRIVER_V1_3
    BOARD_HDMI_V1_3 := 1
  else
    DEFINES += TCC_HDMI_DRIVER_V1_4
    BOARD_HDMI_V1_4 := 1
  endif
endif


ifeq ($(PM_SUSPEND_USE), 1)
  DEFINES += PM_SUSPEND_USE
endif

TCC_CHIP_REV :=1
ifeq ($(TCC_CHIP_REV), 1)
  DEFINES += TCC_CHIP_REV
endif

#--------------------------------------------------------------
# for USB 3.0 Device
#--------------------------------------------------------------
ifneq ($(filter $(DEFINES),CONFIG_CHIP_TCC8935S CONFIG_CHIP_TCC8933S CONFIG_CHIP_TCC8937S),)
TCC_USB_30_USE := 0
else
TCC_USB_30_USE := 1
endif

ifeq ($(TCC_USB_30_USE), 1)
  DEFINES += TCC_USB_30_USE
endif

