
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "alt_types.h"
#include "threading.h"
#include "includes.h"

#include "terminalParse.h"
#include "wavPlay.h"
/* ----------------------------------- Functions ----------------------------------- */


/* The main function creates two task and starts multi-tasking */

int main(){
	OSInit();
	Mbox1 = OSMboxCreate((void*) 0);
	Mbox2 = OSMboxCreate((void*) 0);


	OSTaskCreateExt(task1,
					NULL,
					(void *)&task1_stk[TASK_STACKSIZE-1],
					TASK1_PRIORITY,
					TASK1_PRIORITY,
					task1_stk,
					TASK_STACKSIZE,
					NULL,
					0);

	playStatus.pause = 0;
	playStatus.shuffle = 0;
	playStatus.repeat = 0;
	playStatus.volume = 31;
	OSStart();
	return 0;
}

void task1(void* pdata){
	terminalInterface(); // infinite loop over the terminal
}

void task2(void* pdata){
	audioController(pdata);
	OSTaskDel(OS_PRIO_SELF);
}

void task3(void* pdata){
	playlist_manager(pdata);
	OSTaskDel(OS_PRIO_SELF);
}
