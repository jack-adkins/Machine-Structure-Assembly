/*
*     uarray2.h
*     jadkin05, kdhaya01, 09/17/2024
*     iii
*     
*     Contains all function headers for using UArray2_T.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct UArray2_T *UArray2_T;




UArray2_T UArray2_new(int cols, int rows, int ELEMENT_SIZE);

int UArray2_width(UArray2_T array);

int UArray2_height(UArray2_T array);

int UArray2_size(UArray2_T array);

void* UArray2_at(UArray2_T array, int col, int row);

void UArray2_map_col_major(UArray2_T array, void apply(int c, int r, 
                            UArray2_T a, void *p1, void *p2), void *cl);

void UArray2_map_row_major(UArray2_T array, void apply(int c, int r, 
                            UArray2_T a, void *p1, void *p2), void *cl);

void UArray2_free(UArray2_T *array);