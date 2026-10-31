#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void emit(int fd, unsigned short type, unsigned short code, int value) {
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.code = code;
    event.value = value;
    if (write(fd, &event, sizeof(event)) != sizeof(event)) {
        perror("write");
    }
}

int main(void) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open /dev/uinput");
        return 1;
    }
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_F12);

    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_USB;
    setup.id.vendor = 0x1209;
    setup.id.product = 0x0001;
    snprintf(setup.name, UINPUT_MAX_NAME_SIZE, "Codex Screenshot Keyboard");
    if (ioctl(fd, UI_DEV_SETUP, &setup) < 0 || ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("uinput setup");
        close(fd);
        return 1;
    }
    sleep(2);
    emit(fd, EV_KEY, KEY_F12, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    usleep(100000);
    emit(fd, EV_KEY, KEY_F12, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    sleep(2);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
