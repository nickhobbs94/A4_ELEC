/*
	Every function that is implemented on our terminal will be in this file.
	This allows for easy editing and extensions.
*/

#ifndef TERMINALFUNCTIONS_H
#define TERMINALFUNCTIONS_H

/* Includes */
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "conversions.h"
#include "alt_types.h"
#include "efs.h"
#include "ls.h"
#include "SD_functions.h"
#include "altstring.h"
#include "threading.h"
#include "includes.h"
#include "wavPlay.h"
#include "terminalFunctions.h"
#include "LCD_Control.h"
#include "terminalParse.h"
#include "AUDIO.h"
#include "opencores_i2c.h"
#include "opencores_i2c_regs.h"
#include "altera_up_avalon_audio_regs_dgz.h"
#include "altera_up_avalon_audio_dgz.h"

/* Magic numbers */
#define NUMBER_OF_LEDS 18
#define BINARY_BITS_IN_DECIMAL(decimalNumber) log2(decimalNumber)+1
#define MAX_LED_REGISTER 131071
#define MIN_LED_REGISTER -131072


/* ----------------------------------- Functions ----------------------------------- */

/* Print to the LCD and the console */
alt_32 echo(alt_32 argc, alt_8* argv[]){
	alt_32 i;
	LCD_Init();

	/* Loop over each argument provided to the function and print each one to the LCD and console */
	for (i=1; i<argc; i++){
		puttyPrintLine(argv[i]);
		puttyPrintLine("\r\n");
		LCD_Show_Text(argv[i]);
		LCD_Show_Text((alt_8*) " ");
	}

	if (argc <= 1){
		puttyPrintLine("<Empty input string>\n\r");
	}
	return 0;
}

/* Add two or more numbers and show the sum on the LCD */
alt_32 add(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s number1 number2 ...\n\r", argv[0]);
		return -1;
	}

	alt_32 sum = 0;
	alt_32 i;
	alt_32 temp;
	alt_32 tempsum;
	LCD_Init();
	alt_8 printstring[MAX_STRINGLEN];
	altmemset(printstring, '\0', MAX_STRINGLEN);

	for (i=1; i<argc; i++){
		/* Print each argument */
		altstrcat(printstring, argv[i]);

		/* Print a trailing '+' for all but the last argument */
		if (i<argc-1) {
			altstrcat(printstring, "+");
		}

		/* Get an int from each arg and add it to temp */
		temp = intfromstring(argv[i]);
		if (temp == 0 && (argv[i][0]!='0' || (argv[i][0]!='-' && argv[i][1]!='0') )){
			puttyPrintLine("Invalid input integer (too big/not a number)\n\r");
			return -1;
		}
		tempsum = temp + sum;
		/* Check for signed overflow */
		if (temp > 0 && sum > 0 && tempsum < 0){
			puttyPrintLine("Addition overflow\n\r");
			return -1;
		} else if (temp < 0 && sum < 0 && tempsum >= 0){
			puttyPrintLine("Signed addition overflow\n\r");
			return -1;
		}
		sum = tempsum;
	}
	puttyPrintLine("%s=%d\n\r",printstring,sum);
	LCD_Show_Decimal(sum);
	return 0;
}

/* Updates the red LEDs to be the binary representation of a number */
alt_32 ledr(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s number1\n\r", argv[0]);
		return -1;
	}
	alt_32 dec=0;
	dec = intfromstring(argv[1]);

	/* Return from the program if input wasn't a number or bigger than the max for the LEDs*/
	if ((dec == 0 && (altstrcmp(argv[1], (alt_8*)"0") != 0))){
		puttyPrintLine("Argument 1 for ledr is invalid: %s\n\r",argv[1]);
		return -1;
	} else if (dec > MAX_LED_REGISTER || dec < MIN_LED_REGISTER){
		puttyPrintLine("User input outside of acceptable range (%d,%d)\n\r",MIN_LED_REGISTER,MAX_LED_REGISTER);
		return -1;
	}
	IOWR(LED_RED_BASE, 0, dec);
	return 0;
}

/* shows the switch configuration as hex on the 7-seg display */
alt_32 switch_function(alt_32 argc, alt_8* argv[]){
	alt_32 dec = IORD(SWITCH_PIO_BASE,0);
	alt_32 i;
	alt_32* hex = calloc(8,sizeof(alt_32));
	decimaltohex(hex,dec);
	static const alt_u8  Map[] = {
		63, 6, 91, 79, 102, 109, 125, 7,
		127, 111, 119, 124, 57, 94, 121, 113
	};  // 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f
	for(i=0;i<8;++i) {
		//printf("%x\n",hex[i]);
		IOWR(SEG7_DISPLAY_BASE,i,Map[hex[i]]);
	}
	free(hex);
	return 0;
}

/* lists the contents of a directory */
alt_32 ls_path(alt_32 argc, alt_8* argv[]){
	EmbeddedFileSystem* efsl;
	DirList list;
	alt_8 isEmptyDir = 1;
	
	efsl = *(SD_mount());

	if (efsl==NULL) return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());

	if (argc>1){
		SD_updatePath(path,argv[1]);
	}
	//printf("PATH: %s\n",path);
	puttyPrintLine("PATH: %s\n\r",path);

	/* Read directory */
	alt_8 checkpath = ls_openDir(&list,&(efsl->myFs),path);
	//printf("checkpath=%d\n",checkpath);
	if (checkpath!=0){
		//printf("Unable to read path: %s\n",path);
		puttyPrintLine("Unable to read path: %s\n\r",path);
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}

	char attribute;
	while(ls_getNext(&list)==0){
		isEmptyDir = 0;
		attribute = SD_getFileAttribute(list.currentEntry.Attribute);
		//printf("%s \t%c \t(%d)\n",
		//		list.currentEntry.FileName,
		//		attribute,
		//		list.currentEntry.FileSize
		//		);
		puttyPrintLine((alt_8*)list.currentEntry.FileName);
		if (attribute == 'd'){
			puttyPrintLine("        [Folder]        ");
		} else {
			puttyPrintLine("        [ File ]        ");
		}
		puttyPrintLine("(%d bytes)\n\r",list.currentEntry.FileSize);
	}

	if (isEmptyDir){
		//printf("The directory is empty\n");
		puttyPrintLine("The directory is empty\n\r");
	}
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	return 0;
}

/* changes the current directory */
alt_32 change_dir(alt_32 argc, alt_8* argv[]){
	EmbeddedFileSystem* efsl;
	DirList list;
	efsl = *(SD_mount());


	alt_8* path;
	path = SD_getCurrentPath();

	/* check it is a valid directory that the user wants to change the path to */
	alt_8 checkpath=0;
	if (argc>1){
		checkpath = ls_openDir(&list,&(efsl->myFs),(char*)argv[1]);
		if (checkpath==0){
			SD_updatePath(path,argv[1]);
		} else {
			printf("Invalid path: %s\n",argv[1]);
			puttyPrintLine("Invalid path: %s\n\r",argv[1]);
		}
	}
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	
	printf("PATH: %s\n",path);
	puttyPrintLine("PATH: %s\n\r",path);
	return 0;
}

alt_32 tf_mount(alt_32 argc, alt_8* argv[]){
	SD_mount();
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	return 0;
}

alt_32 tf_unmount(alt_32 argc, alt_8* argv[]){
	SD_unmount();
	return 0;
}


alt_32 make_directory(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		printf("Syntax: %s directory\n",argv[0]);
		puttyPrintLine("Syntax: %s directory\n\r",argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	printf("Making %s\n",path);
	puttyPrintLine("Making %s\n\r",path);
	alt_32 check;

	check = makedir(&(efsl->myFs), path);
	if (check != 0){
		puttyPrintLine("This directory already exists\n\r");
	}

	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

/* For ability to delete directories
 * Had to edit efsl file: /FileSystemLibSrc/src/ui.c line 139 from
 * 		if((fs_findFile(fs,(eint8*)filename,&loc,0))==1){
 * to
 * 		if((fs_findFile(fs,(eint8*)filename,&loc,0))){
 */
alt_32 delete_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;

	efsl = *(SD_mount());

	if (efsl==NULL){
		return -1;
	}

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	/* Remove the file */
	alt_16 result = rmfile(&(efsl->myFs), path);

	if(result == -1){
		puttyPrintLine("Error: file does not exist\n\r");
	}

	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	
	return 0;
}

alt_32 write_new_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath contents...\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;
	File file;
	alt_8 write_buffer[SD_MAX_PATH_LENGTH];
	altmemset(write_buffer,'\0',SD_MAX_PATH_LENGTH);
	alt_32 i;
	for(i=2;i<argc;i++){
		altstrcat(write_buffer,argv[i]);
		if (i<argc-1){
			altstrcat(write_buffer," ");
		}
	}

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	if(file_fopen(&file, &(efsl->myFs), path, 'w')!=0){
				puttyPrintLine("Could not open file for writing\n\r");
				if (UNMOUNT_SD_AFTER_OPERATION){
					SD_unmount();
				}
				return 0;
	}
	puttyPrintLine("File opened for writing.\n\r");

	if(file_write(&file,altstrlen((alt_8*)write_buffer),(alt_8*)write_buffer) == altstrlen((alt_8*)write_buffer)){
		puttyPrintLine("File written.\n\r");
	} else {
		puttyPrintLine("Could not write file.\n\r");
	}

	file_fclose(&file);
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

alt_32  read_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;
	File file;
	euint8 buffer[513];
	euint16 e;//,f;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[1]);

	if(file_fopen(&file, &(efsl->myFs), path, 'r')!=0){
				puttyPrintLine("Could not open file for reading\n\r");
				if (UNMOUNT_SD_AFTER_OPERATION){
					SD_unmount();
				}
				return -1;
	}
	printf("File %s opened for reading.\n",path);
	//puttyPrintLine("Opened file: ");
	//puttyPrintLine(path);
	//puttyPrintLine(" for reading\n");

	while((e=file_read(&file,512,buffer))){
		//printf("%d\n", e);
		/*for(f=0;f<e;f++){
			printf("%c",buffer[f]);
		}*/
		if (e>=513) {
			printf("ERROR READING FILE\n\r");
		} else {
			buffer[e] = '\0';
			printf("%s", buffer);
		}
		

	}
	puttyPrintLine("\n\r");

	file_fclose(&file);
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

alt_32 copy_file(alt_32 argc, alt_8* argv[]){
	if (argc <= 2){
		puttyPrintLine("Syntax: %s source destination\n\r", argv[0]);
		return -1;
	}
	EmbeddedFileSystem* efsl;
	File file1, file2;
	euint8 buffer[512];
	euint16 e;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path of read file */
	alt_8 read_file_path[SD_MAX_PATH_LENGTH];
	altstrcpy(read_file_path,SD_getCurrentPath());
	SD_updatePath(read_file_path,argv[1]);

	/* Get absolute path of write file */
	alt_8 write_file_path[SD_MAX_PATH_LENGTH];
	altstrcpy(write_file_path,SD_getCurrentPath());
	SD_updatePath(write_file_path,argv[2]);

	if(file_fopen(&file1, &(efsl->myFs), read_file_path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}
	puttyPrintLine("File opened for reading.\n\r");

	e=file_read(&file1,512,buffer);
	if(file_fopen(&file2, &(efsl->myFs), write_file_path, 'w')!=0){
		puttyPrintLine("Could not open file for writing\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}
	puttyPrintLine("File opened for writing.\n\r");

	if(file_write(&file2,e,(alt_8*)buffer)==e){
		puttyPrintLine("File written.\n\r");
	} else {
		puttyPrintLine("could not write file.\n\r");
	}

	file_fclose(&file1);
	file_fclose(&file2);
	
	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}


alt_32 wav_play(alt_32 argc, alt_8* argv[]){
	/* Check number of arguments */
	if (argc <= 1){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		return -1;
	}

	Wave_Header header_data;
	alt_u8 errorCheck = 0;

	/* Check and get the wav header */
	header_data = check_header(argv[1], &errorCheck); // TODO get errors working
	
	puttyPrintLine("Chunk ID: ");
	puttyPrintChars(header_data.Chunk_ID, STRING_LEN);
	puttyPrintLine("\n\rFormat: ");
	puttyPrintChars(header_data.Format, STRING_LEN);

	/* stop interrupt */
	alt_up_audio_dev*  audio_dev;
	audio_dev = alt_up_audio_open_dev(AUDIO_NAME);
	alt_up_audio_disable_write_interrupt(audio_dev);


	/* Send the file in the mail */
	POST mail;
	printf("%s\n", argv[1]);
	altstrcpy(mail.filename, argv[1]);
	mail.header_data = header_data;
	OSMboxPost(Mbox1, &mail);
	return 0;
}


#endif

