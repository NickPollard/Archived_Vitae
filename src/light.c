// light.c

#include "common.h"
#include "light.h"
//---------------------
#include "mem/allocator.h"

light* light_createTestLight() {
	light* l = mem_alloc(sizeof(light));
	return l;
}
