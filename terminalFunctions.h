#ifndef TERMINALFUNCTIONS_H
#define TERMINALFUNCTIONS_H

#include "alt_types.h"

/* Function prototypes */
alt_32 echo(alt_32 argc, alt_8* argv[]);
alt_32 add(alt_32 argc, alt_8* argv[]);
alt_32 ledr(alt_32 argc, alt_8* argv[]);
alt_32 switch_function(alt_32 argc, alt_8* argv[]);
alt_32 ls_path(alt_32 argc, alt_8* argv[]);
alt_32 change_dir(alt_32 argc, alt_8* argv[]);
alt_32 tf_mount(alt_32 argc, alt_8* argv[]);
alt_32 tf_unmount(alt_32 argc, alt_8* argv[]);
alt_32 make_directory(alt_32 argc, alt_8* argv[]);
alt_32 delete_file(alt_32 argc, alt_8* argv[]);
alt_32 write_new_file(alt_32 argc, alt_8* argv[]);
alt_32 read_file(alt_32 argc, alt_8* argv[]);
alt_32 copy_file(alt_32 argc, alt_8* argv[]);
alt_32 wav_play(alt_32 argc, alt_8* argv[]);
alt_32 playlist(alt_32 argc, alt_8* argv[]);
alt_32 tf_pause(alt_32 argc, alt_8* argv[]);
alt_32 play(alt_32 argc, alt_8* argv[]);
alt_32 repeat(alt_32 argc, alt_8* argv[]);
alt_32 norepeat(alt_32 argc, alt_8* argv[]);
alt_32 volume(alt_32 argc, alt_8* argv[]);
alt_32 stop(alt_32 argc, alt_8* argv[]);

#endif

