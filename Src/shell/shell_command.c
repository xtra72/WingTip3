#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "assert.h"
#include "config.h"
#include "me_i10kl.h"
#include "device.h"
#include "shell.h"
#include "utils.h"
#include "shell/shell_device.h"
#include "shell/shell_client.h"
#include "shell/shell_i10kl.h"
#include "shell/shell_system.h"
#include "shell/shell_log.h"

extern  CONFIG  config_;

RET_VALUE SHELL_COMMAND_getVersion(char *argv[], uint32_t argc, struct _SHELL_COMMAND* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", CONFIG_VERSION);
        ret = RET_OK;
    }   
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "help") == 0)
        {
            SHELL_printf("Version\n");
            ret = RET_OK;
        }
    }
   
    
    return  ret;
}

RET_VALUE SHELL_COMMAND_activation(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_OK;
    
    if (argc == 1)
    {
        SHELL_printf("%16s : %s\n", "Activation", SYSTEM_getActivation()?"Enable":"Disable");
        if (SYSTEM_getActivation())
        {
            FI_TIME time;
            SYSTEM_getActivationTime(&time);
            SHELL_printf("%16s : %s\n", "Activation Time", FI_TIME_toString(time, "%Y-%m-%d %H:%M:%S"));
        }
    }
    else if (argc == 2)
    {
        FI_TIME currentTime;
        
        FI_TIME_get(&currentTime);
        
        if ((strcasecmp(argv[1], "on") == 0) || (strcasecmp(argv[1], "enable") == 0))
        {
            if (currentTime >= 946684800)
            {
                    SYSTEM_setActivation(true);
            }
            else
            {
                SHELL_printf("Time must be set first.\n");
                ret = RET_INVALID_ARGUMENT;
            }
        }
        else if ((strcasecmp(argv[1], "off") == 0) || (strcasecmp(argv[1], "disable") == 0))
        {
            SYSTEM_setActivation(false);
        }
            
    }
    else
    {
        ret = RET_INVALID_ARGUMENT;
    }

    if (ret != RET_OK)
    {
        SHELL_printf("Error : Invalid arguments.\n");
    }
    
    return  RET_OK;
}



RET_VALUE SHELL_COMMAND_date(char *argv[], uint32_t argc, struct _SHELL_COMMAND* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        FI_TIME time;
        
        ret = FI_TIME_get(&time);
        if (ret == RET_OK)
        {
            SHELL_printf("%s (%d)\n", FI_TIME_toString(time, "%Y-%m-%d %H:%M:%S"), time);
        }
        else 
        {
            SHELL_printf("Error : Can't get time!\n");
        }
   }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "init") == 0)
        {
            FI_TIME_init();
        }
        else 
        {
            uint32_t    i;
            uint32_t    length;
            
            length = strlen(argv[1]);
            if (length == 14)
            {
                ret = RET_OK;
                
                for(i = 0 ; i < length ; i++)
                {
                    if (!isdigit(argv[1][i]))
                    {
                        SHELL_printf("Error : Invalid arguments.\n");
                        ret = RET_INVALID_ARGUMENT;
                        break;
                    }
                }
            }
            
            if (ret == RET_OK)
            {
                struct tm   tm;
                uint32_t    value;

                memset(&tm, 0, sizeof(tm));
                
                ret = RET_INVALID_ARGUMENT;
                
                value =  strtoul(&argv[1][12], 0, 10);
                argv[1][12] = 0;
                if (value < 60)
                {
                    tm.tm_sec   = value;
                    value = strtoul(&argv[1][10], 0, 10);
                    argv[1][10] = 0;
                    if (value < 60)
                    {
                        tm.tm_min   = value;
                        value = strtoul(&argv[1][8], 0, 10);
                        argv[1][8] = 0;
                        if (value < 24)
                        {
                            tm.tm_hour  = value;
                            value = strtoul(&argv[1][6], 0, 10);
                            argv[1][6] = 0;
                            if (1 <= value && value <= 31)
                            {
                                tm.tm_mday  = value;
                                value = strtoul(&argv[1][4], 0, 10);
                                argv[1][4] = 0;
                                if (1 <= value && value <= 12)
                                {
                                    tm.tm_mon   = value - 1;
                                    value = strtoul(&argv[1][0], 0, 10);
                                    if (2000 <= value && value <= 2100)
                                    {
                                        tm.tm_year   = value - 1900;

                                        value = mktime(&tm) - FI_TIME_getTimeZone();
                                        SHELL_printf("value : %s", ctime(&value));
                                        ret = FI_TIME_set(value);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return  ret;
}


RET_VALUE SHELL_COMMAND_config(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    switch(argc)
    {
    case    1:
        {
            SHELL_printf("\n%s\n", "[ System ]");
            SHELL_printf("%16s : %s\n", "S/N", config_.serialNumber);
            SHELL_SYSTEM_printConfig(&config_.system);

            SHELL_printf("\n%s\n", "[ Client ]");
            SHELL_CLIENT_printConfig(&config_.client);
            
            SHELL_printf("\n%s\n", "[ Device ]");
            SHELL_DEVICE_printConfig(&config_.device);
            
            SHELL_printf("\n%s\n", "[ ME I10KL ]");
            SHELL_I10KL_printConfig(&config_.me_i10kl);

            SHELL_printf("\n%s\n", "[ Log ]");
            SHELL_LOG_printConfig(&config_.log);

            SHELL_printf("\n%s\n", "[ Debug ]");
            SHELL_printf("%16s : %s\n", "Trace", TRACE_getEnable()?"ON":"OFF");

            ret = RET_OK;
        }
        break;
        
    case    2:
        {
            if (strcasecmp(argv[1], "save") == 0)
            {
                CONFIG* config = pvPortMalloc(sizeof(CONFIG));
                if (config == NULL)
                {
                    ret = RET_NOT_ENOUGH_MEMORY;
                    break;
                }
                
                memcpy(config, &config_, sizeof(CONFIG));
                ret = SYSTEM_getConfig(&config->system);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get system settings!\n");
                    break;                        
                }

                ret = CLIENT_getConfig(&config->client);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get client settings!\n");
                    break;                        
                }
                
                ret = DEVICE_getConfig(&config->device);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get device settings!\n");
                    break;                        
                }

                ret = SHELL_getConfig(&config->shell);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get shell settings!\n");
                    break;                        
                }
                
                ret = ME_I10KL_getConfig(&config->me_i10kl);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get ME I10KL settings!\n");
                    break;                        
                }
                
                ret = LOG_getConfig(&config->log);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get LOG settings!\n");
                    break;                        
                }
                
                ret = TRACE_getConfig(&config->trace);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Can't get TRACE settings!\n");
                    break;                        
                }
                
                ret = CONFIG_save(config);
                if (ret != RET_OK)
                {
                    vPortFree(config);
                    SHELL_printf("Failed to save settings!\n");
                    break;                        
                }
                
                memcpy(&config_, config, sizeof(CONFIG));
                vPortFree(config);
            }
            else if (strcasecmp(argv[1], "load") == 0)
            {
                ret = CONFIG_load(&config_);
            }
            else if (strcasecmp(argv[1], "clean") == 0)
            {
                ret = CONFIG_clear();
            }
        }
        break;
        
        
    default:
        ret = RET_INVALID_ARGUMENT;
    }
    
   
    return  ret;
}

RET_VALUE SHELL_COMMAND_serialNumber(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    if (argc == 1)
    {
        SHELL_printf("%s\n", config_.serialNumber);
    }
    else if (argc == 3)
    {
        if (strcasecmp(argv[1], "set") == 0)
        {
            if (strlen(argv[2]) < CONFIG_SERIAL_NUMBER_LEN)
            {
                strcpy(config_.serialNumber, argv[2]);
                SHELL_printf("S/N changed to %s\n", config_.serialNumber);
            }
            else 
            {
                SHELL_printf("Invalid S/N\n");
            }
        }
    }

    return  RET_OK;
}

RET_VALUE SHELL_COMMAND_bat(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_printf("Battery Level : %d %\n", SYS_getBatteryLevel());

    return  RET_OK;
}

RET_VALUE   SHELL_COMMAND_sleep(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    HAL_PWR_EnterSTANDBYMode();

    return  RET_OK;
}

RET_VALUE SHELL_COMMAND_extPower(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
  	if (argc == 1)
  	{
		SHELL_printf("Ext Power 1.8V : %s\n", SYS_getExtPower18()?"ON":"OFF");
		SHELL_printf("Ext Power 3.3V : %s\n", SYS_getExtPower33()?"ON":"OFF");
  	}
  	else if (argc == 2)
  	{
        if (strcasecmp(argv[1], "on") == 0)
        {
            ME_I10KL_setPower(true);
        }
        else if (strcasecmp(argv[1], "off") == 0)
        {
            ME_I10KL_setPower(false);
        }
    }
  	else if (argc == 3)
  	{
		if (strcasecmp(argv[1], "18") == 0)
        {
            if (strcasecmp(argv[2], "on") == 0)
            {
         		SYS_setExtPower18(true);
            }
            else if (strcasecmp(argv[2], "off") == 0)
            {
         		SYS_setExtPower18(false);
            }
        }
		else if (strcasecmp(argv[1], "33") == 0)
        {
            if (strcasecmp(argv[2], "on") == 0)
            {
         		SYS_setExtPower33(true);
            }
            else if (strcasecmp(argv[2], "off") == 0)
            {
         		SYS_setExtPower33(false);
            }
        }
    }
  
    return  RET_OK;
}


RET_VALUE SHELL_COMMAND_at(char *argv[], uint32_t argc, struct _SHELL_COMMAND* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    static  char    response[64];
    uint32_t        receivedLength;
    
    if (argc == 1)
    {
        ret = ME_I10KL_at(NULL, response, sizeof(response), &receivedLength);
        if (ret == RET_OK)
        {
            SHELL_printf("%s\n", response);
        }
    }
    if (argc == 2)
    {
        ret = ME_I10KL_at(argv[1], response, sizeof(response), &receivedLength);
        if (ret == RET_OK)
        {
            SHELL_printf("%s\n", response);
        }
    }
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
        
    return  ret;
}

RET_VALUE SHELL_COMMAND_socket(char *argv[], uint32_t argc, struct _SHELL_COMMAND* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 3)
    {   
        uint32_t    protocol = 0;
        if (strcasecmp(argv[1], "tcp") == 0)
        {
            protocol = 6;
        }
        else if (strcasecmp(argv[1], "udp") == 0)
        {
            protocol = 17;
        }
        else 
        {
            SHELL_printf("Not supported protocol : %s\n", argv[2]);
            return  RET_OK;
        }
        
        uint32_t    socket;
        uint16_t    port = strtoul(argv[2], 0, 10);
        
        ret = ME_I10KL_createSocket(protocol, port, &socket);
        if (ret == RET_OK)
        {
            SHELL_printf("Socket : %d\n", socket);
        }
    }
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
        
    return  ret;
}

RET_VALUE SHELL_COMMAND_receive(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    static  char    response[64];
    
    ret = ME_I10KL_receive(response, sizeof(response));
    if (ret == RET_OK)
    {
        SHELL_printf("Response : %s\n", response);
    }
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
        
    return  ret;
}

RET_VALUE SHELL_COMMAND_reset(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SYS_reset();
    
    return  RET_OK;
}


RET_VALUE SHELL_COMMAND_trace(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    switch(argc)
    {
    case    1:
        {
            SHELL_printf("%16s : %s\n", "Trace", TRACE_getEnable()?"ON":"OFF");
            ret = RET_OK;
        }
        break;
        
    case    2:
        {
            if (strcasecmp(argv[1], "on") == 0)
            {
                TRACE_setEnable(true);
                SHELL_printf("Trace started.\n");
                ret = RET_OK;
            }
            else if (strcasecmp(argv[1], "off") == 0)
            {
                TRACE_setEnable(false);
                SHELL_printf("Trace stopped.\n");
                ret = RET_OK;
            }
        }
        break;
    }
    
    return  ret;
}
