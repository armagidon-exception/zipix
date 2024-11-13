#ifndef SHRINK_H
#define SHRINK_H

#include "bytebuffer.h"
#include <stdint.h>

int shrink_decompress(ByteBuffer *src, uint8_t *dest);

int shrink_compress(ByteBuffer *src, uint8_t *dest);

#endif /* end of include guard: SHRINK_H */
