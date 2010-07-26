// engine.c
#include "src/common.h"
#include "src/engine.h"
#include "src/ticker.h"
#include "src/window.h"
#include <sys/time.h>

// Use GTK library
#define __GTK__

ticklist* test_tickers = NULL;
ticklist* sprite_movers = NULL;
ticklist* ticking_widgets = NULL;

void test_load_and_add_sprite(TextureLibrary* textures, spritebuffer* b, const char* file, int x, int y) {
	sprite* s = sprite_create_from_bitmap(textures, file);
	spritebuffer_add_sprite(b, s);
	sprite_set_x_y(s, x, y);
}

// tick - process a frame of game update
int engine_tick(engine* e) {
	float dt = frame_timer_delta(e->timer);

	// tick all tickers
	tick_all(sprite_movers, dt);
	tick_all(ticking_widgets, dt);

	return true;
}


// init - initialises the engine
void init(int argc, char** argv) {
	engine* e = (engine*)malloc(sizeof(engine));
#ifdef __GTK__
	gtk_init(&argc, &argv);
	gtk_idle_add((GtkFunction)engine_tick, e);
#endif

	e->timer = (frame_timer*)malloc(sizeof(frame_timer));
	frame_timer_init(e->timer);
}

void spritemover_set_active(spritemover* m) {
	ticklist_add(sprite_movers, m);
}

// run - executes the main loop of the engine
void run(window* rootWindow) {
	TextureLibrary* textures = texture_library_create();
	// Sprite test
	sprite* s = sprite_create_from_bitmap(textures, "assets/img/test64.png");
	sprite_set_x_y(s, 64, 64);
	spritebuffer_add_sprite(rootWindow->buffer, s);

	test_load_and_add_sprite(textures, rootWindow->buffer, "assets/img/test64.png", 0, 128);
	test_load_and_add_sprite(textures, rootWindow->buffer, "assets/img/test64.png", 300, 300);

	sprite_movers = ticklist_create(spritemover_tick, 1);
	spritemover* sm = spritemover_create(s, 0.f, 0.f, 10.f, 10.f);
	spritemover_set_active(sm);

#define MAX_TICKING_WIDGETS 16
	ticking_widgets = ticklist_create(tick_window_tick, MAX_TICKING_WIDGETS);
	ticklist_add(ticking_widgets, (GtkWidget*)rootWindow->GTKwindow);

#ifdef __GTK__
	gtk_main();
#endif
}

