/*
*     unblackedges.c
*     jadkin05, kdhaya01, 09/27/2024
*     iii
*     
*     Removes black edges from a pbm file.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bit2.h"
#include "uarray.h"
#include "pnmrdr.h"


/*
*     fillBit2
*
*     Fills the Bit2 bitmap with bits
*
*     Parameters:
*       int c:           column index in the Bit2
*       int r:           row index in the Bit2
*       Bit2_T page:     the 2d bitmap
*       int x:           unused
*       void *p1:        pointer to the pnmreader
*
*     Return:
*       void
*
*     Expects:
*           
*/
void fillBit2(int c, int r, Bit2_T page, int x, void *p1) 
{
        int current = Pnmrdr_get(*((Pnmrdr_T*) p1));
        Bit2_put(page, c, r, current);
        (void) x;
}

/*
*     checkBounds
*
*     Checks to see if an index is within the bounds of the bitmap
*
*     Parameters:
*       int cRow:        current row index
*       int cCol:        current column index
*       int maxR:        maximum row index
*       int maxC:        maximum col index
*
*     Return:
*       bool true/false: bool for if the index is within the bitmap bounds
*
*     Expects:
*
*/
bool checkBounds(int cRow, int cCol, int maxR, int maxC) 
{
        if (cRow < 0 || cRow > maxR) {
                return false;
        } else if (cCol < 0 || cCol >maxC) {
                return false;
        }
        return true;
}

/*
*     checkBlack
*
*     Turns black pixels into white ones
*
*     Parameters:
*       Bit2_T page:    the 2d bitmap
*       int r:           row index of pixel
*       int c:           column index of pixel
*
*     Return:
*       void
*
*     Expects:
*           
*/
void checkBlack(Bit2_T page, int r, int c) 
{
        int current = Bit2_get(page, c, r);
        if (current == 1) {
                Bit2_put(page, c, r, 0);
        }
}

/*
*     edgeFinder
*
*     Finds and convert all outer black edges
*
*     Parameters:
*       int c:           col index of the pixel
*       int r:           row index of the pixel
*       Bit2_T page:     the 2d bitmap
*       int x:           unused
*       void *p1:        unused
*
*     Return:
*       void
*
*     Expects:
*
*/
void edgeFinder(int c, int r, Bit2_T page, int x, void *p1) 
{
        int current = Bit2_get(page, c, r);
        bool border = false;
        int maxw = Bit2_width(page);
        int maxh = Bit2_height(page);
        if (c == 0 || r == 0 || c == (maxw - 1) || r == (maxh - 1)) {
                border = true;
        }
        if (current == 1 && border) {
                if (checkBounds(r - 1, c, maxw, maxh)) {
                        checkBlack(page, r - 1, c);
                }
                if (checkBounds(r + 1, c, maxw, maxh)) {
                        checkBlack(page, r + 1, c);
                }
                if (checkBounds(r, c - 1, maxw, maxh)) {
                        checkBlack(page, r, c - 1);
                }
                if (checkBounds(r, c + 1, maxw, maxh)) {
                        checkBlack(page, r, c + 1);
                }
                Bit2_put(page, c, r, 0);
        }

        (void) x;
        (void) p1;
}

/*
*     simplePrint
*
*     Prints the height width raster of the bitmap being output
*
*     Parameters:
*       int c:           column index in the Bit2
*       int r:           row index in the Bit2 (unused)
*       Bit2_T page:     the 2d bitmap
*       int mark:        bit value being printed
*       void *p1:        pointer to the pnmreader
*
*     Return:
*       void
*
*     Expects:
*           
*/
void simplePrint(int c, int r, Bit2_T page, int mark, void *p1) 
{
        FILE* fp = p1;
        if (c == Bit2_width(page) - 1) {
                fprintf(fp, "%d\n", mark);
        } else {
                fprintf(fp, "%d ", mark);
        }
        (void) r;
}

/*
*     pbmwrite
*
*     Writes the header and bitmap to output in pbm format
*
*     Parameters:
*       FILE* fp:        location for the output
*       Bit2_T page:     the 2d bitmap
*
*     Return:
*       void
*
*     Expects:
*           
*/
void pbmwrite(FILE* fp, Bit2_T page)
{
        int h = Bit2_height(page);
        int w = Bit2_width(page);
    
        fprintf(fp, "P1\n");
        fprintf(fp, "%d %d\n", w, h);

        Bit2_map_row_major(page, simplePrint, fp);
}

/*
*     readFileRun
*
*     Manages the Bit2 that does the unblackedges for the given pbm file
*
*     Parameters:
*       FILE *input:     the input to read from     
*
*     Return:
*       void
*
*     Expects:
*           File is open for reading
*/
void readFileRun(FILE *input) {
        Pnmrdr_T reader; 
        reader = Pnmrdr_new(input);
        Pnmrdr_mapdata mapData;
        mapData =  Pnmrdr_data(reader);

        if (reader == NULL) {  
                fprintf(stderr, "Error: Not a valid PbM file.\n");
                exit(EXIT_FAILURE);
        }
        if (mapData.type != Pnmrdr_bit) {
                fprintf(stderr, "Error: Not a PGM file.\n");
                Pnmrdr_free(&reader);
                exit(EXIT_FAILURE);
        }

        int w = (int)mapData.width;
        int h = (int)mapData.height;
        Bit2_T page = Bit2_new(w,h);
        Bit2_map_row_major(page, fillBit2, &reader);
        Bit2_map_row_major(page, edgeFinder, &reader);
        
        pbmwrite(stdout, page);

        Pnmrdr_free(&reader);
        Bit2_free(&page);
}


/*
*     main
*
*     Open the input/file and starts the unblackedges process
*
*     Parameters:
*       int argc:        The number of arguments supplied
*       char *argv[]     The collection of command line arguments supplied
*
*     Return:
*           
*
*     Expects: There will be a file supplied to be read
*
*/
int main(int argc, char *argv[]) 
{
        if (argc == 1) {
                readFileRun(stdin);
        } else {
                for (int i = 1; i < argc; i++) {
                        FILE *fp = fopen(argv[i], "r");
                        if (fp == NULL) {
                                fprintf(stderr, 
                                        "%s: %s %s %s\n",
                                        argv[0], "Could not open file",
                                        argv[i], "for reading");
                                exit(1);
                        }
                        readFileRun(fp);
                        fclose(fp);
                }
        }
}


