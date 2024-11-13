#include "deflate.h"
#include <string.h>

int deflate_uncompress(ByteBuffer *src, uint8_t *dest) {
  memcpy(dest, src->buffer, src->size);
  return 0;
}
