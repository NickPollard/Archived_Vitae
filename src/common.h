// Standard C libraries
#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "common.fwd.h"

#include "debug/debug.h"
#include "profile.h"

// For sleep
#include <unistd.h>

// For openGL compatibility
#define GL_GLEXT_PROTOTYPES

// Boolean defines
#define true 1
#define false 0

#define TERM_RED "[1;31;40m"
#define TERM_GREEN "[1;32;40m"
#define TERM_WHITE "[0;37;40m"

#define then ?
#define otherwise :

#ifdef ANDROID

#define OPENGL_ES

// *** Logging
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vitae", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "vitae", __VA_ARGS__))

#define printf( ... )  LOGI( __VA_ARGS__ )

#endif

#define vAssert( a )	if ( !(a) ) { \
							printf( "Assert Failed: " #a "(%s:%d)\n", __FILE__, __LINE__ ); \
							assert( (a) ); \
						}


// types
//typedef unsigned int uint;
typedef unsigned int u32;
typedef unsigned char uchar;
typedef unsigned char ubyte;
typedef unsigned char u8;
typedef const char* String;

// *** Asset & Resource Loading

// Printing
#define printError( format, args... ) 	{ \
											printf( "%sError%s: ", TERM_RED, TERM_WHITE ); \
											printf( format, args ); \
											printf( " [File: %s, Line: %d]\n", __FILE__, __LINE__ ); \
										}
