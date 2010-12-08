//
// External File - taken from Jckarter's open-gl hello world
// https://github.com/jckarter/hello-gl
//
// All code copyright Jckarter. Used without permission
//

// Added include guards
#ifndef __UTIL_H__
#define __UTIL_H__

void *file_contents(const char *filename, GLint *length);
void *read_tga(const char *filename, int *width, int *height);

#endif // __UTIL_H__
