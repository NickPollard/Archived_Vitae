// debugdraw.h
#ifndef __DEBUGDRAW__
#define __DEBUGDRAW__

#include "src/maths/maths.h"

// Draw a debug cross at the point *center*
void debugdraw_cross( const vector* center, float radius );

void debugdraw_drawRect2D( vector* from, vector* to );

void debugdraw_line2d( vector from, vector to, vector color );

void debugdraw_preTick( float dt );

#endif // __DEBUGDRAW__
