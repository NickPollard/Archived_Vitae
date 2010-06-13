#include "common.h"
#include "sprite.h"

void sprite_render_to_canvas(sprite* s, const canvas* c) {
	// Draw to the DrawingArea of the canvas
	GdkGC* gc = gdk_gc_new(((GtkWidget*)c->area)->window);
	gdk_draw_pixbuf((GdkDrawable*)c->area, gc, s->image, 0, 0, s->x, s->y, s->width, s->height, GDK_RGB_DITHER_NONE, 0, 0);
}

sprite* sprite_create_from_bitmap(const char* bitmapName) {
	sprite* s = malloc(sizeof(sprite));
	// Load the pixbuf
	GError* error = NULL;
	GdkPixbuf* pix = gdk_pixbuf_new_from_file(bitmapName, &error);
	if (!pix) {
		fprintf(stderr, "Error trying to load pixbuf: %s\n", bitmapName);
	}
	s->width = gdk_pixbuf_get_height(pix);
	s->height = gdk_pixbuf_get_width(pix);
	return s;
}

void sprite_set_x_y(sprite* s, int x, int y) {
	s->x = x;
	s->y = y;
}	
