/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int individualFilePosition; //keep track where we are in a individual file.

	unsigned short d_reclen;    //length of this record 
    unsigned char fileType;    
    char d_name[256]; 
	int flagPassed;
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	//*** TODO ***:  Modify to save or set any information needed
	//
	//	
	if (startup == 0) b_init();  //Initialize our system

	
	b_io_fd returnFd = b_getFCB();				// get our own file descriptor
	fcbArray[returnFd].buf = malloc(sizeof(char)* MAXFCBS);	//allocate memory
	fcbArray[returnFd].flagPassed = flags;

	if(fcbArray[returnFd].buf == NULL){		//catch test
		prinft("Failed to allocate memory");
		return - 1;
	}

	parseData *fdData = parsePath(filename);	//obtain file

	//get file information.
	fcbArray[returnFd].d_reclen = fdData->dirPointer->ii->d_reclen;	
	fcbArray[returnFd].fileType = fdData->dirPointer->ii->fileType;
	fcbArray[returnFd].d_name == fdData->dirPointer->ii->d_name;

	//To-Do: Free mallocs.
	free(fdData->dirPointer);
	free(fdData);
	
	return (returnFd);
	} 


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	/*
	Note:
		fd = file we are working in.
		offset = the position amount we are trying to shift (foward or backward)
		whence = either from Start of File, Current Spot in file, or End of File.
	*/
	int fileLength = sizeof(fcbArray[fd].buf);
	if(offset > 0){
		//Start offset from start of file.
	if(whence = SEEK_SET){
		//set file position to start of file minus offset.
		fcbArray[fd].individualFilePosition = fcbArray[fd].buf[0] - offset;
	}
	//Start offset from end of file.
	else if(whence = SEEK_END){
		//set file position to end of file minus offset.
		fcbArray[fd].individualFilePosition = fileLength - offset;

	}
	//Start offset from current file position.
	else{
		//setfile position to current file position minus offset.
		fcbArray[fd].individualFilePosition =- offset; 
	}

	}
	if(offset < 0){

	//Start offset from start of file.
	if(whence = SEEK_SET){
	//set position to start of buffer plus offset.
	fcbArray[fd].individualFilePosition = fcbArray[fd].buf[0] + offset;

	}
	//Start offset from end of file.
	else if(whence = SEEK_END){
	//set position to end of file plus offset.
	fcbArray[fd].individualFilePosition = fileLength + offset; 	

	}
	//Start offset from current file position.
	else{
	//set position to current position plus offset.
	fcbArray[fd].individualFilePosition += offset;
	}
	}

	return fcbArray[fd].individualFilePosition; //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	
		
	return (0); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{

	}
