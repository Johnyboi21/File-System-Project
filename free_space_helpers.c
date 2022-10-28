/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:  Richard Aguilar
*         Melisa Sever
*         Ryan Scott
*         Jonathan Valadez
*
* Student IDs: 977075554
*              921662115
*              921814228
*              922274961
*
* GitHub Name: raguilar0917
* Group Name: The Beerman Fan Club
* Project: Basic File System
*
* File: free_space_helpers.c
*
* Description: Implements functions from associated header.
    Contains functions useful for manipulating free space, 
    as well as sending free blocks to other functions
*
* 
*
**************************************************************/

#include "free_space_helpers.h"
#include "bit_math.h"
#include "constants.h"
#include <stdio.h>


/*
    Returns first free block starting from param
*/
int GetFreeBlock(int start_pos){
    int block = -1;             // Ret val
    int location_in_byte = 0;   // Will be added to block if free found

    // Loop over bitmap; break if free bit found
        // This can be optimized a bit if we know which bytes are not free
    for(int i = start_pos; i < 5 * vcb->size_of_block; i++){
		location_in_byte = FindFreeBit(bitmap[i]);
        if(location_in_byte != -1){
            // One byte (i) and offset by location_in_bytes bits
            block = i*8+location_in_byte;
            break;
        }
	}

    return block;
}


/*
    Param: int number of blocks to receive
    Returns: Pointer to integer array of blocks

    Returns NULL if there are fewer than requested blocks available
*/
int* GetNFreeBlocks(int blocks){
    int free_blocks[blocks];    // Array to hold blocks

    int start_pos = 0;          // Starting read position for bitmap
    int possible_block = -1;    // Set to ret of GetFreeBlock

    // Find blocks free blocks and add to array
    for(int i = 0; i < blocks; i++){
        possible_block = GetFreeBlock(start_pos);
        if(possible_block == -1){   // Failed to find enough free blocks
            return NULL;
        }
        else{
            free_blocks[i] = possible_block;
            start_pos = possible_block/8;   // Start searching from that byte next time
            MarkOneBlockUsed(possible_block);
        }
    }

    int* ret = free_blocks;
    return ret; 
}

/*
    Param: int array of blocks, size of arr
    Returns number of blocks marked as used
*/
int MarkBlocksUsed(int blocks[], int size){
    int marked = 0;
    int block = 0;      // Current working block
    int byte = 0;       // Index into bitmap of the block
    for(int i = 0; i < size; i++){
        block = blocks[i];
        byte = block/8;
        // Set byte to new value after flip
        bitmap[byte] = FlipBitUsed(bitmap[byte], (block % 8));
        marked++;
    }

    return marked;
}

/*
    Version of MarkBlocksUsed that marks only a single block
    Param: int block to be marked
    Returns: number of marked blocks
*/
int MarkOneBlockUsed(int block){
    int arr[1] = {block};
    return MarkBlocksUsed(arr, 1);
}