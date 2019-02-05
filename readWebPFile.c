#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "webp/decode.h"

typedef struct {
  uint8_t *data;
  unsigned int width;
  unsigned int height;
  unsigned int ch;
} WEBP_DATA;

WEBP_DATA readWebPFile (const char *path) {
  WEBP_DATA webp_data;
  webp_data.data = NULL;
  FILE *fi = fopen(path, "rb");
  if (fi == NULL) {
    printf("%sは開けない\n", path);
    return webp_data;
  }
  struct stat statinfo;
  if (stat(path, &statinfo) != 0) {
    printf("%sのファイルサイズ取得に失敗\n", path);
    fclose(fi);
    return webp_data;
  }
  long file_size = statinfo.st_size;
  uint8_t *file_data = (uint8_t*) malloc(file_size);
  if (file_data == NULL) {
    printf("malloc失敗\n");
    fclose(fi);
    return webp_data;
  }
  fread(file_data, file_size, 1, fi);
  fclose(fi);

  WebPBitstreamFeatures features;
  VP8StatusCode ret = WebPGetFeatures(file_data, file_size, &features);
  if (ret != VP8_STATUS_OK ) {
    printf("WebPGetFeatures error\n");
    free(file_data);
    return webp_data;
  }
  int width, height;
  uint8_t *data = features.has_alpha == 0 ?
    WebPDecodeRGB (file_data, file_size, &width, &height):
    WebPDecodeRGBA(file_data, file_size, &width, &height);
  free(file_data);

  unsigned int ch = features.has_alpha == 0 ? 3 : 4;
  long data_size = width * height * ch;
  webp_data.data = (uint8_t*) malloc(data_size);
  memcpy(webp_data.data, data, data_size);
  webp_data.width = width;
  webp_data.height = height;
  webp_data.ch = ch;
  WebPFree(data);
  return webp_data;
}
