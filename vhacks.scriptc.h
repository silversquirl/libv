// A collection of useful C preprocessor hacks
// Used within libv, but also usable externally

// Expose public-facing API if the header is included without _v_HACKS_INTERNAL
#if !defined(VHACKS_H) && !defined(_v_HACKS_INTERNAL)
#define VHACKS_H

#define vhack_static_assert _v_hack_static_assert
#define vhack_count _v_hack_count

#endif

#ifndef VHACKS_INTERNAL_H
#define VHACKS_INTERNAL_H

// C99-compatible alternative to _Static_assert
#if __STDC_VERSION__ >= 201112L
#define _v_hack_static_assert(cond) _Static_assert(cond, "(" #cond ") evaluated to 0")
#else
// Neat trick adapted from the Linux kernel
#define _v_hack_static_assert(cond) ((void)sizeof (struct { int x : (cond) ? 1 : -1; }))
#endif

// A macro to count the number of arguments it is passed
// Only works up to 1024 arguments for sanity
#define _v_hack_count(...) _v_hack_count_impl(__VA_ARGS__)

// Code is generated here for _v_hack_count_impl
// See the Makefile, scripts/scriptc.sh and scripts/vhacks/count.sh for details
#pragma script ./scripts/vhacks/count.sh

#endif

#ifdef _v_HACKS_INTERNAL
#undef _v_HACKS_INTERNAL
#endif
