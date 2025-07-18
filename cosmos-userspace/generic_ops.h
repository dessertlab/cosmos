#ifndef _GENERIC_OPS_
#define _GENERIC_OPS_

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "injector/definitions.h"
#include <semaphore.h>

//COSMOS_Rec
#define SET_BUFFER_SIZE 1
#define RESET 2
#define TRACING_CONTROL 3
#define SET_FROM_EXIT 8
#define SIGNAL_SEM_DUMP_rec 21
#define SET_DUMP_VMCS 22
#define CREATE_EFF_REC_BUFF 23
#define READ_EFF_REC_BUFF 24

//COSMOS_Fi
#define SIGNAL_SEM_DUMP_fi 20
#define SET_DUMP_VMCS 22
#define CONTROL_EXEC 27
#define INJECT_OP 28

//COSMOS_Ut
#define TEST_GUEST_ERROR 18
#define GET_GUEST_ERROR 19
#define DUMP_VMCS12 20

void initialize_vmexit(vmexit *exit);
void write_vmexit_to_file(FILE *file, vmexit exit, size_t i, uint8_t scarta);
void read_vmexit_buffer_from_file(FILE *file, vmexit* b, size_t dim);
void print_vmexit (vmexit b);
void print_hashmap(const Exit_Reasons_count* hashmap);
void init_hashmap(Exit_Reasons_count* hashmap);
void print_seed (const char* filepath_input, const char* filepath_output, size_t num_exits);
unsigned long long extract_guest_cr0 (size_t num_exit) ;

int is_in (int* exit_reasons, size_t dim, size_t exit_reason);
void remove_exit (int* exit_reasons, size_t dim, size_t exit_reason); 
int areAllElementsMinusOne(int* exit_reasons, size_t dim);
unsigned long long extract_exit_reason (vmexit e);
void restoreAndReinject (vmexit* b, size_t dim, const char* snap_name, int rnr_fd);
struct_da_cui_ripartire restoreAndWait(const char* snap_name, int rnr_fd, int fuzz_fd, size_t buffer_size,
    unsigned long long exit_reason, unsigned long long guest_cr0);
unsigned long long getVMCSField(unsigned int field, const dump_vmcs12_t* vmcs);
size_t getVMCSField_Size(unsigned int field, const dump_vmcs12_t* vmcs);

#endif