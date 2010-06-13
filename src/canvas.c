// Canvas

#include "common.h"
#include "canvas.h"

canvas* canvas_create() {
	return (canvas*)malloc(sizeof(canvas));
}
