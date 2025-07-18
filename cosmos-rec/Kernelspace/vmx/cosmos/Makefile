# Makefile

PWD := $(shell pwd)
KDIR := /usr/src/linux-source-5.19.0/linux-source-5.19.0
COSMOS_DIR := /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx/cosmos

include $(COSMOS_DIR)/common_RnR.mk

obj-m += cosmos_rec.o cosmos_utilities.o

KBUILD_EXTMOD := /usr/src/linux-source-5.19.0/linux-source-5.19.0

ifeq ($(INCLUDE_COSMOS_REC), 1)
	ccflags-y += -D_COSMOS_REC_
endif

ifeq ($(INCLUDE_COSMOS_FJ), 1)
	ccflags-y += -D_COSMOS_FJ_
endif

ifeq ($(INCLUDE_COSMOS_UT), 1)
	ccflags-y += -D_COSMOS_UT_
endif

ifeq ($(INCLUDE_COSMOS_DBG), 1)
	ccflags-y += -D_COSMOS_DBG_
endif

default:

	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o
	rm -rf *.ko
	rm -rf *.mod*
	rm -rf Module.symvers
	rm -rf modules.order

.PHONY: clean
