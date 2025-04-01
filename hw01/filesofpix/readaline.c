/*
*     readaline.c
*     jadkin05, kdhaya01, 09/16/2024
*     filesofpix
*     
*     Contains the implementation of the readaline function, used to read a
*     a corrupted pgm.
*/


#include <stdlib.h>
#include <stdio.h>
#include "readaline.h"

/*
*     readaline
*
*     Reads a single line of the corrupted pgm file, allocates space for it,
*     then moves the file seeker to th enext line
*
*     Parameters:
*           FILE *input_fp:     The file containing the corrupted pgm
*           char **datapp:      Pointer to the character pointer for the 
*                               read in line
*
*     Return:
*           size_t total_bytes:     The byte size of the line
*
*     Expects: The file fp exists and has already been opened
*              THe line being read in will not be over 1000 chars long
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
size_t readaline(FILE *inputfd, char **datapp){
    #define MAX_LENGTH 1000
    int total_bytes = -1;
    if (inputfd == NULL) {
        fprintf(stderr, "Error Opening File");
        exit(EXIT_FAILURE);
    }

    if (feof(inputfd)) {
        *datapp = NULL;
        return total_bytes;
    }
    char *curr_line = (char *)malloc(MAX_LENGTH * sizeof(char));
    if (curr_line == NULL) {
        fprintf(stderr, "RUNTIME ERROR\n");
        exit(EXIT_FAILURE);
    }
    /*
    * Gets the line along with its size while also checking that it is not
    * too long
    */
    if (fgets(curr_line,MAX_LENGTH, inputfd) != NULL){
        total_bytes = 0;
        for (int i = 0; curr_line[i] != '\0'; i++) {
            total_bytes++;
        }
        if ((total_bytes >= 1) && (curr_line[total_bytes - 1] != '\n') && 
        (!feof(inputfd))) {
            fprintf(stderr, "readaline: input line too long\n");
            exit(4);
        }
    }

    *datapp = curr_line;

    fseek(inputfd, 0, SEEK_CUR);
    return total_bytes;
}