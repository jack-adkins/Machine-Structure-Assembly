/****************************************************************************
 *             memory.h
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * Date: 11/17/24
 *
 * Summary:
 * This file defines the functions used in our memory module, which is
 * used to handle all memory related access or functionality. One of the
 * functions is simply to initialize the Universal Machine itself, while
 * the other two represent two different instructions that users can give.
****************************************************************************/

#ifndef MEMORY
#define MEMORY

#include "seq.h"
#include <stdint.h>
#include "uarray.h"
#include "segmentData.h"

UArray_T getSegment(uint32_t size);
void unmapSegment(uint32_t index, Seq_T segList);

#endif