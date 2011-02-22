// font.h

#pragma once

// Test init function, based on the main func in the complete program implementation of stb TrueType ((C) Sean Barrett 2009)
void font_init();

// Load a TrueTypeFont into the font engine
// This allocates texture(s) for the glyphs
// And computes the glyphs
void vfont_loadTTF( String path );

void vfont_bindGlyph( const char c );

void font_renderGlyph( float x, float y, const char c );

void font_renderString( float x, float y, String string );
