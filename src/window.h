// Window management library
#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtk.h>

#include "canvas.h"
#include "spritebuffer.h"

typedef struct {
	GtkWindow* GTKwindow;
	int width;
	int height;
	canvas* canv;
	spritebuffer* buffer;
} window;

typedef struct {
	spritebuffer* buffer;
	canvas* canv;
} renderData;

// window_create - creates a new window, and allocates memory for it
// TAKES width and height of the window, as ints
window* window_create(int w, int h);


// window_show - makes a window visible
// TAKES a pointer to the window
void window_show(window* w);


// window_on_exit - event handler for when a window is destroyed
// TAKES a pointer to the window, custom data
void window_on_exit(GtkWindow* w, gpointer data);

void area_on_draw(GtkWidget* w, GdkEventExpose* e, gpointer data);

renderData* renderData_create(spritebuffer* s, canvas* c);

void tick_window_tick(void* t, float dt);

#endif // __WINDOW_H__
