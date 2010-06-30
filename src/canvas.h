#ifndef __CANVAS_H__
#define __CANVAS_H__

// GTK headers
#include <gtk/gtk.h>

typedef struct {
 GtkDrawingArea* area;
} canvas;

canvas* canvas_create();

inline GdkWindow* canvas_get_gdkwindow(const canvas* c) {
	return ((GtkWidget*)c->area)->window;
}
#endif // __CANVAS_H__
