#include <semaphore.h>
#include <time.h>

/*
 * winebuild writes the PE entry point as __wine_spec_dll_entry.  The
 * upstream x86_64 shim gets this from winecrt0, but this small ARM64 build
 * deliberately has no Wine development sysroot.  The bridge has no imports
 * or process-local initialization, so the minimal entry pair is sufficient.
 */
int DllMain(void *instance, unsigned long reason, void *reserved) {
    (void)instance;
    (void)reason;
    (void)reserved;
    return 1;
}

int __wine_spec_dll_entry(void *instance, unsigned long reason, void *reserved) {
    return DllMain(instance, reason, reserved);
}

sem_t *sem_open_native(const char *name, int oflag, unsigned int mode, unsigned int value) {
    return sem_open(name, oflag, mode, value);
}

int sem_post_native(sem_t *sem) { return sem_post(sem); }
int sem_wait_native(sem_t *sem) { return sem_wait(sem); }
int sem_trywait_native(sem_t *sem) { return sem_trywait(sem); }
int sem_timedwait_native(sem_t *sem, const struct timespec *abstime) {
    return sem_timedwait(sem, abstime);
}
int sem_unlink_native(const char *name) { return sem_unlink(name); }
int sem_close_native(sem_t *sem) { return sem_close(sem); }

char *get_product_info(void) {
    return "Wine-ShmBridge ARM64 for Steam Frame";
}

char *get_version_info(void) { return "1.0.0-arm64"; }
