// Window management library

#include <gtk/gtk.h>

typedef struct {
	GtkWindow* GTKwindow;
	int width;
	int height;
} window;

window* window_create(int w, int h);

void window_show(window* w);

void window_on_exit(GtkWindow* w, gpointer data);
