/*
*     bit2.h
*     jadkin05, kdhaya01, 09/27/2024
*     iii
*     
*     Contains all function headers for using Bit2_T.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Bit2_T *Bit2_T;

Bit2_T Bit2_new(int cols, int rows);

int Bit2_width(Bit2_T b);

int Bit2_height(Bit2_T b);

int Bit2_put(Bit2_T b, int col, int row, int mark);

int Bit2_get(Bit2_T b, int col, int row);

void Bit2_map_col_major(Bit2_T b, void apply(int c, int r, Bit2_T b2, 
                        int x, void *p1), void *cl);

void Bit2_map_row_major(Bit2_T b, void apply(int c, int r, Bit2_T b2, 
                        int x, void *p1), void *cl);

void Bit2_free(Bit2_T *b);