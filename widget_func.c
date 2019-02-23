#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

GtkWidget *window;
GtkWidget *scroll_window;
GtkWidget *d_area;
GtkWidget *button_box;

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
  int b_height = gtk_widget_get_allocated_height(button_box);
  int s_width9 = rectangle.width * 0.9;
  int s_height9 = rectangle.height * 0.85;
  double w_hi = p_width <= s_width9 ? 1 : (double)s_width9 / p_width;
  double h_hi = p_height <= s_height9 ? 1 : (double)s_height9 / p_height;
  double hi = MIN(w_hi, h_hi);
  int n_width = (double)p_width * hi;
  int n_height = (double)p_height * hi;
  gtk_window_resize(
      GTK_WINDOW(window), n_width, n_height + b_height);
  return hi;
}
double get_window_fix_scale (GdkPixbuf *pixbuf) {
  double ps = get_par_scale();
  double width = gdk_pixbuf_get_width(pixbuf) * ps;
  double height = gdk_pixbuf_get_height(pixbuf) * ps;
  double w_width = gtk_widget_get_allocated_width(scroll_window);
  double w_height = gtk_widget_get_allocated_height(scroll_window);
  double w_hi = (double)w_width / width;
  double h_hi = (double)w_height / height;
  return MIN(w_hi, h_hi);
}

void set_title (GdkPixbuf *pixbuf, double now_scale, const char *fname) {
  int width = gdk_pixbuf_get_width(pixbuf);
  int height = gdk_pixbuf_get_height(pixbuf);
  gchar *title = 
    g_strdup_printf("%s %dx%d %d%%",
      fname, width, height,(int)(now_scale * 100));
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
void fullscreen () {
  static _Bool full = FALSE;
  GtkWindow *win = GTK_WINDOW(window);
  if (full) {
    gtk_widget_show(button_box);
    gtk_window_unfullscreen(win);
  }
  else {
    // ボタンにフォーカスがあるとハングアップする為注意 //
    gtk_widget_hide(button_box);
    gtk_window_fullscreen(win);
  }
  full = !full;
}

void inc_adjustment (GtkAdjustment *adjust, double arg) {
  double v = gtk_adjustment_get_value(adjust);
  gtk_adjustment_set_value(adjust, v + arg);
}
void v_inc_scroll (double arg) {
  GtkAdjustment *adjust = 
    gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)scroll_window);
  inc_adjustment (adjust, arg);
}
void h_inc_scroll (double arg) {
  GtkAdjustment *adjust = 
    gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)scroll_window);
  inc_adjustment (adjust, arg);
}
void scroll_up () { v_inc_scroll(-100); }
void scroll_down () { v_inc_scroll(100); }
void scroll_left () { h_inc_scroll(-100); }
void scroll_right () { h_inc_scroll(100); }

GtkWidget* init_builder (const char* path) {
  GtkBuilder *builder = gtk_builder_new(); 
  gtk_builder_add_from_file(builder, path, NULL); 
  gtk_builder_connect_signals(builder, NULL); 
  window = (GtkWidget*)gtk_builder_get_object(builder, "window"); 
  scroll_window =
    (GtkWidget*)gtk_builder_get_object(builder, "scroll_window"); 
  d_area = (GtkWidget*)gtk_builder_get_object(builder, "draw_area");
  button_box = (GtkWidget*)gtk_builder_get_object(builder, "button_box");
  g_object_unref(builder);
  return window;
}
void load_css (const char* css_file) {
  GtkCssProvider *provider = gtk_css_provider_new();
  GdkDisplay *display = gdk_display_get_default();
  GdkScreen *screen = gdk_display_get_default_screen(display);
  gtk_style_context_add_provider_for_screen (
        screen, GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  GError *error = 0;
  gtk_css_provider_load_from_file(
      provider, g_file_new_for_path(css_file), &error);
  g_object_unref(provider);
}
