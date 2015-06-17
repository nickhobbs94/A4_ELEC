#ifndef THREADING_H
#define THREADING_H

#include "includes.h"
#include "wavPlay.h"


void task1(void* pdata);
void task2(void* pdata);
void task3(void* pdata);


/* Definition of Task Stacks */

#define   TASK_STACKSIZE       8192

/* Definition of Task Priorities */

#define TASK1_PRIORITY      7
#define TASK2_PRIORITY      6
#define TASK3_PRIORITY      5

OS_EVENT* Mbox1;
OS_EVENT* Mbox2;

OS_MEM* Memory_Partition;


typedef struct Mail {
	alt_8 filename[13];
	Wave_Header header_data;
} POST;

OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];
OS_STK task3_stk[TASK_STACKSIZE];

#endif

