// debugtext.c

#include "common.h"
#include "debugtext.h"
//-----------------------
#include "font.h"

void PrintDebugText( debugtextframe* frame, const char* string ) {
	assert( strlen(string) < kDebugTextLineLength );
	strncpy( frame->lines[frame->lineCount], string, kDebugTextLineLength );
	frame->lines[frame->lineCount][kDebugTextLineLength-1] = '\0'; // Ensure null-termination
	frame->lineCount++;
}

debugtextframe* debugtextframe_create( float x, float y, float lineHeight) {
	debugtextframe* f = malloc( sizeof( debugtextframe ));
	memset( f, 0, sizeof( debugtextframe ));
	f->x = x;
	f->y = y;
	f->lineHeight = lineHeight;
	return f;
}

void debugtextframe_tick( debugtextframe* f, float dt ) {
	f->lineCount = 0;
}

void debugtextframe_render( debugtextframe* f ) {
	printf( "debugtext frame render!\n" );
	float x = f->x;
	float y = f->y;
	for ( int i = 0; i < f->lineCount; i++) {
		font_renderString( x, y, f->lines[i] );
		y += f->lineHeight;
	}
}
