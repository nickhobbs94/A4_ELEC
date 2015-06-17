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

	EmbeddedFileSystem* efsl;
	File file;
	euint8 buffer[513];
	euint16 e;//,f;
	alt_8* playlist[50];
	alt_8 numberOfSongs;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path, SD_getCurrentPath());
	SD_updatePath(path, argv[2]);

	if(file_fopen(&file, &(efsl->myFs), path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}
	printf("File %s opened for reading.\n",path);

	while((e=file_read(&file,512,buffer))){
		if (e>=513) {
			puttyPrintLine("ERROR READING FILE\n\r");
		} else {
			buffer[e] = '\0';
			//puttyPrintLine("%s", buffer);
		}
	}
	//puttyPrintLine("\n\r");

	numberOfSongs = string_parser(buffer, playlist);

	if(altstrcmp(*playlist, "playlist")){
		puttyPrintLine("This file is not a playlist\n\r");
		return -1;
	}

	alt_32 i;

	for(i=1 ; i<numberOfSongs ; ++i){
		puttyPrintLine("%d. %s\n\r", i, playlist[i]);
	}

	file_fclose(&file);

	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}
	return 0;
}

alt_32 playlist_add(alt_32 argc, alt_8* argv[]){

	EmbeddedFileSystem* efsl;
	File fileread, filewrite, file;
	euint8 buffer[513];
	euint8 write_buffer[513];
	euint16 e;//,f;
	alt_8* playlist[50];
	alt_8 numberOfSongs;

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path, SD_getCurrentPath());
	SD_updatePath(path, argv[2]);

	if(file_fopen(&fileread, &(efsl->myFs), path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}
	printf("File %s opened for reading.\n", path);

	while((e=file_read(&fileread,512,buffer))){
		if (e>=513) {
			puttyPrintLine("ERROR READING FILE\n\r");
		} else {
			buffer[e] = '\0';
			//puttyPrintLine("%s", buffer);
		}
	}

	altstrcpy(write_buffer, buffer);

	numberOfSongs = string_parser(buffer, playlist);

	if(altstrcmp(*playlist, "playlist")){
		puttyPrintLine("This file is not a playlist\n\r");
		return -1;
	}

	file_fclose(&fileread);

	/* Remove the file */
	alt_16 result = rmfile(&(efsl->myFs), path);

	puttyPrintLine("Playlist deleted\n\r");

	alt_32 i;

	for(i=3 ; i<argc ; i++){
		if(file_fopen(&file, &(efsl->myFs), argv[i], 'r')!=0){
			puttyPrintLine("%s does not exist, moved onto next file\n\r", argv[i]);
		} else {
			altstrcat(write_buffer," ");
			altstrcat(write_buffer, argv[i]);
			file_fclose(&file);
		}
	}

	puttyPrintLine("%s\n\r", write_buffer);

	if(file_fopen(&filewrite, &(efsl->myFs), path, 'w')!=0){
		puttyPrintLine("Could not open file for writing!!!!\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}

	puttyPrintLine("File opened for writing.\n\r");

	e = 200; // <<< this is dodgy!! check this
	puttyPrintLine("%x\n\r", e);
	if(file_write(&filewrite,e,(alt_8*)write_buffer)==e){
		puttyPrintLine("File written.\n\r");
	} else {
		puttyPrintLine("could not write file.\n\r");
	}

	file_fclose(&filewrite);

	fs_umount(&efsl->myFs);

	return 0;
}

alt_32 playlist_new(alt_32 argc, alt_8* argv[]){

	EmbeddedFileSystem* efsl;
	File file;
	alt_8 write_buffer[SD_MAX_PATH_LENGTH];
	altmemset(write_buffer,'\0',SD_MAX_PATH_LENGTH);
	//alt_32 i;

	altstrcat(write_buffer,"playlist ");

	efsl = *(SD_mount());

	if (efsl==NULL)
		return -1;

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[2]);

	if(file_fopen(&file, &(efsl->myFs), path, 'w')!=0){
		puttyPrintLine("%s\n\r", path);
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

alt_32 playlist_delete(alt_32 argc, alt_8* argv[]){

	EmbeddedFileSystem* efsl;
	File file;
	euint8 buffer[513];
	euint16 e;//,f;
	alt_8* playlist[50];
	alt_8 numberOfSongs;

	efsl = *(SD_mount());

	if (efsl==NULL){
		return -1;
	}

	/* Get absolute path */
	alt_8 path[SD_MAX_PATH_LENGTH];
	altstrcpy(path,SD_getCurrentPath());
	SD_updatePath(path,argv[2]);

	if(file_fopen(&file, &(efsl->myFs), path, 'r')!=0){
		puttyPrintLine("Could not open file for reading\n\r");
		if (UNMOUNT_SD_AFTER_OPERATION){
			SD_unmount();
		}
		return -1;
	}

	printf("File %s opened for reading.\n",path);

	while((e=file_read(&file,512,buffer))){
		if (e>=513) {
			puttyPrintLine("ERROR READING FILE\n\r");
		} else {
			buffer[e] = '\0';
		}
	}

	numberOfSongs = string_parser(buffer, playlist);

	if(altstrcmp(*playlist, "playlist")){
		puttyPrintLine("This file is not a playlist\n\r");
		return -1;
	}

	file_fclose(&file);

	/* Remove the file */
	alt_16 result = rmfile(&(efsl->myFs), path);

	puttyPrintLine("Playlist deleted\n\r");

	/*if(result == -1){
		puttyPrintLine("Error: file does not exist\n\r");
	}*/

	if (UNMOUNT_SD_AFTER_OPERATION){
		SD_unmount();
	}

	return 0;
}

alt_32 playlist_remove(alt_32 argc, alt_8* argv[]){
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

alt_32 playlist_help(alt_32 argc, alt_8* argv[]){
	puttyPrintLine("playlist print\t playlist_name \t\t: prints the contents of the 'playlist_name'\n\r");
	puttyPrintLine("playlist add\t playlist_name args...\t: adds the songs listed in args (as file paths) to 'playlist_name'\n\r");
	puttyPrintLine("playlist new\t playlist_name \t\t: adds a new playlist called 'playlist_name'\n\r");
	puttyPrintLine("playlist delete\t playlist_name \t\t: deletes the playlist called 'playlist_name'\n\r");
	puttyPrintLine("playlist remove\t playlist_name args...\t: removes the songs listed in args (as file paths) from 'playlist_name'\n\r");
	puttyPrintLine("playlist play\t playlist_name \t\t: plays 'playlist_name'\n\r");
	return 0;
}



