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
* File: directory_entry.c
*
* Description: Implements functions from associated head.
    Contains functions used for directory management, 
    including init for directories
*
* 
*
**************************************************************/

#include "directory_entry.h"
#include "bit_math.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include "free_space_helpers.h"
#include <string.h>
#include "fsLow.h"

/*
    1. Allocate space on disk
    2. Create buffer in memory
    3. Init each directory entry in buffer (array), init to empty state
    4. Init "." DE to itself
    5. Init ".." DE to parent

    6, LBAWrite buffer, #blocks, block position

    If parent == null, doing root

    Returns: Int representing block number of directory
*/

int DirectoryInit(DE* parent){
    int is_root = 0;     // Acts differently if root init
    if(parent == NULL){
        is_root = 1;
    }


    int total_bytes = MAX_DIRENTRIES * sizeof(DE);
    int blocks = total_bytes/512;     // Number of blocks to occupy
    // Catch edge case to prevent too few blocks
    if(total_bytes % 512 != 0){
        blocks = blocks+1;
    }

    DE* directory = malloc(total_bytes);
    if(directory == NULL){
        printf("Failed to allocate memory for directory init\n");
        return -1;
    }

    // Set all entries to empty state
    for(int i=0; i < MAX_DIRENTRIES; i++){
        directory[i].name[0] = 0;
     //   strncpy(directory[i]->name, name, 256);
    }


    // Get a pointer to free blocks
    int* free_blocks = GetNFreeBlocks(blocks);
    if(free_blocks == NULL){

    }
    // Set name and location of DEs

    
    char* name = ".\0";
    char* name2 = "..\0";
    strncpy(directory[0].name, name, 256);
    directory[0].location = free_blocks[0];

    strncpy(directory[1].name, name2, 256);

    // Handle if root or not
    if(is_root == 1){
        directory[1].last_modified = free_blocks[0];

    }
    else{
        directory[1].location = parent->location;

    }

    // Set current time to creation date and last modify
    time_t create_time = time(0);
    directory[0].creation_date = create_time;
    directory[0].last_modified = create_time;

    directory[1].creation_date = create_time;
    directory[1].last_modified = create_time;

    LBAwrite(directory, blocks, free_blocks[0]);
    vcb->blocks_available = vcb->blocks_available-30;
    free(directory);
   // for(int i=0; i < MAX_DIRENTRIES; i++){
     //   free(directory[i]);
   // }
    return free_blocks[0];
}