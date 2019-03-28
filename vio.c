#define V_INTERNAL
#include <stdio.h>
#include <stdlib.h>
#include "vfs.h"
#include "vhacks.h"
#include "vio.h"

enum {_v_BUF_LEN_START = 1024};

char *v_fread_a_buf(v_file_t *f, size_t len, size_t *out_len) {
	// Initialise read and allocated lengths
	size_t rlen = 0, alen = _v_BUF_LEN_START;
	// Initialise buffer for storing data
	char *buf = malloc(alen);
	if (!buf) return NULL;

	// Save the current error code and set it to 0
	enum v_file_error err_save = f->err;
	f->err = 0;

	for (;;) {
		// Fill the buffer with data
		rlen += v_fread(f, buf+rlen, alen-rlen);

		if (rlen < alen) {
			// If the buffer isn't full, it's either an error or an EOF
			if (f->err) {
				f->err |= err_save;
				free(buf);
				return NULL;
			} else {
				break;
			}
		}

		// Need more space
		char *tmp = realloc(buf, alen*=2);
		if (!tmp) {
			free(buf);
			return NULL;
		}
		buf = tmp;
	}

	// NUL-terminate
	buf = realloc(buf, rlen+1);
	buf[rlen] = 0;

	// Restore the error value
	f->err = err_save;

	// Return the values
	if (out_len) *out_len = rlen;
	return buf;
}

char *v_fread_a_std_buf(FILE *f, size_t len, size_t *out_len) {
	// Initialise read and allocated lengths
	size_t rlen = 0, alen = _v_BUF_LEN_START;
	// Initialise buffer for storing data
	char *buf = malloc(alen);
	if (!buf) return NULL;

	for (;;) {
		// Fill the buffer with data
		rlen += fread(buf+rlen, 1, alen-rlen, f);

		if (rlen < alen) {
			// If the buffer isn't full, it's either an error or an EOF
			if (ferror(f)) {
				free(buf);
				return NULL;
			} else {
				break;
			}
		}

		// Need more space
		char *tmp = realloc(buf, alen*=2);
		if (!tmp) {
			free(buf);
			return NULL;
		}
		buf = tmp;
	}

	// NUL-terminate
	buf = realloc(buf, rlen+1);
	buf[rlen] = 0;

	// Return the values
	if (out_len) *out_len = rlen;
	return buf;
}

char *v_fgets_a(FILE *f) {
	size_t rlen = 0, alen = _v_BUF_LEN_START;
	char *buf = malloc(alen);
	if (!buf) return NULL;

	for (;;) {
		// alen - rlen should never be two or less
		buf[alen-1] = 0;
		buf[alen-2] = 0;
		// Check to make absolutely sure that this is the case and no maintainer has done a stupid
		vhack_static_assert(_v_BUF_LEN_START > 2);

		if (!fgets(buf+rlen, alen-rlen, f)) {
			free(buf);
			return NULL;
		}

		if (!buf[alen-1] && buf[alen-2]) {
			// Buffer is full
			
			// If it ends with a newline, we're done
			if (buf[alen-2] == '\n') return buf;

			// No newline found, we need to loop again
			char *tmp = realloc(buf, alen*=2);
			if (!tmp) {
				free(buf);
				return NULL;
			}
			buf = tmp;
		} else {
			// Buffer is not full, meaning we read a newline before filling it
			return buf;
		}
	}
}
