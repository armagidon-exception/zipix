#include "shrink.h"
#include <string.h>

int shrink_decompress(ByteBuffer *src, uint8_t *dest) {
  memcpy(dest, src->buffer, src->size);
  return 0;
}

int shrink_compress(ByteBuffer *src, uint8_t *dest);
