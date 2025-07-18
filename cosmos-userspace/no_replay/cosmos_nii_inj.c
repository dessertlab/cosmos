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
#include <sys/klog.h>

#include "../generic_ops.h"
#include "../types.h"
#include "../injector/operation_lib.h"

#define WORKLOAD_NAME "boot"
#define EXIT_HASHMAP_DIM 15
#define NUM_REPS 1

#define DEVICE_REC_PATH "/dev/cosmos_rec"
#define DEVICE_UT_PATH "/dev/cosmos_ut"
#define DEVICE_INJECTOR_PATH "/dev/cosmos_fi"

#define BUFFER_SIZE 500000  // Default buffer size

void readExitsFromFile(int array[], const char *filename);
void writeExitsToFile(int array[], const char *filename);
int isFileEmpty(const char *filepath);
int readBitPos(const char *filepath); 
int L1fileExists(const char *filepath);

int injector_fd;
int fd;

vmexit* buffer_seeds = NULL; 
vmexit* to_inject_buffer = NULL; 

int main(int argc, char *argv[]) {

    if (argc != 5)
	    printf("Usage error.\n"); 	
    
    fd = open(DEVICE_REC_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    injector_fd = open(DEVICE_INJECTOR_PATH, O_RDWR);
    if (injector_fd < 0) {
        perror("Failed to open the injector device file");
        return -1;
    }

    size_t buffer_size = atoi(argv[1]); 
    size_t idx_op_from_file = atoi(argv[2]);
    size_t bit_pos_from_file = atoi(argv[3]);
    unsigned long long guest_cr0 = strtoul(argv[4], NULL, 16);
    size_t idx_rep_from_file = 0; 

    size_t from_exit = 1; 
    uint8_t mutation_mode = 2; 

    ioctl(fd, SET_BUFFER_SIZE, buffer_size);
    ioctl(fd, SET_FROM_EXIT, from_exit);

    FILE* file_seeds = fopen("/home/test/sync/trace/seeds.txt", "r");

    buffer_seeds = malloc (buffer_size * sizeof (vmexit));
    if (buffer_seeds == NULL) {
        printf("Cannot allocate buffer in memory.\n");
        exit(EXIT_FAILURE);
    }

    read_vmexit_buffer_from_file(file_seeds, buffer_seeds, buffer_size);

    to_inject_buffer = malloc(buffer_size * sizeof(vmexit));
    if (to_inject_buffer == NULL) {
        printf("Cannot allocate to_inject_buffer in memory.\n");
        exit(EXIT_FAILURE);
    }

    system("rm -f seeds_mutated.txt; rm -f seeds_mutated_output.txt; rm -f mutations_dbg.txt");

    enum GuestError guestError = -1;
    uint8_t bit_flip_mode = x64_BIT_FLIP;  

    char command_fuzz[400];
    char filepath[200]; 
    sprintf(command_fuzz, "mkdir -p ./tests/%s", WORKLOAD_NAME);
    system(command_fuzz);

    srand(time(NULL));

    // BOOT WORKLOAD
    int *exit_reasons_wl = malloc(EXIT_HASHMAP_DIM * sizeof(int));
    readExitsFromFile(exit_reasons_wl, "./exit_reasons");
    
    size_t i = 0;
    // For each exit
    while (i <= buffer_size) {

        // If the exit is in exit_reasons
        if (is_in(exit_reasons_wl, EXIT_HASHMAP_DIM, extract_exit_reason(buffer_seeds[i])) && 
            extract_guest_cr0(i) == guest_cr0) {
            sprintf(command_fuzz, "mkdir -p ./tests/%s/%s", WORKLOAD_NAME, 
                exit_reason_names[extract_exit_reason(buffer_seeds[i])]);
            system(command_fuzz);
            
            // For every field touched by a VMCS op
            for (size_t idx_op = idx_op_from_file; idx_op < buffer_seeds[i].op_counter; idx_op++) {
                // If the op is a VMREAD (we don't need to inject also VMWRITE)

                if (extract_exit_reason(buffer_seeds[i]) == 12) {
                    if (buffer_seeds[i].ops[idx_op].field == 0x4824) continue;
                    if (buffer_seeds[i].ops[idx_op].field == 0x4016) continue;
                    if (buffer_seeds[i].ops[idx_op].field == 0x6c04) continue;
                }  

                if (buffer_seeds[i].ops[idx_op].type == 0) {

                    sprintf(command_fuzz, "mkdir -p ./tests/%s/%s/%s_%lx", WORKLOAD_NAME, 
                        exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                        buffer_seeds[i].ops[idx_op].field);
                    system(command_fuzz);

                    sprintf(command_fuzz, "echo %zu > ./idx_op", idx_op);
                    system(command_fuzz); 

                    bit_pos_from_file = readBitPos("./bit_pos");

                    // For 64 bits 
                    for (size_t bit_pos = bit_pos_from_file; bit_pos < 64; bit_pos++) {

                        sprintf(command_fuzz, "mkdir -p ./tests/%s/%s/%s_%lx/bit_%zu", WORKLOAD_NAME, 
                            exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                            buffer_seeds[i].ops[idx_op].field, bit_pos);
                        system(command_fuzz);

                        sprintf(command_fuzz, "echo %zu > ./bit_pos", bit_pos);
                        system(command_fuzz);

                        idx_rep_from_file = readBitPos("./idx_rep");

                        // For three times
                        for (size_t p = idx_rep_from_file; p < NUM_REPS; p++) {

                            sprintf(command_fuzz, "echo %zu > ./idx_rep", p);
                            system(command_fuzz);
                            
                            vmexit* mutated_seed = malloc(sizeof(vmexit));
                            initialize_vmexit (mutated_seed); 
#if _DBG_TEST_
                            sprintf(command_fuzz, "echo \"[%s]: op [%lx], bit [%zu], rep [%zu].\" | sudo tee /dev/kmsg", exit_reason_names[extract_exit_reason(buffer_seeds[i])],
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p);
			                system(command_fuzz);
#endif
                            
                            sprintf(command_fuzz, "mkdir -p ./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p);
                            system(command_fuzz); 

                            system("sudo chown -R test /var/log");
                            system("[ -f /var/log/kern.log ] && sudo truncate -s0 /var/log/kern.log");

                            sprintf(filepath, "./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/seed_mutated.txt", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p); 

                            FILE *file_seeds_mutated = fopen(filepath, "w");
                            if (!file_seeds_mutated) {
                                printf("Can't open file_seeds_mutated.txt");
                                return -1;
                            }

                            sprintf(filepath, "./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/mutations_made.txt", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p); 

                            FILE *file_mutations_made = fopen(filepath, "w");
                            if (!file_mutations_made) {
                                printf("Can't open file_mutations_made.txt");
                                return -1;
                            }

                            sprintf(filepath, "./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/mutations_dbg.txt", WORKLOAD_NAME, 
                            exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                            buffer_seeds[i].ops[idx_op].field, bit_pos, p); 

                            FILE *file_mutations_dbg = fopen(filepath, "w");
                            if (!file_mutations_dbg) {
                                printf("Can't open file_mutations_dbg.txt");
                                return -1;
                            }

                            close(injector_fd);
                            close(fd); 

                            system("/home/test/sync/trace/Hermes_nii/bit_flip/rinserisci_Hermes.sh");
                            sleep(1);
                            sprintf(command_fuzz, "/home/test/sync/trace/Hermes_nii/bit_flip/riavvia_VM.sh %s", HYP); 
                            system(command_fuzz);
                            sleep(1); 

                            fd = open(DEVICE_REC_PATH, O_RDWR);
                            injector_fd = open(DEVICE_INJECTOR_PATH, O_RDWR);

                            ioctl(fd, SET_BUFFER_SIZE, buffer_size);
                            ioctl(fd, SET_FROM_EXIT, from_exit);

                            struct_da_cui_ripartire control_struct; 

                            control_struct = restoreAndWait(SNAPSHOT, fd, injector_fd, buffer_size, 
                                extract_exit_reason(buffer_seeds[i]), guest_cr0);

                            if (control_struct.d_vmcs12 == NULL) {
                                printf("d_vmcs12 NULL!\n");
                                continue;
                            }

                            // Per ora iniettiamo un field alla volta, un bit_flip alla volta. Costruiamo la VMExit
                            mutated_seed->start.name = 0xffffffff;
                            mutated_seed->start.value = 0xffffffff;
                            mutated_seed->start.type = 1;
                            mutated_seed->op_counter = 1; 
                            mutated_seed->ops[0].field = buffer_seeds[i].ops[idx_op].field;
                            mutated_seed->ops[0].value = getVMCSField(buffer_seeds[i].ops[idx_op].field, control_struct.d_vmcs12);

                            if (mutated_seed->ops[0].value == 0xffffffffffffffff) {
                                bit_pos = 63;
                                free(mutated_seed); 
                                fclose(file_mutations_dbg);
                                fclose(file_seeds_mutated);
                                fclose(file_mutations_made);
                                break;
                            }
                            mutated_seed->ops[0].type = 0; 

                            size_t field_size = getVMCSField_Size(buffer_seeds[i].ops[idx_op].field, control_struct.d_vmcs12);

                            if (field_size == 8)
                                bit_flip_mode = x64_BIT_FLIP;
                            else if (field_size == 4)
                                bit_flip_mode = x32_BIT_FLIP;
                            else if (field_size == 2)
                                bit_flip_mode = x16_BIT_FLIP;

                            free(control_struct.d_vmcs12); 

                            printf("Pre mutation field-value-size [%lx]-[%llx]-[%zu].\n", mutated_seed->ops[0].field, mutated_seed->ops[0].value, field_size);

                            guestError = fire(buffer_seeds[i], mutated_seed, mutation_mode, 
                                file_seeds_mutated, file_mutations_made, file_mutations_dbg, injector_fd, bit_flip_mode, 0, i, bit_pos, 0);
                            if (guestError != 0) {
                                printf ("Guest error: %d.\n", guestError);
                                fprintf(file_mutations_made, "Guest Error: %d\n", guestError);
                            }

                            printf("Post mutation field-value [%lx]-[%llx].\n", mutated_seed->ops[0].field, mutated_seed->ops[0].value);

                            ioctl(fd, INJECT_OP, mutated_seed);

                            printf("Exit iniettata: %zu.\n", control_struct.exit_da_cui_ripartire);

                            ioctl(fd, CONTROL_EXEC, 0);
                            ioctl(fd, SET_DUMP_VMCS, 0); 
                            ioctl(fd, SIGNAL_SEM_DUMP_rec, 0); 

                            sleep (5); 

                            if (strcmp(HYP, "KVM") == 0) 
                                sprintf(command_fuzz, "sudo timeout 1s telnet localhost 4444 > ./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/l2.txt", WORKLOAD_NAME, 
                                    exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                    buffer_seeds[i].ops[idx_op].field, bit_pos, p); 
                            else if (strcmp(HYP, "Xen") == 0)
                                sprintf(command_fuzz, "timeout 5s sshpass -p test ssh -p 2222 root@localhost \"xl dmesg\" > ./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/l2.txt", WORKLOAD_NAME, 
                                    exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                    buffer_seeds[i].ops[idx_op].field, bit_pos, p); 
                            system(command_fuzz);

                            sprintf(filepath, "./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/l1.txt", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p);
                            sprintf(command_fuzz, "sudo cp /home/test/test_snapshot/%s/%s_logs.txt %s", HYP, HYP, filepath);
                            system(command_fuzz);

                            printf("Uccido qemu..\n");

                            system("sudo pkill qemu");

                            printf("Disattivo... Exit attuale: %zu.\n", control_struct.exit_da_cui_ripartire);
                            ioctl(fd, CONTROL_EXEC, 0);
                            ioctl(fd, SET_DUMP_VMCS, 0); 
                            ioctl(fd, TRACING_CONTROL, 0); 

                            vmexit* buffer_to_read = malloc ((control_struct.exit_da_cui_ripartire + 1000) * sizeof (vmexit));
                            if (buffer_to_read == NULL) {
                                printf("Cannot allocate buffer in memory.\n");
                                break; 
                            }

                            ssize_t bytes_read = read(fd, buffer_to_read, control_struct.exit_da_cui_ripartire + 1000);
                            if (bytes_read < 0) {
                                perror("Failed to read from the buffer");
                                close(fd);
                                return -1;
                            }

                            sprintf(filepath, "./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/l1.txt", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p); 
                            sprintf(command_fuzz, "sudo cp /var/log/kern.log ./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/l0.txt", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p); 
                            system(command_fuzz);

                            sprintf(command_fuzz, "[ -f /home/test/test_snapshot/%s/%s_logs.txt ] && sudo truncate -s0 /home/test/test_snapshot/%s/%s_logs.txt", HYP, HYP, HYP, HYP);
                            system(command_fuzz);

                            sprintf(filepath, "./tests/%s/%s/%s_%lx/bit_%zu/rep_%zu/accuracy", WORKLOAD_NAME, 
                                exit_reason_names[extract_exit_reason(buffer_seeds[i])], WORKLOAD_NAME, 
                                buffer_seeds[i].ops[idx_op].field, bit_pos, p); 

                            FILE *file_record_replay = fopen(filepath, "w");
                            if (file_record_replay == NULL) {
                                printf ("File %s not existant.\n", filepath);
                                break;
                            }

                            //size_t f = control_struct.exit_da_cui_ripartire - 100
                            for (size_t f = 0; f < control_struct.exit_da_cui_ripartire + 1000; f++) {
                                vmexit exit = buffer_to_read[f];
                                write_vmexit_to_file (file_record_replay, exit, f, 0);
                            }

                            printf("File salvati..\n");

                            free(mutated_seed); 
                            free(buffer_to_read); 
                            fclose(file_record_replay);
                            fclose(file_seeds_mutated);
                            fclose(file_mutations_made);
                        }
                        sprintf(command_fuzz, "echo 0 > ./idx_rep");
                        system(command_fuzz);

                        if (bit_flip_mode == x16_BIT_FLIP && bit_pos >= 15) break; 
                        if (bit_flip_mode == x32_BIT_FLIP && bit_pos >= 31) break;
                    }
                }
                sprintf(command_fuzz, "echo 0 > ./bit_pos");
                system(command_fuzz);
            }

            remove_exit(exit_reasons_wl, EXIT_HASHMAP_DIM, extract_exit_reason(buffer_seeds[i]));
            sprintf(command_fuzz, "sudo echo 0 > ./idx_op");
            system(command_fuzz);
            writeExitsToFile(exit_reasons_wl, "./exit_reasons");

            i = 0;
        }
        else {
            if (areAllElementsMinusOne(exit_reasons_wl, EXIT_HASHMAP_DIM)) break; 
            to_inject_buffer[i] = buffer_seeds[i];
            i++;
        }
    }
    
    fclose(file_seeds); 
    free(exit_reasons_wl);
    free(buffer_seeds);
    free(to_inject_buffer); 
}

void readExitsFromFile(int array[], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return;
    }

    for (int i = 0; i < EXIT_HASHMAP_DIM; i++) {
        if (fscanf(file, "%d", &array[i]) != 1) {
            fprintf(stderr, "Errore nella lettura del numero %d\n", i);
            break;
        }
    }

    fclose(file);
}

void writeExitsToFile(int array[], const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return;
    }

    for (int i = 0; i < EXIT_HASHMAP_DIM; i++) {
        fprintf(file, "%d\n", array[i]);
    }

    fclose(file);
}

int isFileEmpty(const char *filepath) {
    FILE *file = fopen(filepath, "r");

    if (file == NULL) {
        printf("Impossibile aprire il file %s.\n", filepath);
        return -1; // Restituisci un valore negativo per indicare un errore
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    // printf("Lunghezza file %ld\n", file_size);

    fclose(file);

    if (file_size < 100) {
        return 1; // Il file è vuoto
    } else {
        return 0; // Il file non è vuoto
    }
}

int readBitPos(const char *filepath) {
    FILE *file = fopen(filepath, "r");

    if (file == NULL) {
        printf("Impossibile aprire il file %s.\n", filepath);
        return -1; // Restituisci un valore negativo per indicare un errore
    }

    int number;
    
    // Leggi il numero dal file
    if (fscanf(file, "%d", &number) == 1) 
        return number; 
    else
        return -1;

}

int L1fileExists(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}
