// file.c
#include "common.h"
#include "file.h"
//-----------------------
#include <assert.h>

FILE* vfile_open( const char* path, const char* mode ) {
   // *** load the ttf file
   FILE* file = fopen( path, mode );
   if ( !file ) {
	   printf( "Error loading file: \"%s\"\n", path );
	   assert( file );
   }
   return file;
}
