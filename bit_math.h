/*
    Param: A byte
    Returns: Number of "free" bits in the byte
*/
int BitCounter(unsigned char target);


/*
    Param: A byte
    Returns: Index of first free bit in byte
    or -1 if none found
*/
int FindFreeBit(unsigned char target);


/*
    Param: int base and exponent
    Returns: base to the exponent power
*/
int power(int base, int exp);


/*
    Param: A byte and a bit index between 0 and 7
    Turns given bit into a 0

*/
int FlipBitUsed(unsigned char byte, int bit);