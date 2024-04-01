LOCAL_DIR := $(GET_LOCAL_DIR)

ARCH := arm
ARM_CPU := cortex-a8
#arm1136j-s
CPU := generic

DEFINES += WITH_CPU_EARLY_INIT=1 WITH_CPU_WARM_BOOT=0 ARM_CPU_CORE_SCORPION=1
DEFINES += PLATFORM_NAND=1 # qsd8k_hack
DEFINES += MEMBASE=0x28000000

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/msm_shared/include

DEVS += fbcon
MODULES += dev/fbcon

OBJS += \
	$(LOCAL_DIR)/arch_init.o \
	$(LOCAL_DIR)/platform.o \
	$(LOCAL_DIR)/interrupts.o \
	$(LOCAL_DIR)/gpio.o \
	$(LOCAL_DIR)/panel.o \
	$(LOCAL_DIR)/acpuclock.o

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

include platform/msm_shared/rules.mk

