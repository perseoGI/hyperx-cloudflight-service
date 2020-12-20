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

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "hyperx-cloud-flight-service.h"
#include <stdbool.h>


int connect_to_server(void);


int main (int argc, char *argv[]) {

	if (argc < 2){
		printf("[USAGE]: %s <Operation>[battery | mic_status | etc.]\n", argv[0]);
	}
	HyperXOp op;
	char *operation = argv[1];
	HyperXReponseU response;
	int socket_descriptor;

	if (strcmp ("battery", operation) == 0)
		op = E_BATTERY_LEVEL;
	else if (strcmp ("mic_status", operation) == 0)
		op = E_MIC_STATE;

	// TODO rest of operations

	else {
		printf ("Bad argument\n");
		return 1;
	}

	socket_descriptor = connect_to_server();

	if (send(socket_descriptor, &op, sizeof(HyperXOp), 0) < sizeof(HyperXOp)){
		perror("send");
		return 1;
	}

	if(recv(socket_descriptor, &response, sizeof(HyperXReponseU), 0) < sizeof(HyperXReponseU)) {
		perror("recv");
		return 1;
	}

	close(socket_descriptor);

	switch(op){
		case E_BATTERY_LEVEL:
			printf("%d\n", response.battery_level);
			/*printf("Battery Level: %d %% \n", response.battery_level);*/
			break;
		case E_MIC_STATE:
			printf("Mic muted: %d\n", response.mic_is_muted);
			/*printf("Mic muted: %s\n", response.mic_is_muted ? "TRUE": "FALSE");*/
			break;
	}

}


int connect_to_server(void){
	int socket_descriptor;
	struct sockaddr_un server_address;

	socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_descriptor < 0){
		perror("socket");
		return -1;
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sun_family = AF_UNIX;
	strcpy(server_address.sun_path, SOCKET_PATH);

	if (connect(socket_descriptor, (struct sockaddr *)&server_address, SUN_LEN(&server_address)) < 0 ) {
		perror("connect() failed");
		return -1;
	}
	return socket_descriptor;
}
