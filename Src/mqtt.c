#include <string.h>
#include "assert.h"
#include "config.h"
#include "me_i10kl.h"
#include "mqtt.h"

RET_VALUE   MQTT_MSG_createConnect(char* buffer, uint32_t bufferLength, uint32_t* frameLength, char* clientId, char* userId, char *passwd)
{
    ASSERT(clientId != NULL);

    uint32_t    length, payloadLength;
    uint8_t     flags = MQTT_CLEAN_SESSION;
    
    length = 12 + strlen(clientId) + 2;
    if ((userId != NULL) && (strlen(userId) != 0))
    {
        flags |= MQTT_USERNAME_FLAG;
        length += strlen(userId) + 2;
    }
    
    if ((passwd != NULL)  && (strlen(passwd) != 0))
    {
        flags |= MQTT_PASSWORD_FLAG;
        length += strlen(passwd) + 2;
    }
    
    if (buffer != NULL)
    {
        if (length <= 127)
        {
            if (bufferLength < 2 + length) 
            {
                return  RET_BUFFER_TOO_SMALL;
            }
            
            buffer[0] = MQTT_MSG_CONNECT; 
            buffer[1] = length; 
            payloadLength = 2;
        }
        else 
        {
            if (bufferLength < 3 + length) 
            {
                return  RET_BUFFER_TOO_SMALL;
            }

            buffer[0] = MQTT_MSG_CONNECT; 
            buffer[1] = (length & 0x7F) | 0x80; 
            buffer[2] = (length >> 7) & 0x7F;
            payloadLength = 3;
        }
    }
    else 
    {
        if (length <= 127)
        {
            *frameLength = 2 + length;
            return  RET_OK;
        }
        else 
        {
            *frameLength = 3 + length;
            return  RET_OK;
        }
    }

    buffer[payloadLength++] = 0x00;
    buffer[payloadLength++] = 0x06;
    buffer[payloadLength++] = 'M';
    buffer[payloadLength++] = 'Q';
    buffer[payloadLength++] = 'I';
    buffer[payloadLength++] = 's';
    buffer[payloadLength++] = 'd';
    buffer[payloadLength++] = 'p';
    buffer[payloadLength++] = 0x03;
    buffer[payloadLength++] = flags;
    buffer[payloadLength++] = 0;
    buffer[payloadLength++] = 60;
    length = strlen(clientId);
    buffer[payloadLength++] = (length >> 8) & 0xFF;
    buffer[payloadLength++] = (length     ) & 0xFF;
    memcpy(&buffer[payloadLength], clientId, length);
    payloadLength += length;

    if ((userId != NULL) && (strlen(userId) != 0))
    {
        length = strlen(userId);
        buffer[payloadLength++] = (length >> 8) & 0xFF;
        buffer[payloadLength++] = (length     ) & 0xFF;
        memcpy(&buffer[payloadLength], userId, length);
        payloadLength += length;
    }
    
    if ((passwd != NULL) && (strlen(passwd) != 0))
    {
        length = strlen(passwd);
        buffer[payloadLength++] = (length >> 8) & 0xFF;
        buffer[payloadLength++] = (length     ) & 0xFF;
        memcpy(&buffer[payloadLength], passwd, length);
        payloadLength += length;
    }

    *frameLength = payloadLength;
    
    return  RET_OK;
}


RET_VALUE   MQTT_MSG_createPublish(char* buffer, uint32_t bufferLength, uint32_t* frameLength, char* topic, char *message)
{
    ASSERT((topic != NULL) && (message != NULL));

    uint32_t    length, payloadLength;
    
    length = strlen(topic) + 2 + strlen(message);
    
    if (buffer != NULL)
    {
        if (length <= 127)
        {
            if (bufferLength < 2 + length) 
            {
                return  RET_BUFFER_TOO_SMALL;
            }
            
            buffer[0] = MQTT_MSG_PUBLISH; 
            buffer[1] = length; 
            payloadLength = 2;
        }
        else 
        {
            if (bufferLength < 3 + length) 
            {
                return  RET_BUFFER_TOO_SMALL;
            }

            buffer[0] = MQTT_MSG_PUBLISH; 
            buffer[1] = (length & 0x7F) | 0x80; 
            buffer[2] = (length >> 7) & 0x7F;
            payloadLength = 3;
        }
    }
    else 
    {
        if (length <= 127)
        {
            *frameLength = 2 + length;
            return  RET_OK;
        }
        else 
        {
            *frameLength = 3 + length;
            return  RET_OK;
        }
    }

    length = strlen(topic);
    buffer[payloadLength++] = (length >> 8) & 0xFF;
    buffer[payloadLength++] = (length     ) & 0xFF;
    memcpy(&buffer[payloadLength], topic, length);
    payloadLength += length;

    length = strlen(message);
    memcpy(&buffer[payloadLength], message, length);
    payloadLength += length;

    *frameLength = payloadLength;
    
    return  RET_OK;
}
