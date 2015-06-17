/**
 * terminalParse contains the main infinite loop for the terminal 
 * it also includes functions to interpret the commands given and pass them
 * to terminalFunctions
 */


/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "altstring.h"
#include "terminalFunctions.h"
#include "alt_types.h"
#include "terminalParse.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <io.h>
#include "AUDIO.h"
#include "altera_up_avalon_audio_dgz.h"
#include "altera_up_avalon_audio_regs_dgz.h"

#include "LCD_Control.h"
#include "io.h"
#include "system.h"

#include "includes.h"


/* Magic numbers */
#define STRING_PARSER_MAXNUM_WORDS(string) string/2+1

#define BACKSPACE 0x7f
#define UP 0x41
#define DOWN 0x42
#define LEFT 0x44
#define RIGHT 0x43
#define SPECIAL1 0x1b
#define SPECIAL2 0x5b



/* ----------------------------------- Functions ----------------------------------- */

void terminalInterface(){
	while(1){
		alt_8 string[PUTTY_LINE_LENGTH];
		alt_32 stringlength = puttyGetline(string,PUTTY_LINE_LENGTH);

		if (stringlength < 0){
			puttyPrintLine("\n\rYou have entered too many characters for that command\n\r");
		}

		alt_8* array_of_words[50];
		alt_8 numwords = string_parser(string,array_of_words);
		alt_32 returncode = command_interpreter(numwords, array_of_words);

		if (returncode<0 && numwords > 0){
			puttyPrintLine("Command not found\n\r");
		}
	}
}

alt_32 command_interpreter(alt_32 argc, alt_8* argv[]){
	struct terminal_functions {
		char* command_string;
		alt_32 (*command_function)(alt_32 argc, alt_8* argv[]);
	} terminal_commands[] = {
		/* 
		Enter new terminal funtions below. 
		SYNTAX: {"terminalcommand", function_name} 
		*/
		{"echo",echo}, 
		{"add",add}, 
		{"ledr",ledr}, 
		{"switch",switch_function}, 
		{"mount",tf_mount},
		{"unmount",tf_unmount},
		{"ls",ls_path},
		{"cd",change_dir},
		{"mkdir",make_directory},
		{"rm",delete_file},
		{"touch",write_new_file},
		{"cp",copy_file},
		{"cat",read_file},
		{"wav",wav_play},
		{"playlist", playlist},
		{NULL,NULL} // This null function is to check we've read all the functions, new functions must go above it.
	};
	
	int i;
	/* loops through the terminal commands and compares to the input arg */
	for (i=0; terminal_commands[i].command_string != NULL; i++) {
		if (altstrcmp(terminal_commands[i].command_string, argv[0]) == 0){
			terminal_commands[i].command_function(argc, argv);
			return 0;
		}
	}
	return -1;
}


/*
USAGE: alt_8 string_parser(alt_8* string, alt_8* array_of_words[]);
GOTCHAS: the size of array_of_words should be malloc'd to be able to hold all the words, use at your own risk
SUGGESTION: use STRING_PARSER_MAXNUM_WORDS(string) to return the suggested size of array_of_words
*/
alt_8 string_parser(alt_8* string, alt_8* array_of_words[]){
	if (*(string)=='\0'){
		return 0;   // return 0 if empty string
	}
	alt_u8 count = 0;
	alt_32 string_length = altstrlen(string);
	
	if (*(string)!=' '){	// if the first character is not a space then start the first word there
		count++;
		array_of_words[0] = string;
	}
	alt_32 i;
	for (i=0; i<string_length; i++){	
		if (*(string+i)==' ' || *(string+i)=='\n' || *(string+i)=='\r'){
			*(string+i)='\0'; // replace spaces with nulls
			if(*(string+i+1)!='\0' && *(string+i+1)!=' ' && *(string+i)!='\n' && *(string+i)!='\r'){
				array_of_words[count]=string+i+1; // set pointer to the next word
				count++;
			}
		} else if (*(string+i)=='\0'){
			i=string_length;
		}
	}
	return count;  // returns the number of words found
}



void puttyPrintLine(const char* format,...){
	va_list vlist;
	va_start(vlist,format);
	char buf[PUTTY_LINE_LENGTH];
	vsprintf(buf,format,vlist);
	alt_32 uart_pointer = open("/dev/uart_0", O_WRONLY);
	write(uart_pointer, buf, strlen(buf));
	close(uart_pointer);
}

void puttyPrintChars(alt_8 string[], alt_32 length){
	alt_32 uart_pointer = open("/dev/uart_0", O_WRONLY);
	write(uart_pointer, string, length);
	close(uart_pointer);
}



alt_32 puttyGetline(alt_8 string[], alt_32 lineLength){
	alt_32 i=0;
	alt_32 uart_pointer = open("/dev/uart_0", O_RDWR);//, O_NONBLOCK);
	altmemset(string,'\0',lineLength);
	alt_8 charbuffer='\0';
	alt_8 enterPressed=0;
	
	do {
		read(uart_pointer,&charbuffer,1);
		
		/* Echo newlines correctly to putty */
		if (charbuffer=='\r'){
			charbuffer='\n';
			write(uart_pointer,&charbuffer,1);
			charbuffer='\r';
			write(uart_pointer,&charbuffer,1);
			enterPressed=1;
		} 
		
		/* Interpret backspaces correctly in the string by decrementing i */
		else if (charbuffer==BACKSPACE){
			if (i>0){
				write(uart_pointer,&charbuffer,1);
				i=i-1;
				string[i]='\0';
			}
		} 
		
		/* Interpret arrow keys */
		else if (charbuffer==SPECIAL1){
			//write(uart_pointer,&charbuffer,1);
			read(uart_pointer,&charbuffer,1);
			//write(uart_pointer,&charbuffer,1);
			if (charbuffer==SPECIAL2){
				read(uart_pointer,&charbuffer,1);
				switch(charbuffer){
					case LEFT:
						charbuffer=SPECIAL1;
						write(uart_pointer,&charbuffer,1);
						charbuffer=SPECIAL2;
						write(uart_pointer,&charbuffer,1);
						charbuffer=LEFT;
						write(uart_pointer,&charbuffer,1);
						if (i>0) i--;
						break;
					case RIGHT:
						if (string[i]!='\0'){
							charbuffer=SPECIAL1;
							write(uart_pointer,&charbuffer,1);
							charbuffer=SPECIAL2;
							write(uart_pointer,&charbuffer,1);
							charbuffer=RIGHT;
							write(uart_pointer,&charbuffer,1);
							i++;
						}
						break;
					default:
						break;
				}
			}
		}

		/* Echo everything else */
		else{
			//printf("%x\n",string[i]);
			write(uart_pointer,&charbuffer,1);
			string[i] = charbuffer;
			i++;
		}
		//printf("i is %d\nstring is %s\n",i,string);

	} while(!enterPressed && i<lineLength); // loop until newline or line length is reached
	if (i>=lineLength){
		string[0]='\0';
		return -1;
	}
	printf("> %s\n",string);
	close(uart_pointer);
	return i;
}


