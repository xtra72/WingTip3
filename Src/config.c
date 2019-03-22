#include "stm32l1xx_hal.h"
#include "config.h"
#include "crc16.h"
#include "trace.h"
#include "flash.h"

#define TRACE(...)  TRACE_printf("CONFIG", __VA_ARGS__)

RET_VALUE   CONFIG_saveAt(uint32_t index, CONFIG* config);

char    *CONFIG_SHELL_BANNER[] = 
{
    "\r\n==========================================",
    "\r\n=   (C) COPYRIGHT 2018 FutureICT         =",
    "\r\n=                         FCN-10S        =",
    "\r\n=                         By DevTeam     =",
    "\r\n==========================================",
    "\r\n",
    "\r\n",
    NULL
};

const CONFIG  defaultConfig = 
{
    .crc        =   0,
    .sequence   =   0,
    .serialNumber=  "FCN-10S-00000000",
    .system     =
    {
        .activation = 0,
        .activationTime = 0,
        .wdog   =
        {
            .enable = false
        }
    },
    .client     = 
    {
        .id     =   CONFIG_DEFAULT_CLIENT_ID,
        .autoRun=   false,
        .maxRetryCount = 3,
        .server = 
        {
            .ip     = CONFIG_DEFAULT_SERVER_IP,
            .port   = CONFIG_DEFAULT_SERVER_PORT,
            .userId = 0,
            .topic  = CONFIG_DEFAULT_TOPIC
        },
        
        .opMode = 
        {
            .mode = CLIENT_MODE_SLEEP_WITH_ALARM,
            .interval = 
            {
                .start = 0,
                .period = 600
            },
            .alarm = 
            {
                .count = 2,
                .times = 
                {
                    0,
                    43200,
                }        
            }
        },
        .delay =
        {
            .enable = true,
            .mode = CLIENT_DELAY_MODE_WAKEUP_DELAY,
            .base = 180,
            .period = 240,
            .member = 24,
            .offset = 0
        },
        .message = 
        {
            .voltage = true,
            .current = true,
            .power = true,
            .energy = true
        },

    },
    .me_i10kl =
    {
        .serial = 
        {
            .port = SERIAL_PORT_2,
            .baudrate = SERIAL_BAUDRATE_115200,
            .parity = SERIAL_PARITY_NONE,
            .dataBits = SERIAL_DATA_BITS_8,
            .stopBits = SERIAL_STOP_BITS_1,
            .priority = 5
        },
        .timeout = CONFIG_DEFAULT_MODEM_TIMEOUT
    },
    .device = 
    {
        .id = CONFIG_DEFAULT_DEVICE_ID,
        .swap = CONFIG_DEFAULT_DEVICE_SWAP,
        .type = CONFIG_DEFAULT_DEVICE_TYPE,
        .serial = 
        {
            .port = SERIAL_PORT_3,
            .baudrate = SERIAL_BAUDRATE_9600,
            .parity = SERIAL_PARITY_NONE,
            .dataBits = SERIAL_DATA_BITS_8,
            .stopBits = SERIAL_STOP_BITS_1,
            .priority = 5
        },
        .timeout = CONFIG_DEFAULT_DEVICE_TIMEOUT
    },
    .shell = 
    {
        .serial = 
        {
            .port = SERIAL_PORT_1,
            .baudrate = SERIAL_BAUDRATE_38400,
            .parity = SERIAL_PARITY_NONE,
            .dataBits = SERIAL_DATA_BITS_8,
            .stopBits = SERIAL_STOP_BITS_1,
            .priority = 5
        }
    },
    .log = 
    {
        .enable = true
    },
    .trace = 
    {
        .enable = false
    }
};

RET_VALUE   CONFIG_init(void)
{
    return  RET_OK;
}

RET_VALUE   CONFIG_loadDefault(CONFIG* config)
{
    ASSERT(config != NULL);
    
    memcpy(config, &defaultConfig, sizeof(CONFIG));
    
    return  RET_OK;
}

RET_VALUE   CONFIG_load(CONFIG* config)
{
    ASSERT(config != NULL);
    
    uint32_t    i;
    CONFIG*     last = NULL;
    
    for(i = 0 ; i < CONFIG_SLOT_COUNT ; i++)
    {
        CONFIG* item = (CONFIG*)(CONFIG_START_ADDRESS + CONFIG_SIZE * i);
        if (CONFIG_isValid(item))
        {
            if ((last == NULL) ||(last->sequence < item->sequence))
            {
                last = item;
            }
        }
    }
    
    if (last == NULL)
    {
        return  RET_ERROR;
    }
    
    memcpy(config, last, sizeof(CONFIG));
    
    return  RET_OK;
}

RET_VALUE   CONFIG_save(CONFIG* config)
{
    ASSERT(config != NULL);
    
    uint32_t    i, index = 0;
    CONFIG*     last = NULL;
    
    for(i = 0 ; i < CONFIG_SLOT_COUNT ; i++)
    {
        CONFIG* item = (CONFIG*)(CONFIG_START_ADDRESS + CONFIG_SIZE * i);
        if (CONFIG_isValid(item))
        {
            if ((last == NULL) ||(last->sequence < item->sequence))
            {
                index = i;
                last = item;
            }
        }
    }
    
        config->sequence++;
    config->crc = CRC16_calc(&config->sequence, sizeof(CONFIG) - sizeof(config->crc));
    
    if (last == NULL)
    {        
        return  FLASH_write(CONFIG_START_ADDRESS, config, sizeof(CONFIG));
    }

    index = (index + 1) % CONFIG_SLOT_COUNT;
    return  FLASH_write(CONFIG_START_ADDRESS + CONFIG_SIZE * index, config, sizeof(CONFIG));
}


RET_VALUE   CONFIG_clear(void)
{
    return  FLASH_erase(CONFIG_START_ADDRESS, CONFIG_SIZE * CONFIG_SLOT_COUNT);
}

bool    CONFIG_isValid(CONFIG* config)
{
    ASSERT(config != NULL);
    
    return  config->crc == CRC16_calc(&config->sequence, sizeof(CONFIG) - sizeof(config->crc));
}
