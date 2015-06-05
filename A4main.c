/*
	This is where the main function for every question for assessment 4 will be. Along with some general functions
	that will be used to communicate via putty.
*/


/* Includes */
//#include "home.h"
#include "uni.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "terminalParse.h"
#include "alt_types.h"
#include "SD_functions.h"
#include "threading.h"
#include "includes.h"


/* ----------------------------------- Functions ----------------------------------- */

void task1(void* pdata) {
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


void task2(void* pdata) {
	static int count = 0;
	unsigned char err;
	unsigned short timeout=0;
	POST mailBox;
	while(1){
		/*mailBox = *(POST*)OSMboxPend(Mbox1, timeout, &err);
		printf("task2 :) %d %s\n", count, mailBox.filename);*/
		OSTimeDly(1);
		count++;
	}
}



/* Definition of Task Stacks */

#define   TASK_STACKSIZE       2048

/* Definition of Task Priorities */

#define TASK1_PRIORITY      2
#define TASK2_PRIORITY      1

/* The main function creates two task and starts multi-tasking */

int main(void)
{
	OSInit();
	Mbox1 = OSMboxCreate((void*) 0);
	Mbox2 = OSMboxCreate((void*) 0);
	OS_STK task1_stk[TASK_STACKSIZE];
	OS_STK task2_stk[TASK_STACKSIZE];

	OSTaskCreateExt(task1,
					NULL,
					(void *)&task1_stk[TASK_STACKSIZE-1],
					TASK1_PRIORITY,
					TASK1_PRIORITY,
					task1_stk,
					TASK_STACKSIZE,
					NULL,
					0);

	OSTaskCreateExt(task2,
					NULL,
					(void *)&task2_stk[TASK_STACKSIZE-1],
					TASK2_PRIORITY,
					TASK2_PRIORITY,
					task2_stk,
					TASK_STACKSIZE,
					NULL,
					0);

	OSStart();
	return 0;
}

