/**
 * syscalls.c - Minimal syscall retarget stubs for bare-metal STM32G0
 * Using official ARM GNU Toolchain with newlib-nano, so memset/memcpy
 * etc. come from libc_nano. We only need the low-level I/O stubs.
 */
#include <sys/stat.h>
#include <errno.h>

extern char _end;           /* defined by linker script */
static char *heap_end = &_end;

void *_sbrk(int incr)
{
    char *prev = heap_end;
    heap_end += incr;
    return (void *)prev;
}

int _write(int fd, char *ptr, int len)   { (void)fd; (void)ptr; return len; }
int _read(int fd, char *ptr, int len)    { (void)fd; (void)ptr; (void)len; return 0; }
int _close(int fd)                       { (void)fd; return -1; }
int _fstat(int fd, struct stat *st)      { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)                      { (void)fd; return 1; }
int _lseek(int fd, int off, int whence)  { (void)fd; (void)off; (void)whence; return 0; }
int _kill(int pid, int sig)              { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void)                        { return 1; }
