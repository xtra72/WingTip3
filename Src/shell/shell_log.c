#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assert.h"
#include "config.h"
#include "system.h"
#include "client.h"
#include "shell.h"
#include "utils.h"
#include "trace.h"
#include "shell/shell_log.h"


RET_VALUE SHELL_LOG_disable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_LOG_enable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_LOG_show(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_LOG_count(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_LOG_clear(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_LOG_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);

RET_VALUE   SHELL_LOG_printClientLogTitle(void);
RET_VALUE SHELL_LOG_printClientLog(CLIENT_LOG* clientLog);

static SHELL_COMMAND   commandSet_[] = 
{
    {   
        .name = "enable",     
        .function = SHELL_LOG_enable,         
        .shortHelp = "Log Enable"
    },
    {   
        .name = "disable",     
        .function = SHELL_LOG_disable,         
        .shortHelp = "Log Disable"
    },
    {   
        .name = "count",     
        .function = SHELL_LOG_count,         
        .shortHelp = "Count"
    },
    {   
        .name = "show",     
        .function = SHELL_LOG_show,         
        .shortHelp = "Show"
    },
    {   
        .name = "clear",     
        .function = SHELL_LOG_clear,         
        .shortHelp = "Delete all logs"
    },
    {   
        .name = "help",     
        .function = SHELL_LOG_help,         
        .shortHelp = "Help"
    },
    {
        .name = NULL,     
    }
};

RET_VALUE   SHELL_COMMAND_log(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_COMMAND;
    if (argc == 1)
    {
        LOG_CONFIG   config;
        uint32_t    count;
        LOG_getConfig(&config);

        SHELL_LOG_printConfig(&config);
        LOG_getCount(&count);
        LOG_showDebug();
        SHELL_printf("%16s : %d\n", "Count", count);
    }
    else 
    {
        SHELL_COMMAND*   subcommand = commandSet_;
        while(subcommand->name != NULL)
        {
            if (strcasecmp(subcommand->name, argv[1]) == 0)
            {
                ret = subcommand->function(&argv[1], argc - 1, subcommand);
                break;
            }
            
            subcommand++;
        }
    }
    
    return  ret;
}

RET_VALUE SHELL_LOG_enable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    LOG_setEnable(true);
    
    return  RET_OK;
}

RET_VALUE SHELL_LOG_disable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    LOG_setEnable(false);
    
    return  RET_OK;
}

RET_VALUE SHELL_LOG_show(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    LOG*        log = pvPortMalloc(sizeof(LOG));
    if (log == NULL)
    {
        SHELL_printf("Not enough memory!\n");
        return  RET_OK;
    }
    
    if (argc == 1)
    {        
        uint32_t    count = 0;
        
        LOG_getCount(&count);
        for(uint32_t i = 0 ; i < count ; i++)
        {
            if (LOG_getAt(i, log) == RET_OK)
            {                
                if (log->type == LOG_TYPE_MESSAGE)
                {
                    SHELL_printf("[%2d][%s] : %s\n", i, FI_TIME_toString(log->time, "%Y-%m-%d %H:%M:%S"), log->body.message);
                }
                else if (log->type == LOG_TYPE_RAW)
                {
                    if (log->body.raw.length == sizeof(CLIENT_LOG))
                    {
                        CLIENT_LOG* clientLog = (CLIENT_LOG*)log->body.raw.data;
                        SHELL_printf("[%2d][%s] : ", i, FI_TIME_toString(log->time, "%Y-%m-%d %H:%M:%S"));
                        SHELL_LOG_printClientLog(clientLog);
                        SHELL_printf("\n");
                    }
                    else
                    {
                        SHELL_printf("Invalid log data length : %d != %d\n", log->body.raw.length, sizeof(CLIENT_LOG));
                    }
                }
                else 
                {
                    SHELL_printf("Invalid log type : %d\n", log->type);
                }
            }

        }    
    }
    else if (argc > 1)
    {
        if (strcasecmp(argv[1], "raw") == 0)
        {
            uint32_t    maxCount = 0;
            uint32_t    count = 0;
            uint32_t    index = 0;

            LOG_getCount(&maxCount);
            
            if (argc == 3)
            {
                if (strToUint32(argv[2], &count))
                {
                    if (count < maxCount)
                    {
                        maxCount = count;
                    }
                    ret = RET_OK;
                }
            }
            else if (argc == 4)
            {
                if (strToUint32(argv[2], &index) && strToUint32(argv[3], &count))
                {
                    if (index + count < maxCount)
                    {
                        maxCount = index + count;
                    }
                    
                    ret = RET_OK;
                }
            }

            if (ret == RET_OK)
            {
                SHELL_printf("[  ][%s] : ", FI_TIME_toString(log->time, "%Y-%m-%d %H:%M:%S"));
                SHELL_LOG_printClientLogTitle();
                SHELL_printf("\n");
                for(; index < maxCount ; index++)
                {
                    if (LOG_getAt(index, log) == RET_OK)
                    {                
                        if (log->body.raw.length == sizeof(CLIENT_LOG))
                        {
                            CLIENT_LOG* clientLog = (CLIENT_LOG*)log->body.raw.data;
                            SHELL_printf("[%2d][%s] : ", index, FI_TIME_toString(log->time, "%Y-%m-%d %H:%M:%S"));
                            SHELL_LOG_printClientLog(clientLog);
                            SHELL_printf("\n");
                        }
                        else
                        {
                            SHELL_printf("Invalid log data length : %d != %d\n", log->body.raw.length, sizeof(CLIENT_LOG));
                        }
                    }
                }
            }
            else 
            {
                for(uint32_t i = 1 ; i < argc ; i++)
                {
                    uint32_t index;
                    
                    if (strToUint32(argv[i], &index))
                    {
                        if (LOG_getAt(index, log))
                        {
                            if (log->type == LOG_TYPE_MESSAGE)
                            {
                                SHELL_printf("%2s : %s\n", FI_TIME_toString(log->time, "%Y-%m-%d %H:%M:%S"), log->body.message);                    
                            }
                            else if (log->type == LOG_TYPE_RAW)
                            {
                                if (log->body.raw.length == sizeof(CLIENT_LOG))
                                {
                                    CLIENT_LOG* clientLog = (CLIENT_LOG*)log->body.raw.data;
                                    SHELL_printf("%s : ", FI_TIME_toString(log->time, "%Y-%m-%d %H:%M:%S"));
                                    SHELL_LOG_printClientLog(clientLog);
                                    SHELL_printf("\n");
                                }
                            }
                        }
                    }
                }    
            }
        }
    }
       
    vPortFree(log);

    return  RET_OK;
}

RET_VALUE SHELL_LOG_count(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    uint32_t    count = 0;
    
    LOG_getCount(&count);
    
    SHELL_printf("%d\n", count);
    
    return  RET_OK;
}

RET_VALUE SHELL_LOG_clear(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    LOG_clear();
    
    return  RET_OK;
}

RET_VALUE SHELL_LOG_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_COMMAND*   subcommand = commandSet_;
    while(subcommand->name != NULL)
    {
        SHELL_printf("%-8s : %s\n", subcommand->name, subcommand->shortHelp, subcommand);
        
        subcommand++;
    }

    return  RET_OK;
}

RET_VALUE   SHELL_LOG_printConfig(LOG_CONFIG* config)
{
    ASSERT(config != NULL);
    
    SHELL_printf("%16s : %s\n", "Status", config->enable?"Enable":"Disable");

    return  RET_OK;
}
RET_VALUE   SHELL_LOG_printClientLogTitle(void)
{
    SHELL_printf("[ %20s ]", "WakeUp");
    SHELL_printf("[ %20s ]", "Data Read");
    SHELL_printf("[ %20s ]", "Modem Init");
    SHELL_printf("[ %20s ]", "Time Sync");
    SHELL_printf("[ %20s ]", "Server Connection");
    SHELL_printf("[ %20s ]", "Data Tranfer");
    SHELL_printf("[ %20s ]", "Server Disconnection");
    SHELL_printf("[ %20s ]", "Next WakeUp");
    
    return  RET_OK;
}
    
RET_VALUE   SHELL_LOG_printClientLog(CLIENT_LOG* clientLog)
{
    ASSERT(clientLog);
    
    if (clientLog->wakeUpTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->wakeUpTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Unknown");
    }
    
    if (clientLog->device.dataTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->device.dataTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Failed");
    }
    
    if (clientLog->modem.initFinishedTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->modem.initFinishedTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Failed");
    }
    
    if (clientLog->timeSync.finishedTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->timeSync.finishedTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Failed");
    }
    
    if (clientLog->server.connectedTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->server.connectedTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Failed");
    }
    
    if (clientLog->dataTransfer.finishedTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->dataTransfer.finishedTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Failed");
    }
    
    if (clientLog->server.disconnectedTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->server.disconnectedTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Failed");
    }
    
    if (clientLog->nextWakeUpTime != 0)
    {
        SHELL_printf("[ %20s ]", FI_TIME_toString(clientLog->nextWakeUpTime, "%Y-%m-%d %H:%M:%S"));
    }
    else
    {
        SHELL_printf("[ %20s ]", "Unknown");
    }
    
    return  RET_OK;
}
    