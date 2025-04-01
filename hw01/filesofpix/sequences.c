/*
*     sequences.c
*     jadkin05, kdhaya01, 09/16/2024
*     filesofpix
*     
*     Contains all function implementations used for restoration.
*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "readaline.h"
#include "sequences.h"
#include "seq.h"
#include "atom.h"


/*
*     readIntoSequence
*
*     Uses the readaline function to read in the corrupted pgm, atomize the
*     the lines and store it into a Sequence Hanson data structrue
*
*     Parameters:
*           FILE *fp:       The file containing the corrupted pgm
*
*     Return:
*           Seq_T seq_line:     The sequence contain the corrupted pgm lines
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
Seq_T readIntoSequence(FILE *fp) {
    /*Initialize our datapp pointer*/
    char c = 'c';
    int byte_size = 0;
    char *char_ptr = &c;
    char **datapp = &char_ptr;
    Seq_T seq_line = Seq_new(0); 
    /*
    * Reads in each line of the file then atomizes it before adding it to a
    * sequence where it will be stored
    */
    byte_size = readaline(fp, datapp);
    const char *curr_line = Atom_string(*datapp);
    Seq_addhi(seq_line, (void *)curr_line);
    while(byte_size != -1) {
        byte_size = readaline(fp, datapp);
        if (byte_size != -1) {
            const char *curr_line = Atom_string(*datapp);
            Seq_addhi(seq_line, (void *)curr_line);
        }
    }
    free(*datapp);
    return seq_line;
}

/*
*     isolateChars
*
*     Uses the readline function to read in the corrupted pgm, atomize the
*     the lines and store it into a Sequence Hanson data structrue
*
*     Parameters:
*           FILE *fp:       The file containing the corrupted pgm
*
*     Return:
*           Seq_T seq_line:     The sequence contain the corrupted pgm lines
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
const char *isolateChars(const char *line) {
    /*Finding the length of the line*/
    int len = 0;
    while (line[len] != '\n' && line[len] != '\0') {
        len++;
    }
    char *result = (char *)malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "RUNTIME ERROR\n");
        exit(EXIT_FAILURE);
    }
    int index = 0;
    /*Only grabs the 'infused'(non-integer) charaters*/
    for (int i = 0; i < len; i++) {
        if (line[i] < 48 || line[i] > 57) { 
            result[index] = line[i];
            index++;
        }
    }
    result[index] = '\n';
    /*Atomize and send to parent func to be added to the infusions seq*/
    const char *infused = Atom_string(result);
    free(result);
    return infused;
}

/*
*     findInfusion
*
*     Reads the given pgm and separates corrupted pgm lines into their 
*     individual infusions
*
*     Parameters:
*           Seq_T fullpgm:       The corrupted pgm file
*
*     Return:
*           Seq_T infusions:     The sequence containing the corrupted pgm
*                                infusions
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
Seq_T findInfusion(Seq_T full_pgm) {
    Seq_T infusions = Seq_new(0);
    /*Traverses every line of the sequence*/
    for (int i = 0; i < Seq_length(full_pgm); i++) {
        const char *line = Seq_get(full_pgm, i); 
        const char *infused = isolateChars(line);
        Seq_addhi(infusions, (void *)infused);  
    }
    return infusions;
}


/*
*     findCorrectSequence
*
*     Compares between the indivual infusions to find the matching pair, helping
*     determine which are from the pgm file before corruption
*
*     Parameters:
*           Seq_T infusions:       The sequence containing the corrupted pgm
*                                  infusions
*
*     Return:
*           NULL     
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
const char *findCorrectSequence(Seq_T infusions) {
    Seq_T already_checked = Seq_new(0);
    /*Larger to loop that uses each infusion seq as the base to compare once*/
    for (int i = 0; i < Seq_length(infusions); i++) {
        const char *curr_checking = Seq_get(infusions, i);
        /*
        * Checks the base infusion code against every code in the already
        * checked sequence and then adds it to that already checked sequence 
        * until a match is found
        */
        for (int j = 0; j < i; j++) {
            if (Seq_length(already_checked) != 0) {
                const char *checked_infusion = Seq_get(already_checked, j);
                if (strcmp(curr_checking, checked_infusion) == 0) {
                    Seq_free(&already_checked);
                    return curr_checking;
                }
            }
        }
        Seq_addhi(already_checked, (void *)curr_checking);
    }
    /*Just in case no matching infusion codes ar efound*/
    Seq_free(&already_checked); 
    return NULL;
}

/*
*     restore_PGM
*
*     Uses the correct infusion to compare and take lines from the corrupted
*     file that match it, creating the pgm file without any of the extra
*     corrupted lines
*
*     Parameters:
*           Seq_T infusions:       The sequence containing the corrupted pgm
*                                  infusions
*
*           Seq_T full_pgm:        The corrupted pgm file
*
*           const char correct_infusion: Contains correct infusion, allowing
*                                        the function to remove any lines
*                                        containing incorrect infusions
*
*     Return:
*           Seq_T restored_pgm:     The corrupted pgm file without extra
*                                   corrupted lines
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
Seq_T restore_PGM(Seq_T full_pgm, Seq_T infusions, 
const char *correct_infusion) {
    /*
    * Uses the fact that the original corrupted lines and their infusion
    * sequence/coding are in the same order in their respective sequences
    */
    Seq_T restored_pgm = Seq_new(0);
    const char* line_to_add = NULL;
    for (int i = 0; i < Seq_length(infusions); i ++) {
        const char *current_line = Seq_get(infusions, i);
        if (strcmp(current_line, correct_infusion) == 0) {
            line_to_add = Seq_get(full_pgm, i);
            Seq_addhi(restored_pgm, (void *)line_to_add);
        }
    }
    return restored_pgm;
}

/*
*     replace_WS
*
*     Replacing the infusions from the pgm file without extra corrupted lines
*     with whitespaces, restoring the pgm to its original state before
*     corruption
*
*     Parameters:
*           Seq_T restored_pgm:    The corrupted pgm file without extra
*                                  corrupted lines
*
*     Return:
*           Seq_T restored_ws:     The pgm file, now restored and no longer
*                                  containing any corruption
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
Seq_T replace_WS(Seq_T restored_pgm) {
    /*Another sequnce to store the version with whitespace*/
    Seq_T restored_ws = Seq_new(0);
    /*Making sure that only integer chars aren't turned to whitespace*/
    for (int i = 0; i < Seq_length(restored_pgm); i ++) {
        char *current_line = Seq_get(restored_pgm, i);
        int len = strlen(current_line);
        for (int j = 0; j < len ; j++) {
            if (current_line[j] < 48 || current_line[j] > 57) { 
                current_line[j] = 32;
            }
        }
        Seq_addhi(restored_ws, (void *)current_line);
    }
    return restored_ws;
}

/*
*     readOut
*
*     Prints the width, height, and maxval of the restored pgm file, as well as
*     pgm file itself
*
*     Parameters:
*           Seq_T restored_ws:     The pgm file, now restored and no longer
*                                  containing any corruption

            int width:             Width of the restored pgm file
*
*     Return:
*           void
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
void readOut(Seq_T restored_ws, int width) {
    /*Sending everything to stdout*/
    printf("P5\n");
    printf("%d ", width);
    int height = Seq_length(restored_ws);
    printf("%d\n", height);
    int maxval = 255;
    printf("%d\n", maxval);
    for (int i = 0; i < Seq_length(restored_ws); i++) {
        const char *printer = Seq_get(restored_ws, i);
        printf("%s\n", printer);
    }
}

/*
*     getWidth
*
*     Helper function to obtain width of restored pgm file
*
*     Parameters:
*           Seq_T restored_ws:     The pgm file, now restored and no longer
*                                  containing any corruption
*
*     Return:
*           int i:                 Width of the restored pgm file
*
*     Expects: The file fp exists and has already been opened
*
*     Notes: Implemented to only read lines 1000 characters or less
*
*/
int getWidth(Seq_T restored_pgm) {
    char* lin = Seq_get(restored_pgm, 0);
    int i = strlen(lin);
    return i;
}