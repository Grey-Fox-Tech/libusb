#include "../usb.h"
#include <asm/byteorder.h>
#include <errno.h>
#include <linux/usbdevice_fs.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#define TIMEOUT 1000

static int __usb_bulk_msg(usb_dev_t fd, struct usbdevfs_urb* uurb)
{
    int res;
    do {
        res = ioctl(fd, USBDEVFS_SUBMITURB, uurb);
    } while (res == -1 && errno == EINTR);

    if (res < 0) {
        return 1;
    }

    // TODO retrieve uurb_id to caller
    uint8_t uurb_id[8];
    do {
        res = ioctl(fd, USBDEVFS_REAPURB, uurb_id);
    } while (res == -1 && errno == EINTR);

    if (res < 0)
        return 1;

    return 0;
}

// TODO review w_index, maybe can be deducted from feature_selector
int usb_clear_feature(usb_dev_t fd, uint16_t feature_selector, uint16_t w_index)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD;
    usbct.bRequest = USB_REQ_CLEAR_FEATURE;
    usbct.wValue = feature_selector;
    usbct.wIndex = w_index;
    usbct.wLength = 0;
    usbct.timeout = TIMEOUT;

    switch (feature_selector) {
    case USB_DEVICE_REMOTE_WAKEUP:
        usbct.bRequestType |= USB_RECIP_DEVICE;
        break;
    case USB_ENDPOINT_HALT:
        usbct.bRequestType |= USB_RECIP_ENDPOINT;
        break;
    case USB_DEVICE_TEST_MODE:
        usbct.bRequestType |= USB_RECIP_DEVICE;
        break;
    default:;
    }

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_get_configuration(usb_dev_t fd, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_IN;
    usbct.bRequest = USB_REQ_GET_CONFIGURATION;
    usbct.wValue = 0;
    usbct.wIndex = 0;
    usbct.wLength = 1;
    usbct.timeout = TIMEOUT;
    usbct.data = buff;

    *wlen = usbct.wLength;
    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_get_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_IN;
    usbct.bRequest = USB_REQ_GET_DESCRIPTOR;
    usbct.wValue = (uint16_t)((type << 8) | index);
    usbct.wIndex = langid;
    usbct.timeout = TIMEOUT;
    usbct.data = buff;

    switch (type) {
    case USB_DT_DEVICE:
        usbct.wLength = USB_DT_DEVICE_SIZE;
        break;
    case USB_DT_CONFIG:
        usbct.wLength = USB_DT_CONFIG_SIZE;
        break;
    case USB_DT_STRING:
        usbct.wLength = USB_MAX_STRING_LEN;
        break;
    case USB_DT_INTERFACE:
        usbct.wLength = USB_DT_INTERFACE_SIZE;
        break;
    case USB_DT_ENDPOINT:
        usbct.wLength = USB_DT_ENDPOINT_SIZE;
        break;
    case USB_DT_DEVICE_QUALIFIER:
        usbct.wLength = 32;
        break;
    case USB_DT_OTHER_SPEED_CONFIG:
        usbct.wLength = 32;
        break;
    }

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;

    // we need to ask for the complete configuration (byte 2 and 3 holds the total config size)
    if (type == USB_DT_CONFIG) {
        usbct.wLength = (buff[3] << 8) | buff[2];
        if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
            return 1;
    }
    if (wlen)
        *wlen = buff[0];
    return 0;
}

int usb_get_interface(usb_dev_t fd, uint16_t interface, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
    usbct.bRequest = USB_REQ_GET_INTERFACE;
    usbct.wValue = 0;
    usbct.wIndex = interface;
    usbct.wLength = 1;
    usbct.timeout = TIMEOUT;
    usbct.data = buff;

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;

    if (wlen)
        *wlen = usbct.wLength;
    return 0;
}

int usb_get_status(usb_dev_t fd, uint8_t recipient, uint8_t w_index, void* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | recipient;
    usbct.bRequest = USB_REQ_GET_STATUS;
    usbct.wValue = 0;
    usbct.wIndex = w_index;
    usbct.wLength = 2;
    usbct.timeout = TIMEOUT;
    usbct.data = buff;

    *wlen = usbct.wLength;
    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_set_address(usb_dev_t fd, uint16_t address)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    usbct.bRequest = USB_REQ_SET_ADDRESS;
    usbct.wValue = address;
    usbct.wIndex = 0;
    usbct.wLength = 0;
    usbct.timeout = TIMEOUT;

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_set_configuration(usb_dev_t fd, uint16_t config)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_OUT;
    usbct.bRequest = USB_REQ_SET_CONFIGURATION;
    usbct.wValue = config;
    usbct.wIndex = 0;
    usbct.wLength = 0;
    usbct.timeout = TIMEOUT;

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_set_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD;
    usbct.bRequest = USB_REQ_SET_DESCRIPTOR;
    usbct.wValue = (uint16_t)((type << 8) | index);
    usbct.wIndex = langid;
    usbct.wLength = 126;
    usbct.timeout = TIMEOUT;

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_set_interface(usb_dev_t fd, uint16_t interface, uint16_t alternate_setting)
{
    /*
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
    usbct.bRequest = USB_REQ_SET_INTERFACE;
    usbct.wValue = alternate_setting;
    usbct.wIndex = interface;
    usbct.wLength = 0;
    usbct.timeout = TIMEOUT;

    int res;
    if ((res = ioctl(fd, USBDEVFS_CONTROL, &usbct)) == -1)
        return 1;
    */
    /* using USBDEVFS_INTERFACE request generate logs */
    struct usbdevfs_setinterface __setif;
    __setif.altsetting = alternate_setting;
    __setif.interface = interface;
    if (ioctl(fd, USBDEVFS_SETINTERFACE, &__setif) == -1)
        return 1;
    return 0;
}

int usb_synch_frame(usb_dev_t fd, uint8_t endpoint, void* buff)
{
    struct usbdevfs_ctrltransfer usbct;
    usbct.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_ENDPOINT;
    usbct.bRequest = USB_REQ_SYNCH_FRAME;
    usbct.wValue = 0;
    usbct.wIndex = endpoint;
    usbct.wLength = 2;
    usbct.timeout = TIMEOUT;
    usbct.data = buff;

    if (ioctl(fd, USBDEVFS_CONTROL, &usbct) == -1)
        return 1;
    return 0;
}

int usb_detach_interface(usb_dev_t fd, uint16_t interface)
{
    struct usbdevfs_ioctl __uioctl;
    __uioctl.ifno = interface;
    __uioctl.ioctl_code = USBDEVFS_DISCONNECT;
    __uioctl.data = NULL;

    if (ioctl(fd, USBDEVFS_IOCTL, &__uioctl) == -1)
        return 1;
    return 0;
}

int usb_claim_interface(usb_dev_t fd, uint16_t interface)
{
    int __iface = interface;
    return (ioctl(fd, USBDEVFS_CLAIMINTERFACE, &__iface) == -1) ? 1 : 0;
}

int usb_release_interface(usb_dev_t fd, uint16_t interface)
{
    int __iface = interface;
    return (ioctl(fd, USBDEVFS_RELEASEINTERFACE, &__iface) == -1) ? 1 : 0;
}

int usb_get_driver(usb_dev_t fd, uint16_t interface, char* driver, size_t len)
{
    struct usbdevfs_getdriver __ugd;
    __ugd.interface = interface;

    if (ioctl(fd, USBDEVFS_GETDRIVER, &__ugd) == -1)
        return 1;

    // FIX
    size_t __dlen = strlen(__ugd.driver) + 1, __len = len < __dlen ? len : __dlen;
    memcpy(driver, __ugd.driver, __len);

    return 0;
}

int usb_bulk_send(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len)
{
    uint8_t* __data = (uint8_t*)data;
    struct usbdevfs_urb uurb = { 0 };
    uurb.type = USBDEVFS_URB_TYPE_BULK;
    uurb.endpoint = USB_DIR_OUT | endpoint;
    uurb.status = -1;
    uurb.buffer = __data;
    uurb.buffer_length = len;

    if (__usb_bulk_msg(fd, &uurb))
        return -1;

    return uurb.actual_length;
}

int usb_bulk_recv(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len)
{
    struct usbdevfs_urb uurb = { 0 };
    uurb.type = USBDEVFS_URB_TYPE_BULK;
    uurb.endpoint = USB_DIR_IN | endpoint;
    uurb.status = -1;
    uurb.buffer = data;
    uurb.buffer_length = len;

    if (__usb_bulk_msg(fd, &uurb))
        return -1;

    return uurb.actual_length;
}

int usb_get_string(usb_dev_t fd, uint8_t index, uint16_t langid, char* buff)
{
    uint8_t __buff[USB_MAX_STRING_LEN];
    char* __buffp = buff;
    int __rlen;
    if (usb_get_descriptor(fd, USB_DT_STRING, index, langid, __buff, &__rlen) != 0) {
        *__buffp = '\0';
        return 1;
    }

    int i;
    // i = 2 skips length and descriptor type
    for (i = 2; i < __rlen; ++i)
        if (__buff[i])
            *__buffp++ = __buff[i];
    *__buffp = '\0';

    return 0;
}
