#include <stdint.h>
typedef struct {
  uint8_t *data;
  unsigned int width;
  unsigned int height;
  unsigned int ch;
} WEBP_DATA;

WEBP_DATA readWebPFile (const char *path);

