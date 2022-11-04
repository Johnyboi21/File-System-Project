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


char* int_to_char(int input){
    int hundreds = input / 100;
    input = input - 100*hundreds;
    int tens = input / 10;
    input = input - 10*tens;
    
    char* number = malloc(256);
    
    int index = 0;
    if(hundreds > 0){
        number[index] = hundreds+48;
        index++;
    }    
    if(tens > 0){
        number[index] = tens+48;
        index++;
    }   
    if(input > 0){
        number[index] = input+48;
        index++;
    }   
    
    number[index] = 0;
    
    
    
    return number;
    
}

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
 //   printf("Total bytes is %d\n", total_bytes);
    int blocks = total_bytes/512;     // Number of blocks to occupy
    // Catch edge case to prevent too few blocks
    if(total_bytes % 512 != 0){
        blocks = blocks+1;
    }

    DE* directory = malloc(512*blocks);
    // Get a pointer to free blocks
    int free_blocks = GetNFreeBlocks(blocks);
    LBAread(directory, blocks, free_blocks);
    if(directory == NULL){
        printf("Failed to allocate memory for directory init\n");
        return -1;
    }

    // Set all entries to empty state
    for(int i=0; i < MAX_DIRENTRIES; i++){
        char* name = "\0";
       // directory[i].name[0] = 0;
       // printf("%d\n", i);
   
        strncpy(directory[i].name, name, 256);
        directory[i].is_directory = 0;
    }

//printf("a\n");

    
    if(free_blocks == 0){
        printf("Failed to find free blocks.\n");
    }
    // Set name and location of DEs

  //  printf("free_blocks[0] is %d\n", free_blocks);
    
    char* name = ".\0";
    char* name2 = "..\0";
    strncpy(directory[0].name, name, 256);
    directory[0].location = free_blocks;

    strncpy(directory[1].name, name2, 256);
    
    directory[0].size = blocks;
    directory[1].size = blocks;
    directory[0].is_directory = 1;
    directory[1].is_directory = 1;
    // Handle if root or not
    if(is_root == 1){
        directory[1].location = free_blocks;
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

    
   // for(int i=0; i < MAX_DIRENTRIES; i++){
     //   free(directory[i]);
   // }


    // Set location of new directory in parent
    // TODO: Handle for no free entries
    if(is_root == 0){
       // printf("Naming new file\n");
        int loc = findEmptyEntry(parent);       // Find index of first free entry
        if(loc != -1){
            
        
         //   printf("Dir being put at %d\n", loc); 
            parent[loc].location = free_blocks;     // Set location of DE to new Dir

            // May take out naming and have something else deal with it
            char* new = int_to_char(loc);
            strncpy(parent[loc].name, new, 256);    // Set new name of dir
            parent[loc].size = blocks;
            LBAwrite(parent, 30, parent[0].location);   // Write changes to parent
            free(new);
            // TODO: This seems bad
                // We need to read and write parent whenever we make changes to one of the 50 files? 1.5 kb?
        }

    }


    //void* realbuff = malloc(512 * blocks);
    //memcpy(realbuff, directory, );
    uint64_t write = LBAwrite(directory, blocks, free_blocks);
    vcb->blocks_available = vcb->blocks_available-blocks;
    MarkBlocksUsed(free_blocks, blocks);

    LBAread(directory, directory[0].size, directory[0].location); // Refresh parent after changes

    free(directory);

    return free_blocks;
}


/*
    Iterates over given directory pointer
    Searches for given file_name. 
    Returns index of found DE, or -1 if not found

*/

int findFileInDirectory(DE* dir, char* file_name){
    int i = 0;
    int not_found = 1;

    // We can start at 2 because 0 and 1 are known
    for(i=2; i < MAX_DIRENTRIES; i++){
        if(strcmp(dir[i].name, file_name) == 0){ // Free directory has null name
            not_found = 0;
          //  printf("Found requested at %d\n", i);
            break;
        }
        else{
            //printf("Dir name was %s\n", dir[i].name);
        }


    }

    if(not_found == 1){
        printf("Couldn't find requested directory\n");
        i = -1;
    }

    return i;
}

/*
    Uses findFileInDirectory
    Returns index of empty DE if found, else -1

*/
int findEmptyEntry(DE* dir){
    return findFileInDirectory(dir, "\0");
}


/*
    Adds several directories and files to the system
*/
void initTestDirs(){
    DE* dir = malloc(vcb->size_of_block * vcb->root_size);
    LBAread(dir, vcb->root_size, vcb->root_starting_index);

    printf("\nGoing to create 3 new directories in root\n");
    DirectoryInit(dir);
    DirectoryInit(dir);
    DirectoryInit(dir);

    printFilesInDir(dir);

    printf("Changing to dir named %s\n", dir[3].name);
    printf("%ld\n", LBAread(dir, dir[3].size, dir[3].location));
    printf("Now using dir at %ld\n", dir[0].location);


    printFilesInDir(dir);

    printf("Going to create 5 new directories in here\n");
    DirectoryInit(dir);
    DirectoryInit(dir);
    DirectoryInit(dir);
    DirectoryInit(dir);
    DirectoryInit(dir);

    printFilesInDir(dir);

    printf("\nChanging to dir named %s\n", dir[4].name);
    LBAread(dir, dir[4].size, dir[4].location);

    printf("Going to create 1 new directories in here\n");
    DirectoryInit(dir);
    printFilesInDir(dir);

    printf("Adding a file to dir\n");
    createFileInDir(dir);
    printFilesInDir(dir);



}


int createFileInDir(DE* dir){
    int ret = 1;
    
    int index = findEmptyEntry(dir);
    if (index == -1){
        printf("Failed to find space for new file\n");
        return -1;
    }

    int size = 256;
    char* new_file = malloc(size);
    dir[index].size = size;
    dir[index].is_directory = 0;
    char* new = int_to_char(index);
    strncpy(dir[index].name, new, 256);    // Set new name of file
    LBAwrite(dir, dir[0].size, dir[0].location);

    LBAread(dir, dir[0].size, dir[0].location); // Read after changes

    free(new);
    free(new_file);
    return ret;
}

void printFilesInDir(DE* dir){
    int skipped = 0;
    for(int i=0; i < MAX_DIRENTRIES; i++){
        if(strcmp(dir[i].name, "\0") == 0){
            skipped++;
        }
        else
            printf("Entry %d is:     %s\n", i-skipped, dir[i].name);

    }
}

/*
    Prints names of all files in given directory,
    including empty dirs
*/
void printFilesInDirWithEmpty(DE* dir){
    for(int i=0; i < MAX_DIRENTRIES; i++){
        printf("Entry %d is:     %s\n", i, dir[i].name);

    }
}
