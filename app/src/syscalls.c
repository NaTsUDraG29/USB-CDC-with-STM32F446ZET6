/* ============================================================================
 * Minimal Newlib Syscalls for Bare-Metal STM32
 * Provides working _sbrk() for malloc() and I/O stubs
 * ============================================================================ */

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>

#undef errno
extern int errno;

extern char _end;    /* Defined by the linker script */
extern char _estack; /* Defined by the linker script */

void *_sbrk(ptrdiff_t incr)
{
    static char *heap_end = NULL;
    char *prev_heap_end;

    if (heap_end == NULL) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;

    /* Leave 4 KB safety margin below stack */
    if ((heap_end + incr) > (&_estack - 0x1000)) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    return len;
}

void _exit(int status)
{
    (void)status;
    while (1);
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}
