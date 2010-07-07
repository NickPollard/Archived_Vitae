// engine.c
#include "src/common.h"
#include "src/engine.h"
#include "src/ticker.h"

// Use GTK library
#define __GTK__

ticklist* test_tickers = NULL;

float frame_delta() {
	static unsigned long long clocktime = 0;
	unsigned long long oldtime = clocktime;
	clocktime = rdtsc();

	#define invClockPerSecond 0.000000001f
	return (float)(clocktime - oldtime) * invClockPerSecond;
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
	tick_all(test_tickers, dt);

	return true;
}


// init - initialises the engine
void init(int argc, char** argv) {
#ifdef __GTK__
	gtk_init(&argc, &argv);
	gtk_idle_add((GtkFunction)engine_tick, NULL);
#endif
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

	test_tickers = ticklist_create(tick_tester_tick, 8);
	tick_tester tickerA = { 0, 1 };
	tick_tester tickerB = { 0, 2 };
	tick_tester tickerC = { 0, 5 };
	ticklist_add(test_tickers, &tickerA);
	ticklist_add(test_tickers, &tickerB);
	ticklist_add(test_tickers, &tickerC);

#ifdef __GTK__
	gtk_main();
#endif
}
