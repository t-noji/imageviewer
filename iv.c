#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "readWebPFile.h"
#include "widget_func.h"

#define CONF_PATH "./"

GdkPixbuf *image_pixbuf = NULL;
const char *now_fname = NULL;
const char *now_dir = NULL;
double now_scale = 1;
GList* flist = NULL;

_Bool path_type_check (const char *path, const char *type) {
  char *file_ex = strrchr(path, '.');
  return file_ex && strcmp(file_ex + 1, type) == 0;
}
GList* get_path_filelist (const char *dir_name) {
  GDir *dir = g_dir_open(dir_name, 0, NULL);
  if (!dir) {
    g_print("Does not open dir %s\n", dir_name);
    return NULL;
  }
  GList *list = NULL;
  _Bool type_check (const char* name) {
    return path_type_check(name, "webp")
      || path_type_check(name, "jpg") || path_type_check(name, "png");
  }
  const char* name;
  while (name = g_dir_read_name(dir))
    if (type_check(name))
      list = g_list_prepend(list, (void*)g_strdup(name));
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

GdkPixbuf* pixbuf_from_file (const char *path){
  return path_type_check(path, "webp") ?
    webp_to_pixbuf(path) : gdk_pixbuf_new_from_file(path, NULL);
}
void set_pixbuf (GdkPixbuf *pixbuf, const char *path) {
  if (!pixbuf) return;
  if (image_pixbuf) g_object_unref(image_pixbuf);
  image_pixbuf = pixbuf;
  now_fname = path;
  now_scale = set_window_size_from_pixbuf(pixbuf);
  re_draw(image_pixbuf, now_scale, now_fname);
}
void set_pixbuf_from_file (const char *path) {
  set_pixbuf(pixbuf_from_file(path), path);
}


G_MODULE_EXPORT void area_draw
(GtkWidget *widget, cairo_t *cr, gpointer data) {
  if (!image_pixbuf) return;
  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_render_background(context, cr, 0, 0, width, height);
  double scale = now_scale * get_par_scale();
  cairo_scale(cr, scale, scale);
  gdk_cairo_set_source_pixbuf(cr, image_pixbuf, 0.0, 0.0);
  cairo_paint(cr);
}
G_MODULE_EXPORT void window_fix_click 
(GtkButton *button, gpointer p) {
  now_scale = get_window_fix_scale(image_pixbuf);
  re_draw(image_pixbuf, now_scale, now_fname);
}
G_MODULE_EXPORT void dot_by_dot_click 
(GtkButton *button, gpointer p) {
  now_scale = 1;
  re_draw(image_pixbuf, now_scale, now_fname);
}
G_MODULE_EXPORT void plus_click (GtkButton *button, gpointer p) {
  if (now_scale > 4) return;
  now_scale *= 1.125;
  re_draw(image_pixbuf, now_scale, now_fname);
}
G_MODULE_EXPORT void minus_click (GtkButton *button, gpointer p) {
  if (now_scale < 0.025) return;
  now_scale *= 1.0 / 1.125;
  re_draw(image_pixbuf, now_scale, now_fname);
}
G_MODULE_EXPORT void back_click 
(GtkButton *button, gpointer p) {
  if (flist) {
    if (flist->prev) flist = flist->prev;
    gchar *path = g_strdup_printf("%s/%s", now_dir, flist->data);
    set_pixbuf_from_file(path);
    g_free(path);
  }
}
G_MODULE_EXPORT void next_click 
(GtkButton *button, gpointer p) {
  if (flist) {
    if (flist->next) flist = flist->next;
    gchar *path = g_strdup_printf("%s/%s", now_dir, flist->data);
    set_pixbuf_from_file(path);
    g_free(path);
  }
}

void* thread_read_webp_file (const char *path) {
  pthread_exit(pixbuf_from_file(path));
}

char** get_dir_file_name (const char *path) {
  char **result = (char**)malloc(sizeof(char**) * 2);
  result[0] =
      strrchr(path, '/') == NULL ? g_get_current_dir():
      path[0] == '/'             ? g_path_get_dirname(path):
        ({gchar *cd = g_get_current_dir(),
                *dn = g_path_get_dirname(path),
                *zn = g_strdup_printf("%s/%s", cd, dn);
          g_free(cd); g_free(dn);
          zn;});
  result[1] = g_path_get_basename(path);
  return result;
}

int main (int argc, char *argv[]) {
  pthread_t thread;
  char *dir_name = NULL;
  char *file_name = NULL;
  if (argc > 1) {
    char *path = argv[1];
    pthread_create(&thread, NULL, (void*)thread_read_webp_file, path);
    char **df = get_dir_file_name(path);
    dir_name = df[0];
    file_name = df[1];
    free(df);
  }
  else dir_name = g_get_current_dir();
  now_dir = dir_name;

  gtk_init(&argc, &argv); 
  GtkWidget *window = init_builder(CONF_PATH "iv.ui");
  load_css(CONF_PATH "iv.css");
  gtk_widget_show_all(window);
  GList *first = flist = get_path_filelist(dir_name);

  if (file_name) {
    if (flist)
      while (strcmp(file_name, flist->data) && (flist = flist->next));
    if (!flist) flist = first;
    void *status;
    pthread_join(thread, &status);
    set_pixbuf((GdkPixbuf*)status, argv[1]);
  }
  gtk_main();
  return 0;
}
