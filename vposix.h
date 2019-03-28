#ifndef VPOSIX_H
#define VPOSIX_H

#ifndef _POSIX_VERSION
#error "vposix.h included on non-POSIX system"
#endif

#ifndef V_INTERNAL
#error "Please do not directly include vposix.h, it is only for internal use"
#endif

// This header provides helper stuff for POSIX

#include <stdint.h>

// Kinda belongs in vtime.c, but is also used from vfs.c so it's here
#include <time.h>
#include "vtime.h"
static inline v_ntime_t _v_posix_timespec_to_ntime(struct timespec ts) {
	return (v_ntime_t){ts.tv_sec, ts.tv_nsec};
}

#endif