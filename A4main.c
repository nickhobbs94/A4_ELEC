/*
	This is where the OS functions are made and run by main
*/


/* Includes */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "alt_types.h"
#include "threading.h"
#include "includes.h"

#include "terminalParse.h"
#include "wavPlay.h"
/* ----------------------------------- Functions ----------------------------------- */

void task1(void* pdata) {
	terminalInterface(); // infinite loop over the terminal
}



void task2(void* pdata) {
	/*static int count = 0;
	unsigned char err;
	unsigned short timeout=0;
	POST mailBox;
	while(1){
		mailBox = *(POST*)OSMboxPend(Mbox1, timeout, &err);
		printf("task2 :) %d %s\n", count, mailBox.filename);
		OSTimeDly(1);
		count++;
	}*/

	audioController();
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

