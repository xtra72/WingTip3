#ifndef CONFIG_H_
#define CONFIG_H_

#include "target.h"
#include "shell.h"
#include "me_i10kl.h"
#include "client.h"
#include "device.h"
#include "fi_time.h"
#include "system.h"
#include "log.h"

extern  char    *CONFIG_SHELL_BANNER[];

#define CONFIG_PRODUCT_NAME         "FCN-10S"
#define CONFIG_VERSION              "19.02.10-A"

#define CONFIG_USER_MEMORY_START        0x08020000
#define CONFIG_USER_MEMORY_END          0x0803FFFF

#define CONFIG_DEFAULT_WDOG_TIMEOUT     120000
#define CONFIG_DEFAULT_DEVICE_ID        1
#define CONFIG_DEFAULT_DEVICE_SWAP      false 
#define CONFIG_DEFAULT_DEVICE_TYPE      DEVICE_TYPE_ACURRA_A3300_2
#define CONFIG_DEFAULT_DEVICE_TIMEOUT   5000

#define CONFIG_DEFAULT_MODEM_TIMEOUT    10000

#define CONFIG_DEFAULT_CLIENT_ID        "NB_00000"
#define CONFIG_DEFAULT_SERVER_IP        {203, 226, 64, 79}
#define CONFIG_DEFAULT_SERVER_PORT      1883
#define CONFIG_DEFAULT_TOPIC            "dhi/energy"

#define CONFIG_CLIENT_PERIOD_MIN        18

#define CONFIG_SHELL_STACK_SIZE 256
#define CONFIG_SHELL_PRIORITY   osPriorityNormal

#define CONFIG_SERIAL_NUMBER_LEN    32
#define CONFIG_PASSWORD_LEN         32

#define CONFIG_START_ADDRESS        0x08020000  /* Start @ of user Flash area */
#define CONFIG_SIZE                 0x00000400
#define CONFIG_SLOT_COUNT           4

#define CONFIG_LOG_START_ADDRESS    0x08022000  /* Start @ of user Flash area */
#define CONFIG_LOG_SIZE             FLASH_PAGE_SIZE
#define CONFIG_LOG_COUNT_MAX        64

typedef struct
{
    uint32_t        crc;
    uint32_t        sequence;
    
    char            password[CONFIG_PASSWORD_LEN];
    char            serialNumber[CONFIG_SERIAL_NUMBER_LEN];
    
    SYSTEM_CONFIG   system;
    CLIENT_CONFIG   client;
    ME_I10KL_CONFIG me_i10kl;
    DEVICE_CONFIG   device;
    SHELL_CONFIG    shell;
    LOG_CONFIG      log;
    TRACE_CONFIG    trace;
}   CONFIG;


RET_VALUE   CONFIG_init(void);
RET_VALUE   CONFIG_loadDefault(CONFIG* config);
RET_VALUE   CONFIG_load(CONFIG* config);
RET_VALUE   CONFIG_save(CONFIG* config);
bool        CONFIG_isValid(CONFIG* config);
RET_VALUE   CONFIG_clear(void);


extern  const   char firmwareVersion[];

extern  CONFIG  config;
#endif