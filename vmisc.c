#define V_INTERNAL
#include <stdlib.h>
#include <string.h>
#include "vmisc.h"

#ifndef v_strdup
char *v_strdup(const char *s) {
	size_t len = strlen(s)+1;
	char *new = malloc(len);
	memcpy(new, s, len);
	return new;
}
#endif

#ifndef v_strnlen
size_t v_strnlen(const char *s, size_t max) {
	size_t len = 0;
	while (len < max && *s) s++, len++;
	return len;
}
#endif

char *v_strndup(const char *s, size_t len) {
	len = v_strnlen(s, len);
	char *new = malloc(len+1);
	memcpy(new, s, len);
	new[len] = 0;
	return new;
}

void *v_memdup(const void *s, size_t len) {
	char *new = malloc(len);
	memcpy(new, s, len);
	return new;
}
