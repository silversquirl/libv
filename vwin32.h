#ifndef VWIN32_H
#define VWIN32_H

#ifndef _WIN32
#error "vwin32.h included on non-Windows system"
#endif

#ifndef V_INTERNAL
#error "Please do not directly include vwin32.h, it is only for internal use"
#endif

// This header provides helper stuff for WIN32

#include <windows.h>
#include <stdint.h>
#include <limits.h>

// Some Windows APIs use numbers separated into a low and high DWORD. This macro converts that into an actual usable integer.
#define _v_win32_highlow_conv(newtype, high, low) ((newtype)(high) << (CHAR_BIT*sizeof (low)) | (newtype)(low))

// This kinda belongs in vtime.c, but it's also used from vfs.c so it's here
#include "vtime.h"
#define _v_WIN32_FT_TO_UNIX_EPOCH 116444736000000000L
#define _v_WIN32_FT_TICKS_PER_SEC (V_NS_PER_SEC / 100)
// Converts from a Windows FILETIME into a v_ntime_t
// ftime is evaluated more than once; don't wrap a function call with this
// Sidenote: FILETIME is a double-whammy of Win32 stupidity: a stupid encoding on top of a stupid encoding. 100ns intervals WHY? Probably so it can fit in a 64-bit integer, but it's better to just do what POSIX and vtime do and store the nanosecond component separately. On top of that, Win32 thinks it's running on some ancient machine from the dawn of time that doesn't support 64-bit integers, so it encodes it in a stupid DWORD pair. </rant>
static inline v_ntime_t _v_win32_ftime_to_ntime(FILETIME ftime) {
	uint64_t time = _v_win32_highlow_conv(uint64_t, ftime.dwHighDateTime, ftime.dwLowDateTime);
	return (v_ntime_t){
		time / _v_WIN32_FT_TICKS_PER_SEC - _v_WIN32_FT_TO_UNIX_EPOCH,
		time % _v_WIN32_FT_TICKS_PER_SEC,
	};
}

#endif
