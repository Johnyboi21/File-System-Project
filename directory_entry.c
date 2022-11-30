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
* Description: Implements functions from associated header.
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
#include "mfs.h" // Can remove this one later, just for test dir
#include "constants.h"


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

    Returns: struct representing starting block number of directory
        and index within parent of new directory
*/

struct new_dir_data* DirectoryInit(DE* parent){
    printf("In directory init\n");

    new_dir_data* ret_data = malloc(sizeof(new_dir_data));

    if(ret_data == NULL){
        printf("\nFailed to allocate memory.\n");

        return NULL;
    }

    int is_root = 0;     // Acts differently if root init
    if(parent == NULL){
        is_root = 1;
        
    }
    else{
        
        ret_data->newDir = NULL;
    }

    

    int total_bytes = MAX_DIRENTRIES * sizeof(DE);
 //   printf("Total bytes is %d\n", total_bytes);
    int blocks = total_bytes/vcb->size_of_block;     // Number of blocks to occupy
    // Catch edge case to prevent too few blocks
    if(total_bytes % vcb->size_of_block != 0){
        blocks = blocks+1;
    }
    printf("Malloc %d blocks\n", blocks);
    printf("That's %ld bytes\n", vcb->size_of_block*blocks);

    DE* directory = malloc(vcb->size_of_block*blocks);
            printf("Malloc success\n");

    if(directory == NULL){
        printf("Failed to allocate memory for directory init\n");

        free(ret_data);
        ret_data = NULL;

        return NULL;
    }

    // Get a pointer to free blocks
        printf("About to read directory\n");

    int free_blocks = GetNFreeBlocks(blocks);
    LBAread(directory, blocks, free_blocks);

    directory[0].size = blocks;
    directory[1].size = blocks;
    directory[0].size_bytes = total_bytes;
    directory[1].size_bytes = total_bytes;
    // Set all entries to empty state
    for(int i=0; i < ((directory->size*vcb->size_of_block)/(sizeof(DE))); i++){
        char* name = "\0";
       // directory[i].name[0] = 0;
       // printf("%d\n", i);
   
        strncpy(directory[i].name, name, MAX_DE_NAME);
        directory[i].is_directory = 1;
    }

//printf("a\n");

    
    if(free_blocks == 0){
        printf("Failed to find free blocks.\n");
    }
    // Set name and location of DEs

  //  printf("free_blocks[0] is %d\n", free_blocks);
    
    char* name = ".\0";
    char* name2 = "..\0";
    strncpy(directory[0].name, name, MAX_DE_NAME);
    directory[0].location = free_blocks;

    strncpy(directory[1].name, name2, MAX_DE_NAME);
    
    
    directory[0].is_directory = 1;
    directory[1].is_directory = 1;
    // Handle if root or not
    if(is_root == 1){
        directory[1].location = free_blocks;
    }
    else{
        directory[1].location = parent->location;
        directory[1].size = parent->size;
        directory[1].size_bytes = parent->size_bytes;

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

        while(loc == -1){
            printf("Not enough dir entries to create directory. Resizing. Current size in blocks is %ld\n", parent[0].size);
            parent = resize(parent); 
            printf("Size is now %ld\n", parent[0].size);
            directory[1].location = parent->location;
            directory[1].size = parent->size;
            ret_data->newDir = parent;
            loc = findEmptyEntry(parent); 
            break;
        }

        
        if(loc != -1){
            
        
         //   printf("Dir being put at %d\n", loc); 
            parent[loc].location = free_blocks;     // Set location of DE to new Dir

            // May take out naming and have something else deal with it
            char* new = int_to_char(loc);
            strncpy(parent[loc].name, new, MAX_DE_NAME);    // Set new name of dir
            parent[loc].size = blocks;


            printf("About to write %ld blocks to %ld\n", parent[0].size, parent[0].location);
            printf("Also this is index %d\n", loc);
            LBAwrite(parent, parent[0].size, parent[0].location);   // Write changes to parent
            printf("LBAwrite happened\n");
            

            free(new);
            // TODO: This seems bad
                // We need to read and write parent whenever we make changes to one of the 50 files? 1.5 kb?

            ret_data->index = loc;

        }
        else{
            printf("There weren't enough free entries.\n");
            
        }

    }


    //void* realbuff = malloc(512 * blocks);
    //memcpy(realbuff, directory, );

    printf("Writing %d blocks to location %d\n", blocks, free_blocks);
    uint64_t write = LBAwrite(directory, blocks, free_blocks);
    printf("FInished that write\n");
    LBAread(directory, directory[0].size, directory[0].location); // Refresh parent after changes

    free(directory);
    directory = NULL;


    ret_data->location = free_blocks;
    printf("ret_data tells us that the new dir is at %d\n", free_blocks);
   // printf("And one more time, parent is %ld blocks and is at %ld\n", parent->size, parent->location);
    return ret_data;
}




DE* resize(DE* dir){
    return addNBlocksToDE(dir, dir->size);
}

DE* addNBlocksToDE(DE* dir, int extraSize){

    // Blocks in new dir
    int newDirSize = dir->size + extraSize;
    int numEntries = ((newDirSize*vcb->size_of_block)/(sizeof(DE)));
    int free_blocks = GetNFreeBlocks(newDirSize);
    if(free_blocks == -1){
        printf("Error: Not enough free space for resize.\n");
        return NULL;
    }

    int old_location = dir->location;

    // Create new directory with new size
    DE* newDir = malloc(vcb->size_of_block * newDirSize);
    // Read location of newDIr
    LBAread(newDir, newDirSize, free_blocks);


    // Initialize data in newDir if is directory
    if(dir->is_directory == 1){
        for(int i=0; i < numEntries; i++){
            char* name = "\0";

            strncpy(newDir[i].name, name, MAX_DE_NAME);
            newDir[i].is_directory = 1;

        }
    }

    
    memcpy(newDir, dir, dir->size * vcb->size_of_block);

    // Free up space of old dir
    MarkBlocksFree(dir->location, dir->size);

    newDir->location = free_blocks;
    newDir->size = newDirSize;
    if(newDir->is_directory == 1){
        newDir->size_bytes = sizeof(DE) * numEntries;

    }
    else{
        newDir->size_bytes = dir->size_bytes;
    }

    // Update VCB info on root if needed
    if(dir->location == vcb->root_starting_index){
        printf("We're resizing root\n");
        vcb->root_starting_index = newDir->location;
        vcb->root_size = newDir->size;
    }

    
    time_t t = time(0);
    newDir->last_modified = t;

    

    
    // Buffer to help in grabbing info from other DEs
    DE* buf;

    // Update all children with new location and size of directory
        // Non directories don't need to do this
    
    if(dir->is_directory == 1){
        // Blocks to hold first two DEs
        int blocksNeeded = (sizeof(DE)*2)/vcb->size_of_block;
        if (sizeof(DE)*2 % vcb->size_of_block != 0){
            blocksNeeded++;
        }
        for(int i = 2; i < numEntries; i++){
            buf = malloc(2 * vcb->size_of_block);
            LBAread(buf, blocksNeeded, newDir[i].location);

            printf("That updated %d, named %s\n", i, buf[1].name);
            printf("Location and size were %ld and %ld, now ", buf[1].location, buf[1].size);
            buf[1].location = newDir->location;
            buf[1].size = newDir->size;
            buf[1].last_modified = newDir->last_modified;
            buf[1].size_bytes = newDir->size_bytes;
            printf("%ld and %ld\n", buf[1].location, buf[1].size);
            

            LBAwrite(buf, blocksNeeded, buf->location);

            

            free(buf);
            buf = NULL;
        }
    }


    // If we resized root, then root's [1] entry also needs to know the new info
    if(old_location == newDir[1].location){
        
        printf("Root stuff. Location check is %ld\n", newDir[1].location);
        

        newDir[1].location = newDir->location;
        newDir[1].size = newDir->size;
        newDir[1].last_modified = newDir->last_modified;

    }

   

    // Read the parent of the newDir
    buf = malloc(newDir[1].size * vcb->size_of_block);
    LBAread(buf, newDir[1].size, newDir[1].location);

    // Find the directory in the parent to update the parent's entry
    for(int i = 2; i < 1; i++){

        if(buf[i].location == old_location){
            buf[i].location = newDir->location;
            
            buf[i].size = newDir->size;
            buf[i].last_modified = newDir->last_modified;

            LBAwrite(buf, buf->size, buf->location);
            break;
        }
    }
    

    free(buf);
    buf = NULL;

    printf("Resized directory to %ld, moving it to %ld\n", newDir->size, newDir->location);


    free(dir);
    dir = newDir;

        printf("Proofs %ld, %ld\n", dir->size, dir->location);

    return dir;
}

/*
    Iterates over given directory pointer
    Searches for given file_name. 
    Returns index of found DE, or -1 if not found

*/

int findFileInDirectory(DE* dir, char* file_name){
    int i = 0;
    int not_found = 1;

    for(i=0; i < ((dir->size*vcb->size_of_block)/(sizeof(DE))); i++){
      //  printf("Find file %d\n", i);
        if(strcmp(dir[i].name, file_name) == 0){ // Free directory has null name
            not_found = 0;
            //printf("Found requested at %d. Is dir? %d\n", i, dir[i].is_directory);
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
    printf("\nRemoving 2\n");
    fs_rmdir("/2");
    
    LBAread(dir, vcb->root_size, vcb->root_starting_index);
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



    int blocks = sizeof(DE)*2 / vcb->size_of_block;

    if((sizeof(DE) * 2) % vcb->size_of_block != 0){
        blocks++;
    }


    int free_blocks = GetNFreeBlocks(blocks);
    printf("Created file at %d with size %d\n", free_blocks, blocks);

    time_t create_time = time(0);

    // Init with space for . and ..
    DE* new_file = malloc(blocks * vcb->size_of_block);
    LBAread(new_file, blocks, free_blocks);

    new_file->size_bytes = 0;
    new_file->size = blocks;
    new_file->is_directory = 0;
    new_file->location = free_blocks;

    new_file->creation_date = create_time;
    new_file->last_modified = create_time;
    strncpy(new_file->name, ".\0", MAX_DE_NAME);


    strncpy(new_file[1].name, "..\0", MAX_DE_NAME);
    new_file[1].location = dir->location;
    new_file[1].is_directory = 1;
    new_file[1].size = dir->size;
    new_file[1].size_bytes = dir->size_bytes;
    new_file[1].creation_date = dir->creation_date;
    new_file[1].last_modified = dir->last_modified;

    dir[index].size = new_file->size;
    dir[index].is_directory = 0;
    dir[index].location = free_blocks;
    dir[index].last_modified = new_file->last_modified;
    dir[index].creation_date = new_file->creation_date;
    dir[index].size_bytes = new_file->size_bytes;


    // Set new name of file
    char * name = "TestFile\0";
    strncpy(dir[index].name, name, MAX_DE_NAME);




    char* file_content = malloc(50);

    char* text = "Here is a test message for files of 50 characters\0";

    

    strncpy(file_content, text, 50);


    char* n = (char*)new_file;
    memcpy(n+(sizeof(DE)*2), file_content, 50);

    

    new_file->size_bytes+=50;

    LBAwrite(dir, dir->size, dir->location);

    LBAwrite(new_file, blocks, free_blocks);


    char* read = malloc(new_file->size_bytes);

    strncpy(read, n+sizeof(DE)*2, new_file->size_bytes);

    free(file_content);
    free(new_file);
    free(read);
    return ret;
}





void printFilesInDir(DE* dir){
    int skipped = 0;
    for(int i=0; i < ((dir->size*vcb->size_of_block)/(sizeof(DE))); i++){
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
    for(int i=0; i < ((dir->size*vcb->size_of_block)/(sizeof(DE))); i++){
        printf("Entry %d is:     %s\n", i, dir[i].name);

    }
}


/*
    Returns number of files in given dir pointer
    Returns 2 on empty directory (contains . and .. only)
*/
int numberFilesInDir(DE* dir){
    int files = 0;

    for(int i=0; i < ((dir->size*vcb->size_of_block)/(sizeof(DE))); i++){
        if(strcmp(dir[i].name, "\0") != 0){
            files++;
        }
    }

    return files;
}