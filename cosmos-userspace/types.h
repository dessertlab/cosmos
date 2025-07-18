#ifndef _TYPES_
#define _TYPES_

#include <inttypes.h>
#include <semaphore.h>

#define NUM_REGS 15
#define DIM_REG 9
#define NUM_OPS 100 // Max operations vmwrite / vmread in an exit

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

typedef struct {
    int key;
    char exit_reason[100];
    int count;
} Exit_Reasons_count; 

typedef struct {
    sem_t *semaphore; 
    const char* snap_name; 
} UnpauseThreadData; 

typedef size_t exit_reasons_idle_t[];

enum GuestError {
    Success,
    GuestInvalidState, // nested_vmx_check_guest_state
    GuestInvalidControls, // nested_vmx_check_controls
    GuestInvalidAddressSpaceSize, // nested_vmx_check_address_space_size
    GuestInvalidHostState, // nested_vmx_check_host_state
    GuestInvalidExecutionControls // nested_check_vm_execution_controls
};

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

enum vmcs_field {
	VIRTUAL_PROCESSOR_ID		= 0x00000000,
	POSTED_INTR_NV			= 0x00000002,
	GUEST_ES_SELECTOR		= 0x00000800,
	GUEST_CS_SELECTOR		= 0x00000802,
	GUEST_SS_SELECTOR		= 0x00000804,
	GUEST_DS_SELECTOR		= 0x00000806,
	GUEST_FS_SELECTOR		= 0x00000808,
	GUEST_GS_SELECTOR		= 0x0000080a,
	GUEST_LDTR_SELECTOR		= 0x0000080c,
	GUEST_TR_SELECTOR		= 0x0000080e,
	GUEST_INTR_STATUS		= 0x00000810,
	GUEST_PML_INDEX			= 0x00000812,
	HOST_ES_SELECTOR		= 0x00000c00,
	HOST_CS_SELECTOR		= 0x00000c02,
	HOST_SS_SELECTOR		= 0x00000c04,
	HOST_DS_SELECTOR		= 0x00000c06,
	HOST_FS_SELECTOR		= 0x00000c08,
	HOST_GS_SELECTOR		= 0x00000c0a,
	HOST_TR_SELECTOR		= 0x00000c0c,
	IO_BITMAP_A			= 0x00002000,
	IO_BITMAP_A_HIGH		= 0x00002001,
	IO_BITMAP_B			= 0x00002002,
	IO_BITMAP_B_HIGH		= 0x00002003,
	MSR_BITMAP			= 0x00002004,
	MSR_BITMAP_HIGH			= 0x00002005,
	VM_EXIT_MSR_STORE_ADDR		= 0x00002006,
	VM_EXIT_MSR_STORE_ADDR_HIGH	= 0x00002007,
	VM_EXIT_MSR_LOAD_ADDR		= 0x00002008,
	VM_EXIT_MSR_LOAD_ADDR_HIGH	= 0x00002009,
	VM_ENTRY_MSR_LOAD_ADDR		= 0x0000200a,
	VM_ENTRY_MSR_LOAD_ADDR_HIGH	= 0x0000200b,
	PML_ADDRESS			= 0x0000200e,
	PML_ADDRESS_HIGH		= 0x0000200f,
	TSC_OFFSET			= 0x00002010,
	TSC_OFFSET_HIGH			= 0x00002011,
	VIRTUAL_APIC_PAGE_ADDR		= 0x00002012,
	VIRTUAL_APIC_PAGE_ADDR_HIGH	= 0x00002013,
	APIC_ACCESS_ADDR		= 0x00002014,
	APIC_ACCESS_ADDR_HIGH		= 0x00002015,
	POSTED_INTR_DESC_ADDR		= 0x00002016,
	POSTED_INTR_DESC_ADDR_HIGH	= 0x00002017,
	EPT_POINTER			= 0x0000201a,
	EPT_POINTER_HIGH		= 0x0000201b,
	EOI_EXIT_BITMAP0		= 0x0000201c,
	EOI_EXIT_BITMAP0_HIGH		= 0x0000201d,
	EOI_EXIT_BITMAP1		= 0x0000201e,
	EOI_EXIT_BITMAP1_HIGH		= 0x0000201f,
	EOI_EXIT_BITMAP2		= 0x00002020,
	EOI_EXIT_BITMAP2_HIGH		= 0x00002021,
	EOI_EXIT_BITMAP3		= 0x00002022,
	EOI_EXIT_BITMAP3_HIGH		= 0x00002023,
	VMREAD_BITMAP			= 0x00002026,
	VMREAD_BITMAP_HIGH		= 0x00002027,
	VMWRITE_BITMAP			= 0x00002028,
	VMWRITE_BITMAP_HIGH		= 0x00002029,
	XSS_EXIT_BITMAP			= 0x0000202C,
	XSS_EXIT_BITMAP_HIGH		= 0x0000202D,
	ENCLS_EXITING_BITMAP		= 0x0000202E,
	ENCLS_EXITING_BITMAP_HIGH	= 0x0000202F,
	TSC_MULTIPLIER			= 0x00002032,
	TSC_MULTIPLIER_HIGH		= 0x00002033,
	GUEST_PHYSICAL_ADDRESS		= 0x00002400,
	GUEST_PHYSICAL_ADDRESS_HIGH	= 0x00002401,
	VMCS_LINK_POINTER		= 0x00002800,
	VMCS_LINK_POINTER_HIGH		= 0x00002801,
	GUEST_IA32_DEBUGCTL		= 0x00002802,
	GUEST_IA32_DEBUGCTL_HIGH	= 0x00002803,
	GUEST_IA32_PAT			= 0x00002804,
	GUEST_IA32_PAT_HIGH		= 0x00002805,
	GUEST_IA32_EFER			= 0x00002806,
	GUEST_IA32_EFER_HIGH		= 0x00002807,
	GUEST_IA32_PERF_GLOBAL_CTRL	= 0x00002808,
	GUEST_IA32_PERF_GLOBAL_CTRL_HIGH= 0x00002809,
	GUEST_PDPTR0			= 0x0000280a,
	GUEST_PDPTR0_HIGH		= 0x0000280b,
	GUEST_PDPTR1			= 0x0000280c,
	GUEST_PDPTR1_HIGH		= 0x0000280d,
	GUEST_PDPTR2			= 0x0000280e,
	GUEST_PDPTR2_HIGH		= 0x0000280f,
	GUEST_PDPTR3			= 0x00002810,
	GUEST_PDPTR3_HIGH		= 0x00002811,
	GUEST_BNDCFGS			= 0x00002812,
	GUEST_BNDCFGS_HIGH		= 0x00002813,
	HOST_IA32_PAT			= 0x00002c00,
	HOST_IA32_PAT_HIGH		= 0x00002c01,
	HOST_IA32_EFER			= 0x00002c02,
	HOST_IA32_EFER_HIGH		= 0x00002c03,
	HOST_IA32_PERF_GLOBAL_CTRL	= 0x00002c04,
	HOST_IA32_PERF_GLOBAL_CTRL_HIGH	= 0x00002c05,
	PIN_BASED_VM_EXEC_CONTROL	= 0x00004000,
	CPU_BASED_VM_EXEC_CONTROL	= 0x00004002,
	EXCEPTION_BITMAP		= 0x00004004,
	PAGE_FAULT_ERROR_CODE_MASK	= 0x00004006,
	PAGE_FAULT_ERROR_CODE_MATCH	= 0x00004008,
	CR3_TARGET_COUNT		= 0x0000400a,
	VM_EXIT_CONTROLS		= 0x0000400c,
	VM_EXIT_MSR_STORE_COUNT		= 0x0000400e,
	VM_EXIT_MSR_LOAD_COUNT		= 0x00004010,
	VM_ENTRY_CONTROLS		= 0x00004012,
	VM_ENTRY_MSR_LOAD_COUNT		= 0x00004014,
	VM_ENTRY_INTR_INFO_FIELD	= 0x00004016,
	VM_ENTRY_EXCEPTION_ERROR_CODE	= 0x00004018,
	VM_ENTRY_INSTRUCTION_LEN	= 0x0000401a,
	TPR_THRESHOLD			= 0x0000401c,
	SECONDARY_VM_EXEC_CONTROL	= 0x0000401e,
	PLE_GAP				= 0x00004020,
	PLE_WINDOW			= 0x00004022,
	VM_INSTRUCTION_ERROR		= 0x00004400,
	VM_EXIT_REASON			= 0x00004402,
	VM_EXIT_INTR_INFO		= 0x00004404,
	VM_EXIT_INTR_ERROR_CODE		= 0x00004406,
	IDT_VECTORING_INFO_FIELD	= 0x00004408,
	IDT_VECTORING_ERROR_CODE	= 0x0000440a,
	VM_EXIT_INSTRUCTION_LEN		= 0x0000440c,
	VMX_INSTRUCTION_INFO		= 0x0000440e,
	GUEST_ES_LIMIT			= 0x00004800,
	GUEST_CS_LIMIT			= 0x00004802,
	GUEST_SS_LIMIT			= 0x00004804,
	GUEST_DS_LIMIT			= 0x00004806,
	GUEST_FS_LIMIT			= 0x00004808,
	GUEST_GS_LIMIT			= 0x0000480a,
	GUEST_LDTR_LIMIT		= 0x0000480c,
	GUEST_TR_LIMIT			= 0x0000480e,
	GUEST_GDTR_LIMIT		= 0x00004810,
	GUEST_IDTR_LIMIT		= 0x00004812,
	GUEST_ES_AR_BYTES		= 0x00004814,
	GUEST_CS_AR_BYTES		= 0x00004816,
	GUEST_SS_AR_BYTES		= 0x00004818,
	GUEST_DS_AR_BYTES		= 0x0000481a,
	GUEST_FS_AR_BYTES		= 0x0000481c,
	GUEST_GS_AR_BYTES		= 0x0000481e,
	GUEST_LDTR_AR_BYTES		= 0x00004820,
	GUEST_TR_AR_BYTES		= 0x00004822,
	GUEST_INTERRUPTIBILITY_INFO	= 0x00004824,
	GUEST_ACTIVITY_STATE		= 0X00004826,
	GUEST_SYSENTER_CS		= 0x0000482A,
	VMX_PREEMPTION_TIMER_VALUE	= 0x0000482E,
	HOST_IA32_SYSENTER_CS		= 0x00004c00,
	CR0_GUEST_HOST_MASK		= 0x00006000,
	CR4_GUEST_HOST_MASK		= 0x00006002,
	CR0_READ_SHADOW			= 0x00006004,
	CR4_READ_SHADOW			= 0x00006006,
	CR3_TARGET_VALUE0		= 0x00006008,
	CR3_TARGET_VALUE1		= 0x0000600a,
	CR3_TARGET_VALUE2		= 0x0000600c,
	CR3_TARGET_VALUE3		= 0x0000600e,
	EXIT_QUALIFICATION		= 0x00006400,
	GUEST_LINEAR_ADDRESS		= 0x0000640a,
	GUEST_CR0			= 0x00006800,
	GUEST_CR3			= 0x00006802,
	GUEST_CR4			= 0x00006804,
	GUEST_ES_BASE			= 0x00006806,
	GUEST_CS_BASE			= 0x00006808,
	GUEST_SS_BASE			= 0x0000680a,
	GUEST_DS_BASE			= 0x0000680c,
	GUEST_FS_BASE			= 0x0000680e,
	GUEST_GS_BASE			= 0x00006810,
	GUEST_LDTR_BASE			= 0x00006812,
	GUEST_TR_BASE			= 0x00006814,
	GUEST_GDTR_BASE			= 0x00006816,
	GUEST_IDTR_BASE			= 0x00006818,
	GUEST_DR7			= 0x0000681a,
	GUEST_RSP			= 0x0000681c,
	GUEST_RIP			= 0x0000681e,
	GUEST_RFLAGS			= 0x00006820,
	GUEST_PENDING_DBG_EXCEPTIONS	= 0x00006822,
	GUEST_SYSENTER_ESP		= 0x00006824,
	GUEST_SYSENTER_EIP		= 0x00006826,
	HOST_CR0			= 0x00006c00,
	HOST_CR3			= 0x00006c02,
	HOST_CR4			= 0x00006c04,
	HOST_FS_BASE			= 0x00006c06,
	HOST_GS_BASE			= 0x00006c08,
	HOST_TR_BASE			= 0x00006c0a,
	HOST_GDTR_BASE			= 0x00006c0c,
	HOST_IDTR_BASE			= 0x00006c0e,
	HOST_IA32_SYSENTER_ESP		= 0x00006c10,
	HOST_IA32_SYSENTER_EIP		= 0x00006c12,
	HOST_RSP			= 0x00006c14,
	HOST_RIP			= 0x00006c16,
};

typedef struct {
	dump_vmcs12_t* d_vmcs12; 
	size_t exit_da_cui_ripartire;
} struct_da_cui_ripartire;

#endif