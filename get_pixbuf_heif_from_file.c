#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf-io.h>
#include <libheif/heif.h>

static void cleanup_heif_context (guchar* pixels, gpointer hc) {
  g_free(pixels);
  heif_context_free((struct heif_context*)hc);
}
GdkPixbuf* get_pixbuf_heif_from_file (const char *path) {
	struct heif_error err;

	struct heif_context *hc = heif_context_alloc();
  err = heif_context_read_from_file(hc, path, NULL);
  if (err.code != heif_error_Ok) {
		g_warning("%s", err.message);
	  heif_context_free(hc);
    return NULL;
	}
	struct heif_image_handle *hdl;
	err = heif_context_get_primary_image_handle(hc, &hdl);
	if (err.code != heif_error_Ok) {
		g_warning("%s", err.message);
	  heif_context_free(hc);
    return NULL;
	}
	struct heif_image *img;
	err = heif_decode_image(
      hdl, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, NULL);
	if (err.code != heif_error_Ok) {
		g_warning("%s", err.message);
	  heif_context_free(hc);
    return NULL;
	}

	int width = heif_image_get_width(img, heif_channel_interleaved);
	int height = heif_image_get_height(img, heif_channel_interleaved);
	int stride;
	const uint8_t *data = heif_image_get_plane_readonly(
      img, heif_channel_interleaved, &stride);

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(
      data, GDK_COLORSPACE_RGB, TRUE, 8, width, height,
      stride, cleanup_heif_context, hc);

  return pixbuf;
}
