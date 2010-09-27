// Canvas

#include "common.h"
#include "canvas.h"

// Creates a new blank Canvas
canvas* canvas_create() {
	return (canvas*)malloc(sizeof(canvas));
}

