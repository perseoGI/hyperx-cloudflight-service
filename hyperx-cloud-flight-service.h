#ifndef HYPER_X_CLOUD_FLIGHT
#define HYPER_X_CLOUD_FLIGHT

#include <inttypes.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/hyperx-service"
#define IPC_SECURE_SEND_TRIES 10

typedef enum {
	E_BATTERY_LEVEL = 0,
	E_VOLUME_LEVEL,
	E_MIC_STATE,
	E_POWER_STATE
} HyperXOp;

typedef union {
	uint8_t battery_level;
	bool mic_is_muted;
} HyperXReponseU;


#endif // HYPER_X_CLOUD_FLIGHT
