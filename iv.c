#include <gtk/gtk.h>
#include <string.h>
#include "get_pixbuf_webp_from_file.h"
#include "get_pixbuf_heif_from_file.h"
#include "widget_func.h"

#define CONF_PATH ""

GdkPixbuf *image_pixbuf = NULL;
char *now_dir = NULL;
double now_scale = 1;
GList* flist = NULL;

char* get_dir_name (const char *path) {
  gchar *cf = g_canonicalize_filename(path, NULL);
  gchar *dir = g_path_get_dirname(cf);
  g_free(cf);
  return dir;
}

_Bool path_type_check (const char *path, const char *type) {
  char *file_ex = strrchr(path, '.');
  return file_ex && strcmp(file_ex + 1, type) == 0;
}
_Bool path_is_image (const char* name) {
  return path_type_check(name, "webp") || path_type_check(name, "heif")
    || path_type_check(name, "jpg") || path_type_check(name, "png");
}
GList* get_path_filelist (const char *dir_name) {
  GDir *dir = g_dir_open(dir_name, 0, NULL);
  if (!dir) {
    g_print("Does not open dir %s\n", dir_name);
    return NULL;
  }
  GList *list = NULL;
  const char *name;
  while ((name = g_dir_read_name(dir)))
    if (path_is_image(name))
      list = g_list_prepend(list, (void*)g_strdup(name));
  g_dir_close(dir);
  return list;
}
GList* get_now_path_filelist (GList* f_list, const char *file_name) {
  GList *list = f_list;
  if (file_name)
    while (strcmp(file_name, list->data) && (list = list->next));
  return list ? list : f_list;
}

GdkPixbuf* pixbuf_from_file (const char *path){
  return path_type_check(path, "webp") ? get_pixbuf_webp_from_file(path):
         path_type_check(path, "heif") ? get_pixbuf_heif_from_file(path):
                               gdk_pixbuf_new_from_file(path, NULL);
}
void set_pixbuf (GdkPixbuf *pixbuf) {
  if (!pixbuf) return;
  if (image_pixbuf) g_object_unref(image_pixbuf);
  image_pixbuf = pixbuf;
  now_scale = get_window_fix_scale(pixbuf);
}
void set_pixbuf_from_file (const char *path) {
  set_pixbuf(pixbuf_from_file(path));
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
  if (image_pixbuf) now_scale = get_window_fix_scale(image_pixbuf);
  re_draw(image_pixbuf, now_scale, flist->data);
}
G_MODULE_EXPORT void dot_by_dot_click 
(GtkButton *button, gpointer p) {
  now_scale = 1;
  re_draw(image_pixbuf, now_scale, flist->data);
}
G_MODULE_EXPORT void plus_click (GtkButton *button, gpointer p) {
  if (now_scale > 4) return;
  now_scale *= 1.125;
  re_draw(image_pixbuf, now_scale, flist->data);
}
G_MODULE_EXPORT void minus_click (GtkButton *button, gpointer p) {
  if (now_scale < 0.025) return;
  now_scale *= 1.0 / 1.125;
  re_draw(image_pixbuf, now_scale, flist->data);
}
G_MODULE_EXPORT void back_click 
(GtkButton *button, gpointer p) {
  if (flist) {
    if (flist->prev) flist = flist->prev;
    gchar *path = g_build_filename(now_dir, (char*)flist->data, NULL);
    set_pixbuf_from_file(path);
    g_free(path);
    re_draw(image_pixbuf, now_scale, flist->data);
  }
}
G_MODULE_EXPORT void next_click 
(GtkButton *button, gpointer p) {
  if (flist) {
    if (flist->next) flist = flist->next;
    gchar *path = g_build_filename(now_dir, (char*)flist->data, NULL);
    set_pixbuf_from_file(path);
    g_free(path);
    re_draw(image_pixbuf, now_scale, flist->data);
  }
}
G_MODULE_EXPORT void key_press
(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  switch (event->keyval) {
    case GDK_KEY_Up:
      scroll_up();
      break;
    case GDK_KEY_Down:
      scroll_down();
      break;
    case GDK_KEY_Left:
      scroll_left();
      break;
    case GDK_KEY_Right:
      scroll_right();
      break;
    case GDK_KEY_j:
    case GDK_KEY_Page_Down:
      next_click(NULL, NULL);
      break;
    case GDK_KEY_k:
    case GDK_KEY_Page_Up:
      back_click(NULL, NULL);
      break;
    case GDK_KEY_1:
      dot_by_dot_click(NULL, NULL);
      break;
    case GDK_KEY_s:
      window_fix_click(NULL, NULL);
      break;
    case GDK_KEY_plus:
      plus_click(NULL, NULL);
      break;
    case GDK_KEY_minus:
      minus_click(NULL, NULL);
      break;
    case GDK_KEY_f:
    case GDK_KEY_F11:
      fullscreen();
      break;
    case GDK_KEY_q:
      gtk_main_quit();
      break;
  }
}

static GtkTargetEntry target[] = { { "text/uri-list", 0, 0 } };
G_MODULE_EXPORT void window_drag_data_received_cb
(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
 GtkSelectionData *selection_data, guint info, guint time, gpointer p) {
  gchar **uris = gtk_selection_data_get_uris(selection_data);
  gchar *fname = g_filename_from_uri(uris[0], NULL, NULL);
  if (path_is_image(fname)) {
    g_free(now_dir);
    g_list_free_full(flist, g_free);
    now_dir = get_dir_name(fname);
    flist = get_now_path_filelist(get_path_filelist(now_dir), now_dir);
    set_pixbuf_from_file(fname);
    re_draw(image_pixbuf, now_scale, flist->data);
  }
  g_strfreev(uris);
  g_free(fname);
}

int drag_start_x = -1, drag_start_y = -1;
G_MODULE_EXPORT void d_area_motion
(GtkWidget *widget, GdkEventMotion *event, gpointer p) {
  if (event->state & GDK_BUTTON1_MASK) {
    if (drag_start_x > 0) h_inc_scroll(drag_start_x - event->x);
    if (drag_start_y > 0) v_inc_scroll(drag_start_y - event->y);
  }
}
G_MODULE_EXPORT void d_area_button_release
(GtkWidget *widget, GdkEventMotion *event, gpointer p) {
  drag_start_x = -1;
  drag_start_y = -1;
}
GtkMenu *menu;
G_MODULE_EXPORT void d_area_button_press
(GtkWidget *widget, GdkEventButton *event, gpointer p) {
  drag_start_x = event->x;
  drag_start_y = event->y;
  if (event->type == GDK_2BUTTON_PRESS) fullscreen();
  else if (event->button == 3)
    gtk_menu_popup_at_pointer(menu, (GdkEvent*)event);
}
void make_menu () {
  menu = (GtkMenu*)gtk_menu_new();
  GtkWidget *back = gtk_menu_item_new_with_label("< Back");
  g_signal_connect(G_OBJECT(back), "activate", G_CALLBACK(back_click),NULL);
  gtk_menu_attach(menu, back, 0, 1, 0, 1);
  GtkWidget *next = gtk_menu_item_new_with_label("Next >");
  g_signal_connect(G_OBJECT(next), "activate", G_CALLBACK(next_click),NULL);
  gtk_menu_attach(menu, next, 1, 2, 0, 1);
  GtkWidget *plus = gtk_menu_item_new_with_label("+ Plus");
  g_signal_connect(G_OBJECT(plus), "activate", G_CALLBACK(plus_click),NULL);
  gtk_menu_attach(menu, plus, 0, 1, 1, 2);
  GtkWidget *minus = gtk_menu_item_new_with_label("Minus-");
  g_signal_connect(G_OBJECT(minus),"activate",G_CALLBACK(minus_click),NULL);
  gtk_menu_attach(menu, minus, 1, 2, 1, 2);
  GtkWidget *fix = gtk_menu_item_new_with_label("Window fix");
  g_signal_connect(
      G_OBJECT(fix), "activate", G_CALLBACK(window_fix_click), NULL);
  gtk_menu_attach(menu, fix, 0, 2, 2, 3);
  GtkWidget *dbd = gtk_menu_item_new_with_label("Dot by dot");
  g_signal_connect(
      G_OBJECT(dbd), "activate", G_CALLBACK(dot_by_dot_click), NULL);
  gtk_menu_attach(menu, dbd, 0, 2, 3, 4);
  GtkWidget *full = gtk_menu_item_new_with_label("Full screen");
  g_signal_connect(
      G_OBJECT(full), "activate", G_CALLBACK(fullscreen), NULL);
  gtk_menu_attach(menu, full, 0, 2, 4, 5);
  gtk_widget_show_all((GtkWidget*)menu);
}

gpointer thread_get_pixbuf_from_file (gpointer path) {
  g_thread_exit(pixbuf_from_file((const char*)path));
  return NULL;
}

int main (int argc, char *argv[]) {
  gtk_init(&argc, &argv); // GTK関連引数の除去効果あり //
 
  GThread *thread = NULL;
  char *dir_name = NULL;
  char *file_name = NULL;

  if (argc > 1) {
    char *path = argv[1];
    dir_name = get_dir_name(path);
    file_name = g_path_get_basename(path);
    if (path_is_image(file_name))
      thread = g_thread_new(NULL, thread_get_pixbuf_from_file, path);
  }
  else dir_name = g_get_current_dir();

  flist = get_now_path_filelist(get_path_filelist(dir_name), file_name);
  now_dir = dir_name;

  GtkWidget *window = init_builder(CONF_PATH "iv.ui");
  make_menu();
  load_css(CONF_PATH "iv.css");
  gtk_drag_dest_set(
      window, GTK_DEST_DEFAULT_ALL, target, 1, GDK_ACTION_COPY);
  gtk_widget_show_all(window);

  if (thread) {
    gpointer status = g_thread_join(thread);
    set_pixbuf((GdkPixbuf*)status);
    now_scale = set_window_size_from_pixbuf(image_pixbuf);
    re_draw(image_pixbuf, now_scale, flist->data);
  }
  gtk_main();
  return 0;
}
