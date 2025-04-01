/*
*     sudoku.c
*     jadkin05, kdhaya01, 09/26/2024
*     iii
*     
*     Determines if a sudoku pgm is solved or not
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "uarray2.h"
#include "uarray.h"
#include "pnmrdr.h"


/*
*     one_through_nine
*
*     Tests to see that no the maxiumum pixel count in the sudoku board is nine
*     pixels, as well as no pixel having zero intensity
*
*     Parameters:
*           char list[9]:           
*
*     Return:
*           bool true/false:     Depending on if the sudoku solution passes the
*                                function, will return either true or false
*
*     Expects:
*           Sudoku solution follows expected parameters
*
*
*/
bool one_through_nine(char list[9]) 
{
        bool found[9] = {false};
        for (int i = 0; i < 9; i++) {
                if (list[i] >= '1' && list[i] <= '9') {
                        int int_index = list[i] - '1';
                        found[int_index] = true;
                }
        }
        for (int i = 0; i < 9; i++) {
                if (!found[i]) {
                        return false;
                }
        }
        return true;
}

/*
*     testRows
*
*     Tests to see that no two rows in the sudoku board have the same 
*     intensity
*
*     Parameters:
*           UArray2_T board:     the given sudoku board      
*
*     Return:
*           bool true/false:     Depending on if the sudoku solution passes the
*                                function, will return either true or false
*
*     Expects:
*           Sudoku solution follows expected parameters
*
*
*/
bool testRows(UArray2_T board) 
{
        char cRow[9];
        bool passed = false;
        for (int i_row = 0; i_row < 9; i_row++) {
                for (int j_col = 0; j_col < 9; j_col++) {
                        int *curr = UArray2_at(board, j_col, i_row);
                        char c = *curr + '0';
                        cRow[j_col] = c;
                }
                passed = one_through_nine(cRow);
                if (!passed) {
                        return false;
                }
        }
        return passed;
}

/*
*     testColumns
*
*     Tests to see that no two columns in the sudoku board have the same 
*     intensity
*
*     Parameters:
*           UArray2_T board:     the given sudoku board      
*
*     Return:
*           bool true/false:     Depending on if the sudoku solution passes the
*                                function, will return either true or false
*
*     Expects:
*           Sudoku solution follows expected parameters
*
*
*/
bool testColumns(UArray2_T board) 
{
        char cCol[9];
        bool passed = false;
        for (int i_col = 0; i_col < 9; i_col++) {
                for (int j_row = 0; j_row < 9; j_row++) {
                        int *curr = UArray2_at(board, i_col, j_row);
                        char c = *curr + '0';
                        cCol[j_row] = c;
                }
                passed = one_through_nine(cCol);
                if (!passed) {
                        return false;
                }
        }
        return passed;
}

/*
*     testSquare
*
*     Tests to see that if the sudoku board was split from the 9x9 graymap into
*     3x3 submaps, no two pixels in each submap have the same intensity
*
*     Parameters:
*           UArray2_T board:     the given sudoku board      
*
*     Return:
*           bool true/false:     Depending on if the sudoku solution passes the
*                                function, will return either true or false
*
*     Expects:
*           Sudoku solution follows expected parameters
*
*
*/
bool testSquare(UArray2_T board) 
{
        char cSqr[9];
        bool passed = false;
        for (int r_idx = 0; r_idx < 3; r_idx++) {
            for (int c_idx = 0; c_idx < 3; c_idx++) {
                for (int r_inner = 0; r_inner < 3; r_inner++) {
                    for (int c_inner = 0; c_inner < 3; c_inner++) {
                            int *curr = UArray2_at(board, (c_idx * 3) + 
                                        c_inner, (r_idx * 3) + r_inner);
                            char c = *curr + '0';
                            cSqr[(r_inner * 3) + c_inner] = c;
                    }
                }
                passed = one_through_nine(cSqr);
                if (!passed) {
                        return false;
                }
            }
        }
        return true;
}

/*
*     checkSolved
*
*     Checks to see that sudoku baord passes the testRows, testColumns, and 
*     testSquare tests
*
*     Parameters:
*           UArray2_T board:     the given sudoku board      
*
*     Return:
*           bool true/false:     Depending on if the sudoku solution passes the
*                                function, will return either true or false
*
*     Expects:
*           Sudoku solution follows expected parameters
*
*
*/
bool checkSolved(UArray2_T board) 
{
        bool row_check = testRows(board);
        bool column_check = testColumns(board);
        bool square_check = testSquare(board);
        if (row_check && column_check && square_check) {
                UArray2_free(&board);
                return true;
        } else {
                UArray2_free(&board);
                return false;
        }
}

/*
*     readFileRun
*
*     Reads and validates the given pgm file, a 9x9 sudoku board
*
*     Parameters:
*           FILE *input:     the location of the given pgm file     
*
*     Return:
*           bool true/false:     Depending on if the given file is a pgm, will 
*                                return either true or false
*           Causes exit failures if the input file isnt the correct format
*
*     Expects:
*           Sudoku solution follows expected parameters
*
*
*/
bool readFileRun(FILE *input) 
{
        if (input == NULL) {
                fprintf(stderr, "Error Opening File");
                exit(EXIT_FAILURE);
        }

        Pnmrdr_T reader;
        Pnmrdr_mapdata mapData;

        reader = Pnmrdr_new(input);
        if (reader == NULL) {  
                fprintf(stderr, "Error: Not a valid PGM file.\n");
                exit(EXIT_FAILURE);
        }
        mapData = Pnmrdr_data(reader);
        if (mapData.type != Pnmrdr_gray) {
                fprintf(stderr, "Error: Not a PGM file.\n");
                Pnmrdr_free(&reader);
                exit(EXIT_FAILURE);
        }
        if (mapData.denominator != 9 || mapData.width != 9 || 
            mapData.height != 9){
                fprintf(stderr, "Error: Not Proper Sudoku format\n");
                Pnmrdr_free(&reader);
                exit(EXIT_FAILURE);
        }
        int w = (int)mapData.width;
        int h = (int)mapData.height;
        UArray2_T board = UArray2_new(w, h, sizeof(int));
        for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                        int elmnt = (int)Pnmrdr_get(reader);
                        *(int *)UArray2_at(board, j, i) = elmnt;
                }
        }
        Pnmrdr_free(&reader);
        return checkSolved(board);
}

/*
*     main
*
*     Calls all the functions used to determine if the given sudoku board is
*     solved
*
*     Parameters:
*           int argc:       The number of arguments supplied
*           char *argv[]    The collection of command line arguments supplied
*
*     Return:
*           EXIT_SUCCESS:   Given sudoku board is determined to have been solved
*           EXIT_FAILURE:   Given sudoku board is determined to have not been
*                           solved
*
*     Expects: There will be a file supplied to be read
*
*
*/
int main(int argc, char *argv[])
{
        bool good_board = false;
        if (argc == 1) {
                good_board = readFileRun(stdin);
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
                        good_board = readFileRun(fp);
                        fclose(fp);
                }
        }
        if (!good_board) {
                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}