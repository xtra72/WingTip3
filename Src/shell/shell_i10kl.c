#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assert.h"
#include "config.h"
#include "me_i10kl.h"
#include "client.h"
#include "shell.h"
#include "utils.h"
#include "shell/shell_i10kl.h"


RET_VALUE SHELL_I10KL_status(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_I10KL_wakeUp(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_I10KL_autoSleep(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_I10KL_timeout(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_I10KL_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);

static SHELL_COMMAND   commandSet_[] = 
{
    {   
        .name = "autosleep",    
        .function = SHELL_I10KL_autoSleep,     
        .shortHelp = "I10KL Auto Sleep"
    },
    {   
        .name = "status",    
        .function = SHELL_I10KL_status,     
        .shortHelp = "I10KL Status"
    },
    {   
        .name = "wakeup",    
        .function = SHELL_I10KL_wakeUp,     
        .shortHelp = "I10KL WakeUp"
    },
    {   
        .name = "timeout",    
        .function = SHELL_I10KL_timeout,     
        .shortHelp = "I10KL Timeout"
    },
    {   
        .name = "help",     
        .function = SHELL_I10KL_help,         
        .shortHelp = "Help"
    },
    {
        .name = NULL,     
    }
};

RET_VALUE   SHELL_COMMAND_i10kl(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_COMMAND;
    if (argc == 1)
    {
        ME_I10KL_CONFIG   config;
        
        ME_I10KL_getConfig(&config);
        SHELL_printf("%16s : %s\n", "Power Status", ME_I10KL_getPower()?"ON":"OFF");
        SHELL_I10KL_printConfig(&config);
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


RET_VALUE SHELL_I10KL_status(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        bool    status;
        
        ret = ME_I10KL_getWakeUp(&status);
        if (ret == RET_OK)
        {
            SHELL_printf("PSM_WAKEUP : %s\n", (status?"HI":"LO"));
        }

        ret = ME_I10KL_getState0(&status);
        if (ret == RET_OK)
        {
            SHELL_printf("SYS_STATE : %s\n", (status?"HI":"LO"));
        }

        ret = ME_I10KL_getPSM(&status);
        if (ret == RET_OK)
        {
            SHELL_printf("PSM_MON : %s\n", (status?"HI":"LO"));
        }

        ret = ME_I10KL_getAutoSleep(&status);
        if (ret == RET_OK)
        {
            SHELL_printf("SYS_WAKE : %s\n", (status?"LO":"HI"));
        }
    }
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
        
    return  ret;
}


RET_VALUE SHELL_I10KL_wakeUp(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        ret = ME_I10KL_setWakeUp(true);
        vTaskDelay(500);
        ret = ME_I10KL_setWakeUp(false);
    }
    
        if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
        
    return  ret;
}


RET_VALUE SHELL_I10KL_autoSleep(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        bool    autoSleep;
        
        ret = ME_I10KL_getAutoSleep(&autoSleep);
        if (ret == RET_OK)
        {
            SHELL_printf("Auto Sleep : %s\n", (autoSleep)?"ON":"OFF");
        }
    }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "on") == 0)
        {
            ret = ME_I10KL_setAutoSleep(true);
        }
        else if (strcasecmp(argv[1], "off") == 0)
        {
            ret = ME_I10KL_setAutoSleep(false);
        }
    }
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
        
    return  ret;
}


RET_VALUE SHELL_I10KL_power(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", (ME_I10KL_getPower())?"ON":"OFF");
        ret = RET_OK;
    }
    else if (argc == 2)
    {
         if (strcasecmp(argv[1], "on") == 0)
        {
            ret = ME_I10KL_setPower(true);
        }
        else if (strcasecmp(argv[1], "off") == 0)
        {
            ret = ME_I10KL_setPower(false);
        }
    }
    
    return  ret;
}

RET_VALUE SHELL_I10KL_timeout(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        uint32_t time;
        
        ME_I10KL_getTimeout(&time);

        SHELL_printf("%d ms\n", time);
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint32_t    previous_timeout, timeout;
        
        if (strToUint32(argv[1], &timeout))
        {
            ME_I10KL_getTimeout(&previous_timeout);
            
            ret = ME_I10KL_setTimeout(timeout);
            if (ret == RET_OK)
            {
                SHELL_printf("Timeout changed from %d ms to %d ms\n", previous_timeout, timeout);            
            }
            else
            {
                SHELL_printf("Failed to change timeout.\n");            
            }
        }
        else
        {
            SHELL_printf("Invalid timeout : %s\n", argv[1]);
        }
    }
    
    return  ret;
}
RET_VALUE SHELL_I10KL_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_COMMAND*   subcommand = commandSet_;
    while(subcommand->name != NULL)
    {
        SHELL_printf("%-8s : %s\n", subcommand->name, subcommand->shortHelp, subcommand);
        
        subcommand++;
    }

    return  RET_OK;
}

RET_VALUE   SHELL_I10KL_printConfig(ME_I10KL_CONFIG* config)
{
    ASSERT(config != NULL);
    
    SHELL_printf("%16s : %d ms\n", "Timeout", config->timeout);

    return  RET_OK;
}