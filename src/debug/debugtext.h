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
