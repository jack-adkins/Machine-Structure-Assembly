#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <a2methods.h>
#include <a2plain.h>
#include <a2blocked.h>
#include <pnm.h>
#include <assert.h>


typedef struct TransformData {
    A2Methods_T methods;
    A2Methods_mapfun *map;
    int rotate_angle;
};