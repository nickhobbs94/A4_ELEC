#ifndef WAVPLAY_H
#define WAVPLAY_H

#include "alt_types.h"

#define SIZE_OF_HEADER 44
#define OUTPUT_BUFFER_LEN 512
#define READ_IN 4096
#define MAX_FILENAME_LEN 12



Wave_Header check_header (alt_8 filename[], alt_u8 *err) {
	/* Try to mount SD card */
	EmbeddedFileSystem* efsl;
	efsl = *(SD_mount());
	if (efsl==NULL){
		return;
	}

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path, SD_getCurrentPath());
	SD_updatePath(path, filename);

	/* Try to open file */
	File file;
	if(file_fopen(&file, &(efsl->myFs), path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return;
	}

	/* Try to read header from file */
	euint8 buffer[SIZE_OF_HEADER];
	euint16 result;
	result = file_read(&file,SIZE_OF_HEADER,buffer);
	if (result != SIZE_OF_HEADER){
		puttyPrintLine("Error reading header\n\r");
		return;
	}

	/* Init audio codec and device (only needs to be done once) */
	if ( !AUDIO_Init() ) {
		printf("Unable to initialise audio codec\n");
		return;
	}

	/* Set pointer to header structure to the newly read header */
	Wave_Header* header_data;
	header_data = (Wave_Header*) buffer;

	if (charsCompare(header_data->Format, "WAVE", 4) == -1) {
		puttyPrintLine("Wrong format or corrupt header\n\r");
		return;
	}

	/* Set the sampling frequency (do this every time a new stream is loaded) */
	switch(header_data->Sample_Rate) {
		case 8000:   AUDIO_SetSampleRate(RATE_ADC8K_DAC8K_USB);           break;
		case 32000:	 AUDIO_SetSampleRate(RATE_ADC32K_DAC32K_USB);         break;
		case 44100:  AUDIO_SetSampleRate(RATE_ADC44K_DAC44K_USB);         break;
		case 48000:  AUDIO_SetSampleRate(RATE_ADC48K_DAC48K_USB);         break;
		case 96000:  AUDIO_SetSampleRate(RATE_ADC96K_DAC96K_USB);         break;
		default:     puttyPrintLine("Non-standard sampling rate\n\r");    return;
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

	file_fclose(&file);
	return *header_data;
}


alt_32 load_fifo (alt_u8 init_flag) {
	static alt_8 filename[12];
	static alt_32 totalBytesRead;
	static alt_32 FILE_BUFFER_LEN;
	static alt_up_audio_dev*  audio_dev;
	static Wave_Header header_data;
	static alt_32 bytesPerSample;



	if (init_flag){
		/* get filename and header from mail */
		alt_u8 err;
		alt_u16 timeout = 0;
		POST mail = *(POST*)OSMboxPend(Mbox1, timeout, &err);
		altstrcpy(filename, mail.filename);
		header_data = mail.header_data;
		printf("mail: %s, %d\n", filename, header_data.Bits_Per_Sample);

		bytesPerSample = header_data.Bits_Per_Sample / 8;
		FILE_BUFFER_LEN = OUTPUT_BUFFER_LEN * header_data.Num_Channels * bytesPerSample;
		audio_dev = alt_up_audio_open_dev(AUDIO_NAME);
		totalBytesRead = 0;
	}

	/* make buffers */
	alt_u32 buffer1[OUTPUT_BUFFER_LEN];
	alt_u32 buffer2[OUTPUT_BUFFER_LEN];


	/* read file */


	/* load buffers into FIFO */

	/* set interrupt to run */
	return 0;
}

#endif
