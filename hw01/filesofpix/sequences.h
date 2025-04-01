/*
*     sequences.h
*     jadkin05, kdhaya01, 09/16/2024
*     filesofpix
*     
*     Contains all function definitions used for restoration.
*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "readaline.h"
#include "seq.h"
#include "atom.h"

#ifndef SEQUENCES_H
#define SEQUENCES_H


Seq_T readIntoSequence(FILE *fp);

const char *isolateChars(const char *line);
Seq_T findInfusion(Seq_T full_pgm);


const char *findCorrectSequence(Seq_T infusions);

Seq_T restore_PGM(Seq_T full_pgm, Seq_T infusions, 
const char *correct_infusion);

Seq_T replace_WS(Seq_T restored_pgm);

void readOut(Seq_T restored_ws, int width);
int getWidth(Seq_T pgm);
#endif