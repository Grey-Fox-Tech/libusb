#ifndef __USB_INCLUDED_H__
#define __USB_INCLUDED_H__

#include <linux/usb/ch9.h>
#include <stdint.h>
#include <stdio.h>

typedef int usb_dev_t;

/* This request is used to clear or disable a specific feature. */
int usb_clear_feature(usb_dev_t fd, uint16_t feature_selector, uint16_t w_index);

/* This request returns the current device configuration value */
int usb_get_configuration(usb_dev_t fd, uint8_t* buff, int* wlen);

/* This request returns the specified descriptor if the descriptor exists */
int usb_get_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen);

/* This request returns the selected alternate setting for the specified interface. */
int usb_get_interface(usb_dev_t fd, uint16_t interface, uint8_t* buff, int* wlen);

/* This request returns status for the specified recipient. */
int usb_get_status(usb_dev_t fd, uint8_t recipient, uint8_t w_index, void* buff, int* wlen);

/* This request sets the device address for all future device accesses. */
int usb_set_address(usb_dev_t fd, uint16_t address);

/* This request sets the device configuration. */
int usb_set_configuration(usb_dev_t fd, uint16_t config);

/* This request is optional and may be used to update existing descriptors or new descriptors may be added. */
int usb_set_descriptor(usb_dev_t fd, uint8_t type, uint8_t index, uint16_t langid, uint8_t* buff, int* wlen);

/* This request allows the host to select an alternate setting for the specified interface. */
int usb_set_interface(usb_dev_t fd, uint16_t interface, uint16_t alternate_setting);

/* This request is used to set and then report an endpoint’s synchronization frame. */
int usb_synch_frame(usb_dev_t fd, uint8_t endpoint, void* buff);

int usb_detach_interface(usb_dev_t fd, uint16_t interface);
int usb_claim_interface(usb_dev_t fd, uint16_t interface);
int usb_release_interface(usb_dev_t fd, uint16_t interface);
int usb_get_driver(usb_dev_t fd, uint16_t interface, char* driver, size_t len);

/* Send bulk data to endpoint */
int usb_bulk_send(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len);

/* Receive bulk data from endpoint */
int usb_bulk_recv(usb_dev_t fd, uint16_t endpoint, void* data, uint32_t len);

#endif // __USB_INCLUDED_H__
