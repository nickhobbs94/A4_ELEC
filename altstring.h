/*
altstring.h is an alt_types version of string.h
*/

#ifndef ALTSTRING_H
#define ALTSTRING_H

/* Includes */
#include "alt_types.h"

/* Function prototypes */
alt_32 altstrlen(alt_8* string);
alt_8* altstrcpy(alt_8* destination, alt_8* source);
alt_32 altstrcmp(alt_8* string1, alt_8* string2);
alt_8* altstrcat(alt_8* destination, alt_8* source);
void altmemset(alt_8* destination, alt_8 value, alt_32 size);
alt_32 string_replace(alt_8 string[], alt_8 find, alt_8 replace, alt_32 limit, alt_32 step);
alt_32 altstrcount(alt_8 string[], alt_8 find);
alt_8 altsplitstring(alt_8* string, alt_8* array_of_words[], alt_8 character);
alt_32 charsCompare(alt_8* string1, alt_8* string2, alt_32 length);


void uppercasestring(alt_8* lowerCaseString);
alt_32 formatStringForFAT(alt_8* instring, alt_8* outstring);

#endif

