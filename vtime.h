#ifndef VTIME_H
#define VTIME_H

#include <stdbool.h>
#include <stdint.h>

#if defined(_POSIX_VERSION)
#elif defined(_WIN32)
#else
#error "Unsupported operating system! v's time abstraction only supports Windows and POSIX"
#endif

// Seconds since UNIX epoch (1970-01-01 00:00:00)
// We define this instead of using time_t because the format of time_t is undefined
typedef int64_t v_time_t;

// More complex time format supporting nanosecond accuracy
// v_time_t should be favored where 1 second accuracy is sufficient
typedef struct {
	v_time_t sec;
	uint32_t ns; // Will never go over 10^12, so 32-bit is fine (2^32 is approx 4.29 * 10^12)
} v_ntime_t;

#define V_NS_PER_SEC 1000000000L
#define V_US_PER_SEC 1000000L
#define V_MS_PER_SEC 1000L

#define v_time_to_ntime(time) ((v_ntime_t){time, 0})
// WARNING: does not round, simply discards the nanosecond component
// To round see v_ntime_round
#define v_ntime_to_time(ntim) ((ntim).sec)

// Round a ntime to the closest second
v_time_t v_ntime_round(v_ntime_t t);

// Add two ntimes - nanosecond values must be valid (ie. under V_NS_PER_SEC)
v_ntime_t v_ntime_add(v_ntime_t a, v_ntime_t b);

// Find the difference between two ntimes - nanosecond values must be valid (ie. under V_NS_PER_SEC)
v_ntime_t v_ntime_sub(v_ntime_t a, v_ntime_t b);

// Convert a number of nanoseconds to a ntime
v_ntime_t v_ntime_ns(uint64_t ns);

// Convert a number of microseconds to a ntime
v_ntime_t v_ntime_us(uint64_t us);

// Convert a number of milliseconds to a ntime
v_ntime_t v_ntime_ms(uint64_t ms);

// Get the current time
v_time_t v_time_now(void);

// Get the current time
// WARNING: the system clock may not have nanosecond accuracy, so there may be a margin of error in the returned time
v_ntime_t v_ntime_now(void);

// Get the current monotonic time
// WARNING: the monotonic clock may not have nanosecond accuracy
v_ntime_t v_ntime_mono(void);

// Convert a time into local timezone calendar components
struct tm v_time_cal_local(v_time_t t);

// Convert a time into UTC calendar components
struct tm v_time_cal_utc(v_time_t t);

// Pause execution of the current thread for the duration specified by a time
void v_time_sleep(v_time_t t);

// Pause execution of the current thread for the duration specified by a ntime
// WARNING: the underlying API used may not have nanosecond accuracy, and even if it does, scheduling decisions may mean that the process does not sleep for exactly the amount of time required. However, it will never sleep for less time than specified.
void v_ntime_sleep(v_ntime_t t);

// If you need nanosecond-precision in your dates, you can pass the sec field into one of the v_time_cal_ functions and then combine that with the ns field

#endif
