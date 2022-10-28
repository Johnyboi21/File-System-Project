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
#include "directory_entry.h"
#include "free_space_helpers.h"
#include "constants.h"

#define SIGNATURE 0xC0FFE
//#define MAX_DE_NAME 256
//#define MAX_DIRENTRIES 51
#define DIRECTORY_BYTE_SIZE 60


/* Defined in directory_entry.h
typedef struct DE{
	char name[MAX_DE_NAME];
	uint64_t beginning_block;
	uint64_t size;
	uint64_t location;
	time_t creation_date;
	time_t last_modified;
} DE;
*/

typedef struct Directory{
	char directory_name[MAX_DE_NAME];
	
}Directory;


//might have to change add externt ot be used throughout the project


void printVCB();
void initBitmap();
void initRootDir();
//int getFreeBlock();


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize){


	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//loading the VCB block into 
	vcb = malloc(blockSize);
	if(vcb == NULL){ return -1; }; // returns a error that the Volume Control Block isn't initialized

	LBAread(vcb, 1, 0);

	if(vcb->signature == SIGNATURE){
		printf("\nVCB already exist!\n");
		printVCB();

	}
	else{
		vcb->size_of_block = blockSize;
		vcb->number_of_blocks = numberOfBlocks;
		vcb->blocks_available = numberOfBlocks;
		vcb->signature = SIGNATURE;
		initBitmap();
		LBAwrite(vcb, 1, 0);

		//free block map that represents the whole volume
		//beginds directly after the VCB;


		printf("\nVCB Initialized!!\n");
		printVCB();

        DirectoryInit(NULL);
        printf("\nMade root!\n");
		printVCB();

	}
	return 0;
}


void initBitmap(){

	//allocate bitmap i.e  5 * 512 (bytes)
	bitmap = malloc(5 * vcb->size_of_block);

	//size of blocks in bytes -not bit.

	//first byte - 0000 0011 (0 means allocated, 1 free).
	bitmap[0] = 0x03;
	
	//Set rest of "bits" as free
	for(int i = 1; i < 5 * vcb->size_of_block; i++){
		bitmap[i] = 0xFF; //i.e 1111 1111
	}

	//step d of free space
	LBAwrite(bitmap, 5, 1);

	/*
	Return the starting block number of the free space
	 to the VCB init that called you so it knows how to set the 
	 VCB structure variable that indicates where free space starts.
	 Or mark it yourself if the VCB is a global structure.
	*/
	vcb->bitmap_starting_index = 1;
}


void initRootDir(){

}
	
	
void exitFileSystem (){
    LBAwrite(vcb, 1, 0);
    LBAwrite(bitmap, 5, 1);
	free(vcb);
	free(bitmap);
	printf ("System exiting\n");
	}

void printVCB(){
	printf("Size of a Block: %ld\nNumber of Blocks: %ld\nBlocks Available: %ld\nSignature: %ld\n", vcb->size_of_block, vcb->number_of_blocks, vcb->blocks_available, vcb->signature);
}