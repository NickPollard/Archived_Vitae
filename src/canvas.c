// Canvas

#include "common.h"
#include "canvas.h"

canvas* canvas_create() {
	return (canvas*)malloc(sizeof(canvas));
}

GdkWindow* canvas_get_gdkwindow(const canvas* c) {
	return ((GtkWidget*)c->area)->window;
}
