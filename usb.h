#ifndef __USB_INCLUDED_H__
#define __USB_INCLUDED_H__

#include <linux/usb/ch9.h>
#include <stdint.h>
#include <stdio.h>

typedef int usb_dev_t;

int usb_clear_feature(usb_dev_t fd, uint16_t feature_selector, uint16_t w_index);
int usb_get_configuration(usb_dev_t fd, uint8_t* buff, int* wlen);
int usb_get_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen);
int usb_get_interface(usb_dev_t fd, uint16_t interface, uint8_t* buff, int* wlen);
int usb_get_status(usb_dev_t fd, uint8_t recipient, uint8_t w_index, void* buff, int* wlen);
int usb_set_address(usb_dev_t fd, uint16_t address);
int usb_set_configuration(usb_dev_t fd, uint16_t config);
int usb_set_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen);
int usb_set_interface(usb_dev_t fd, uint16_t interface, uint16_t alternate_setting);
int usb_synch_frame(usb_dev_t fd, uint8_t endpoint, void* buff);
int usb_detach_interface(usb_dev_t fd, uint16_t interface);
int usb_claim_interface(usb_dev_t fd, uint16_t interface);
int usb_release_interface(usb_dev_t fd, uint16_t interface);
int usb_get_driver(usb_dev_t fd, uint16_t interface, char* driver, size_t len);
int usb_bulk_send(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len);
int usb_bulk_recv(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len);

#endif // __USB_INCLUDED_H__
