/*
*     restoration.c
*     jadkin05, kdhaya01, 09/16/2024
*     filesofpix
*     
*     Main program that reads a corrupted pgm, restores it, and outputs 
*     the restored version.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sequences.h"
#include "seq.h"
#include "atom.h"


/*
*     main
*
*     Calls all the functions used to restore a corrupted PGM
*
*     Parameters:
*           int argc:       The number of arguments supplied
*           char *argv[]    The collection of command line arguments supplied
*
*     Return:
*
*     Expects: There will be a file supplied to be read
*
*     Notes: 
*
*/
int main(int argc, char *argv[]) {
    /*Open the file for reading*/
    if (argc != 2) {
        fprintf(stderr, "RUNTIME ERROR\n");
        return EXIT_FAILURE;
    }
    FILE *fp = fopen(argv[1], "r");

    /*Read the file into our Sequence Structure*/
    Seq_T full_pgm = Seq_new(0);
    full_pgm = readIntoSequence(fp);

    /*Isolate the infused sequence from each line*/
    Seq_T infusions = Seq_new(0);
    infusions = findInfusion(full_pgm);

    /*Find the infusion sequence used on the original lines*/
    const char *correct_infusion = NULL;
    correct_infusion = findCorrectSequence(infusions);

    /*Remove added lines from the PGM*/
    Seq_T restored_pgm = Seq_new(0);
    restored_pgm = restore_PGM(full_pgm, infusions, correct_infusion);

    /*Restore the original lines with their white space*/
    Seq_T restored_ws = Seq_new(0);
    restored_ws = replace_WS(restored_pgm);
    
    /*Find header details and read to stdout*/
    int width =  0;
    width = getWidth(restored_pgm);
    readOut(restored_ws, width);

    /*Free the Sequences used*/
    Seq_free(&full_pgm);
    Seq_free(&restored_pgm);
    Seq_free(&restored_ws);
    fclose(fp);
    return 0;
}

