#include <ctype.h>
#include <time.h>
#include "target.h"
#include "main.h"
#include "config.h"
#include "serial.h"
#include "device.h"
#include "crc16.h"
#include "utils.h"

#define DEVICE_CONFIG_STREAM_BUFFER_SIZE    64
#define DEVICE_READ_HOLDING_REGISTER	0x03
#define	DEVICE_READ_INPUT_REGISTER		0x04

static  DEVICE_CONFIG   config_;
static  SERIAL_HANDLE   serial_;
static  uint8_t         buffer_[DEVICE_CONFIG_STREAM_BUFFER_SIZE];
static  uint16_t        registers_[12];

static  const char* typeNames_[] =
{
    "SIM",
    "GIPAM2000",
    "ACURRA A3300",
    "ACURRA_A3300_2",
    "LS HCUM"
};

static const DEVICE_DESCRIPTION  deviceDescriptions[] = 
{
    {
        .type = DEVICE_TYPE_SIM
    },
    {
        .type = DEVICE_TYPE_GIPAM2000,
        .serial = 
        {
            .port = SERIAL_PORT_3,
            .baudrate = SERIAL_BAUDRATE_19200,
            .parity = SERIAL_PARITY_NONE,
            .dataBits = SERIAL_DATA_BITS_8,
            .stopBits = SERIAL_STOP_BITS_1
        }
    },
    {
        .type = DEVICE_TYPE_ACURRA_A3300,
        .serial = 
        {
            .port = SERIAL_PORT_3,
            .baudrate = SERIAL_BAUDRATE_9600,
            .parity = SERIAL_PARITY_EVEN,
            .dataBits = SERIAL_DATA_BITS_9,
            .stopBits = SERIAL_STOP_BITS_1
        }
    },
    {
        .type = DEVICE_TYPE_ACURRA_A3300_2,
        .serial = 
        {
            .port = SERIAL_PORT_3,
            .baudrate = SERIAL_BAUDRATE_9600,
            .parity = SERIAL_PARITY_EVEN,
            .dataBits = SERIAL_DATA_BITS_9,
            .stopBits = SERIAL_STOP_BITS_1
        }
    },
    {
        .type = DEVICE_TYPE_LS_HCUM,
        .serial = 
        {
            .port = SERIAL_PORT_3,
            .baudrate = SERIAL_BAUDRATE_38400,
            .parity = SERIAL_PARITY_NONE,
            .dataBits = SERIAL_DATA_BITS_8,
            .stopBits = SERIAL_STOP_BITS_1
        }
    }
};

RET_VALUE   DEVICE_GIPAM2000_readData(DEVICE_DATA* data, uint32_t timeout);
RET_VALUE   DEVICE_ACURRA_A3300_readData(DEVICE_DATA* data, uint32_t timeout);
RET_VALUE   DEVICE_ACURRA_A3300_readData2(DEVICE_DATA* data, uint32_t timeout);
RET_VALUE   DEVICE_LS_HCUM_readData(DEVICE_DATA* data, uint32_t timeout);
RET_VALUE   DEVICE_SIM(DEVICE_DATA* data, uint32_t timeout);

RET_VALUE   DEVICE_init(DEVICE_CONFIG* config)
{
    RET_VALUE   ret;
    
    ASSERT(config != NULL);

    memcpy(&config_, config, sizeof(config_));
    
    if (config_.type != DEVICE_TYPE_SIM)
    {
        ret = SERIAL_open(&config_.serial, DEVICE_CONFIG_STREAM_BUFFER_SIZE, &serial_);
        
    }
    else 
    {
        ret = RET_OK;
    }
    
    return  ret;
}

RET_VALUE   DEVICE_final(void)
{
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   DEVICE_getConfig(DEVICE_CONFIG* config)
{
    memcpy(config, &config_, sizeof(config_));
    
    return  RET_OK;
}

RET_VALUE   DEVICE_isSupportType(DEVICE_TYPE type)
{
    if (type < DEVICE_TYPE_MAX)
    {
        return  RET_OK;
    }
    
    return  RET_ERROR;
}


RET_VALUE   DEVICE_getID(uint32_t* id)
{
    ASSERT(id != NULL);
    
    *id = config_.id;
    
    return  RET_OK;
}

RET_VALUE   DEVICE_setID(uint32_t id)
{
    config_.id = id;
    
    return  RET_OK;
}

RET_VALUE   DEVICE_getBaudrate(SERIAL_BAUDRATE* baudrate)
{
    ASSERT(baudrate != NULL);
    
    *baudrate = config_.serial.baudrate;
 
    return  RET_OK;
}

RET_VALUE   DEVICE_setBaudrate(SERIAL_BAUDRATE baudrate)
{
    
	config_.serial.baudrate = baudrate;  

    return  RET_OK;
}

RET_VALUE   DEVICE_getParity(SERIAL_PARITY* parity)
{
    ASSERT(parity != NULL);
    
    *parity = config_.serial.parity;
 
    return  RET_OK;
}

RET_VALUE   DEVICE_setParity(SERIAL_PARITY parity)
{
    config_.serial.parity = parity;  

    return  RET_OK;
}

RET_VALUE   DEVICE_getDataBits(SERIAL_DATA_BITS* databits)
{
    ASSERT(databits != NULL);
    
    *databits = config_.serial.dataBits;
 
    return  RET_OK;
}

RET_VALUE   DEVICE_setDataBits(SERIAL_DATA_BITS databits)
{
    config_.serial.dataBits = databits;  

    return  RET_OK;
}

RET_VALUE   DEVICE_getStopBits(SERIAL_STOP_BITS* stopbit)
{
    ASSERT(stopbit != NULL);
    
    *stopbit = config_.serial.stopBits;
 
    return  RET_OK;
}

RET_VALUE   DEVICE_setStopBits(SERIAL_STOP_BITS stopbit)
{
    config_.serial.stopBits = stopbit;  

    return  RET_OK;
}

RET_VALUE   DEVICE_getType(DEVICE_TYPE *type)
{
    ASSERT(type != NULL);
    
    *type = config_.type;
    
    return  RET_OK;
}

RET_VALUE   DEVICE_setType(DEVICE_TYPE type)
{
    RET_VALUE   ret;
    
    ret = DEVICE_isSupportType(type);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    if (config_.type != type)
    {
        config_.type = type;
        config_.serial.baudrate = deviceDescriptions[type].serial.baudrate;
        config_.serial.parity  = deviceDescriptions[type].serial.parity;
        config_.serial.dataBits = deviceDescriptions[type].serial.dataBits;
        config_.serial.stopBits = deviceDescriptions[type].serial.stopBits;
    }

    return  ret;
}

char const*   DEVICE_getTypeName(DEVICE_TYPE type)
{
    RET_VALUE   ret;
    
    ret = DEVICE_isSupportType(type);
    if (ret != RET_OK)
    {
        return  "Unknown";
    }
    
    return  typeNames_[type];
}

uint32_t    DEVICE_getTimeout(void)
{
    return  config_.timeout;
}

RET_VALUE   DEVICE_setTimeout(uint32_t timeout)
{
    config_.timeout = timeout;
    
    return  RET_OK;
}

//RET_VALUE   DEVICE_readHoldingRegisters(uint8_t id, uint16_t address, uint16_t count, uint16_t *registers, uint32_t timeout)
RET_VALUE   DEVICE_readHoldingRegisters(uint8_t id, uint8_t function, uint16_t address, uint16_t count, uint16_t *registers, uint32_t timeout)
{
    uint8_t     i, bufferLength = 0, receivedLength = 0;
    uint16_t    crc;
        

    buffer_[bufferLength++] = id;
    buffer_[bufferLength++] = function;
    buffer_[bufferLength++] = (address >> 8) & 0xFF;
    buffer_[bufferLength++] = (address & 0xFF);
    buffer_[bufferLength++] = (count >> 8) & 0xFF;
    buffer_[bufferLength++] = (count & 0xFF);
    
    crc = CRC16_calc(buffer_, bufferLength);
    buffer_[bufferLength++] = (crc & 0xFF);
    buffer_[bufferLength++] = (crc >> 8) & 0xFF;

    RET_VALUE   ret = SERIAL_puts(serial_, buffer_, bufferLength, timeout);
    //bufferLength = sizeof(buffer_);
    bufferLength = 3 + count * 2 + 2;
    if (ret == RET_OK)
    {
      TickType_t  startTick = xTaskGetTickCount();
      
      receivedLength = 0;
        while(bufferLength > 0)
        {
            TickType_t currentTick = xTaskGetTickCount();

            if (currentTick - startTick >= timeout)
            {
                break;
            }
            uint8_t length = 0;
            length = SERIAL_gets(serial_, (uint8_t*)&buffer_[receivedLength], bufferLength, timeout - (currentTick - startTick));
            /*if (length == 0)
            {
                break;
            }*/
            
            receivedLength  += length;
            bufferLength    -= length;
        }
    
        if (!receivedLength) 
        {
            ret = RET_TIMEOUT;
        }
        else if (receivedLength != (3 + count * 2 + 2))
        {
            ret = RET_INVALID_FRAME;
        }
        else 
        {
            crc = CRC16_calc(buffer_, receivedLength - 2);
            if (((crc & 0xFF) != buffer_[receivedLength - 2]) || (((crc >> 8) & 0xFF) != buffer_[receivedLength - 1]))
            {
                ret = RET_INVALID_FRAME;
            }
        }
        
    }

    if (ret == RET_OK)
    {
        for(i = 0 ; i < count ; i++)
        {
            registers[i] = ((uint16_t)buffer_[3+i*2] << 8) | buffer_[3+i*2 + 1];
        }
    }
    
    return  ret;
}    

RET_VALUE   DEVICE_readData(DEVICE_DATA* data, uint32_t timeout)
{
    switch(config_.type)
    {
    case    DEVICE_TYPE_GIPAM2000:
        return  DEVICE_GIPAM2000_readData(data, timeout);
    case    DEVICE_TYPE_ACURRA_A3300:
        return  DEVICE_ACURRA_A3300_readData(data, timeout);
    case    DEVICE_TYPE_ACURRA_A3300_2:
        return  DEVICE_ACURRA_A3300_readData2(data, timeout);
    case    DEVICE_TYPE_LS_HCUM :
        return  DEVICE_LS_HCUM_readData(data, timeout);
    case    DEVICE_TYPE_SIM:
        return  DEVICE_SIM(data, timeout);

    }

    return  RET_ERROR;
}

RET_VALUE   DEVICE_GIPAM2000_readData(DEVICE_DATA* data, uint32_t timeout)
{
    RET_VALUE   ret;
        
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_INPUT_REGISTER, 12, 6, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    osDelay(10);
    int32_t value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    data->voltage.RS = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[2] << 16) | ((uint32_t)registers_[3]));
    data->voltage.ST = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[4] << 16) | ((uint32_t)registers_[5]));
    data->voltage.TR = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);
osDelay(10);
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_INPUT_REGISTER, 20, 6, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    data->current.R = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[2] << 16) | ((uint32_t)registers_[3]));
    data->current.S = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[4] << 16) | ((uint32_t)registers_[5]));
    data->current.T = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);
    osDelay(10);
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_INPUT_REGISTER, 38, 2, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    //data->totalPower = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);  // GIPAM2K totalPower W
	data->totalPower = (int32_t)(IEEE754_Binary32ToDouble(value) * 0.001 * 100); // GIPAM2K totalPower KW
osDelay(10);
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_INPUT_REGISTER, 48, 2, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    //data->totalEnergy = (int32_t)(IEEE754_Binary32ToDouble(value) * 100); // GIPAM2K totalEnergy Wh
	data->totalEnergy = (int32_t)(IEEE754_Binary32ToDouble(value) * 0.001); // GIPAM2K totalEnergy KWh
	
    return  ret;
}

RET_VALUE   DEVICE_ACURRA_A3300_readData(DEVICE_DATA* data, uint32_t timeout)
{
    RET_VALUE   ret;
    uint16_t    scale;
    
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 108, 1, &scale, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 104, 3, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    data->voltage.RS = (uint32_t)(registers_[0] * scale * 0.1 * 100);
    data->voltage.ST = (uint32_t)(registers_[1] * scale * 0.1 * 100);
    data->voltage.TR = (uint32_t)(registers_[2] * scale * 0.1 * 100);
    
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 117, 1, &scale, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 109, 3, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    data->current.R = (uint32_t)(registers_[0] * scale * 0.001 * 100);
    data->current.S = (uint32_t)(registers_[1] * scale * 0.001 * 100);
    data->current.T = (uint32_t)(registers_[2] * scale * 0.001 * 100);

    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 122, 2, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    data->totalPower = (int32_t)(((int16_t)registers_[0]) * (uint16_t)registers_[1] * 0.001 * 100);

    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 141, 2, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    data->totalEnergy = ((int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1])));

    return  ret;
}


RET_VALUE   DEVICE_ACURRA_A3300_readData2(DEVICE_DATA* data, uint32_t timeout)
{
    RET_VALUE   ret;
        
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 9006, 12, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    int32_t value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    data->voltage.RS = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[2] << 16) | ((uint32_t)registers_[3]));
    data->voltage.ST = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[4] << 16) | ((uint32_t)registers_[5]));
    data->voltage.TR = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[6] << 16) | ((uint32_t)registers_[7]));
    data->current.R = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[8] << 16) | ((uint32_t)registers_[9]));
    data->current.S = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    value = (int32_t)(((uint32_t)registers_[10] << 16) | ((uint32_t)registers_[11]));
    data->current.T = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);
    
    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 9026, 2, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    data->totalPower = (int32_t)(IEEE754_Binary32ToDouble(value) * 100);

    ret = DEVICE_readHoldingRegisters(config_.id, DEVICE_READ_HOLDING_REGISTER, 9049, 2, registers_, timeout);
    if (ret != RET_OK)
    {
        return  ret;
    }
    
    value = (int32_t)(((uint32_t)registers_[0] << 16) | ((uint32_t)registers_[1]));
    //data->totalEnergy = (int32_t)(IEEE754_Binary32ToDouble(value));
	data->totalEnergy = value;
    return  ret;
}

RET_VALUE   DEVICE_LS_HCUM_readData(DEVICE_DATA* data, uint32_t timeout)
{
    RET_VALUE   ret;
    uint8_t     hcum_request[13];
    
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////Request Frame///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    // NAME : SOH  CMD  ROUTER  HCU_ID  DATA_PORT   ETX BCC               //
    // BYTE :  1    1      2       5         1       1   1     : 13 byte  //
    ////////////////////////////////////////////////////////////////////////
    
    hcum_request[0] = 0x01;                                     //SOH : always 01h
    hcum_request[1] = 0x44;                                     //CMD 'D' : request = 'D', response = 'd'
    hcum_request[2] = 0x02;                                     //STX : always 02h
    hcum_request[3] = 0x00;                                     //Routers MSB
    hcum_request[4] = 0x00;                                     //Routers LSB
    // HCU ID = #1#2#3#4#5 (5th interger) -> ASCII
    hcum_request[5] = (config_.id / 10000) + 0x30;              // HCU ID #1
    hcum_request[6] = ((config_.id % 10000) / 1000 ) + 0x30;    // HCU ID #2
    hcum_request[7] = ((config_.id % 1000) / 100 ) + 0x30;      // HCU ID #3
    hcum_request[8] = ((config_.id % 100) / 10 ) + 0x30;        // HCU ID #4
    hcum_request[9] = (config_.id % 10) + 0x30;                 // HCU ID #5
    hcum_request[10] = 0x31 ;                                   // read port 1 data(WHM) is '1'(0x31), read all port data is 'A'(0x41) 
    hcum_request[11] = 0x03;                                    //ETX
    uint8_t BCC_Result = 0;
    for(uint8_t i = 1 ; i < 12 ; i++)
    {
        BCC_Result +=   hcum_request[i];
    }
    hcum_request[12] = BCC_Result;                               //BCC : Last one Byte
    
    /////////////Send Request And Receive Response///////////////////////////
    
    /////Send Request//////
    
    ret = SERIAL_puts(serial_, hcum_request, sizeof(hcum_request), timeout);
    
    /////Receive Response //////
    
    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////////////Response Frame///////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    //////// Name : SOH CMD STX HCU_ID  PORT1_DATA  ETX BCC
    //////// byte :  1   1   1     5         9       1   1
    
    uint8_t hcum_response[19];
    memset(hcum_response, 0x00, sizeof(hcum_response));
    
    uint8_t response_length = 19;
    uint8_t receivedLength = 0;
    if (ret == RET_OK)
    {
        TickType_t  startTick = xTaskGetTickCount();
      
        receivedLength = 0;
        while(response_length > 0)
        {
            TickType_t currentTick = xTaskGetTickCount();

            if (currentTick - startTick >= timeout)
            {
                break;
            }
            uint8_t length = 0;
            length = SERIAL_gets(serial_, (uint8_t*)&hcum_response[receivedLength], response_length, timeout - (currentTick - startTick));
            /*if (length == 0)
            {
                break;
            }*/
            
            receivedLength  += length;
            response_length -= length;
        }
    
        if (!receivedLength) 
        {
            ret = RET_TIMEOUT;
        }
        else if (receivedLength != 19)
        {
            ret = RET_INVALID_FRAME;
        }
        else 
        {
            if( (hcum_response[1] != 'd') || (hcum_response[0] != 0x01) || (hcum_response[2] != 0x02))
            {
                ret = RET_INVALID_FRAME;
            }
            else
            {
                uint8_t response_bcc = 0;
                for( uint8_t i = 1 ; i < 18 ; i++)
                {
                    response_bcc += hcum_response[i];
                }
                if( response_bcc != hcum_response[18])
                {
                    ret = RET_INVALID_FRAME;
                }
            }
        }
    }

    /////////Data Save //////
    if (ret == RET_OK)
    {
        data->voltage.RS = 0;
        data->voltage.ST = 0;
        data->voltage.TR = 0;
        data->current.R = 0;
        data->current.S = 0;
        data->current.T = 0;
        data->totalPower = 0;
        uint8_t data_str[10];
        memset(data_str, 0x00, sizeof(data_str));
        memcpy(data_str, &hcum_response[8], 9);
        data->totalEnergy = (int32_t)(atoi((char*)data_str) * 0.001 * 100);
    }

    return  ret;
}

RET_VALUE   DEVICE_SIM(DEVICE_DATA* data, uint32_t timeout)
{
    data->voltage.RS += rand() % 100;
    if (data->voltage.RS > 22000)
    {
        data->voltage.RS = 19000;
    }
    data->voltage.ST += rand() % 100;
    if (data->voltage.ST > 22000)
    {
        data->voltage.ST = 19000;
    }
    data->voltage.TR += rand() % 100;
    if (data->voltage.TR > 22000)
    {
        data->voltage.TR = 19000;
    }
    data->current.R += rand() % 100;
    if (data->current.R > 10000)
    {
        data->current.R = 0;
    }
    data->current.S += rand() % 100;
    if (data->current.S > 10000)
    {
        data->current.S = 0;
    }
    data->current.T += rand() % 100;
    if (data->current.T > 10000)
    {
        data->current.T = 0;
    }
    data->totalPower += rand() % 100;
    if (data->totalPower > 10000)
    {
        data->totalPower = 0;
    }
    data->totalEnergy += rand() % 100;
    if (data->totalEnergy > 1000000)
    {
        data->totalEnergy = 0;
    }
        
    return  RET_OK;
}
