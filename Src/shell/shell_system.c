#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assert.h"
#include "config.h"
#include "system.h"
#include "client.h"
#include "shell.h"
#include "utils.h"
#include "shell/shell_system.h"


RET_VALUE SHELL_SYSTEM_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);

static SHELL_COMMAND   commandSet_[] = 
{
    {   
        .name = "help",     
        .function = SHELL_SYSTEM_help,         
        .shortHelp = "Help"
    },
    {
        .name = NULL,     
    }
};

RET_VALUE   SHELL_COMMAND_system(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_COMMAND;
    if (argc == 1)
    {
        SYSTEM_CONFIG   config;
        
        SYSTEM_getConfig(&config);
        SHELL_SYSTEM_printConfig(&config);
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

RET_VALUE SHELL_SYSTEM_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_COMMAND*   subcommand = commandSet_;
    while(subcommand->name != NULL)
    {
        SHELL_printf("%-8s : %s\n", subcommand->name, subcommand->shortHelp, subcommand);
        
        subcommand++;
    }

    return  RET_OK;
}

RET_VALUE   SHELL_SYSTEM_printConfig(SYSTEM_CONFIG* config)
{   
    ASSERT(config != NULL);

    SHELL_printf("%16s : %s\n", "Activation", SYSTEM_getActivation()?"Enable":"Disable");
    if (SYSTEM_getActivation())
    {
        FI_TIME time;
        SYSTEM_getActivationTime(&time);
        SHELL_printf("%16s : %s\n", "Activation Time", FI_TIME_toString(time, "%Y-%m-%d %H:%M:%S"));
    }
    
    SHELL_printf("%16s : %s\n", "Watch Dog", config->wdog.enable?"Enable":"Disable");

    return  RET_OK;
}