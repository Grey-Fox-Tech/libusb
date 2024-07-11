#include "../src/usb.h"
#include "assert.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define DEV_USB_DIR "/dev/bus/usb"
#define DEVICE "014"

int main(void)
{
    usb_dev_t dev;
    unsigned char buffer[4096] = { 0 };

    FILE* f;

    if (!(f = fopen(DEV_USB_DIR "/003/" DEVICE, "rb"))) {
        printf("FOPEN ERROR\n");
        return 1;
    }

    int c;
    while ((c = fgetc(f)) != EOF)
        printf("%.2X", c);

    fclose(f);

    int fd = open(DEV_USB_DIR "/003/" DEVICE, O_RDWR);

    printf("%d\n", fd);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    dev = fd;

    ASSERT_TEST_INIT();

    int status;

    if (usb_detach_interface(dev, 0x00) < 0) {
        printf("DETACH INTERFACE ERROR\n");
        return 1;
    }

    if (usb_claim_interface(dev, 0x00) < 0) {
        printf("CLAIM INTERFACE ERROR\n");
        return 1;
    }

    usb_int_recv(dev, 0x02, buffer, 4096);

    getchar();

    for (int i = 0; i < 4096; ++i)
        printf("%.2X", buffer[i]);
    putchar('\n');

err:

    if (usb_release_interface(dev, 0x00) < 0) {
        printf("RELEASE INTERFACE ERROR\n");
        return 1;
    }

    close(fd);

    ASSERT_TEST_SUMMARY();

    return 0;
}
