/*
*     a2plain.c
*     jadkin05, kdhaya01, 10/09/2024
*     locality
*     
*     Contains all function implementations for using the a2methods suite for 
*     plain arrays.
*/


#include <string.h>

#include <a2plain.h>
#include <a2methods.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/


/*
*     new
*     
*     Creates new 2D blocked array for the A2Methods array
*
*     Parameters:
*           int width:      Width/number of columns in 2D blocked array
*           int height:     Height/number of columns in 2D blocked array
*           int size:       Size in bytes of element in array
*
*     Return:
*           UArray2_new(width, height, size)
*                        Function that will create and return said 2D blocked
*                        array
*               
*     Expects:
*           All given inputs to be positive integers
*           Memory allocation for array and blocks will succeed, otherwise
*           program terminates with exit(1)
*
*/
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        /*Ignore blocksize as this uses UArray2, not UArray2b*/
        (void) blocksize;
        return UArray2_new(width, height, size);
}



static void a2free(A2Methods_UArray2 *array)
{
    UArray2_free((UArray2_T *)array);
}

static int width(A2Methods_UArray2 array)
{
    return UArray2_width(array);
}

static int height(A2Methods_UArray2 array)
{
    return UArray2_height(array);
}

static int size(A2Methods_UArray2 array)
{
    return UArray2_size(array);
}

/*No actual blocksize for regular UArray2s*/
static int blocksize(A2Methods_UArray2 array)
{
    (void)array;
    return 1;
}


static A2Methods_Object *at(A2Methods_UArray2 array, int col, int row)
{
    return UArray2_at(array, col, row);
}



static void map_row_major(A2Methods_UArray2 array, A2Methods_applyfun apply, 
                          void *cl)
{
        UArray2_map_row_major(array, (UArray2_applyfun *)apply, cl);
}

static void map_col_major(A2Methods_UArray2 array, A2Methods_applyfun apply, 
                          void *cl)
{
        UArray2_map_col_major(array, (UArray2_applyfun *)apply, cl);
}


/*No map_block_major Function*/

/*No map_block_major Function*/

/*
* A2Methods_smallapplyfun: A function pointer for applying a function to an
*                           element
* void *cl:                A pointer to the context/data passed to the function
*/
struct small_closure {
    A2Methods_smallapplyfun *apply;
    void *cl;
};

/*
*     apply_small
*     
*     Callback to element within 2D array where small closure is applied
*
*     Parameters:
*           int i:          Row index of element in 2D array
*           int j:          Column index of element in 2D array
*           Uarray2_T array:Given 2D array
*           void *elem:     Pointer to element in 2D array
*           void *vcl:      Pointer to closure, referring to apply function in
*                           the small_closure struct
*
*     Return:
*           void
*               
*     Expects:
*           
*
*/
static void apply_small(int i, int j, UArray2_T array, void *elem, void *vcl) 
{
    struct small_closure *cl = vcl;
    (void)i;
    (void)j;
    (void)array;
    cl->apply(elem, cl->cl);
}

/*
*     small_map_row_major
*     
*     Perform small closure to each row in the 2D array 
*
*     Parameters:
*           A2Methods_UArray2 array:
*                           Pointer to 2D array
*           A2Methods_smallapplyfun apply:
*                         Function from small_closure struct
*           void *cl:     A pointer to the context/data passed to the function
*
*     Return:
*           void
*               
*     Expects:
*           
*
*/
static void small_map_row_major(A2Methods_UArray2 array, 
                                A2Methods_smallapplyfun apply, void *cl)
{
    struct small_closure mycl = { apply, cl };
    UArray2_map_row_major(array, apply_small, &mycl);
}

/*
*     small_map_col_major
*     
*     Perform small closure to each column in the 2D array 
*
*     Parameters:
*           A2Methods_UArray2 array:
*                           Pointer to 2D array
*           A2Methods_smallapplyfun apply:
*                         Function from small_closure struct
*           void *cl:     A pointer to the context/data passed to the function
*
*     Return:
*           void
*               
*     Expects:
*           
*
*/
static void small_map_col_major(A2Methods_UArray2 array, 
                                A2Methods_smallapplyfun apply, void *cl)
{
    struct small_closure mycl = { apply, cl };
    UArray2_map_col_major(array, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,                   // blocksize
        at,
        map_row_major,
        map_col_major,
        NULL,                   // map_block_major
        map_row_major,          // map_default
        small_map_row_major,
        small_map_col_major,
        NULL,                   // small_map_block_major
        small_map_row_major,                   // small_map_default
};

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
