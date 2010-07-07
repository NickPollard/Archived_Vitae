// Canvas

#include "common.h"
#include "canvas.h"

// Creates a new blank Canvas
canvas* canvas_create() {
	return (canvas*)malloc(sizeof(canvas));
}

// Get the GdkWindow associated with a given canvas
// This is used to draw upon, eg. for gdk_draw_pixbuf()
GdkWindow* canvas_get_gdkwindow(const canvas* c) {
	return ((GtkWidget*)c->area)->window;
}
