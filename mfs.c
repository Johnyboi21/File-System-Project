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
	
int fs_delete(char* filename){      //removes a file

};	




