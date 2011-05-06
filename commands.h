#ifndef __COMMANDS_H
#define __COMMANDS_H


#define PUT 0
#define GET 1

//extern int quiet;
extern __u32 cmd;
extern char *arg1;
extern char *arg2;
extern __u8 sendDirection;


int do_cancel(libusb_device_handle* fd);
int do_cmd_ready(libusb_device_handle* fd);
int do_cmd_reset(libusb_device_handle* fd);
int do_hdd_size(libusb_device_handle* fd);
int do_hdd_dir(libusb_device_handle* fd, char *path);
int do_hdd_file_put(libusb_device_handle* fd, char *srcPath, char *dstPath);
int do_hdd_file_get(libusb_device_handle* fd, char *srcPath, char *dstPath);
void decode_dir(struct tf_packet *p);
int do_hdd_del(libusb_device_handle* fd, char *path);
int do_hdd_rename(libusb_device_handle* fd, char *srcPath, char *dstPath);
int do_hdd_mkdir(libusb_device_handle* fd, char *path);
int do_cmd_turbo(libusb_device_handle* fd, char *state);
void progressStats(__u64 totalSize, __u64 bytes, time_t startTime);
void finalStats(__u64 bytes, time_t startTime);


#endif __COMMANDS_H
