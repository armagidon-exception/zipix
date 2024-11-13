#ifndef DEFLATE_H
#define DEFLATE_H

#include "bytebuffer.h"
#include <stdint.h>

int deflate_uncompress(ByteBuffer *src, uint8_t *dest);
int deflate_compress(ByteBuffer *src, uint8_t *dest);

#endif /* end of include guard: DEFLATE_H */
