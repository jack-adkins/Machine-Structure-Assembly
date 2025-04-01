/*
*     all_structs.h
*     jadkin05, alall01, 10/22/2024
*     arith
*     
*     Struct implementations for use in compression/decompression
*/

#ifndef ALL_STRUCTS_H
#define ALL_STRUCTS_H

#include <inttypes.h>
#include "a2methods.h"
#include "a2plain.h"
#include "uarray2.h"

typedef struct cvc_closure *cvc_closure; 
typedef struct cw_data *cw_data; 

/*Struct to store useful data for converting between RBG and CVC
colorspace format and vice versa*/
struct cvc_closure{
        A2Methods_UArray2 pix;
        int rgb_denom;
        A2Methods_T methods;
};

/*Struct to represent a pixel in component video color format*/
struct Pnm_cvc{
        float y;
        float b;
        float r;
};

/*Struct to hold the values throughout the decompression step*/
struct cw_data{
        uint64_t a;
        int64_t b;
        int64_t c;
        int64_t d;
        uint64_t Pb;
        uint64_t Pr;
        float Pb1;
        float Pr1;
        float val1;
        float val2;
        float val3;
        float val4;
};

#endif