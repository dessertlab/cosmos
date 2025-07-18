#include "generic_ops.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

void initialize_vmexit(vmexit *exit) {
    // Inizializza la struttura vmexit e tutti i suoi campi a zero
    exit->start.name = 0;
    exit->start.value = 0;
    exit->start.type = 0;

    for (int i = 0; i < NUM_REGS; ++i) {
        exit->regs_in[i].name = 0;
        exit->regs_in[i].value = 0;
        exit->regs_in[i].type = 0;

        exit->regs_out[i].name = 0;
        exit->regs_out[i].value = 0;
        exit->regs_out[i].type = 0;
    }

    for (int i = 0; i < NUM_OPS; ++i) {
        exit->ops[i].field = 0;
        exit->ops[i].value = 0;
        exit->ops[i].type = 0;
    }

    exit->op_counter = 0;
}

void write_vmexit_to_file(FILE *file, vmexit exit, size_t i, uint8_t scarta) {

    size_t count_exit_scartate = 0; 

    if (exit.start.name == 0 && scarta == 1) {
        count_exit_scartate++;
        printf("Scartate: %zu exits, num: %zu.\n", count_exit_scartate, i);
        return; 
    }

    fprintf(file, "%x\n", exit.start.name);
    fprintf(file, "%lx\n", exit.start.value);
    fprintf(file, "%u\n", exit.op_counter);

    for (int j = 0; j < NUM_REGS; j++) {
        reg reg_in = exit.regs_in[j];
        fprintf(file, "%x\n", reg_in.name);
        fprintf(file, "%lx\n", reg_in.value);
        fprintf(file, "%u\n", reg_in.type);
    }

    uint8_t dim_ops_effettiva = sizeof(exit.ops) / sizeof(exit.ops[0]);
    for (int j = 0; j < exit.op_counter; j++) {
        if (j > dim_ops_effettiva) break; 
        op op_entry = exit.ops[j];
        fprintf(file, "%lx\n", op_entry.field);
        fprintf(file, "%llx\n", op_entry.value);
        fprintf(file, "%u\n", op_entry.type);
    }

    for (int j = 0; j < NUM_REGS; j++) {
        reg reg_out = exit.regs_out[j];
        fprintf(file, "%x\n", reg_out.name);
        fprintf(file, "%lx\n", reg_out.value);
        fprintf(file, "%u\n", reg_out.type);
    }
}

reg read_vmxexit_buffer_from_file_reg(FILE *file, size_t idx_exit) {
    reg tmp; 

    if (fscanf(file, "%x\n%lx\n%hhu\n", &tmp.name, &tmp.value,
                       &tmp.type) != 3) {
        printf("Error reading regs for exit: %zu. ", idx_exit);
        tmp.name = 0;
        tmp.value = 0; 
        tmp.type = 0; 
    }

    return tmp; 
}

op read_vmxexit_buffer_from_file_op(FILE *file, size_t idx_exit) {
    op tmp; 

    if (fscanf(file, "%lx\n%llx\n%hhu\n", &tmp.field, &tmp.value,
                       &tmp.type) != 3) {
        printf("Error reading op for exit: %zu. ", idx_exit);
    }

    return tmp; 
}

void read_vmexit_buffer_from_file(FILE *file, vmexit* b, size_t dim) {
    size_t i = 0; 

    while (i < dim) {
        b[i].start = read_vmxexit_buffer_from_file_reg(file, i);

        if (b[i].start.name == 0) break; 

        b[i].op_counter = b[i].start.type; 

        for (int j = 0; j < NUM_REGS; j++) {
            b[i].regs_in[j] = read_vmxexit_buffer_from_file_reg(file, i); 
        }

        for (int j = 0; j < b[i].op_counter; j++) {
            b[i].ops[j] = read_vmxexit_buffer_from_file_op(file, i); 
        }

        // NOT USEFUL WHEN IRIS SEED
        for (int j = 0; j < NUM_REGS; j++) {
            b[i].regs_out[j] = read_vmxexit_buffer_from_file_reg(file, i); 
        }

        i++;
    }
}

void print_vmexit (vmexit b) {
    size_t i = 0; 

    if (b.start.name == 0) {
        printf("SEED vuoto.\n");
        return; 
    }

    printf ("%x\n", b.start.name);
    printf ("%lx\n", b.start.value);
    printf ("%x\n", b.start.type);

    for (i = 0; i < NUM_REGS; i++) {
        printf ("%x\n", b.regs_in[i].name);
        printf ("%lx\n", b.regs_in[i].value);
        printf ("%x\n", b.regs_in[i].type);
    }

    for (i = 0; i < b.op_counter; i++) {
        printf ("%lx\n", b.ops[i].field);
        printf ("%llx\n", b.ops[i].value);
        printf ("%x\n", b.ops[i].type);
    }

    for (i = 0; i < NUM_REGS; i++) {
        printf ("%x\n", b.regs_out[i].name);
        printf ("%lx\n", b.regs_out[i].value);
        printf ("%x\n", b.regs_out[i].type);
    }
} 

void print_hashmap(const Exit_Reasons_count* hashmap) {
    size_t i;

    for (i = 0; i < 75; i++) {
        if (hashmap[i].count > 0) {
            printf("[%s]: %d.\n", hashmap[i].exit_reason, hashmap[i].count);
        }
    }
}

int is_in (int* exit_reasons, size_t dim, size_t exit_reason) {
    for (size_t i=0; i<dim; i++) {
        if (exit_reasons[i] == (int) exit_reason) return 1; 
    }
    return 0; 
}

void remove_exit (int* exit_reasons, size_t dim, size_t exit_reason) {
    for (size_t i=0; i<dim; i++) {
        if (exit_reasons[i] == (int) exit_reason) exit_reasons[i] = -1; 
    }
}

// Function to check if all elements of the array are equal to zero
int areAllElementsMinusOne(int* exit_reasons, size_t dim) {
    for (size_t i = 0; i< dim; i++) {
        if (exit_reasons[i] != -1) return 0;
    }

    return 1;
}

unsigned long long extract_exit_reason (vmexit e) {
    for (size_t i = 0; i < e.op_counter; i++) {
        if (e.ops[i].field == 0x4402)
            return e.ops[i].value; 
    }

    return 0; 
}

unsigned long long extract_guest_cr0 (size_t num_exit) {

    char command[200]; 
    sprintf(command, "cat /home/test/sync/trace/vmcs_dump.txt | grep -A 30 -E \"EXIT NUM %zu\" | grep \"guest_cr0\" | awk '/guest_cr0:/ {print $2}'", num_exit);
    FILE *pipe = popen(command, "r");

    char buffer[50];

    // Legge e stampa il risultato
    while (fgets(buffer, sizeof(buffer), pipe) != NULL);

    // Chiude la pipe
    if (pclose(pipe) == -1) {
        perror("Errore durante la chiusura della pipe");
        return 1;
    }

    unsigned long long result = strtoull(buffer, NULL, 16);

    return result; 
}

void* unpause_guest(void* arg) {
    UnpauseThreadData *threadData = (UnpauseThreadData *)arg;
    sem_wait(threadData->semaphore);
    printf("[THREAD] unpausing guest\n");

    char command[200]; ;
    sprintf(command, "sshpass -p test ssh -p 2222 root@%s \"/home/test/resume_qemu.sh\"", VM_IP);
    system(command);

    printf("%s.\n", command);

    free(threadData); 

    return NULL;
}

void init_hashmap(Exit_Reasons_count* hashmap) {
    // Initialize the hashmap with key-value pairs
    hashmap[0]  = (Exit_Reasons_count) {0,  "EXIT_REASON_EXCEPTION_NMI",       0};
    hashmap[1]  = (Exit_Reasons_count) {1,  "EXIT_REASON_EXTERNAL_INTERRUPT",  0};
    hashmap[2]  = (Exit_Reasons_count) {2,  "EXIT_REASON_TRIPLE_FAULT",        0};
    hashmap[3]  = (Exit_Reasons_count) {3,  "EXIT_REASON_INIT_SIGNAL",         0};
    hashmap[4]  = (Exit_Reasons_count) {4,  "EXIT_REASON_SIPI_SIGNAL",         0};
    hashmap[7]  = (Exit_Reasons_count) {7,  "EXIT_REASON_PENDING_VIRT_INTR",   0};
    hashmap[8]  = (Exit_Reasons_count) {8,  "EXIT_REASON_NMI_WINDOW",          0};
    hashmap[9]  = (Exit_Reasons_count) {9,  "EXIT_REASON_TASK_SWITCH",         0};
    hashmap[10] = (Exit_Reasons_count) {10, "EXIT_REASON_CPUID",               0};
    hashmap[12] = (Exit_Reasons_count) {12, "EXIT_REASON_HLT",                 0};
    hashmap[13] = (Exit_Reasons_count) {13, "EXIT_REASON_INVD",                0};
    hashmap[14] = (Exit_Reasons_count) {14, "EXIT_REASON_INVLPG",              0};
    hashmap[15] = (Exit_Reasons_count) {15, "EXIT_REASON_RDPMC",               0};
    hashmap[16] = (Exit_Reasons_count) {16, "EXIT_REASON_RDTSC",               0};
    hashmap[18] = (Exit_Reasons_count) {18, "EXIT_REASON_VMCALL",              0};
    hashmap[19] = (Exit_Reasons_count) {19, "EXIT_REASON_VMCLEAR",             0};
    hashmap[20] = (Exit_Reasons_count) {20, "EXIT_REASON_VMLAUNCH",            0};
    hashmap[21] = (Exit_Reasons_count) {21, "EXIT_REASON_VMPTRLD",             0};
    hashmap[22] = (Exit_Reasons_count) {22, "EXIT_REASON_VMPTRST",             0};
    hashmap[23] = (Exit_Reasons_count) {23, "EXIT_REASON_VMREAD",              0};
    hashmap[24] = (Exit_Reasons_count) {24, "EXIT_REASON_VMRESUME",            0};
    hashmap[25] = (Exit_Reasons_count) {25, "EXIT_REASON_VMWRITE",             0};
    hashmap[26] = (Exit_Reasons_count) {26, "EXIT_REASON_VMOFF",               0};
    hashmap[27] = (Exit_Reasons_count) {27, "EXIT_REASON_VMON",                0};
    hashmap[28] = (Exit_Reasons_count) {28, "EXIT_REASON_CR_ACCESS",           0};
    hashmap[29] = (Exit_Reasons_count) {29, "EXIT_REASON_DR_ACCESS",           0};
    hashmap[30] = (Exit_Reasons_count) {30, "EXIT_REASON_IO_INSTRUCTION",      0};
    hashmap[31] = (Exit_Reasons_count) {31, "EXIT_REASON_MSR_READ",            0};
    hashmap[32] = (Exit_Reasons_count) {32, "EXIT_REASON_MSR_WRITE",           0};
    hashmap[33] = (Exit_Reasons_count) {33, "EXIT_REASON_INVALID_STATE",       0};
    hashmap[34] = (Exit_Reasons_count) {34, "EXIT_REASON_MSR_LOAD_FAIL",       0};
    hashmap[36] = (Exit_Reasons_count) {36, "EXIT_REASON_MWAIT_INSTRUCTION",   0};
    hashmap[37] = (Exit_Reasons_count) {37, "EXIT_REASON_MONITOR_TRAP_FLAG",   0};
    hashmap[39] = (Exit_Reasons_count) {39, "EXIT_REASON_MONITOR_INSTRUCTION", 0};
    hashmap[40] = (Exit_Reasons_count) {40, "EXIT_REASON_PAUSE_INSTRUCTION",   0};
    hashmap[41] = (Exit_Reasons_count) {41, "EXIT_REASON_MCE_DURING_VMENTRY",  0};
    hashmap[43] = (Exit_Reasons_count) {43, "EXIT_REASON_TPR_BELOW_THRESHOLD", 0};
    hashmap[44] = (Exit_Reasons_count) {44, "EXIT_REASON_APIC_ACCESS",         0};
    hashmap[45] = (Exit_Reasons_count) {45, "EXIT_REASON_EOI_INDUCED",         0};
    hashmap[46] = (Exit_Reasons_count) {46, "EXIT_REASON_GDTR_IDTR",           0};
    hashmap[47] = (Exit_Reasons_count) {47, "EXIT_REASON_LDTR_TR",             0};
    hashmap[48] = (Exit_Reasons_count) {48, "EXIT_REASON_EPT_VIOLATION",       0};
    hashmap[49] = (Exit_Reasons_count) {49, "EXIT_REASON_EPT_MISCONFIG",       0};
    hashmap[50] = (Exit_Reasons_count) {50, "EXIT_REASON_INVEPT",              0};
    hashmap[51] = (Exit_Reasons_count) {51, "EXIT_REASON_RDTSCP",              0};
    hashmap[52] = (Exit_Reasons_count) {52, "EXIT_REASON_PREEMPTION_TIMER",    0};
    hashmap[53] = (Exit_Reasons_count) {53, "EXIT_REASON_INVVPID",             0};
    hashmap[54] = (Exit_Reasons_count) {54, "EXIT_REASON_WBINVD",              0};
    hashmap[55] = (Exit_Reasons_count) {55, "EXIT_REASON_XSETBV",              0};
    hashmap[56] = (Exit_Reasons_count) {56, "EXIT_REASON_APIC_WRITE",          0};
    hashmap[57] = (Exit_Reasons_count) {57, "EXIT_REASON_RDRAND",              0};
    hashmap[58] = (Exit_Reasons_count) {58, "EXIT_REASON_INVPCID",             0};
    hashmap[59] = (Exit_Reasons_count) {59, "EXIT_REASON_VMFUNC",              0};
    hashmap[60] = (Exit_Reasons_count) {60, "EXIT_REASON_ENCLS",               0};
    hashmap[61] = (Exit_Reasons_count) {61, "EXIT_REASON_RDSEED",              0};
    hashmap[62] = (Exit_Reasons_count) {62, "EXIT_REASON_PML_FULL",            0};
    hashmap[63] = (Exit_Reasons_count) {63, "EXIT_REASON_XSAVES",              0};
    hashmap[64] = (Exit_Reasons_count) {64, "EXIT_REASON_XRSTORS",             0};
    hashmap[67] = (Exit_Reasons_count) {67, "EXIT_REASON_UMWAIT",              0};
    hashmap[68] = (Exit_Reasons_count) {68, "EXIT_REASON_TPAUSE",              0};
    hashmap[74] = (Exit_Reasons_count) {74, "EXIT_REASON_BUS_LOCK",            0};
}

void print_seed (const char* filepath_input, const char* filepath_output, size_t num_exits) {
    vmexit* buffer_seed = malloc (num_exits * sizeof (vmexit));
    if (buffer_seed == NULL) {
        printf("Cannot allocate buffer_record_replay in memory.\n");
        return;
    }
    FILE* file_input = fopen(filepath_input, "r");
    if (file_input == NULL) {
        printf ("File seeds input not existant.\n");
        return;
    }
    FILE* file_output = fopen(filepath_output, "w+");
    if (file_output == NULL) {
        printf ("File seeds output not existant.\n");
        return;
    }

    read_vmexit_buffer_from_file(file_input, buffer_seed, num_exits);

    for (size_t i = 0; i < num_exits; i++) {
        fprintf(file_output, "SEED #%zu.\n", i);

        for (size_t j = 0; j < buffer_seed[i].op_counter; j++) {
            if (buffer_seed[i].ops[j].type == 0) {
                fprintf(file_output, "RD: \t FIELD(%lx), \t VALUE(%llx).\n", 
                    buffer_seed[i].ops[j].field, buffer_seed[i].ops[j].value);
            }
            else if (buffer_seed[i].ops[j].type == 1) {
                fprintf(file_output, "WR: \t FIELD(%lx), \t VALUE(%llx).\n", 
                    buffer_seed[i].ops[j].field, buffer_seed[i].ops[j].value);
            }
        }

        for (size_t j = 0; j < NUM_REGS; j++) {
            fprintf(file_output, "REG: \t NAME(%s), \t\t I_VALUE(%lx), O_VALUE(%lx)\n", 
                registers[j], buffer_seed[i].regs_in[j].value,
                buffer_seed[i].regs_out[j].value);
        }

        fprintf(file_output, "EXIT REASON: %s.\n\n", exit_reason_names[extract_exit_reason(buffer_seed[i])]);
    }

    fclose(file_output);
    fclose(file_input); 
    free(buffer_seed); 
}

struct_da_cui_ripartire restoreAndWait(const char* snap_name, int rnr_fd, int fuzz_fd, size_t buffer_size,
    unsigned long long exit_reason, unsigned long long guest_cr0) {
    char command[200];

    ioctl(rnr_fd, CONTROL_EXEC, 1);
    ioctl(rnr_fd, SET_DUMP_VMCS, 1); 
    ioctl(rnr_fd, TRACING_CONTROL, 1); 

    struct_da_cui_ripartire to_ret;
    to_ret.d_vmcs12 = NULL; 
    to_ret.exit_da_cui_ripartire = 0; 

    for (size_t i = 0; i < buffer_size; i++) {
        to_ret.d_vmcs12 = malloc(sizeof(dump_vmcs12_t));

        if (i == 0) {
            sprintf(command, "echo \"loadvm %s\" | sudo socat - unix-connect:/home/test/test_snapshot/%s/qemu-monitor-socket", snap_name, HYP);
            system(command);
        }

        while (ioctl(fuzz_fd, DUMP_VMCS12, to_ret.d_vmcs12) == 0);
        
        if (to_ret.d_vmcs12->guest_cr0 == guest_cr0 && to_ret.d_vmcs12->vm_exit_reason == exit_reason) {
            printf("Dump exit: %lx.\n", to_ret.d_vmcs12->vm_exit_reason);
            to_ret.exit_da_cui_ripartire = i;
            return to_ret;
        }

        ioctl(rnr_fd, SIGNAL_SEM_DUMP_rec, 0); 
        
        free(to_ret.d_vmcs12);
    }

    return to_ret; 
}

unsigned long long getVMCSField(unsigned int field, const dump_vmcs12_t* vmcs) {
    switch (field) {
        case 0x00000000:
            return vmcs->virtual_processor_id;
        case 0x00000002:
            return vmcs->posted_intr_nv;
        case 0x00000800:
            return vmcs->guest_es_selector;
        case 0x00000802:
            return vmcs->guest_cs_selector;
        case 0x00000804:
            return vmcs->guest_ss_selector;
        case 0x00000806:
            return vmcs->guest_ds_selector;
        case 0x00000808:
            return vmcs->guest_fs_selector;
        case 0x0000080a:
            return vmcs->guest_gs_selector;
        case 0x0000080c:
            return vmcs->guest_ldtr_selector;
        case 0x0000080e:
            return vmcs->guest_tr_selector;
        case 0x00000810:
            return vmcs->guest_intr_status;
        case 0x00000812:
            return vmcs->guest_pml_index;
        case 0x00000c00:
            return vmcs->host_es_selector;
        case 0x00000c02:
            return vmcs->host_cs_selector;
        case 0x00000c04:
            return vmcs->host_ss_selector;
        case 0x00000c06:
            return vmcs->host_ds_selector;
        case 0x00000c08:
            return vmcs->host_fs_selector;
        case 0x00000c0a:
            return vmcs->host_gs_selector;
        case 0x00000c0c:
            return vmcs->host_tr_selector;
        case 0x00004000:
            return vmcs->pin_based_vm_exec_control;
        case 0x00004002:
            return vmcs->cpu_based_vm_exec_control;
        case 0x00004004:
            return vmcs->exception_bitmap;
        case 0x00004006:
            return vmcs->page_fault_error_code_mask;
        case 0x00004008:
            return vmcs->page_fault_error_code_match;
        case 0x0000400a:
            return vmcs->cr3_target_count;
        case 0x0000400c:
            return vmcs->vm_exit_controls;
        case 0x0000400e:
            return vmcs->vm_exit_msr_store_count;
        case 0x00004010:
            return vmcs->vm_exit_msr_load_count;
        case 0x00004012:
            return vmcs->vm_entry_controls;
        case 0x00004014:
            return vmcs->vm_entry_msr_load_count;
        case 0x00004016:
            return vmcs->vm_entry_intr_info_field;
        case 0x00004018:
            return vmcs->vm_entry_exception_error_code;
        case 0x0000401a:
            return vmcs->vm_entry_instruction_len;
        case 0x0000401c:
            return vmcs->tpr_threshold;
        case 0x0000401e:
            return vmcs->secondary_vm_exec_control;
        case 0x00004400:
            return vmcs->vm_instruction_error;
        case 0x00004402:
            return vmcs->vm_exit_reason;
        case 0x00004404:
            return vmcs->vm_exit_intr_info;
        case 0x00004406:
            return vmcs->vm_exit_intr_error_code;
        case 0x00004408:
            return vmcs->idt_vectoring_info_field;
        case 0x0000440a:
            return vmcs->idt_vectoring_error_code;
        case 0x0000440c:
            return vmcs->vm_exit_instruction_len;
        case 0x0000440e:
            return vmcs->vmx_instruction_info;
        case 0x00004800:
            return vmcs->guest_es_limit;
        case 0x00004802:
            return vmcs->guest_cs_limit;
        case 0x00004804:
            return vmcs->guest_ss_limit;
        case 0x00004806:
            return vmcs->guest_ds_limit;
        case 0x00004808:
            return vmcs->guest_fs_limit;
        case 0x0000480a:
            return vmcs->guest_gs_limit;
        case 0x0000480c:
            return vmcs->guest_ldtr_limit;
        case 0x0000480e:
            return vmcs->guest_tr_limit;
        case 0x00004810:
            return vmcs->guest_gdtr_limit;
        case 0x00004812:
            return vmcs->guest_idtr_limit;
        case 0x00004814:
            return vmcs->guest_es_ar_bytes;
        case 0x00004816:
            return vmcs->guest_cs_ar_bytes;
        case 0x00004818:
            return vmcs->guest_ss_ar_bytes;
        case 0x0000481a:
            return vmcs->guest_ds_ar_bytes;
        case 0x0000481c:
            return vmcs->guest_fs_ar_bytes;
        case 0x0000481e:
            return vmcs->guest_gs_ar_bytes;
        case 0x00004820:
            return vmcs->guest_ldtr_ar_bytes;
        case 0x00004822:
            return vmcs->guest_tr_ar_bytes;
        case 0x00004824:
            return vmcs->guest_interruptibility_info;
        case 0x00004826:
            return vmcs->guest_activity_state;
        case 0x0000482a:
            return vmcs->guest_sysenter_cs;
        case 0x0000482e:
            return vmcs->vmx_preemption_timer_value;
        case 0x00004c00:
            return vmcs->host_ia32_sysenter_cs;
        case 0x00006000:
            return vmcs->cr0_guest_host_mask;
        case 0x00006002:
            return vmcs->cr4_guest_host_mask;
        case 0x00006004:
            return vmcs->cr0_read_shadow;
        case 0x00006006:
            return vmcs->cr4_read_shadow;
            // Valore esadecimale: vmcs->cr4_read_shadow
            break;
        case 0x00006010:
            return vmcs->exit_qualification;
            // Valore esadecimale: vmcs->exit_qualification
            break;
        case 0x00006012:
            return vmcs->guest_linear_address;
            // Valore esadecimale: vmcs->guest_linear_address
            break;
        case 0x00006400:
            return vmcs->exit_qualification;
            break; 
        case 0x00006800:
            return vmcs->guest_cr0;
            // Valore esadecimale: vmcs->guest_cr0
            break;
        case 0x00006802:
            return vmcs->guest_cr3;
            // Valore esadecimale: vmcs->guest_cr3
            break;
        case 0x00006804:
            return vmcs->guest_cr4;
            // Valore esadecimale: vmcs->guest_cr4
            break;
        case 0x00006806:
            return vmcs->guest_es_base;
            // Valore esadecimale: vmcs->guest_es_base
            break;
        case 0x00006808:
            return vmcs->guest_cs_base;
            // Valore esadecimale: vmcs->guest_cs_base
            break;
        case 0x0000680A:
            return vmcs->guest_ss_base;
            // Valore esadecimale: vmcs->guest_ss_base
            break;
        case 0x0000680C:
            return vmcs->guest_ds_base;
            // Valore esadecimale: vmcs->guest_ds_base
            break;
        case 0x0000680E:
            return vmcs->guest_fs_base;
            // Valore esadecimale: vmcs->guest_fs_base
            break;
        case 0x00006810:
            return vmcs->guest_gs_base;
            // Valore esadecimale: vmcs->guest_gs_base
            break;
        case 0x00006812:
            return vmcs->guest_ldtr_base;
            // Valore esadecimale: vmcs->guest_ldtr_base
            break;
        case 0x00006814:
            return vmcs->guest_tr_base;
            // Valore esadecimale: vmcs->guest_tr_base
            break;
        case 0x00006816:
            return vmcs->guest_gdtr_base;
            // Valore esadecimale: vmcs->guest_gdtr_base
            break;
        case 0x00006818:
            return vmcs->guest_idtr_base;
            // Valore esadecimale: vmcs->guest_idtr_base
            break;
        case 0x0000681A:
            return vmcs->guest_dr7;
            // Valore esadecimale: vmcs->guest_dr7
            break;
        case 0x0000681C:
            return vmcs->guest_rsp;
            // Valore esadecimale: vmcs->guest_rsp
            break;
        case 0x0000681E:
            return vmcs->guest_rip;
            // Valore esadecimale: vmcs->guest_rip
            break;
        case 0x00006820:
            return vmcs->guest_rflags;
            // Valore esadecimale: vmcs->guest_rflags
            break;
        case 0x00006822:
            return vmcs->guest_pending_dbg_exceptions;
            // Valore esadecimale: vmcs->guest_pending_dbg_exceptions
            break;
        case 0x00006824:
            return vmcs->guest_sysenter_esp;
            // Valore esadecimale: vmcs->guest_sysenter_esp
            break;
        case 0x00006826:
            return vmcs->guest_sysenter_eip;
            // Valore esadecimale: vmcs->guest_sysenter_eip
            break;
        case 0x00006C00:
            return vmcs->host_cr0;
            // Valore esadecimale: vmcs->host_cr0
            break;
        case 0x00006C02:
            return vmcs->host_cr3;
            // Valore esadecimale: vmcs->host_cr3
            break;
        case 0x00006C04:
            return vmcs->host_cr4;
            // Valore esadecimale: vmcs->host_cr4
            break;
        case 0x00006C06:
            return vmcs->host_fs_base;
            // Valore esadecimale: vmcs->host_fs_base
            break;
        case 0x00006C08:
            return vmcs->host_gs_base;
            // Valore esadecimale: vmcs->host_gs_base
            break;
        case 0x00006C0A:
            return vmcs->host_tr_base;
            // Valore esadecimale: vmcs->host_tr_base
            break;
        case 0x00006C0C:
            return vmcs->host_gdtr_base;
            // Valore esadecimale: vmcs->host_gdtr_base
            break;
        case 0x00006C0E:
            return vmcs->host_idtr_base;
            // Valore esadecimale: vmcs->host_idtr_base
            break;
        case 0x00006C10:
            return vmcs->host_ia32_sysenter_esp;
            // Valore esadecimale: vmcs->host_ia32_sysenter_esp
            break;
        case 0x00006C12:
            return vmcs->host_ia32_sysenter_eip;
            // Valore esadecimale: vmcs->host_ia32_sysenter_eip
            break;
        case 0x00006C14:
            return vmcs->host_rsp;
            // Valore esadecimale: vmcs->host_rsp
            break;
        case 0x00006C16:
            return vmcs->host_rip;
            // Valore esadecimale: vmcs->host_rip
            break;
        default: 
            if (field < sizeof(VMCS_FIELDS) / sizeof(VMCS_FIELDS[0]) && VMCS_FIELDS[field] != NULL) {
                printf("Field not implemented: %s\n", VMCS_FIELDS[field]);
            } else {
                printf("Field not found: 0x%08X\n", field);
            }
            break;
        }
    return 0xffffffffffffffff; 
}

size_t getVMCSField_Size(unsigned int field, const dump_vmcs12_t* vmcs) {
    switch (field) {
        case 0x00000000:
            return sizeof(vmcs->virtual_processor_id);
        case 0x00000002:
            return sizeof(vmcs->posted_intr_nv);
        case 0x00000800:
            return sizeof(vmcs->guest_es_selector);
        case 0x00000802:
            return sizeof(vmcs->guest_cs_selector);
        case 0x00000804:
            return sizeof(vmcs->guest_ss_selector);
        case 0x00000806:
            return sizeof(vmcs->guest_ds_selector);
        case 0x00000808:
            return sizeof(vmcs->guest_fs_selector);
        case 0x0000080a:
            return sizeof(vmcs->guest_gs_selector);
        case 0x0000080c:
            return sizeof(vmcs->guest_ldtr_selector);
        case 0x0000080e:
            return sizeof(vmcs->guest_tr_selector);
        case 0x00000810:
            return sizeof(vmcs->guest_intr_status);
        case 0x00000812:
            return sizeof(vmcs->guest_pml_index);
        case 0x00000c00:
            return sizeof(vmcs->host_es_selector);
        case 0x00000c02:
            return sizeof(vmcs->host_cs_selector);
        case 0x00000c04:
            return sizeof(vmcs->host_ss_selector);
        case 0x00000c06:
            return sizeof(vmcs->host_ds_selector);
        case 0x00000c08:
            return sizeof(vmcs->host_fs_selector);
        case 0x00000c0a:
            return sizeof(vmcs->host_gs_selector);
        case 0x00000c0c:
            return sizeof(vmcs->host_tr_selector);
        case 0x00004000:
            return sizeof(vmcs->pin_based_vm_exec_control);
        case 0x00004002:
            return sizeof(vmcs->cpu_based_vm_exec_control);
        case 0x00004004:
            return sizeof(vmcs->exception_bitmap);
        case 0x00004006:
            return sizeof(vmcs->page_fault_error_code_mask);
        case 0x00004008:
            return sizeof(vmcs->page_fault_error_code_match);
        case 0x0000400a:
            return sizeof(vmcs->cr3_target_count);
        case 0x0000400c:
            return sizeof(vmcs->vm_exit_controls);
        case 0x0000400e:
            return sizeof(vmcs->vm_exit_msr_store_count);
        case 0x00004010:
            return sizeof(vmcs->vm_exit_msr_load_count);
        case 0x00004012:
            return sizeof(vmcs->vm_entry_controls);
        case 0x00004014:
            return sizeof(vmcs->vm_entry_msr_load_count);
        case 0x00004016:
            return sizeof(vmcs->vm_entry_intr_info_field);
        case 0x00004018:
            return sizeof(vmcs->vm_entry_exception_error_code);
        case 0x0000401a:
            return sizeof(vmcs->vm_entry_instruction_len);
        case 0x0000401c:
            return sizeof(vmcs->tpr_threshold);
        case 0x0000401e:
            return sizeof(vmcs->secondary_vm_exec_control);
        case 0x00004400:
            return sizeof(vmcs->vm_instruction_error);
        case 0x00004402:
            return sizeof(vmcs->vm_exit_reason);
        case 0x00004404:
            return sizeof(vmcs->vm_exit_intr_info);
        case 0x00004406:
            return sizeof(vmcs->vm_exit_intr_error_code);
        case 0x00004408:
            return sizeof(vmcs->idt_vectoring_info_field);
        case 0x0000440a:
            return sizeof(vmcs->idt_vectoring_error_code);
        case 0x0000440c:
            return sizeof(vmcs->vm_exit_instruction_len);
        case 0x0000440e:
            return sizeof(vmcs->vmx_instruction_info);
        case 0x00004800:
            return sizeof(vmcs->guest_es_limit);
        case 0x00004802:
            return sizeof(vmcs->guest_cs_limit);
        case 0x00004804:
            return sizeof(vmcs->guest_ss_limit);
        case 0x00004806:
            return sizeof(vmcs->guest_ds_limit);
        case 0x00004808:
            return sizeof(vmcs->guest_fs_limit);
        case 0x0000480a:
            return sizeof(vmcs->guest_gs_limit);
        case 0x0000480c:
            return sizeof(vmcs->guest_ldtr_limit);
        case 0x0000480e:
            return sizeof(vmcs->guest_tr_limit);
        case 0x00004810:
            return sizeof(vmcs->guest_gdtr_limit);
        case 0x00004812:
            return sizeof(vmcs->guest_idtr_limit);
        case 0x00004814:
            return sizeof(vmcs->guest_es_ar_bytes);
        case 0x00004816:
            return sizeof(vmcs->guest_cs_ar_bytes);
        case 0x00004818:
            return sizeof(vmcs->guest_ss_ar_bytes);
        case 0x0000481a:
            return sizeof(vmcs->guest_ds_ar_bytes);
        case 0x0000481c:
            return sizeof(vmcs->guest_fs_ar_bytes);
        case 0x0000481e:
            return sizeof(vmcs->guest_gs_ar_bytes);
        case 0x00004820:
            return sizeof(vmcs->guest_ldtr_ar_bytes);
        case 0x00004822:
            return sizeof(vmcs->guest_tr_ar_bytes);
        case 0x00004824:
            return sizeof(vmcs->guest_interruptibility_info);
        case 0x00004826:
            return sizeof(vmcs->guest_activity_state);
        case 0x0000482a:
            return sizeof(vmcs->guest_sysenter_cs);
        case 0x0000482e:
            return sizeof(vmcs->vmx_preemption_timer_value);
        case 0x00004c00:
            return sizeof(vmcs->host_ia32_sysenter_cs);
        case 0x00006000:
            return sizeof(vmcs->cr0_guest_host_mask);
        case 0x00006002:
            return sizeof(vmcs->cr4_guest_host_mask);
        case 0x00006004:
            return sizeof(vmcs->cr0_read_shadow);
        case 0x00006006:
            return sizeof(vmcs->cr4_read_shadow);
            // Valore esadecimale: vmcs->cr4_read_shadow
            break;
        case 0x00006010:
            return sizeof(vmcs->exit_qualification);
            // Valore esadecimale: vmcs->exit_qualification
            break;
        case 0x00006012:
            return sizeof(vmcs->guest_linear_address);
            // Valore esadecimale: vmcs->guest_linear_address
            break;
        case 0x00006400:
            return sizeof(vmcs->exit_qualification);
            break; 
        case 0x00006800:
            return sizeof(vmcs->guest_cr0);
            // Valore esadecimale: vmcs->guest_cr0
            break;
        case 0x00006802:
            return sizeof(vmcs->guest_cr3);
            // Valore esadecimale: vmcs->guest_cr3
            break;
        case 0x00006804:
            return sizeof(vmcs->guest_cr4);
            // Valore esadecimale: vmcs->guest_cr4
            break;
        case 0x00006806:
            return sizeof(vmcs->guest_es_base);
            // Valore esadecimale: vmcs->guest_es_base
            break;
        case 0x00006808:
            return sizeof(vmcs->guest_cs_base);
            // Valore esadecimale: vmcs->guest_cs_base
            break;
        case 0x0000680A:
            return sizeof(vmcs->guest_ss_base);
            // Valore esadecimale: vmcs->guest_ss_base
            break;
        case 0x0000680C:
            return sizeof(vmcs->guest_ds_base);
            // Valore esadecimale: vmcs->guest_ds_base
            break;
        case 0x0000680E:
            return sizeof(vmcs->guest_fs_base);
            // Valore esadecimale: vmcs->guest_fs_base
            break;
        case 0x00006810:
            return sizeof(vmcs->guest_gs_base);
            // Valore esadecimale: vmcs->guest_gs_base
            break;
        case 0x00006812:
            return sizeof(vmcs->guest_ldtr_base);
            // Valore esadecimale: vmcs->guest_ldtr_base
            break;
        case 0x00006814:
            return sizeof(vmcs->guest_tr_base);
            // Valore esadecimale: vmcs->guest_tr_base
            break;
        case 0x00006816:
            return sizeof(vmcs->guest_gdtr_base);
            // Valore esadecimale: vmcs->guest_gdtr_base
            break;
        case 0x00006818:
            return sizeof(vmcs->guest_idtr_base);
            // Valore esadecimale: vmcs->guest_idtr_base
            break;
        case 0x0000681A:
            return sizeof(vmcs->guest_dr7);
            // Valore esadecimale: vmcs->guest_dr7
            break;
        case 0x0000681C:
            return sizeof(vmcs->guest_rsp);
            // Valore esadecimale: vmcs->guest_rsp
            break;
        case 0x0000681E:
            return sizeof(vmcs->guest_rip);
            // Valore esadecimale: vmcs->guest_rip
            break;
        case 0x00006820:
            return sizeof(vmcs->guest_rflags);
            // Valore esadecimale: vmcs->guest_rflags
            break;
        case 0x00006822:
            return sizeof(vmcs->guest_pending_dbg_exceptions);
            // Valore esadecimale: vmcs->guest_pending_dbg_exceptions
            break;
        case 0x00006824:
            return sizeof(vmcs->guest_sysenter_esp);
            // Valore esadecimale: vmcs->guest_sysenter_esp
            break;
        case 0x00006826:
            return sizeof(vmcs->guest_sysenter_eip);
            // Valore esadecimale: vmcs->guest_sysenter_eip
            break;
        case 0x00006C00:
            return sizeof(vmcs->host_cr0);
            // Valore esadecimale: vmcs->host_cr0
            break;
        case 0x00006C02:
            return sizeof(vmcs->host_cr3);
            // Valore esadecimale: vmcs->host_cr3
            break;
        case 0x00006C04:
            return sizeof(vmcs->host_cr4);
            // Valore esadecimale: vmcs->host_cr4
            break;
        case 0x00006C06:
            return sizeof(vmcs->host_fs_base);
            // Valore esadecimale: vmcs->host_fs_base
            break;
        case 0x00006C08:
            return sizeof(vmcs->host_gs_base);
            // Valore esadecimale: vmcs->host_gs_base
            break;
        case 0x00006C0A:
            return sizeof(vmcs->host_tr_base);
            // Valore esadecimale: vmcs->host_tr_base
            break;
        case 0x00006C0C:
            return sizeof(vmcs->host_gdtr_base);
            // Valore esadecimale: vmcs->host_gdtr_base
            break;
        case 0x00006C0E:
            return sizeof(vmcs->host_idtr_base);
            // Valore esadecimale: vmcs->host_idtr_base
            break;
        case 0x00006C10:
            return sizeof(vmcs->host_ia32_sysenter_esp);
            // Valore esadecimale: vmcs->host_ia32_sysenter_esp
            break;
        case 0x00006C12:
            return sizeof(vmcs->host_ia32_sysenter_eip);
            // Valore esadecimale: vmcs->host_ia32_sysenter_eip
            break;
        case 0x00006C14:
            return sizeof(vmcs->host_rsp);
            // Valore esadecimale: vmcs->host_rsp
            break;
        case 0x00006C16:
            return sizeof(vmcs->host_rip);
            // Valore esadecimale: vmcs->host_rip
            break;
        default: 
            if (field < sizeof(VMCS_FIELDS) / sizeof(VMCS_FIELDS[0]) && VMCS_FIELDS[field] != NULL) {
                printf("Field not implemented: %s\n", VMCS_FIELDS[field]);
            } else {
                printf("Field not found: 0x%08X\n", field);
            }
            break;
        }
    return 0xffffffffffffffff; 
}
