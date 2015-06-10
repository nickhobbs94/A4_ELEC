#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H

#include "alt_types.h"
//-------------------------------------------------------------------------
void  LCD_Init();
void  LCD_Show_Text(alt_8* Text);
void  LCD_Line2();
void  LCD_Show_Decimal(alt_32 input);

#endif
