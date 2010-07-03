#ifndef __CANVAS_H__
#define __CANVAS_H__

// GTK headers
#include <gtk/gtk.h>

typedef struct {
 GtkDrawingArea* area;
} canvas;

canvas* canvas_create();

GdkWindow* canvas_get_gdkwindow(const canvas* c);
#endif // __CANVAS_H__
