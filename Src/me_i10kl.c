#include <ctype.h>
#include <time.h>
#include "target.h"
#include "main.h"
#include "config.h"
#include "serial.h"
#include "me_i10kl.h"
#include "mqtt.h"

#define TRACE(...)  TRACE_printf("ME_I10KL", __VA_ARGS__)


RET_VALUE   ME_I10KL_command(char* cmd, char* params, char* response, uint32_t length);
uint32_t    ME_I10KL_addParams(char* buffer, uint32_t bufferSize, char* param);
RET_VALUE   ME_I10KL_request(char* cmd, char* requestParam, char* responseParam, uint32_t maxResponseParamLength, uint32_t* responseParamLength);
RET_VALUE   ME_I10KL_clearBuffer(void);
RET_VALUE   ME_I10KL_sendAndReceive(uint8_t* sendBuffer, uint32_t sendBufferLength, uint8_t* receiveBuffer, uint32_t receiveBufferLength, uint32_t *receivedLength, uint32_t timeout);
RET_VALUE   ME_I10KL_parse(char*   buffer, char** list, uint32_t maxCount, uint32_t* count);
char*       ME_I10KL_RESULT_getParams(char** results, uint32_t count, char* header);
bool        ME_I10KL_RESULT_isOK(char** results, uint32_t count);

static  ME_I10KL_CONFIG config_;
static  bool            powerStatus_ = false;

#define ME_I10KL_CONFIG_STREAM_BUFFER_SIZE  512

static  SERIAL_HANDLE   serial_;
static  uint8_t         sendBuffer[ME_I10KL_CONFIG_STREAM_BUFFER_SIZE];
static  uint8_t         tempBuffer[ME_I10KL_CONFIG_STREAM_BUFFER_SIZE];
static  uint8_t         receiveBuffer[ME_I10KL_CONFIG_STREAM_BUFFER_SIZE];

RET_VALUE   ME_I10KL_init(ME_I10KL_CONFIG* config)
{
    RET_VALUE   ret;
    
    ASSERT(config != NULL);

    memcpy(&config_, config, sizeof(config_));
    
    ret = SERIAL_open(&config_.serial, ME_I10KL_CONFIG_STREAM_BUFFER_SIZE, &serial_);
    
    return  ret;
}

RET_VALUE   ME_I10KL_final(void)
{
    return  RET_NOT_SUPPORTED_FUNCTION;
}


RET_VALUE   ME_I10KL_getConfig(ME_I10KL_CONFIG* config)
{
    ASSERT(config != NULL);

    memcpy(config, &config_, sizeof(config_));
    
    return  RET_OK;
}

RET_VALUE   ME_I10KL_start(void)
{
    ME_I10KL_reset();
    return  RET_OK;
}

RET_VALUE   ME_I10KL_stop(void)
{
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   ME_I10KL_getVersion(char* version, uint32_t maxVersionLength)
{
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   ME_I10KL_getTime(FI_TIME* time)
{
    RET_VALUE   ret;
    uint32_t receivedLength = 0;
    
    ME_I10KL_clearBuffer();
    
    uint32_t messageLength = sprintf((char *)sendBuffer, "AT+CCLK?\r\n");
   
    ret = ME_I10KL_sendAndReceive(sendBuffer, messageLength, receiveBuffer, sizeof(receiveBuffer), &receivedLength, config_.timeout);
    if (ret == RET_OK)
    {
        char*   results[4];
        uint32_t    result_count = 0;
        ret = ME_I10KL_parse((char *)receiveBuffer, results, 4, &result_count);
        if (ret == RET_OK)
        {
            int i;
            if (strcasecmp(results[result_count-1], "OK") != 0)
            {
                return  RET_ERROR;
            }
            
            char*   timeString = ME_I10KL_RESULT_getParams(results, result_count, "+CCLK: \"");
            if (timeString == NULL)
            {
                TRACE("Invalid response!\n");
                return  RET_ERROR;
            }            
            timeString += 8;
            if (strlen(timeString) < 18)
            {
                TRACE("Invalid frame length : %d\n", strlen(timeString));
                return  RET_ERROR;
            }

            for(i = 0 ; i < 18 ; i++)
            {
                if (((i+1) % 3) != 0)
                {
                    if (!isdigit(timeString[i]))
                    {
                        TRACE("Not digit : %d, %c\n", i, timeString[i]);
                        return  RET_ERROR;
                    }
                }
                else 
                {
                    timeString[i] = 0;
                }
            }
            
            struct tm    tm;
            tm.tm_year = 100+ atoi(&timeString[0]);
            tm.tm_mon  = atoi(&timeString[3]) - 1;
            tm.tm_mday = atoi(&timeString[6]);
            tm.tm_hour = atoi(&timeString[9]);
            tm.tm_min  = atoi(&timeString[12]);
            tm.tm_sec  = atoi(&timeString[15]);
            tm.tm_isdst = 0;
            
            TRACE("%04d-%02d-%02d %02d:%02d:%02d\n", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            *time = mktime(&tm) - FI_TIME_getTimeZone();
        }
    }
    
    return  ret;
}

RET_VALUE   ME_I10KL_createSocket(uint32_t protocol, uint16_t port, uint32_t* socket)
{
    RET_VALUE   ret;
    uint32_t receivedLength = 0;
    
    ME_I10KL_clearBuffer();
    
    uint32_t messageLength;
    if (protocol == 6)
    {
        messageLength = sprintf((char *)sendBuffer, "AT+NSOCR=STREAM,6,%d,0\r\n", port);
    }
    else if (protocol == 17)

    {
        messageLength = sprintf((char *)sendBuffer, "AT+NSOCR=DGRAM,17,%d,1\r\n", port);
    }
    else 
    {
        return  RET_INVALID_ARGUMENT;
    }
    
    ret = ME_I10KL_sendAndReceive(sendBuffer, messageLength, receiveBuffer, sizeof(receiveBuffer), &receivedLength, config_.timeout);
    if (ret == RET_OK)
    {
        char*   results[4];
        uint32_t    result_count = 0;
        ret = ME_I10KL_parse((char *)receiveBuffer, results, 4, &result_count);
        if (ret == RET_OK)
        {
            int i;
            for(i = 0 ; i < result_count ; i++)
            {
                TRACE("%d : %s\n", i, results[i]);
            }
            
            if (strcasecmp(results[result_count-1], "OK") != 0)
            {
                return  RET_ERROR;
            }
            
            *socket = strtoul(results[0], 0, 10);
        }
    }
    
    return  ret;

}


RET_VALUE   ME_I10KL_closeSocket(uint32_t socket)
{
    RET_VALUE   ret;
    uint32_t receivedLength = 0;
    
    ME_I10KL_clearBuffer();
    
    uint32_t messageLength = sprintf((char *)sendBuffer, "AT+NSOCL=%d\r\n", socket);
   
    ret = ME_I10KL_sendAndReceive(sendBuffer, messageLength, receiveBuffer, sizeof(receiveBuffer), &receivedLength, config_.timeout);
    if (ret == RET_OK)
    {
        char*   results[4];
        uint32_t    result_count = 0;
        ret = ME_I10KL_parse((char *)receiveBuffer, results, 4, &result_count);
        if (ret == RET_OK)
        {
            int i;
            for(i = 0 ; i < result_count ; i++)
            {
                TRACE("%d : %s\n", i, results[i]);
            }
            
            if (strcasecmp(results[result_count-1], "OK") != 0)
            {
                return  RET_ERROR;
            }
        }
    }
    
    return  ret;

}

RET_VALUE   ME_I10KL_MQTT_connect(uint16_t socket, uint8_t* ip, uint16_t port, char* id, char* userId, char* passwd)
{
    RET_VALUE   ret;
    uint32_t    receivedLength = 0;
    uint32_t    payloadLength = 0;
    uint32_t    messageLength = 0;
    uint32_t    i;
    
    ret = MQTT_MSG_createConnect(NULL, 0, &payloadLength, id, userId, passwd);
    if (ret != RET_OK)
    {
        TRACE("Error : %d\n", ret);
        return  ret;
    }
    
    ME_I10KL_clearBuffer();
    
    messageLength = sprintf((char *)sendBuffer, "AT+NSOST=%d,%d.%d.%d.%d,%d,%d,", socket, ip[0], ip[1], ip[2], ip[3], port, payloadLength);
    if (sizeof(sendBuffer) < messageLength + payloadLength * 2 + 2 + 1)
    {
        return  RET_DATA_TOO_LONG;
    }
    
    MQTT_MSG_createConnect((char *)tempBuffer, sizeof(tempBuffer), &payloadLength, id, userId, passwd);

    for(i = 0 ; i < payloadLength ; i++)
    {
        messageLength += sprintf((char*)&sendBuffer[messageLength], "%02X", tempBuffer[i]);
    }
    messageLength += sprintf((char*)&sendBuffer[messageLength], "\r\n");
    
    ret = ME_I10KL_sendAndReceive(sendBuffer, messageLength, receiveBuffer, sizeof(receiveBuffer), &receivedLength, config_.timeout);
    if (ret == RET_OK)
    {
        char*       results[16];
        uint32_t    count;
        ret = ME_I10KL_parse((char *)receiveBuffer, results, 16, &count);
        if (ret == RET_OK)
        {
            if (2 <= count)
            {
                if (strcasecmp(results[1], "OK") != 0)
                {
                    ret = RET_ERROR;
                }
            }
            else 
            {
                ret = RET_ERROR;
            }
        }
    }
    
    return  ret;

}


RET_VALUE   ME_I10KL_MQTT_publish(uint16_t socket, uint8_t* ip, uint16_t port, char* topic, char* message)
{
    RET_VALUE   ret;
    uint32_t    receivedLength = 0;
    uint32_t    payloadLength = 0;
    uint32_t    messageLength = 0;
    uint32_t    i;
    
    ret = MQTT_MSG_createPublish(NULL, 0, &payloadLength, topic, message);
    if (ret != RET_OK)
    {
        TRACE("Error : %d\n", ret);
        return  ret;
    }
    
    ME_I10KL_clearBuffer();
    
    messageLength = sprintf((char *)sendBuffer, "AT+NSOST=%d,%d.%d.%d.%d,%d,%d,", socket, ip[0], ip[1], ip[2], ip[3], port, payloadLength);
    if (sizeof(sendBuffer) < messageLength + payloadLength * 2 + 2 + 1)
    {
        return  RET_DATA_TOO_LONG;
    }
    
    ret = MQTT_MSG_createPublish((char *)tempBuffer, sizeof(tempBuffer), &payloadLength, topic, message);
    if (ret != RET_OK)
    {
        TRACE("Error : %d\n", ret);
        return  ret;
    }
    
    for(i = 0 ; i < payloadLength ; i++)
    {
        messageLength += sprintf((char*)&sendBuffer[messageLength], "%02X", tempBuffer[i]);
    }
    messageLength += sprintf((char*)&sendBuffer[messageLength], "\r\n");
    
    ret = ME_I10KL_sendAndReceive(sendBuffer, messageLength, receiveBuffer, sizeof(receiveBuffer), &receivedLength, config_.timeout);
    if (ret == RET_OK)
    {
        char*       results[16];
        uint32_t    count;
        ret = ME_I10KL_parse((char *)receiveBuffer, results, 16, &count);
        if (ret == RET_OK)
        {
            if ((count < 2) || (strcasecmp(results[1], "OK") != 0))
            {
                return  RET_ERROR;
            }
            
            if ((count > 2) && (strncasecmp(results[2], "AT+NSOCLIND", 10) != 0))
            {
                return  RET_SOCKET_CLOSED;
            }            
        }
    }
    
    return  ret;

}

RET_VALUE   ME_I10KL_send(char* data, bool confirmed)
{

    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   ME_I10KL_receive(char* response, uint32_t length)
{
    uint32_t receivedLength = SERIAL_gets(serial_, (uint8_t*)response, length,  config_.timeout);
    if (receivedLength == 0)
    {
        return  RET_TIMEOUT;
    }

    return  RET_OK;
}

RET_VALUE   ME_I10KL_at(char* cmd, char* buffer, uint32_t bufferLength, uint32_t* receivedLength)
{
    RET_VALUE   ret;

    ME_I10KL_clearBuffer();
    
    uint32_t messageLength = sprintf((char *)sendBuffer, "AT");
    if (cmd != NULL)
    {
        messageLength += sprintf((char *)&sendBuffer[messageLength], "+%s", cmd);
    }
    messageLength += sprintf((char *)&sendBuffer[messageLength], "\r\n");
    
    ret = ME_I10KL_sendAndReceive(sendBuffer, messageLength, (uint8_t *)buffer, bufferLength, receivedLength, config_.timeout);
    
    return  ret;
}


RET_VALUE   ME_I10KL_reset(void)
{
    TRACE("Reset\n"); 
    HAL_GPIO_WritePin(LTE_RESET_N_GPIO_Port, LTE_RESET_N_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(LTE_RESET_N_GPIO_Port, LTE_RESET_N_Pin, GPIO_PIN_SET);
    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_setWakeUp(bool status)
{
    if (status)
    {
        HAL_GPIO_WritePin(LTE_WAKEUP_N_GPIO_Port, LTE_WAKEUP_N_Pin, GPIO_PIN_SET);
    }
    else 
    {
        HAL_GPIO_WritePin(LTE_WAKEUP_N_GPIO_Port, LTE_WAKEUP_N_Pin, GPIO_PIN_RESET);
    }
    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_getWakeUp(bool *status)
{
    *status = (HAL_GPIO_ReadPin(LTE_WAKEUP_N_GPIO_Port, LTE_WAKEUP_N_Pin) == GPIO_PIN_SET);
    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_getState0(bool* status)
{
    *status = (HAL_GPIO_ReadPin(LTE_STATE0_GPIO_Port, LTE_STATE0_Pin) == GPIO_PIN_SET);
    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_clearBuffer(void)
{
    while(1)
    {
        uint32_t receivedLength = SERIAL_gets(serial_, (uint8_t*)receiveBuffer, sizeof(receiveBuffer), 100);
        if (receivedLength == 0)
        {
            break;
        }
    }

    return  RET_OK;
}   

RET_VALUE   ME_I10KL_setAutoSleep(bool autoSleep)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (autoSleep)
    {
      GPIO_InitStruct.Pin = AP_WAKE_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(AP_WAKE_GPIO_Port, &GPIO_InitStruct);
    }
    else 
    {
      GPIO_InitStruct.Pin = AP_WAKE_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(AP_WAKE_GPIO_Port, &GPIO_InitStruct);
      
      HAL_GPIO_WritePin(AP_WAKE_GPIO_Port, AP_WAKE_Pin, GPIO_PIN_SET);

    }    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_getAutoSleep(bool* autoSleep)
{
    ASSERT(autoSleep != NULL);

    *autoSleep = (HAL_GPIO_ReadPin(AP_WAKE_GPIO_Port, AP_WAKE_Pin) == GPIO_PIN_RESET);
    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_getPSM(bool* status)
{
    ASSERT(status != NULL);

    *status = (HAL_GPIO_ReadPin(PSM_MON_GPIO_Port, PSM_MON_Pin) == GPIO_PIN_SET);
    
    return  RET_OK;
}   

RET_VALUE   ME_I10KL_sendAndReceive(uint8_t* sendBuffer, uint32_t sendBufferLength, uint8_t* receiveBuffer, uint32_t receiveBufferLength, uint32_t *receivedLength, uint32_t timeout)
{
    TRACE("SEND : %s\n", (char *)sendBuffer);

    memset(receiveBuffer, 0, receiveBufferLength);
    RET_VALUE   ret = SERIAL_puts(serial_, sendBuffer, sendBufferLength, timeout);
    if (ret == RET_OK)
    {
        *receivedLength = 0;
        while(1)
        {
            uint32_t length = SERIAL_gets(serial_, (uint8_t*)&receiveBuffer[*receivedLength], receiveBufferLength, timeout);
            if (length == 0)
            {
                break;
            }
            
            *receivedLength      += length;
            receiveBufferLength -= length;
        }
    
        if (!*receivedLength) 
        {
            ret = RET_TIMEOUT;
        }
    }

    if (ret == RET_OK)
    {
        receiveBuffer[*receivedLength] = NULL;
        TRACE("RCVD : %s\n", receiveBuffer);
    }
    
    return  ret;
}    

RET_VALUE   ME_I10KL_parse(char*   buffer, char** list, uint32_t maxCount, uint32_t* count)
{
    *count = 0;

    while(*buffer != NULL)
    {
        if ((*buffer != '\n') && (*buffer != '\r'))
        {
            if (*count >= maxCount)
            {
                break;
            }
            
            list[(*count)++] = buffer++;
            while((*buffer != '\n') && (*buffer != '\r') && (*buffer != NULL))
            {
                buffer++;
            }
            
            if (*buffer != NULL)
            {
                *buffer = NULL;
                buffer++;
            }
        }
        else 
        {
            buffer++;
        }
    }
 
    if (*count != 0)
    {
        TRACE("Response Parser\n");
        for(uint32_t i = 0 ; i < *count ; i++)
        {
            TRACE("%d : %s\n", i, list[i]);
        }

        return  RET_OK;
    }
    
    return  RET_INVALID_ARGUMENT;
}

char*   ME_I10KL_RESULT_getParams(char** results, uint32_t count, char* header)
{
    uint32_t    i;
    
    for(i = 0 ; i < count ; i++)
    {
        if (strncasecmp(results[i], header, strlen(header)) == 0)
        {
            return  results[i];
        }
    }
    
    return  NULL;
}


bool    ME_I10KL_RESULT_isOK(char** results, uint32_t count)
{
    uint32_t    i;
    
    for(i = 0 ; i < count ; i++)
    {
        if (strcasecmp(results[i], "OK") == 0)
        {
            return  true;
        }
    }
    
    return  false;
}


bool   ME_I10KL_getPower(void)
{
    return  powerStatus_;
}   

RET_VALUE   ME_I10KL_setPower(bool on)
{
    if (on)
    {
        TRACE("Power ON\n"); 
        HAL_GPIO_WritePin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin, GPIO_PIN_SET);
    }
    else 
    {
        HAL_GPIO_WritePin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin, GPIO_PIN_RESET);
    }

    powerStatus_  = on;

    return  RET_OK;
}   

RET_VALUE   ME_I10KL_setTimeout(uint32_t timeout)
{
    config_.timeout = timeout;
    
    return  RET_OK;
}

RET_VALUE   ME_I10KL_getTimeout(uint32_t* timeout)
{
    *timeout = config_.timeout;
    
    return  RET_OK;
}