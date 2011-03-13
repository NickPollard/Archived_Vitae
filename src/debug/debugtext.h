// debugtext.h

#define kDebugTextMaxLines 64
#define kDebugTextLineLength 256

typedef struct debugtextframe_s {
	float x;
	float y;
	float lineHeight;
	u8 lineCount;
	char lines[kDebugTextMaxLines][kDebugTextLineLength];
} debugtextframe;

debugtextframe* debugtextframe_create( float x, float y, float lineHeight);

void debugtextframe_tick( void* f, float dt );

void debugtextframe_render( debugtextframe* f );
