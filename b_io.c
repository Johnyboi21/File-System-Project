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
#include "constants.h"
#include "directory_entry.h"
#include "fsLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int bufLen;		//holds how many valid bytes are in the buffer
	int individualFilePosition; //keep track where we are in a individual file.
	int currentBlock;	//holds currentBlock  
	int size_bytes;	//holds size of entry in bytes
    int positionInParent; // File's location in parent


	unsigned short d_reclen;    //length of this record 
    uint64_t    directoryStartLocation;
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

    printf("Opening %s\n", filename);
	if (startup == 0) b_init();  //Initialize our system
	parseData *fdData = parsePath(filename);	//obtain file


    // Check if dir given in path
    if(fdData->testDirectoryStatus == 1){ 
        printf("Error: Target is not a file.\n");
        free(fdData->dirPointer);
        free(fdData);
        return -1;
    }
    // If path is valid but last token DNE, create new dir with last token name  
    else if(fdData->testDirectoryStatus == 0 && fdData->dirPointer != NULL){
        // Make sure they have the flag before creating
        if(((flags | O_CREAT) != flags)){
            printf("Error: File does not exist: No create permission\n");
            if(fdData->dirPointer != NULL){
                free(fdData->dirPointer);
            }

            free(fdData);
            return -1;
        }
        if(createFile(fdData) == 0){
            printf("Error: Could not create target file.\n");
            free(fdData->dirPointer);
            free(fdData);
            return -1;
        }

        free(fdData->dirPointer);
        free(fdData);

        fdData = parsePath(filename);
        
    }
    else if(fdData->testDirectoryStatus != 2){
        printf("Error: Path was invalid.\n");
        free(fdData);
        return -1;
    }


	
	b_io_fd returnFd = b_getFCB();				// get our own file descriptor
    printf("Got an fcd of %d\n", returnFd);
    int elem = fdData->directoryElement;
	DE* tempEntryPtr = malloc(vcb->size_of_block * fdData->dirPointer->d_reclen);
	LBAread(tempEntryPtr, fdData->dirPointer->d_reclen, fdData->dirPointer->directoryStartLocation);

	fcbArray[returnFd].buf = malloc(tempEntryPtr[elem].size * vcb->size_of_block);	//allocate memory
    LBAread(fcbArray[returnFd].buf, tempEntryPtr[elem].size, tempEntryPtr[elem].location);
	fcbArray[returnFd].flagPassed = flags;

	if(fcbArray[returnFd].buf == NULL){		//catch test
		printf("Failed to allocate memory");
		return - 1;
	}

    
	fcbArray[returnFd].size_bytes = tempEntryPtr[fdData->directoryElement].size_bytes;
	

	//get file information.
	fcbArray[returnFd].d_reclen = tempEntryPtr[elem].size;	
	fcbArray[returnFd].fileType = tempEntryPtr[elem].is_directory;
	fcbArray[returnFd].d_name == fdData->nameOfLastToken;
	fcbArray[returnFd].size_bytes = tempEntryPtr[elem].size_bytes;
	fcbArray[returnFd].currentBlock = 0;
    fcbArray[returnFd].index = 0;
    fcbArray[returnFd].bufLen = fcbArray[returnFd].size_bytes;
    fcbArray[returnFd].directoryStartLocation = tempEntryPtr[elem].location;
    fcbArray[returnFd].positionInParent = elem;

	//To-Do: Free mallocs.
	free(fdData->dirPointer);
	free(fdData);
    free(tempEntryPtr);
	
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
    if ((fd < 0) || (fd >= MAXFCBS)){
        return (-1);                    //invalid file descriptor
    }

	//Flag check for write call
	if(!fcbArray[fd].flagPassed & O_WRONLY){
		printf("\nb_read ERROR: NO WRITE FLAG PASSED.\n");
		return -1;
	}
    //total bytes written count;
    int bytesWroteCount = 0;

    b_fcb* file = &fcbArray[fd];
    int available = vcb->size_of_block * file->d_reclen;
    available-= (sizeof(DE)*2 + file->size_bytes);
    
    printf("Bytes available in file is %d. Trying to write %d\n", available, count);
    while(bytesWroteCount < count){
        int bytesWritten = 0;
        if((bytesWroteCount + B_CHUNK_SIZE) > count){
            bytesWritten = B_CHUNK_SIZE;
        }
        else{
            bytesWritten = (count - bytesWroteCount);
        }

        //copy user data to our writee buffer.
        memcpy(file->buf + (sizeof(DE)*2) + file->individualFilePosition,
         buffer, count);

        bytesWroteCount += bytesWritten;
    }
	//EOF check
	if(bytesWroteCount + count > fcbArray->size_bytes){;
		printf("EOF Reached: \n");
		count = file->size_bytes - bytesWroteCount + count;
	}


    /*
    DE* saveFile = malloc(file->d_reclen * vcb->size_of_block);
    LBAread(saveFile, file->d_reclen, file->directoryStartLocation);
    saveFile->size_bytes = file->size_bytes;
    saveFile->size = file->size_bytes;
    time_t time_mod = time(0);
    saveFile->last_modified = time_mod;
    */
    

    DE* saveFile = (DE*)file->buf;
    saveFile->size_bytes = 22;
    saveFile->size = file->d_reclen;
    time_t time_mod = time(0);
    saveFile->last_modified = time_mod;

    // Update parent with new info of file
    DE* parent = malloc(saveFile[1].size * vcb->size_of_block);
    LBAread(parent, saveFile[1].size, saveFile[1].location);
    parent[file->positionInParent].size_bytes = saveFile->size_bytes;
    parent[file->positionInParent].size = saveFile->size;
    parent[file->positionInParent].location = saveFile->location;
    parent[file->positionInParent].last_modified = time_mod;
    printf("Writing %d blocks to %d\n", file->d_reclen, file->directoryStartLocation);
    LBAwrite(parent, parent->size, parent->location);
    LBAwrite(saveFile, file->d_reclen, file->directoryStartLocation);
    
    free(parent);
    printf("Wrote %d\n", 22);
    return bytesWroteCount;
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
int b_read (b_io_fd fd, char * buffer, int count){

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)){
		return 0; 					//invalid file descriptor
	}

    int flag = fcbArray[fd].flagPassed;
	if(!((flag & O_RDONLY == flag) || ( flag & O_RDWR) == flag)){
		printf("\nb_read ERROR: NO READ OR READ-WRITE FLAG PASSED.\n");
		return 0;
	}

    printf("About to read file with a count of %d\n", count);

	b_fcb* fcb = &fcbArray[fd];
	int part1, part2, part3;
	int blocksToCopy, bytesRead, bytesReturned;
	
	int bytesInBuffer = fcbArray[fd].size_bytes - fcbArray[fd].index;
    printf("Bytes in buffer is %d\n", bytesInBuffer);
    printf("size_bytes is %d and index is %d\n", fcbArray[fd].size_bytes, fcbArray[fd].index);

	int deliveredBytes = fcbArray[fd].currentBlock * B_CHUNK_SIZE - bytesInBuffer;
    printf("deliveredBytes is %d\n", deliveredBytes);
	if ((count + deliveredBytes) > fcbArray[fd].size_bytes){
		count = (fcbArray[fd].size_bytes) - deliveredBytes;
        count = 50;
        printf("Count + delivered was greater that size, so changed count to %d\n", count);
		if (count < 0){
			return -1;
		}
	}

	if (count <= bytesInBuffer)
	{
        printf("Count was <= bytesInBuffer. Only need part1\n");
		part1 = count;
		part2 = 0;
		part3 = 0;
	}
	else
	{
		part1 = bytesInBuffer;
		part3 = count - bytesInBuffer;
		blocksToCopy = part3 / B_CHUNK_SIZE; 
		part2 = blocksToCopy * vcb->size_of_block;//mfs_blockSize;
		part3 = part3 - part2;

        printf("Set part1 to %d, part2 to %d, and part3 to %d\n", part1, part2, part3);
        printf("blocksToCopy is %d\n", blocksToCopy);
	}


	if (part1 > 0)
	{
        printf("Read starting with offset %ld. Reading %d bytes.\n", (sizeof(DE)*2 + fcbArray[fd].index), part1);
		memcpy(buffer, fcbArray[fd].buf + sizeof(DE)*2 + fcbArray[fd].index, part1);
		fcbArray[fd].index += part1;

        printf("Read part 1. Position is now %d\n", fcbArray[fd].index);
	}

	if (part2 > 0)
	{
        // 
		bytesRead = LBAread(buffer + part1, blocksToCopy, fcbArray[fd].currentBlock + fcbArray[fd].directoryStartLocation);
		fcbArray[fd].currentBlock += blocksToCopy;
		bytesRead = bytesRead * B_CHUNK_SIZE;
		part2 = bytesRead;
	}

	if (part3 > 0)
	{
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].directoryStartLocation);
		bytesRead = bytesRead * vcb->size_of_block;
		fcbArray[fd].currentBlock += 1;
		fcbArray[fd].index = 0;
		fcbArray[fd].bufLen = bytesRead;
		if (bytesRead < part3)
		{
			part3 = bytesRead;
		}

		if (part3 > 0)
		{
			
			memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			fcbArray[fd].index += part3;
		}
	}

	bytesReturned = part1 + part2 + part3;

    printf("Read is returning %d\n", bytesReturned);
	return (bytesReturned); 

}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
        if(fd < 0 || fd >= MAXFCBS){
            return -1;
        }

        b_fcb* file = &fcbArray[fd];
        if (file->buf != NULL){
           free(file->buf);
        }

        file->buf = NULL;


        return 1;
	}
