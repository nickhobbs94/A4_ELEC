#ifndef PLAYLIST_FUNCTIONS_H
#define PLAYLIST_FUNCTIONS_H

#include "alt_types.h"

alt_32 playlist_print(alt_32 argc, alt_8* argv[]);
alt_32 playlist_add(alt_32 argc, alt_8* argv[]);
alt_32 playlist_new(alt_32 argc, alt_8* argv[]);
alt_32 playlist_delete(alt_32 argc, alt_8* argv[]);
alt_32 playlist_remove(alt_32 argc, alt_8* argv[]);
alt_32 playlist_help(alt_32 argc, alt_8* argv[]);
alt_32 playlist_play(alt_32 argc, alt_8* argv[]);

#define MAX_SIZE_PLAYLIST_FILE 2000

#endif
