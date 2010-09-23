// light.c

#include "common.h"
#include "light.h"

light* light_createTestLight() {
	light* l = malloc(sizeof(light));
	return l;
}
