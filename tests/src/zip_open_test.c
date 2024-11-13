#include "../../include/log.h"
#include "../../include/zip.h"
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <stdint.h>
#include <stdio.h>

Test(zip, test_zip_open) {
  FILE *file = fopen("test.zip", "r");
  if (file) {
    struct zip_file *zip = zip_open_file(file);
    cr_expect(zip != NULL, "Zip is null");
    if (!zip) {
      return;
    }
    zip_close_file(zip);
    fclose(file);
  } else {
    log_fatal("Test file not found.");
    return;
  }
}

Test(zip, test_zip_stream_read) {
  FILE *file = fopen("test.zip", "r");
  if (file) {
    struct zip_file *zip = zip_open_file(file);
    cr_expect(zip != NULL, "Zip is null");
    if (!zip) {
      return;
    }

    struct zip_file_stream *stream = zip_open_stream(zip_get_entry(zip, 0));
    int16_t byte;
    while ((byte = zip_next_byte(stream)) >= 0) {
      printf("%c", byte);
    }
    printf("\n");
    zip_close_stream(stream);

    zip_close_file(zip);
    fclose(file);
  } else {
    log_fatal("Test file not found.");
    return;
  }
}
