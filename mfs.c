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
    if(pathname[0] == '\0'){
        printf("Empty path\n");
    }

    //here for identification of absolute or relative
    if(pathname[0] == '/'){
        printf("Start at root\n");
    }

    /*
    after our initial / root recognition.
    copy the rest of our pathname array "string"
    to our variable a.
    */

    char* a = malloc(strlen(pathname));
        strncpy(a, pathname, strlen(pathname));

    char delim[] = "/";
    parseData data;

    /*
    this would get the first root directory name after its /

    Example:
    A/B/C/D
    this current element would return A, once that strtok recongizes tha delim "/"
    */
    char* current_element = strtok(a, delim);
    data.directoryIndex++;

    printf("Active Directory Index Total: %d\n", data.directoryIndex);
    /*
    this gets the rest of the characters in the given string
    referencing the prior string.

    Example:
    our prior string was "A", so it will take that reference and cotinue
    the next string which would be B after that delim is found.
    */
    //char* next_element = strtok(NULL, delim);

    while(current_element != NULL){
        printf("%s\n", current_element);
        current_element = strtok(NULL,delim);

        data.directoryIndex++;
        if(current_element == NULL){
            data.directoryIndex--; // (n-1) we want active valid entries index
            printf("Active Directory Index Total: %d\n", data.directoryIndex);
        }
    }
   // token = strtok(NULL, "/");
     
    //directory pointer return to n -1
    data.dirPointer = current_element[data.directoryIndex];

    if(data.directoryIndex != NULL){
        printf("Last element doesn't exist.");
        return -1;
    }
    else
        printf("We have %d directory entries", data.directoryIndex);

    /*
    Why do we want to know this?
    If we want to create a new directory E, we need a previous directory 
    to locate and create e within.
    */

    free(a);
};




