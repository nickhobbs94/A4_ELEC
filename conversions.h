#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "alt_types.h"

alt_32 intfromstring(alt_8 string[]);
alt_8 charfromint(alt_32 number);
void decimaltobinary(alt_32* binaryArray, alt_32 decimal);
void decimaltohex(alt_32* hexArray, alt_32 decimal);
alt_32 extract_little(alt_u8* buffer, alt_32 size);

#endif
