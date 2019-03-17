#ifndef LOG_H__
#define LOG_H__

#include "time.h"

#define LOG_SLOT_SIZE   256

typedef uint16_t    LOG_TYPE;

#define LOG_TYPE_MESSAGE    1
#define LOG_TYPE_RAW        2

typedef struct
{
    uint32_t    startAddress;
    uint32_t    size;
    uint32_t    maxCount;
    bool        enable;
}   LOG_CONFIG;

typedef struct
{
    uint32_t    type;
    FI_TIME     time;
    union
    {
        char        message[LOG_SLOT_SIZE - sizeof(uint32_t) * 4];
        struct
        {
            uint32_t    length;
            uint8_t     data[LOG_SLOT_SIZE - sizeof(uint32_t) * 5];
        }   raw;
    }   body;
}   LOG;
    
typedef struct
{
    uint32_t    index;
    LOG         log;
    uint32_t    crc;
}   LOG_SLOT;

RET_VALUE   LOG_init(LOG_CONFIG* config);
RET_VALUE   LOG_getConfig(LOG_CONFIG* config);
RET_VALUE   LOG_isValidConfig(LOG_CONFIG* config);
RET_VALUE   LOG_setEnable(bool enable);
RET_VALUE   LOG_getCount(uint32_t* count);
RET_VALUE   LOG_getAt(uint32_t index, LOG* log);
RET_VALUE   LOG_message(char* message);
RET_VALUE   LOG_raw(void* data, uint32_t length);
RET_VALUE   LOG_save(LOG* log);
RET_VALUE   LOG_clear(void);
RET_VALUE   LOG_printf(const char *format, ... );
RET_VALUE   LOG_showDebug();

#endif
