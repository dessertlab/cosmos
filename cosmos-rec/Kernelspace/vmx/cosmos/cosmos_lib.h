#ifndef _TRACE_MY_KVM_
#define _TRACE_MY_KVM_

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kvm_types.h>

MODULE_LICENSE("GPL");

#define BUFFER_SIZE 600000  // Default buffer size

#define NUM_REGS 15
#define DIM_REG 9
#define NUM_OPS 100 // Max operations vmwrite / vmread in an exit

#define NUM_EXIT_REASONS 75

typedef struct {
    unsigned long field;
    unsigned long long value;
    uint8_t type;
} op;

typedef struct {
    uint32_t name;
    unsigned long value;
    uint8_t type;
} reg;

typedef struct {
    reg start;
    reg regs_in[NUM_REGS];
    op ops[NUM_OPS];
    uint8_t op_counter;
    reg regs_out[NUM_REGS];
} vmexit;

void write_regs_in (vmexit* b, size_t exit_counter, unsigned long regs[], bool write_op_counter, size_t buffer_size);
void write_regs_out (vmexit* b, size_t exit_counter, unsigned long regs[], size_t buffer_size);
void write_op (vmexit* b, size_t exit_counter, unsigned long field, unsigned long long value, uint8_t type, size_t buffer_size);

typedef struct {
    unsigned long long cr0_guest_host_mask;
	unsigned long long cr4_guest_host_mask;
	unsigned long long cr0_read_shadow;
	unsigned long long cr4_read_shadow;
	unsigned long vm_exit_reason;
	unsigned long long exit_qualification;
	unsigned long long guest_linear_address;
	unsigned long long guest_cr0;
	unsigned long long guest_cr3;
	unsigned long long guest_cr4;
	unsigned long long guest_es_base;
	unsigned long long guest_cs_base;
	unsigned long long guest_ss_base;
	unsigned long long guest_ds_base;
	unsigned long long guest_fs_base;
	unsigned long long guest_gs_base;
	unsigned long long guest_ldtr_base;
	unsigned long long guest_tr_base;
	unsigned long long guest_gdtr_base;
	unsigned long long guest_idtr_base;
	unsigned long long guest_dr7;
	unsigned long long guest_rsp;
	unsigned long long guest_rip;
	unsigned long long guest_rflags;
	unsigned long long guest_pending_dbg_exceptions;
	unsigned long long guest_sysenter_esp;
	unsigned long long guest_sysenter_eip;
	unsigned long long host_cr0;
	unsigned long long host_cr3;
	unsigned long long host_cr4;
	unsigned long long host_fs_base;
	unsigned long long host_gs_base;
	unsigned long long host_tr_base;
	unsigned long long host_gdtr_base;
	unsigned long long host_idtr_base;
	unsigned long long host_ia32_sysenter_esp;
	unsigned long long host_ia32_sysenter_eip;
	unsigned long long host_rsp;
	unsigned long long host_rip;
	unsigned int pin_based_vm_exec_control;
	unsigned int cpu_based_vm_exec_control;
	unsigned int exception_bitmap;
	unsigned int page_fault_error_code_mask;
	unsigned int page_fault_error_code_match;
	unsigned int cr3_target_count;
	unsigned int vm_exit_controls;
	unsigned int vm_exit_msr_store_count;
	unsigned int vm_exit_msr_load_count;
	unsigned int vm_entry_controls;
	unsigned int vm_entry_msr_load_count;
	unsigned int vm_entry_intr_info_field;
	unsigned int vm_entry_exception_error_code;
	unsigned int vm_entry_instruction_len;
	unsigned int tpr_threshold;
	unsigned int secondary_vm_exec_control;
	unsigned int vm_instruction_error;
	unsigned int vm_exit_intr_info;
	unsigned int vm_exit_intr_error_code;
	unsigned int idt_vectoring_info_field;
	unsigned int idt_vectoring_error_code;
	unsigned int vm_exit_instruction_len;
	unsigned int vmx_instruction_info;
	unsigned int guest_es_limit;
	unsigned int guest_cs_limit;
	unsigned int guest_ss_limit;
	unsigned int guest_ds_limit;
	unsigned int guest_fs_limit;
	unsigned int guest_gs_limit;
	unsigned int guest_ldtr_limit;
	unsigned int guest_tr_limit;
	unsigned int guest_gdtr_limit;
	unsigned int guest_idtr_limit;
	unsigned int guest_es_ar_bytes;
	unsigned int guest_cs_ar_bytes;
	unsigned int guest_ss_ar_bytes;
	unsigned int guest_ds_ar_bytes;
	unsigned int guest_fs_ar_bytes;
	unsigned int guest_gs_ar_bytes;
	unsigned int guest_ldtr_ar_bytes;
	unsigned int guest_tr_ar_bytes;
	unsigned int guest_interruptibility_info;
	unsigned int guest_activity_state;
	unsigned int guest_sysenter_cs;
	unsigned int host_ia32_sysenter_cs;
	unsigned int vmx_preemption_timer_value;
	unsigned short virtual_processor_id;
	unsigned short posted_intr_nv;
	unsigned short guest_es_selector;
	unsigned short guest_cs_selector;
	unsigned short guest_ss_selector;
	unsigned short guest_ds_selector;
	unsigned short guest_fs_selector;
	unsigned short guest_gs_selector;
	unsigned short guest_ldtr_selector;
	unsigned short guest_tr_selector;
	unsigned short guest_intr_status;
	unsigned short host_es_selector;
	unsigned short host_cs_selector;
	unsigned short host_ss_selector;
	unsigned short host_ds_selector;
	unsigned short host_fs_selector;
	unsigned short host_gs_selector;
	unsigned short host_tr_selector;
	unsigned short guest_pml_index;
} dump_vmcs12_t;  

/*
void print_buffer_i (vmexit* b, size_t pos) {
    size_t i = 0; 

    printk ("SEED %lu.\n", pos); 

    printk ("%u\n", b[pos].start.name);
    printk ("%lu\n", b[pos].start.value);
    printk ("%u\n", b[pos].start.type);

    for (i = 0; i < NUM_REGS; i++) {
        printk ("%u\n", b[pos].regs_in[i].name);
        printk ("%lu\n", b[pos].regs_in[i].value);
        printk ("%u\n", b[pos].regs_in[i].type);
    }

    for (i = 0; i < b[pos].op_counter; i++) {
        printk ("%lu\n", b[pos].ops[i].field);
        printk ("%llu\n", b[pos].ops[i].value);
        printk ("%u\n", b[pos].ops[i].type);
    }

    for (i = 0; i < NUM_REGS; i++) {
        printk ("%u\n", b[pos].regs_out[i].name);
        printk ("%lu\n", b[pos].regs_out[i].value);
        printk ("%u\n", b[pos].regs_out[i].type);
    }
} 
*/

void print_exit (vmexit e);

#endif