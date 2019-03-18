#ifndef CLIENT_H_
#define CLIENT_H_

#include "target.h"
#include "serial.h"
#include "ret_value.h"
#include "fi_time.h"
#include "device.h"
typedef enum
{
    CLIENT_MODE_IDLE,
    CLIENT_MODE_SLEEP_WITH_INTERVAL,
    CLIENT_MODE_SLEEP_WITH_ALARM
}   CLIENT_MODE;

typedef struct
{
    char        id[32];
    bool        autoRun;
    uint32_t    maxRetryCount;
    struct
    {
        uint8_t     ip[4];
        uint16_t    port;
        char        userId[32];
        char        topic[64];       
    }   server;

    struct
    {
        CLIENT_MODE mode;
        struct 
        {
            uint32_t    start;
            uint32_t    period;
        } interval;
        struct
        {
            uint32_t    count;
            uint32_t    times[24];        
        } alarm;
    }   opMode;
}   CLIENT_CONFIG;

typedef enum    
{
    CLIENT_STATUS_UNINIT,
    CLIENT_STATUS_REINIT,
    CLIENT_STATUS_SYSTEM_INIT_DONE,
    CLIENT_STATUS_READ_DATA,
    
    CLIENT_STATUS_MODEM_INIT,
    CLIENT_STATUS_MODEM_WAKEUP,
    CLIENT_STATUS_MODEM_INIT_DONE,
    CLIENT_STATUS_MODEM_RESET,
    CLIENT_STATUS_MODEM_FINAL,
    CLIENT_STATUS_MODEM_DETACH,
    
    CLIENT_STATUS_TIME_SYNC_START,
    CLIENT_STATUS_TIME_SYNC,
    CLIENT_STATUS_TIME_SYNC_FINISHED,
    
    CLIENT_STATUS_SERVER_CONNECT_START,
    CLIENT_STATUS_SERVER_CONNECT,
    CLIENT_STATUS_SERVER_CONNECTED,
    
    CLIENT_STATUS_DATA_TRANSFER_START,
    CLIENT_STATUS_DATA_TRANSFER,
    CLIENT_STATUS_DATA_TRANSFER_PART1,
    CLIENT_STATUS_DATA_TRANSFER_PART2,
    CLIENT_STATUS_DATA_TRANSFER_PART3,
    CLIENT_STATUS_DATA_TRANSFER_FINISHED,
    
    CLIENT_STATUS_SOCKET_CLOSE,
    CLIENT_STATUS_FINIALIZE,
    
    CLIENT_STATUS_SET_NEXT_ALARM,
    CLIENT_STATUS_SET_NEXT_INTERVAL,
    
    CLIENT_STATUS_READY_TO_SLEEP,
    CLIENT_STATUS_GO_TO_SLEEP,
    
    CLIENT_STATUS_IDLE,
    
    CLIENT_STATUS_RESET,
    CLIENT_STATUS_MAX
}   CLIENT_STATUS;


typedef struct  
{
    FI_TIME wakeUpTime;
    FI_TIME nextWakeUpTime;
    struct
    {
        FI_TIME     dataTime;
        uint8_t     readRetryCount;
        DEVICE_DATA data;
    }   device;
    
    struct
    {
        FI_TIME     startTime;
        FI_TIME     initFinishedTime;
        uint8_t     initCount;
    }   modem;
    
    struct
    {
        FI_TIME     startTime;
        FI_TIME     finishedTime;
        uint8_t     retryCount;
    }   timeSync;
    
    struct
    {
        FI_TIME startTime;
        FI_TIME connectedTime;
        FI_TIME disconnectedTime;
        uint8_t     retryCount;
    }   server;
    
    struct
    {
        FI_TIME     startTime;
        FI_TIME     finishedTime;
        uint8_t     retryCount;
    }   dataTransfer;
    
}   CLIENT_LOG;

RET_VALUE   CLIENT_init(CLIENT_CONFIG* config);
RET_VALUE   CLIENT_getConfig(CLIENT_CONFIG* config);

bool        CLIENT_getAutoRun(void);
RET_VALUE   CLIENT_setAutoRun(bool on);

CLIENT_MODE CLIENT_getMode(void);
RET_VALUE   CLIENT_setMode(CLIENT_MODE  mode);
CLIENT_STATUS   CLIENT_getStatus(void);
const char*     CLIENT_getStatusString(CLIENT_STATUS status);

RET_VALUE   CLIENT_setServerInfo(uint8_t*   ip, uint16_t    port);
RET_VALUE   CLIENT_setIntervalStartTime(FI_TIME startTime);
uint32_t    CLIENT_getIntervalStartTime(void);
uint32_t    CLIENT_getIntervalPeriod(void);
RET_VALUE   CLIENT_setIntervalPeriod(uint32_t interval);

RET_VALUE   CLIENT_setServerIP(uint8_t *ip);
RET_VALUE   CLIENT_getServerIP(uint8_t *ip);

RET_VALUE   CLIENT_setServerPort(uint16_t port);
uint16_t    CLIENT_getServerPort(void);

RET_VALUE   CLIENT_setUserId(char* userId);
char*       CLIENT_getUserId(void);

RET_VALUE   CLIENT_setTopic(char* topic);
char*       CLIENT_getTopic(void);

RET_VALUE   CLIENT_setRetryCount(uint32_t count);
uint32_t    CLIENT_getRetryCount(void);

RET_VALUE   CLIENT_loop(void);

RET_VALUE   CLIENT_MQTT_connect(void);
RET_VALUE   CLIENT_MQTT_disconnect(void);
RET_VALUE   CLIENT_MQTT_pub(char *topic, char *message);


RET_VALUE   CLIENT_goToSleep(void);

RET_VALUE   CLIENT_deleteAllAlarms(void);
RET_VALUE   CLIENT_addAlarm(uint32_t alarm);
RET_VALUE   CLIENT_delAlarm(uint32_t index);
uint32_t    CLIENT_getAlarmCount(void);
RET_VALUE   CLIENT_getAlarm(uint32_t index, uint32_t *alarm);
RET_VALUE   CLIENT_getAlarms(uint32_t* alarms, uint32_t maxAlarms, uint32_t *count);
RET_VALUE   CLIENT_getNextAlarm(FI_TIME time, uint32_t *alarm);

RET_VALUE   CLIENT_setId(char* Id);
char*       CLIENT_getId(void);
#endif
