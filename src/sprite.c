#include "common.h"
#include "sprite.h"

void sprite_render_to_canvas(sprite* s, canvas* c) {
	// Draw to the DrawingArea of the canvas
	GdkWindow* w = canvas_get_gdkwindow(c);
	GdkGC* gc = gdk_gc_new(w);
	gdk_draw_pixbuf(w, 
			gc, 
			s->image, 
			0, 0, 
			s->x, s->y, 
			s->width, s->height, 
			GDK_RGB_DITHER_NONE, 
			0, 0
			);
}

sprite* sprite_create_from_bitmap(TextureLibrary* lib, const char* bitmapName) {
	sprite* s = malloc(sizeof(sprite));
	s->image = texture_load_by_id(lib, bitmapName);
	s->width = gdk_pixbuf_get_height(s->image);
	s->height = gdk_pixbuf_get_width(s->image);
	return s;
}

void sprite_set_x_y(sprite* s, int x, int y) {
	s->x = x;
	s->y = y;
}	
