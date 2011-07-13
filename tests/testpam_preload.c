
#include <dlfcn.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __APPLE__
# define _PATH_LIBC       "libc.dylib"
#elif defined(__linux__)
# define _PATH_LIBC       "libc.so.6"
#else
# define _PATH_LIBC       "libc.so"
#endif

static void _preload_init(void) __attribute((constructor));

int (*_sys_open)(const char *pathname, int flags, ...);
FILE *(*_sys_fopen)(const char *filename, const char *mode);

static void
fatal(const char *msg)
{
	perror(msg);
	exit(1);
}

static void
_preload_init(void)
{
	void *libc;

#ifndef DL_LAZY
# define DL_LAZY RTLD_LAZY
#endif
	if (!(libc = dlopen(_PATH_LIBC, DL_LAZY))) {
		fatal("couldn't dlopen " _PATH_LIBC);
	} else if (!(_sys_open = dlsym(libc, "open"))) {
		fatal("couldn't dlsym 'open'");
	} else if (!(_sys_fopen = dlsym(libc, "fopen"))) {
		fatal("couldn't dlsym 'fopen'");
	}
}

const char *
_replace(const char *filename)
{
	if (strcmp(filename, "/etc/pam.d/testpam") == 0) {
		return ("testpam.pamd");
	} else if (strcmp(filename, "/etc/duo/pam_duo.conf") == 0) {
		return ("confs/mockduo.conf");
	}
	return (filename);
}

int
open(const char *filename, int flags, ...)
{
	return ((*_sys_open)(_replace(filename), flags));
}

FILE *
fopen(const char *filename, const char *mode)
{
	return ((*_sys_fopen)(_replace(filename), mode));
}
