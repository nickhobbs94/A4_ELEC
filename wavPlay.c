#include <stdio.h>
#include <stdlib.h>
#include "wavPlay.h"
#include "alt_types.h"
#include "threading.h"
#include "efs.h"
#include "SD_functions.h"
#include "terminalParse.h"
#include "AUDIO.h"
#include "opencores_i2c.h"
#include "opencores_i2c_regs.h"
#include "altera_up_avalon_audio_regs_dgz.h"
#include "altera_up_avalon_audio_dgz.h"
#include "altstring.h"
#include "conversions.h"

#define SIZE_OF_HEADER 44
#define FILE_BUFFER_LEN 8192
#define MAX_OUTPUT_BUFFER_LEN FILE_BUFFER_LEN/2
#define READ_IN 4096
#define MAX_FILENAME_LEN 12



alt_32 check_header (Wave_Header* header_data) {
	/* Init audio codec and device (only needs to be done once) */
	if ( !AUDIO_Init() ) {
		printf("Unable to initialise audio codec\n");
		return -1;
	}


	if (charsCompare(header_data->Format, (alt_8*)"WAVE", 4) == -1) {
		puttyPrintLine("Wrong format or corrupt header\n\r");
		return -1;
	}

	/* Set the sampling frequency (do this every time a new stream is loaded) */
	switch(header_data->Sample_Rate) {
		case 8000:   AUDIO_SetSampleRate(RATE_ADC8K_DAC8K_USB);           break;
		case 32000:	 AUDIO_SetSampleRate(RATE_ADC32K_DAC32K_USB);         break;
		case 44100:  AUDIO_SetSampleRate(RATE_ADC44K_DAC44K_USB);         break;
		case 48000:  AUDIO_SetSampleRate(RATE_ADC48K_DAC48K_USB);         break;
		case 96000:  AUDIO_SetSampleRate(RATE_ADC96K_DAC96K_USB);         break;
		default:     puttyPrintLine("Non-standard sampling rate\n\r");    return -1;
	}

	/* Print some header data */
	puttyPrintLine("Chunk ID: ");
	puttyPrintChars(header_data->Chunk_ID, STRING_LEN);
	puttyPrintLine("\n\rFormat: ");
	puttyPrintChars(header_data->Format, STRING_LEN);

	puttyPrintLine("\n\rSampleRate: %d\n\rNumChannels: %d\n\r", header_data->Sample_Rate, header_data->Num_Channels);
	puttyPrintLine("Size: %d\n\rBits Per Sample: %d\n\r", header_data->Subchunk2_Size, header_data->Bits_Per_Sample);

	/* Check the number of channels */
	if (header_data->Num_Channels != 1 && header_data->Num_Channels != 2){
		puttyPrintLine("Invalid number of channels\n\r");
	}

	return 0;
}


alt_32 load_fifo (alt_u8* fileBuffer, alt_32 file_buffer_size, alt_32 output_buffer_size, Wave_Header* header_data) {
	alt_u32 buffer1[MAX_OUTPUT_BUFFER_LEN];
	alt_u32 buffer2[MAX_OUTPUT_BUFFER_LEN];

	alt_up_audio_dev*  audio_dev;
	audio_dev = alt_up_audio_open_dev(AUDIO_NAME);

	alt_32 bytesPerSample = header_data->Bits_Per_Sample / 8;
	//printf("bytesPerSample %d\n", (int)bytesPerSample);

	alt_32 shift = 32 - header_data->Bits_Per_Sample;
	//printf("shift %d\n", (int)shift);
	alt_32 mask = 0xffffffff;

	if (header_data->Bits_Per_Sample == 8){
		shift--;
		mask = 0x0fffffff;
	}

	/* Split the buffer into the number of channels and amplify if not 32 bit sampling */
	alt_32 i;
	for (i=0; i<output_buffer_size; i++){
		switch(header_data->Num_Channels){
			case 1:
				buffer1[i] = extract_little(fileBuffer + i*bytesPerSample, bytesPerSample) & mask;
				buffer2[i] = buffer1[i] & mask;
				buffer1[i] = buffer1[i] << shift;
				buffer2[i] = buffer2[i] << shift;
				break;
			case 2:
				buffer1[i] = extract_little(fileBuffer + i*bytesPerSample*2, bytesPerSample) & mask;
				buffer2[i] = extract_little(fileBuffer + i*bytesPerSample*2 + bytesPerSample, bytesPerSample) & mask;
				buffer1[i] = buffer1[i] << shift;
				buffer2[i] = buffer2[i] << shift;
				break;
			default:
				printf("Unsupported number of channels\n");
				return -1;
		}
	}
	/* Write data into right and left channel of audio codec FIFO */
	//printf("alt_up_audio_write_fifo(%d, %d, %d, %d)", audio_dev, buffer1, size/header_data->Num_Channels, )
	alt_up_audio_write_fifo(audio_dev, (unsigned int*)buffer1, output_buffer_size, ALT_UP_AUDIO_RIGHT);
	alt_up_audio_write_fifo(audio_dev, (unsigned int*)buffer2, output_buffer_size, ALT_UP_AUDIO_LEFT);
	return 0;
}


alt_32 audioController(void* pdata) {
	alt_8 filename[20];
	altstrcpy(filename, pdata);
	File file;
	EmbeddedFileSystem efs;
	
	alt_u8 check = efs_init(&efs, "/dev/sda");
	if (check==0){
		printf("Filesystem correctly initialized\n");
	} else {
		printf("Could not init filesystem\n");
		return -1;
	}

	check = file_fopen(&file, &efs.myFs, (unsigned char*)filename, 'r');

	Wave_Header header_data;
	file_read(&file, sizeof(Wave_Header), (euint8*) &header_data);

	check = check_header(&header_data);

	if (check != 0){
		return -1;
	}

	alt_32 totalBytesRead = 0;
	alt_32 bytesRead;
	alt_u8 fileBuffer[FILE_BUFFER_LEN];
	alt_32 output_buffer_size = 1024;
	alt_32 fileBuffer_size = output_buffer_size * (header_data.Bits_Per_Sample / 8) * header_data.Num_Channels;

	alt_up_audio_dev*  audio_dev;
	audio_dev = alt_up_audio_open_dev(AUDIO_NAME);

	printf("begin loop\n");

	while (totalBytesRead < header_data.Subchunk2_Size){
		bytesRead = file_read(&file, fileBuffer_size, fileBuffer);
		if (bytesRead == 0){
			break;
		}

		totalBytesRead += bytesRead;
		/* wait until FIFO has enough space */
		//printf("%d %d\n", alt_up_audio_write_fifo_space(audio_dev,ALT_UP_AUDIO_RIGHT), output_buffer_size);
		while(alt_up_audio_write_fifo_space(audio_dev,ALT_UP_AUDIO_RIGHT) < output_buffer_size / header_data.Num_Channels){
			//printf("delay\n");
			OSTimeDly(2);
		}
		while(alt_up_audio_write_fifo_space(audio_dev,ALT_UP_AUDIO_LEFT) < output_buffer_size / header_data.Num_Channels){
			OSTimeDly(2);
		}

		load_fifo(fileBuffer, fileBuffer_size, output_buffer_size, &header_data);
	}
	printf("EXIT\n");

	file_fclose(&file);
	fs_umount(&efs.myFs);
	return 0;
}


alt_32 readFile(alt_8* filename, alt_u8* fileBuffer, File** filePointer, alt_32 size){
	EmbeddedFileSystem* efsl;
	File file;
	euint16 e;

	if (filePointer == NULL){
		efsl = *(SD_mount());

		if (efsl==NULL)
			return -1;

		/* Get path */
		alt_8 path[SD_MAX_PATH_LENGTH];
		altstrcpy(path,filename);

		if(file_fopen(&file, &(efsl->myFs), (char*)path, 'r')!=0){
			puttyPrintLine("Could not open file for reading\n\r");
			return -1;
		}

		*filePointer = &file;
	} else {
		file = **filePointer;
	}

	e=file_read(&file,size,fileBuffer);

	return (alt_32)e;
}

