#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

GtkWidget *window;
GtkWidget *scroll_window;
GtkWidget *d_area;

GdkMonitor* get_gdk_monitor () {
  GdkDisplay *display = gdk_display_get_default();
  return gdk_display_get_monitor(display, 0);
  // 本来 at_windowで取得したいがGDK-CRITICALが出る為断念 //
}
double get_par_scale () {
  int scale = gdk_monitor_get_scale_factor(get_gdk_monitor());
  return 1 / (double)scale;
}
double set_window_size_from_pixbuf (GdkPixbuf *pixbuf) {
  double par_scale = get_par_scale();
  int p_width = gdk_pixbuf_get_width(pixbuf) * par_scale;
  int p_height = gdk_pixbuf_get_height(pixbuf) * par_scale;
  GdkRectangle rectangle;
  gdk_monitor_get_geometry(get_gdk_monitor(), &rectangle);
  int b_height = 33;
  int s_width9 = rectangle.width * 0.9;
  int s_height9 = rectangle.height * 0.85;
  double w_hi = p_width <= s_width9 ? 1 : (double)s_width9 / p_width;
  double h_hi = p_height <= s_height9 ? 1 : (double)s_height9 / p_height;
  double hi = MIN(w_hi, h_hi);
  int n_width = (double)p_width * hi;
  int n_height = (double)p_height * hi;
  gtk_window_resize(
      GTK_WINDOW(window), n_width + 2, n_height + b_height + 2);
  return hi;
}
double get_window_fix_scale (GdkPixbuf *pixbuf) {
  double ps = get_par_scale();
  double width = gdk_pixbuf_get_width(pixbuf) * ps;
  double height = gdk_pixbuf_get_height(pixbuf) * ps;
  double w_width = gtk_widget_get_allocated_width(scroll_window) - 2;
  double w_height = gtk_widget_get_allocated_height(scroll_window) - 2;
  double w_hi = (double)w_width / width;
  double h_hi = (double)w_height / height;
  return MIN(w_hi, h_hi);
}

void set_title (GdkPixbuf *pixbuf, double now_scale, const char *fname) {
  int width = gdk_pixbuf_get_width(pixbuf);
  int height = gdk_pixbuf_get_height(pixbuf);
  const char *slash_p = strrchr(fname, '/');
  const char *f_name = slash_p == NULL ? fname : slash_p + 1;
  gchar *title = 
    g_strdup_printf("%s %dx%d %d%%",
      f_name, width, height,(int)(now_scale * 100));
  gtk_window_set_title(GTK_WINDOW(window), title);
  g_free(title);
}
void re_draw (GdkPixbuf *pixbuf, double now_scale, const char *fname) {
  if (!pixbuf) return;
  int width = gdk_pixbuf_get_width(pixbuf);
  int height = gdk_pixbuf_get_height(pixbuf);
  double par_scale = get_par_scale();
  int ad_width = (double)width * now_scale * par_scale;
  int ad_height = (double)height * now_scale * par_scale;
  gtk_widget_set_size_request(d_area, ad_width, ad_height);
  set_title(pixbuf, now_scale, fname);
  gtk_widget_queue_draw(d_area);
}

GtkWidget* init_builder (const char* path) {
  GtkBuilder *builder = gtk_builder_new(); 
  gtk_builder_add_from_file(builder, path, NULL); 
  gtk_builder_connect_signals(builder, NULL); 
  window = (GtkWidget*)gtk_builder_get_object(builder, "window"); 
  scroll_window =
    (GtkWidget*)gtk_builder_get_object(builder, "scroll_window"); 
  d_area = (GtkWidget*)gtk_builder_get_object(builder,"draw_area");
  gtk_widget_set_name(scroll_window, "draw_area");
  return window;
}
void load_css (const char* css_file) {
  GtkCssProvider *provider = gtk_css_provider_new ();
  GdkDisplay *display = gdk_display_get_default ();
  GdkScreen *screen = gdk_display_get_default_screen (display);
  gtk_style_context_add_provider_for_screen (
        screen, GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  GError *error = 0;
  gtk_css_provider_load_from_file(
      provider, g_file_new_for_path(css_file), &error);
  g_object_unref(provider);
}