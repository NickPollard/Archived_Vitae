// light.h
#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "common.fwd.h"
#include "maths.h"

// *** Light ***
struct light_s {
	transform*	trans;
	color		col;
};

light* light_createTestLight();
#endif // __LIGHT_H__
