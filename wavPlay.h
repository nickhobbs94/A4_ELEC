#ifndef WAVPLAY_H
#define WAVPLAY_H

#include "alt_types.h"
#include "efs.h"


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


/* Function Prototypes */
alt_32 check_header (Wave_Header* header_data);
alt_32 load_fifo (alt_u8* fileBuffer, alt_32 file_buffer_size, alt_32 output_buffer_size, Wave_Header* header_data);
void audio_irq_handler(void* context);
alt_32 audioController(void* pdata);
alt_32 readFile(alt_8* filename, alt_u8* fileBuffer, File** filePointer, alt_32 size);
alt_32 playlist_manager(void* pdata);


#define MAX_LEN_PLAYLIST 20

#endif
