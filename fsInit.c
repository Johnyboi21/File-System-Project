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

        bitmap = malloc(vcb->number_of_blocks);

        LBAread(bitmap, 5, 1);

	}
	else{
		vcb->size_of_block = blockSize;
		vcb->number_of_blocks = numberOfBlocks;
		vcb->blocks_available = numberOfBlocks;
		vcb->signature = SIGNATURE;
		
		LBAwrite(vcb, 1, 0);

        initBitmap();
		//free block map that represents the whole volume
		//begins directly after the VCB;


		printf("\nVCB Initialized!!\n");
		printVCB();


        new_dir_data* data = DirectoryInit(NULL);
        vcb->root_starting_index = data->location;

        data = NULL;
        free(data);
        printf("\nMade root!\n");

        vcb->root_size = 30;

        int i = GetFreeBlock(0);
        printf("First free block is now at %d\n", i);

       // initTestDirs();

        printVCB();
	}


    //parsePath("/Cool Directory\0");
 //   fs_mkdir("/NewDir\0", 0);
    current_working_dir = malloc(MAX_PATH_LENGTH);
    strncpy(current_working_dir, "/\0", 2);


	return 0;
}


void initBitmap(){

	//allocate bitmap 
        // It needs to be number_of_blocks/8 bytes large; 1 bit = 1 block
    int bytes = vcb->number_of_blocks/8;
    if(vcb->number_of_blocks % 8 != 0){
        bytes++;
    }

    int blocks = bytes/vcb->size_of_block;
    if(bytes % vcb->size_of_block != 0){
        blocks++;
    }

    // We only need `bytes` size, but LBAwrite wants it in blocks
    bitmap = malloc(vcb->size_of_block*blocks);

	
	// Initialize all blocks to free
        
	for(int i = 0; i < bytes; i++){
		bitmap[i] = 0xFF; //i.e 1111 1111
	}

    // Note that, because of satisfying LBAwrite, we have more bits than blocks
        // Blocks after `bytes` are marked not free because of this
    for(int i = bytes; i < vcb->size_of_block*blocks; i++){
		bitmap[i] = 0x00; //i.e 1111 1111
	}


    vcb->bitmap_size_blocks = blocks;
    vcb->bitmap_size_bytes = bytes;    


    // Mark first first block used, for VCB
    GetNFreeBlocks(1);

    // Mark bitmap blocks used
    GetNFreeBlocks(blocks);

	//step d of free space
    printf("Writing %d blocks for bitmap\n", blocks);
	LBAwrite(bitmap, blocks, 1);

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
    LBAwrite(bitmap, vcb->bitmap_size_blocks, vcb->bitmap_starting_index);
	free(vcb);
	free(bitmap);
    free(current_working_dir);
	printf ("System exiting\n");
	}

void printVCB(){
	printf("Size of a Block: %ld\nNumber of Blocks: %ld\nBlocks Available: %ld\nSignature: %ld\n", vcb->size_of_block, vcb->number_of_blocks, vcb->blocks_available, vcb->signature);
}