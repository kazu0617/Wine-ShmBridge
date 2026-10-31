#include <windows.h>
#include <stdio.h>

typedef void *(WINAPI *open_fn)(const char *, int, unsigned int, unsigned int);
typedef int (WINAPI *handle_fn)(void *);
typedef int (WINAPI *unlink_fn)(const char *);

int main(void)
{
    const char *name = "/resonite-shmbridge-arm64-test";
    HMODULE module = LoadLibraryA("shmbridge.dll.so");
    open_fn open_sem;
    handle_fn post_sem, wait_sem, close_sem;
    unlink_fn unlink_sem;
    void *sem;

    if (!module) return printf("LoadLibrary failed: %lu\n", GetLastError()), 1;
    open_sem = (open_fn)GetProcAddress(module, "sem_open");
    post_sem = (handle_fn)GetProcAddress(module, "sem_post");
    wait_sem = (handle_fn)GetProcAddress(module, "sem_wait");
    close_sem = (handle_fn)GetProcAddress(module, "sem_close");
    unlink_sem = (unlink_fn)GetProcAddress(module, "sem_unlink");
    if (!open_sem || !post_sem || !wait_sem || !close_sem || !unlink_sem)
        return printf("GetProcAddress failed\n"), 2;
    unlink_sem(name);
    sem = open_sem(name, 0100, 0777, 0);
    if (sem == (void *)-1) return printf("sem_open failed: %lu\n", GetLastError()), 3;
    if (post_sem(sem)) return printf("sem_post failed: %lu\n", GetLastError()), 4;
    if (wait_sem(sem)) return printf("sem_wait failed: %lu\n", GetLastError()), 5;
    if (close_sem(sem)) return printf("sem_close failed: %lu\n", GetLastError()), 6;
    if (unlink_sem(name)) return printf("sem_unlink failed: %lu\n", GetLastError()), 7;
    printf("shmbridge unixlib round-trip: OK\n");
    return 0;
}
