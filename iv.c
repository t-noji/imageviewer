#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "readWebPFile.h"

GtkWidget *window;
GtkWidget *scroll_window;
GtkWidget *d_area;

GdkPixbuf *image_pixbuf = NULL;
const char *now_fname = NULL;
double now_scale = 1;
GList* flist = NULL;

_Bool path_type_check (const char *path, const char *type) {
  char *file_ex = strrchr(path, '.');
  return file_ex && strcmp(file_ex + 1, type) == 0;
}
GList* get_path_filelist (const char *dir_name) {
  GDir *dir = g_dir_open(dir_name, 0, NULL);
  if (!dir) return NULL;
  GList *list = NULL;
  _Bool type_check (const char* name) {
    return path_type_check(name, "webp")
      || path_type_check(name, "jpg") || path_type_check(name, "png");
  }
  const char* name;
  while (name = g_dir_read_name(dir)) if (type_check(name)) {
    char *tmp = (char*)malloc(sizeof(name));
    strcpy(tmp, name);
    list = g_list_prepend(list, (void*)tmp);
  }
  g_dir_close(dir);
  return list;
}
static void destroy_data (guchar *data, gpointer p) { g_free(data); }
GdkPixbuf* webp_to_pixbuf (const char *path) {
  WEBP_DATA wd = readWebPFile(path);
  if (!wd.data) return NULL;
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(wd.data,
      GDK_COLORSPACE_RGB, wd.ch == 4, 8,
      wd.width, wd.height, wd.width * wd.ch, destroy_data, NULL);
  return pixbuf;
}
GdkMonitor* get_gdk_monitor () {
  GdkDisplay *display = gdk_display_get_default();
  return gdk_display_get_monitor(display, 0);
  // 本来 at_windowで取得したいがGDK-CRITICALが出る為断念 //
}
double get_par_scale () {
  int scale = gdk_monitor_get_scale_factor(get_gdk_monitor());
  return 1 / (double)scale;
}
void set_window_size_from_pixbuf (GdkPixbuf *pixbuf) {
  double par_scale = get_par_scale();
  int p_width = gdk_pixbuf_get_width(pixbuf) * par_scale;
  int p_height = gdk_pixbuf_get_height(pixbuf) * par_scale;
  GdkRectangle rectangle;
  gdk_monitor_get_geometry(get_gdk_monitor(), &rectangle);
  int b_height = 34;
  int s_width9 = rectangle.width * 0.9;
  int s_height9 = rectangle.height * 0.85;
  double w_hi = p_width <= s_width9 ? 1 : (double)s_width9 / p_width;
  double h_hi = p_height <= s_height9 ? 1 : (double)s_height9 / p_height;
  double hi = w_hi < h_hi ? w_hi : h_hi;
  int n_width = (double)p_width * hi;
  int n_height = (double)p_height * hi;
  now_scale = hi;
  gtk_window_resize(GTK_WINDOW(window), n_width + 2, n_height + b_height +2);
}
void set_title () {
  int width = gdk_pixbuf_get_width(image_pixbuf);
  int height = gdk_pixbuf_get_height(image_pixbuf);
  gchar *title = 
    g_strdup_printf("%s %dx%d %d%%",
      now_fname, width, height, (int)(now_scale * 100));
  gtk_window_set_title(GTK_WINDOW(window), title);
  g_free(title);
}
void re_draw () {
  if (!image_pixbuf) return;
  int width = gdk_pixbuf_get_width(image_pixbuf);
  int height = gdk_pixbuf_get_height(image_pixbuf);
  double par_scale = get_par_scale();
  int ad_width = (double)width * now_scale * par_scale;
  int ad_height = (double)height * now_scale * par_scale;
  gtk_widget_set_size_request(d_area, ad_width, ad_height);
  set_title();
  gtk_widget_queue_draw(d_area);
}
void set_pixbuf (GdkPixbuf *pixbuf, const char *path) {
  if (!pixbuf) return;
  if (image_pixbuf) g_object_unref(image_pixbuf);
  image_pixbuf = pixbuf;
  now_fname = path;
  set_window_size_from_pixbuf(pixbuf);
  re_draw();
}

GdkPixbuf* pixbuf_from_file (const char *path){
  return path_type_check(path, "webp") ?
    webp_to_pixbuf(path) : gdk_pixbuf_new_from_file(path, NULL);
}
void set_pixbuf_from_file (const char *path) {
  set_pixbuf(pixbuf_from_file(path), path);
}

G_MODULE_EXPORT void area_draw
(GtkWidget *widget, cairo_t *cr, gpointer data) {
  if (!image_pixbuf) return;
  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  guint width = gtk_widget_get_allocated_width(widget);
  guint height = gtk_widget_get_allocated_height(widget);
  gtk_render_background(context, cr, 0, 0, width, height);
  double scale = now_scale * get_par_scale();
  cairo_scale(cr, scale, scale);
  gdk_cairo_set_source_pixbuf(cr, image_pixbuf, 0.0, 0.0);
  cairo_paint(cr);
}
G_MODULE_EXPORT void window_fix_click 
(GtkButton *button, gpointer p) {
  double ps = get_par_scale();
  double width = gdk_pixbuf_get_width(image_pixbuf) * ps;
  double height = gdk_pixbuf_get_height(image_pixbuf) * ps;
  double w_width = gtk_widget_get_allocated_width(scroll_window) - 2;
  double w_height = gtk_widget_get_allocated_height(scroll_window) - 2;
  double w_hi = (double)w_width / width;
  double h_hi = (double)w_height / height;
  now_scale = MIN(w_hi, h_hi);
  re_draw();
}
G_MODULE_EXPORT void dot_by_dot_click 
(GtkButton *button, gpointer p) {
  now_scale = 1;
  re_draw();
}
G_MODULE_EXPORT void plus_click (GtkButton *button, gpointer p) {
  if (now_scale > 4) return;
  now_scale *= 1.25;
  re_draw();
}
G_MODULE_EXPORT void minus_click (GtkButton *button, gpointer p) {
  if (now_scale < 0.025) return;
  now_scale *= 0.8;
  re_draw();
}
G_MODULE_EXPORT void back_click 
(GtkButton *button, gpointer p) {
  if (flist->prev) flist = flist->prev;
  set_pixbuf_from_file((char*)flist->data);
}
G_MODULE_EXPORT void next_click 
(GtkButton *button, gpointer p) {
  if (flist->next) flist = flist->next;
  set_pixbuf_from_file((char*)flist->data);
}

void* thread_read_webp_file (const char* path) {
  pthread_exit(pixbuf_from_file(path));
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
  g_object_unref (provider);
}

int main (int argc, char *argv[]) {
  pthread_t thread;
  if (argc > 1)
    pthread_create(&thread, NULL, (void*)thread_read_webp_file, argv[1]);

  gtk_init(&argc, &argv); 

  GtkBuilder *builder = gtk_builder_new(); 
  gtk_builder_add_from_file(builder, "iv.ui", NULL); 
  gtk_builder_connect_signals(builder, NULL); 
  load_css("iv.css");

  window = (GtkWidget*)gtk_builder_get_object(builder, "window"); 
  scroll_window =
    (GtkWidget*)gtk_builder_get_object(builder, "scroll_window"); 
  d_area = (GtkWidget*)gtk_builder_get_object(builder,"draw_area");
  gtk_widget_set_name(scroll_window, "draw_area");
  
  gtk_widget_show_all(window);
  flist = get_path_filelist(g_get_current_dir());

  if (argc > 1) {
    while (strcmp(argv[1], flist->data) && (flist = flist->next));
    void *status;
    pthread_join(thread, &status);
    set_pixbuf((GdkPixbuf*)status, argv[1]);
  }
  gtk_main();
  return 0;
}
