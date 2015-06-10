#ifndef THREADING_H
#define THREADING_H

#include "includes.h"
#include "wavPlay.h"

OS_EVENT* Mbox1;
OS_EVENT* Mbox2;

OS_MEM* Memory_Partition;


typedef struct {
	alt_8 filename[12];
	Wave_Header header_data;
} POST;

#endif

