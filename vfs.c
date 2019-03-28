#define V_INTERNAL
#include <stddef.h>
#include <stdlib.h>
#include "vfs.h"
#include "vmisc.h"

//////// FILE ////////

#if defined(_POSIX_VERSION)
#include <sys/stat.h>
#include <fcntl.h>

static enum v_file_error _v_posix_ferror(void) {
	// TODO
	return V_FERROR_UNKNOWN;
}

v_file_t *v_fopen(const char *path, enum v_file_mode mode) {
	v_file_t *f = malloc(sizeof *f);
	if (!f) return NULL;

	bool read = mode & V_READ;
	bool write = mode & V_WRITE;
	bool append = mode & V_APPEND;
	bool no_trunc = mode & V_NO_TRUNC;

	int pmode = 0;

	if (read && write) pmode = O_RDWR;
	else if (read) pmode = O_RDONLY;
	else if (write) pmode = O_WRONLY;

	if (append) pmode |= O_APPEND;
	else if (!read) pmode |= O_CREAT;

	int f->fd = open(path, pmode);
	if (!f->fd) {
		free(f);
		return NULL;
	}

	f->mode = mode;
	f->err = 0;
	return f;
}

void v_fclose(v_file_t *f) {
	close(f->fd);
	free(f);
}

size_t v_fread(v_file_t *restrict f, void *restrict data, size_t len) {
	if (!(f->mode & V_READ)) {
		f->err |= V_FERROR_MODE;
		return 0;
	}
	size_t rlen = read(data, 1, len, f->fd);
	if (rlen < len) f->err |= _v_posix_ferror();
	return rlen;
}

size_t v_fwrite(v_file_t *restrict f, const void *restrict data, size_t len) {
	if (!(f->mode & V_WRITE)) {
		f->err |= V_FERROR_MODE;
		return 0;
	}
	size_t wlen = write(data, 1, len, f->fd);
	if (wlen < len) f->err |= _v_posix_ferror();
	return wlen;
}

#elif defined(_WIN32)
#include <windows.h>

static enum v_file_error _v_win32_ferror(void) {
	// TODO
	return V_FERROR_UNKNOWN;
}

v_file_t *v_fopen(const char *path, enum v_file_mode mode) {
	v_file_t *f = malloc(sizeof *f);
	if (!f) return NULL;

	bool read = mode & V_READ;
	bool write = mode & V_WRITE;
	bool append = mode & V_APPEND;
	bool no_exist = mode & V_NO_EXIST;

	bool lock_read = mode & V_LOCK_READ;
	bool lock_write = mode & V_LOCK_WRITE;
	bool lock_delete = mode & V_LOCK_DELETE;

	// NO_TRUNC requires WRITE
	if (!write && no_exist) return NULL;

	f->h = CreateFileA(
		path,
		(read ? GENERIC_READ : 0) | (write ? GENERIC_WRITE : 0),
		(lock_read ? 0 : FILE_SHARE_READ) | (lock_write ? 0 : FILE_SHARE_WRITE) | (lock_delete ? 0 : FILE_SHARE_DELETE),
		NULL,
		no_exist ? CREATE_NEW : append || read ? OPEN_ALWAYS : CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (f->h == INVALID_HANDLE_VALUE) {
		free(f);
		return NULL;
	}

	f->err = 0;
	f->mode = mode;
	return f;
}

void v_fclose(v_file_t *f) {
	CloseHandle(f->h);
	free(f);
}

#define DWORD_MAX ((DWORD)~0)

size_t v_fread(v_file_t *restrict f, void *restrict vdata, size_t len) {
	if (!(f->mode & V_READ)) {
		f->err |= V_FERROR_MODE;
		return 0;
	}

	char *data = vdata;

	// The Windows file API only supports DWORDs for length. A DWORD is 32-bit, but size_t may be 64-bit, so we may need to call ReadFile multiple times.
	size_t rlen = 0;
	do {
		DWORD read_bytes;
		if (!ReadFile(f->h, data + rlen, len, &read_bytes, NULL)) {
			f->err |= _v_win32_ferror();
			return rlen;
		}
		rlen += read_bytes;
	} while ((len > DWORD_MAX) && (len -= DWORD_MAX, 1)); // This is a neat little trick for sneaking in some post-comparison code that only runs the second time round the loop

	return rlen;
}

size_t v_fwrite(v_file_t *restrict f, const void *restrict vdata, size_t len) {
	if (!(f->mode & V_WRITE)) {
		f->err |= V_FERROR_MODE;
		return 0;
	}

	const char *data = vdata;

	// The Windows file API only supports DWORDs for length. A DWORD is 32-bit, but size_t may be 64-bit, so we may need to call ReadFile multiple times.
	size_t wlen = 0;
	do {
		DWORD written_bytes;
		if (!WriteFile(f->h, data + wlen, len, &written_bytes, NULL)) {
			f->err |= _v_win32_ferror();
			return wlen;
		}
		wlen += written_bytes;
	} while ((len > DWORD_MAX) && (len -= DWORD_MAX, 1)); // This is a neat little trick for sneaking in some post-comparison code that only runs the second time round the loop

	return wlen;
}

#endif

//////// STAT ////////

#if defined(_POSIX_VERSION)
#include <sys/stat.h>
#include "vposix.h"

static inline void _v_posix_stat2stat(v_stat_t *restrict v, struct stat *restrict st) {
	v->symlink = S_ISLNK(st->st_mode);
	v->directory = S_ISDIR(st->st_mode);
	v->size = st->st_size;
	v->access_time = _v_posix_timespec_to_ntime(st->st_atim);
	v->modify_time = _v_posix_timespec_to_ntime(st->st_mtim);
}

int v_stat(const char *restrict path, v_stat_t *restrict buf) {
	struct stat stat_buf;
	if (stat(path, &stat_buf)) return -1;
	_v_posix_stat2stat(buf, &stat_buf);
	return 0;
}

int v_stat_nofollow(const char *restrict path, v_stat_t *restrict buf) {
	struct stat stat_buf;
	if (lstat(path, &stat_buf)) return -1;
	_v_posix_stat2stat(buf, &stat_buf);
	return 0;
}

int v_file_stat(v_file_t *restrict f, v_stat_t *restrict buf) {
	struct stat stat_buf;
	if (fstat(f->fd, &stat_buf)) return -1;
	_v_posix_stat2stat(buf, &stat_buf);
	return 0;
}

#elif defined(_WIN32)
#include <windows.h>
#include "vwin32.h"

// Written as a macro because there are two very similar types.
// Using a macro allows us to write the code once because the field names are the same.
#define _v_win32_attr2stat(v, attr) do { \
		(v)->symlink = false; /* TODO: make this work */ \
		(v)->directory = !!(attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); \
		(v)->size = _v_win32_highlow_conv(v_fsize_t, attr.nFileSizeHigh, attr.nFileSizeLow); \
		(v)->access_time = _v_win32_ftime_to_ntime(attr.ftLastAccessTime); \
		(v)->modify_time = _v_win32_ftime_to_ntime(attr.ftLastWriteTime); \
	} while (0)

int v_stat(const char *restrict path, v_stat_t *restrict buf) {
	WIN32_FILE_ATTRIBUTE_DATA attr;
	if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr)) return -1;
	_v_win32_attr2stat(buf, attr);
	return 0;
}

int v_stat_nofollow(const char *restrict path, v_stat_t *restrict buf) {
	return v_stat(path, buf); // TODO: make this gooder
}

int v_file_stat(v_file_t *restrict f, v_stat_t *restrict buf) {
	BY_HANDLE_FILE_INFORMATION info;
	if (!GetFileInformationByHandle(f->h, &info)) return -1;
	_v_win32_attr2stat(buf, info);
	return 0;
}

#endif

//////// DIR ////////

#if defined(_POSIX_VERSION)
#include <dirent.h>

static enum v_dir_error _v_posix_derror(void) {
	// TODO
	return V_DERROR_UNKNOWN;
}

v_dir_t *v_dopen(const char *path) {
	v_dir_t *d = malloc(sizeof *d);
	if (!d) return NULL;

	DIR *dir = opendir(path);
	if (!dir) {
		free(d);
		return NULL;
	}

	d->dir = dir;
	d->err = 0;
	return d;
}

void v_dclose(v_dir_t *d) {
	closedir(d->dir);
	free(d);
}

int v_dread(v_dir_t *restrict d, char **restrict path) {
	free(*path); // nop if NULL
	*path = NULL; // In case the caller tries to free *path

	struct dirent *result, *entry = malloc(sizeof *entry + NAME_MAX + 1);
	if (readdir_r(d->dir, entry, &result)) {
		v->err |= _v_posix_derror();
		return -1;
	}
	if (!result) return 1;

	*path = v_strdup(entry->d_name);
	free(entry);
	return 0;
}
#elif defined(_WIN32)
#include <windows.h>

static enum v_dir_error _v_win32_derror(void) {
	switch (GetLastError()) {
	case ERROR_NO_MORE_FILES:
		return 0;

	// TODO: catch more errors

	default:
		return V_DERROR_UNKNOWN;
	}
}

v_dir_t *v_dopen(const char *path) {
	v_dir_t *d = malloc(sizeof *d);
	if (!d) return NULL;

	// Windows only allows you to list directories by using a wildcard. wtf Windows
	size_t path_len = strlen(path);
	char path_wc[path_len + sizeof "\\*"];
	memcpy(path_wc, path, path_len);
	memcpy(path_wc + path_len, "\\*", sizeof "\\*");

	WIN32_FIND_DATAA data;
	d->h = FindFirstFileA(path_wc, &data);
	if (d->h == INVALID_HANDLE_VALUE) {
		free(d);
		return NULL;
	}

	d->first = v_strdup(data.cFileName);
	d->err = 0;
	return d;
}

void v_dclose(v_dir_t *d) {
	FindClose(d->h);
	free(d);
}

int v_dread(v_dir_t *restrict d, char **restrict path) {
	free(*path); // nop if NULL
	*path = NULL; // In case the caller tries to free *path

	if (d->first) {
		*path = d->first;
		d->first = NULL;
		return 0;
	}

	WIN32_FIND_DATAA data;
	if (!FindNextFileA(d->h, &data)) {
		int err = _v_win32_derror();
		// _v_win32_derror returns 0 if and only if it's ERROR_NO_MORE_FILES
		if (!err) return 1;
		d->err |= err;
		return -1;
	}

	*path = v_strdup(data.cFileName);
	return 0;
}
#endif
