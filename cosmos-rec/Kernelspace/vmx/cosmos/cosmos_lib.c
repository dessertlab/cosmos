#include "cosmos_lib.h"

void write_regs_in (vmexit* b, size_t exit_counter, unsigned long regs[], bool write_op_counter, size_t buffer_size) {

    size_t i = 0;

    uint32_t regs_name [NUM_REGS] = {
        0x1fffffff,
        0x2fffffff,
        0x3fffffff,
        0x4fffffff,
        0x5fffffff,
        0x6fffffff,
        0x7fffffff,
        0x8fffffff,
        0x9fffffff,
        0xafffffff,
        0xbfffffff,
        0xcfffffff,
        0xdfffffff,
        0xfffffff,
        0xefffffff,
    };

    if (exit_counter >= buffer_size) return;

    b[exit_counter].start.name = 0xffffffff;
    b[exit_counter].start.value = 0xffffffff;
    b[exit_counter].start.type = 2;
 
    for (i = 0; i < NUM_REGS; i++) {
        b[exit_counter].regs_in[i].name = regs_name[i];
        b[exit_counter].regs_in[i].value = regs[i];
        b[exit_counter].regs_in[i].type = 2;
    }

    if (write_op_counter)
        b[exit_counter].op_counter = 0;

}

void write_regs_out (vmexit* b, size_t exit_counter, unsigned long regs[], size_t buffer_size) {

    size_t i = 0;

    uint32_t regs_name [NUM_REGS] = {
        0x1fffffff,
        0x2fffffff,
        0x3fffffff,
        0x4fffffff,
        0x5fffffff,
        0x6fffffff,
        0x7fffffff,
        0x8fffffff,
        0x9fffffff,
        0xafffffff,
        0xbfffffff,
        0xcfffffff,
        0xdfffffff,
        0xfffffff,
        0xefffffff,
    };

    if (exit_counter >= buffer_size) return; 

    for (i = 0; i < NUM_REGS; i++) {
        b[exit_counter].regs_out[i].name = regs_name[i];
        b[exit_counter].regs_out[i].value = regs[i];
        b[exit_counter].regs_out[i].type = 2;
    }

    b[exit_counter].start.type = b[exit_counter].op_counter;
}

void write_op (vmexit* b, size_t exit_counter, unsigned long field, unsigned long long value, uint8_t type, size_t buffer_size) {

    if (exit_counter >= buffer_size) return;

    if (b[exit_counter].op_counter < NUM_OPS) {
        b[exit_counter].ops[b[exit_counter].op_counter].field = field;
        b[exit_counter].ops[b[exit_counter].op_counter].value = value;
        b[exit_counter].ops[b[exit_counter].op_counter].type = type;

        b[exit_counter].op_counter++;
    }

}
