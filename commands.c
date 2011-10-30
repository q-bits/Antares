#include <stdio.h>
#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>



#include <libusb/libusb.h>
#include "usb_io.h"
#include "commands.h"

#define EPROTO 1

struct tf_packet packet;
struct tf_packet reply;

int do_cmd_turbo(libusb_device_handle* fd, char *state)
{
    int r;


    int turbo_on = atoi(state);

    if(0 == _stricmp("ON", state))
    {
        turbo_on = 1;
    }

    r = send_cmd_turbo(fd, turbo_on);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }

    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
            trace(1,
                  printf( "Turbo Mode: %s\n",
                          turbo_on ? "ON" : "OFF"));
            return 0;
            break;

        case FAIL:
            printf( "ERROR: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            fprintf(stdout, "ERROR: Unhandled packet\n");
    }
    return -EPROTO;
}

int do_cmd_reset(libusb_device_handle* fd)
{
    int r;

    r = send_cmd_reset(fd);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }

    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
            printf("TF5000PVRt should now reboot\n");
            return 0;
            break;

        case FAIL:
            fprintf(stderr, "ERROR: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled packet\n");
    }
    return -EPROTO;
}

int do_cmd_ready(libusb_device_handle* fd)
{
    int r;

    r = send_cmd_ready(fd);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }

    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
            printf("Device reports ready.\n");
            return 0;
            break;

        case FAIL:
            fprintf(stderr, "ERROR: Device reports %s\n",
                    decode_error(&reply));
            get_u32(&reply.data);
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled packet\n");
            return -1;
    }
    return -EPROTO;
}

int do_cancel(libusb_device_handle* fd)
{
    int r;
    r = send_cancel(fd);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }

    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
            //printf("In progress operation cancelled\n");
            return 0;
            break;

        case FAIL:
            printf("ERROR: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            printf("ERROR: Unhandled packet\n");
    }
    return -EPROTO;
}

int do_hdd_size(libusb_device_handle* fd)
{
    int r;

    r = send_cmd_hdd_size(fd);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }

    switch (get_u32(&reply.cmd))
    {
        case DATA_HDD_SIZE:
        {
            __u32 totalk = get_u32(&reply.data);
            __u32 freek = get_u32(&reply.data[4]);

            printf("Total %10u kB %7u MB %4u GB\n", totalk, totalk / 1024,
                   totalk / (1024 * 1024));
            printf("Free  %10u kB %7u MB %4u GB\n", freek, freek / 1024,
                   freek / (1024 * 1024));
            return 0;
            break;
        }

        case FAIL:
            printf("ERROR: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled packet\n");
    }
    return -EPROTO;
}

int do_hdd_dir(libusb_device_handle* fd, char *path)
{
    int r;

    r = send_cmd_hdd_dir(fd, path);
    if(r < 0)
    {
        return -EPROTO;
    }

    while(0 < get_tf_packet(fd, &reply))
    {
        switch (get_u32(&reply.cmd))
        {
            case DATA_HDD_DIR:
                decode_dir(&reply);
                send_success(fd);
                break;

            case DATA_HDD_DIR_END:
                return 0;
                break;

            case FAIL:
                fprintf(stderr, "ERROR: Device reports %s\n",
                        decode_error(&reply));
                return -EPROTO;
                break;

            default:
                fprintf(stderr, "ERROR: Unhandled packet\n");
                return -EPROTO;
        }
    }
    return -EPROTO;
}

void decode_dir(struct tf_packet *p)
{
    __u16 count =
        (get_u16(&p->length) - PACKET_HEAD_SIZE) / sizeof(struct typefile);
    struct typefile *entries = (struct typefile *) p->data;
    int i;
    time_t timestamp;

    for(i = 0; (i < count); i++)
    {
        char type;

        switch (entries[i].filetype)
        {
            case 1:
                type = 'd';
                break;

            case 2:
                type = 'f';
                break;

            default:
                type = '?';
        }

        /* This makes the assumption that the timezone of the Toppy and the system
         * that puppy runs on are the same. Given the limitations on the length of
         * USB cables, this condition is likely to be satisfied. */
        timestamp = tfdt_to_time(&entries[i].stamp);
        printf("%c %20llu %24.24s %s\n", type, get_u64(&entries[i].size),
               ctime(&timestamp), entries[i].name);
    }
}


int do_hdd_file_put(libusb_device_handle* fd, char *srcPath, char *dstPath)
{
    int result = -EPROTO;
    time_t startTime = time(NULL);
    enum
    {
        START,
        DATA,
        END,
        FINISHED
    } state;
    int src = -1;
    int r;
    int update = 0;
    struct __stat64 srcStat;
    __u64 fileSize;
    __u64 byteCount = 0;

    trace(4, fprintf(stderr, "%s\n", __FUNCTION__));

    src = _open(srcPath, _O_RDONLY | _O_BINARY);
    if(src < 0)
    {
        fprintf(stderr, "ERROR: Can not open source file: %s\n",
                strerror(errno));
        return errno;
    }

    if(0 != _fstat64(src, &srcStat))
    {
        fprintf(stderr, "ERROR: Can not examine source file: %s\n",
                strerror(errno));
        result = errno;
        goto out;
    }

    fileSize = srcStat.st_size;
    if(fileSize == 0)
    {
        fprintf(stderr, "ERROR: Source file is empty - not transfering.\n");
        result = -1;
        goto out;
    }

    r = send_cmd_hdd_file_send(fd, PUT, dstPath);
    if(r < 0)
    {
        goto out;
    }

    state = START;
    while(0 < get_tf_packet(fd, &reply))
    {
        update = (update + 1) % 16;
        switch (get_u32(&reply.cmd))
        {
            case SUCCESS:
                switch (state)
                {
                    case START:
                    {
                        /* Send start */
                        struct typefile *tf = (struct typefile *) packet.data;

                        put_u16(&packet.length, PACKET_HEAD_SIZE + 114);
                        put_u32(&packet.cmd, DATA_HDD_FILE_START);
                        time_to_tfdt64(srcStat.st_mtime, &tf->stamp);
                        tf->filetype = 2;
                        put_u64(&tf->size, srcStat.st_size);
                        strncpy((char *) tf->name, dstPath, 94);
                        tf->name[94] = '\0';
                        tf->unused = 0;
                        tf->attrib = 0;
                        trace(3,
                              fprintf(stderr, "%s: DATA_HDD_FILE_START\n",
                                      __FUNCTION__));
                        r = send_tf_packet(fd, &packet);
                        if(r < 0)
                        {
                            //fprintf(stderr, "ERROR: Incomplete send.\n");
                            goto out;
                        }
                        state = DATA;
                        break;
                    }

                    case DATA:
                    {
                        int payloadSize = sizeof(packet.data) - 9;
                        int w = _read(src, &packet.data[8], payloadSize);

                        /* Detect a Topfield protcol bug and prevent the sending of packets
                           that are a multiple of 512 bytes. */
                        if((w > 4)
                           &&
                           (((((PACKET_HEAD_SIZE + 8 + w) +
                               1) & ~1) % 0x200) == 0))
                        {
                            _lseeki64(src, -4, SEEK_CUR);
                            w -= 4;
                            payloadSize -= 4;
                        }

                        put_u16(&packet.length, PACKET_HEAD_SIZE + 8 + w);
                        put_u32(&packet.cmd, DATA_HDD_FILE_DATA);
                        put_u64(packet.data, byteCount);
                        byteCount += w;

                        /* Detect EOF and transition to END */
                        if((w < 0) || (byteCount >= fileSize))
                        {
                            state = END;
                        }

                        if(w > 0)
                        {
                            trace(3,
                                  fprintf(stderr, "%s: DATA_HDD_FILE_DATA\n",
                                          __FUNCTION__));
                            r = send_tf_packet(fd, &packet);
                            if(r < w)
                            {
                                //fprintf(stderr, "ERROR: Incomplete send.\n");
                                goto out;
                            }
                        }

                        if(!update)
                        {
                            progressStats(fileSize, byteCount, startTime);
                        }
                        break;
                    }

                    case END:
                        /* Send end */
                        put_u16(&packet.length, PACKET_HEAD_SIZE);
                        put_u32(&packet.cmd, DATA_HDD_FILE_END);
                        trace(3,
                              fprintf(stderr, "%s: DATA_HDD_FILE_END\n",
                                      __FUNCTION__));
                        r = send_tf_packet(fd, &packet);
                        if(r < 0)
                        {
                            //fprintf(stderr, "ERROR: Incomplete send.\n");
                            goto out;
                        }
                        state = FINISHED;
                        break;

                    case FINISHED:
                        result = 0;
                        goto out;
                        break;
                }
                break;

            case FAIL:
                fprintf(stderr, "ERROR: Device reports %s\n",
                        decode_error(&reply));
                goto out;
                break;

            default:
                fprintf(stderr, "ERROR: Unhandled packet\n");
                break;
        }
    }
    finalStats(byteCount, startTime);

  out:
    _close(src);
    return result;
}

int do_hdd_file_get(libusb_device_handle* fd, char *srcPath, char *dstPath)
{
    int result = -EPROTO;
    time_t startTime = time(NULL);
    enum
    {
        START,
        DATA,
        ABORT
    } state;
    int dst = -1;
    int r;
    int update = 0;
    __u64 byteCount = 0;
    struct utimbuf mod_utime_buf = { 0, 0 };

    dst = _open(dstPath, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY);
    if(dst < 0)
    {
        fprintf(stderr, "ERROR: Can not open destination file: %s\n",
                strerror(errno));
        return errno;
    }

    r = send_cmd_hdd_file_send(fd, GET, srcPath);
    if(r < 0)
    {
        goto out;
    }

    state = START;
    while(0 < (r = get_tf_packet(fd, &reply)))
    {
        update = (update + 1) % 16;
        switch (get_u32(&reply.cmd))
        {
            case DATA_HDD_FILE_START:
                if(state == START)
                {
                    struct typefile *tf = (struct typefile *) reply.data;

                    byteCount = get_u64(&tf->size);
                    mod_utime_buf.actime = mod_utime_buf.modtime =
                        tfdt_to_time(&tf->stamp);

                    send_success(fd);
                    state = DATA;
                }
                else
                {
                    fprintf(stderr,
                            "ERROR: Unexpected DATA_HDD_FILE_START packet in state %d\n",
                            state);
                    send_cancel(fd);
                    state = ABORT;
                }
                break;

            case DATA_HDD_FILE_DATA:
                if(state == DATA)
                {
                    __u64 offset = get_u64(reply.data);
                    __u16 dataLen =
                        get_u16(&reply.length) - (PACKET_HEAD_SIZE + 8);
                    int w;

                    if(!update)
                    {
                        progressStats(byteCount, offset + dataLen, startTime);
                    }

                    if(r < get_u16(&reply.length))
                    {
                        fprintf(stderr,
                                "ERROR: Short packet %d instead of %d\n", r,
                                get_u16(&reply.length));
                        /* TODO: Fetch the rest of the packet */
                    }

                    w = _write(dst, &reply.data[8], dataLen);
                    if(w < dataLen)
                    {
                        /* Can't write data - abort transfer */
                        fprintf(stderr, "ERROR: Can not write data: %s\n",
                                strerror(errno));
                        send_cancel(fd);
                        state = ABORT;
                    }
                }
                else
                {
                    fprintf(stderr,
                            "ERROR: Unexpected DATA_HDD_FILE_DATA packet in state %d\n",
                            state);
                    send_cancel(fd);
                    state = ABORT;
                }
                break;

            case DATA_HDD_FILE_END:
                send_success(fd);
                result = 0;
                goto out;
                break;

            case FAIL:
                fprintf(stderr, "ERROR: Device reports %s\n",
                        decode_error(&reply));
                send_cancel(fd);
                state = ABORT;
                break;

            case SUCCESS:
                goto out;
                break;

            default:
                fprintf(stderr, "ERROR: Unhandled packet (cmd 0x%x)\n",
                        get_u32(&reply.cmd));
        }
    }
    _utime64(dstPath,(struct __utimbuf64 *) &mod_utime_buf);
    finalStats(byteCount, startTime);

  out:
    _close(dst);
    return result;
}


int do_hdd_del(libusb_device_handle* fd, char *path)
{
    int r;

    r = send_cmd_hdd_del(fd, path);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }
    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
            return 0;
            break;

        case FAIL:
            fprintf(stderr, "ERROR: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled packet\n");
    }
    return -EPROTO;
}

int do_hdd_rename(libusb_device_handle* fd, char *srcPath, char *dstPath)
{
    int r;

    r = send_cmd_hdd_rename(fd, srcPath, dstPath);
    if(r < 0)
    {
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
        return -EPROTO;
    }
    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
            return 0;
            break;

        case FAIL:
            fprintf(stderr, "ERROR: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled packet\n");
    }
    return -EPROTO;
}

int do_hdd_mkdir(libusb_device_handle* fd, char *path)
{
    int r;
    if (verbose) printf("do_hdd_mkdir, path=%s\n",path);
    r = send_cmd_hdd_create_dir(fd, path);
    if(r < 0)
    {
		if (verbose) printf(" send_cmd_hdd_create returned %d to do_hdd_mkdir.\n",r);
        return -EPROTO;
    }

    r = get_tf_packet(fd, &reply);
    if(r < 0)
    {
		if (verbose) printf(" get_tf_packet returned %d to do_hdd_mkdir.\n",r);
        return -EPROTO;
    }
    switch (get_u32(&reply.cmd))
    {
        case SUCCESS:
			if (verbose) printf(" Success received, in do_hdd_mkdir.\n");
            return 0;
            break;

        case FAIL:
            printf("ERROR in do_hdd_mkdir: Device reports %s\n",
                    decode_error(&reply));
            break;

        default:
            printf( "ERROR in do_hdd_mkdir: Unhandled packet.\n");
    }
    return -EPROTO;
}


void progressStats(__u64 totalSize, __u64 bytes, time_t startTime)
{
    int delta = (int)(time(NULL) - startTime);

    if(delta > 0)
    {
        double rate = (double)bytes / delta;
        int eta = (int)((totalSize - bytes) / rate);

        fprintf(stderr,
                "\r%6.2f%%, %5.2f Mbits/s, %02d:%02d:%02d elapsed, %02d:%02d:%02d remaining",
                100.0 * ((double) (bytes) / (double) totalSize),
                ((bytes * 8.0) / delta) / (1000 * 1000), delta / (60 * 60),
                (delta / 60) % 60, delta % 60, eta / (60 * 60),
                (eta / 60) % 60, eta % 60);
    }
}

void finalStats(__u64 bytes, time_t startTime)
{
    int delta = (int)(time(NULL) - startTime);

    if(delta > 0)
    {
        fprintf(stderr, "\n%.2f Mbytes in %02d:%02d:%02d (%.2f Mbits/s)\n",
                (double) bytes / (1000.0 * 1000.0),
                delta / (60 * 60), (delta / 60) % 60, delta % 60,
                ((bytes * 8.0) / delta) / (1000.0 * 1000.0));
    }
}
