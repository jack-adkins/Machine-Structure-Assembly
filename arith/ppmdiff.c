/*
*     ppmdiff.c
*     jadkin05, alall01, 10/18/2024
*     arith
*     
*     Determines the similarity of 2 ppm files
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "uarray2.h"


Pnm_ppm readFile(FILE *fp);
double difference(Pnm_ppm I1, Pnm_ppm I2);
void fourDecimals(double E);


int main(int argc, char *argv[])
{
        Pnm_ppm I1, I2;
        if (argc == 1) {
                I1 = readFile(stdin);
                I2 = readFile(stdin);
        } else if (argc != 3) {
                fprintf(stderr, "Invalid number of arguments provided");
                exit(EXIT_FAILURE);
        } else {
                FILE *fp1 = fopen(argv[1], "r");
                if (fp1 == NULL) {
                        fprintf(stderr, 
                                "%s: %s %s %s\n",
                                argv[0], "Could not open file",
                                argv[1], "for reading");
                        exit(1);
                }
                FILE *fp2 = fopen(argv[2], "r");
                if (fp2 == NULL) {
                        fprintf(stderr, 
                                "%s: %s %s %s\n",
                                argv[0], "Could not open file",
                                argv[2], "for reading");
                        exit(1);
                }
                I1 = readFile(fp1);
                I2 = readFile(fp2);
                fclose(fp1);
                fclose(fp2);
        }
        double E = difference(I1, I2);
        fourDecimals(E);
}

Pnm_ppm readFile(FILE *fp) 
{
    A2Methods_T methods = uarray2_methods_plain;
    return Pnm_ppmread(fp, methods);
}


double difference(Pnm_ppm I1, Pnm_ppm I2) 
{
        int width1 = I1->width;
        int width2 = I2->width;
        int height1 = I1->height;
        int height2 = I2->height;
        if (abs(width1 - width2) > 1 || abs(height1 - height2) > 1) {
                fprintf(stderr, "Incompatible Image Dimensions\n");
                printf("1.0");
                exit(EXIT_FAILURE);
        }
        int smallW = I1->width;
        if (smallW > width2) {
                smallW = width2;
        }
        int smallH = I1->height;
        if (smallW > height2) {
                smallH = height2;
        }
        double diffR, diffG, diffB, sumDiff;
        double total = 0;
        for (int i = 0; i < smallH; i++) {
                for (int j = 0; j < smallW; j++) {
                        Pnm_rgb p1 = I1->methods->at(I1->pixels, j, i);
                        Pnm_rgb p2 = I2->methods->at(I2->pixels, j, i);
                        double d1 = I1->denominator;
                        double d2 = I2->denominator;
                        diffR = (p1->red / d1) - (p2->red / d2);
                        diffG = (p1->green / d1) - (p2->green / d2);
                        diffB = (p1->blue / d1) - (p2->blue / d2);
                        sumDiff = (diffR * diffR) + (diffG * diffG) + 
                        (diffB * diffB);
                        total += sumDiff;
                }
        }
        double denom = 3 * smallW * smallH;
        return sqrt(total / denom);
}

void fourDecimals(double E) {
        printf("%.4f\n", E);
}


