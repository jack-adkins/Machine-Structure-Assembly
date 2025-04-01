#include <stdlib.h>
#include <stdio.h>
#include <pnmrdr.h>

// #ifndef PNMRDR_INCLUDED
// #define PNMRDR_INCLUDED


// #define T Pnmrdr_T        /* This #define macro says, every instance of T actually */
//                         /* refers to the Pnmrdr_T struct                        */


// typedef struct T *T;      /* This typedef creates a new type : now T refers to a pointer  */ 
//                         /* to the struct T. Since T actually refers to the              */
//                         /* Pnmrdr_T struct, now whenever you see T in the code,         */
//                         /* it actually refers to a struct *Pnmrdr_T                     */

// struct Pnmrdr_mapdata {
//     Pnmrdr_maptype type;
//     unsigned width, height;
//     unsigned denominator;
// };

// typedef enum { Pnmrdr_bit = 1, Pnmrdr_gray = 2, Pnmrdr_rgb = 3 } Pnmrdr_maptype;

// char *Pnmrdr_maptype_names[] = {
//         "invalid map type 0", "bitmap", "graymap", "pixmap"
// };

// extern char *Pnmrdr_maptype_names[];

int main(int argc, char *argv[]) {
    // argc: argument count
    // argv: argument vector (array of strings)

    printf("Number of arguments: %d\n", argc);

    FILE *fp = fopen(argv[1], "r");

    if (fp == NULL) {
            fprintf(stderr, 
                    "%s: %s %s %s\n",
                    argv[0], "Could not open file",
                    argv[1], "for reading");
            exit(1);
    }
    Pnmrdr_T new_struct = Pnmrdr_new(fp);
    printf("1\n");
    Pnmrdr_mapdata map_data = Pnmrdr_data(new_struct);
    printf("2\n");
    // printf("Width: %d, Height: %f", Pnmrdr_data->width, Pnmrdr_data->height)
    unsigned final_data = Pnmrdr_get(new_struct);
    (void) final_data;
    (void) map_data;
    printf("3\n");

    return 0;
}


// #undef T
// #endif