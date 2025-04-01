/****************************************************************************
 *             segmentData.h
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * Date: 11/17/24
 *
 * Summary:
 * This file holds the struct that is central to the universal machine, which
 * contains the values in each segment, a sequence of unused indexes (to be
 * able to reuse memory later on), an integer representing the current word
 * that the program is in, an integer to signal any failure, and an array
 * containing the values at each register.
****************************************************************************/

#ifndef SEGMENT_DATA_H
#define SEGMENT_DATA_H

#include "seq.h"
#include "table.h"
#include "uarray.h"
#include "list.h"

typedef struct SegmentData {
        Seq_T segmentList;
        Seq_T unusedIndexes;
        int currWord;
        UArray_T registers;
} SegmentData;

#endif