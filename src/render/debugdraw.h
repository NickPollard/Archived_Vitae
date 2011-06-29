// debugdraw.h
#ifndef __DEBUGDRAW__
#define __DEBUGDRAW__

#include "src/maths.h"

// Draw a debug cross at the point *center*
void debugdraw_cross( const vector* center, float radius );

void debugdraw_drawRect2D( vector* from, vector* to );

#endif // __DEBUGDRAW__
