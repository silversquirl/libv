# libv

libv is a simple set of abstractions over platform-specific APIs. It is designed to allow writing cross-platform C code with minimal effort.

Many of the APIs provided by libv are similar to their POSIX counterparts, however this is not a design requirement and the libv APIs may deviate from standards where I believe it makes things easier for the programmer (one example of this is `v_dread`, which is both thread-safe and very easy to use in a loop, unlike POSIX's `readdir` and `readdir_r`).

Similarly, while POSIX naming conventions are sometimes used, I have made an effort to make function and type names follow very regular patterns. In most cases, a function `FUNC` that operates on a type `v_TYPE_t` will be named `v_TYPE_FUNC`.

## Goals

libv has three main goals:

- To wrap platform-specific APIs in a cross-platform manner
- To create more predictable implementations of some aspects of the C and POSIX standards
- To supply helper functions for common tasks

Examples of where each of these are done:

- vfs wraps platform-specific filesystem APIs
- vtime supplies `v_time_t`, a precisely-defined implementation of the standard C and POSIX `time_t`
- vio provides helper functions for reading files into dynamically-allocated buffers of the correct size

## Bug reports

Any and all bug reports are welcome, but here's a few specific categories:

- Incorrectness - The most obvious one: if libv isn't doing what the documentation says it should, either the code or the documentation needs to be fixed.
- Nonconformance - Any place where platform-independent libv code does not strictly conform to standard C99 is incorrect, as is any incorrect or undefined usage of the POSIX, Win32 or any other APIs used by platform-specific components of libv.
- Warnings - Any warnings emitted by a C compiler when compiling libv are errors and should be fixed. To get it to temporarily compile, you may wish to disable -Werror by editing the `Makefile`.
- Performance issues - libv aims to be almost as fast as the native APIs it wraps. If you find a piece of libv is running slower than you would expect, please report it.
- Inconsistency - libv's API should be consistent. If it is not, please report this so it can be fixed in the next major release.
