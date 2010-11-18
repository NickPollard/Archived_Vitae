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
	sprite* s = mem_alloc(sizeof(sprite));
	s->image = texture_load_by_id(lib, bitmapName);
	s->width = gdk_pixbuf_get_height(s->image);
	s->height = gdk_pixbuf_get_width(s->image);
	return s;
}

void sprite_set_x_y(sprite* s, int x, int y) {
	s->x = x;
	s->y = y;
}	

//
// SpriteMover
//

spritemover* spritemover_create(sprite* spr, float x, float y, float vx, float vy) {
	spritemover* s = (spritemover*)mem_alloc(sizeof(spritemover));
	s->s = spr;
	s->position.x = x;
	s->position.y = y;
	s->velocity.x = vx;
	s->velocity.y = vy;
	return s;
}

void spritemover_tick(void* mover, float dt) {
	spritemover* s = (spritemover*)mover;
	s->position.x += s->velocity.x * dt;
	s->position.y += s->velocity.y * dt;
	s->s->x = (int)s->position.x;
	s->s->y = (int)s->position.y;
//	printf("spritemover tick! At (%.2f, %.2f), moving (%.2f, %.2f)\n", s->position.x, s->position.y, s->velocity.x, s->velocity.y);
}
