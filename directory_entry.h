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
* File: directory_entry.h
*
* Description: Contains definition of directory entry structure 
    and prototypes for functions related to manipulating the 
    directory
*
* 
*
**************************************************************/

#include <time.h>
#include <stdint.h>
//const int size_of_block = 512;
typedef struct DE{
	char name[256];             // Name of file
	uint64_t size;              // Size of DE in bytes
	uint64_t location;          // Location on disk
    uint64_t is_directory;
	time_t creation_date;       // Date created
	time_t last_modified;       // Recent modify date
} DE;

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

int DirectoryInit(DE* parent);


/*
    Iterates over given directory pointer
    Searches for given file_name. 
    Returns index of found DE, or -1 if not found

*/

int findFileInDirectory(DE* dir, char* file_name);


/*
    Uses findFileInDirectory
    Returns index of empty DE if found, else -1

*/

int findEmptyEntry(DE* dir);


/*
    Adds several directories and files to the system
*/
void initTestDirs();


/*
    Prints names of all files in given directory
*/
void printFilesInDir(DE* dir);

/*
    Prints names of all files in given directory
    including empty dirs
*/
void printFilesInDirWithEmpty(DE* dir);


/*
    Test function that creates blank file in dir
*/
int createFileInDir(DE* dir);