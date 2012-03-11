/* $Id: tf_fwio.h,v 1.1 2006/01/05 04:25:10 msteveb Exp $ */

/*

  Copyright (c) 2005 Steve Bennett <msteveb at ozemail.com.au>

  This file is part of libtopfield.

  libtopfield is free software; you can redistribute it and/or modify
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
#ifndef TF_FWIO_H
#define TF_FWIO_H

/* Provides an interface to the Topfield 5000PVR USB functionality */

//#include "tf_types.h"
//#include "tf_open.h"
#include "usb.h"
#include "usb_io.h"


/* The maximum size of the data in a packet is 32768, although
   this will be split into multiple usb packets if necessary
*/
#define MAX_DATA_SIZE 0x8000

typedef struct {
	unsigned seq;
	unsigned len;		/* will be no more than MAX_DATA_SIZE */
	unsigned long offset;
	int sysid;
} tf_fw_data_t;

/**
 * Initiate uploading firmware.
 * Returns:
 * 0 if OK and details of requested block are in *fw_data
 * 1 if OK and finished
 * -1 if error
 *
 */
int tf_fw_upload(libusb_device_handle *tf, tf_fw_data_t *fw_data);

/**
 * Send data requested by tf_fw_upload() or tf_fw_upload_next()
 * Data is buf, len
 *
 * Return values are as for tf_fw_upload()
 */
int tf_fw_upload_next(libusb_device_handle *tf, const void *buf, size_t len, tf_fw_data_t *fw_data);

/**
 * Reboot via bootloader.
 * Returns 0 if OK, or -1 on error.
 */
int tf_fw_reboot(libusb_device_handle *tf);

#endif
