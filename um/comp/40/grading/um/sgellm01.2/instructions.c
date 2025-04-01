/****************************************************************************
 *             instructions.c
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * Date: 11/17/24
 *
 * Summary:
 * This file implements the instructions interface, where users can give
 * a certain instruction (through their .um file), which we read in through
 * the command loop. Note that the map and unmap functions export most of
 * their responsibilities to functions in the memory module.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "segmentData.h"
#include "instructions.h"
#include <stdint.h>
#include "uarray.h"
#include "except.h"
#include "bitpack.h"
#include "memory.h"
#include "assert.h"

Except_T divideByZero;
Except_T invalidOutput;
Except_T invalidAccess;

/********** cmov ********
 *
 * Moves the value in register b into register a as long as register c
 * is not 0.
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Edits the value in register a
 *      
 ************************/
void cmov(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);
        if (*cVal != 0) {
                uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
                uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
                *aVal = *bVal;
        }
}

/********** sload ********
 *
 * Uses the values in registers b & c to identify a word to load into 
 * register a
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *      - THe sequence in SegmentData has been initialized
 *
 * Notes: 
 *      - Edits the value in register a
 *      
 ************************/
void sload(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        UArray_T segB = (UArray_T)Seq_get(sd->segmentList, *bVal);

        if (segB == NULL) {
                RAISE(invalidAccess);
        }

        uint32_t *wordC = (uint32_t *)UArray_at(segB, *cVal);

        if (*cVal >= (uint32_t)UArray_length(segB)) {
                RAISE(invalidAccess);
        }

        *aVal = *wordC;
}

/********** sstore ********
 *
 * Uses the values in registers a & b to identify a word that the value
 * in register c is is stored in
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *      - THe sequence in SegmentData has been initialized
 *
 * Notes: 
 *      - Edits a word in a segment
 *      
 ************************/
void sstore(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        UArray_T segA = (UArray_T)Seq_get(sd->segmentList, *aVal);
        if (segA == NULL) {
                RAISE(invalidAccess);
        }

        /* check if the index is out of bounds */
        if ((*bVal >= (uint32_t)UArray_length(segA)) &&
            (UArray_length(segA) > 0)) {
                RAISE(invalidAccess);
        }

        /* reference returns EXIT_FAILURE here, although the spec indicates
        * that an unchecked runtime error should be thrown */
        if (UArray_length(segA) > 0) {
                uint32_t *wordB = (uint32_t *)UArray_at(segA, *bVal);
                *wordB = *cVal;
        }
}

/********** add ********
 *
 * Adds the values in registers b & c and stores the sum in register a
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Mods the result with 2^32 to keep the sum in range of what 
 *        u_int32_ts can store
 *      - Edits the value in register a
 *      
 ************************/
void add(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        uint32_t result = *bVal + *cVal;

        /* take the result mod 2^32 */
        result %= 4294967296;

        *aVal = result;
}

/********** mult ********
 *
 * Multiplies the values in registers b & c and stores the product
 * in register a
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Mods the result with 2^32 to keep the product in range of what 
 *        u_int32_ts can store
 *      - Edits the value in register a
 *      
 ************************/
void mult(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        uint32_t result = *bVal * *cVal;
        /* take the result mod 2^32 */
        result %= 4294967296;

        *aVal = result;
}

/********** divide ********
 *
 * Divides the value in registers b by the value in register c and 
 * stores the result in register a
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Edits the value in register a
 *      - sd is not null
 *      
 ************************/
void divide(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        assert(sd != NULL);
        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        if (*cVal == 0) {
                RAISE(divideByZero);
        }

        uint32_t result = *bVal / *cVal;
        *aVal = result;
}

/********** nand ********
 *
 * Perfomrms a bitwise and on the values in registers b & c then stores
 * the bitwise inverse of the result in register a
 *
 * Parameters:
 *      Um_register a:          register a
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Edits the value in register a
 *      
 ************************/
void nand(Um_register a, Um_register b, Um_register c, SegmentData *sd)
{
        assert(sd != NULL);
        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        uint32_t nand = ~(*bVal & *cVal);

        *aVal = nand;
}

/********** halt ********
 *
 * Stops computation of the current instruction
 *
 * Parameters:
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *
 * Notes: 
 *      - Edits the currword value in SegmentData
 *      
 ************************/
void halt(SegmentData *sd)
{
        assert(sd != NULL);
        sd->currWord = -1;
}

/********** map_seg ********
 *
 * Maps a new segment at an unused index in the segments sequence
 *
 * Parameters:
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Calls the mapSegment function in the memory implementations
 *      
 ************************/
void map_seg(Um_register b, Um_register c, SegmentData *sd)
{
        assert(sd != NULL);
        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        UArray_T newSeg = getSegment(*cVal);

        /* if all indexes in the sequence are being used, add to the back */
        if (Seq_length(sd->unusedIndexes) == 0) {
                Seq_addhi(sd->segmentList, newSeg);
                *bVal = Seq_length(sd->segmentList) - 1;
        } else {
                /* if there are unused indexes, pick one for the new value */
                uint32_t validIndex = (uint32_t)(uintptr_t)
                                      Seq_get(sd->unusedIndexes, 0);
                Seq_remlo(sd->unusedIndexes);
                Seq_put(sd->segmentList, validIndex, newSeg);
                *bVal = validIndex;
        }
}

/********** unmap_seg ********
 *
 * Unmaps a segment at a given index in the segments sequence
 *
 * Parameters:
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Calls the unmapSegment function in the memory implementations
 *      
 ************************/
void unmap_seg(Um_register c, SegmentData *sd)
{
        assert(sd != NULL);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        unmapSegment(*cVal, sd->segmentList);

        /* put a NULL in the place of the old index */
        Seq_put(sd->segmentList, *cVal, NULL);

        Seq_addhi(sd->unusedIndexes, (void*)(uintptr_t)(*cVal));
}

/********** input ********
 *
 * Takes input in from stdin and stores it in register c
 *
 * Parameters:
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *      - The provided input is a value between 0-255
 *
 * Notes: 
 *      - Edits the value in register c
 *      
 ************************/
void input(Um_register c, SegmentData *sd)
{
        assert(sd != NULL);

        FILE *f = stdin;
        uint32_t val;
        /* if stdin is not at the end of the file, get the char */
        if (feof(f)) {
                val = ~(0);
        } else {
                val = fgetc(f);
        }

        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);
        *cVal = val;
}

/********** output ********
 *
 * Sends the value register c in stdout
 *
 * Parameters:
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *      - The value to be output is between 0-255
 *
 * Notes: 
 *      - Writes to stdout
 *      
 ************************/
void output(Um_register c, SegmentData *sd)
{
        assert(sd != NULL);

        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);

        if (*cVal > 255) {
                RAISE(invalidOutput);
        }

        putchar(*cVal);
}

/********** load_program ********
 *
 * Replaces segment 0 with an already existing segment then updates the
 * program counter
 *
 * Parameters:
 *      Um_register b:          register b
 *      Um_register c:          register c
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *      - The given value in register c is for a word that exists in 
 *        the register being duplicated
 *      - The corresponding segment given by register b has been mapped
 *
 * Notes: 
 *      - Initializes memory for a new segment and frees the memory for 
 *        the old segment 0
 *      - Moves the location of the program counter
 *      
 ************************/
void load_program(Um_register b, Um_register c, SegmentData *sd)
{
        assert(sd != NULL);

        uint32_t *bVal = (uint32_t *)UArray_at(sd->registers, b);
        uint32_t *cVal = (uint32_t *)UArray_at(sd->registers, c);
        if (*bVal != 0) {
                UArray_T wantedSegment = (UArray_T)Seq_get(sd->segmentList,
                                                           *bVal);

                if (wantedSegment == NULL) {
                        RAISE(invalidAccess);
                }

                UArray_T copy = UArray_copy(wantedSegment,
                                            UArray_length(wantedSegment));

                /* free the original segment 0 */
                UArray_T oldSeg0 = (UArray_T)Seq_get(sd->segmentList, 0);
                UArray_free(&oldSeg0);

                Seq_put(sd->segmentList, 0, copy);
        }
        sd->currWord = *cVal;
}

/********** load_val ********
 *
 * Loads a given value into register a
 *
 * Parameters:
 *      uint32_t inst:          instruction to be unpacked
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The registers in SegmentData have been initialized
 *
 * Notes: 
 *      - Edits the value in register a
 *      
 ************************/
void load_val(uint32_t inst, SegmentData *sd)
{
        assert(sd != NULL);

        uint64_t val = Bitpack_getu(inst, 25, 0);

        uint64_t a = Bitpack_getu(inst, 3, 25);

        uint32_t *aVal = (uint32_t *)UArray_at(sd->registers, a);

        *aVal = val;
}