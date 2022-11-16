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
    //    printf("Going to put the new file in index %d of the dir starting at \n", d->index, parent->location);
        // Rename new file to given name
        strncpy(parent[d->index].name, data->nameOfLastToken, 256);
        
        free(d);
        d = NULL;
        // Save changes
        LBAwrite(parent, parent[0].size, parent[0].location);
        printf("\nWrote changes\n");
      //  printFilesInDir(parent);

        free(parent);
        parent = NULL;
    }
    else if(data->testDirectoryStatus == 1 || data->testDirectoryStatus == 2){
            printf("Error: File already exists\n");
            ret_value = -1;

        }
    else {
        printf("Error: Invalid path\n");
        ret_value = -1;
    }


    free(data);
    data = NULL;
    
    return(ret_value);
    
};

int fs_rmdir(const char *pathname){
    DE* dir = malloc(vcb->root_size*vcb->size_of_block);
    int is_success = 1;

    printf("********Trying to remove: %s\n", pathname);
    
    if(dir == NULL){
        printf("rmdir failed to allocate memory.\n");
        is_success = 0;
    }

    parseData* dir_info = parsePath(pathname);
    // TODO check NULL pointer on these, and do it before malloc in case we need to exit

    printf("Test status is %d\n", dir_info->testDirectoryStatus);
    if(dir_info->testDirectoryStatus != 1){
        printf("Path was not a directory\n");
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


            MarkBlocksFree(dir[dir_info->directoryElement].location, dir[dir_info->directoryElement].size);

            LBAwrite(dir, dir[0].size, dir[0].location);      
  
        }
        else{
            printf("Could not remove directory: Directory was not empty.\n");
            is_success = 0;
        }

    }


    
    
    free(dir_info);
    free(dir);

    dir_info = NULL;
    dir = NULL;

    return is_success;
};

// Directory iteration functions
fdDir * fs_opendir(const char *pathname){
    char* path = malloc(MAX_PATH_LENGTH);
    strncpy(path, pathname, MAX_PATH_LENGTH);

    //Parse data and get information
    if(fs_isDir(path) == 0){ printf("\nfs_opendir ERROR: Pathname is not a directory\n"); return NULL;}

    printf("About to parse in open\n");
    parseData* parsed_data = parsePath(path);
    printf("Finished parse in open\n");
    unsigned short d_reclen = parsed_data->dirPointer->d_reclen;
    unsigned short dirEntryPosition = parsed_data->dirPointer->dirEntryPosition;
    uint64_t directoryStartLocation = parsed_data->dirPointer->directoryStartLocation;

    DE* directoryPtr = malloc(d_reclen*vcb->size_of_block);
    LBAread(directoryPtr, d_reclen, directoryStartLocation);


    // Make a copy of fdDir in parsed_data
    fdDir* fdPtr = malloc(sizeof(fdDir));
    fdPtr->directoryStartLocation = directoryPtr->location;
    fdPtr->d_reclen = directoryPtr->size;
    fdPtr->dirEntryPosition = 0;
    
    //free stuff
    free(parsed_data);
    free(directoryPtr);
    free(path);

    /**
     * REMEMBER TOO FREE 
     *  directoryPtr
     *  fdPtr
    */

    printf("FInished in open\n");
    return fdPtr;
};

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
   // printf("In readdir\n");
    struct fs_diriteminfo* retPtr = NULL;
    //********** Figure out what data this dirp has
   // printf("Dirp has reclen %d, starts at %ld\n", dirp->d_reclen, dirp->directoryStartLocation);
    DE* directory = malloc(dirp->d_reclen * vcb->size_of_block);
    LBAread(directory, dirp->d_reclen, dirp->directoryStartLocation);
    dirp->ii = malloc(sizeof(struct fs_diriteminfo));
    for(int i = dirp->dirEntryPosition; i < MAX_DIRENTRIES; i++){
   //     printf("Before compare, name is %ld\n", strlen(directory[i].name));
        if(strcmp(directory[i].name, "\0") != 0){
            strncpy(dirp->ii->d_name, directory[i].name, MAX_DE_NAME);
            dirp->ii->fileType = directory[i].is_directory; //have something to tell you file type
            dirp->dirEntryPosition = i + 1;
            retPtr = dirp->ii;
            break;
        }
    }


    free(directory);

   // printf("Finished readdir\n");
    return retPtr;
};

int fs_closedir(fdDir *dirp){
    free(dirp);


    return 1;
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


/*
    Uses strtok to count num tokens in path
    Returns number of tokens
*/

int countPathTokens(char* pathname){
    int numTokens = 0;

    char* pathCopy = malloc(MAX_PATH_LENGTH);
    strncpy(pathCopy, pathname, MAX_PATH_LENGTH);


    char* token = strtok(pathCopy, "/");

    while(token != NULL){
        numTokens++;

        token = strtok(NULL, "/");
    }

    
    free(pathCopy);
    pathCopy = NULL;

    return numTokens;
}

/*
    Takes valid path and formats to be in line with what we expect
    Ex. Remove extra '/', resolve and remove '.' and '..' 

    Returns buffer containing formatted path
*/

char* formatPath(char *pathname){
  //  printf("Formatting %s\n", pathname);
    int numTokens = countPathTokens(pathname);
   // printf("It has %d tokens\n", numTokens);

    /* 
        Array of char to track which tokens should be in final path
        1 for add token, 0 for ignore

        ith value set to 0 if '.' is found
        (i-1)th value set to 0 if ".." is found
            This can cascade with multiple ".." in a row. 
    */
    char* tokenTracker = malloc(sizeof(char*)* (numTokens));
    for(int i=0; i < numTokens; i++){
        tokenTracker[i] = 1;
    }

    // Current position in tokenTracker
    int tokenIter = 0;


    // Return path to be manipulated
        // In the end, will represent an absolute path
    char* newPath = malloc(MAX_PATH_LENGTH);
    newPath[0] = '/';
    newPath[1] = '\0';
    int pathOffset = 1; // Offset from start of newPath

    // Copy of path to gather tokens from
    char* pathCopy = malloc(MAX_PATH_LENGTH);
    strncpy(pathCopy, pathname, MAX_PATH_LENGTH);
    
    if(pathname[0] != '/'){
        printf("Formatting relative\n");
    }

    char* currentToken = strtok(pathCopy, "/");
    char* nextToken;


    /*

        /A/B/C/../C/../../B/E/F/../G
        Should be /A/B/E/G
        So final is 
        100000011001
        111111111111
        
        Token iter is 3
        111 See .., set 1100 (now have 1 previousDir, set 3 and 3-1 to 0)
        Token iter is 5
        11001 see .., set 110000 (now 1, set 5 and 5-1 to 0)
        Token iter is 6
        110000 see.., set (now 2, set 6 and 6-2 to 0)
    */

    /*
        Resolves "." and ".." by marking associated tokens 
        as unneeded, so we don't add them to the path later

    */
    while(currentToken != NULL){
        if(strcmp(currentToken, ".\0") == 0){
            tokenTracker[tokenIter] = 0;

        }
        else if(strcmp(currentToken, "..\0") == 0){

            // Set tokenIter and last 1 to 0
            tokenTracker[tokenIter] = 0;
            for(int i=tokenIter-1; i >= 0; i--){
                if(tokenTracker[i] == 1){
                    tokenTracker[i] = 0;
                    break;
                }
            }
        }

        tokenIter++;
        currentToken = strtok(NULL, "/");

    }

   // printf("After that, state of things is ");
    for(int i =0; i < numTokens; i++){
        printf("%d", tokenTracker[i]);
    }

    printf("\n");

    // Reset strtok and iterator
    strncpy(pathCopy, pathname, MAX_PATH_LENGTH);
  //  printf("Pathcopy is %s right now\n", pathCopy);
    currentToken = strtok(pathCopy, "/");
    tokenIter = 0;
    /*
        Adds ith token to path if ith entry in tokenTracker == 1
    */
    while(currentToken != NULL){
     //   printf("Looking at string %s, %d\n", currentToken, tokenIter);
        if(tokenTracker[tokenIter] == 1){
        //    newPath[pathOffset] = '/';
          //  pathOffset++;
            strncpy(newPath+pathOffset, currentToken, MAX_PATH_LENGTH-pathOffset);
            // Note that token contains null char, which we copy, yet path offset increment 
                // does not include it. We'll overwrite that null char on each copy
            
            
            pathOffset+=strlen(currentToken);
            newPath[pathOffset] = '/';
            newPath[pathOffset+1] = '\0';
            pathOffset++;

//            printf("Added %dth entry, which was %s. Path is now %s\n", tokenIter, currentToken, newPath);
        }
        

        currentToken = strtok(NULL, "/");
        tokenIter++;

    }


  //  printf("Path %s was converted to %s\n", pathname, newPath);


    free(tokenTracker);
    free(pathCopy);
    tokenTracker = NULL;
    pathCopy = NULL;
   
    return newPath;
}


/*
    Uses given path name to adjust current_working_directory

    Returns 0 on success, else -1

*/
int fs_setcwd(char *pathname){       //linux chdir
    parseData* data = parsePath(pathname);
    int ret_val = 0;
    int is_relative = 0;

    printf("Coming into set cwd with %s\n", pathname);
    
    if(data->testDirectoryStatus == 2){
        printf("Error: Not a directory\n");
        ret_val = -1;
    }
    else if(data->testDirectoryStatus == 0){
        printf("Error: Invalid path\n");
        ret_val = -1;
    }
    else{
        
        // Holds full new cwd
        char* newPath;

        // Handle relative or absolute
        if(pathname[0] != '/'){
            printf("CD with relative\n");

            // Get CWD
            char* cwd = malloc(MAX_PATH_LENGTH);
            fs_getcwd(cwd, MAX_PATH_LENGTH);

            printf("CWD is %s\n", cwd);

            // Add cwd to path, then tack on the relative path after it
                // Redundancy for clarity. Imagine starting at 0 and rebuilding
            strncpy(current_working_dir, cwd, MAX_PATH_LENGTH);
            strncpy(current_working_dir+strlen(cwd), pathname, MAX_PATH_LENGTH-strlen(cwd));

            // Format the new full path to remove unnecessary tokens and chars
            newPath = formatPath(current_working_dir);

            // Overwrite with tweaked path
            strncpy(current_working_dir, newPath, MAX_PATH_LENGTH);
            
            free(cwd);
            cwd = NULL;
        }
        else{

            // Format the given path, then save
            newPath = formatPath(pathname);
            strncpy(current_working_dir, newPath, MAX_PATH_LENGTH);
        }


        
        free(newPath);
        newPath = NULL;
        
        printf("Changed cwd to %s\n", current_working_dir);

    }


    free(data->dirPointer);
    free(data);
    data = NULL;
    return ret_val;
}; 
 
int fs_isFile(char * filename){     //return 1 if file, 0 otherwise
    int isFileValue = 0;
    parseData *isFileData = parsePath(filename);

    if(isFileData->testDirectoryStatus == 2){
        printf("This is a file\n");
        isFileValue = 1;    //This is a file
        
    }
    else{
        printf("This is not a file\n");
        isFileValue = 0;    //Not a file
        
    }
    
    free(isFileData);   //free in relation to fact that parsePath malloc'd

    return isFileValue;
}
	
int fs_isDir(char * pathname){      //return 1 if directory, 0 otherwise
    int isDirRet = 0;
    parseData *isDirData = parsePath(pathname);

    if(isDirData->testDirectoryStatus == 1){
      //  printf("This is a directory\n");
        isDirRet = 1; //Is Directory
        
    }
    else{
        //printf("This is not a directory\n");
        isDirRet = 0;  //Is not a Directory
        
    }

    free(isDirData);   //free in relation to fact that parsePath malloc'd

    return isDirRet;
}
	

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



int fs_stat(const char* pathname, struct fs_stat* buf){
    char* path = malloc(MAX_PATH_LENGTH);
    strncpy(path, pathname, MAX_PATH_LENGTH);

    if(!fs_isDir(path)){ printf("\nfs_stat ERROR: Pathname Isn't a Directory"); return -1; }
    parseData* parsed_data = parsePath(path);

    DE* directory = malloc(parsed_data->dirPointer->d_reclen * vcb->size_of_block);
    LBAread(directory, parsed_data->dirPointer->d_reclen, parsed_data->dirPointer->directoryStartLocation);
    //int file_index = findFileInDirectory(directory, pathname);

    buf->st_blksize = vcb->size_of_block;
    buf->st_blocks = directory[parsed_data->directoryElement].size;
    buf->st_size = parsed_data->dirPointer->d_reclen * vcb->size_of_block;
    buf->st_createtime = directory[parsed_data->directoryElement].creation_date;
    buf->st_modtime = directory[parsed_data->directoryElement].last_modified;
    
     //if(file_index != -1){
        //fill in struct here
     //}  
    
    free(directory);
    free(path);

    return 1;
}




/*
abosolute -> want starting directory as our root.
want to read it or store root in global memory.

relative -> get current working directory.

*/
struct parseData *parsePath(const char *pathname){
   // printf("\n\nAttempting to parse %s\n", pathname);
    //30 Blocks i.e 30 * 512 (1 block is 512 bytes)
    DE* dirBuffer = malloc(MAX_DIRENTRIES * vcb->size_of_block);
    char delim[] = "/";
    parseData* data = malloc(sizeof(parseData));
    int isEmptyPath = 0;

    // Init data elements
    data->directoryElement = -1;
    data->dirPointer = NULL;
    data->testDirectoryStatus = 0;
    


    if(pathname[0] == '\0'){
        printf("Empty path\n");
        isEmptyPath = 1;
    }

    char* pathBuffer = malloc(MAX_PATH_LENGTH);
    
    //here for identification of absolute or relative
    if(pathname[0] == '/'){
     //   printf("Start at root\n");
        LBAread(dirBuffer, vcb->root_size, vcb->root_starting_index);
     //   printFilesInDir(dirBuffer);

        
    }
    else{
    //    printf("Apparently it's relative. The path is %s\n", pathname);
      //  printf("Relative path. CWD is %s\n", current_working_dir);
     //   printf("Going to go through with %s", fs_getcwd(pathBuffer, MAX_PATH_LENGTH));
        parseData* relativeData = parsePath(fs_getcwd(pathBuffer, MAX_PATH_LENGTH));
        
        LBAread(dirBuffer, relativeData->dirPointer->d_reclen, relativeData->dirPointer->directoryStartLocation);
        //printf("Finished reading relative.\n");
     //   printFilesInDir(dirBuffer);

        
        free(relativeData->dirPointer);
        free(relativeData);
        relativeData = NULL;
    }

    /*
    after our initial / root recognition.
    copy the rest of our pathname array "string"
    to our variable a.
    */

    strncpy(pathBuffer, pathname, MAX_PATH_LENGTH);
    /*
    this would get the first root directory name after its /

    Example:
    A/B/C/D
    this current element would return A, once that strtok recongizes tha delim "/"
    */
    char* current_element = strtok(pathBuffer, delim);
   // printf("Current token is %s\n", current_element);
    
    /*
    this gets the rest of the characters in the given string
    referencing the prior string.

    Example:
    our prior string was "A", so it will take that reference and cotinue
    the next string which would be B after that delim is found.
    */
    char* next_element = strtok(NULL, delim);
//    printf("Next token is %s\n", next_element);

    
    int indexOfSearch = -1;
    
    // Check first element in path
    if(current_element != NULL){
  //      printf("\nAbout to name it\n");
        strncpy(data->nameOfLastToken, current_element, 256);
  //      printf("Success\n\n");

        indexOfSearch = findFileInDirectory(dirBuffer, current_element);
       // printf("Found that element at %d. Is it dir? %ld\n", indexOfSearch, dirBuffer[indexOfSearch].is_directory);

        if(indexOfSearch != -1){
            // If it's a directory, we want to read into our dirBuffer
            if(dirBuffer[indexOfSearch].is_directory == 1){
                LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);
   //             printf("\n\nNew directory is: \n");
           //     printFilesInDir(dirBuffer);

                // If this is the last element of the path, we ended in a directory
                if(next_element == NULL){
   //                 printf("Last element is a directory\n");
                    data->testDirectoryStatus = 1;
                    data->directoryElement = indexOfSearch;

                }
            }
            else{
                // If it wasn't a directory and it's the last elem, then must be a file
                if(next_element == NULL){
   //                 printf("Last element is a file\n");

                    data->testDirectoryStatus = 2;
                    data->directoryElement = indexOfSearch;
                }
            }
        }
        else{
    //        printf("Found element in path that does not exist\n");

            // If we aren't at the last element when we find something that doesn't exist, 
                // path is invalid
            if(next_element == NULL){
   //             printf("Last element does not exist. If you're mkdir, this is good\n");
            }
            else{
                printf("Error: Path was invalid.\n");
                // Prevents us from giving dir info if path invalid
                dirBuffer = NULL; 
            }
        }
    }
    // If first elem. is NULL and path isn't empty, we have path "/"
    else if(isEmptyPath == 0){ 
        LBAread(dirBuffer, vcb->root_size, vcb->root_starting_index);
        data->testDirectoryStatus = 1;

    }

    

    // Check the rest of the elements in the path, if they exist
        // Mostly duplicated from above, except while condition
    while(next_element != NULL && dirBuffer != NULL){
  //      printf("In the while\n");
        current_element = next_element;
        strncpy(data->nameOfLastToken, current_element, 256);

//        printf("Current token is now %s\n", current_element);

        // Find index of this file
        indexOfSearch = findFileInDirectory(dirBuffer, current_element);
 //       printf("Found that element at %d\n", indexOfSearch);

        //printf("%s\n", next_element);
        next_element = strtok(NULL,delim);
   //     printf("Next token is %s\n", next_element);

        // If file exists
        if(indexOfSearch != -1){
            // If it's a directory, we want to read into our dirBuffer
            if(dirBuffer[indexOfSearch].is_directory == 1){
                LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);
 //               printf("\n\nNew directory is: \n");
            //    printFilesInDir(dirBuffer);

                // If this is the last element of the path, we ended in a directory
                if(next_element == NULL){
 //                   printf("Last element is a directory\n");
                    data->testDirectoryStatus = 1;
                    data->directoryElement = indexOfSearch;

                }
            }
            else{
                // If it wasn't a directory and it's the last elem, then must be a file
                if(next_element == NULL){
  //                  printf("Last element is a file\n");

                    data->testDirectoryStatus = 2;
                    data->directoryElement = indexOfSearch;
                }
            }
        }
        else{
  //          printf("Found element in path that does not exist\n");

            // If we aren't at the last element when we find something that doesn't exist, 
                // path is invalid
            if(next_element == NULL){
 //               printf("Last element does not exist. If you're mkdir, this is good\n");

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

  //  printf("Last token read was %s\n", data->nameOfLastToken);
    
    /*
    Why do we want to know this?
    If we want to create a new directory E, we need a previous directory 
    to locate and create e within.
    */
    free(pathBuffer);
    free(dirBuffer);


    return data;
}




