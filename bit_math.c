#include "bit_math.h"
#include <stdio.h>
/*
    Param: A byte
    Returns: Number of "free" bits in the byte
*/
int BitCounter(unsigned char target){
    int x = 0; // Number of free bits found
    int bit = 0x80; // Bit we are checking; goes high to low

    // Check each bit in target
    while(bit > 0){
        if((target & bit) == bit){
            x++;
        }
        bit = bit/2;
    }

    return x;
}

/*
    Param: A byte
    Returns: Index of first free bit in byte
    or -1 if none found
*/
int FindFreeBit(unsigned char target){
    int x = 0; // Number of free bits found
    int bit = 0x80; // Bit we are checking; goes high to low

    // Check each bit in target. Return bit if free
    while(bit > 0){
        if((target & bit) == bit){
            return x; 
        }
        x++;
        bit = bit/2;
    }

    return -1;
}

/*
    Param: int base and exponent
    Returns: base to the exponent power
*/
int power(int base, int exp){
    int result = 1;
    for(int i = 0; i < exp; i++){
        result = result * base;
    }

    return result;
}


/*
    Param: A byte and a bit index between 0 and 7
    Turns given bit into a 0

    Returns: Byte with changes
*/
int FlipBitUsed(unsigned char byte, int bit){
    bit = power(2, 7-bit);  // 7-bit because index is left to right
    byte = byte & ~bit;

    return byte;
}