#ifndef THREADING_H
#define THREADING_H

#include "includes.h"

OS_EVENT* Mbox1;
OS_EVENT* Mbox2;

#define STRING_LEN 4
/* Define structure of header */
typedef struct{
	/* "RIFF" Descriptor */
	alt_8 Chunk_ID[STRING_LEN];
	alt_32 Chunk_Size;
	alt_8 Format[STRING_LEN];
	/* "fmt" sub-chunk */
	alt_8 Subchunk_ID[STRING_LEN];
	alt_32 Subchunk_Size;
	alt_16 Audio_Format;
	alt_16 Num_Channels;
	alt_32 Sample_Rate;
	alt_32 Byte_Rate;
	alt_16 Block_Align;
	alt_16 Bits_Per_Sample;
	/* "data" sub-chunk */
	alt_8 Subchunk2_ID[STRING_LEN];
	alt_32 Subchunk2_Size;
} Wave_Header;

typedef struct {
	alt_8 filename[12];
	Wave_Header header_data;
} POST;

#endif

