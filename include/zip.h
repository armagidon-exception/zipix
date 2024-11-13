#ifndef ZIP_H
#define ZIP_H

#include <stdint.h>
#include <stdio.h>

struct zip_file;
struct zip_file_entry;
struct zip_file_stream;

typedef enum { SHRINK, DEFLATE } compression_method;

struct zip_file *zip_open_file(FILE *file);
int zip_close_file(struct zip_file *file);

struct zip_file_stream *zip_open_stream(struct zip_file_entry *entry);
int16_t zip_next_byte(struct zip_file_stream *entry);
int zip_close_stream(struct zip_file_stream *entry);

size_t zip_get_entries(struct zip_file *file);
struct zip_file_entry *zip_get_entry(struct zip_file *file, size_t index);

#endif /* end of include guard: ZIP_H */
