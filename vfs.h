#ifndef VFS_H
#define VFS_H

#include <stdbool.h>
#include <stdint.h>

#if defined(_POSIX_VERSION)
#include <dirent.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported operating system! v's filesystem abstraction only supports Windows and POSIX"
#endif

#include "vtime.h"

typedef uint64_t v_fsize_t;

enum v_file_mode {
	V_READ = 1<<0,
	V_WRITE = 1<<1,
	V_READWRITE = V_READ | V_WRITE,

	V_APPEND = 1<<2,

	// With this option, it is an error if the file to be opened exists
	// It is an error to use this option without V_WRITE
	V_NO_EXIST = 1<<3,

	// Ignored on non-Windows systems
	V_LOCK_READ = 1<<10,
	V_LOCK_WRITE = 1<<11,
	V_LOCK_DELETE = 1<<12,
	V_LOCK_ALL = V_LOCK_READ | V_LOCK_WRITE | V_LOCK_DELETE,
};

enum v_file_error {
	V_FERROR_NONE = 0,
	V_FERROR_MODE = 1<<0,
	V_FERROR_UNKNOWN = 1<<31,
};

typedef struct {
#if defined(_POSIX_VERSION)
	int fd;
#elif defined(_WIN32)
	HANDLE h;
#endif
	enum v_file_mode mode;
	enum v_file_error err;
} v_file_t;

v_file_t *v_fopen(const char *path, enum v_file_mode mode);
void v_fclose(v_file_t *f);
size_t v_fread(v_file_t *restrict f, void *restrict data, size_t len);
size_t v_fwrite(v_file_t *restrict f, const void *restrict data, size_t len);

typedef struct {
	bool symlink;
	bool directory;
	v_fsize_t size;
	v_ntime_t access_time;
	v_ntime_t modify_time;
} v_stat_t;

// Same semantics as POSIX stat
int v_stat(const char *restrict path, v_stat_t *restrict buf);

// Same semantics as POSIX lstat
int v_stat_nofollow(const char *restrict path, v_stat_t *restrict buf);

// Same semantics as POSIX fstat
int v_fstat(v_file_t *restrict f, v_stat_t *restrict buf);

enum v_dir_error {
	V_DERROR_NONE = 0,
	V_DERROR_UNKNOWN = 1<<31,
};

typedef struct {
#if defined(_POSIX_VERSION)
	DIR *dir;
#elif defined(_WIN32)
	HANDLE h;
	char *first; // Windows' API returns an entry when opening, so we store it here until v_dread is called
#endif
	enum v_dir_error err;
} v_dir_t;

v_dir_t *v_dopen(const char *path);
void v_dclose(v_dir_t *d);
// Returns 0 on success, 1 on end-of-directory and -1 on error after setting d->err
// If *path is not NULL, it will be freed
// *path must be freed when it is finished with
int v_dread(v_dir_t *restrict d, char **restrict path);

/*
Common usage of v_dread:

	v_dir_t *d = v_dopen(my_dir_path);
	if (!d) return 1;

	char *path = NULL;
	while (!v_dread(d, &path)) {
		printf("%s\n", path);
	}
	int err = d->err;
	v_dclose(d);
	if (err) return 1;

This nice little loop works for a number of reasons:

 1. v_dread frees path if it is not NULL
 2. v_dread returns 0 if and only if the function is successful
 3. If an error occurs, the error code is stored in the struct, not returned from the function

Important things to note:

 - path must be set to NULL before the loop, otherwise the behaviour is undefined according to the C standard
 - free(path) is not needed, because v_dread frees path even if it returns nonzero
 - d is freed when closing, so the error value must be saved beforehand
 - d should be closed even if an error occurs
*/

#endif
