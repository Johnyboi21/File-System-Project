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
#include <stdlib.h>
#include <stdio.h>


#include "mfs.h"
#include "b_io.h"
#include "directory_entry.h"
#include "fsLow.h"
#include "free_space_helpers.h"
#include "constants.h"


/*
    Creates a directory with the name of the last toke in pathname
    Ignores the mode, but could be used to set permissions

    Returns 0 on success, -1 on failure
*/
int fs_mkdir(const char *pathname, mode_t mode){
    parseData* data = parsePath(pathname);
    int ret_value = 0;

    if(data->testDirectoryStatus == 0 && data->dirPointer != NULL){
        DE* parent = malloc(vcb->size_of_block * data->dirPointer->d_reclen);

        LBAread(parent, data->dirPointer->d_reclen, data->dirPointer->directoryStartLocation);

        // Create new directory in parent

        new_dir_data* d = DirectoryInit(parent);
        printf("Going to put the new file in index %d of the dir starting at \n", d->index, parent->location);
        // Rename new file to given name
        strncpy(parent[d->index].name, data->nameOfLastToken, 256);
        d = NULL;
        free(d);
        // Save changes
        LBAwrite(parent, parent[0].size, parent[0].location);
        printf("\nWrote changes\n");
        printFilesInDir(parent);

        parent = NULL;
        free(parent);
    }
    else if(data->testDirectoryStatus == 1 || data->testDirectoryStatus == 2){
            printf("Error: File already exists\n");
            ret_value = -1;

        }
    else {
        printf("Error: Invalid path\n");
        ret_value = -1;
    }

    data = NULL;
    free(data);
    return(ret_value);
    
};

int fs_rmdir(const char *pathname){
    DE* dir = malloc(vcb->root_size*vcb->size_of_block);
    int is_success = 1;
    
    if(dir == NULL){
        printf("rmdir failed to allocate memory.\n");
        is_success = 0;
    }

    parseData* dir_info = parsePath(pathname);
    // TODO check NULL pointer on these, and do it before malloc in case we need to exit

    if(dir_info->testDirectoryStatus != 1){
        printf("Path was not a directoyr\n");
        is_success = 0;
    }


    if(is_success == 1){
        LBAread(dir, dir_info->dirPointer->d_reclen, dir_info->dirPointer->directoryStartLocation);
        int files = numberFilesInDir(dir);

        if(files == 2){
            
            // Move to parent 
            LBAread(dir, dir[1].size, dir[1].location);
            printf("Dir was empty. Removing dir named %s\n", dir[dir_info->directoryElement].name);

            char* name = "\0";
            strncpy(dir[dir_info->directoryElement].name, name, 256);


            //TODO: Make a function for marking blocks free and do that here
            MarkBlocksFree(dir[dir_info->directoryElement].location, dir[dir_info->directoryElement].size);

            LBAwrite(dir, dir[0].size, dir[0].location);      
  
        }
        else{
            printf("Could not remove directory: Directory was not empty.\n");
            is_success = 0;
        }

    }


    dir_info = NULL;
    dir = NULL;
    
    free(dir_info);
    free(dir);

    return is_success;
};

// Directory iteration functions
fdDir * fs_opendir(const char *pathname){

    /*
    fdDir* fd = malloc(sizeof(fdDir));
    parseData* dir_info = parsePath(pathname);

    fd->d_reclen = dir_info->dirPointer[0].size;
    fd->directoryStartLocation = dir_info->dirPointer[0].location;
    fd->dirEntryPosition = 0;

    return fd;

    */
};

struct fs_diriteminfo *fs_readdir(fdDir *dirp){

};

int fs_closedir(fdDir *dirp){
  //  free(dirp);


    //return 1;
};

// Misc directory functions

/*
    char* pathname is char buffer to copy to
    size_t size is size of buffer
*/
char * fs_getcwd(char *pathname, size_t size){
    strncpy(pathname, current_working_dir, size);

    return pathname;
};

int fs_setcwd(char *pathname){       //linux chdir
    parseData* data = parsePath(pathname);
    int ret_val = 0;

    if(data->testDirectoryStatus == 2){
        printf("Error: Not a directory\n");
        ret_val = -1;
    }
    else if(data->testDirectoryStatus == 0){
        printf("Error: Invalid path\n");
        ret_val = -1;
    }
    else{
        current_working_dir = pathname; // TODO: Find out if cwd should be malloc to max path size
        printf("Changed cwd to %s\n", current_working_dir);

    }


    free(data);
    return ret_val;
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
    printf("\n\nAttempting to parse %s\n", pathname);
    //30 Blocks i.e 30 * 512 (1 block is 512 bytes)
    DE* dirBuffer = malloc(15360);
    char delim[] = "/";
    parseData* data = malloc(sizeof(parseData));
    data->directoryElement = -1;
    data->dirPointer = NULL;
    data->testDirectoryStatus = 0;
    


    if(pathname[0] == '\0'){
        printf("Empty path\n");
    }

    //here for identification of absolute or relative
    if(pathname[0] == '/'){
        printf("Start at root\n");
        LBAread(dirBuffer, vcb->root_size, vcb->root_starting_index);
        printFilesInDir(dirBuffer);
    }

    /*
    after our initial / root recognition.
    copy the rest of our pathname array "string"
    to our variable a.
    */

    // +1 to catch null terminated
    char* a = malloc(strlen(pathname)+1);
    strncpy(a, pathname, strlen(pathname)+1);
    /*
    this would get the first root directory name after its /

    Example:
    A/B/C/D
    this current element would return A, once that strtok recongizes tha delim "/"
    */
    char* current_element = strtok(a, delim);
    printf("Current token is %s\n", current_element);
    
    /*
    this gets the rest of the characters in the given string
    referencing the prior string.

    Example:
    our prior string was "A", so it will take that reference and cotinue
    the next string which would be B after that delim is found.
    */
    char* next_element = strtok(NULL, delim);
    printf("Next token is %s\n", next_element);

    
    int indexOfSearch = -1;
    
    // Check first element in path
    if(current_element != NULL){
        printf("\nAbout to name it\n");
        strncpy(data->nameOfLastToken, current_element, 256);
        printf("Success\n\n");

        indexOfSearch = findFileInDirectory(dirBuffer, current_element);
       // printf("Found that element at %d. Is it dir? %ld\n", indexOfSearch, dirBuffer[indexOfSearch].is_directory);

        if(indexOfSearch != -1){
            // If it's a directory, we want to read into our dirBuffer
            if(dirBuffer[indexOfSearch].is_directory == 1){
                LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);
                printf("\n\nNew directory is: \n");
                printFilesInDir(dirBuffer);

                // If this is the last element of the path, we ended in a directory
                if(next_element == NULL){
                    printf("Last element is a directory\n");
                    data->testDirectoryStatus = 1;
                    data->directoryElement = indexOfSearch;

                }
            }
            else{
                // If it wasn't a directory and it's the last elem, then must be a file
                if(next_element == NULL){
                    printf("Last element is a file\n");

                    data->testDirectoryStatus = 2;
                    data->directoryElement = indexOfSearch;
                }
            }
        }
        else{
            printf("Found element in path that does not exist\n");

            // If we aren't at the last element when we find something that doesn't exist, 
                // path is invalid
            if(next_element == NULL){
                printf("Last element does not exist. If you're mkdir, this is good\n");
            }
            else{
                printf("Error: Path was invalid.\n");
                // Prevents us from giving dir info if path invalid
                dirBuffer = NULL; 
            }
        }
    }

    

    // Check the rest of the elements in the path, if they exist
        // Mostly duplicated from above, except while condition
    while(next_element != NULL && dirBuffer != NULL){
        printf("In the while\n");
        current_element = next_element;
        strncpy(data->nameOfLastToken, current_element, 256);

        printf("Current token is now %s\n", current_element);

        // Find index of this file
        indexOfSearch = findFileInDirectory(dirBuffer, current_element);
        printf("Found that element at %d\n", indexOfSearch);

        //printf("%s\n", next_element);
        next_element = strtok(NULL,delim);
        printf("Next token is %s\n", next_element);

        // If file exists
        if(indexOfSearch != -1){
            // If it's a directory, we want to read into our dirBuffer
            if(dirBuffer[indexOfSearch].is_directory == 1){
                LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);
                printf("\n\nNew directory is: \n");
                printFilesInDir(dirBuffer);

                // If this is the last element of the path, we ended in a directory
                if(next_element == NULL){
                    printf("Last element is a directory\n");
                    data->testDirectoryStatus = 1;
                    data->directoryElement = indexOfSearch;

                }
            }
            else{
                // If it wasn't a directory and it's the last elem, then must be a file
                if(next_element == NULL){
                    printf("Last element is a file\n");

                    data->testDirectoryStatus = 2;
                    data->directoryElement = indexOfSearch;
                }
            }
        }
        else{
            printf("Found element in path that does not exist\n");

            // If we aren't at the last element when we find something that doesn't exist, 
                // path is invalid
            if(next_element == NULL){
                printf("Last element does not exist. If you're mkdir, this is good\n");

            }
            else{
                printf("Error: Path was invalid.\n");
                // Prevents us from giving dir info if path invalid
                dirBuffer = NULL; 
            }
            break;
        }
        
    }
   
    if(dirBuffer != NULL){
        fdDir* fd = malloc(sizeof(fdDir));
        fd->directoryStartLocation = dirBuffer->location;
        fd->d_reclen = dirBuffer->size;
        data->dirPointer = fd;
    }
    else{
        data->dirPointer = NULL;

    }
    data->directoryElement = indexOfSearch;

    printf("Last token read was %s\n", data->nameOfLastToken);
    /*
    Why do we want to know this?
    If we want to create a new directory E, we need a previous directory 
    to locate and create e within.
    */
    free(a);
    free(dirBuffer);


    return data;
}




