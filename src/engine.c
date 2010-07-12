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

static unsigned long long oldTime = 0;

float frame_delta() {
	struct timeval time;
	gettimeofday(&time, NULL);

	unsigned long long newTime = time.tv_sec * 1000000 + time.tv_usec;
	float delta = (float)(newTime - oldTime);
	oldTime = newTime;
	return delta * 0.000001;
}

void test_load_and_add_sprite(TextureLibrary* textures, spritebuffer* b, const char* file, int x, int y) {
	sprite* s = sprite_create_from_bitmap(textures, file);
	spritebuffer_add_sprite(b, s);
	sprite_set_x_y(s, x, y);
}

// tick - process a frame of game update
int engine_tick() {
	float dt = frame_delta();

	printf("Tick! %.8f\n", dt);

	// tick all tickers
	tick_all(sprite_movers, dt);
	tick_all(ticking_widgets, dt);

	return true;
}


// init - initialises the engine
void init(int argc, char** argv) {
#ifdef __GTK__
	gtk_init(&argc, &argv);
	gtk_idle_add((GtkFunction)engine_tick, NULL);
#endif

	// Init time
	struct timeval time;
	gettimeofday(&time, NULL);
	oldTime = time.tv_sec * 1000000 + time.tv_usec;
}

// run - executes the main loop of the engine
void run(window* rootWindow) {
	TextureLibrary* textures = texture_library_create();
	// Sprite test
	sprite* s = sprite_create_from_bitmap(textures, "assets/img/test64.png");
	spritebuffer_add_sprite(rootWindow->buffer, s);
	sprite_set_x_y(s, 64, 64);

	test_load_and_add_sprite(textures, rootWindow->buffer, "assets/img/test64.png", 0, 128);
	test_load_and_add_sprite(textures, rootWindow->buffer, "assets/img/test64.png", 300, 300);

	/*
	test_tickers = ticklist_create(tick_tester_tick, 8);
	tick_tester tickerA = { 0, 1 };
	tick_tester tickerB = { 0, 2 };
	tick_tester tickerC = { 0, 5 };
	ticklist_add(test_tickers, &tickerA);
	ticklist_add(test_tickers, &tickerB);
	ticklist_add(test_tickers, &tickerC);
	*/
	sprite_movers = ticklist_create(spritemover_tick, 1);
	spritemover* sm = spritemover_create(s, 0.f, 0.f, 10.f, 10.f);
	ticklist_add(sprite_movers, sm);

#define MAX_TICKING_WIDGETS 16
	ticking_widgets = ticklist_create(tick_window_tick, MAX_TICKING_WIDGETS);
	ticklist_add(ticking_widgets, (GtkWidget*)rootWindow->GTKwindow);

#ifdef __GTK__
	gtk_main();
#endif
}
