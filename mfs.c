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
* File: mfs.c
*
* Description: To be updated... 
*
**************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include "mfs.h"
#include "b_io.h"
#include "directory_entry.h"
#include "fsLow.h"
#include "free_space_helpers.h"
#include "constants.h"


// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode){

};

int fs_rmdir(const char *pathname){

};

// Directory iteration functions
fdDir * fs_opendir(const char *pathname){

};

struct fs_diriteminfo *fs_readdir(fdDir *dirp){


};

int fs_closedir(fdDir *dirp){

};

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size){

};

int fs_setcwd(char *pathname){       //linux chdir

}; 
 
int fs_isFile(char * filename){     //return 1 if file, 0 otherwise
int retValue = 0;

if(parsePath(filename)->testDirectoryStatus == 2){
    retValue = 1;

    return retValue;
}
else{
    retValue = 0;
    return retValue;
}

};
	
int fs_isDir(char * pathname){      //return 1 if directory, 0 otherwise

};	
	

/*
Conditions:
    1.)A file has to exist.

Steps:
    1.)Find file that exists & want to delete
    2.)Mark the blocks that the file was using as free
    3.)Set file to null
    4.)Mark the directory entry as unused.



*/
int fs_delete(char* filename){      //removes a file


};	


/*
abosolute -> want starting directory as our root.
want to read it or store root in global memory.

relative -> get current working directory.

*/
struct parseData *parsePath(const char *pathname){
    //30 Blocks i.e 30 * 512 (1 block is 512 bytes)
    DE* dirBuffer = malloc(15360);
    char delim[] = "/";
    parseData data;

    int totalElements = 0;
    int finished = 0;


    if(pathname[0] == '\0'){
        printf("Empty path\n");
    }

    //here for identification of absolute or relative
    if(pathname[0] == '/'){
        printf("Start at root\n");
        LBAread(dirBuffer, vcb->root_size, vcb->root_starting_index);
    }

    /*
    after our initial / root recognition.
    copy the rest of our pathname array "string"
    to our variable a.
    */

    char* a = malloc(strlen(pathname));
        strncpy(a, pathname, strlen(pathname));
    /*
    this would get the first root directory name after its /

    Example:
    A/B/C/D
    this current element would return A, once that strtok recongizes tha delim "/"
    */
    char* current_element = strtok(a, delim);

    int indexOfSearch = findFileInDirectory(dirBuffer, current_element);

    if(indexOfSearch != -1 && dirBuffer[indexOfSearch].is_directory == 1){
        LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);
        data.testDirectoryStatus = 1; //Valid path
    }
    //Invalid directory entry.
    else{
       printf("Element doesn't exist\n");  
       data.testDirectoryStatus = 0; //Invlaid path
    }

    data.directoryElement++;

    printf("Active Directory Index Total: %d\n", data.directoryElement);
    char* next_element = strtok(NULL, delim);
    /*
    this gets the rest of the characters in the given string
    referencing the prior string.

    Example:
    our prior string was "A", so it will take that reference and cotinue
    the next string which would be B after that delim is found.
    */
    //char* next_element = strtok(NULL, delim);

    while(current_element != NULL){
        current_element = next_element;
        int indexOfSearch = findFileInDirectory(dirBuffer, current_element);

        //printf("%s\n", next_element);
        next_element = strtok(NULL,delim);

        if(indexOfSearch != -1 && dirBuffer[indexOfSearch].is_directory == 1){
        LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);
        data.testDirectoryStatus = 1; //Valid path
        }
        //Invalid directory entry.
        else{
             if(indexOfSearch != -1 && current_element == NULL && dirBuffer[indexOfSearch].is_directory == 0) {
                 data.directoryElement--; // (n-1) we want active valid entries index
                 printf("Active Directory Index Total: %d\n", data.directoryElement);
                 finished = 1;
                 data.testDirectoryStatus = 2; //Valid path pointing to file.
             }else{
                printf("Last element doesn't exist\n");
                data.testDirectoryStatus = 0; //Invalid path.
                break;
             }
        }
        
        data.directoryElement++;
        totalElements = data.directoryElement;

        if(current_element == NULL){
            data.directoryElement--; // (n-1) we want active valid entries index
            printf("Active Directory Index Total: %d\n", data.directoryElement);
            finished = 1;
        }
    }
   
   if(finished){
        fdDir* fd = malloc(sizeof(fdDir));
        fd->directoryStartLocation = dirBuffer->location;
        fd->d_reclen = dirBuffer->size;
        data.dirPointer = fd;
   }

    /*
    Why do we want to know this?
    If we want to create a new directory E, we need a previous directory 
    to locate and create e within.
    */
    free(a);
    free(dirBuffer);
};




