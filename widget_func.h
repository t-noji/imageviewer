#include <gtk/gtk.h>
double get_par_scale ();
double get_window_fix_scale (GdkPixbuf *pixbuf);
double set_window_size_from_pixbuf (GdkPixbuf *pixbuf);
void set_title (GdkPixbuf *image_pixbuf);
void re_draw ();
void fullscreen();
void h_inc_scroll();
void v_inc_scroll();
void scroll_up();
void scroll_down();
void scroll_left();
void scroll_right();
GtkWidget* init_builder (const char* path);
void load_css (const char* css_file);
