// Window management library

#include "common.h"
#include "window.h"

// window_create - creates a new window, and allocates memory for it
// TAKES width and height of the window, as ints
window* window_create(int w, int h) {
	// Allocate memory
	window* win = malloc(sizeof(window));

	// Set params
	win->width = w;
	win->height = h;

	// Create GTK window
	win->GTKwindow = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);

	// Set up event handlers
	g_signal_connect(win->GTKwindow, "destroy", G_CALLBACK(window_on_exit), NULL);

	return win;
}

// window_show - makes a window visible
// TAKES a pointer to the window
void window_show(window* w) {
	gtk_widget_show_all((GtkWidget*)w->GTKwindow);
}

void window_on_exit(GtkWindow* w, gpointer data) {
	gtk_main_quit();
}
