#include "usb.h"
#include <asm/byteorder.h>
#include <errno.h>
#include <linux/usbdevice_fs.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#define TIMEOUT 1000

static int __usb_msg(usb_dev_t fd, unsigned long request, void* _)
{
    int __res;
    do {
        __res = ioctl(fd, request, _);
    } while (__res == -1 && errno == EINTR);

    if (__res < 0) {
        return 1;
    }

    return 0;
}

static int __usb_sync_msg(usb_dev_t fd, struct usbdevfs_urb* uurb)
{
    int __res;
    __res = __usb_msg(fd, USBDEVFS_SUBMITURB, uurb);

    // TODO retrieve uurb_id to caller
    uint8_t uurb_id[8];
    __res = __usb_msg(fd, USBDEVFS_REAPURB, uurb_id);

    return 0;
}

static int __usb_async_msg(usb_dev_t fd, struct usbdevfs_urb* uurb)
{
    int __res;
    __res = __usb_msg(fd, USBDEVFS_SUBMITURB, uurb);

    // TODO retrieve uurb_id to caller
    uint8_t uurb_id[8];
    __res = __usb_msg(fd, USBDEVFS_REAPURBNDELAY, uurb_id);

    return 0;
}

// TODO review w_index, maybe can be deducted from feature_selector
int usb_clear_feature(usb_dev_t fd, uint16_t feature_selector, uint16_t w_index)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD;
    __usbct.bRequest = USB_REQ_CLEAR_FEATURE;
    __usbct.wValue = feature_selector;
    __usbct.wIndex = w_index;
    __usbct.wLength = 0;
    __usbct.timeout = TIMEOUT;

    switch (feature_selector) {
    case USB_DEVICE_REMOTE_WAKEUP:
        __usbct.bRequestType |= USB_RECIP_DEVICE;
        break;
    case USB_ENDPOINT_HALT:
        __usbct.bRequestType |= USB_RECIP_ENDPOINT;
        break;
    case USB_DEVICE_TEST_MODE:
        __usbct.bRequestType |= USB_RECIP_DEVICE;
        break;
    default:;
    }

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;
    return 0;
}

int usb_get_configuration(usb_dev_t fd, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_IN;
    __usbct.bRequest = USB_REQ_GET_CONFIGURATION;
    __usbct.wValue = 0;
    __usbct.wIndex = 0;
    __usbct.wLength = 1;
    __usbct.timeout = TIMEOUT;
    __usbct.data = buff;

    *wlen = __usbct.wLength;
    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;
    return 0;
}

int usb_get_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_IN;
    __usbct.bRequest = USB_REQ_GET_DESCRIPTOR;
    __usbct.wValue = (uint16_t)((type << 8) | index);
    __usbct.wIndex = langid;
    __usbct.timeout = TIMEOUT;
    __usbct.data = buff;

    switch (type) {
    case USB_DT_DEVICE:
        __usbct.wLength = USB_DT_DEVICE_SIZE;
        break;
    case USB_DT_CONFIG:
        __usbct.wLength = USB_DT_CONFIG_SIZE;
        break;
    case USB_DT_STRING:
        __usbct.wLength = USB_MAX_STRING_LEN;
        break;
    case USB_DT_INTERFACE:
        __usbct.wLength = USB_DT_INTERFACE_SIZE;
        break;
    case USB_DT_ENDPOINT:
        __usbct.wLength = USB_DT_ENDPOINT_SIZE;
        break;
    case USB_DT_DEVICE_QUALIFIER:
        __usbct.wLength = 32;
        break;
    case USB_DT_OTHER_SPEED_CONFIG:
        __usbct.wLength = 32;
        break;
    }

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;

    // we need to ask for the complete configuration (byte 2 and 3 holds the total config size)
    if (type == USB_DT_CONFIG) {
        __usbct.wLength = (buff[3] << 8) | buff[2];
        if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
            return 1;
    }

    if (wlen)
        *wlen = buff[0];

    return 0;
}

int usb_get_interface(usb_dev_t fd, uint16_t interface, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
    __usbct.bRequest = USB_REQ_GET_INTERFACE;
    __usbct.wValue = 0;
    __usbct.wIndex = interface;
    __usbct.wLength = 1;
    __usbct.timeout = TIMEOUT;
    __usbct.data = buff;

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;

    if (wlen)
        *wlen = __usbct.wLength;

    return 0;
}

int usb_get_status(usb_dev_t fd, uint8_t recipient, uint8_t w_index, void* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | recipient;
    __usbct.bRequest = USB_REQ_GET_STATUS;
    __usbct.wValue = 0;
    __usbct.wIndex = w_index;
    __usbct.wLength = 2;
    __usbct.timeout = TIMEOUT;
    __usbct.data = buff;

    *wlen = __usbct.wLength;
    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;

    return 0;
}

int usb_set_address(usb_dev_t fd, uint16_t address)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    __usbct.bRequest = USB_REQ_SET_ADDRESS;
    __usbct.wValue = address;
    __usbct.wIndex = 0;
    __usbct.wLength = 0;
    __usbct.timeout = TIMEOUT;

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;

    return 0;
}

int usb_set_configuration(usb_dev_t fd, uint16_t config)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_OUT;
    __usbct.bRequest = USB_REQ_SET_CONFIGURATION;
    __usbct.wValue = config;
    __usbct.wIndex = 0;
    __usbct.wLength = 0;
    __usbct.timeout = TIMEOUT;

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
        return 1;

    return 0;
}

int usb_set_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen)
{
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD;
    __usbct.bRequest = USB_REQ_SET_DESCRIPTOR;
    __usbct.wValue = (uint16_t)((type << 8) | index);
    __usbct.wIndex = langid;
    __usbct.wLength = 126;
    __usbct.timeout = TIMEOUT;

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
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
    struct usbdevfs_ctrltransfer __usbct;
    __usbct.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_ENDPOINT;
    __usbct.bRequest = USB_REQ_SYNCH_FRAME;
    __usbct.wValue = 0;
    __usbct.wIndex = endpoint;
    __usbct.wLength = 2;
    __usbct.timeout = TIMEOUT;
    __usbct.data = buff;

    if (ioctl(fd, USBDEVFS_CONTROL, &__usbct) == -1)
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
    struct usbdevfs_urb __uurb = { 0 };
    __uurb.type = USBDEVFS_URB_TYPE_BULK;
    __uurb.endpoint = USB_DIR_OUT | endpoint;
    __uurb.status = -1;
    __uurb.buffer = __data;
    __uurb.buffer_length = len;

    if (__usb_sync_msg(fd, &__uurb))
        return -1;

    return __uurb.actual_length;
}

int usb_bulk_recv(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len)
{
    struct usbdevfs_urb __uurb = { 0 };
    __uurb.type = USBDEVFS_URB_TYPE_BULK;
    __uurb.endpoint = USB_DIR_IN | endpoint;
    __uurb.status = -1;
    __uurb.buffer = data;
    __uurb.buffer_length = len;

    if (__usb_sync_msg(fd, &__uurb))
        return -1;

    return __uurb.actual_length;
}

int usb_async_send(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len)
{
    uint8_t* __data = (uint8_t*)data;
    struct usbdevfs_urb __uurb = { 0 };
    __uurb.type = USBDEVFS_URB_TYPE_ISO;
    __uurb.endpoint = USB_DIR_OUT | endpoint;
    __uurb.status = -1;
    __uurb.buffer = __data;
    __uurb.buffer_length = len;
    __uurb.usercontext = NULL;

    if (__usb_sync_msg(fd, &__uurb))
        return -1;

    return __uurb.actual_length;
}

int usb_async_recv(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len)
{
    struct usbdevfs_urb __uurb = { 0 };
    __uurb.type = USBDEVFS_URB_TYPE_ISO;
    __uurb.endpoint = USB_DIR_IN | endpoint;
    __uurb.status = -1;
    __uurb.buffer = data;
    __uurb.buffer_length = len;
    __uurb.signr = SIGUSR1;

    if (__usb_async_msg(fd, &__uurb))
        return -1;

    return __uurb.actual_length;
}

int usb_int_recv(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len)
{
    struct usbdevfs_urb __uurb = { 0 };
    __uurb.type = USBDEVFS_URB_TYPE_INTERRUPT;
    __uurb.endpoint = USB_DIR_IN | endpoint;
    __uurb.status = -1;
    __uurb.buffer = data;
    __uurb.buffer_length = len;
    __uurb.signr = SIGUSR2;

    if (__usb_async_msg(fd, &__uurb))
        return -1;

    return __uurb.actual_length;
}

int usb_get_string(usb_dev_t fd, uint8_t index, uint16_t langid, char* buff)
{
    uint8_t __buff[USB_MAX_STRING_LEN];
    char* __buffp = buff;
    int __rlen, i;

    if (usb_get_descriptor(fd, USB_DT_STRING, index, langid, __buff, &__rlen) != 0) {
        *__buffp = '\0';
        return 1;
    }

    // i = 2 skips length and descriptor type
    for (i = 2; i < __rlen; ++i)
        if (__buff[i])
            *__buffp++ = __buff[i];
    *__buffp = '\0';

    return 0;
}
