// font.h

#pragma once

void test_font_renderChar( char* c );

// Test init function, based on the main func in the complete program implementation of stb TrueType ((C) Sean Barrett 2009)
void font_init();

void font_bindGlyph( const char c );

void font_renderGlyph( float x, float y, const char c );

void font_renderString( float x, float y, const char* string );
