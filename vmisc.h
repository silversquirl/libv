#ifndef VMISC_H
#define VMISC_H

#include <stddef.h>
#include <string.h>

// POSIX defines a bunch of useful stuff
// Windows defines some of this useful stuff, but sometimes under different names
// Most other systems don't define this useful stuff
// There's also some useful stuff that isn't defined by POSIX but should be
// This header and its accompanying C file sorts all that out so we can use all the useful stuff

#if defined(_POSIX_VERSION)
#define v_strdup strdup
#elif defined(_WIN32)
#define v_strdup _strdup
#else
char *v_strdup(const char *s);
#endif

#if defined(_POSIX_VERSION) || defined(_WIN32)
#define v_strnlen strnlen
#else
size_t v_strnlen(const char *s, size_t max);
#endif

#if defined(_POSIX_VERSION)
#define v_strndup strndup
#else
char *v_strndup(const char *s, size_t len);
#endif

// This isn't defined anywhere but it should be because it's damn useful
void *v_memdup(const void *s, size_t len);

#endif
