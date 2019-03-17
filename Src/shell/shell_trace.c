#include <string.h>
#include <stdlib.h>
#include "assert.h"
#include "config.h"
#include "me_i10kl.h"
#include "shell.h"
#include "utils.h"

RET_VALUE SHELL_TRACE_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_TRACE_enable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_TRACE_disable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);

static const SHELL_COMMAND   commandSet_[] = 
{
    {   
        .name = "help",     
        .function = SHELL_TRACE_help,         
        .shortHelp = "Help",        
        .fullHelp = "Help"
    },
    {   
        .name = "enable",
        .function = SHELL_TRACE_enable,
        .shortHelp = "Enable",
        .fullHelp = "Enable"
    },
    {   
        .name = "disable",
        .function = SHELL_TRACE_disable,
        .shortHelp = "Disable",
        .fullHelp = "Disable"
    },
    {   
        .name = NULL 
    }
};

RET_VALUE   SHELL_COMMAND_trace(char *argv[], uint32_t argc, struct _SHELL_COMMAND  const* command)
{
    RET_VALUE   ret = RET_INVALID_COMMAND;
    if (argc == 1)
    {
        char*   argc[] = {"help"};

        ret = SHELL_TRACE_help(argc, 1, &commandSet_[0]);
    }
    else 
    {
        SHELL_COMMAND const*   subcommand = commandSet_;
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

RET_VALUE SHELL_TRACE_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_COMMAND const*   subcommand = commandSet_;
    while(subcommand->name != NULL)
    {
        SHELL_printf("%-8s : %s\n", subcommand->name, subcommand->shortHelp, subcommand);
        
        subcommand++;
    }

    return  RET_OK;
}
RET_VALUE SHELL_TRACE_enable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        TRACE_setEnable(true);
    }   
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
    
    return  ret;
}

RET_VALUE SHELL_TRACE_disable(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        TRACE_setEnable(false);
    }
    
    if (ret != RET_OK)
    {
        SHELL_printf("Error : %d\n", ret);
    }
    
    return  ret;
}


