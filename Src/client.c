#include <math.h>
#include "client.h"
#include "trace.h"
#include "me_i10kl.h"
#include "config.h"
#include "device.h"
#include "utils.h"

#define LOOP_INTERVAL   1000
#define MODEM_INIT_TIME 20000
#define MODEM_REQUEST_RETRY_INTERVAL    10000
#define TRACE(...)  TRACE_printf("CLIENT", __VA_ARGS__)

const   char*   statusNames_[] = 
{
    "Uninit",   //  CLIENT_STATUS_UNINIT,
    "Reinit",   //  CLIENT_STATUS_REINIT,
    "Init Done",   //  CLIENT_STATUS_SYSTEM_INIT_DONE,
    "Read Data",   //  CLIENT_STATUS_READ_DATA,
    "Read Data Finished", // CLIENT_STATUS_READ_DATA_FINISHED
    
    "Modem Init",   //  CLIENT_STATUS_MODEM_INIT,
    "Modem Wakeup",   //  CLIENT_STATUS_MODEM_WAKEUP,
    "Modem Init Done",   //  CLIENT_STATUS_MODEM_INIT_DONE,
    "Modem Reset",   //  CLIENT_STATUS_MODEM_RESET,
    "Modem Final",   //  CLIENT_STATUS_MODEM_FINAL,
    "Modem Detach",       // CLIENT_STATUS_MODEM_DETACH,
    
    "Calcuate Transfer Offset", 
    
    "Time Sync Start",   //  CLIENT_STATUS_TIME_SYNC_START,
    "Time Sync",   //  CLIENT_STATUS_TIME_SYNC,
    "Time Sync Finished",   //  CLIENT_STATUS_TIME_SYNC_FINISHED,
    
    "Start server connection",
    "Server Connect",   //  CLIENT_STATUS_SERVER_CONNECT,
    "Server Connected",   //  CLIENT_STATUS_SERVER_CONNECTED,
    
    "Data transfer start",
    "Send Data",   //  CLIENT_STATUS_SEND_DATA,
    "Send Data 1/3",   //  CLIENT_STATUS_SEND_DATA1,
    "Send Data 2/3",   //  CLIENT_STATUS_SEND_DATA2,
    "Send Data 3/3",   //  CLIENT_STATUS_SEND_DATA3,
    "Data transfer finshed ",   //  CLIENT_STATUS_SEND_COMPLETED,
    
    "Socket Close",   //  CLIENT_STATUS_SOCKET_CLOSE,
    "Finailize",   //  CLIENT_STATUS_FINIALIZE,
    
    "Set Sleep Mode",   //  CLIENT_STATUS_SET_SLEEP,
    "Set Interval Mode",   //  CLIENT_STATUS_SET_INTERVAL,
    
    "Ready to sleep",   // CLIENT_STATUS_READY_TO_SLEEP,
    "Go to Sleep",   //  CLIENT_STATUS_SLEEP,
    
    "IDLE",   //  CLIENT_STATUS_IDLE,
    
    "Reset",   //  CLIENT_STATUS_RESET
};

static  CLIENT_STATUS status_ = CLIENT_STATUS_UNINIT;
static  bool        wakeUp_ = false;
static  bool        sysState_ = false;
static  bool        psmMon_ = false;
static  bool        sysWake = false;
static  uint32_t    modemInitializationTime = 15000;
static  char        buffer_[512];

static  DEVICE_DATA data_;
static  uint32_t    socket_ = 0;
static  uint32_t    retryCount_ = 0;
static  uint32_t    modemResetCount_ = 0;

static CLIENT_CONFIG  config_;
static  CLIENT_LOG      log_;

RET_VALUE   CLIENT_setStatus(CLIENT_STATUS status);

RET_VALUE   CLIENT_init(CLIENT_CONFIG* config)
{
    ASSERT(config != NULL);
    
    memcpy(&config_, config, sizeof(CLIENT_CONFIG));
    
    status_ = CLIENT_STATUS_UNINIT;
    
    return  RET_OK;
}

RET_VALUE   CLIENT_getConfig(CLIENT_CONFIG* config)
{
    ASSERT(config != NULL);
    
    memcpy(config, &config_, sizeof(CLIENT_CONFIG));
    
    return  RET_OK;
}

bool        CLIENT_getAutoRun(void)
{
    return  config_.autoRun;
}

RET_VALUE   CLIENT_setAutoRun(bool on)
{
    config_.autoRun = on;
    
    return  RET_OK;
}

CLIENT_MODE CLIENT_getMode(void)
{
    return  config_.opMode.mode;
}

RET_VALUE   CLIENT_setMode(CLIENT_MODE  mode)
{
    config_.opMode.mode = mode;
    
    return  RET_OK;
}

RET_VALUE   CLIENT_setRetryCount(uint32_t count)
{
    config_.maxRetryCount = count;
    
    return  RET_OK;
}

uint32_t    CLIENT_getRetryCount(void)
{
    return  config_.maxRetryCount;
}

CLIENT_STATUS   CLIENT_getStatus(void)
{
    return  status_;
}

const char*       CLIENT_getStatusString(CLIENT_STATUS status)
{
    if (status < CLIENT_STATUS_MAX)
    {
        return  statusNames_[status];
    }
    
    return  "Unknown";
}

RET_VALUE   CLIENT_setServerInfo(uint8_t*   ip, uint16_t    port)
{
    if (ip != NULL)
    {
        memcpy(config_.server.ip, ip, 4);
    }
    
    if (port != 0)
    {
        config_.server.port = port;
    }
    
    return  RET_OK;
}

RET_VALUE   CLIENT_setIntervalStartTime(FI_TIME startTime)
{
    config_.opMode.interval.start = startTime;
    
    return  RET_OK;
}

uint32_t    CLIENT_getIntervalStartTime(void)
{
    return  config_.opMode.interval.start;
}

uint32_t    CLIENT_getIntervalPeriod(void)
{
    return  config_.opMode.interval.period;
}

RET_VALUE   CLIENT_setIntervalPeriod(uint32_t period)
{
    config_.opMode.interval.period = period;
    
    return  RET_OK;
}


RET_VALUE   CLIENT_setServerIP(uint8_t *ip)
{
    ASSERT(ip != NULL);
    
    memcpy(config_.server.ip, ip, 4);
    
    return  RET_OK;
}

RET_VALUE   CLIENT_getServerIP(uint8_t *ip)
{
    ASSERT(ip != NULL);

    memcpy(ip, config_.server.ip, 4);
    
    return  RET_OK;
}

RET_VALUE   CLIENT_setServerPort(uint16_t port)
{
    config_.server.port = port;
    
    return  RET_OK;
}

uint16_t    CLIENT_getServerPort(void)
{
    return  config_.server.port;
}

RET_VALUE   CLIENT_setUserId(char* userId)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    ASSERT(userId != NULL);
    
    if (strlen(userId) < sizeof(config_.server.userId))
    {
        strcpy(config_.server.userId, userId);
        
        ret = RET_OK;
    }
        
    return  ret;
}

char*   CLIENT_getUserId(void)
{   
    return  config_.server.userId;
}

RET_VALUE   CLIENT_setTopic(char* topic)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    ASSERT(topic != NULL);
    
    if (strlen(topic) < sizeof(config_.server.topic))
    {
        strcpy(config_.server.topic, topic);
        
        ret = RET_OK;
    }
        
    return  ret;
}

char*   CLIENT_getTopic(void)
{
    return  config_.server.topic;
}


RET_VALUE   CLIENT_loop(void)
{
    RET_VALUE   ret;
    static  uint32_t    waitingTime = 0;
    
    if (!config_.autoRun)
    {
        return  RET_OK;
    }
    
    if (waitingTime > 0)
    {
        waitingTime--;
        return  RET_OK;
    }

    switch(status_)
    {
    case        CLIENT_STATUS_UNINIT:
        {        
            
            uint32_t    logCount = 0;
            LOG         log;
            
            memset(&log_, 0, sizeof(log_));
            
            CLIENT_setStatus(CLIENT_STATUS_SYSTEM_INIT_DONE);
            LOG_getCount(&logCount);
            if (logCount > 0)
            {
                if (LOG_getAt(logCount - 1, &log) == RET_OK)
                {
                    if (log.type == LOG_TYPE_RAW)
                    {
                        log_.wakeUpTime = ((CLIENT_LOG*)log.body.raw.data)->nextWakeUpTime;
                        FI_TIME_set(log_.wakeUpTime);
                        TRACE("Set Time to WakeUp Time : %s", FI_TIME_toString(log_.wakeUpTime, "%Y-%m-%d %H:%M:%S"));
                    }
                }
            }

        }
        break;

    case        CLIENT_STATUS_REINIT:
        {        
            CLIENT_setStatus(CLIENT_STATUS_SYSTEM_INIT_DONE);
        }
        break;

    case    CLIENT_STATUS_SYSTEM_INIT_DONE:
        {
            TRACE("External Power ON\n");
            SYS_setExtPower(true);
            CLIENT_setStatus(CLIENT_STATUS_READ_DATA);
            retryCount_ = 0;
        }
        break;

    case    CLIENT_STATUS_READ_DATA:
        {
            TRACE("Data read from Device\n");
            ret = DEVICE_readData(&data_, 1000);
            if (ret != RET_OK)
            {
                data_.valid = false;
                waitingTime = 10000;
                retryCount_++;
                log_.device.readRetryCount = retryCount_;
                TRACE("Data read failed : %d", retryCount_);
                if (config_.maxRetryCount <= retryCount_ )
                {
                    CLIENT_setStatus(CLIENT_STATUS_FINIALIZE);
                }
            }
            else
            {
                uint32_t    length = 0;
                
                TRACE("Data read done!\n");

                length += sprintf(&buffer_[length], "\"ID\":\"%s\"", config_.id);
                length += sprintf(&buffer_[length], ",\"RSV\":\"%s%d.%02d\"", (data_.voltage.RS < 0)?"-":"", abs(data_.voltage.RS / 100), abs(data_.voltage.RS % 100));
                length += sprintf(&buffer_[length], ",\"STV\":\"%s%d.%02d\"", (data_.voltage.ST < 0)?"-":"", abs(data_.voltage.ST / 100), abs(data_.voltage.ST % 100));
                length += sprintf(&buffer_[length], ",\"TRV\":\"%s%d.%02d\"", (data_.voltage.TR < 0)?"-":"", abs(data_.voltage.TR / 100), abs(data_.voltage.TR % 100));
                length += sprintf(&buffer_[length], ",\"RC\":\"%s%d.%02d\"", (data_.current.R < 0)?"-":"", abs(data_.current.R / 100), abs(data_.current.R % 100));
                length += sprintf(&buffer_[length], ",\"SC\":\"%s%d.%02d\"", (data_.current.S < 0)?"-":"", abs(data_.current.S / 100), abs(data_.current.S % 100));
                length += sprintf(&buffer_[length], ",\"TC\":\"%s%d.%02d\"", (data_.current.T < 0)?"-":"", abs(data_.current.T / 100), abs(data_.current.T % 100));
                length += sprintf(&buffer_[length], ",\"TEW\":\"%s%d.%02d\"", (data_.totalPower < 0)?"-":"", abs(data_.totalPower / 100), abs(data_.totalPower % 100));
                length += sprintf(&buffer_[length], ",\"TEWH\":\"%d\"", data_.totalEnergy);
                length += sprintf(&buffer_[length], "}");

                FI_TIME_get(&log_.device.dataTime);
                memcpy(&log_.device.data, &data_, sizeof(data_));
                
                CLIENT_setStatus(CLIENT_STATUS_READ_DATA_FINISHED);
                FI_TIME_get(&log_.modem.startTime);
            }

        }
        break;
        
    case    CLIENT_STATUS_READ_DATA_FINISHED:
        {
            if (config_.delay.enable && config_.delay.mode == CLIENT_DELAY_MODE_TRANSTER_DELAY)
            {
                waitingTime = config_.delay.offset * 1000;
                TRACE("Standby modem wakeup : %d seconds\n", waitingTime / 1000); 
            }
            
            CLIENT_setStatus(CLIENT_STATUS_MODEM_INIT);
        }

    case    CLIENT_STATUS_MODEM_INIT:
        {
            TRACE("Initialized\n");
            log_.modem.initCount++;
            
            ME_I10KL_reset();
            waitingTime = MODEM_INIT_TIME;
            CLIENT_setStatus(CLIENT_STATUS_MODEM_WAKEUP);
        }
        break;
        
        
    case    CLIENT_STATUS_MODEM_WAKEUP:
        {
            ret = ME_I10KL_getWakeUp(&wakeUp_);
            if (ret == RET_OK)
            {
                TRACE("PSM_WAKEUP : %s\n", (wakeUp_?"HI":"LO"));
            }

            ret = ME_I10KL_getState0(&sysState_);
            if (ret == RET_OK)
            {
                TRACE("SYS_STATE : %s\n", (sysState_?"HI":"LO"));
            }

            ret = ME_I10KL_getPSM(&psmMon_);
            if (ret == RET_OK)
            {
                TRACE("PSM_MON : %s\n", (psmMon_?"HI":"LO"));
            }

            ret = ME_I10KL_getAutoSleep(&sysWake);
            if (ret == RET_OK)
            {
                TRACE("SYS_WAKE : %s\n", (sysWake?"LO":"HI"));
            }
            
            if (psmMon_ == false)
            {
                TRACE("Modem wakeup!\n");

                ret = ME_I10KL_setWakeUp(true);
                vTaskDelay(500);
                ret = ME_I10KL_setWakeUp(false);
                
                ret = ME_I10KL_setAutoSleep(false);
                waitingTime = modemInitializationTime;
            }
            else if (sysState_ == false)
            {
                TRACE("Waiting for modem initialzation!\n");
            }
            else 
            {
                CLIENT_setStatus(CLIENT_STATUS_MODEM_INIT_DONE);
                retryCount_ = 0;    
            }

        }
        break;
        
    case    CLIENT_STATUS_MODEM_INIT_DONE:
        {
            uint32_t    receivedLength;
            
            FI_TIME_get(&log_.modem.initFinishedTime);
            
            ret = ME_I10KL_at(NULL, buffer_, sizeof(buffer_), &receivedLength);
            if (ret == RET_OK)
            {
                FI_TIME_get(&log_.modem.initFinishedTime);
                TRACE("Modem initialization complete!\n");
                
                if (config_.delay.enable && (config_.delay.base != 0)&& (config_.delay.offset == 0))
                {
                    CLIENT_setStatus(CLIENT_STATUS_CALCULATE_TRANSFER_OFFSET);
                }
                else
                {
                    CLIENT_setStatus(CLIENT_STATUS_TIME_SYNC_START);
                }
            }
            else 
            {
                TRACE("No response from I10KL.\n");
                retryCount_++;
                
                if (config_.maxRetryCount <= retryCount_)
                {
                    retryCount_ = 0;
                    TRACE("Modem initialization retry failed.");
                    CLIENT_setStatus(CLIENT_STATUS_MODEM_RESET);
                }
                else 
                {
                    TRACE("Modem initialization retry : %d\n", retryCount_);
                }
            }
        }
        break;
        
    case    CLIENT_STATUS_CALCULATE_TRANSFER_OFFSET:
        {
            char    imei[32];
                
            memset(imei, 0, sizeof(imei));
            ret = ME_I10KL_getIMEI(imei, sizeof(imei));
            if (ret == RET_OK)
            {
                uint32_t    value = 0;
                if (strlen(imei) > 9)
                {
                    strToUint32(&imei[strlen(imei) - 9], &value);
                }
                else
                {
                    strToUint32(imei, &value);
                }

                if (value != 0)
                {                 
                    uint32_t count = (config_.delay.period + CONFIG_CLIENT_PERIOD_MIN - 1) / CONFIG_CLIENT_PERIOD_MIN;
                    
                    config_.delay.offset = value % config_.delay.base * config_.delay.period + ((value / config_.delay.base) % count) * CONFIG_CLIENT_PERIOD_MIN;

                    TRACE("Group ID : %d, Index : %d, Offset : %d", value % config_.delay.base, ((value / config_.delay.base) % count),  config_.delay.offset);
                    SYSTEM_globalConfigSave();
                }

                CLIENT_setStatus(CLIENT_STATUS_TIME_SYNC_START);                
            }
            else 
            {
                TRACE("No response from I10KL.\n");
                retryCount_++;
                
                if (config_.maxRetryCount <= retryCount_)
                {
                    retryCount_ = 0;
                    TRACE("Modem initialization retry failed.");
                    CLIENT_setStatus(CLIENT_STATUS_MODEM_RESET);
                }
                else 
                {
                    TRACE("Modem initialization retry : %d\n", retryCount_);
                }
            }

        }
        break;
        
    case    CLIENT_STATUS_TIME_SYNC_START:
        {
            FI_TIME_get(&log_.timeSync.startTime);
            CLIENT_setStatus(CLIENT_STATUS_TIME_SYNC);
        }
        break;
        
    case    CLIENT_STATUS_TIME_SYNC:
        {
            FI_TIME requestTime;
            FI_TIME currentTime;
            FI_TIME time;

            FI_TIME_get(&requestTime);
            
            log_.timeSync.retryCount++;
            
            ret = ME_I10KL_getTime(&time);
            if (ret == RET_OK)
            {
                TRACE("Time Sync : %s", FI_TIME_toString(time, "%Y-%m-%d %H:%M:%S"));
                FI_TIME_get(&currentTime);
            
                if ((currentTime > time) && ((currentTime - time) < 60))
                {
                    time += (currentTime - time) / 2;
                }

                FI_TIME_set(time);

                CLIENT_setStatus(CLIENT_STATUS_TIME_SYNC_FINISHED);
            }
            else 
            {
                waitingTime = MODEM_REQUEST_RETRY_INTERVAL;
                retryCount_++;
                
                if (config_.maxRetryCount <= retryCount_)
                {
                    TRACE("Time synchronization retry times exceeded : %d\n", retryCount_);
                    CLIENT_setStatus(CLIENT_STATUS_MODEM_RESET);
                }
            }
        }
        break;

    case    CLIENT_STATUS_TIME_SYNC_FINISHED:
        {
            FI_TIME_get(&log_.timeSync.finishedTime);
            CLIENT_setStatus(CLIENT_STATUS_SERVER_CONNECT_START);
        }
        break;

    case    CLIENT_STATUS_SERVER_CONNECT_START:
        {
            FI_TIME_get(&log_.server.startTime);
            CLIENT_setStatus(CLIENT_STATUS_SERVER_CONNECT);
        }
        break;
        
    case    CLIENT_STATUS_SERVER_CONNECT:
        {
            log_.server.retryCount ++;
            if (log_.server.retryCount == 1)
            {
                FI_TIME_get(&log_.server.startTime);
            }
            TRACE("Attempt to connect to server : %d\n", log_.server.retryCount);
            
            ret = CLIENT_MQTT_connect();
            if (ret == RET_OK)
            {
                TRACE("MQTT connected\n");
                CLIENT_setStatus(CLIENT_STATUS_SERVER_CONNECTED);
            }
            else 
            {
                waitingTime = MODEM_REQUEST_RETRY_INTERVAL;
                retryCount_++;
                
                if (config_.maxRetryCount <= retryCount_)
                {
                    TRACE("Server connection retry times exceeded : %d\n", retryCount_);
                    CLIENT_setStatus(CLIENT_STATUS_MODEM_RESET);
                }
            }
        }
        break;
        
    case    CLIENT_STATUS_SERVER_CONNECTED:
        {
            TRACE("Server connected\n");
            FI_TIME_get(&log_.server.connectedTime);
            CLIENT_setStatus(CLIENT_STATUS_DATA_TRANSFER_START);    
        }
        break;
        
    case    CLIENT_STATUS_DATA_TRANSFER_START:
        {
            TRACE("Start data transfer!\n");
            FI_TIME_get(&log_.dataTransfer.startTime);
            CLIENT_setStatus(CLIENT_STATUS_DATA_TRANSFER);    
        }
        break;
        
    case    CLIENT_STATUS_DATA_TRANSFER:
        {
            uint32_t    length = 0;
            
            log_.dataTransfer.retryCount++;
            
            TRACE("Attempt to data transfer : %d\n", log_.dataTransfer.retryCount);
            
            length = sprintf(buffer_, "{");
            //length += sprintf(&buffer_[length], "\"ts\":%d,", time_);
            length += sprintf(&buffer_[length], "\"ID\":\"%s\"", config_.id);
            length += sprintf(&buffer_[length], ",\"RSV\":\"%s%d.%02d\"", (data_.voltage.RS < 0)?"-":"", abs(data_.voltage.RS / 100), abs(data_.voltage.RS % 100));
            length += sprintf(&buffer_[length], ",\"STV\":\"%s%d.%02d\"", (data_.voltage.ST < 0)?"-":"", abs(data_.voltage.ST / 100), abs(data_.voltage.ST % 100));
            length += sprintf(&buffer_[length], ",\"TRV\":\"%s%d.%02d\"", (data_.voltage.TR < 0)?"-":"", abs(data_.voltage.TR / 100), abs(data_.voltage.TR % 100));
            length += sprintf(&buffer_[length], ",\"RC\":\"%s%d.%02d\"", (data_.current.R < 0)?"-":"", abs(data_.current.R / 100), abs(data_.current.R % 100));
            length += sprintf(&buffer_[length], ",\"SC\":\"%s%d.%02d\"", (data_.current.S < 0)?"-":"", abs(data_.current.S / 100), abs(data_.current.S % 100));
            length += sprintf(&buffer_[length], ",\"TC\":\"%s%d.%02d\"", (data_.current.T < 0)?"-":"", abs(data_.current.T / 100), abs(data_.current.T % 100));
            length += sprintf(&buffer_[length], ",\"TEW\":\"%s%d.%02d\"", (data_.totalPower < 0)?"-":"", abs(data_.totalPower / 100), abs(data_.totalPower % 100));
            length += sprintf(&buffer_[length], ",\"TEWH\":\"%d\"", data_.totalEnergy);
            length += sprintf(&buffer_[length], "}");
            
            ret = CLIENT_MQTT_pub(config_.server.topic, buffer_);
            switch(ret)
            {
            case    RET_OK:
                {
                    CLIENT_setStatus(CLIENT_STATUS_DATA_TRANSFER_FINISHED);
                }
                break;
                
            case    RET_SOCKET_CLOSED:
                {
                    TRACE("Socket disconnected during data transfer.");
                    CLIENT_setStatus(CLIENT_STATUS_SOCKET_CLOSE);
                }
                break;                    
                
            default:
                {
                    retryCount_++;
                    if (config_.maxRetryCount <= retryCount_)
                    {
                        TRACE("Failed to retransmit.");
                        CLIENT_setStatus(CLIENT_STATUS_MODEM_RESET);
                    }
                }
                break;                    
            }
        }
        break;

    case    CLIENT_STATUS_DATA_TRANSFER_FINISHED:
        {
            TRACE("Data transfer complete finished.\n");
            FI_TIME_get(&log_.dataTransfer.finishedTime);
            CLIENT_setStatus(CLIENT_STATUS_MODEM_FINAL);
        }
        break;
        
    case    CLIENT_STATUS_MODEM_FINAL:
        {
            CLIENT_MQTT_disconnect();
            FI_TIME_get(&log_.server.disconnectedTime);
            CLIENT_setStatus(CLIENT_STATUS_MODEM_DETACH);
        }
        break;
        
    case    CLIENT_STATUS_MODEM_DETACH:
        {
            ME_I10KL_detach();
            
            ME_I10KL_setAutoSleep(true);
            CLIENT_setStatus(CLIENT_STATUS_FINIALIZE);
        }
        break;
        
    case    CLIENT_STATUS_FINIALIZE:
        {
            SYS_setExtPower(false);

            CLIENT_MODE mode;
            mode = CLIENT_getMode();
            if (mode == CLIENT_MODE_SLEEP_WITH_ALARM)
            {
                CLIENT_setStatus(CLIENT_STATUS_SET_NEXT_ALARM);
            }
            else if (mode == CLIENT_MODE_SLEEP_WITH_INTERVAL)
            {
                CLIENT_setStatus(CLIENT_STATUS_SET_NEXT_INTERVAL);
            }
            else if (mode == CLIENT_MODE_IDLE)
            {
                CLIENT_setStatus(CLIENT_STATUS_IDLE);
            }
        }
        break;
        
    case    CLIENT_STATUS_SET_NEXT_ALARM:
        {
            SHELL_printf("Set Alarm\n");
            
            if (CLIENT_getAlarmCount() != 0)
            {
                FI_TIME     currentTime;
                FI_CLOCK    nextAlarm;
                
                FI_TIME_get(&currentTime);
                ret = CLIENT_getNextAlarm(currentTime, &nextAlarm);
                if (ret == RET_OK)
                {
                    if (config_.delay.enable && (config_.delay.mode == CLIENT_DELAY_MODE_WAKEUP_DELAY))
                    {
                        TRACE("WakeUp : %d + %d", nextAlarm, config_.delay.offset);
                        nextAlarm += config_.delay.offset;
                    }
                    else
                    {
                        TRACE("WakeUp : %d", nextAlarm);
                    }
                    
                    log_.nextWakeUpTime = nextAlarm;
                    FI_TIME_setAlarm(nextAlarm);
                }
                
                CLIENT_setStatus(CLIENT_STATUS_READY_TO_SLEEP);
            }
        }
        break;
        
    case    CLIENT_STATUS_SET_NEXT_INTERVAL:
        {
            FI_TIME     currentTime;
            uint32_t    intervalStartTime;
            uint32_t    nextAlarm;
            uint32_t    nextClock;
            RTC_TimeTypeDef     rtcTime;
            RTC_DateTypeDef     rtcDate;
                            
            FI_TIME_get(&currentTime);
            FI_TIME_toRTCDateTime(currentTime, &rtcTime, &rtcDate);
            
            intervalStartTime = CLIENT_getIntervalStartTime();
            if (intervalStartTime != 0)
            {
                if (currentTime < intervalStartTime)
                {
                    nextAlarm = intervalStartTime;
                }
                else 
                {
                    nextAlarm = intervalStartTime + ((currentTime - intervalStartTime) / CLIENT_getIntervalPeriod() + 1) * CLIENT_getIntervalPeriod();
                }

                if (config_.delay.enable && (config_.delay.mode == CLIENT_DELAY_MODE_WAKEUP_DELAY))
                {
                    TRACE("WakeUp : %d + %d", nextAlarm, config_.delay.offset);
                    nextAlarm += config_.delay.offset;
                }
                FI_TIME_toRTCDateTime(nextAlarm, &rtcTime, &rtcDate);
                nextClock = rtcTime.Hours * SECONDS_OF_HOUR + rtcTime.Minutes * SECONDS_OF_MINUTE + rtcTime.Seconds;
                
                if (config_.delay.enable && (config_.delay.mode == CLIENT_DELAY_MODE_WAKEUP_DELAY))
                {
                    TRACE("WakeUp : %d + %d", nextAlarm, config_.delay.offset);
                    nextAlarm += config_.delay.offset;
                }
                else
                {
                    TRACE("WakeUp : %d", nextAlarm);
                }

                log_.nextWakeUpTime = nextAlarm;
                FI_TIME_setAlarm(nextClock);
                
                CLIENT_setStatus(CLIENT_STATUS_READY_TO_SLEEP);
            }
            else
            {
                CLIENT_setStatus(CLIENT_STATUS_IDLE);
            }
        }
        break;

    case    CLIENT_STATUS_READY_TO_SLEEP:
        {
            LOG_raw(&log_, sizeof(log_));
            CLIENT_setStatus(CLIENT_STATUS_GO_TO_SLEEP);
        }
        break;

        
    case    CLIENT_STATUS_GO_TO_SLEEP:
        {
            CLIENT_goToSleep();
        }
        break;

    case    CLIENT_STATUS_SOCKET_CLOSE:
        {
            CLIENT_setStatus(CLIENT_STATUS_MODEM_RESET);
        }
        break;

    case    CLIENT_STATUS_MODEM_RESET:
        {
            SHELL_printf("Restart\n");
            CLIENT_MQTT_disconnect();
            modemResetCount_++;

            CLIENT_setStatus(CLIENT_STATUS_MODEM_DETACH);
            
            if (config_.maxRetryCount < modemResetCount_)
            {
                CLIENT_setStatus(CLIENT_STATUS_MODEM_INIT);
            }
        }
        break;
        
    case    CLIENT_STATUS_IDLE:
        {
        }
        break;
    }
    
    if (waitingTime == 0)
    {
        waitingTime = LOOP_INTERVAL;
    }
    
    return  RET_OK;
}
 

RET_VALUE   CLIENT_MQTT_connect(void)
{
    RET_VALUE   ret;
    
    if (!socket_)
    {
        ret = ME_I10KL_createSocket(6, 1024, &socket_);
        if (ret != RET_OK)
        {
            TRACE("Socket creation failed.\n");
            return  ret;
        }
        
        TRACE("Create socket : %d\n", socket_);
    }
    
    ret = ME_I10KL_MQTT_connect(socket_, config_.server.ip, config_.server.port, config_.id, config_.server.userId, NULL);
    if (ret == RET_OK)
    {
        TRACE("Connected\n");
    }
    
    return  ret;

}


RET_VALUE   CLIENT_MQTT_pub(char *topic, char *message)
{
    RET_VALUE   ret;

    ret = ME_I10KL_MQTT_publish(socket_, config_.server.ip, config_.server.port, config_.server.topic, message);
    if (ret == RET_OK)
    {
         ret = ME_I10KL_closeSocket(socket_);
         socket_ = 0;
    }

    return  ret;
}


RET_VALUE   CLIENT_MQTT_disconnect(void)
{
    RET_VALUE   ret;

    if (socket_)
    {
        ret = ME_I10KL_closeSocket(socket_);
        if (ret != RET_OK)
        {
            TRACE("Socket close failed.\n");
        }
        else
        {
            TRACE("Close socket : %d\n", socket_);
        }
        socket_ = 0;
    }
    
    
    return  ret;

}

RET_VALUE   CLIENT_setStatus(CLIENT_STATUS status)
{
    retryCount_ = 0;
    status_  = status;
    
    TRACE("Status : %s\n", statusNames_[status_]);
    return  RET_OK;
}

uint32_t    CLIENT_getAlarmCount(void)
{
    return  config_.opMode.alarm.count;
}

RET_VALUE   CLIENT_deleteAllAlarms(void)
{
    config_.opMode.alarm.count = 0;
    
    return  RET_OK;
}

RET_VALUE   CLIENT_addAlarm(uint32_t alarm)
{
    uint32_t    i;
    uint32_t    index = 0;
    
    for(i = 0; i < config_.opMode.alarm.count ; i++)
    {
        if (config_.opMode.alarm.times[i] == alarm)
        {
            return  RET_ALREADY_EXIST;
        }
    
        if (config_.opMode.alarm.times[i] < alarm)
        {
            index = i+1;
        }
    }
    
    
    for(i = config_.opMode.alarm.count ; i > index ; i--)
    {
        config_.opMode.alarm.times[i] = config_.opMode.alarm.times[i-1];
    }
    
    config_.opMode.alarm.times[index] = alarm;
    config_.opMode.alarm.count++;
    
    return  RET_OK;
}

RET_VALUE   CLIENT_delAlarm(uint32_t index)
{
    if (index < config_.opMode.alarm.count)
    {
        uint32_t    i;
        
        for(i = index ; i < config_.opMode.alarm.count - 1; i++)
        {
            config_.opMode.alarm.times[i] = config_.opMode.alarm.times[i+1];
        }
        
        config_.opMode.alarm.count --;
        
        return  RET_OK;
    }

    return  RET_OUT_OF_RANGE;
}

RET_VALUE   CLIENT_getAlarm(uint32_t index, uint32_t *alarm)
{
    ASSERT(alarm != NULL);
    
    if (index < config_.opMode.alarm.count)
    {
        *alarm = config_.opMode.alarm.times[index];
        
        return  RET_OK;
    }
    
    return  RET_OUT_OF_RANGE;
}

RET_VALUE   CLIENT_getAlarms(uint32_t* alarms, uint32_t maxAlarms, uint32_t *count)
{
    ASSERT((alarms != NULL) && (count != NULL));
    
    uint32_t    i = 0;
     
    for(i = 0 ; (i < config_.opMode.alarm.count) && (i < maxAlarms) ; i++)
    {
        alarms[i] = config_.opMode.alarm.times[i];
    }
    
    *count = i;
    
    return  RET_OK;
}


RET_VALUE   CLIENT_getNextAlarm(FI_TIME time, FI_CLOCK *alarm)
{
    ASSERT(alarm != NULL);
    
    uint32_t    i;
    RTC_TimeTypeDef     rtcTime;
    RTC_DateTypeDef     rtcDate;
    
    FI_TIME_toRTCDateTime(time, &rtcTime, &rtcDate);

    
    uint32_t    currentClock = (rtcTime.Hours * 60 + rtcTime.Minutes) * 60 + rtcTime.Seconds;
    
    for(i = 0 ; i < config_.opMode.alarm.count ; i++)
    {
        if (currentClock < config_.opMode.alarm.times[i])
        {
            break;
        }
    }
    
    if (i == config_.opMode.alarm.count)
    {
        *alarm = config_.opMode.alarm.times[0];
    }
    else 
    {
        *alarm = config_.opMode.alarm.times[i];
    }
    
    return  RET_OK;
}

RET_VALUE   CLIENT_goToSleep(void)
{
    HAL_PWR_EnterSTANDBYMode();
    
    return  RET_OK;
}

RET_VALUE   CLIENT_setId(char* Id)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    ASSERT(Id != NULL);
    
    if (strlen(Id) < sizeof(config_.id))
    {
        strcpy(config_.id, Id);
        
        ret = RET_OK;
    }
        
    return  ret;
}

char*   CLIENT_getId(void)
{   
    return  config_.id;
}
    
RET_VALUE   CLIENT_setDelayBase(uint32_t base)
{
    config_.delay.base = base;

    return  RET_OK;
}
    
uint32_t    CLIENT_getDelayBase(void)
{
    return  config_.delay.base;
}
        
RET_VALUE   CLIENT_setDelayMode(CLIENT_DELAY_MODE mode)
{
    config_.delay.mode = mode;

    return  RET_OK;
}
    
CLIENT_DELAY_MODE   CLIENT_getDelayMode(void)
{
    return  config_.delay.mode;
}
        
RET_VALUE   CLIENT_setDelayPeriod(uint32_t period)
{
    config_.delay.period= period;

    return  RET_OK;
}
    
uint32_t    CLIENT_getDelayPeriod(void)
{
    return  config_.delay.period;
}
        
RET_VALUE   CLIENT_setDelayOffset(uint32_t offset)
{
    config_.delay.offset= offset;

    return  RET_OK;
}
    
uint32_t    CLIENT_getDelayOffset(void)
{
    return  config_.delay.offset;
}