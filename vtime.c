#define V_INTERNAL
#include "vtime.h"

v_time_t v_ntime_round(v_ntime_t t) {
	if (t.sec < 0) t.ns = -t.ns;
	t.ns += V_NS_PER_SEC/2;
	return t.sec + t.ns/V_NS_PER_SEC;
}

v_ntime_t v_ntime_add(v_ntime_t a, v_ntime_t b) {
	v_ntime_t res = {a.sec + b.sec, a.ns + b.ns};
	if (res.ns >= V_NS_PER_SEC) {
		res.sec++;
		res.ns -= V_NS_PER_SEC;
	}
	return res;
}

v_ntime_t v_ntime_sub(v_ntime_t a, v_ntime_t b) {
	v_ntime_t res = {a.sec - b.sec, 0};
	if (a.ns < b.ns) {
		res.sec--;
		a.ns += V_NS_PER_SEC;
	}
	res.ns = a.ns - b.ns;
	return res;
}

v_ntime_t v_ntime_ns(uint64_t ns) {
	return (v_ntime_t){
		ns / V_NS_PER_SEC,
		ns % V_NS_PER_SEC,
	};
}

v_ntime_t v_ntime_us(uint64_t us) {
	return (v_ntime_t){
		us / V_US_PER_SEC,
		us % V_US_PER_SEC,
	};
}

v_ntime_t v_ntime_ms(uint64_t ms) {
	return (v_ntime_t){
		ms / V_MS_PER_SEC,
		ms % V_MS_PER_SEC,
	};
}

#if defined(_POSIX_VERSION)
#include <time.h>
#include "vposix.h"

v_time_t v_time_now(void) {
	return time(NULL);
}

v_ntime_t v_ntime_now(void) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return _v_posix_timespec_to_ntime(ts);
}

v_ntime_t v_ntime_mono(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return _v_posix_timespec_to_ntime(ts);
}

struct tm v_time_cal_local(v_time_t t) {
	// C11 defines an optional localtime_s, but if we have POSIX we know we can use localtime_r
	struct tm;
	localtime_r(t, &tm); // FIXME: v_time_t may be bigger than time_t
	return tm;
}

struct tm v_time_cal_utc(v_time_t t) {
	struct tm;
	gmtime_r(t, &tm); // FIXME: v_time_t may be bigger than time_t
	return tm;
}

void v_time_sleep(v_time_t t) {
	v_ntime_sleep(v_time_to_ntime(t));
}

void v_ntime_sleep(v_ntime_t t) {
	struct timespec ts = {t.sec, t.ns};
	for (;;) {
		if (!nanosleep(&ts, &ts) || errno != EINTR) {
			// If errno is not EINTR, the specified time is invalid. We don't signal this to the caller because they should've checked that.
			return;
		}
	}
}

#elif defined(_WIN32)
#include <windows.h>
#include <time.h>
#include "vwin32.h"

v_time_t v_time_now(void) {
	FILETIME t;
	GetSystemTimeAsFileTime(&t);
	return v_ntime_to_time(_v_win32_ftime_to_ntime(t));
}

v_ntime_t v_ntime_now(void) {
	FILETIME t;
	GetSystemTimePreciseAsFileTime(&t);
	return _v_win32_ftime_to_ntime(t);
}

#define _v_MICROS_PER_SEC 1000000
#define _v_NS_PER_MICRO (V_NS_PER_SEC/_v_MICROS_PER_SEC)

v_ntime_t v_ntime_mono(void) {
	static LONGLONG ticks_per_sec = 0;
	if (!ticks_per_sec) {
		LARGE_INTEGER tps;
		QueryPerformanceFrequency(&tps);
		ticks_per_sec = tps.QuadPart;
	}

	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	t.QuadPart *= _v_MICROS_PER_SEC;
	t.QuadPart /= ticks_per_sec;

	return (v_ntime_t){
		t.QuadPart / _v_MICROS_PER_SEC,
		(t.QuadPart % _v_MICROS_PER_SEC) * _v_NS_PER_MICRO,
	};
}

struct tm v_time_cal_local(v_time_t t) {
	// Win32, the abomination that it is, defines a version of localtime_s that is COMPLETELY INCOMPATIBLE with the version specified by C11
	struct tm tm;
	localtime_s(&tm, &t); // FIXME: v_time_t may be bigger than time_t
	return tm;
}

struct tm v_time_cal_utc(v_time_t t) {
	struct tm tm;
	gmtime_s(&tm, &t); // FIXME: v_time_t may be bigger than time_t
	return tm;
}

void v_time_sleep(v_time_t t) {
	Sleep(t * V_MS_PER_SEC);
}

void v_ntime_sleep(v_ntime_t t) {
	uint64_t ns_component = (t.ns * V_MS_PER_SEC) / V_NS_PER_SEC;
	if (ns_component == 0) ns_component = 1; // We don't want to sleep for less time than specified
	Sleep(t.sec * V_MS_PER_SEC + ns_component);
}

#endif
