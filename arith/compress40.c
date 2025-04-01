/*
*     compress40.c
*     jadkin05, alall01, 10/22/2024
*     arith
*     
*     Program to compress and decompress ppm images
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "uarray2.h"
#include "compress40.h"
#include "arith40.h"
#include "all_structs.h"
#include "bitpack.h"


static void (*compress_or_decompress)(FILE *input) = compress40;

/*Compression Definitions*/
Pnm_ppm trimBorders(Pnm_ppm untrimmed, A2Methods_T methods);
Pnm_ppm RGBtoCVC(Pnm_ppm rgb, A2Methods_T methods, A2Methods_mapfun *map);
void apply_convert_to_cvc(int i, int j, void *array2D, void *elem, void *cl);
uint32_t *packWords(Pnm_ppm cvcPPM, A2Methods_mapfun *map);
void apply_block_packing(int i, int j, void *array2D, void *elem, void *cl);
int *DCT(float y1, float y2, float y3, float y4);
void writeOut_C(uint32_t *all_cw, int width, int height);

/*Decompression Definitions*/
A2Methods_UArray2 readHeader(FILE *input, A2Methods_T methods, 
A2Methods_mapfun *map);
A2Methods_UArray2 setupPixmap(int width, int height, A2Methods_T methods, 
A2Methods_mapfun *map,  FILE *fp);
void apply_handle_codewords(int i, int j, void *array2D, void *elem, void *cl);
A2Methods_UArray2 unpackCW(A2Methods_UArray2 ua2, A2Methods_T methods, 
A2Methods_mapfun *map);
void apply_unpack_codewords(int i, int j, void *array2D, void *elem, void *cl);
A2Methods_UArray2 floatConvert(A2Methods_UArray2 ua2, A2Methods_T methods, 
A2Methods_mapfun *map);
void apply_float_conversions(int i, int j, void *array2D, void *elem, void *cl);
void inverseDCT(cw_data x);
Pnm_ppm blocks_to_cvcPixels(A2Methods_UArray2 ua2, A2Methods_T methods, 
A2Methods_mapfun *map);
void apply_make_cvc_pixelmap(int i, int j, void *array2D, void *elem, void *cl);
Pnm_ppm CVCtoRGB(Pnm_ppm cvc, A2Methods_T methods, A2Methods_mapfun *map, 
int dnm);
void apply_convert_to_rgb(int i, int j, void *array2D, void *elem, void *cl);
void writeOut_D(Pnm_ppm final);



/********** main ********
 *
 * Handles and runs the compression and decompression
 *
 * Parameters:
 *      int argc: the number of arguments provided
 *      char *argv[]: array of the arguments provided           
 *
 * Return:
 *      EXIT_SUCCESS if run to completion
 * 
 * Expects:
 *      - A single file for compression or decompression is given
 *      - A command specifying compression or decompression is given
 *
 * Notes: 
 *      
 ************************/
int main(int argc, char *argv[])
{
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0) {
                        compress_or_decompress = compress40;
                } else if (strcmp(argv[i], "-d") == 0) {
                        compress_or_decompress = decompress40;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n",
                                argv[0], argv[i]);
                        exit(1);
                } else if (argc - i > 2) {
                        fprintf(stderr, "Usage: %s -d [filename]\n"
                                "       %s -c [filename]\n",
                                argv[0], argv[0]);
                        exit(1);
                } else {
                        break;
                }
        }
        assert(argc - i <= 1);
        if (i < argc) {
                FILE *fp = fopen(argv[i], "r");
                assert(fp != NULL);
                compress_or_decompress(fp);
                fclose(fp);
        } else {
                compress_or_decompress(stdin);
        }

        return EXIT_SUCCESS; 
}

/********** compress40 ********
 *
 * Reads in pixel data, converts to CVC form, performs DCT and packs into 
 * 32-bit code words to ultimately compress PPM image
 *
 * Parameters:
 *      File *fp: pointer to PPM image file               
 *
 * Return:
 *      none
 * 
 * Expects:
 *      - fp should be a pointer to a valid PPM file 
 *
 * Notes: 
 *      - frees memory for pnm_ppm object and array of code words
 *      
 ************************/
void compress40(FILE *fp)
{
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        Pnm_ppm ppm_original = Pnm_ppmread(fp, methods);
        ppm_original = trimBorders(ppm_original, methods);
        ppm_original = RGBtoCVC(ppm_original, methods, map);
        uint32_t *all_cw = packWords(ppm_original, map);
        writeOut_C(all_cw, ppm_original->width, ppm_original->height);

        free(all_cw);
        Pnm_ppmfree(&ppm_original);
}

/********** decompress40 ********
 *
 * Reads in 32-bit code words and unpacks it from input; converts to CVC form, 
 * uses inverse DCT, converts back to RGB to decompress an image
 *
 * Parameters:
 *      File *fp: pointer to input compressed image file               
 *
 * Return:
 *      none
 * 
 * Expects:
 *      - fp should be a valid pointer to compressed image data
 *
 * Notes: 
 *      - frees memory for temp array and final ppm object 
 *      
 ************************/
void decompress40(FILE *fp)
{
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        A2Methods_UArray2 starter = readHeader(fp, methods, map);
        A2Methods_UArray2 block_data = unpackCW(starter, methods, map);
        block_data = floatConvert(block_data, methods, map);
        Pnm_ppm toOutput = blocks_to_cvcPixels(block_data, methods, map);
        toOutput = CVCtoRGB(toOutput, methods, map, toOutput->denominator);
        writeOut_D(toOutput);

        Pnm_ppmfree(&toOutput);
        methods->free(&block_data);
        methods->free(&starter);
        
}


/*START OF COMPRESSION FUNCTIONS*/

/********** trimBorders ********
 *
 * If width/height odd, function trims borders of PPM image to ensure
 * image dimensions remain even for compression
 *
 * Parameters:
 *      Pnm_ppm untrimmed:   PPM image that must be trimmed
 *      A2Methods_T methods: methods to alter 2D array             
 *
 * Return:
 *      Pnm_ppm: image with even dimensions 
 * 
 * Expects:
 *      - 'untrimmed' is a PPM object
 *      - 'methods' is a valid 'A2Methods_T' object
 *
 * Notes: 
 *      - frees original pixel array if it must be trimmed
 *      
 ************************/
Pnm_ppm trimBorders(Pnm_ppm untrimmed, A2Methods_T methods)
{
        if (untrimmed->width % 2 == 0 && untrimmed->height % 2 == 0) {
                return untrimmed;
        } else {
                int newWidth = untrimmed->width;
                int newHeight = untrimmed->height;
                if (newWidth % 2 != 0) {
                        newWidth--;
                }
                if (newHeight % 2 != 0) {
                        newHeight--;
                }
                A2Methods_UArray2 trimmed = methods->new(newWidth, newHeight, 
                sizeof(struct Pnm_rgb));
                for (int i = 0; i < newHeight; i++) {
                        for (int j = 0; j < newWidth; j++) {
                                Pnm_rgb toAdd = methods->at(trimmed, j, i);
                                Pnm_rgb old = methods->at(untrimmed->pixels, 
                                j, i);
                                toAdd->red = old->red;
                                toAdd->green = old->green;
                                toAdd->blue = old->blue;
                        }
                }
                methods->free(&untrimmed->pixels);
                untrimmed->pixels = trimmed;
                untrimmed->height = newHeight;
                untrimmed->width = newWidth;
        }
        return untrimmed;
}

/********** RGBtoCVC ********
 *
 * Converts RGB PPM image to CVC
 *
 * Parameters:
 *      Pnm_ppm rgb: original RGB image
 *      A2Methods_T methods: methods to alter 2D array 
 *      A2Methods_mapfun:    mapping function to traverse 2D array             
 *
 * Return:
 *      Pnm_ppm: altered image in CVC format
 * 
 * Expects:
 *      - 'rgb' must be a valid RGB PPM image
 *      - 'map' is a valid function pointer
 *      - 'methods' is a valid object
 *
 * Notes: 
 *      - frees memory for RGB pixel array
 *      
 ************************/
Pnm_ppm RGBtoCVC(Pnm_ppm rgb, A2Methods_T methods, A2Methods_mapfun *map)
{
        int cvcWidth = rgb->width;
        int cvcHeight = rgb->height;

        A2Methods_UArray2 *cvcPixels = methods->new(cvcWidth, cvcHeight, 
        sizeof(struct Pnm_cvc));


        /*Custom struct storing all data we need access to in
        the apply function*/
        cvc_closure cl = (cvc_closure)malloc(sizeof(struct cvc_closure));
        if (cl == NULL) {  
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
        }
        cl->pix = cvcPixels;
        cl->rgb_denom = rgb->denominator;
        cl->methods = methods;

        map(rgb->pixels, apply_convert_to_cvc, cl);
        methods->free(&rgb->pixels);
        rgb->pixels = cl->pix;

        free(cl);
        return rgb;
}

/********** apply_convert_to_cvc ********
 *
 * takes single pixel and converts it from RGB to CVC
 *
 * Parameters:
 *      int i: current row index in 2D array
 *      int j: current column index in 2D array
 *      void *array2D: unused 2D array
 *      void *elem: pointer to current RGB pixel
 *      void *cl: closure with information for transformation            
 *
 * Return:
 *      none
 * 
 * Expects:
 *      - 'elem' points to valid RGB pixel
 *
 * Notes: 
 *      - frees memory for pnm_ppm object and array of code words
 *      
 ************************/
void apply_convert_to_cvc(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) array2D;
        cvc_closure cvc = cl;

        struct Pnm_cvc cvc_pixel;
        Pnm_rgb rgb_pixel = elem;

        float dnm = cvc->rgb_denom;

        float r = rgb_pixel->red / dnm;
        float g = rgb_pixel->green / dnm;
        float b = rgb_pixel->blue / dnm;

        cvc_pixel.y = 0.299 * r + 0.587 * g + 0.114 * b;
        cvc_pixel.b = -0.168736 * r - 0.331264 * g + 0.5 * b;
        cvc_pixel.r = 0.5 * r - 0.418688 * g - 0.081312 * b;

        struct Pnm_cvc *temp = cvc->methods->at(cvc->pix, i, j);
        *temp = cvc_pixel;
}

/********** packWords ********
 *
 * packs CVC pixels into 32-bit code words during compression step
 *
 * Parameters:
 *      Pnm_ppm cvcPPM: image in CVC
 *      A2Methods_mapfun *map: mapping function to traverse
 * 
 * Return:
 *      uint32_T t*: array of packed 32-bit code words
 * 
 * Expects:
 *      - 'cvcPPM' must be a valid PPM object in CVC form
 *      - 'map' is valid function pointer
 *
 * Notes: 
 *      - frees memory for array of code words
 *      - each code word is a 2x2 block of pixels
 *      
 ************************/
uint32_t *packWords(Pnm_ppm cvcPPM, A2Methods_mapfun *map)
{
        int numBlocks = (int)ceil((float)cvcPPM->width * cvcPPM->height / 4);
        //Mention it is freed elsewhere
        uint32_t *codewords = malloc(numBlocks * sizeof(uint32_t));
        if (codewords == NULL) {  
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
        }

        map(cvcPPM->pixels, apply_block_packing, codewords);

        return codewords;
}

/********** apply_block_packing ********
 *
 * packs 2x2 block of pixels into 32-bit code word
 *
 * Parameters:
 *      int i: current row index in 2D array
 *      int j: current column index in 2D array
 *      void *array2D: 2D array with pixels
 *      void *elem: unused pointer to current pixel 
 *      void *cl: closure with array of code words        
 *
 * Return:
 *      none
 * 
 * Expects:
 *      - 'array2D' is a valid array of pixels
 *
 * Notes: 
 *      - top-left pixels of every 2x2 block
 *      
 ************************/
void apply_block_packing(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) elem;
        /*Looking for "top-left corners"*/
        if (!(i % 2 == 0 && j % 2 == 0)) {
                return;
        }
        uint32_t *all_codewords = cl;
        struct Pnm_cvc *pix1 = UArray2_at(array2D, i, j);
        struct Pnm_cvc *pix2 = UArray2_at(array2D, i+1, j);
        struct Pnm_cvc *pix3 = UArray2_at(array2D, i, j+1);
        struct Pnm_cvc *pix4 = UArray2_at(array2D, i+1, j+1);
        float avg_Pb = (pix1->b + pix2->b + pix3->b + pix4->b) / 4.0;
        float avg_Pr = (pix1->r + pix2->r + pix3->r + pix4->r) / 4.0;

        unsigned qz_Pb = Arith40_index_of_chroma(avg_Pb);
        unsigned qz_Pr = Arith40_index_of_chroma(avg_Pr);

        int *abcd = DCT(pix1->y, pix2->y, pix3->y, pix4->y);

        uint32_t codeword = 0;
        /*Using Bitpack to pack the 32-bit codeword*/
        codeword = Bitpack_newu(codeword, 4, 0, qz_Pr);
        codeword = Bitpack_newu(codeword, 4, 4, qz_Pb);
        codeword = Bitpack_news(codeword, 6, 8, abcd[3]);
        codeword = Bitpack_news(codeword, 6, 14, abcd[2]);
        codeword = Bitpack_news(codeword, 6, 20, abcd[1]);
        codeword = Bitpack_newu(codeword, 6, 26, abcd[0]);

        int index = ((j / 2) * UArray2_width(array2D) / 2) + (i / 2);
        all_codewords[index] = codeword;

        free(abcd);
}

/********** DCT ********
 *
 * calculates Discrete Cosine Transformation on 4 luminance values from 2x2 
 * block of pixels and returns quantized coefficient
 *
 * Parameters:
 *      float y1: 1st Y value
 *      float y2: 2nd Y value
 *      float y3: 3rd Y value
 *      float y4: 4th Y value          
 *
 * Return:
 *      int *: pointer to array of 4 quantized DCT coeff
 * 
 * Expects:
 *      - all Y values must be valid floating point numbers
 *
 * Notes: 
 *      - a is computed through average of 4 Y values
 *      - b, c, d are computed by difference of Y values and quantized
 *        within range [-0.3 to 0.3]
 *      
 ************************/
int *DCT(float y1, float y2, float y3, float y4)
{
        float flts[4];
        int *qz = malloc(4 * sizeof(int));
        if (qz == NULL) {  
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
        }


        /*a, b, c, d values*/
        flts[0] = (y4 + y3 + y2 + y1) / 4.0;
        flts[1] = (y4 + y3 - y2 - y1) / 4.0;
        flts[2] = (y4 - y3 + y2 - y1) / 4.0;
        flts[3] = (y4 - y3 - y2 + y1) / 4.0;

        /*Transformation for a*/
        qz[0] = round(flts[0] * 63);

        /*Quantizing b, c, d*/
        for (int i = 1; i < 4; i++) {
                if (flts[i] < -0.3) {
                        flts[i] = -0.3;
                } else if (flts[i] > 0.3) {
                        flts[i] = 0.3;
                }
                qz[i] = (int)round((flts[i] / 0.3) * 31);
        }

        return qz;
}

/********** writeOut_C ********
 *
 * writes the packed 32-bit codewords to output in Big-endian order 
 * along with a relevant header for the data
 *
 * Parameters:
 *      uint32_t *all_cw:         array pointer of the packed codewords
 *      int width:                width of the image that was compressed
 *      int height:               height of the image that was compressed
 *
 * Return:
 *      none
 *
 * Expects:
 *      - 
 *
 * Notes: 
 *      - Uses the putchar() function to write to standard output
 *      
 ************************/
void writeOut_C(uint32_t *all_cw, int width, int height)
{
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
        
        int length = (width * height / 4);
        for (int i = 0; i < length; i++) {
                uint32_t codeword = all_cw[i];

                uint32_t mask = 255; /*8 bits, all 1s mask*/
                /*Big-endian order*/
                putchar((codeword >> 24) & mask);
                putchar((codeword >> 16) & mask);
                putchar((codeword >> 8) & mask);
                putchar(codeword & mask);
        }
}

/*END OF COMPRESSION FUNCTIONS*/

/*START OF DECOMPRESSION FUNCTIONS*/


/********** readHeader ********
 *
 * reads in the compressed 32-bit codewords along with the header 
 *
 * Parameters:
 *      FILE *input:              file pointer to an opened file for reading
 *      A2Methods_T methods:      methods suite
 *      A2Methods_mapfun *map:    default mapping funciton (row-major)
 *
 * Return:
 *      A2Methods_UArray2:      UArray2 of all the 32-bit codewords 
 *
 * Expects:
 *      - Header includes a with and height and is followed by a newline then
 *        a sequence of codewords
 *
 * Notes: 
 *      - 
 *      
 ************************/
A2Methods_UArray2 readHeader(FILE *input, A2Methods_T methods, 
A2Methods_mapfun *map)
{
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
        &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        /*One codeword for 4 pixels*/
        width /= 2;
        height /= 2;

        return setupPixmap(width, height, methods, map, input);
}

/********** setupPixmap ********
 *
 * creates and fills a UArray2 of all the 32-bit codewords
 *
 * Parameters:
 *      int width:                width to make the codewords Uarray2
 *      int height:               height to make the codewords Uarray2
 *      A2Methods_T methods:      methods suite
 *      A2Methods_mapfun *map:    default mapping funciton (row-major)
 *      FILE *fp:                 file pointer to an opened file for reading
 *
 * Return:
 *      A2Methods_UArray2 cw_all: UArray2 of all the 32-bit codewords 
 *
 * Expects:
 *      - Width and height are set to hold the exact amount of codewords
 *        available
 *
 * Notes: 
 *      - Calls an apply function to read the codewords in Big Endian order
 *      
 ************************/
A2Methods_UArray2 setupPixmap(int width, int height, A2Methods_T methods, 
A2Methods_mapfun *map, FILE *fp)
{
        A2Methods_UArray2 cw_all = methods->new(width, height, 
        sizeof(uint64_t));
        map(cw_all, apply_handle_codewords, fp);

        return cw_all;
}

/********** apply_handle_codewords ********
 *
 * reads in a codeword in big endian order and stores it in the UArray2
 *
 * Parameters:
 *      int i:                    current col index
 *      int j:                    current row index
 *      void *array2D:            pointer to codewords UA2
 *      void *elem:               pointer to the element at i, j in the UA2
 *      void *cl:                 closure pointing to the file being read
 *
 * Return:
 *      None 
 *
 * Expects:
 *      - Elem and cl are pointing to uint64_ts and FILEs respectively
 *
 * Notes: 
 *      - array2D, i, j are all unused
 *      - updates the uint64_t objects in the all_cw UA2
 *      
 ************************/
void apply_handle_codewords(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) array2D;
        (void) i;
        (void) j;

        uint64_t *cw = elem;
        FILE *toRead = cl;
        assert(toRead != NULL);

        uint64_t holder = 0;
        uint64_t word_being_built = 0;

        /*Reading in assuming that codewords are in Big Endian order*/
        for (int x = 24; x >= 0; x = x - 8) {
                holder = getc(toRead);
                word_being_built = word_being_built | (holder << x);
        }
        *cw = word_being_built;
}

/********** unpackCW ********
 *
 * unpacks the codewords UArray2 into a Uarray2 of codeword data structs
 *
 * Parameters:
 *      A2Methods_UArray2 ua2:    UA2 of the codewords
 *      A2Methods_T methods:      methods suite
 *      A2Methods_mapfun *map:    default mapping funciton (row-major)
 *
 * Return:
 *      A2Methods_UArray2 unpacked: UArray2 of structs containing the data from
 *                                  each codeword
 *
 * Expects:
 *      - 'unpacked' is freed elesewhere after its done being used
 *
 * Notes: 
 *      - Calls an apply function to unpack each codeword
 *      - Uses the cw_data struct to store a, b, c, d, Pb, Pr as uint64_ts
 *      
 ************************/
A2Methods_UArray2 unpackCW(A2Methods_UArray2 ua2, A2Methods_T methods, 
A2Methods_mapfun *map)
{
        struct cvc_closure *cl = malloc(sizeof(*cl));
        if (cl == NULL) {  
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
        }

        A2Methods_UArray2 unpacked = methods->new(methods->width(ua2), 
        methods->height(ua2), sizeof(struct cw_data));

        /*Using a closure struct to be able to pass the apply function both the
        methods suite and the new UA2, unapcked*/
        cl->pix = unpacked;
        cl->methods = methods;

        map(ua2, apply_unpack_codewords, cl);

        free(cl);
        return unpacked;
}

/********** apply_unpack_codewords ********
 *
 * unpacks the data in a codeword and stores it in a separate struct
 *
 * Parameters:
 *      int i:                    current col index
 *      int j:                    current row index
 *      void *array2D:            pointer to codewords UA2
 *      void *elem:               pointer to the element at i, j in the UA2
 *      void *cl:                 closure pointer to the object containing
 *                                the Uarray2 of cw_data structs
 *
 * Return:
 *      None 
 *
 * Expects:
 *      - elem and cl are pointing to initialized uint64_ts and cvc_closures 
 *        respectively
 *   
 * Notes: 
 *      - array2D is unused
 *      - updates the cw_data objects in the cvc->pix UA2 (aka unpacked)
 *      
 ************************/
void apply_unpack_codewords(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) array2D;
        
        uint64_t currCW = *((uint64_t *)elem);
        struct cvc_closure *closure = cl;
        cw_data CWDATA = closure->methods->at(closure->pix, i, j);

        CWDATA->Pr = Bitpack_getu(currCW, 4, 0);
        CWDATA->Pb = Bitpack_getu(currCW, 4, 4);
        CWDATA->d = Bitpack_gets(currCW, 6, 8);
        CWDATA->c = Bitpack_gets(currCW, 6, 14);
        CWDATA->b = Bitpack_gets(currCW, 6, 20);
        CWDATA->a = Bitpack_getu(currCW, 6, 26);
}

/********** floatConvert ********
 *
 * converts the codeword data from uint64_ts to floats and applys an inverse
 * discrete cosine transformation
 *
 * Parameters:
 *      A2Methods_UArray2 ua2:    UA2 of the cw_data structs
 *      A2Methods_T methods:      methods suite
 *      A2Methods_mapfun *map:    default mapping funciton (row-major)
 *
 * Return:
 *      A2Methods_UArray2 ua2: UArray2 of structs containing the data from
 *                             each codeword, now with float variables
 *
 * Expects:
 *      - 
 *
 * Notes: 
 *      - Calls an apply function to convert each cw_data struct
 *      
 ************************/
A2Methods_UArray2 floatConvert(A2Methods_UArray2 ua2, A2Methods_T methods, 
A2Methods_mapfun *map)
{
        void *cl = NULL;
        (void) methods;

        map(ua2, apply_float_conversions, cl);

        return ua2;
}

/********** apply_float_conversions ********
 *
 * converts a, b, c, d, Pb, Pr in a cw_data struct to floats and sends them 
 * through an inverse DCT as needed
 *
 * Parameters:
 *      int i:                    current col index
 *      int j:                    current row index
 *      void *array2D:            pointer to the cw_data UA2
 *      void *elem:               pointer to the element at i, j in the UA2
 *      void *cl:                 closure void pointer 
 *
 * Return:
 *      None 
 *
 * Expects:
 *      - elem is pointing to a cw_data struct with initialized uint64_t vals
 *   
 * Notes: 
 *      - array2D, i, j, cl are all unused
 *      - calls the inverseDCT function for the float a, b, c, d values
 *      - updates the cw_data objects in the ua2
 *      
 ************************/
void apply_float_conversions(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) i;
        (void) j;
        (void) array2D;
        (void) cl;
        
        if (elem == NULL) {
                fprintf(stderr, 
                "Error: Null pointer in apply_float_conversions\n");
                exit(EXIT_FAILURE);
        }

        cw_data curr = ((cw_data)elem);

        curr->Pb1 = Arith40_chroma_of_index(curr->Pb);
        curr->Pr1 = Arith40_chroma_of_index(curr->Pr);
        
        /*a value*/
        curr->val1 = (float)(curr->a) / 63.0;

        /*b, c, d values*/
        curr->val2 = (float)(curr->b) * 0.3f / 31.0f;
        curr->val3 = (float)(curr->c) * 0.3f / 31.0f;
        curr->val4 = (float)(curr->d) * 0.3f / 31.0f;

        inverseDCT(curr);
}

/********** inverseDCT ********
 *
 * applys the inverse of a discrete cosine transformation to get the luminance
 * values from a, b, c, & d
 *
 * Parameters:
 *      cw_data x:                struct containing the a,b,c,d floats
 *      
 * Return:
 *      none
 *
 * Expects:
 *      - the val1-4 floats in the cw_data struct are initialized
 *
 * Notes: 
 *      - Updates the val1-4 variables in each cw_data struct so that they
 *        now represent luma values
 *      
 ************************/
void inverseDCT(cw_data x)
{
        float y1 = x->val1 - x->val2 - x->val3 + x->val4;
        float y2 = x->val1 - x->val2 + x->val3 - x->val4;
        float y3 = x->val1 + x->val2 - x->val3 - x->val4;
        float y4 = x->val1 + x->val2 + x->val3 + x->val4;

        /*val1,2,3,4 now represent each y value for each 2x2 block*/
        /*a,b,c,d values are no longer needed*/
        x->val1 = y1;
        x->val2 = y2;
        x->val3 = y3;
        x->val4 = y4;
}

/********** blocks_to_cvcPixels ********
 *
 * creates and fills a pnm_ppm with pixels in cvc format from the 
 * Uarray2 of cw_data
 *
 * Parameters:
 *      A2Methods_UArray2 ua2:    UA2 of the cw_data structs
 *      A2Methods_T methods:      methods suite
 *      A2Methods_mapfun *map:    default mapping funciton (row-major)
 *
 * Return:
 *      Pnm_ppm final:  Pnm_ppm representing the full decompressed image 
 *                      with pixels in cvc format
 *
 * Expects:
 *      - The pnm_ppm being created is freed at the end of decompress40
 *
 * Notes: 
 *      - Calls an apply function to convert transfer the data from a single 
 *        cw_data struct into 4 different cvc pixels
 *      - We found a denominator value of 255 to be best for flowers.ppm
 *      
 ************************/
Pnm_ppm blocks_to_cvcPixels(A2Methods_UArray2 ua2, A2Methods_T methods, 
A2Methods_mapfun *map)
{
        int denominator = 255;
        int cvcWidth = UArray2_width(ua2) * 2;
        int cvcHeight = UArray2_height(ua2) * 2;
        A2Methods_UArray2 cvcPixels = methods->new(cvcWidth, cvcHeight, 
        sizeof(struct Pnm_cvc));

        /*Using a closure struct to be able to pass the apply function both the
        methods suite and the new cvcPixels UA2*/
        cvc_closure cl = (cvc_closure)malloc(sizeof(struct cvc_closure));
        if (cl == NULL) {  
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
        }
        cl->pix = cvcPixels;
        cl->methods = methods;

        map(ua2, apply_make_cvc_pixelmap, cl);

        Pnm_ppm final = (Pnm_ppm)malloc(sizeof(struct Pnm_ppm));
        final->pixels = cl->pix;
        final->denominator = denominator;
        final->methods = methods;
        final->width = cvcWidth;
        final->height = cvcHeight;

        free(cl);
        return final;
}

/********** apply_make_cvc_pixelmap ********
 *
 * converts the floats in a cw_data struct to y, Pb, Pr vals for a 2x2 block
 *
 * Parameters:
 *      int i:                    current col index
 *      int j:                    current row index
 *      void *array2D:            pointer to the cw_data UA2
 *      void *elem:               pointer to the element at i, j in the UA2
 *      void *cl:                 closure pointer to the object containing
 *                                the cvc_pixel pixelmap
 *
 * Return:
 *      None 
 *
 * Expects:
 *      - elem is pointing to a cw_data struct with initialized float vals
 *      - the object cl points to has an allocated pix object of correct
 *        size
 *   
 * Notes: 
 *      - array2D is unused
 *      
 ************************/
void apply_make_cvc_pixelmap(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) array2D;
        
        cw_data currCW = ((cw_data)elem);
        cvc_closure cvc = cl;

        struct Pnm_cvc *pix1 = cvc->methods->at(cvc->pix, (i*2), (j*2));
        struct Pnm_cvc *pix2 = cvc->methods->at(cvc->pix, (i*2)+1, (j*2));
        struct Pnm_cvc *pix3 = cvc->methods->at(cvc->pix, (i*2), (j*2)+1);
        struct Pnm_cvc *pix4 = cvc->methods->at(cvc->pix, (i*2)+1, (j*2)+1);

        pix1->y = currCW->val1;
        pix2->y = currCW->val2;
        pix3->y = currCW->val3;
        pix4->y = currCW->val4;

        pix1->b = currCW->Pb1;
        pix2->b = currCW->Pb1;
        pix3->b = currCW->Pb1;
        pix4->b = currCW->Pb1;

        pix1->r = currCW->Pr1;
        pix2->r = currCW->Pr1;
        pix3->r = currCW->Pr1;
        pix4->r = currCW->Pr1;
}

/********** CVCtoRGB ********
 *
 * converts image from CVC to RGB
 *
 * Parameters:
 *      Pnm_ppm cvc: image in CVC color space
 *      A2Methods_T methods: methods for altering 2D array
 *      A2Methods_mapfun *map: mapping function to help traverse array
 *      int dnm: denominator to scale RGB values              
 *
 * Return:
 *      Pnm_ppm: image transformed back to RGB format
 * 
 * Expects:
 *      - 'cvc' is a valid Pnm_ppm
 *      - 'methods' is a valid A2Methods_T object
 *      - 'map' is a function pointer
 *
 * Notes: 
 *      - frees CVC pixel array
 *      
 ************************/
Pnm_ppm CVCtoRGB(Pnm_ppm cvc, A2Methods_T methods, 
A2Methods_mapfun *map, int dnm)
{
        int rgbWidth = cvc->width;
        int rgbHeight = cvc->height;

        A2Methods_UArray2 *rgbPixels = methods->new(rgbWidth, rgbHeight, 
        sizeof(struct Pnm_rgb));

        //Need to free this eventually
        /*Custom struct storing all data we need access to in
        the apply function*/
        cvc_closure cl = (cvc_closure)malloc(sizeof(struct cvc_closure));
        if (cl == NULL) {  
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
        }
        cl->pix = rgbPixels;
        cl->rgb_denom = dnm;
        cl->methods = methods;

        map(cvc->pixels, apply_convert_to_rgb, cl);
        methods->free(&cvc->pixels);
        cvc->pixels = cl->pix;

        free(cl);
        return cvc;
}

/********** apply_convert_to_rgb ********
 *
 * takes single pixel and converts it from CVC to RGB
 *
 * Parameters:
 *      int i: current row index in 2D array
 *      int j: current column index in 2D array
 *      void *array2D: unused 2D array
 *      void *elem: pointer to current CVC pixel
 *      void *cl: closure with information for transformation            
 *
 * Return:
 *      none
 * 
 * Expects:
 *      - 'elem' points to valid RGB pixel
 *
 * Notes: 
 *      - frees memory for pnm_ppm object and array of code words
 *      
 ************************/
void apply_convert_to_rgb(int i, int j, void *array2D, void *elem, void *cl)
{
        (void) array2D;
        cvc_closure rgb = cl;

        Pnm_rgb rgb_pixel = rgb->methods->at(rgb->pix, i, j);
        struct Pnm_cvc *cvc_pixel = elem;

        float dnm = (float)rgb->rgb_denom;

        float y = cvc_pixel->y;
        float b = cvc_pixel->b;
        float r = cvc_pixel->r;

        float toR = (1.0 * y + 0.0 * b + 1.402 * r);
        float toG = (1.0 * y - 0.344136 * b - 0.714136 * r);
        float toB = (1.0 * y + 1.772 * b + 0.0 * r);

        /*Check for edge cases where RGB values get our of range*/
        if (toR < 0) {
                toR = 0.0;
        }
        if (toR > 1) {
                toR = 1.0;
        }
        if (toG < 0) {
                toG = 0.0;
        }
        if (toG > 1) {
                toG = 1.0;
        }
        if (toB < 0) {
                toB = 0.0;
        }
        if (toB > 1) {
                toB = 1.0;
        }

        toR = toR * dnm;
        toG = toG * dnm;
        toB = toB * dnm;

        rgb_pixel->red = (int)(toR);
        rgb_pixel->green = (int)(toG);
        rgb_pixel->blue = (int)(toB);
}

/********** writeOut_D ********
 *
 * Writes a decompressed PPM image to stdout 
 *
 * Parameters:
 *      pnm_ppm final: pointer to PPM image             
 *
 * Return:
 *      none
 * 
 * Expects:
 *      - final must be a valid Pnm_ppm object
 *
 * Notes: 
 *      - writes image in PPM format to stdout
 *      
 ************************/
void writeOut_D(Pnm_ppm final)
{
    Pnm_ppmwrite(stdout, final);
}

/*END OF DECOMPRESSION FUNCTIONS*/
