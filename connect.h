#ifndef __CONNECT_H
#define __CONNECT_H

libusb_device_handle* connect_device();
libusb_device_handle* connect_device2(int *reason);
int disconnect_device(libusb_device_handle * dh);


#endif
