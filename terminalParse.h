
#ifndef TERMINALPARSE_H
#define TERMINALPARSE_H

/* Includes */
#include "alt_types.h"

/* Function prototypes */
void terminalInterface();
alt_32 command_interpreter(alt_32 argc, alt_8* argv[]);
alt_8 string_parser(alt_8* string, alt_8* array_of_words[]);

#endif

