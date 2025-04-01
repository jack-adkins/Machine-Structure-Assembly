#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitpack.h"
#include <sys/stat.h>
#include <string.h>
#include "assert.h"

typedef struct Seg {
        uint32_t length;
        uint32_t *words;
} Seg;

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

#define opcode(inst) inst >> 28;
#define a(inst) (inst >> 6) & 0x7
#define b(inst) (inst >> 3) & 0x7
#define c(inst) inst & 0x7
#define a_loadval(inst) (inst >> 25) & 0x7
#define val(inst) inst << 7 >> 7
#define incrCurrWord(cWord) cWord++;

#define INITSIZE 35000
#define GET_WORD(inst, currWord) uint32_t inst = allSegments[0].words[currWord]
#define equals(a, b) a = b
#define no_argc(argc) (void)argc;
#define returnVal return EXIT_SUCCESS;
#define freed(val) free(val);
#define stop break;
#define newu(word, width, lsb, val)  \
uint32_t mask = ((uint32_t)((1 << width) - 1)) << lsb; \
word = word & ~mask; \
word = word | (val << lsb);

static inline void commandLoop(char *filename)
{
        FILE *f = fopen(filename, "r");
        uint32_t maxVal = 4294967296;
        Seg *allSegments = (Seg *)malloc(INITSIZE * sizeof(Seg));
        uint32_t *unusedIndexes = (uint32_t *)malloc(INITSIZE * sizeof(uint32_t));
        uint32_t registers[8] = {0};
        uint32_t allocSize = INITSIZE;
        uint32_t currSize = 1;
        uint32_t currWord = 0;
        uint32_t unusedSize = 0;
        uint32_t unusedAllocSize = INITSIZE;
        for (uint32_t i = 0; i < INITSIZE; i++) {
                allSegments[i] = (Seg){0, NULL};
        }

        struct stat sb;
        stat(filename, &sb);
        long long byteSize = (long long)sb.st_size;
        uint32_t initLen = byteSize / 4;
        allSegments[0].length = initLen;
        allSegments[0].words = (uint32_t *)malloc(byteSize);
        for (uint32_t j = 0; j < initLen; j++) {
                uint32_t word = 0;
                for (int i = 24; i >= 0; i -= 8) {
                        newu(word, 8, i, getc(f));
                }
                allSegments[0].words[j] = word;
        }
        while (1) {
                GET_WORD(instruction, currWord);
                uint8_t a = a(instruction);
                uint8_t b = b(instruction);
                uint8_t c = c(instruction);
                Um_opcode opcode = opcode(instruction);
                switch(opcode) {
                        case CMOV:
                                switch(registers[c]) {
                                        case 0:
                                                stop
                                        default:
                                                registers[a] = registers[b];
                                                stop
                                }
                                incrCurrWord(currWord);
                                stop
                        case SLOAD:
                                registers[a] = allSegments[registers[b]].words[registers[c]];
                                incrCurrWord(currWord);
                                stop
                        case SSTORE:
                                allSegments[registers[a]].words[registers[b]] = registers[c];
                                incrCurrWord(currWord);
                                stop
                        case ADD:
                        {
                                registers[a] = (registers[b] + registers[c]);
                                incrCurrWord(currWord);
                                stop
                        }
                        case MUL:
                        {
                                registers[a] = (registers[b] * registers[c]);
                                incrCurrWord(currWord);
                                stop
                        }
                        case DIV:
                        {
                                registers[a] = registers[b] / registers[c];
                                incrCurrWord(currWord);
                                stop
                        }
                        case NAND:
                                registers[a] = ~(registers[b] & registers[c]);
                                incrCurrWord(currWord);
                                stop
                        case ACTIVATE:
                        {
                                if (allocSize == currSize) {
                                        allocSize *= 2;
                                        allSegments = (Seg *)realloc(allSegments, allocSize * sizeof(Seg));

                                        for (uint32_t i = currSize; i < allocSize; i++) {
                                                allSegments[i].length = 0;
                                                allSegments[i].words = NULL;
                                        } 
                                }
                                uint32_t size = registers[c];
                                Seg newSeg = {size, NULL};
                                newSeg.words = (uint32_t *)malloc(size * sizeof(uint32_t));
                                for(uint32_t i = 0; i < size; i++) {
                                        newSeg.words[i] = 0;
                                }
                                switch(unusedSize) {
                                        case 0:
                                        {
                                                allSegments[currSize] = newSeg;
                                                registers[b] = currSize;
                                                currSize++;
                                                stop
                                        }
                                        default:
                                        {
                                                uint32_t lastOpenedIndex = unusedSize - 1;
                                                uint32_t validIndex = unusedIndexes[lastOpenedIndex];
                                                unusedIndexes[lastOpenedIndex] = 0;
                                                unusedSize--;
                                                allSegments[validIndex] = newSeg;
                                                registers[b] = validIndex;
                                                stop
                                        }
                                }
                                incrCurrWord(currWord);
                                stop
                        }
                        case INACTIVATE:
                        {
                                if (unusedSize == unusedAllocSize) {
                                        unusedAllocSize *= 2;
                                        unusedIndexes = (uint32_t *)realloc(unusedIndexes, unusedAllocSize * sizeof(uint32_t));

                                        for (uint32_t i = unusedSize; i < unusedAllocSize; i++) {
                                                unusedIndexes[i] = 0;
                                        }
                                }
                                uint32_t cVal = registers[c];
                                Seg segToUnmap = allSegments[cVal];
                                free(segToUnmap.words);
                                allSegments[cVal].length = 0;
                                allSegments[cVal].words = NULL;
                                unusedIndexes[unusedSize] = cVal;
                                unusedSize++;
                                incrCurrWord(currWord);
                                stop
                        }
                        case HALT:
                                goto cleanup;
                        case OUT:
                                putchar(registers[c]);
                                incrCurrWord(currWord);
                                stop
                        case IN:
                        {
                                FILE *f = stdin;
                                registers[c] = (uint32_t)fgetc(f);
                                incrCurrWord(currWord);
                                stop
                        }
                        case LOADP:
                        {
                                uint32_t bVal = registers[b];
                                switch(registers[b])
                                {
                                        default:
                                        {
                                                Seg wantedSegment = allSegments[bVal];
                                                allSegments[0].length = wantedSegment.length;
                                                allSegments[0].words = (uint32_t *)realloc(allSegments[0].words, wantedSegment.length * 4);
                                                memcpy(allSegments[0].words, wantedSegment.words, wantedSegment.length * 4);
                                                stop
                                        }
                                        case 0:
                                                stop
                                }
                                currWord = registers[c];
                                stop
                        }
                        case LV:
                        {
                                uint32_t val = val(instruction);
                                uint8_t a = a_loadval(instruction);
                                registers[a] = val;
                                incrCurrWord(currWord);
                                stop
                        }
                        default:
                                stop
                }
        }

cleanup:
        for (uint32_t i = 0; i < allocSize; i++) {
                if (allSegments[i].words != NULL) {
                        free(allSegments[i].words);
                }
        }
        free(unusedIndexes);
        free(allSegments);
        fclose(f);
}

int main(int argc, char *argv[])
{
        no_argc(argc);
        commandLoop(argv[1]);
        returnVal;
}