/*
*     uarray2.c
*     jadkin05, kdhaya01, 09/26/2024
*     iii
*     
*     Contains all function implementations for using UArray2_T.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "uarray2.h"
#include "uarray.h"
#include "uarrayrep.h"
#include "mem.h"

struct UArray2_T {
    UArray_T outer_Uarray; 
    int rows;
    int cols;
    int element_size;
};




/*
*   UArray2_new
*
*   Allocates space for and creates a 2D Uarray comprised of an outer Uarray
*   of "inner" UArrays.
*
*   Parameters:
*       int cols:  The desired # of cols in the 2D Uarray (width)
*       int rows:  The desired # of rows in the 2D Uarray (height)
*
*   Returns:
*       UArray2_T:  The pointer to the 2D Uarray (start of the outer 1D Uarray)
*
*   Expects: 
*       All integer values being provided are positive
*
*/
UArray2_T UArray2_new(int cols, int rows, int ELEMENT_SIZE) 
{
        UArray_T memory_sizing = UArray_new(cols, ELEMENT_SIZE);
        UArray2_T outer_col = (UArray2_T)malloc(sizeof(struct UArray2_T));
        if (outer_col == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
        }
        outer_col->rows = rows;
        outer_col->cols = cols;
        outer_col->element_size = ELEMENT_SIZE;
        outer_col->outer_Uarray = UArray_new(rows, sizeof(memory_sizing));
        for (int i = 0; i < rows; i++) {
                UArray_T inner_row = UArray_new(cols, ELEMENT_SIZE);
                *(UArray_T *)UArray_at(outer_col->outer_Uarray, i) = inner_row;
        }
        UArray_free(&memory_sizing);
        return outer_col;
}

/*
*   UArray2_width
*
*   Gets the number of columns in the 2D Uarray (width)
*
*   Parameters:
*       UArray2_T array:    The UArray2 for which you want the width of
*
*   Returns:
*       int width:  The number of columns in the 2D Uarray (width)
*
*   Expects: 
*       array is a UArray2 pointer
*
*/
int UArray2_width(UArray2_T array) 
{
        assert(array);

        return array->cols;
}

/*
*   UArray2_height
*
*   Gets the number of rows in the 2D Uarray (height)
*
*   Parameters:
*       UArray2_T array:    The UArray2 for which you want the height of
*
*   Returns:
*       int height:  The number of rowss in the 2D Uarray (height)
*
*   Expects: 
*       array is a UArray2 pointer
*
*/
int UArray2_height(UArray2_T array) 
{
        assert(array);
        
        return array->rows;
}

/*
*   UArray2_size
*
*   Gets the element size of the boxes in the "inner" UArray
*
*   Parameters:
*       UArray2_T array:    The UArray2 for which you want the element size of
*
*   Returns:
*       int size:  The size in bytes of a box/element in the "inner" UArray
*
*   Expects: 
*       array is a Urray2 pointer
*
*/
int UArray2_size(UArray2_T array) 
{
        assert(array);
        
        return array->element_size;
}

/*
*   UArray2_at
*
*   Sets the value of an element at the desired location within the UArray2
*
*   Parameters:
*       UArray2_T array:    The UArray2 you want to set a value for
*       int col, row:  The (col,row) location of the element's value to be set 
*
*   Returns:
*       void*:  void pointer
*
*   Expects: 
*       array is a Urray2 pointer
*       Both integers are positive and within the dimensions of the array
*
*/
void* UArray2_at(UArray2_T array, int col, int row) 
{
        assert(array);
        assert(row >=0);
        assert(row < array->rows);
        assert(col >= 0);
        assert(col < array->cols);
        
        UArray_T *inner_row = UArray_at(array->outer_Uarray, row);
        return UArray_at(*inner_row, col);
}

/*
*   UArray2_map_col_major
*
*   Maps the UArray2 doing so column by column (column major)
*
*   Parameters:
*       UArray2_T array:    The UArray2 you want to map
*       void (*func):  The function that checks for the corner of the UArray2
*                      and prints out the current index
*       bool *ok:   Boolean stating if the Uarray2's dimensions are correct
*
*   Returns:
*       void:   no return value
*       Prints out the every index of the UArray2 column by column
*
*   Expects: 
*       array is a Urray2 pointer
*       The function being provided has matching parameter value types
*
*/
void UArray2_map_col_major(UArray2_T array, void apply(int c, int r, 
                            UArray2_T a, void *p1, void *p2), void *cl) 
{
        assert(array);
        assert(apply);
        
        for (int i = 0; i < array->cols; i++) {
                for (int j = 0; j < array->rows; j++) {
                        apply(i, j, array, UArray2_at(array, i, j), cl);
                }
        }
}

/*
*   UArray2_map_row_major
*
*   Maps the UArray2 doing so row by row (row major)
*
*   Parameters:
*       UArray2_T array:    The UArray2 you want to map
*       void (*func):  The function that checks for the corner of the UArray2
*                      and prints out the current index
*       bool *ok:   Boolean stating if the Uarray2's dimensions are correct
*
*   Returns:
*       void:   no return value
*       Prints out the every index of the UArray2 row by row
*
*   Expects: 
*       array is a Urray2 pointer
*       The function being provided has matching parameter value types
*
*/
void UArray2_map_row_major(UArray2_T array, void apply(int c, int r, 
                            UArray2_T a, void *p1, void *p2), void *cl) 
{
        assert(array);
        assert(apply);
        
        for (int j = 0; j < array->rows; j++) {;
                for (int i = 0; i < array->cols; i++) {
                        apply(i, j, array, UArray2_at(array, i, j), cl);
                }
        }
}

/*
*   UArray2_free
*
*   Frees all memory that has been allocated for the UArray2
*
*   Parameters:
*       UArray2_T *array:    The address of the UArray2 you want to free
*
*   Returns:
*       void:   no return value
*
*   Expects: 
*       array is a Urray2 pointer that has been allocated
*
*/
void UArray2_free(UArray2_T *array) 
{
        assert(*array);
        
        for (int i = 0; i < UArray2_height(*array); i++) {
                UArray_T *inner_row = UArray_at((*array)->outer_Uarray, i);
                UArray_free(inner_row);
        }
        UArray_T outer_row = (*array)->outer_Uarray;
        UArray_free(&outer_row);
        free(*array);
}