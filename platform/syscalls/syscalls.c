/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

int errno;
void *__dso_handle __attribute__((weak));

// ----------------------------------------------------------------------------

#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utime.h>
#include <limits.h>
#include <signal.h>

void __initialize_args(int *p_argc, char ***p_argv);

// This is the standard default implementation for the routine to
// process args. It returns a single empty arg.
// For semihosting applications, this is redefined to get the real
// args from the debugger. You can also use it if you decide to keep
// some args in a non-volatile memory.

void __attribute__((weak))
__initialize_args(int *p_argc, char ***p_argv)
{
  // By the time we reach this, the data and bss should have been initialised.

  // The strings pointed to by the argv array shall be modifiable by the
  // program, and retain their last-stored values between program startup
  // and program termination. (static, no const)
  static char name[] = "";

  // The string pointed to by argv[0] represents the program name;
  // argv[0][0] shall be the null character if the program name is not
  // available from the host environment. argv[argc] shall be a null pointer.
  // (static, no const)
  static char *argv[2] =
      {name, NULL};

  *p_argc = 1;
  *p_argv = &argv[0];
}

// These functions are defined here to avoid linker errors in freestanding
// applications. They might be called in some error cases from library
// code.
//
// If you detect other functions to be needed, just let us know
// and we'll add them.

__attribute__((weak)) int
raise(int sig __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

__attribute__((weak)) int
kill(pid_t pid __attribute__((unused)), int sig __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

// ----------------------------------------------------------------------------

// If you need the empty definitions, remove the -ffreestanding option.

// Forward declarations
int _chown(const char *path, uid_t owner, gid_t group);

int _close(int fildes);

int _execve(char *name, char **argv, char **env);

int _fork(void);

int _fstat(int fildes, struct stat *st);

int _getpid(void);

int _gettimeofday(struct timeval *ptimeval, void *ptimezone);

int _isatty(int file);

int _kill(int pid, int sig);

int _link(char *existing, char *_new);

int _lseek(int file, int ptr, int dir);

int _readlink(const char *path, char *buf, size_t bufsize);

int _symlink(const char *path1, const char *path2);

clock_t _times(struct tms *buf);

int _wait(int *status);

// Definitions

int __attribute__((weak))
_chown(const char *path __attribute__((unused)),
       uid_t owner __attribute__((unused)), gid_t group __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_execve(char *name __attribute__((unused)), char **argv __attribute__((unused)),
        char **env __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_fork(void)
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_fstat(int fildes __attribute__((unused)),
       struct stat *st __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_getpid(void)
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_gettimeofday(struct timeval *ptimeval __attribute__((unused)),
              void *ptimezone __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_isatty(int file __attribute__((unused)))
{
  errno = ENOSYS;
  return 0;
}

int __attribute__((weak))
_kill(int pid __attribute__((unused)), int sig __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_link(char *existing __attribute__((unused)),
      char *_new __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_readlink(const char *path __attribute__((unused)),
          char *buf __attribute__((unused)), size_t bufsize __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int __attribute__((weak))
_symlink(const char *path1 __attribute__((unused)),
         const char *path2 __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

clock_t __attribute__((weak))
_times(struct tms *buf __attribute__((unused)))
{
  errno = ENOSYS;
  return ((clock_t)-1);
}

int __attribute__((weak))
_wait(int *status __attribute__((unused)))
{
  errno = ENOSYS;
  return -1;
}

int	lstat (const char *__restrict __path, struct stat *__restrict __buf ){
    errno = ENOSYS;
    return -1;
}

int utime(const char *filename, const struct utimbuf *times){
    errno = ENOSYS;
    return -1;
}
