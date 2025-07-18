#include "operation_lib.h"
#include <time.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "definitions.h"

unsigned long x16_random_bit_flip(unsigned long in_value, new_Field* new) {
    unsigned long out_value = in_value;   
    int msb = 15;    
    int lsb = 0;        
    int pos = rand() % (msb + 1 - lsb) + lsb;    
    out_value ^= 1UL << pos;
    new->bit_position = (uint8_t) pos;         
    return out_value;
}

unsigned long x16_bit_flip(unsigned long in_value, new_Field* new, uint8_t pos) {
    unsigned long out_value = in_value;       
    out_value ^= 1UL << pos;
    new->bit_position = (uint8_t) pos;         
    return out_value;
}

unsigned long random_bit_flip(unsigned long in_value, new_Field* new) {
    unsigned long out_value = in_value;   
    int msb = 31;    
    int lsb = 0;        
    int pos = rand() % (msb + 1 - lsb) + lsb;    
    out_value ^= 1UL << pos;    
    new->bit_position = (uint8_t) pos;     
    return out_value;
}

unsigned long bit_flip(unsigned long in_value, new_Field* new, uint8_t pos) {
    unsigned long out_value = in_value;       
    out_value ^= 1UL << pos;    
    new->bit_position = (uint8_t) pos;     
    return out_value;
}

unsigned long long x64_random_bit_flip(unsigned long long in_value, new_Field* new) {
    unsigned long out_value = in_value;   
    int msb = 63;    
    int lsb = 0;        
    int pos = rand() % (msb + 1 - lsb) + lsb;    
    out_value ^= 1UL << pos;   
    new->bit_position = (uint8_t) pos;      
    return out_value;
}

unsigned long long x64_bit_flip(unsigned long long in_value, new_Field* new, uint8_t pos) {
    unsigned long out_value = in_value;          
    out_value ^= 1UL << pos;   
    new->bit_position = (uint8_t) pos;      
    return out_value;
}

unsigned long long change_vm_exit_reason() {
    uint8_t random_exit_reason = rand() % NUM_EXIT_REASONS; 

    return exit_reasons_to_change[random_exit_reason];
}

void gen_single_regs_mutation(vmexit* mutated_e,  new_Field* new, FILE *mutations_dbg) {
    /* choose a random reg to mutate */
    uint8_t random_reg_idx = rand() % NUM_REGS;
    unsigned long old_value = mutated_e->regs_in[random_reg_idx].value;
    unsigned long mutated_value = random_bit_flip (old_value, new);

    mutated_e->regs_in[random_reg_idx].value = mutated_value;

#ifdef _DBG_
        fprintf(mutations_dbg, "Changed register %s: [%lx] -> [%lx].\n", registers[random_reg_idx], old_value, mutated_value);
#endif
}

void gen_single_random_op_mutation(vmexit* mutated_e, uint8_t bit_flip_mode, new_Field* new, FILE *mutations_dbg) {
     /* choose a random op to mutate */
    uint8_t random_op_idx = rand() % mutated_e->op_counter;
    unsigned long long old_value = mutated_e->ops[random_op_idx].value;

    unsigned long long mutated_value;
    if (bit_flip_mode == x16_BIT_FLIP)
        mutated_value = x16_random_bit_flip(old_value, new);
    else if (bit_flip_mode == x32_BIT_FLIP)
        mutated_value = random_bit_flip(old_value, new);
    else if (bit_flip_mode == x64_BIT_FLIP)
        mutated_value = x64_random_bit_flip(old_value, new); 
    
    unsigned long field = mutated_e->ops[random_op_idx].field;
    if (field_blacklist[field] != 1) {
        mutated_e->ops[random_op_idx].value = mutated_value;

#ifdef _DBG_
            fprintf(mutations_dbg, "Changed field %s (%lx): [%llx] -> [%llx].\n", VMCS_FIELDS[field], field, old_value, mutated_value);
#endif

        new->field_modified = field; 
        new->value_modified = mutated_value;
    }
    else if (field == 0x00004402) {
        mutated_e->ops[random_op_idx].value = change_vm_exit_reason(); 

#ifdef _DBG_
            fprintf(mutations_dbg, "Changed EXIT_REASON [%s] -> [%s].\n", 
                exit_reason_names[old_value], exit_reason_names[mutated_e->ops[random_op_idx].value]);
#endif

        new->field_modified = field; 
        new->value_modified = mutated_e->ops[random_op_idx].value;
    }
    else {
        new->field_modified = -1; 
        new->value_modified = -1;
    }
}

void gen_single_op_mutation(vmexit* mutated_e, uint8_t bit_flip_mode, new_Field* new, size_t idx_op, uint8_t bit_pos, FILE *mutations_dbg) {
    unsigned long long old_value = mutated_e->ops[idx_op].value;

    unsigned long long mutated_value;
    if (bit_flip_mode == x16_BIT_FLIP)
        mutated_value = x16_bit_flip(old_value, new, bit_pos);
    else if (bit_flip_mode == x32_BIT_FLIP)
        mutated_value = bit_flip(old_value, new, bit_pos);
    else if (bit_flip_mode == x64_BIT_FLIP)
        mutated_value = x64_bit_flip(old_value, new, bit_pos); 
    
    unsigned long field = mutated_e->ops[idx_op].field;

    if (field_blacklist[field] != 1) {
        mutated_e->ops[idx_op].value = mutated_value;

#ifdef _DBG_
            fprintf(mutations_dbg, "Changed field %s (%lx): [%llx] -> [%llx].\n", VMCS_FIELDS[field], field, old_value, mutated_value);
#endif

        new->field_modified = field; 
        new->value_modified = mutated_value;
    }
    else if (field == 0x00004402) {
        mutated_e->ops[idx_op].value = change_vm_exit_reason(); 

#ifdef _DBG_
            fprintf(mutations_dbg, "Changed EXIT_REASON [%s] -> [%s].\n", 
                exit_reason_names[old_value], exit_reason_names[mutated_e->ops[idx_op].value]);
#endif

        new->field_modified = field; 
        new->value_modified = mutated_e->ops[idx_op].value;
    }
    else {
        new->field_modified = -1; 
        new->value_modified = -1;
    }
}

void gen_mutation(vmexit* mutated_e, uint8_t mutation_mode, uint8_t bit_flip_mode, size_t idx_op, new_Field* new, uint8_t bit_pos, FILE *mutations_dbg) {
    switch (mutation_mode) {
        case MUTATION_REGS:
            gen_single_regs_mutation(mutated_e, new, mutations_dbg);
            break; 

        case MUTATION_NON_RANDOM_OP_FIELDS:
            gen_single_op_mutation(mutated_e, bit_flip_mode, new, idx_op, bit_pos, mutations_dbg);
            break; 

        case MUTATION_RANDOM_OP_FIELDS: 
            gen_single_random_op_mutation(mutated_e, bit_flip_mode, new, mutations_dbg); 
            break; 

        case MUTATION_ALL:
            gen_single_regs_mutation(mutated_e, new, mutations_dbg);
            // To implement random and non random 
            break; 

        default: 
            return; 
            break;
    }
}

int fire(vmexit e, vmexit* mutated_e, uint8_t mutation_mode, FILE *file_seeds_mutated, FILE *file_mutations_made, FILE *mutations_dbg,
    int fuzzer_fd, uint8_t bit_flip_mode, size_t idx_op, size_t i_exit, uint8_t bit_pos, uint8_t copy) { 

    if (copy)
        *mutated_e = e;
    new_Field *new = malloc(sizeof(new_Field));

#ifdef _DBG_
    if (mutations_dbg == NULL) {
        perror("Error mutation_dbg file");
        return 1;
    }

    fprintf(mutations_dbg, "\nVM EXIT n. %zu.\n", i_exit);
#endif

    for (size_t i = 0; i < M_CONS; i++) {
        if (idx_op < mutated_e->op_counter)
            gen_mutation(mutated_e, mutation_mode, bit_flip_mode, idx_op, new, bit_pos, mutations_dbg);
        else 
            printf("Op idx %zu > %u op counter.\n", idx_op, mutated_e->op_counter);
    } 

#ifdef _DBG_
    fclose(mutations_dbg);
#endif

    enum GuestError guestError = 0;
    ioctl(fuzzer_fd, TEST_GUEST_ERROR, mutated_e);
    ioctl(fuzzer_fd, GET_GUEST_ERROR, &guestError);

    write_vmexit_to_file(file_seeds_mutated, *mutated_e, 0, 1);

    if (file_mutations_made != NULL) {
        fprintf(file_mutations_made, "field_modified: %lx\n", new->field_modified);
        fprintf(file_mutations_made, "value_modified: %llx\n", new->value_modified);
        fprintf(file_mutations_made, "bit_position: %u\n", new->bit_position);
    }

    free(new); 

    return guestError;
}