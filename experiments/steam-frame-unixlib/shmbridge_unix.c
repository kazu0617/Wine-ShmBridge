#define _GNU_SOURCE
#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

#include "shmbridge_abi.h"

typedef int32_t NTSTATUS;
typedef NTSTATUS (*unixlib_entry_t)(void *);

static void finish(int result, int *out_result, int *out_error)
{
    int error = errno;
    *out_result = result;
    *out_error = result < 0 ? error : 0;
}

static NTSTATUS call_sem_open(void *args)
{
    struct sem_open_params *p = args;
    sem_t *result = sem_open((const char *)(uintptr_t)p->name, p->oflag, p->mode, p->value);
    int error = errno;
    p->result = (uint64_t)(uintptr_t)result;
    p->error = result == SEM_FAILED ? error : 0;
    return 0;
}

static NTSTATUS call_sem_post(void *args)
{
    struct sem_handle_params *p = args;
    finish(sem_post((sem_t *)(uintptr_t)p->handle), &p->result, &p->error);
    return 0;
}

static NTSTATUS call_sem_wait(void *args)
{
    struct sem_handle_params *p = args;
    finish(sem_wait((sem_t *)(uintptr_t)p->handle), &p->result, &p->error);
    return 0;
}

static NTSTATUS call_sem_trywait(void *args)
{
    struct sem_handle_params *p = args;
    finish(sem_trywait((sem_t *)(uintptr_t)p->handle), &p->result, &p->error);
    return 0;
}

static NTSTATUS call_sem_timedwait(void *args)
{
    struct sem_timedwait_params *p = args;
    finish(sem_timedwait((sem_t *)(uintptr_t)p->handle,
                         (const struct timespec *)(uintptr_t)p->abstime),
           &p->result, &p->error);
    return 0;
}

static NTSTATUS call_sem_unlink(void *args)
{
    struct sem_unlink_params *p = args;
    finish(sem_unlink((const char *)(uintptr_t)p->name), &p->result, &p->error);
    return 0;
}

static NTSTATUS call_sem_close(void *args)
{
    struct sem_handle_params *p = args;
    finish(sem_close((sem_t *)(uintptr_t)p->handle), &p->result, &p->error);
    return 0;
}

__attribute__((visibility("default"))) const unixlib_entry_t __wine_unix_call_funcs[] =
{
    call_sem_open, call_sem_post, call_sem_wait, call_sem_trywait,
    call_sem_timedwait, call_sem_unlink, call_sem_close,
};

__attribute__((visibility("default"))) const unixlib_entry_t __wine_unix_call_wow64_funcs[] =
{
    call_sem_open, call_sem_post, call_sem_wait, call_sem_trywait,
    call_sem_timedwait, call_sem_unlink, call_sem_close,
};
