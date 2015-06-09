/**
 * terminalParse contains the main infinite loop for the terminal 
 * it also includes functions to interpret the commands given and pass them
 * to terminalFunctions
 */
#ifndef TERMINALPARSE_H
#define TERMINALPARSE_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include "altstring.h"
#include "terminalFunctions.h"
#include "alt_types.h"
#include "terminalParse.h"

/* Magic numbers */
#define STRING_PARSER_MAXNUM_WORDS(string) string/2+1


/* ----------------------------------- Functions ----------------------------------- */

void terminalInterface(){
	while(1){
		alt_8 string[PUTTY_LINE_LENGTH];
		alt_32 stringlength = puttyGetline(string,PUTTY_LINE_LENGTH);

		if (stringlength < 0){
			puttyPrintLine("\n\rYou have entered too many characters for that command\n\r");
		}

		alt_8** array_of_words;
		array_of_words = malloc(STRING_PARSER_MAXNUM_WORDS(stringlength));
		alt_8 numwords = string_parser(string,array_of_words);
		alt_32 returncode = command_interpreter(numwords, array_of_words);

		if (returncode<0 && numwords > 0){
			puttyPrintLine("Command not found\n\r");
		}

		free(array_of_words);
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

#endif

