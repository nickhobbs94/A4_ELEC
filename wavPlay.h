#ifndef WAVPLAY_H
#define WAVPLAY_H

#include "alt_types.h"

/* Function Prototypes */
void audioController();
Wave_Header check_header (alt_8 filename[], alt_u8 *err);
alt_32 load_fifo (alt_u8 init_flag);


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

#endif
