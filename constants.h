#define MAX_DE_NAME 256
#define MAX_DIRENTRIES 51
#include <stdint.h>
#include <time.h>
typedef struct VCB{
	uint64_t size_of_block; //size of a individual block
	uint64_t number_of_blocks; //counts the number of blocks
	uint64_t blocks_available; //holds blocks available
	uint64_t freespace_available; //holds the number of free blocks available
	uint64_t bitmap_starting_index; //where the bitmap starts


	uint64_t signature; //used to check if own the 

} VCB;



VCB* vcb;
unsigned char* bitmap;