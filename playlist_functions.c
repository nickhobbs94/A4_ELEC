#include <stdio.h>
#include "playlist_functions.h"
#include "alt_types.h"
#include "terminalParse.h"
#include "threading.h"
#include "altstring.h"
#include "SD_functions.h"
#include "efs.h"

#define PLAYLIST_HEADER_SIZE 8

alt_32 playlist_print(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("%s\n\r",argv[1]);
	return 0;
}

alt_32 playlist_add(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("%s\n\r",argv[1]);
	return 0;
}

alt_32 playlist_new(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("%s\n\r",argv[1]);
	return 0;
}

alt_32 playlist_delete(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("%s\n\r",argv[1]);
	return 0;
}

alt_32 playlist_remove(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("%s\n\r",argv[1]);
	return 0;
}

alt_32 playlist_help(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("%s\n\r",argv[1]);
	return 0;
}

alt_32 playlist_play(alt_32 argc, alt_8* argv[]){
	if (argc <= 2){
		puttyPrintLine("Syntax: %s filepath\n\r", argv[0]);
		puttyPrintLine("Type \"%s help\" for a list of commands\n\r", argv[0]);
		return -1;
	}
	puttyPrintLine("%s %s\n\r",argv[1], argv[2]);
	OSTaskDel(TASK3_PRIORITY);
	OSTaskDel(TASK2_PRIORITY);
	alt_8 file_contents[MAX_SIZE_PLAYLIST_FILE+1];// = "2ch8bit.wav\n2ch32bit.wav\n2ch16bit.wav\nbebop.wav \n2ch16bit.wav\nbad.wav";

	EmbeddedFileSystem* efsl;
	efsl= *(SD_mount());
	if (efsl==NULL){
		return -1;
	}
	File file;
	alt_32 check = file_fopen(&file, &efsl->myFs, argv[2], 'r');
	if (check != 0){
		return -1;
	}
	alt_32 playlist_size = file_read(&file, MAX_SIZE_PLAYLIST_FILE, file_contents);
	file_contents[playlist_size] = '\0';
	file_fclose(&file);
	printf("PLAYLIST:\n%s\n\n",file_contents);
	check = charsCompare("playlist", file_contents, PLAYLIST_HEADER_SIZE);
	if (check != 0){
		puttyPrintLine("%s is not a valid playlist\n\r", argv[2]);
		return -1;
	}

	static alt_8 playlist_contents[MAX_SIZE_PLAYLIST_FILE+1];
	altstrcpy(playlist_contents, file_contents + PLAYLIST_HEADER_SIZE);

	OSTaskCreateExt(task3,
					playlist_contents,
					(void *)&task3_stk[TASK_STACKSIZE-1],
					TASK3_PRIORITY,
					TASK3_PRIORITY,
					task3_stk,
					TASK_STACKSIZE,
					NULL,
					0);
	return 0;
}

