/****************************************************************************
 *             memory.c
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * Date: 11/17/24
 *
 * Summary:
 * This file implements the functions used in our memory module. The memory
 * module handles all memory related access or change in our Universal Machine.
****************************************************************************/

#include "seq.h"
#include <stdint.h>
#include "segmentData.h"
#include "uarray.h"
#include <stdlib.h>
#include "assert.h"
#include <stdio.h>

Except_T invalidIndex;

/********** getSegment ********
 *
 * returns a new segment of a given size, and initializes it to 0s
 *
 * Parameters:
 *      uint32_t size:                the size of the Uarray to be created
 *
 * Return:
 *      a UArray of the given size
 *
 * Expects:
 *      - size is a positive integer
 *
 * Notes: 
 *      - Allocates memory for the new segment
 *      
 ************************/
UArray_T getSegment(uint32_t size)
{
        UArray_T newSeg = UArray_new(size, sizeof(uint32_t));
        assert(newSeg != NULL);
        /* initialize a uarray with all 0s */
        for (int i = 0; i < UArray_length(newSeg); i++) {
                uint32_t *ptr = (uint32_t *)UArray_at(newSeg, i);
                *ptr = 0;
        }

        return newSeg;
}

/********** unmapSegment ********
 *
 * Unmaps a segment at a given index in the segments sequence
 *
 * Parameters:
 *      uint32_t index:             the index of the segment to unmap
 *      Seq_T segList:              a hanson sequence containing a uarray
 *                                  (or NULL) at each index
 *
 * Return:
 *      void function
 *
 * Expects:
 *      - the index exists in the sequence
 *
 * Notes: 
 *      - Frees the segment at the given index
 *      
 ************************/
void unmapSegment(uint32_t index, Seq_T segList)
{
        UArray_T segToUnmap = (UArray_T)Seq_get(segList, index);

        /* if the index is not valid, raise an exception */
        if ((index >= (uint32_t)Seq_length(segList)) || (segToUnmap == NULL)) {
                RAISE(invalidIndex);
        }

        UArray_free(&segToUnmap);
}