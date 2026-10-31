#ifndef SHMBRIDGE_ABI_H
#define SHMBRIDGE_ABI_H

#include <stdint.h>

struct sem_open_params
{
    uint64_t name;
    int32_t oflag;
    uint32_t mode;
    uint32_t value;
    int32_t error;
    uint64_t result;
};

struct sem_handle_params
{
    uint64_t handle;
    int32_t error;
    int32_t result;
};

struct sem_timedwait_params
{
    uint64_t handle;
    uint64_t abstime;
    int32_t error;
    int32_t result;
};

struct sem_unlink_params
{
    uint64_t name;
    int32_t error;
    int32_t result;
};

enum shmbridge_unix_call
{
    unix_sem_open,
    unix_sem_post,
    unix_sem_wait,
    unix_sem_trywait,
    unix_sem_timedwait,
    unix_sem_unlink,
    unix_sem_close,
};

#endif
