#include "zip.h"
#include "bytebuffer.h"
#include "deflate.h"
#include "log.h"
#include "shrink.h"
#include <alloca.h>
#include <assert.h>
#include <byteswap.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EOCD_SIGNATURE 0x06054b50
#define CENTRAL_DIR_SIGNATURE 0x02014b50

typedef struct eocd_record {
  uint32_t signature;
  uint16_t disknumber;
  uint16_t cds;
  uint16_t disk_entries;
  uint16_t total_entries;
  uint32_t central_directory_size;
  uint32_t offset;
  uint16_t comment_length;
} __attribute__((packed)) eocd_record;

typedef struct central_dir {
  uint32_t signature;
  uint16_t version;
  uint16_t version_needed;
  uint16_t flags;
  uint16_t compression;
  uint16_t modtime;
  uint16_t moddate;
  uint32_t crc32;
  uint32_t compressed_size;
  uint32_t uncompressed_size;
  uint16_t filename_length;
  uint16_t extra_field_length;
  uint16_t file_comment_length;
  uint16_t unused1;
  uint16_t internal_attr;
  uint32_t external_attr;
  uint32_t offset;
  // char *filename;
  // uint8_t *extra_field;
  // char *file_comment;
} __attribute__((packed)) central_dir;

typedef struct {
  uint32_t signature;
  uint16_t version;
  uint16_t flags;
  uint16_t compression;
  uint16_t modtime;
  uint16_t moddate;
  uint32_t crc32;
  uint32_t compressed_size;
  uint32_t uncompressed_size;
  uint16_t filename_length;
  uint16_t extra_field_length;
} __attribute__((packed)) lfhead;

struct zip_file_entry {
  char *filename;
  uint8_t *extra_field;
  char *file_comment;
  size_t size;
  size_t actual_size;
  size_t offset;
  ByteBuffer contents;
  compression_method compression_method;
};

typedef struct zip_file_entry zfe_t;

struct zip_file {
  ByteBuffer filebuf;
  zfe_t *entries;
  uint16_t entries_count;
};
typedef struct zip_file zf_t;

typedef struct zip_file_stream {
  uint8_t *uncompressed_data;
  uint32_t size;
  uint32_t pointer;
} zfs_t;

#define lfhead_size(header)                                                    \
  (sizeof(lfhead) + header->filename_length + header->extra_field_length)

static int64_t find_signature(uint32_t signature, ByteBuffer *buf) {
  if (!buf) {
    log_error("Could not find signature: buf is null");
    return -1;
  }
  int64_t index = -1;
  for (int64_t i = buf->size - 4; i >= 0; i--) {
    uint32_t sig = *((uint32_t *)(buf->buffer + i));
    if (sig == signature) {
      index = i;
      break;
    }
  }

  return index;
}

static int read_file_entry(zfe_t *entry, ByteBuffer buf) {
  lfhead *header = (lfhead *)(buf.buffer + entry->offset);
  size_t header_size = lfhead_size(header);
  uint8_t *file_data = (uint8_t *)buf.buffer + entry->offset + header_size;

  entry->contents = (ByteBuffer){.buffer = file_data, .size = entry->size};

  switch (header->compression) {
  case 8: {
    entry->compression_method = DEFLATE;
    break;
  }
  case 1: {
    entry->compression_method = SHRINK;
    break;
  }
  default: {
    log_error("Could not read file entry: unsupported compression method: %u",
              entry->compression_method);
    return 1;
  }
  }

  return 0;
}

static int read_zip_file(struct zip_file *file) {
  if (!file) {
    log_error("Could not read central dir: file is null");
    return 1;
  }

  if (file->filebuf.size < 2) {
    log_error("Could not read central dir: EOCD signature not found");
    return 1;
  }

  int64_t eocd_pos = find_signature(EOCD_SIGNATURE, &file->filebuf);

  if (eocd_pos < 0) {
    log_error("Could not read central directory: EOCD signature not found");
    return 1;
  }

  eocd_record eocd = *(eocd_record *)(file->filebuf.buffer + eocd_pos);

  uint32_t start_cds = eocd.offset;
  char *str = (char *)file->filebuf.buffer;

  file->entries = calloc(eocd.total_entries, sizeof(zfe_t));

  for (int i = 0; i < eocd.total_entries; i++) {
    central_dir *cdir = (central_dir *)(file->filebuf.buffer + start_cds);
    char *vchunk = str + sizeof(central_dir) + start_cds;

    char *filename = strndup(vchunk, cdir->filename_length);
    vchunk += cdir->filename_length;

    uint8_t *extra_field = malloc(cdir->extra_field_length);
    strncpy((char *)extra_field, vchunk, cdir->extra_field_length);
    vchunk += cdir->extra_field_length;

    char *file_comment = strndup(vchunk, cdir->file_comment_length);

    start_cds += cdir->filename_length + cdir->extra_field_length +
                 cdir->file_comment_length + sizeof(central_dir);

    zfe_t entry = (zfe_t){
        .filename = filename,
        .size = cdir->compressed_size,
        .actual_size = cdir->uncompressed_size,
        .offset = cdir->offset,
        .file_comment = file_comment,
        .extra_field = extra_field,
    };

    if (read_file_entry(&entry, file->filebuf))
      return 1;

    file->entries[file->entries_count] = entry;
    file->entries_count++;
  }
  return 0;
}

struct zip_file *zip_open_file(FILE *file) {
  if (!file) {
    log_error("Could not open zip file: File pointer is null");
    return NULL;
  }
  struct zip_file f = {0};

  if (buf_read_from_stream(&f.filebuf, file) > 0)
    return NULL;
  if (read_zip_file(&f))
    return NULL;

  struct zip_file *zf = (struct zip_file *)malloc(sizeof(struct zip_file));

  if (zf)
    *zf = f;
  return zf;
}

int zip_close_file(struct zip_file *file) {
  if (!file) {
    log_error("Could not close file: Zip file pointer is null");
    return 1;
  }
  for (int i = 0; i < file->entries_count; i++) {
    free(file->entries[i].filename);
    free(file->entries[i].file_comment);
    free(file->entries[i].extra_field);
  }
  free(file->entries);
  free(file->filebuf.buffer);
  free(file);
  return 0;
}

struct zip_file_stream *zip_open_stream(struct zip_file_entry *entry) {
  if (!entry) {
    log_error("Could not open zip stream: entry is null");
    return NULL;
  }
  zfs_t stream = {.pointer = 0,
                  .size = entry->actual_size,
                  .uncompressed_data = malloc(entry->actual_size)};
  switch (entry->compression_method) {
  case SHRINK:
    shrink_decompress(&entry->contents, stream.uncompressed_data);
    break;
  case DEFLATE:
    deflate_uncompress(&entry->contents, stream.uncompressed_data);
    break;
  }

  zfs_t *out = malloc(sizeof(zfs_t));
  *out = stream;

  return out;
}

int16_t zip_next_byte(struct zip_file_stream *entry) {
  if (!entry) {
    return -1;
  }
  if (entry->pointer >= entry->size)
    return -1;
  return entry->uncompressed_data[entry->pointer++];
}

int zip_close_stream(struct zip_file_stream *stream) {
  if (!stream)
    return 1;

  free(stream->uncompressed_data);
  free(stream);
  return 0;
}

size_t zip_get_entries(struct zip_file *file) {
  if (!file)
    return 0;
  return file->entries_count;
}

struct zip_file_entry *zip_get_entry(struct zip_file *file, size_t index) {
  if (!file)
    return 0;
  if (index >= file->entries_count)
    return 0;

  return file->entries + index;
}
