#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint8_t *buffer;
  size_t size;
} ByteBuffer;

int buf_read_from_stream(ByteBuffer *buf, FILE *inputstream);

#endif /* end of include guard: BYTE_BUFFER_H */
