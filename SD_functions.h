#ifndef SD_FUNCTIONS_H
#define SD_FUNCTIONS_H

/* Includes */
#include "alt_types.h"
#include "efs.h"

#define UNMOUNT_SD_AFTER_OPERATION 0
#define SD_MAX_PATH_LENGTH 200

/* Function prototypes */
EmbeddedFileSystem** SD_mount();
void SD_unmount(void);
alt_8 SD_getFileAttribute(alt_u8 input);
alt_8* SD_getCurrentPath();
void SD_updatePath(alt_8* currentPath, alt_8 argument[]);

#endif

