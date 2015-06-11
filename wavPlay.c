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

#define SIZE_OF_HEADER 44
#define OUTPUT_BUFFER_LEN 512
#define READ_IN 4096
#define MAX_FILENAME_LEN 12



Wave_Header check_header (alt_8 filename[], alt_u8 *err) {
	Wave_Header found_header;
	found_header.Chunk_ID[0] = 0;

	/* Try to mount SD card */
	EmbeddedFileSystem* efsl;
	efsl = *(SD_mount());
	if (efsl==NULL){
		return found_header;
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
		return found_header;
	}

	/* Try to read header from file */
	euint8 buffer[SIZE_OF_HEADER];
	euint16 result;
	result = file_read(&file,SIZE_OF_HEADER,buffer);
	if (result != SIZE_OF_HEADER){
		puttyPrintLine("Error reading header\n\r");
		return found_header;
	}

	/* Init audio codec and device (only needs to be done once) */
	if ( !AUDIO_Init() ) {
		printf("Unable to initialise audio codec\n");
		return found_header;
	}

	/* Set pointer to header structure to the newly read header */
	Wave_Header* header_data;
	header_data = (Wave_Header*) buffer;

	if (charsCompare(header_data->Format, "WAVE", 4) == -1) {
		puttyPrintLine("Wrong format or corrupt header\n\r");
		return found_header;
	}

	/* Set the sampling frequency (do this every time a new stream is loaded) */
	switch(header_data->Sample_Rate) {
		case 8000:   AUDIO_SetSampleRate(RATE_ADC8K_DAC8K_USB);           break;
		case 32000:	 AUDIO_SetSampleRate(RATE_ADC32K_DAC32K_USB);         break;
		case 44100:  AUDIO_SetSampleRate(RATE_ADC44K_DAC44K_USB);         break;
		case 48000:  AUDIO_SetSampleRate(RATE_ADC48K_DAC48K_USB);         break;
		case 96000:  AUDIO_SetSampleRate(RATE_ADC96K_DAC96K_USB);         break;
		default:     puttyPrintLine("Non-standard sampling rate\n\r");    return found_header;
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


alt_32 load_fifo (alt_u8* fileBuffer, alt_32 size, Wave_Header* header_data) {
	alt_u32 buffer1[OUTPUT_BUFFER_LEN];
	alt_u32 buffer2[OUTPUT_BUFFER_LEN];

	alt_32 bytesPerSample = header_data->Bits_Per_Sample / 8;
	alt_32 shift = 32 - header_data->Bits_Per_Sample;
	alt_32 mask = 0xffffffff;

	if (header_data->Bits_Per_Sample == 8){
		shift--;
		mask = 0x0fffffff;
	}

	/* Split the buffer into the number of channels and amplify if not 32 bit sampling */
	for (i=0; i<size; i++){
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
	alt_up_audio_write_fifo(audio_dev, (unsigned int*)buffer1, size/header_data->Num_Channels, ALT_UP_AUDIO_RIGHT);
	alt_up_audio_write_fifo(audio_dev, (unsigned int*)buffer2, size/header_data->Num_Channels, ALT_UP_AUDIO_LEFT);
	return 0;
}

void audio_irq_handler(void* context) {
	if (alt_up_audio_write_interrupt_pending(audio_dev)) {
		//Disable audio output interrupt 
		alt_up_audio_disable_write_interrupt(audio_dev);
		POST* mail = (POST*) context;
		Wave_Header* header_data = &(context->header_data);
		load_fifo(context->fileBuffer, context->fileBufferSize, header_data);
	}
}

void audioController() {
	alt_u8 err;
	alt_u16 timeout=0;
	POST mail;
	alt_8 isPlaying = 0;
	alt_32 FILE_BUFFER_LEN;
	alt_u8* fileBuffer;
	alt_u8* currentBuffer;
	alt_u8 currentBufferNumber = 0;

	while (1) {
		mail = *(POST*) OSMboxPend(Mbox1, timeout, &err);
		printf("%s\n", mail.filename);
		/* malloc 2 file buffers */
		FILE_BUFFER_LEN = OUTPUT_BUFFER_LEN * mail.header_data.Num_Channels * mail.header_data.Bits_Per_Sample/8;
		fileBuffer = malloc(2 * FILE_BUFFER_LEN);
		isPlaying = 1;

		/* update mail for ISR */

		/* while playing file */
		while(isPlaying){
			/* check for new mail but don't pend */
			/* read data into current file buffer */
			/* wait until interrupts aren't running anymore */
			alt_irq_register(AUDIO_IRQ, &mail, audio_irq_handler);
			/* set interrupt for current file buffer */
			/* switch to other file buffer */
		}
		free(fileBuffer);
	}
}

