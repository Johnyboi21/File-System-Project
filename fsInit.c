/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"

#define SIGNATURE 0xC0FFEE
#define MAX_DE_NAME 256

typedef struct VCB{
	uint64_t size_of_block; //size of a individual block
	uint64_t number_of_blocks; //counts the number of blocks
	uint64_t blocks_available; //holds blocks available
	uint64_t freespace_available; //holds the number of free blocks available
	uint64_t bitmap_starting_index; //where the bitmap starts


	uint64_t signature; //used to check if own the 

} VCB;

typedef struct DE{
	char name[MAX_DE_NAME];
	uint64_t beginning_block;
	uint64_t size;
	uint64_t location;
	time_t creation_date;
	time_t last_modified;
} DE;

//might have to change add externt ot be used throughout the project
VCB* vcb;
uint32_t* bitmap;

void printVCB();
void initBitmap();

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize){


	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	vcb = malloc(blockSize);
	if(vcb == NULL){ return -1; }; // returns a error that the Volume Control Block isn't initialized

	vcb->number_of_blocks = numberOfBlocks;
	vcb->blocks_available = numberOfBlocks;
	vcb->size_of_block = blockSize;
	vcb->signature = SIGNATURE;

	printf("\nVCB Initialized\n");
	printVCB();
	return 0;
}

void initBitmap(){
	//set up bitmap
	bitmap = malloc(vcb->size_of_block * 5);



}

	
	
void exitFileSystem (){
	free(vcb);
	free(bitmap);
	printf ("System exiting\n");
	}

void printVCB(){
	printf("Size of a Block: %ld\nNumber of Blocks: %ld\nBlocks Available: %ld\nSignature: %ld\n", vcb->size_of_block, vcb->number_of_blocks, vcb->blocks_available, vcb->signature);
}