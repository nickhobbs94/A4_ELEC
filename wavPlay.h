#ifndef WAVPLAY_H
#define WAVPLAY_H

#include "alt_types.h"


Wave_Header check_header (alt_8 filename[], alt_u8 *err);


alt_32 load_fifo (alt_u8 init_flag);

#endif
