#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

/*
 * ARM64 bridge for Wine/ARM64EC.  The upstream Resonite payload includes
 * only an x86-64 ELF Wine bridge, which cannot be dlopen()ed by ARM64 Wine.
 * Keep the same exported POSIX semaphore ABI expected by
 * YellowDogMan.Cloudtoid.Interprocess.
 */

typedef void sem_t;

static void *resolve_next(const char *name)
{
    dlerror();
    return dlsym(RTLD_NEXT, name);
}

__attribute__((visibility("default")))
sem_t *sem_open(const char *name, int oflag, unsigned int mode,
                unsigned int value)
{
    typedef sem_t *(*fn_t)(const char *, int, unsigned int, unsigned int);
    fn_t fn = (fn_t)resolve_next("sem_open");
    if (!fn) {
        errno = ENOSYS;
        return (sem_t *)(intptr_t)-1;
    }
    return fn(name, oflag, mode, value);
}

#define FORWARD_ONE_INT(symbol)                                      \
    __attribute__((visibility("default"))) int symbol(sem_t *sem)   \
    {                                                               \
        typedef int (*fn_t)(sem_t *);                               \
        fn_t fn = (fn_t)resolve_next(#symbol);                       \
        if (!fn) { errno = ENOSYS; return -1; }                      \
        return fn(sem);                                              \
    }

FORWARD_ONE_INT(sem_post)
FORWARD_ONE_INT(sem_wait)
FORWARD_ONE_INT(sem_trywait)
FORWARD_ONE_INT(sem_close)

__attribute__((visibility("default")))
int sem_timedwait(sem_t *sem, const struct timespec *timeout)
{
    typedef int (*fn_t)(sem_t *, const struct timespec *);
    fn_t fn = (fn_t)resolve_next("sem_timedwait");
    if (!fn) { errno = ENOSYS; return -1; }
    return fn(sem, timeout);
}

__attribute__((visibility("default")))
int sem_unlink(const char *name)
{
    typedef int (*fn_t)(const char *);
    fn_t fn = (fn_t)resolve_next("sem_unlink");
    if (!fn) { errno = ENOSYS; return -1; }
    return fn(name);
}
