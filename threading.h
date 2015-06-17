#ifndef THREADING_H
#define THREADING_H

#include "includes.h"
#include "wavPlay.h"


void task1(void* pdata);
void task2(void* pdata);
<<<<<<< HEAD
void task3(void* pdata);
=======
>>>>>>> Roxy


/* Definition of Task Stacks */

#define   TASK_STACKSIZE       8192

/* Definition of Task Priorities */

<<<<<<< HEAD
#define TASK1_PRIORITY      7
#define TASK2_PRIORITY      6
#define TASK3_PRIORITY      5
=======
#define TASK1_PRIORITY      6
#define TASK2_PRIORITY      5
>>>>>>> Roxy

OS_EVENT* Mbox1;
OS_EVENT* Mbox2;

OS_MEM* Memory_Partition;


typedef struct Mail {
	alt_8 filename[13];
	Wave_Header header_data;
} POST;

OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];
<<<<<<< HEAD
OS_STK task3_stk[TASK_STACKSIZE];

struct {
	unsigned char pause :1;
	unsigned char repeat :1;
	unsigned char shuffle :1;
	unsigned char volume :5;
} playStatus;
=======
>>>>>>> Roxy

#endif

