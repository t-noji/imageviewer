#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <gdk-pixbuf/gdk-pixbuf-io.h>
#include "webp/decode.h"

static void destroy_data (guchar *data, gpointer p) {
  WebPFree(data);
}
GdkPixbuf* get_pixbuf_webp_from_file (const char *path) {
  FILE *fi = fopen(path, "rb");
  if (fi == NULL) {
    printf("%sは開けない\n", path);
    return NULL; 
  }
  struct stat statinfo;
  if (stat(path, &statinfo) != 0) {
    printf("%sのファイルサイズ取得に失敗\n", path);
    fclose(fi);
    return NULL;
  }
  long file_size = statinfo.st_size;
  uint8_t *file_data = (uint8_t*)malloc(file_size);
  if (file_data == NULL) {
    printf("malloc失敗\n");
    fclose(fi);
    return NULL;
  }
  fread(file_data, file_size, 1, fi);
  fclose(fi);

  WebPBitstreamFeatures features;
  VP8StatusCode ret = WebPGetFeatures(file_data, file_size, &features);
  if (ret != VP8_STATUS_OK ) {
    printf("WebPGetFeatures error\n");
    free(file_data);
    return NULL;
  }
  int width, height;
  uint8_t *data = features.has_alpha == 0 ?
    WebPDecodeRGB (file_data, file_size, &width, &height):
    WebPDecodeRGBA(file_data, file_size, &width, &height);
  free(file_data);

  int ch = features.has_alpha == 0 ? 3 : 4;
  return gdk_pixbuf_new_from_data(
      data, GDK_COLORSPACE_RGB, ch == 4, 8,
      width, height, width * ch, destroy_data, NULL);
}
