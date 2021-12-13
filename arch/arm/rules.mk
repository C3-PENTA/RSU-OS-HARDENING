LOCAL_DIR := $(GET_LOCAL_DIR)

# can override this in local.mk
ENABLE_THUMB?=true

DEFINES += \
	ARM_CPU_$(ARM_CPU)=1

# do set some options based on the cpu core
HANDLED_CORE := false
ifneq ($(filter $(ARM_CPU),cortex-a8 cortex-a15 cortex-a7),)
DEFINES += \
	ARM_WITH_CP15=1 \
	ARM_WITH_MMU=1 \
	ARM_ISA_ARMv7=1 \
	ARM_WITH_VFP=1 \
	ARM_WITH_NEON=0 \
	ARM_WITH_THUMB=1 \
	ARM_WITH_THUMB2=1 \
	ARM_WITH_CACHE=1 \
	ARM_WITH_L2=1
CFLAGS += -march=armv7-a
HANDLED_CORE := true
#CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif
ifneq ($(filter $(ARM_CPU),cortex-a5 cortex-a9),)
DEFINES += \
	ARM_WITH_CP15=1 \
	ARM_WITH_MMU=1 \
	ARM_ISA_ARMv7=1 \
	ARM_WITH_VFP=1 \
	ARM_WITH_NEON=1 \
	ARM_WITH_THUMB=1 \
	ARM_WITH_THUMB2=1 \
	ARM_WITH_CACHE=1
	#ARM_WITH_L2=1
CFLAGS += -mcpu=$(ARM_CPU)
CFLAGS += -march=armv7-a
HANDLED_CORE := true
#CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif
ifeq ($(ARM_CPU),arm1136j-s)
DEFINES += \
	ARM_WITH_CP15=1 \
	ARM_WITH_MMU=1 \
	ARM_ISA_ARMv6=1 \
	ARM_WITH_THUMB=1 \
	ARM_WITH_CACHE=1 \
	ARM_CPU_ARM1136=1
CFLAGS += -mcpu=$(ARM_CPU)
HANDLED_CORE := true
endif
ifeq ($(ARM_CPU),arm1176jzf-s)
DEFINES += \
	ARM_WITH_CP15=1 \
	ARM_WITH_MMU=1 \
	ARM_ISA_ARMv6=1 \
	ARM_WITH_VFP=1 \
	ARM_WITH_THUMB=1 \
	ARM_WITH_CACHE=1 \
	ARM_CPU_ARM1136=1
CFLAGS += -mcpu=$(ARM_CPU)
HANDLED_CORE := true
endif
ifeq ($(ARM_CPU),arm926ej-s)
DEFINES += \
	ARM_WITH_CP15=1 \
	ARM_WITH_MMU=1 \
	ARM_ISA_ARMv5E=1 \
	ARM_WITH_THUMB=1 \
	ARM_WITH_CACHE=1 \
	ARM_CPU_ARM9=1 \
	ARM_CPU_ARM926=1
CFLAGS += -mcpu=$(ARM_CPU)
HANDLED_CORE := true
endif
ifeq ($(ARM_CPU),arm7tdmi)
DEFINES += \
	ARM_ISA_ARMv4=1 \
	ARM_WITH_THUMB=1 \
	ARM_CPU_ARM7=1
CFLAGS += -mcpu=$(ARM_CPU)
HANDLED_CORE := true
endif

ifneq ($(HANDLED_CORE),true)
$(warning $(LOCAL_DIR)/rules.mk doesnt have logic for arm core $(ARM_CPU))
$(warning this is likely to be broken)
endif

THUMBCFLAGS :=
THUMBINTERWORK :=
ifeq ($(ENABLE_THUMB),true)
THUMBCFLAGS := -mthumb -D__thumb__
THUMBINTERWORK := -mthumb-interwork
endif

INCLUDES += \
	-I$(LOCAL_DIR)/include

BOOTOBJS += \
	$(LOCAL_DIR)/crt0.o

OBJS += \
	$(LOCAL_DIR)/arch.Ao \
	$(LOCAL_DIR)/asm.o \
	$(LOCAL_DIR)/cache.o \
	$(LOCAL_DIR)/cache-ops.o \
	$(LOCAL_DIR)/ops.o \
	$(LOCAL_DIR)/exceptions.o \
	$(LOCAL_DIR)/faults.o \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/thread.o \
	$(LOCAL_DIR)/dcc.o

ifeq ($(UBOOT_RECOVERY_USE), 1)
	DEFINES += CONFIG_UBOOT_RECOVERY_USE
	OBJS += $(LOCAL_DIR)/uboot.o
endif
ifeq ($(CACHE_PL310_USE), 1)
DEFINES += CONFIG_CACHE_PL310=1
OBJS += $(LOCAL_DIR)/pl310_cache.o
endif

# set the default toolchain to arm eabi and set a #define
#TOOLCHAIN_PREFIX ?= arm-linux-androideabi-
ifeq ($(TARGET_GCC_VERSION),4.7)
TOOLCHAIN_PREFIX ?= ../../../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
else
TOOLCHAIN_PREFIX ?= arm-eabi-
endif
ifeq ($(TOOLCHAIN_PREFIX),arm-none-linux-gnueabi-)
# XXX test for EABI better than this
# eabi compilers dont need this
THUMBINTERWORK:=
endif

CFLAGS += $(THUMBINTERWORK)

# make sure some bits were set up
MEMVARS_SET := 0
ifneq ($(MEMBASE),)
MEMVARS_SET := 1
endif
ifneq ($(MEMSIZE),)
MEMVARS_SET := 1
endif
ifeq ($(MEMVARS_SET),0)
$(error missing MEMBASE or MEMSIZE variable, please set in target rules.mk)
endif

LIBGCC := $(shell $(TOOLCHAIN_PREFIX)gcc $(CFLAGS) $(THUMBCFLAGS) -print-libgcc-file-name)
#$(info LIBGCC = $(LIBGCC))

# potentially generated files that should be cleaned out with clean make rule
GENERATED += \
	$(BUILDDIR)/system-onesegment.ld \
	$(BUILDDIR)/system-twosegment.ld

# rules for generating the linker scripts

ifeq ($(DMA_SIZE),)
DMA_SIZE := 0
endif

$(BUILDDIR)/trustzone-test-system-onesegment.ld: $(LOCAL_DIR)/trustzone-test-system-onesegment.ld
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)sed "s/%MEMBASE%/$(MEMBASE)/;s/%MEMSIZE%/$(MEMSIZE)/;s/%DMA_SIZE%/$(DMA_SIZE)/;s/%ROMLITE_PREFLASHED_DATA%/$(ROMLITE_PREFLASHED_DATA)/" < $< > $@

$(BUILDDIR)/trustzone-system-onesegment.ld: $(LOCAL_DIR)/trustzone-system-onesegment.ld
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)sed "s/%MEMBASE%/$(MEMBASE)/;s/%MEMSIZE%/$(MEMSIZE)/;s/%DMA_SIZE%/$(DMA_SIZE)/" < $< > $@

$(BUILDDIR)/system-onesegment.ld: $(LOCAL_DIR)/system-onesegment.ld
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)sed "s/%MEMBASE%/$(MEMBASE)/;s/%MEMSIZE%/$(MEMSIZE)/;s/%DMA_SIZE%/$(DMA_SIZE)/" < $< > $@

$(BUILDDIR)/system-twosegment.ld: $(LOCAL_DIR)/system-twosegment.ld
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)sed "s/%ROMBASE%/$(ROMBASE)/;s/%MEMBASE%/$(MEMBASE)/;s/%MEMSIZE%/$(MEMSIZE)/;s/%DMA_SIZE%/$(DMA_SIZE)/" < $< > $@

