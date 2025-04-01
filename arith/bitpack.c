/*
*     bitpack.c
*     jadkin05, alall01, 10/22/2024
*     arith
*     
*     Function implementations for the Bitpack interface
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <except.h>
#include <stdint.h>
#include <math.h>
#include "bitpack.h"
#include <inttypes.h>

/********** Bitpack_fitsu ********
 *
 * Allows us to determine if unsigned int can fit in a given bit width
 *
 * Parameters:
 *      uint64_t n:     unsigned int to be checked                    
 *      unsigned width: number of bits available to store info             
 *
 *
 * Return:
 *      returns true if 'n' fits within 'width' bits; otherwise, returns false
 *
 * Expects:
 *      - 'width' must be a valid bit size (an int between 0 and 64)
 *
 * Notes: 
 *      - checks to see if 'n' is <= max val that is shown in 'width' bits
 *      
 ************************/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        if (width >= 64) {
                return true;
        }
        /*Place a single 1 at width position*/
        uint64_t x = 1 << width;
        return n <= x;
}

/********** Bitpack_fitss ********
 *
 * Allows us to determine if signed int can fit in a given bit width
 *
 * Parameters:
 *      int64_t n:      the signed int to be checked                
 *      unsigned width: number of bits that can be used for storage     
 *
 * Return:
 *      return true if 'n' fits in 'width' bits as a signed int; otherwise
 *      false
 *
 * Expects:
 *      - `'width' must be valid (int between 0 and 64)
 *
 * Notes: 
 *      - handles negative and positive values by using their ranges
 *      
 ************************/
bool Bitpack_fitss(int64_t n, unsigned width)
{
        bool check = false;
        if (width == 0){
                check = false;
        } else if (n >= 0) {
                /*Account for sign bit*/
                check = Bitpack_fitsu(2 * n, width);
        } else {
                /*Convert 2's complement and adjust range*/
                check = Bitpack_fitsu((~(n-1)) * 2 - 1, width);
        }
        return check;
}

/********** Bitpack_getu ********
 *
 * t
 *
 * Parameters:
 *      uint64_t word:  64-bit word where we extract value                 
 *      unsigned width: width of bit-field needed to be extracted        
 *      unsigned lsb:   least significant bit 
 * 
 *
 * Return:
 *      unsigned value of type uint64_t that is extracted from bit-field
 *
 * Expects:
 *      - 'width' must be valid (int between 0 and 64)
 *      - bit-field must be smaller than word (width + lsb <= 64)
 *
 * Notes: 
 *      - return 0 if 'width' = 0
 *      
 ************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert (width <= 64);
        assert (width + lsb <= 64);
        if (width == 0) {
                return 0;
        }

        /*Mask of 1s shifted to be at the lsb*/
        uint64_t mask = (uint64_t)((1 << width) - 1);
        uint64_t val = word >> lsb;
        
        /*Apply the mask*/
        return val & mask;
}

/********** Bitpack_gets ********
 *
 * extracts signed value from bit-field within 64-bit word
 *
 * Parameters:
 *      uint64_t word: 64-bit word to extract signed value                 
 *      unsigned width: width of bit-field to extract                   
 *      unsigned lsb: least significant bit               
 *
 * Return:
 *      signed value of type int64_t extracted from bit-field
 *
 * Expects:
 *      - 'width' must be valid (int between 0 and 64)
 *      - bit-field must be smaller than word (width + lsb <= 64)
 *
 * Notes: 
 *      - if extracted value has sign bit, the result will be signed value
 *      
 ************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert (width <= 64);
        assert (width + lsb <= 64);
        if (width == 0) {
            return 0;
        }

        uint64_t getu = Bitpack_getu(word, width, lsb);
        uint64_t msb = (uint64_t)(1 << (width - 1)); /*Creating all 1s mask*/

        /*Transforming the unsigned answer to a signed int*/
        if (getu & msb) { /*Check for leading sign bit (negative num)*/
            
            /*Mask of all 1s at positions greater than width*/
            uint64_t mask = ~((uint64_t)((1 << width) - 1));

            getu = getu | mask; /*Now in 2's complement form*/
        }

        int64_t final = (int64_t)getu;
        return final;
}

/*Overflow Exception*/
Except_T Bitpack_Overflow = { "Overflow packing bits" };

/********** Bitpack_newu ********
 *
 * adds unsigned value to bit-field within 64-bit word and returns a new word
 *
 * Parameters:
 *      uint64_t word: 64-bit word where value should be inserted          
 *      unsigned width: width of bit-field                
 *      unsigned lsb: least significant bit 
 *      uint64_t value: unsigned value to insert                         
 *
 * Return:
 *      new 64-bit word of uint64_t type with updated bit-field
 *
 * Expects:
 *      - 'width' must be valid (int between 0 and 64)
 *      - bit-field must be smaller than word (width + lsb <= 64)
 *      - 'value' must be within 'width' bits
 *
 * Notes: 
 *      - overflow exception is raised if 'value' is not within 'bits'
 *      
 ************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, 
unsigned lsb, uint64_t value)
{
        assert (width <= 64);
        assert (width + lsb <= 64);

        if (Bitpack_fitsu(value, width) != true) {
                RAISE(Bitpack_Overflow);
                return word;
        }
        /*Creating and using mask to remove section of 
        size width starting at lsb*/
        uint64_t mask = ((uint64_t)((1 << width) - 1)) << lsb;
        word = word & ~mask;

        /*Place value at the lsb*/
        uint64_t new_word = word | (value << lsb); 
        
        return new_word;
}

/********** Bitpack_news ********
 *
 * adds signed value to bit-field within 64-bit word and returns a new word
 *
 * Parameters:
 *      uint64_t word: 64-bit word where value should be inserted  
 *      unsigned width: width of bit-field                  
 *      unsigned lsb: least significant bit 
 *      uint64_t value: unsigned value to insert                       
 *
 * Return:
 *      new 64-bit word of uint64_t type with updated bit-field
 *
 * Expects:
 *      - 'width' must be valid (int between 0 and 64)
 *      - bit-field must be smaller than word (width + lsb <= 64)
 *      - 'value' must be within 'width' bits
 *
 * Notes: 
 *      - overflow exception is raised if 'value' is not within 'bits'
 *      - converts signed values to unsigned prior to placing into word
 *      
 ************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, 
unsigned lsb, int64_t value)
{
        assert (width <= 64);
        assert (width + lsb <= 64);

        if (Bitpack_fitss(value, width) != true) {
                RAISE(Bitpack_Overflow);
                return word;
        }

        /*Unsigned conversion and placement of new_val at lsb*/
        uint64_t new_val = (uint64_t)value << (64 - width);
        new_val = new_val >> (64 - lsb - width);

        /*Zeroing mask for region aove the spot where value will be added*/
        uint64_t maskTemp = (((1 << (64 - lsb - width)) - 1) << (width + lsb));
        /*Combine with zeroing mask for region before the spot where val 
        will be added*/
        uint64_t mask = maskTemp + ((1 << lsb) -1);

        /*clear out target areas and replace with new_val*/
        uint64_t final = (word & mask) | new_val;
        return final;
}
