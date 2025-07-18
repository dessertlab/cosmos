#ifndef _OP_LIB_
#define _OP_LIB_

#include <stdio.h>
#include <inttypes.h>
#include "../types.h"
#include "../generic_ops.h"

#define MUTATION_REGS 0
#define MUTATION_RANDOM_OP_FIELDS 1
#define MUTATION_NON_RANDOM_OP_FIELDS 2
#define MUTATION_ALL 3

#define x16_BIT_FLIP 0 
#define x32_BIT_FLIP 1
#define x64_BIT_FLIP 2 

#define TEST_GUEST_ERROR 18
#define GET_GUEST_ERROR 19

/**Number of consecutive mutations (it must be a M )**/
#define M_CONS 1

typedef struct {
    unsigned long field_modified; 
    unsigned long long value_modified; 
    uint8_t bit_position; 
} new_Field; 

/* Mutation ops */
unsigned long long change_vm_exit_reason();
void gen_mutation(vmexit* mutated_e, uint8_t mutation_mode, uint8_t bit_flip_mode, size_t idx_op, new_Field* new, uint8_t bit_pos, FILE *mutations_dbg);

/* Fuzzer ops */
int fire(vmexit e, vmexit* mutated_e, uint8_t mutation_mode, FILE *file_seeds_mutated, FILE *file_mutations_made, FILE *mutations_dbg,
    int fuzzer_fd, uint8_t bit_flip_mode, size_t idx_op, size_t i_exit, uint8_t bit_pos, uint8_t copy);

#endif