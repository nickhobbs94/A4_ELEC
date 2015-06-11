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


		alt_irq_register(AUDIO_IRQ, NULL, audio_irq_handler);
		// Here the argument a_void_pointer is of type {\tt void*}, that will
		// be copied to the argument context in the audio_irq_handler
		// function. If you don’t want to pass an argument then replace it
		// by NULL
	}

	/* make buffers */
	alt_u32 buffer1[OUTPUT_BUFFER_LEN];
	alt_u32 buffer2[OUTPUT_BUFFER_LEN];
	alt_32 bytesRead = 0;

	/* open file */

	/* read file */
	/* Read enough data for the number of channels to load one buffer's worth each */
	if (totalBytesRead + FILE_BUFFER_LEN < header_data->Subchunk2_Size){
		bytesRead = file_read(&file, FILE_BUFFER_LEN, fileBuffer);
		totalBytesRead += bytesRead;
	} else {
		printf("End of file\n");
		bytesRead = file_read(&file, (header_data->Subchunk2_Size - totalBytesRead), fileBuffer);
		totalBytesRead += bytesRead;
	}

	/* If the bytes just read is not large enough and the end of the file 
		has not been reached, then return an error */
	if (bytesRead != FILE_BUFFER_LEN   &&   totalBytesRead < header_data->Subchunk2_Size){
		puttyPrintLine("End of file was unexpectedly reached %d %d %d\n\r", bytesRead, totalBytesRead, header_data->Subchunk2_Size);
		free(fileBuffer);
		file_fclose(&file);
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}

	/* close file */

	/* load buffers into FIFO */

	/* set interrupt to run */
	alt_up_audio_enable_write_interrupt(audio_dev);
	return 0;
}

void audio_irq_handler(void* context) {
	if (alt_up_audio_write_interrupt_pending(audio_dev)) {
	//Disable audio output interrupt 
	alt_up_audio_disable_write_interrupt(audio_dev);
	}
}

void audioController() {
	unsigned char err;
	unsigned short timeout=0;
	POST mail;

	while (1) {
		mail = *(POST*) OSMboxPend(Mbox1, timeout, &err);
		printf("%s\n", mail.filename);
	}
}

