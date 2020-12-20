#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/hidraw.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <libudev.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdbool.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "hyperx-cloud-flight-service.h"

uint8_t get_battery_level();
void locate_device(char* s);
void get_headset_data(void);
void something(void);

char device[50];

const char *dev_idVendor = "0951";
const char *dev_idProduct = "16c4";

int fd;
int sd;
int rc;
int sd2;
uint8_t current_battery_level;


bool g_is_muted;

void *get_interruptions (void){
   while(1){
      get_headset_data();
      /*something();*/
   }
}

int main () {
   uint8_t current_battery_level;
   HyperXOp operation_received;
   HyperXReponseU response;

   locate_device(device);
   if (!device[0]){
      return 1;
   }

   fd = open(device, O_RDWR);
   if (fd < 0) {
      perror("open");
      return 1;
   }

   pthread_t get_interruptions_thread_id;

   if(pthread_create(&get_interruptions_thread_id, NULL, &get_interruptions, NULL)) {
      fprintf(stderr, "Error creating thread\n");
      return 1;

   }

   struct sockaddr_un server_address;

   sd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sd < 0){
      perror("socket");
      return 1;
   }

   int enable = 1;
   if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed");

   memset(&server_address, 0, sizeof(server_address));
   server_address.sun_family = AF_UNIX;
   strcpy(server_address.sun_path, SOCKET_PATH);

   rc = bind(sd, (struct sockaddr *)&server_address, SUN_LEN(&server_address));
   if (rc < 0) {
      perror("bind");
      return 1;
   }
   rc = listen(sd, 10);
   if (rc< 0) {
      perror("listen");
      return 1;
   }

   while (1){
      sd2 = accept(sd, NULL, NULL);
      if (sd2 < 0) {
         perror("accept");
         break;
      }

      rc = recv(sd2, &operation_received, sizeof(HyperXOp), 0);
      if (rc < 0) {
         perror("recv");
         break;
      }

      switch (operation_received){
         case E_BATTERY_LEVEL:
            current_battery_level = get_battery_level();
            response.battery_level = current_battery_level;
            break;
         case E_VOLUME_LEVEL:

            break;
         case E_MIC_STATE:
            printf("requested mic_state: %s\n", g_is_muted? "true": "false");
            response.mic_is_muted = g_is_muted;
            break;
      }

      rc = send(sd2, &response, sizeof(HyperXReponseU), 0);
      if (rc < 0) {
         perror("send");
      }

      close(sd2);
   }
}


uint8_t get_battery_level() {
   char request_battery_packet[28] = { 0x21, 0xff, 0x05 };
   char request_battery_response[20];


   if (write(fd, request_battery_packet, sizeof(request_battery_packet)) < 0) {
      perror("write");
   }

   if (read(fd, request_battery_response, sizeof(request_battery_response)) < 0) {
      perror("read");
   }


   uint16_t battery_raw = (request_battery_response[3] << 8 ) + request_battery_response[4];
   uint8_t battery_level = (-138940 + 40 * battery_raw) / 291;
   for (int i = 0; i < sizeof(request_battery_response); ++i)
      printf("%hhx ", request_battery_response[i]);
   printf("\n");
   /*printf("\nraw: %d\n reponse[3-4]: %hhx %hhx\nBattery level: %d\n", battery_raw, request_battery_response[3], request_battery_response[4], battery_level);*/
   printf("battery_level: %d %%\n", battery_level);

   return battery_level;
}



void get_headset_data(void){
   char interruption_packet[5];

   if (read(fd, interruption_packet, sizeof(interruption_packet)) < 0) {
      perror("read");
   }



   // Read next useless packet
   if (interruption_packet[0] == 0x21){
      /*printf("Interruption\n");*/
      /*for (int i = 0; i < sizeof(interruption_packet); ++i){*/
         /*printf("%hhx ", interruption_packet[i]);*/
      /*}*/
      /*printf("\n");*/
      return;
   }

   // Change mic status packet
   if (interruption_packet[0] == 0x65) {
      int mic_flag = interruption_packet[1];
      if (mic_flag == 4){
         g_is_muted = true;
         for (int i = 0; i < IPC_SECURE_SEND_TRIES; ++i)
            system("polybar-msg hook headset-mic-status 2 1>/dev/null 2>&1");
      }
      else {
         g_is_muted = false;
         for (int i = 0; i < IPC_SECURE_SEND_TRIES; ++i)
            system("polybar-msg hook headset-mic-status 1 1>/dev/null 2>&1");
      }
      printf("Muted: %d\n", g_is_muted);

   }

   else if (interruption_packet[0] == 0x1){
      int vol_status_flag = interruption_packet[1];
      bool vol_status;
      if (vol_status_flag == 1)
         vol_status = true;
      else if (vol_status_flag == 2)
         vol_status = false;

      printf("Vol: %s\n", vol_status? "UP":"DOWN" );
   }



   /*if (interruption_packet[0] == 65){*/
   /*if (read(fd, interruption_packet, sizeof(interruption_packet)) < 0) {*/
   /*perror("read");*/
   /*}*/
   /*}*/

}

// Mute
/*Interruption*/
/*65 4 5 f 30*/
/*Interruption*/
/*21 ff 5 f 30*/
/*Interruption*/
/*65 0 5 f 30*/
/*Interruption*/
/*21 ff 5 f 33*/

/*Interruption*/
/*1 1 0 0 0*/
/*Interruption*/
/*1 0 0 0 0*/
/*Interruption*/
/*21 ff 5 f 2e*/


/*Interruption*/
/*1 2 0 0 0*/
/*Interruption*/
/*1 0 0 0 0*/
/*Interruption*/
/*21 ff 5 f 44*/


void something(void){
   char request_battery_packet [64] = {0xff, 0x09, 0x00, 0xfd, 0x04, 0x00, 0xf1, 0x05, 0x81, 0x74, 0xb4, 0x01};
   char receive_battery_packet [64] = {0};
   size_t request_size = sizeof(request_battery_packet);
   int res;

   /*int fd = open(device, O_RDWR|O_NONBLOCK);*/


   /*[> Set Feature <]*/
   res = ioctl(fd, HIDIOCSFEATURE(64), request_battery_packet);
   if (res < 0)
      perror("HIDIOCSFEATURE");
   else
      printf("ioctl HIDIOCSFEATURE returned: %d\n", res);

   /*[> Get Feature <]*/
   receive_battery_packet[0] = 0xff;
   res = ioctl(fd, HIDIOCGFEATURE(64), receive_battery_packet);
   if (res < 0) {
      perror("HIDIOCGFEATURE");
   } else {
      printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
      printf("Report data:\n\t");
      for (int i = 0; i < res; i++)
         printf("%hhx ", receive_battery_packet[i]);
      puts("\n");
   }
}


void locate_device(char* s) {
   s[0] = '\0';

   struct udev* context = udev_new();

   struct udev_enumerate* en = udev_enumerate_new(context);
   udev_enumerate_add_match_subsystem(en, "hidraw");
   udev_enumerate_scan_devices(en);

   struct udev_list_entry *devices = udev_enumerate_get_list_entry(en);
   struct udev_list_entry *entry;

   const char* syspath;
   struct udev_device* device;
   struct udev_device* parent;

   const char* dev_model_id;
   const char* dev_vendor_id;

   const char* found_dev_path = NULL;

   udev_list_entry_foreach(entry, devices) {
      syspath = udev_list_entry_get_name(entry);
      device = udev_device_new_from_syspath(context, syspath);
      parent = udev_device_get_parent_with_subsystem_devtype(device, "usb", "usb_device");

      if (parent != NULL) {
         dev_model_id = udev_device_get_property_value(parent, "ID_MODEL_ID");
         dev_vendor_id = udev_device_get_property_value(parent, "ID_VENDOR_ID");

         if (dev_model_id != NULL && dev_vendor_id != NULL &&
               strcmp(dev_idProduct, dev_model_id) == 0 && strcmp(dev_idVendor, dev_vendor_id) == 0) {

            if ((found_dev_path = udev_device_get_property_value(device, "DEVNAME")) != NULL)
               strcpy(s, found_dev_path);
         }
         udev_device_unref(parent);

      } else udev_device_unref(device);
   }

   udev_enumerate_unref(en);
   udev_unref(context);
}



