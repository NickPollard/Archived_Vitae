// Window management library
#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtk.h>

typedef struct {
	GtkWindow* GTKwindow;
	int width;
	int height;
} window;

// window_create - creates a new window, and allocates memory for it
// TAKES width and height of the window, as ints
window* window_create(int w, int h);


// window_show - makes a window visible
// TAKES a pointer to the window
void window_show(window* w);


// window_on_exit - event handler for when a window is destroyed
// TAKES a pointer to the window, custom data
void window_on_exit(GtkWindow* w, gpointer data);

#endif // __WINDOW_H__
