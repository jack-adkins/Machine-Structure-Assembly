#include "um-dis.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef int Um_opcode;
uint32_t three_register(Um_opcode op, int ra, int rb, int rc);
typedef uint32_t Um_instruction;

int main() {
        
        const char *c = Um_disassemble(3, 1, 2, 3);
        printf("%s", c);
}

uint32_t three_register(Um_opcode op, int ra, int rb, int rc) // return uint32_t
{
        Um_instruction ui = 0;
        Um_instruction op_pos = op << 28;
        Um_instruction a_pos = ra << 6;
        Um_instruction b_pos = rb << 3;

        ui |= op_pos;
        ui |= a_pos;
        ui |= b_pos;
        ui |= (Um_instruction)rc;
        return ui;
}