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
#endif