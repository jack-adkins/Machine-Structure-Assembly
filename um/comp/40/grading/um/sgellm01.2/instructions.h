/****************************************************************************
 *             instructions.h
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * Date: 11/17/24
 *
 * Summary:
 * This file holds the interface for the instructions module, which is
 * used to handle every instruction that can be called from the command
 * loop, which essentially just reads through the file.
****************************************************************************/

#ifndef INSTRUCTIONS_INTERFACE
#define INSTRUCTIONS_INTERFACE

#include <stdio.h>
#include "segmentData.h"
#include <stdint.h>

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

void cmov(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void sload(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void sstore(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void add(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void mult(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void divide(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void nand(Um_register a, Um_register b, Um_register c, SegmentData *sd);
void halt(SegmentData *sd);
void map_seg(Um_register b, Um_register c, SegmentData *sd);
void unmap_seg(Um_register c, SegmentData *sd);
void input(Um_register c, SegmentData *sd);
void output(Um_register c, SegmentData *sd);
void load_program(Um_register b, Um_register c, SegmentData *sd);
void load_val(uint32_t inst, SegmentData *sd);

#endif