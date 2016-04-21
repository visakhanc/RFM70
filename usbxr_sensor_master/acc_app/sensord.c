
/*
General Description:
This program receives data from custom USB device whenever data is ready at the device
The data is printed on console
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#ifndef __MINGW32_VERSION
#   include <syslog.h>
#   include <sys/errno.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <usb.h>    /* this is libusb, see http://libusb.sourceforge.net/ */
#include <stdint.h>

#include "opendevice.h"
#include "../usbconfig.h"

#define CMD_READ    1
/* These are the vendor specific SETUP commands implemented by our USB device */

#ifdef __MINGW32_VERSION
#define sleep(x)    _sleep(x)
#endif

static char *filename = "sensor.csv";
static FILE *fileFp = NULL;


static void usage(char *name)
{
    fprintf(stderr, "usage: %s [-test] [-f csv-file] [-c calibration-file] [-v <vendor-id>] [-d <device-id>] [-debug]\n", name);
}


/* ------------------------------------------------------------------------- */

static void signalReopenFile(int signalNr)
{
#ifndef __MINGW32_VERSION   /* MinGW does not emulate signals */
    signal(SIGHUP, signalReopenFile);   /* re-schedule for this signal */
#endif
    if(fileFp != NULL){
        fclose(fileFp);
        fileFp = NULL;
    }
    fileFp = fopen(filename, "a");
    if(fileFp == NULL){
        printf("error opening file \"%s\" for append: [%d] %s\n", filename, errno, strerror(errno));
        exit(1);
    }
}

static char *getOptionArg(int *index, int argc, char **argv)
{
    (*index)++;
    if(*index >= argc){
        fprintf(stderr, "argument for option \"%s\" missing.\n", argv[*index - 1]);
        usage(argv[0]);
        exit(1);
    }
    return argv[*index];
}


int main(int argc, char **argv)
{
usb_dev_handle      *handle;
unsigned char       buffer[8], prev= 0;
int 				vid, pid;
const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
int                 i, nBytes;
int 				error_count = 0, packet_count = 0;
int 				acc_x, acc_y, acc_z;
	usb_init();
	
    for(i=1;i<argc;i++){
		if(strcmp(argv[i], "-f") == 0){
            filename = getOptionArg(&i, argc, argv);
        }else{
            fprintf(stderr, "option \"%s\" not recognized.\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }
	
	/* compute VID/PID from usbconfig.h so that there is a central source of information */
	vid = rawVid[1]*256 + rawVid[0];
	pid = rawPid[1]*256 + rawPid[0];
	
restart:    /* we jump back here if an error occurred */
	printf("\nWaiting for device...");	
	/* The following function is in opendevice.c: */
    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        exit(1);
    }
	printf("Device detected\n");
    if(usb_set_configuration(handle, 1) < 0){
        printf("error setting USB configuration: %s\n", usb_strerror());
    }
    if(usb_claim_interface(handle, 0) < 0){
        printf("error setting USB interface: %s\n", usb_strerror());
    }
   
    signalReopenFile(1);    /* open file */
    for(;;){
        /* wait for interrupt, set timeout to more than a week */
        nBytes = usb_interrupt_read(handle, USB_ENDPOINT_IN | 1 , (char *)buffer, sizeof(buffer), 700000 * 1000);
        if(nBytes < 0){
            printf("error in USB interrupt read: %s\n", usb_strerror());
            goto usbErrorOccurred;
        }

		if(nBytes < sizeof(buffer)) {
            printf("data format error, only %d bytes received (%d expected)\n", nBytes, sizeof(buffer));
		} else {
			packet_count++;
			acc_x = (int16_t)((buffer[1] << 8)|buffer[2]);
			acc_y = (int16_t)((buffer[3] << 8)|buffer[4]);
			acc_z = (int16_t)((buffer[5] << 8)|buffer[6]);
			printf("acc[x,y,z] = %6d,%6d,%6d\n", acc_x, acc_y, acc_z);
			if((packet_count > 1) && (buffer[0] != (unsigned char)(prev + 1)) ) {
				error_count++;
			}
			prev = buffer[0];
        }
    }
usbErrorOccurred:
    usb_close(handle);
    sleep(5);
    goto restart;
    return 0;
}

/* ------------------------------------------------------------------------- */
