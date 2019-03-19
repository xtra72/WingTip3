#ifndef ME_I10KL_H_
#define ME_I10KL_H_

#include "target.h"
#include "serial.h"
#include "fi_time.h"

typedef struct 
{
     SERIAL_CONFIG   serial;
    uint32_t        timeout;
}   ME_I10KL_CONFIG;

RET_VALUE   ME_I10KL_init(ME_I10KL_CONFIG* config);
RET_VALUE   ME_I10KL_final(void);

RET_VALUE   ME_I10KL_getConfig(ME_I10KL_CONFIG* config);

RET_VALUE   ME_I10KL_start(void);
RET_VALUE   ME_I10KL_stop(void);

RET_VALUE   ME_I10KL_getVersion(char* version, uint32_t maxVersionLength);

RET_VALUE   ME_I10KL_getIMEI(char* imei, uint32_t length);

RET_VALUE   ME_I10KL_attach(void);
RET_VALUE   ME_I10KL_detach(void);

RET_VALUE   ME_I10KL_createSocket(uint32_t protocol, uint16_t port, uint32_t* socket);
RET_VALUE   ME_I10KL_closeSocket(uint32_t socket);

RET_VALUE   ME_I10KL_MQTT_connect(uint16_t socket, uint8_t* ip, uint16_t port, char* id, char* userId, char* passwd);
RET_VALUE   ME_I10KL_MQTT_publish(uint16_t socket, uint8_t* ip, uint16_t port, char* topic, char* message);

RET_VALUE   ME_I10KL_send(char* message, bool confirmed);
RET_VALUE   ME_I10KL_receive(char* response, uint32_t length);

RET_VALUE   ME_I10KL_at(char* request, char* buffer, uint32_t bufferLength, uint32_t* receivedLength);

RET_VALUE   ME_I10KL_setWakeUp(bool status);
RET_VALUE   ME_I10KL_getWakeUp(bool* status);

RET_VALUE   ME_I10KL_getState0(bool* state);

RET_VALUE   ME_I10KL_setAutoSleep(bool autoSleep);
RET_VALUE   ME_I10KL_getAutoSleep(bool* autoSleep);

RET_VALUE   ME_I10KL_setTimeout(uint32_t timeout);
RET_VALUE   ME_I10KL_getTimeout(uint32_t* timeout);

RET_VALUE   ME_I10KL_getPSM(bool* status);

RET_VALUE   ME_I10KL_getTime(FI_TIME* time);

RET_VALUE   ME_I10KL_reset(void);

bool        ME_I10KL_getPower(void);
RET_VALUE   ME_I10KL_setPower(bool on);

#endif
