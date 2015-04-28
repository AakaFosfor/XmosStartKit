/******************************************************************************
 USB Asychronous Bulk Transfer Benchmark for XMOS StartKit  

 Use firmware code from app_custom_bulk_demo (usb-1.2).  

 compile with flag: -lusb-1.0

 Ajith Peter & Rakesh Peter
 Amrita University 2014
*******************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <sys/timeb.h>

#define VID 0x20b1
#define PID 0x00b1
#define NUMTRANSFERS 4	     // Playing around with these two parameters will probably help you 
#define BUFFER_SIZE 65536    // optimize the transfer speeds. I could peak @ 45114982.400000 bytes / second. 
#define DELAY 1000

libusb_context* ctx = NULL;
libusb_device** devs;
libusb_device_handle* deviceHandle;

struct libusb_transfer** transfers = NULL;

pthread_t transfer_thread;

int start = 0;
unsigned long count;


int openUSBDevice()
{
  ssize_t deviceCount = 0;

  /* Initialize USB Context */
  if (libusb_init(&ctx) < 0)
  {
    printf("Error initializing libusb.  Exitting...\n");
    return -1;
  }

  /* Set Debug Level to 3 */
  libusb_set_debug(ctx, 3);

  /* Get the device list and print the number of devices */
  deviceCount = libusb_get_device_list(ctx, &devs);
  printf("%d devices in list....\n", deviceCount);

  /* Obtain a handle to the device of interest */
  deviceHandle = libusb_open_device_with_vid_pid(ctx, VID, PID);
  if (deviceHandle == NULL)
  {
    printf("Opening device failed...\n");
    libusb_exit(ctx);
    return -1;
  }

  /* Check if the Kernel Device Driver is active and attempt to detach interface */
  if (libusb_kernel_driver_active(deviceHandle, 0) == 1)
  {
    printf("Kernel driver active...\n");
    if (libusb_detach_kernel_driver(deviceHandle, 0) == 0)
            printf("Kernel Driver Detached...\n");
  }

  /* Claim USB Interface */
  if (libusb_claim_interface(deviceHandle, 0) < 0)
  {
    printf("Could not claim interface...\n");
    libusb_close(deviceHandle);
    libusb_exit(ctx);
    return -1;
  }
}

static int getMilliCount()
{
  struct timeb tb;
  ftime(&tb);
  int startTime = tb.millitm + (tb.time & 0xfffff) * 1000;
  return startTime;
}

static int getMilliSpan( int startTime )
{
  int milliSpan = getMilliCount() - startTime;
  if (milliSpan < 0) milliSpan += 0x100000 * 1000;
  return milliSpan;
}

static void libusbCallback(struct libusb_transfer* transfer)
{
  if (start == 0)
  {
    start = getMilliCount();
    count = 0;
  }
  else
  {
    if (getMilliSpan(start) > 1000)
    {
      printf("%f bytes / second\n", (float)count / 1.0);
      start = getMilliCount();
      count = 0;
    }
  }

  count += transfer->actual_length;
  libusb_submit_transfer(transfer);
}

int allocateTransfers()
{
  int i;

  if (transfers == NULL)
  {
    transfers = (struct libusb_transfer**)malloc(NUMTRANSFERS * sizeof (struct libusb_transfer*));
    if (transfers == NULL)
    {
       printf("Error allocating transfers.  Exitting...\n");
       return -1;
    }

    for (i = 0; i < NUMTRANSFERS; i++)
    {
      transfers[i] = libusb_alloc_transfer(0);
      if (transfers[i] == NULL)
      {
        printf("Error allocating transfer....\n");
	return -1;
      }
      libusb_fill_bulk_transfer(transfers[i], 
		                deviceHandle, 
				0x81, 
				(unsigned char*)malloc(BUFFER_SIZE * sizeof(unsigned char)), 
				BUFFER_SIZE, 
				(libusb_transfer_cb_fn)&libusbCallback, 
				NULL, 
				0);
      if (transfers[i]->buffer == NULL)
      {
        printf("Error allocating buffer.\n");
	return -1;
      }
    }

    return 0;
  }
  else
  {
    return -1;
  }
}

int submitTransfers()
{
  int i;

  for (i = 0; i < NUMTRANSFERS; i++) libusb_submit_transfer(transfers[i]);
}

static void threadProcess(void* arg)
{
  int ret;

  printf("Started Thread...\n");
  while(1)
  {
    ret = libusb_handle_events(ctx);
  }
}

int waitForCompletion()
{
  pthread_create(&transfer_thread, NULL, (void*) &threadProcess, NULL);
  printf("Waiting for thread completion...\n");
  pthread_join(transfer_thread, NULL);
  return 0;
}

int deallocateUSBDevice()
{
}

int main(int argc, char** argv)
{
  
  if (openUSBDevice() < 0) return -1;
  printf("openUSBDevice complete...\n"); 
 
  if (allocateTransfers() < 0) return -1;
  printf("allocateTransfers() complete...\n");

  submitTransfers();
  printf("submitTransfers() complete..\n");
  
  waitForCompletion();

  deallocateUSBDevice();

  return 0;
}
