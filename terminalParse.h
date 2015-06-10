
#ifndef TERMINALPARSE_H
#define TERMINALPARSE_H

#include <stdarg.h>
#include "alt_types.h"

#define PUTTY_LINE_LENGTH 150   // max line length able to be entered into putty

/* Includes */
#include "alt_types.h"

/* Function prototypes */
void terminalInterface();
alt_32 command_interpreter(alt_32 argc, alt_8* argv[]);
alt_8 string_parser(alt_8* string, alt_8* array_of_words[]);



void puttyPrintLine(const char* format,...);
void puttyPrintChars(alt_8 string[], alt_32 length);
alt_32 puttyGetline(alt_8 string[], alt_32 lineLength);

#endif

