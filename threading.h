#ifndef THREADING_H
#define THREADING_H

#include "includes.h"

OS_EVENT* Mbox1;
OS_EVENT* Mbox2;

#define STRING_LEN 4


typedef struct {
	alt_8 filename[12];
	Wave_Header header_data;
} POST;

#endif

