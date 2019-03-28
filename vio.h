#ifndef VIO_H
#define VIO_H

// IO helpers for vfs and stdio

#include <stdio.h>
#include "vfs.h"

// Allocating read from vfs v_file_t
// If len == 0, reads entire file
#define v_fread_a(f, len) v_fread_a_buf(f, len, NULL)

// Allocating read from vfs v_file_t
// If len == 0, reads entire file
// Sets *out_len to the number of bytes read
char *v_fread_a_buf(v_file_t *f, size_t len, size_t *out_len);

// Allocating read from stdio FILE
// If len == 0, reads entire file
#define v_fread_a_std(f, len) v_fread_a_std_buf(f, len, NULL)

// Allocating read from stdio FILE
// If len == 0, reads entire file
// Sets *out_len to the number of bytes read
char *v_fread_a_std_buf(FILE *f, size_t len, size_t *out_len);

// Allocating single-line read from stdio FILE
char *v_fgets_a(FILE *f);

// Allocating single-line read from stdin
#define v_gets_a() v_fgets_a(stdin)

#endif
