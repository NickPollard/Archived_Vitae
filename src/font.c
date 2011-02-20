// font.c

#include "common.h"
#include "font.h"

// *** The stb TrueType library
#define STB_TRUETYPE_IMPLEMENTATION
#include  "3rdparty/stbTrueType/stb_truetype.h"

unsigned char ttf_buffer[1<<25];

void test_font_renderChar( char* c ) {
	
}

#define FONT_PATH "/usr/share/fonts/truetype/ttf-droid/DroidSans.ttf"

// Test init function, based on the main func in the complete program implementation of stb TrueType ((C) Sean Barrett 2009)
void font_init() {
   stbtt_fontinfo font;
   unsigned char *bitmap;
//   int w,h,i,j,c = (argc > 1 ? atoi(argv[1]) : 'a'), s = (argc > 2 ? atoi(argv[2]) : 20);
   int w,h,c,s;
   c = 'a';
   s = 20;

   // *** load the ttf file
   int ret = fread(ttf_buffer, 1, 1<<25, fopen(FONT_PATH, "rb"));
   (void)ret;

   // *** init the tt system and extract font data
   stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

   for (unsigned char ch = 0; ch < 255; ch++) {
	   // ** extract a bitmap
	   bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), ch, &w, &h, 0,0);

	   // *** display the bitmap
	   /*
		int i, j;
	   for (j=0; j < h; ++j) {
		   for (i=0; i < w; ++i)
			   putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
		   putchar('\n');
	   }
	   */
   }
}
