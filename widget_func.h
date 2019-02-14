#include <gtk/gtk.h>
double get_par_scale ();
double get_window_fix_scale (GdkPixbuf *pixbuf);
double set_window_size_from_pixbuf (GdkPixbuf *pixbuf);
void set_title (GdkPixbuf *image_pixbuf);
void re_draw ();
GtkWidget* init_builder (const char* path);
void load_css (const char* css_file);
