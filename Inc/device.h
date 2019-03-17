#ifndef DEVICE_H_
#define DEVICE_H_

#include "target.h"
#include "serial.h"
#include "fi_time.h"

typedef enum    
{
    DEVICE_TYPE_SIM,
    DEVICE_TYPE_GIPAM2000,
    DEVICE_TYPE_ACURRA_A3300,
    DEVICE_TYPE_ACURRA_A3300_2,
    DEVICE_TYPE_LS_HCUM,
    DEVICE_TYPE_MAX
}   DEVICE_TYPE;

typedef struct 
{
    uint32_t         id;
    DEVICE_TYPE     type;
    SERIAL_CONFIG   serial;
    uint32_t        timeout;
}   DEVICE_CONFIG;

typedef struct 
{
    DEVICE_TYPE     type;
    SERIAL_CONFIG   serial;
    uint32_t        timeout;
}   DEVICE_DESCRIPTION;


typedef struct
{
    bool            valid;
    struct
    {
        int32_t     RS;
        int32_t     ST;
        int32_t     TR;
    }   voltage;
    struct 
    {
        int32_t     R;
        int32_t     S;
        int32_t     T;
    }   current;
    
    int32_t     totalPower;
    int32_t     totalEnergy;
}   DEVICE_DATA;


RET_VALUE   DEVICE_init(DEVICE_CONFIG* config);
RET_VALUE   DEVICE_final(void);

RET_VALUE   DEVICE_getConfig(DEVICE_CONFIG* config);

RET_VALUE   DEVICE_getID(uint32_t *id);
RET_VALUE   DEVICE_setID(uint32_t id);

uint32_t    DEVICE_getTimeout(void);
RET_VALUE   DEVICE_setTimeout(uint32_t timeout);

RET_VALUE   DEVICE_getBaudrate(SERIAL_BAUDRATE* baudrate);
RET_VALUE   DEVICE_setBaudrate(SERIAL_BAUDRATE baudrate);
RET_VALUE   DEVICE_getParity(SERIAL_PARITY* parity);
RET_VALUE   DEVICE_setParity(SERIAL_PARITY parity);
RET_VALUE   DEVICE_getDataBits(SERIAL_DATA_BITS* databits);
RET_VALUE   DEVICE_setDataBits(SERIAL_DATA_BITS databits);
RET_VALUE   DEVICE_getStopBits(SERIAL_STOP_BITS* stopbit);
RET_VALUE   DEVICE_setStopBits(SERIAL_STOP_BITS stopbit);

RET_VALUE   DEVICE_isSupportType(DEVICE_TYPE type);
RET_VALUE   DEVICE_getType(DEVICE_TYPE *type);
RET_VALUE   DEVICE_setType(DEVICE_TYPE type);
char const*   DEVICE_getTypeName(DEVICE_TYPE type);

//RET_VALUE   DEVICE_readHoldingRegisters(uint8_t id, uint16_t address, uint16_t count, uint16_t *registers, uint32_t timeout);
RET_VALUE   DEVICE_readHoldingRegisters(uint8_t id, uint8_t function, uint16_t address, uint16_t count, uint16_t *registers, uint32_t timeout);


RET_VALUE   DEVICE_readData(DEVICE_DATA* data, uint32_t timeout);

#endif
