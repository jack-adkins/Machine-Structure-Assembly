/*
*     ppmtrans.c
*     jadkin05, kdhaya01, 10/09/2024
*     locality
*     
*     Takes a ppm file and performs given rotations on it
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"


void r90(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl);
void r180(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl);
void r270(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl);
void r0(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl);
void writeImage(Pnm_ppm original, A2Methods_UArray2 img_new, 
A2Methods_T methods);


#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

/*
*     usage
*
*     Prints stderr and then terminates the program through exit(1)
*
*     Parameters:
*           const char *progname:
*                           Name of program, passed from argv[0]
*
*     Return:
*           void          
*
*     Expects:
*           Valid progname is given
*
*/
static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] "
                        "[-time time_file] "
                        "[filename]\n",
                        progname);
        exit(1);
}

/*
*     main
*
*     Calls all the functions used to read a given ppm image and from standard
*     input or a file named on command line, transform said image, and then
*     print said transformed image to standard output
*
*     Parameters:
*           int argc:       The number of arguments supplied
*           char *argv[]    The collection of command line arguments supplied
*
*     Return:
*           return 1:      exit with failure
*
*     Expects:
*           Main function can exit program wherever usage function is called
*           methods is not null, supported by assert statement
*           map is not null, supported by assert statement
*
*/
int main(int argc, char *argv[])
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* default to best map */
        /*Currently using row major*/
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);
        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        if (!(i + 1 < argc)) {      /* no time file */
                                usage(argv[0]);
                        }
                        time_file_name = argv[++i];
                } else if (strcmp(argv[i], "-flip horizontal") == 0 ||
                        strcmp(argv[i], "-flip vertical") == 0 ||
                        strcmp(argv[i], "-transpose") == 0) {
                        fprintf(stderr, 
                        "Error: %s is not implemented.\n", argv[i]);
                        exit(EXIT_FAILURE);  /*Unsupported Commands*/
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }
        /*READING INPUT SECTION*/
        FILE *input = (i < argc) ? fopen(argv[i], "r") : stdin;
        
        if (input == NULL) {
                fprintf(stderr, "Could not open file: %s to read\n", argv[i]);
                exit(EXIT_FAILURE);
        }
        Pnm_ppm img = Pnm_ppmread(input, methods);
        if (img == NULL) {
                fprintf(stderr, "Error: Failed to read the image file.\n");
                exit(EXIT_FAILURE);
        }
        if (input != stdin) {
                fclose(input);
        }
        A2Methods_UArray2 img_new = NULL;

        /*TIMER SETUP*/
        CPUTime_T timer = NULL;
        FILE *timingOutput = NULL;
        double total_time;
        if (time_file_name != NULL) {
                timer = CPUTime_New();
                timingOutput = fopen(time_file_name, "w");
                if (timingOutput == NULL) {
                        fprintf(stderr, "Error: Could'nt open timing file.\n");
                        return 1;
                }
        }
        if (timer != NULL) {
               CPUTime_Start(timer);
        }

        /*EXECUTING COMMANDS*/
        if (rotation == 0) {
                img_new = methods->new(img->width, img->height, 
                methods->size(img->pixels));
                map(img->pixels, r0, &img_new);
        } else if(rotation == 90) {
                /*Flipping width and height dimensions*/
                img_new = methods->new(img->height, img->width, 
                methods->size(img->pixels));
                map(img->pixels, r90, &img_new);
        } else if (rotation == 180) {
                img_new = methods->new(img->width, img->height, 
                methods->size(img->pixels));
                map(img->pixels, r180, &img_new);
        } else if (rotation == 270) {
                /*Flipping width and height dimensions*/
                img_new = methods->new(img->height, img->width, 
                methods->size(img->pixels));
                map(img->pixels, r270, &img_new);
        }
        
        /*TIMER COMPLETION*/
        if (timer != NULL) {
                total_time = CPUTime_Stop(timer);
                double seconds = total_time / 1e9;
                int tPixels = img->width * img->height;
                double pixelTime = seconds / tPixels;
                fprintf(timingOutput, 
                "Rotation %d degrees took %.6f seconds\n", rotation, seconds);
                fprintf(timingOutput, "Time per pixel: %.10f seconds\n", 
                pixelTime);
                fclose(timingOutput);
                CPUTime_Free(&timer);
        }

        writeImage(img, img_new, methods);
        Pnm_ppmfree(&img);
        methods->free(&img_new);
}


/*
*     r90
*
*     Rotates image 90 degrees clockwise
*
*     Parameters:
*           int i:          Row index of given element in array
*           int j:          Column index of given element in array
*           A2Methods_Uarray2 array2:
*                           2D array containing elements to rotate
*           void *elmnt:    Pointer to element in array
*           void *cl:       Pointer to new 2D array where rotated elements are
*                           stored
*
*     Return:
*           void          
*
*     Expects:
*           Both ints i and j are within bounds of array
*           Both new and original arrays are properly initialized
*
*/
void r90(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl)
{
        A2Methods_UArray2 ptr = *(A2Methods_UArray2 *)cl;
        A2Methods_T methods = uarray2_methods_plain;

        int new_width = methods->height(array2); 

        int new_col = new_width - 1 - j;
        int new_row = i;

        void *val_new = methods->at(ptr, new_col, new_row);
        memcpy(val_new, elmnt, methods->size(array2));
}

/*
*     r180
*
*     Rotates image 180 degrees
*
*     Parameters:
*           int i:          Row index of given element in array
*           int j:          Column index of given element in array
*           A2Methods_Uarray2 array2:
*                           2D array containing elements to rotate
*           void *elmnt:    Pointer to element in array
*           void *cl:       Pointer to new 2D array where rotated elements are
*                           stored
*
*     Return:
*           void          
*
*     Expects:
*           Both ints i and j are within bounds of array
*           Both new and original arrays are properly initialized
*
*/
void r180(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl)
{
        A2Methods_UArray2 ptr = *(A2Methods_UArray2 *)cl;
        A2Methods_T methods = uarray2_methods_plain;

        int new_col = methods->width(array2) - 1 - i;
        int new_row = methods->height(array2) - 1 - j;

        void *val_new = methods->at(ptr, new_col, new_row);
        memcpy(val_new, elmnt, methods->size(array2));
}

/*
*     r270
*
*     Rotates image 270 degrees
*
*     Parameters:
*           int i:          Row index of given element in array
*           int j:          Column index of given element in array
*           A2Methods_Uarray2 array2:
*                           2D array containing elements to rotate
*           void *elmnt:    Pointer to element in array
*           void *cl:       Pointer to new 2D array where rotated elements are
*                           stored
*
*     Return:
*           void          
*
*     Expects:
*           Both ints i and j are within bounds of array
*           Both new and original arrays are properly initialized
*
*/
void r270(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl)
{
        A2Methods_UArray2 ptr = *(A2Methods_UArray2 *)cl;
        A2Methods_T methods = uarray2_methods_plain;

        int new_height = methods->width(array2); 

        int new_col = j;
        int new_row = new_height - 1 - i;

        void *val_new = methods->at(ptr, new_col, new_row);
        memcpy(val_new, elmnt, methods->size(array2));
}

/*
*     r0
*
*     Rotates image 0 degrees
*
*     Parameters:
*           int i:          Row index of given element in array
*           int j:          Column index of given element in array
*           A2Methods_Uarray2 array2:
*                           2D array containing elements to rotate
*           void *elmnt:    Pointer to element in array
*           void *cl:       Pointer to new 2D array where rotated elements are
*                           stored
*
*     Return:
*           void          
*
*     Expects:
*           Both ints i and j are within bounds of array
*           Both new and original arrays are properly initialized
*
*/
void r0(int i, int j, A2Methods_UArray2 array2, void *elmnt, void *cl)
{
        A2Methods_UArray2 ptr = *(A2Methods_UArray2 *)cl;
        A2Methods_T methods = uarray2_methods_plain;

        void *val_new = methods->at(ptr, i, j);
        memcpy(val_new, elmnt, methods->size(array2));
}

/*
*     writeImage
*
*     Writes rotated/modified image to standard output
*
*     Parameters:
*           A2Methods_Uarray2 img_new:
*                           2D array containing modified image
*           Pnm_ppm original:
*                           Original image before modification
*           A2Methods_T methods:
*                           Contains operations for handling 2D array
*
*     Return:
*           void          
*
*     Expects:
*           img_new and original are both properly initialized
*           Memory allocation for rotated_image succeeds, supported by assert
*           statement
*
*/
void writeImage(Pnm_ppm original, A2Methods_UArray2 img_new, 
A2Methods_T methods)
{
    /*Create the new ppm to write onto*/
    Pnm_ppm rotated = malloc(sizeof(*rotated));
    assert(rotated != NULL);

    /*Collecting all info from the rotated image*/
    rotated->denominator = original->denominator;
    rotated->methods = methods;
    rotated->pixels = img_new;
    rotated->height = methods->height(img_new);
    rotated->width = methods->width(img_new);

    Pnm_ppmwrite(stdout, rotated);

    free(rotated);
}
