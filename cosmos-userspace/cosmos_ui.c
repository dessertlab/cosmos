#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <linux/sched.h>

#include "generic_ops.h"
#include "types.h"
#include "injector/operation_lib.h"

#define DEVICE_REC_PATH "/dev/cosmos_rec"
#define DEVICE_UT_PATH "/dev/cosmos_ut"
#define DEVICE_INJECTOR_PATH "/dev/cosmos_fi"

#define BUFFER_SIZE 500000  // Default buffer size
#define COSMOS_USER_PATH "."

void print_menu();
void stampa_vmcs_su_file(const char* filepath, dump_vmcs12_t* vmcs, size_t header);

int injector_fd;
int fd;
int ut_fd;

vmexit* buffer_seeds = NULL; 
vmexit* to_inject_buffer = NULL; 

// Signal handler function
void signalHandler(int signum) {
    if (signum == SIGINT) {
        printf("Received Ctrl+C. Cleaning up and terminating...\n");
        close(fd);
        close(injector_fd);
        if (buffer_seeds != NULL) free (buffer_seeds); 
        if (to_inject_buffer != NULL) free (to_inject_buffer);
        exit(signum);
    }
    else if (signum == SIGUSR1) {
        printf("Segnale SIGUSR1 catturato. Continua l'esecuzione.\n");
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGUSR1, signalHandler);
    fd = open(DEVICE_REC_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    ut_fd = open(DEVICE_UT_PATH, O_RDWR);
    if (ut_fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    injector_fd = open(DEVICE_INJECTOR_PATH, O_RDWR);
    if (injector_fd < 0) {
        perror("Failed to open the injector device file");
        return -1;
    }

    unsigned int buffer_size = BUFFER_SIZE;
    unsigned int enable_tracing = 0;
    unsigned long from_exit = 1;

    while (1) {
        print_menu();

        int choice;
        printf("Buffer size: %d. Tracing: %d. Enter your choice: ", buffer_size, enable_tracing);
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter buffer size: ");
                scanf("%u", &buffer_size);
                ioctl(fd, SET_BUFFER_SIZE, buffer_size);
                break;
            case 2:
                ioctl(fd, RESET, 0);
                break;
            case 3:
                uint8_t dump_vmcs12 = 0; 
                uint8_t retrieve_rec_performance = 0; 
            
                printf("Enable tracing? (0/1): ");
                scanf("%u", &enable_tracing);
                from_exit = 0; 

                if (enable_tracing == 0)
                    break; 
                else {
                    printf("From which # exit (at least from 1)? ");
                    scanf("%lu", &from_exit);

                    printf("Enable dump vmcs12? "); 
                    scanf("%hhu", &dump_vmcs12);

                    printf("Retrieve performance? "); 
                    scanf("%hhu", &retrieve_rec_performance);
                }

                char vmcs_dump_path[100]; 
                char command[200]; 

                ioctl(fd, SET_FROM_EXIT, from_exit);
                ioctl(fd, SET_DUMP_VMCS, dump_vmcs12);
                ioctl(fd, TRACING_CONTROL, enable_tracing);

                if (retrieve_rec_performance) ioctl(fd, CREATE_EFF_REC_BUFF, 0);

                if (dump_vmcs12) {
                    sprintf(command, "rm -f %s/vmcs_dump.txt", COSMOS_USER_PATH);
                    system(command);
                    sprintf(vmcs_dump_path, "%s/vmcs_dump.txt", COSMOS_USER_PATH);
                    for (size_t i = 0; i < buffer_size; i++) {
                        dump_vmcs12_t* d_vmcs12 = malloc(sizeof(dump_vmcs12_t));

                        while (ioctl(ut_fd, DUMP_VMCS12, d_vmcs12) == 0) sleep (0.1);

                        // printf("[Guest CR0] %llu; [Host CR0] %llu.\n", d_vmcs12->guest_cr0, d_vmcs12->host_cr0);
                        stampa_vmcs_su_file(vmcs_dump_path, d_vmcs12, i);

                        ioctl(fd, SIGNAL_SEM_DUMP_rec, 0); 
                        
                        free(d_vmcs12);
                    }
                }

                break;

            case 4: {
                vmexit* buffer_to_read = malloc (buffer_size * sizeof (vmexit));
                if (buffer_to_read == NULL) {
                    printf("Cannot allocate buffer in memory.\n");
                    break; 
                }

                ssize_t bytes_read = read(fd, buffer_to_read, buffer_size);
                if (bytes_read < 0) {
                    perror("Failed to read from the buffer");
                    close(fd);
                    return -1;
                }

                printf("Read %zd bytes from the buffer\n", bytes_read);
                
                FILE *file = fopen("seeds", "w");
                if (file == NULL) {
                    printf ("File seeds not existant.\n");
                    break;
                }

                // Could be changed to handle the possibility of recording num_vmexits < buffer_size
                size_t num_vmexits = buffer_size;
                for (size_t i = 0; i < num_vmexits; i++) {
                    vmexit exit = buffer_to_read[i];
                    write_vmexit_to_file (file, exit, i, 1);
                }

                file = fopen("seeds_back", "w");
                if (file == NULL) {
                    printf ("File seeds_back not existant.\n");
                    break;
                }

                // Could be changed to handle the possibility of recording num_vmexits < buffer_size
                for (size_t i = 0; i < num_vmexits; i++) {
                    vmexit exit = buffer_to_read[i];
                    write_vmexit_to_file (file, exit, i, 1);
                }

                fclose(file);
                free(buffer_to_read); 

                break;
            }
            case 5: 
                size_t num_exits = 0;
                char file_input[200];
                char file_output[200];
                

                printf("Insert file_input: ");
                scanf("%199s", file_input);
                printf("Insert file_output: ");
                scanf("%199s", file_output);
                printf("Insert num exits: ");
                scanf("%zu", &num_exits);

                print_seed (file_input, file_output, num_exits);

                break;

            case 9: 
                close(fd);
                close(injector_fd);
                return 0;
                break;
            default:
                printf("Invalid choice\n");
                break; 
        }

        printf("\n");
    }
}

void print_menu() {
    printf("1) Buffer size.\n");
    printf("2) Reset buffer.\n");
    printf("3) Tracing.\n");
    printf("4) Read data on seeds.txt file.\n");
    printf("5) Print seeds.txt file.\n");
    printf("9) Exit.\n");
}

void stampa_vmcs_su_file(const char* filepath, dump_vmcs12_t* vmcs, size_t header) {
    if (filepath == NULL || vmcs == NULL) {
        return;
    }

    FILE* file = fopen(filepath, "a");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return;
    }

    fprintf(file, "EXIT NUM %zu.\n", header);
    fprintf(file, "cr0_guest_host_mask: 0x%llx\n", vmcs->cr0_guest_host_mask);
    fprintf(file, "cr4_guest_host_mask: 0x%llx\n", vmcs->cr4_guest_host_mask);
    fprintf(file, "cr0_read_shadow: 0x%llx\n", vmcs->cr0_read_shadow);
    fprintf(file, "cr4_read_shadow: 0x%llx\n", vmcs->cr4_read_shadow);
    fprintf(file, "exit_reason: %s\n", exit_reason_names[vmcs->vm_exit_reason]);
    fprintf(file, "guest_linear_address: 0x%llx\n", vmcs->guest_linear_address);
    fprintf(file, "guest_cr0: 0x%llx\n", vmcs->guest_cr0);
    fprintf(file, "guest_cr3: 0x%llx\n", vmcs->guest_cr3);
    fprintf(file, "guest_cr4: 0x%llx\n", vmcs->guest_cr4);
    fprintf(file, "guest_es_base: 0x%llx\n", vmcs->guest_es_base);
    fprintf(file, "guest_cs_base: 0x%llx\n", vmcs->guest_cs_base);
    fprintf(file, "guest_ss_base: 0x%llx\n", vmcs->guest_ss_base);
    fprintf(file, "guest_ds_base: 0x%llx\n", vmcs->guest_ds_base);
    fprintf(file, "guest_fs_base: 0x%llx\n", vmcs->guest_fs_base);
    fprintf(file, "guest_gs_base: 0x%llx\n", vmcs->guest_gs_base);
    fprintf(file, "guest_ldtr_base: 0x%llx\n", vmcs->guest_ldtr_base);
    fprintf(file, "guest_tr_base: 0x%llx\n", vmcs->guest_tr_base);
    fprintf(file, "guest_gdtr_base: 0x%llx\n", vmcs->guest_gdtr_base);
    fprintf(file, "guest_idtr_base: 0x%llx\n", vmcs->guest_idtr_base);
    fprintf(file, "guest_dr7: 0x%llx\n", vmcs->guest_dr7);
    fprintf(file, "guest_rsp: 0x%llx\n", vmcs->guest_rsp);
    fprintf(file, "guest_rip: 0x%llx\n", vmcs->guest_rip);
    fprintf(file, "guest_rflags: 0x%llx\n", vmcs->guest_rflags);
    fprintf(file, "guest_pending_dbg_exceptions: 0x%llx\n", vmcs->guest_pending_dbg_exceptions);
    fprintf(file, "guest_sysenter_esp: 0x%llx\n", vmcs->guest_sysenter_esp);
    fprintf(file, "guest_sysenter_eip: 0x%llx\n", vmcs->guest_sysenter_eip);
    fprintf(file, "host_cr0: 0x%llx\n", vmcs->host_cr0);
    fprintf(file, "host_cr3: 0x%llx\n", vmcs->host_cr3);
    fprintf(file, "host_cr4: 0x%llx\n", vmcs->host_cr4);
    fprintf(file, "host_fs_base: 0x%llx\n", vmcs->host_fs_base);
    fprintf(file, "host_gs_base: 0x%llx\n", vmcs->host_gs_base);
    fprintf(file, "host_tr_base: 0x%llx\n", vmcs->host_tr_base);
    fprintf(file, "host_gdtr_base: 0x%llx\n", vmcs->host_gdtr_base);
    fprintf(file, "host_idtr_base: 0x%llx\n", vmcs->host_idtr_base);
    fprintf(file, "host_ia32_sysenter_esp: 0x%llx\n", vmcs->host_ia32_sysenter_esp);
    fprintf(file, "host_ia32_sysenter_eip: 0x%llx\n", vmcs->host_ia32_sysenter_eip);
    fprintf(file, "host_rsp: 0x%llx\n", vmcs->host_rsp);
    fprintf(file, "host_rip: 0x%llx\n", vmcs->host_rip);

    // Chiudi il file alla fine
    fclose(file);
}
