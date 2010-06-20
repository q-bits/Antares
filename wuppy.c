/*

  Copyright (C) 2004 Peter Urbanec, Simon Capewell <toppy at urbanec.net>

  This file is based on puppy.

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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <sys/stat.h>
#include "usb.h"
#include "io.h"
#include "usb_io.h"
#include "connect.h"
#include "commands.h"

#define WUPPY_RELEASE "1.0"

int parseArgs(int argc, char *argv[]);

int quiet = 0;
char *program_name = "Antares";
__u32 cmd = 0;
char *arg1 = NULL;
char *arg2 = NULL;
__u8 sendDirection = GET;


#define E_INVALID_ARGS 1
#define E_READ_DEVICE 2
#define E_NOT_TF5000PVR 3
#define E_SET_INTERFACE 4
#define E_CLAIM_INTERFACE 5
#define E_DEVICE_LOCK 6
#define E_LOCK_FILE 7
#define E_GLOBAL_LOCK 8
#define E_RESET_DEVICE 9


int main_old(int argc, char * argv[])
{
	int r;
	libusb_device_handle* fd;

	r = parseArgs(argc, argv);
    if(r != 0)
    {
        return E_INVALID_ARGS;
    }

	// Connect to Topfield device
	fd = connect_device();
	if (fd == NULL)
	{
		printf("ERROR: connecting to Topfield device\n");
		return 0;
	}
	printf("Topfield device connected\n");

    switch (cmd)
    {
        case CANCEL:
            r = do_cancel(fd);
            break;

        case CMD_RESET:
            r = do_cmd_reset(fd);
            break;

        case CMD_HDD_SIZE:
            r = do_hdd_size(fd);
            break;

        case CMD_HDD_DIR:
            r = do_hdd_dir(fd, arg1);
            break;

        case CMD_HDD_FILE_SEND:
            if(sendDirection == PUT)
            {
                r = do_hdd_file_put(fd, arg1, arg2);
            }
            else
            {
                r = do_hdd_file_get(fd, arg1, arg2);
            }
            break;

        case CMD_HDD_DEL:
            r = do_hdd_del(fd, arg1);
            break;

        case CMD_HDD_RENAME:
            r = do_hdd_rename(fd, arg1, arg2);
            break;

        case CMD_HDD_CREATE_DIR:
            r = do_hdd_mkdir(fd, arg1);
            break;

        case CMD_TURBO:
            r = do_cmd_turbo(fd, arg1);
            break;

        default:
            fprintf(stderr, "BUG: Command 0x%08x not implemented\n", cmd);
            r = -1;
    }

	// Disconnect from Topfield device
	disconnect_device(fd);
	printf("Topfield device disconnected\n");

	// Exit
	return 0;
}


void usage(char *myName)
{
    char *usageString =
        "Usage: %s [-pPqv] [-d <device>] -c <command> [args]\n"
        " -p             - packet header output to stderr\n"
        " -P             - full packet dump output to stderr\n"
        " -q             - quiet transfers - no progress updates\n"
        " -v             - verbose output to stderr\n"
        " -c <command>   - one of size, dir, get, put, rename, delete, mkdir, reboot, cancel, turbo\n"
        " args           - optional arguments, as required by each command\n\n"
        "Version: " WUPPY_RELEASE ", Compiled: " __DATE__ "\n";
    fprintf(stderr, usageString, myName);
}


int getopt(int nargc, char * const *nargv, const char *ostr);

int parseArgs(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind;
    int c;
	program_name = argv[0];

    while((c = getopt(argc, argv, "pPqvd:c:")) != -1)
    {
        switch (c)
        {
            case 'v':
                verbose++;
                break;

            case 'p':
                packet_trace = 1;
                break;

            case 'P':
                packet_trace = 2;
                break;

            case 'q':
                quiet = 1;
                break;

            case 'c':
                if(!_stricmp(optarg, "dir"))
                    cmd = CMD_HDD_DIR;
                else if(!_stricmp(optarg, "cancel"))
                    cmd = CANCEL;
                else if(!_stricmp(optarg, "size"))
                    cmd = CMD_HDD_SIZE;
                else if(!_stricmp(optarg, "reboot"))
                    cmd = CMD_RESET;
                else if(!_stricmp(optarg, "put"))
                {
                    cmd = CMD_HDD_FILE_SEND;
                    sendDirection = PUT;
                }
                else if(!_stricmp(optarg, "get"))
                {
                    cmd = CMD_HDD_FILE_SEND;
                    sendDirection = GET;
                }
                else if(!_stricmp(optarg, "delete"))
                    cmd = CMD_HDD_DEL;
                else if(!_stricmp(optarg, "rename"))
                    cmd = CMD_HDD_RENAME;
                else if(!_stricmp(optarg, "mkdir"))
                    cmd = CMD_HDD_CREATE_DIR;
                else if(!_stricmp(optarg, "turbo"))
                    cmd = CMD_TURBO;
                break;

            default:
                usage(argv[0]);
                return -1;
        }
    }

    if(cmd == 0)
    {
        usage(argv[0]);
        return -1;
    }

    if(cmd == CMD_HDD_DIR)
    {
        if(optind < argc)
        {
            arg1 = argv[optind];
        }
        else
        {
            arg1 = "\\";
        }
    }

    if(cmd == CMD_HDD_FILE_SEND)
    {
        if((optind + 1) < argc)
        {
            arg1 = argv[optind];
            arg2 = argv[optind + 1];
        }
        else
        {
            fprintf(stderr,
                    "ERROR: Need both source and destination names.\n");
            return -1;
        }
    }

    if(cmd == CMD_HDD_DEL)
    {
        if(optind < argc)
        {
            arg1 = argv[optind];
        }
        else
        {
            fprintf(stderr,
                    "ERROR: Specify name of file or directory to delete.\n");
            return -1;
        }
    }

    if(cmd == CMD_HDD_RENAME)
    {
        if((optind + 1) < argc)
        {
            arg1 = argv[optind];
            arg2 = argv[optind + 1];
        }
        else
        {
            fprintf(stderr,
                    "ERROR: Specify both source and destination paths for rename.\n");
            return -1;
        }
    }

    if(cmd == CMD_HDD_CREATE_DIR)
    {
        if(optind < argc)
        {
            arg1 = argv[optind];
        }
        else
        {
            fprintf(stderr, "ERROR: Specify name of directory to create.\n");
            return -1;
        }
    }

    if(cmd == CMD_TURBO)
    {
        if(optind < argc)
        {
            arg1 = argv[optind];
        }
        else
        {
            fprintf(stderr, "ERROR: Specify 0=OFF or 1=ON.\n");
            return -1;
        }
    }

    return 0;
}
