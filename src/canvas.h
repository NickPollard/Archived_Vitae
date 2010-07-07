#ifndef __CANVAS_H__
#define __CANVAS_H__

// GTK headers
#include <gtk/gtk.h>

typedef struct {
 GtkDrawingArea* area;
} canvas;

// Creates a new blank Canvas
canvas* canvas_create();

// Get the GdkWindow associated with a given canvas
// This is used to draw upon, eg. for gdk_draw_pixbuf()
GdkWindow* canvas_get_gdkwindow(const canvas* c);
#endif // __CANVAS_H__
