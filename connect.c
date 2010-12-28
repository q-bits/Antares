// USB connect and disconnect functions
// These functions interface directly with the libusb-win32 driver

#include <stdio.h>
#include "usb.h"
#include "usb_io.h"
#include "connect.h"
#include <libusb/libusb.h>



// Connects to the device via USB using the LIBUSB-WIN32 drivers
// Returns
// - dh : if successfully connected returns the USB device handle used by all other functions in this program.
//      : if failed, just returns NULL
libusb_device_handle* connect_device2(int * reason)
{
	libusb_device_handle* dh;
	int success;
	//struct libusb_bus * bus;
	struct libusb_device *dev, *device;
	int i;
	int r;
	int cnt;
	libusb_device **devs;
	struct libusb_device_descriptor desc;
    *reason=0;
	// Initialise USB API
	//libusb_init(NULL);



	cnt = libusb_get_device_list(NULL, &devs);
	dh = NULL;
	device = NULL;
	success = 0;
	for (i=0; i<cnt; i++)
	{
		dev=devs[i];
		r = libusb_get_device_descriptor(dev, &desc);

		if (desc.idVendor==0x11db && desc.idProduct == 0x1000)
		{
			device=dev;
		}
		else {continue;};
		r=libusb_open(device,&dh);
		if (r) {
			fprintf(stderr,"New open call failed");
		    *reason=1;
			continue;
		}

		// Select configuration 0x01
		if (libusb_set_configuration(dh, 0x01))
		{
			fprintf(stderr, "connect: Select configuration failed\n");
			*reason=2;
			continue;
		}

		// Claim interface 0x00
		if (libusb_claim_interface(dh, 0x00))
		{
			fprintf(stderr, "connect: Claim interface failed\n");
			*reason=3;
			continue;
		}
		success=1; break;

	}

	// Loop for a maximum number of attempts (currently 1).
	// Note: this was set at 3 but attempting to force the Humax to do something it doesn't want to
	// do can crash the poor thing! So currently set to 1 just to make it's life a bit easier.

	//	for (i = 0; !success && i < 1; ++i)
	//{
	//	// If second or third pass then reset the device and try again (currently disabled - see above)
	//	if (i > 0 && dh != NULL)
	//	{
	//		if (libusb_reset_device(dh) != 0)
	//		{
	////			fprintf(stderr, "connect: Device reset failed\n");
	//		return NULL;
	//	}
	//Sleep(1000);
	//}

	// Enumerate busses and devices
	//libusb_find_busses();
	//libusb_find_devices();

	// Find the device
	//for (bus = libusb_get_busses(); !device && bus; bus = bus->next)
	//{
	//		for (dev = bus->devices; !device && dev; dev = dev->next)
	//			{
	//		if (dev->descriptor.idVendor == 0x11db && dev->descriptor.idProduct == 0x1000)
	//				device = dev;
	//		}
	//	}
	//	if (!device)
	//	{
	//		fprintf(stderr, "connect: No Topfield devices found\n");
	//		continue;
	//	}
	//
	//		// Open the device
	//		dh = libusb_open(device);
	//		if (dh == NULL)
	//	{
	//		fprintf(stderr, "connect: Open Topfield device failed\n");
	//		continue;
	//	}

	// Select configuration 0x01
	//	if (usb_set_configuration(dh, 0x01))
	//	{
	//		fprintf(stderr, "connect: Select configuration failed\n");
	//		continue;
	//	}

	// Claim interface 0x00
	//	if (usb_claim_interface(dh, 0x00))
	//	{
	//		fprintf(stderr, "connect: Claim interface failed\n");
	//		continue;
	//	}

	// Clear any halt conditions on the bulk transfer endpoint 0x01 (Topfield -> Host)
	/*if (usb_clear_halt(dh, 0x01))
	{
	fprintf(stderr, "connect: Clear halt condition on endpoint 0x01 failed\n");
	usb_release_interface(dh, 0x00);
	continue;
	}*/

	// Force-set connect success flag to ensure connect loop only occurs once
	//success = 1;
	//}

	// If success and device is open then close it
	if (!success && dh)
	{
		libusb_close(dh);
		dh = NULL;
	}

	// Return the device handle as success
	return dh;
}


// Connects to the device via USB using the LIBUSB-WIN32 drivers
// Returns
// - dh : if successfully connected returns the USB device handle used by all other functions in this program.
//      : if failed, just returns NULL
libusb_device_handle* connect_device()
{
	libusb_device_handle* dh;
	int success;
	//struct libusb_bus * bus;
	struct libusb_device *dev, *device;
	int i;
	int r;
	int cnt;
	libusb_device **devs;
	struct libusb_device_descriptor desc;
    int reason=0;
	// Initialise USB API
	//libusb_init(NULL);



	cnt = libusb_get_device_list(NULL, &devs);
	dh = NULL;
	device = NULL;
	success = 0;
	for (i=0; i<cnt; i++)
	{
		dev=devs[i];
		r = libusb_get_device_descriptor(dev, &desc);

		if (desc.idVendor==0x11db && desc.idProduct == 0x1000)
		{
			device=dev;
		}
		else {continue;};
		r=libusb_open(device,&dh);
		if (r) {
			fprintf(stderr,"New open call failed");
		    reason=1;
			continue;
		}

		// Select configuration 0x01
		if (libusb_set_configuration(dh, 0x01))
		{
			fprintf(stderr, "connect: Select configuration failed\n");
			reason=2;
			continue;
		}

		// Claim interface 0x00
		if (libusb_claim_interface(dh, 0x00))
		{
			fprintf(stderr, "connect: Claim interface failed\n");
			reason=3;
			continue;
		}
		success=1; break;

	}

	// Loop for a maximum number of attempts (currently 1).
	// Note: this was set at 3 but attempting to force the Humax to do something it doesn't want to
	// do can crash the poor thing! So currently set to 1 just to make it's life a bit easier.

	//	for (i = 0; !success && i < 1; ++i)
	//{
	//	// If second or third pass then reset the device and try again (currently disabled - see above)
	//	if (i > 0 && dh != NULL)
	//	{
	//		if (libusb_reset_device(dh) != 0)
	//		{
	////			fprintf(stderr, "connect: Device reset failed\n");
	//		return NULL;
	//	}
	//Sleep(1000);
	//}

	// Enumerate busses and devices
	//libusb_find_busses();
	//libusb_find_devices();

	// Find the device
	//for (bus = libusb_get_busses(); !device && bus; bus = bus->next)
	//{
	//		for (dev = bus->devices; !device && dev; dev = dev->next)
	//			{
	//		if (dev->descriptor.idVendor == 0x11db && dev->descriptor.idProduct == 0x1000)
	//				device = dev;
	//		}
	//	}
	//	if (!device)
	//	{
	//		fprintf(stderr, "connect: No Topfield devices found\n");
	//		continue;
	//	}
	//
	//		// Open the device
	//		dh = libusb_open(device);
	//		if (dh == NULL)
	//	{
	//		fprintf(stderr, "connect: Open Topfield device failed\n");
	//		continue;
	//	}

	// Select configuration 0x01
	//	if (usb_set_configuration(dh, 0x01))
	//	{
	//		fprintf(stderr, "connect: Select configuration failed\n");
	//		continue;
	//	}

	// Claim interface 0x00
	//	if (usb_claim_interface(dh, 0x00))
	//	{
	//		fprintf(stderr, "connect: Claim interface failed\n");
	//		continue;
	//	}

	// Clear any halt conditions on the bulk transfer endpoint 0x01 (Topfield -> Host)
	/*if (usb_clear_halt(dh, 0x01))
	{
	fprintf(stderr, "connect: Clear halt condition on endpoint 0x01 failed\n");
	usb_release_interface(dh, 0x00);
	continue;
	}*/

	// Force-set connect success flag to ensure connect loop only occurs once
	//success = 1;
	//}

	// If success and device is open then close it
	if (!success && dh)
	{
		libusb_close(dh);
		dh = NULL;
	}

	// Return the device handle as success
	return dh;
}

// Disconnects from the device via USB
// Inputs
// - dh    : USB device handle
// Returns
// - 1  : operation succeeded
// - 0 : operation failed
int disconnect_device(libusb_device_handle * dh)
{
	int success = 1;

	// Release interface 0x00
	if (libusb_release_interface(dh, 0x00))
	{
		success = 0;
		fprintf(stderr, "disconnect: Release interface failed 0x00\n");
	}

	// Close the device
	libusb_close(dh);
	//if (libusb_close(dh))
	//{
//		success = 0;
//		fprintf(stderr, "disconnect: Close device failed");
//	}

	// Return disconnect status
	return success;
}
