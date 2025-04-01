/*
*     bit2.c
*     jadkin05, kdhaya01, 09/27/2024
*     iii
*     
*     Contains all function implementations for Bit2_T.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "bit2.h"
#include "bit.h"
#include "mem.h"

struct Bit2_T {
    Bit_T flatBit2;
    int h;
    int w;
};


/*
*   Bit2_new
*
*   Allocates space for and creates a '2D' bit array
*
*   Parameters:
*       int cols:        The desired # of cols in the 2D bit array(width)
*       int rows:        The desired # of rows in the 2D bit array(height)
*
*   Returns:
*       Bit2_T:          The pointer to the 2D bit array
*
*   Expects:
*       All integer values being provided are positive
*
*/
Bit2_T Bit2_new(int cols, int rows) 
{
        assert(cols > 0);
        assert(rows > 0);
        
        Bit2_T b;
        NEW(b);
        /*Implementing Bit2 as one long Bit_T*/
        b->flatBit2 = Bit_new(cols * rows);
        b->w = cols;
        b->h = rows;
        return b;
}

/*
*   Bit2_width
*
*   Gets the number of columns in the 2D bit array (width)
*
*   Parameters:
*       Bit2_T b:    The Bit2 for which you want the width of
*
*   Returns:
*       int width:       The number of 'columns' in the 2D bitmap (width)
*
*   Expects:
*       b is a Bit2 pointer
*
*/
int Bit2_width(Bit2_T b) 
{
        assert(b);
        
        return b->w;
}

/*
*   Bit2_height
*
*   Gets the number of rows in the 2D bit array (height)
*
*   Parameters:
*       Bit2_T b:    The Bit2 for which you want the height of
*
*   Returns:
*       int height:      The number of rows in the 2D bitmap (height)
*
*   Expects:
*       b is a Bit2 pointer
*
*/
int Bit2_height(Bit2_T b) 
{
        assert(b);
        
        return b->h;
}

/*
*   Bit2_put
*
*   Puts a bit at the desired index in the bitmap
*
*   Parameters:
*       Bit2_T b:    The Bit2 you want to add to
*       int col:    The column index of the bitmap
*       int row:    The row index of the bitmap
*       int mark:   The value to be added to the bitmap
*
*   Returns:
*       int elmnt:    integer of previous bit's value
*
*   Expects:
*       b is a Bit2 pointer that has been allocated
*       col and row are within the bounds of the bitmap
*       mark has a bit value (1 or 0)
*
*/
int Bit2_put(Bit2_T b, int col, int row, int mark) 
{
        assert(b);
        assert(row >=0);
        assert(row < b->h);
        assert(col >= 0);
        assert(col < b->w);
        assert(mark == 1 || mark == 0);

        int index = (b->w * row) + col;
        int elmnt = Bit_put(b->flatBit2, index, mark);
        return elmnt;
}

/*
*   Bit2_get
*
*   PGets a bit from a desired index in the bitmap
*
*   Parameters:
*       Bit2_T b:    The Bit2 you want to pull from
*       int col:    The column index of the bitmap
*       int row:    The row index of the bitmap
*
*   Returns:
*       int elmnt:    the value of the bit at the specified index
*
*   Expects:
*       b is a Bit2 pointer that has been allocated
*       col and row are within the bounds of the bitmap
*
*/
int Bit2_get(Bit2_T b, int col, int row) 
{
        assert(b);
        assert(row >=0);
        assert(row < b->h);
        assert(col >= 0);
        assert(col < b->w);

        int index = (b->w * row) + col;
        int elmnt = Bit_get(b->flatBit2, index);
        return elmnt;
}

/*
*   Bit2_map_col_major
*
*   Maps the Bit2 doing so column by column (column major)
*
*   Parameters:
*       Bit2_T b:    The Bit2 you want to map
*       void apply:      Function to be called on each index of the bitmap
*       void *cl:   void pointer for closure
*
*   Returns:
*       void:   no return value
*       Calls the apply function
*
*   Expects:
*       b is a Bit2 pointer
*       The function being provided has matching parameter value types
*
*/
void Bit2_map_col_major(Bit2_T b, void apply(int c, int r, Bit2_T b2, int x, 
                        void *p1), void *cl) 
{
        assert(b);
        assert(apply);

        for (int j = 0; j < b->w; j++) {
                for (int i = 0; i < b->h; i++) {
                        apply(j, i, b, Bit2_get(b, j, i), cl);
                }
        }
}

/*
*   Bit2_map_row_major
*
*   Maps the Bit2 doing so row by row (row major)
*
*   Parameters:
*       Bit2_T b:    The Bit2 you want to map
*       void apply:      Function to be called on each index of the bitmap
*       void *cl:   void pointer for closure 
*
*   Returns:
*       void:            no return value
*       Calls the apply function
*
*   Expects:
*       b is a Bit2 pointer
*       The function being provided has matching parameter value types
*
*/
void Bit2_map_row_major(Bit2_T b, void apply(int c, int r, Bit2_T b2, int x, 
                        void *p1), void *cl) 
{
        assert(b);
        assert(apply);

        for (int i = 0; i < b->h; i++) {
                for (int j = 0; j < b->w; j++) {
                        apply(j, i, b, Bit2_get(b, j, i), cl);
                }
        }
}

/*
*   Bit2_free
*
*   Frees all memory that has been allocated for Bit2 
*
*   Parameters:
*       Bit2_T *b:    The address of the Bit2 you want to free
*
*   Returns:
*       void:             no return value
*
*   Expects:
*       *b is a Bit2 pointer that has been allocated
*
*/
void Bit2_free(Bit2_T *b) 
{
        assert(*b);

        Bit_free(&((*b)->flatBit2));
        free(*b);
}