/****************************************************************************
 *             um.c
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * Date: 11/17/24
 *
 * Summary:
 * This file holds the setup that reads in from the command line and
 * initializes the setup that will be used for the um program. On
 * top of that, reading through each instruction in segment 0 takes
 * place in this file, as does freeing all used memory.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "seq.h"
#include "segmentData.h"
#include <stdint.h>
#include "memory.h"
#include "instructions.h"
#include "bitpack.h"
#include <sys/stat.h>
#include "assert.h"

Except_T invalidInstruction;

typedef struct instructionParts {
        Um_register a;
        Um_register b;
        Um_register c;
        Um_opcode op;
} ips;

ips deconstructInstruction(uint32_t inst);
void run(FILE *f, struct stat sb);
void freeData(SegmentData *sd);
Seq_T read_in(FILE *f, struct stat sb);
void commandLoop(SegmentData *sd);
Seq_T initSegments(long long length);

/********** main ********
 *
 * Handles and runs the execution of instructions
 *
 * Parameters:
 *      int argc:               the number of arguments provided
 *      char *argv[]:           array of the arguments provided           
 *
 * Return:
 *      - EXIT_SUCCESS if run to completion
 * 
 * Expects:
 *      - A .um file is provided
 *
 * Notes: 
 *      - Gives the open file to the run fucntion to use
 *      
 ************************/
int main(int argc, char *argv[])
{
        if (argc != 2) {
                fprintf(stderr, "usage: ./um [instructions]\n");
                return EXIT_FAILURE;
        }

        FILE *f = fopen(argv[1], "r");

        if (f == NULL) {
                fprintf(stderr, "Could not open file.\n");
                return EXIT_FAILURE;
        }

        struct stat sb;

        /* read in the length of the file */
        if (stat(argv[1], &sb) == -1) {
                return EXIT_FAILURE;
        }

        run(f, sb);

        fclose(f);

        return EXIT_SUCCESS;
}

/********** run ********
 *
 * Runs our UM interface
 *
 * Parameters:
 *      FILE *f:                file pointer (already opened)
 *      struct stat sb *sd:     stat struct containing the size of the
 *                              instructions file
 *
 * Return:
 *      void
 *
 * Expects:
 *      - The file is already open
 *
 * Notes: 
 *      - Calls read_in to get the instructions
 *      - Calls the commandLoop function to go through the instructions
 *      - Calls the freeData function when finished
 *      
 ************************/
void run(FILE *f, struct stat sb)
{
        Seq_T allSegments = read_in(f, sb);

        /* initialize the registers to 0 */
        UArray_T regs = UArray_new(8, sizeof(uint32_t));
        for (int i = 0; i < 8; i++) {
                uint32_t *ptr = (uint32_t *)UArray_at(regs, i);
                *ptr = 0;
        }

        /* initialize our SegmentData struct */
        SegmentData *sd = (SegmentData *)malloc(sizeof(struct SegmentData));
        assert(sd != NULL);
        sd->segmentList = allSegments;
        sd->unusedIndexes = Seq_new(0);
        sd->currWord = 0;
        sd->registers = regs;
        commandLoop(sd);
        freeData(sd);
}

/********** freeData ********
 *
 * free all used memory
 *
 * Parameters:
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void
 *
 * Expects:
 *      - The sequence of segments has been initialized
 *
 * Notes: 
 *      - Uses the unusedIndexes to determine what needs to be freed
 *      
 ************************/
void freeData(SegmentData *sd)
{
        assert(sd != NULL);
        Seq_free(&(sd->unusedIndexes));
        for (int i = 0; i < Seq_length(sd->segmentList); i++) {
                UArray_T seg = (UArray_T)Seq_get(sd->segmentList, i);
                if (seg != NULL) {
                        UArray_free(&seg);
                }
        }
        Seq_free(&(sd->segmentList));
        UArray_free(&(sd->registers));
        free(sd);
}

/********** read_in ********
 *
 * Reads in the list of instructions from a file
 *
 * Parameters:
 *      FILE *f:                file pointer (already opened)
 *      struct stat sb *sd:     stat struct containing the size of the
 *                              instructions file
 *
 * Return:
 *      Seq_T allSegments:      The sequence containing the segment that has
 *                              all the read in instructions
 *
 * Expects:
 *      - The file is already open
 *
 * Notes: 
 *      - Uses getc and feof to handle the reading
 *      - Uses bitpack to unpack the instructions
 *      - Calls the iniitalizeMem function to set up the sequence of segments
 *      
 ************************/
Seq_T read_in(FILE *f, struct stat sb)
{
        assert(f != NULL);

        long long byteSize = (long long)sb.st_size;

        Seq_T allSegments = initSegments(byteSize);
        UArray_T seg0 = (UArray_T)Seq_get(allSegments, 0);
        int byte;

        /* read in each word by combining 4 bytes */
        for (int j = 0; j < byteSize / 4; j++) {
                uint32_t word = 0;
                for (int i = 24; i >= 0; i -= 8) {
                        byte = getc(f);

                        if (feof(f)) {
                                exit(1);
                        }

                        word = Bitpack_newu(word, 8, i, byte);
                }
                uint32_t *ptr = (uint32_t *)UArray_at(seg0, j);
                *ptr = word;
        }
        return allSegments;
}

/********** commandLoop ********
 *
 * Runs the command loop to read through instructions and execute them
 *
 * Parameters:
 *      SegmentData *sd:        pointer to struct containing all relevant
 *                              structures, counters, and register values
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - The SegmentData struct along with all of its elements
 *        have been initialized
 *
 * Notes: 
 *      - Calls the instruction functions to execute the instructions
 *      - Uses the decnstructInstruction function to unpack instructions
 *      
 ************************/
void commandLoop(SegmentData *sd)
{
        assert(sd != NULL);
        while (sd->currWord != -1) {
                uint32_t *instruction = (uint32_t *)
                                        UArray_at(Seq_get(sd->segmentList, 0),
                                                  sd->currWord);
                ips parts = deconstructInstruction(*instruction);
                Um_register a = parts.a;
                Um_register b = parts.b;
                Um_register c = parts.c;
                switch(parts.op) {
                        case CMOV:
                                cmov(a, b, c, sd);
                                break;
                        case SLOAD:
                                sload(a, b, c, sd);
                                break;
                        case SSTORE:
                                sstore(a, b, c, sd);
                                break;
                        case ADD:
                                add(a, b, c, sd);
                                break;
                        case MUL:
                                mult(a, b, c, sd);
                                break;
                        case DIV:
                                divide(a, b, c, sd);
                                break;
                        case NAND:
                                nand(a, b, c, sd);
                                break;
                        case HALT:
                                halt(sd);
                                break;
                        case ACTIVATE:
                                map_seg(b, c, sd);
                                break;
                        case INACTIVATE:
                                unmap_seg(c, sd);
                                break;
                        case OUT:
                                output(c, sd);
                                break;
                        case IN:
                                input(c, sd);
                                break;
                        case LOADP:
                                load_program(b, c, sd);
                                break;
                        case LV:
                                load_val(*instruction, sd);
                                break;
                        default:
                                RAISE(invalidInstruction);
                }

                if ((parts.op != LOADP) && (parts.op != HALT)) {
                        sd->currWord++;
                }

                if (sd->currWord >= UArray_length(Seq_get
                                                 (sd->segmentList, 0))) {
                        RAISE(invalidInstruction);
                }
        }
}

/********** deconstructInstruction ********
 *
 * Unpacks a uint32_t instruction and returns its various parts
 *
 * Parameters:
 *      uint32_t inst:          the instruction to unpacked
 *
 * Return:
 *      ips parts:              struct containing the parts of the 
 *                              broken down instruction
 *
 * Expects:
 *
 * Notes: 
 *      - Uses the bitpack interface to unpack the instruction
 *      
 ************************/
ips deconstructInstruction(uint32_t inst)
{
        uint64_t opCode = Bitpack_getu(inst, 4, 28);
        uint64_t a = Bitpack_getu(inst, 3, 6);
        uint64_t b = Bitpack_getu(inst, 3, 3);
        uint64_t c = Bitpack_getu(inst, 3, 0);

        ips parts = {a, b, c, opCode};
        return parts;
}

/********** initSegments ********
 *
 * Initializes the sequence of UArrays that represent the segments 
 * being used throughout the UM
 *
 * Parameters:
 *      long long length:       The total size in bytes of the collection
 *                              of segment 0 instructions
 *
 * Return:
 *      Seq_T segments:         The sequence to contain all the segments
 *
 * Expects:
 *      - 
 *
 * Notes: 
 *      - Initializes a new Seq_T and allocates the memory for segment 0
 *      
 ************************/
Seq_T initSegments(long long length)
{
        Seq_T segments = Seq_new(0);
        UArray_T uarray = UArray_new(length / sizeof(uint32_t),
                                     sizeof(uint32_t));
        Seq_addlo(segments, uarray);

        return segments;
}
