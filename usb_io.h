
/* $Id: usb_io.h,v 1.14 2005/08/26 16:25:34 purbanec Exp $ */

/*

  Copyright (C) 2004 Peter Urbanec <toppy at urbanec.net>

  This file is part of puppy.

  puppy is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  puppy is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with puppy; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _USB_IO_H
#define _USB_IO_H 1


#include "usb.h"
#include <libusb/libusb.h>
#include "mjd.h"
#include "tf_bytes.h"

/* Topfield command codes */
#define FAIL                      (0x0001L)
#define SUCCESS                   (0x0002L)
#define CANCEL                    (0x0003L)

#define CMD_READY                 (0x0100L)
#define CMD_RESET                 (0x0101L)
#define CMD_TURBO                 (0x0102L)

#define CMD_HDD_SIZE              (0x1000L)
#define DATA_HDD_SIZE             (0x1001L)

#define CMD_HDD_DIR               (0x1002L)
#define DATA_HDD_DIR              (0x1003L)
#define DATA_HDD_DIR_END          (0x1004L)

#define CMD_HDD_DEL               (0x1005L)
#define CMD_HDD_RENAME            (0x1006L)
#define CMD_HDD_CREATE_DIR        (0x1007L)

#define CMD_HDD_FILE_SEND         (0x1008L)
#define DATA_HDD_FILE_START       (0x1009L)
#define DATA_HDD_FILE_DATA        (0x100aL)
#define DATA_HDD_FILE_END         (0x100bL)

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* Number of milliseconds to wait for a packet transfer to complete. */

/* This is intentionally large enough to allow for a HDD spin up. */
#define TF_PROTOCOL_TIMEOUT 11000


#define trace(level, msg) if(verbose >= level) {print_time(); msg; }

extern int verbose;

/* 0 - disable tracing
   1 - show packet headers
   2+ - dump entire packet
 */
extern int packet_trace;

/* The maximum packet size used by the Toppy.
*/
#define MAXIMUM_PACKET_SIZE 0xFFFFL

/* The size of every packet header. */
#define PACKET_HEAD_SIZE 8

/* Format of a Topfield protocol packet */
#pragma pack(push,1)
struct tf_packet
{
    __u16 length;
    __u16 crc;
    __u32 cmd;
    __u8 data[MAXIMUM_PACKET_SIZE - PACKET_HEAD_SIZE];
};
#pragma pack(pop)

/* Topfield file descriptor data structure. */
#pragma pack(push,1)
struct typefile
{
    struct tf_datetime stamp;
    __u8 filetype;
    __u64 size;
    __u8 name[95];
    __u8 unused;
    __u32 attrib;
};
#pragma pack(pop)

void set_verbose(int verbose, int packet_trace);
void print_time(void);
int send_success(libusb_device_handle* fd);
int send_cancel(libusb_device_handle* fd);
int send_cmd_ready(libusb_device_handle* fd);
int send_cmd_reset(libusb_device_handle* fd);
int send_cmd_turbo(libusb_device_handle* fd, int turbo_on);
int send_cmd_hdd_size(libusb_device_handle* fd);
int send_cmd_hdd_dir(libusb_device_handle* fd, char *path);
int send_cmd_hdd_file_send(libusb_device_handle* fd, __u8 dir, char *path);
int send_cmd_hdd_file_send_with_offset(libusb_device_handle* fd, __u8 dir, char *path, __u64 offset);
int send_cmd_hdd_del(libusb_device_handle* fd, char *path);
int send_cmd_hdd_rename(libusb_device_handle* fd, char *src, char *dst);
int send_cmd_hdd_create_dir(libusb_device_handle* fd, char *path);

void print_packet(struct tf_packet *packet, char *prefix);

int default_timeout(void);

int get_tf_packet(libusb_device_handle* fd, struct tf_packet *packet);

int get_tf_packet1(libusb_device_handle* fd, struct tf_packet * packet, int noreply);

int get_tf_packet2(libusb_device_handle* fd, struct tf_packet *packet, int timeout, int no_reply);

int send_tf_packet(libusb_device_handle* fd, struct tf_packet *packet);

int read_device_descriptor(const libusb_device_handle* fd,
                               struct usb_device_descriptor *desc);
int read_config_descriptor(const libusb_device_handle* fd,
                               struct usb_config_descriptor *desc);
int discard_extra_desc_data(libusb_device_handle* fd, struct usb_descriptor_header *desc,
                                size_t descSize);

void print_device_descriptor(struct usb_device_descriptor *desc);
void print_config_descriptor(struct usb_config_descriptor *desc);

char *decode_error(struct tf_packet *packet);

#endif /* _USB_IO_H */
