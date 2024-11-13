#include "bytebuffer.h"
#include "log.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int buf_read_from_stream(ByteBuffer *buf, FILE *inputstream) {
  if (!buf) {
    log_error("Could not read bytes from stream: pointer to buffer is null");
    return 1;
  }
  if (!inputstream) {
    log_error("Could not read bytes from stream: input stream is null");
    return 1;
  }

  fseek(inputstream, 0, SEEK_END);
  long fsize = ftell(inputstream);

  if (fsize < 0) {
    log_error("Could not read bytes from stream: %s", strerror(errno));
    return 1;
  }
  rewind(inputstream);

  buf->buffer = malloc(fsize);
  buf->size = fsize;

  if (!buf->buffer) {
    log_error("Could not read bytes from stream: %s", strerror(errno));
    return 1;
  }

  fread(buf->buffer, 1, buf->size, inputstream);

  if (ferror(inputstream)) {
    log_error("Could not read bytes from stream: ", strerror(errno));
    return 1;
  }

  return 0;
}
