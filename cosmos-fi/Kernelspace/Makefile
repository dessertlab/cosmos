# SPDX-License-Identifier: GPL-2.0

COSMOS_DIR := /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx/cosmos
include $(COSMOS_DIR)/common_RnR.mk

ccflags-y += -I $(srctree)/arch/x86/kvm
ccflags-$(CONFIG_KVM_WERROR) += -Werror

ifeq ($(CONFIG_FRAME_POINTER),y)
OBJECT_FILES_NON_STANDARD_vmenter.o := y
endif

# For these variables, check common_COSMOS.mk

ifeq ($(INCLUDE_COSMOS_REC), 1)
	ccflags-y += -D_COSMOS_REC
endif

ifeq ($(INCLUDE_COSMOS_FJ), 1)
	ccflags-y += -D_COSMOS_FJ_
endif

ifeq ($(INCLUDE_COSMOS_UT), 1)
	ccflags-y += -D_COSMOS_UT_
endif

ifeq ($(INCLUDE_COSMOS_DBG_), 1)
	ccflags-y += -D_COSMOS_DBG_
endif

include $(srctree)/virt/kvm/Makefile.kvm

kvm-y			+= x86.o emulate.o i8259.o irq.o lapic.o \
			   i8254.o ioapic.o irq_comm.o cpuid.o pmu.o mtrr.o \
			   hyperv.o debugfs.o mmu/mmu.o mmu/page_track.o \
			   mmu/spte.o

ifdef CONFIG_HYPERV
kvm-y			+= kvm_onhyperv.o
endif

kvm-$(CONFIG_X86_64) += mmu/tdp_iter.o mmu/tdp_mmu.o
kvm-$(CONFIG_KVM_XEN)	+= xen.o

kvm-intel-y		+= vmx/vmx.o vmx/vmenter.o vmx/pmu_intel.o vmx/vmcs12.o \
			   vmx/evmcs.o vmx/nested.o vmx/posted_intr.o 

# COSMOS
kvm-intel-y 	+= vmx/cosmos/cosmos_lib.o

kvm-intel-$(CONFIG_X86_SGX_KVM)	+= vmx/sgx.o

kvm-amd-y		+= svm/svm.o svm/vmenter.o svm/pmu.o svm/nested.o svm/avic.o svm/sev.o

ifdef CONFIG_HYPERV
kvm-amd-y		+= svm/svm_onhyperv.o
endif

obj-$(CONFIG_KVM)	+= kvm.o
obj-$(CONFIG_KVM_INTEL)	+= kvm-intel.o
obj-$(CONFIG_KVM_AMD)	+= kvm-amd.o

# COSMOS
ifeq ($(INCLUDE_COSMOS_REC), 1)
	obj-$(CONFIG_KVM_INTEL)	+= vmx/cosmos/cosmos_rec.o 
endif
# COSMOS_FJ
ifeq ($(INCLUDE_COSMOS_FJ), 1)
	obj-$(CONFIG_KVM_INTEL)	+= vmx/cosmos/cosmos_fi.o 
endif

AFLAGS_svm/vmenter.o    := -iquote $(obj)
$(obj)/svm/vmenter.o: $(obj)/kvm-asm-offsets.h

AFLAGS_vmx/vmenter.o    := -iquote $(obj)
$(obj)/vmx/vmenter.o: $(obj)/kvm-asm-offsets.h

$(obj)/kvm-asm-offsets.h: $(obj)/kvm-asm-offsets.s FORCE
	$(call filechk,offsets,__KVM_ASM_OFFSETS_H__)

targets += kvm-asm-offsets.s
clean-files += kvm-asm-offsets.h

KBUILD_EXTMOD := /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx/cosmos
COSMOS_KVM_MAIN_MODVERDIR := $(KBUILD_EXTMOD)
