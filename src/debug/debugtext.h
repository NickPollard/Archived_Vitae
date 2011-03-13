// debugtext.h

#define kDebugTextMaxLines 64
#define kDebugTextLineLength 256

struct debugtextframe_s {
	float x;
	float y;
	float lineHeight;
	u8 lineCount;
	char lines[kDebugTextMaxLines][kDebugTextLineLength];
};

void PrintDebugText( debugtextframe* frame, const char* string );

debugtextframe* debugtextframe_create( float x, float y, float lineHeight);

void debugtextframe_tick( void* f, float dt );

void debugtextframe_render( void* entity );
