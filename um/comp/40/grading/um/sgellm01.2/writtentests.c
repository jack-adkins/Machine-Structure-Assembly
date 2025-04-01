/****************************************************************************
 *             writtentests.c
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * NOTE: this was originally umlab.c, so not all of it was written by us.
 * Date: 11/17/24
 *
 * Summary:
 * This file implements functions in a unit_testing framework, which is
 * defined in testing.c. A unit test is a stream of UM instructions,
 * represented as a Hanson sequence of 32-bit words adhering to the
 * UM's instruction format. 
****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>

typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc);
Um_instruction loadval(unsigned ra, unsigned val);


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction lp(Um_register b, Um_register c) 
{
        return three_register(LOADP, 0, b, c);
}

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

static inline Um_instruction sload(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SLOAD, a, b, c);
}

static inline Um_instruction sstore(Um_register a, Um_register b,
                                    Um_register c) 
{
        return three_register(SSTORE, a, b, c);
}

static inline Um_instruction input(Um_register c)
{
        return three_register(IN, 0, 0, c);
}

static inline Um_instruction mult(Um_register a, Um_register b, Um_register c) 
{
        return three_register(MUL, a, b, c);
}

static inline Um_instruction div(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}

static inline Um_instruction cmov(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

static inline Um_instruction map(Um_register b, Um_register c)
{
        return three_register(ACTIVATE, 0, b, c);
}

static inline Um_instruction nand(Um_register a, Um_register b, Um_register c)
{
        return three_register(NAND, a, b, c);
}

static inline Um_instruction unmap(Um_register c)
{
        return three_register(INACTIVATE, 0, 0, c);
}

Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
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

Um_instruction loadval(unsigned ra, unsigned val)
{
        Um_opcode op = LV;

        Um_instruction ui = 0;
        Um_instruction op_pos = op << 28;
        Um_instruction a_pos = ra << 25;

        ui |= op_pos;
        ui |= a_pos;
        ui |= val;

        return ui;
}

/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_output_test(Seq_T stream)
{
        append(stream, loadval(r3, 'g'));
        append(stream, output(r3));
        append(stream, halt());
}

/*ARITHMETIC TESTS*/
extern void build_add_test(Seq_T stream)
{
        append(stream, loadval(r1, 25));
        append(stream, loadval(r3, 29));
        append(stream, add(r4, r1, r3));
        append(stream, output(r4));
        append(stream, halt());
}

extern void build_mult_test(Seq_T stream)
{
        append(stream, loadval(r0, 5));
        append(stream, loadval(r1, 9));
        append(stream, mult(r2, r1, r0));
        append(stream, output(r2));
        append(stream, halt());
}

extern void build_div_test(Seq_T stream)
{
        append(stream, loadval(r0, 800));
        append(stream, loadval(r1, 20));
        append(stream, div(r2, r0, r1));
        append(stream, output(r2));
        append(stream, halt());
}

extern void build_multdiv_test(Seq_T stream)
{
        append(stream, loadval(r0, 45));
        append(stream, loadval(r1, 72));
        append(stream, mult(r2, r0, r1));
        append(stream, div(r3, r2, r0));
        append(stream, output(r3));
        append(stream, halt());
}

extern void build_allmath_test(Seq_T stream)
{
        append(stream, loadval(r0, 20));
        append(stream, loadval(r1, 10));
        append(stream, loadval(r2, 5));
        append(stream, add(r3, r0, r2));
        append(stream, mult(r4, r1, r3));
        append(stream, add(r5, r4, r2));
        append(stream, add(r6, r5, r2));
        append(stream, div(r7, r6, r2));
        append(stream, output(r7));
        append(stream, halt());
}

/*BNAND TESTS*/
extern void build_bnand_test(Seq_T stream)
{
        append(stream, loadval(r0, 33554431));
        append(stream, loadval(r1, 33554430));
        append(stream, nand(r2, r0, r1));
        append(stream, loadval(r3, 17980645));
        append(stream, div(r4, r2, r3));
        append(stream, loadval(r5, 3));
        append(stream, div(r6, r4, r5));
        append(stream, output(r6));
        append(stream, halt());
}

extern void build_double_bnand_test(Seq_T stream)
{
        append(stream, loadval(r0, 127));
        append(stream, loadval(r1, 97));
        append(stream, nand(r2, r0, r1));
        append(stream, nand(r3, r0, r1));
        append(stream, nand(r4, r2, r3));
        append(stream, output(r4));
        append(stream, halt());
}

void build_map_test(Seq_T stream)
{
        append(stream, map(r2, r3));
        append(stream, halt());
}

void build_unmap_test(Seq_T stream)
{
        append(stream, map(r2, r3));
        append(stream, unmap(r2));
        append(stream, halt());
}

void build_unmap_many_test(Seq_T stream)
{
        append(stream, map(r4, r5));
        for (int i = 0; i < 5000; i++) {
                append(stream, loadval(r7, 12));
                append(stream, map(r6, r7));
                append(stream, unmap(r6));
        }
        append(stream, unmap(r4));
        append(stream, halt());
}

void build_io_test(Seq_T stream)
{
        append(stream, input(r3));
        append(stream, output(r3));
        append(stream, halt());
}

void build_io_eof_test(Seq_T stream)
{
        append(stream, input(r3));
        append(stream, output(r3));

        append(stream, input(r5));
        append(stream, nand(r6, r5, r5));
        append(stream, loadval(r0, 75));
        append(stream, add(r7, r6, r0));
        append(stream, output(r7));
        append(stream, halt());
}

void build_cmov_execute_test(Seq_T stream)
{
        append(stream, loadval(r1, 58));
        append(stream, loadval(r2, 33));
        append(stream, loadval(r3, 89));
        append(stream, cmov(r2, r1, r3));
        append(stream, output(r2));
        append(stream, halt());
}

void build_cmov_no_execute_test(Seq_T stream)
{
        append(stream, loadval(r1, 58));
        append(stream, loadval(r2, 100));
        append(stream, cmov(r2, r1, r3));
        append(stream, output(r2));
        append(stream, halt());
}

void build_lp_not0_test(Seq_T stream)
{
        append(stream, loadval(r1, 4));
        append(stream, halt());
        append(stream, loadval(r2, 2));
        append(stream, map(r0, r1)); // r0 = 1
        append(stream, loadval(r4, 85));
        append(stream, sstore(r0, r2, r4)); // (1, 2) = 85
        append(stream, lp(r0, r2)); // word 2 in index 0
}

void build_lp_0_test(Seq_T stream)
{
        append(stream, loadval(r1, 4));
        append(stream, loadval(r5, 7));
        append(stream, loadval(r2, 2));
        append(stream, map(r0, r1)); // r0 = 1
        append(stream, loadval(r4, 85));
        append(stream, sstore(r0, r2, r4)); // (1, 2) = 85
        append(stream, lp(r6, r5));
        append(stream, halt());
}

void build_segload_test(Seq_T stream)
{
        append(stream, loadval(r0, 6));
        append(stream, loadval(r0, 6));
        append(stream, loadval(r0, 8));
        append(stream, loadval(r0, 3));

        append(stream, sload(r1, r2, r0));
        append(stream, loadval(r7, 2430126));
        append(stream, div(r1, r1, r7));
        append(stream, loadval(r7, 359));
        append(stream, div(r1, r1, r7));
        append(stream, loadval(r7, 48));
        append(stream, add(r1, r1, r7));
        append(stream, output(r1));
        append(stream, halt());
}

void build_sstore_test(Seq_T stream)
{
        for (int i = 0; i < 12; i++) {
                append(stream, loadval(r3, 3));
        }
        append(stream, map(r0, r1));
        append(stream, sstore(r0, r4, r1));
        append(stream, halt());
}

void build_sstore_0_test(Seq_T stream)
{
        for (int i = 0; i < 12; i++) {
                append(stream, loadval(r3, 3));
        }
        append(stream, loadval(r4, 5));
        append(stream, sstore(r0, r4, r1));
        append(stream, halt());
}

void build_long_test(Seq_T stream)
{
        append(stream, loadval(r0, 100));
        append(stream, loadval(r1, 2));
        append(stream, add(r0, r1, r0));
        append(stream, map(r7, r0));
        append(stream, output(r0));
        append(stream, div(r0, r0, r1));
        append(stream, output(r0));
        append(stream, mult(r0, r0, r1));
        append(stream, output(r0));
        append(stream, input(r2));
        append(stream, cmov(r0, r7, r7));
        append(stream, sstore(r7, r2, r2)); // (1, 63) = 63
        append(stream, sload(r5, r7, r2));
        append(stream, output(r5));
        append(stream, unmap(r7));
        append(stream, loadval(r7, 18));
        append(stream, lp(r6, r7));
        append(stream, output(r1)); // never outputs
        append(stream, halt());
}