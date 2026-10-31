#include <windows.h>
#include <winternl.h>
#include <stdint.h>

#include "shmbridge_abi.h"

typedef uint64_t unixlib_handle_t;
typedef NTSTATUS (WINAPI *unix_dispatcher_t)(unixlib_handle_t, unsigned int, void *);

extern IMAGE_DOS_HEADER __ImageBase;
NTSYSAPI NTSTATUS NTAPI NtQueryVirtualMemory(HANDLE, const void *, int, void *, SIZE_T, SIZE_T *);
NTSYSAPI NTSTATUS NTAPI LdrGetDllHandle(const WCHAR *, ULONG *, const UNICODE_STRING *, HMODULE *);
NTSYSAPI void *NTAPI RtlFindExportedRoutineByName(HMODULE, const char *);
NTSYSAPI void NTAPI RtlSetLastWin32Error(ULONG);

static unixlib_handle_t unix_handle;
static unix_dispatcher_t unix_dispatcher;

static NTSTATUS WINAPI unix_call_missing(unixlib_handle_t handle, unsigned int code, void *args)
{
    (void)handle;
    (void)code;
    (void)args;
    return (NTSTATUS)0xc0000135; /* STATUS_DLL_NOT_FOUND */
}

static void init_unix_call(void)
{
    UNICODE_STRING name;
    HMODULE module = NULL;
    void **exported;

    NtQueryVirtualMemory(GetCurrentProcess(), &__ImageBase, 1000,
                         &unix_handle, sizeof(unix_handle), NULL);

    RtlInitUnicodeString(&name, L"ntdll.dll");
    LdrGetDllHandle(NULL, NULL, &name, &module);
    exported = module ? (void **)RtlFindExportedRoutineByName(module, "__wine_unix_call_dispatcher") : NULL;
    unix_dispatcher = exported ? (unix_dispatcher_t)*exported : unix_call_missing;
}

static NTSTATUS unix_call(unsigned int code, void *params)
{
    if (!unix_dispatcher) init_unix_call();
    return unix_dispatcher(unix_handle, code, params);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance);
        init_unix_call();
    }
    return TRUE;
}

__declspec(dllexport) void *WINAPI sem_open(const char *name, int oflag, unsigned int mode, unsigned int value)
{
    struct sem_open_params p = {(uint64_t)(uintptr_t)name, oflag, mode, value, 0, UINT64_MAX};
    if (unix_call(unix_sem_open, &p)) p.error = ERROR_DLL_NOT_FOUND;
    RtlSetLastWin32Error(p.error);
    return (void *)(uintptr_t)p.result;
}

static int handle_call(unsigned int code, void *handle)
{
    struct sem_handle_params p = {(uint64_t)(uintptr_t)handle, 0, -1};
    if (unix_call(code, &p)) p.error = ERROR_DLL_NOT_FOUND;
    RtlSetLastWin32Error(p.error);
    return p.result;
}

__declspec(dllexport) int WINAPI sem_post(void *handle) { return handle_call(unix_sem_post, handle); }
__declspec(dllexport) int WINAPI sem_wait(void *handle) { return handle_call(unix_sem_wait, handle); }
__declspec(dllexport) int WINAPI sem_trywait(void *handle) { return handle_call(unix_sem_trywait, handle); }
__declspec(dllexport) int WINAPI sem_close(void *handle) { return handle_call(unix_sem_close, handle); }

__declspec(dllexport) int WINAPI sem_timedwait(void *handle, const void *abstime)
{
    struct sem_timedwait_params p = {
        (uint64_t)(uintptr_t)handle, (uint64_t)(uintptr_t)abstime, 0, -1
    };
    if (unix_call(unix_sem_timedwait, &p)) p.error = ERROR_DLL_NOT_FOUND;
    RtlSetLastWin32Error(p.error);
    return p.result;
}

__declspec(dllexport) int WINAPI sem_unlink(const char *name)
{
    struct sem_unlink_params p = {(uint64_t)(uintptr_t)name, 0, -1};
    if (unix_call(unix_sem_unlink, &p)) p.error = ERROR_DLL_NOT_FOUND;
    RtlSetLastWin32Error(p.error);
    return p.result;
}

__declspec(dllexport) const char *WINAPI get_product_info(void) { return "wine-shmbridge-unixlib"; }
__declspec(dllexport) const char *WINAPI get_version_info(void) { return "1.0-arm64-unixlib"; }
