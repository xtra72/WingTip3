#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "assert.h"
#include "config.h"
#include "me_i10kl.h"
#include "client.h"
#include "shell.h"
#include "utils.h"
#include "shell/shell_device.h"
#include "shell/shell_client.h"

RET_VALUE SHELL_CLIENT_autorun(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_ip(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_port(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_userId(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_topic(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_mode(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_alarm(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_connect(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_pub(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_retryCount(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_Id(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_CLIENT_delay(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);

static SHELL_COMMAND   commandSet_[] = 
{
    {
        .name = "autorun",
        .function = SHELL_CLIENT_autorun,
        .shortHelp = "Auto Run"
    },
    {
        .name = "ip",
        .function = SHELL_CLIENT_ip,
        .shortHelp = "Server IP"
    },
    {
        .name = "port",     
        .function = SHELL_CLIENT_port,         
        .shortHelp = "Server Port"
    },
    {
        .name = "userid",     
        .function = SHELL_CLIENT_userId,         
        .shortHelp = "User ID"
    },
    {
        .name = "topic",     
        .function = SHELL_CLIENT_topic,         
        .shortHelp = "Topic"
    },
    {
        .name = "mode",     
        .function = SHELL_CLIENT_mode,         
        .shortHelp = "Get/Set Data Transfer mode"
    },
    {   
        .name       = "alarm",    
        .function   = SHELL_CLIENT_alarm,     
        .shortHelp  = "Alarm Configuration"
    },
    {
        .name = "connect",     
        .function = SHELL_CLIENT_connect,         
        .shortHelp = "Connect"
    },
    {
        .name = "pub",     
        .function = SHELL_CLIENT_pub,         
        .shortHelp = "MQTT Publish"
    },
    {
        .name = "retry",     
        .function = SHELL_CLIENT_retryCount,         
        .shortHelp = "Retry Count"
    },
    {
        .name = "delay",     
        .function = SHELL_CLIENT_delay,         
        .shortHelp = "Delay"
    },
    {   
        .name = "help",     
        .function = SHELL_CLIENT_help,         
        .shortHelp = "Help"
    },
    {   
        .name = "id",     
        .function = SHELL_CLIENT_Id,         
        .shortHelp = "Id"
    },
    {
        .name = NULL,     
    }
};

RET_VALUE   SHELL_COMMAND_client(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_COMMAND;
    if (argc == 1)
    {
        CLIENT_CONFIG   config;
        
        CLIENT_getConfig(&config);
        SHELL_printf("%16s : %s\n", "Status", CLIENT_getStatusString(CLIENT_getStatus()));
        
        SHELL_CLIENT_printConfig(&config);
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

RET_VALUE SHELL_CLIENT_autorun(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", CLIENT_getAutoRun()?"ON":"OFF");
    }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "help") == 0)
        {
            SHELL_printf("USAGE\n");
            SHELL_printf("  %s [on|off]\n", argv[0]);
            ret = RET_OK;
        }
        else if ((strcasecmp(argv[1], "enable") == 0) ||
            (strcasecmp(argv[1], "on") == 0) ||
            (strcasecmp(argv[1], "1") == 0))
        {
            CLIENT_setAutoRun(true);
            ret = RET_OK;
        }
        else if ((strcasecmp(argv[1], "disable") == 0) ||
                 (strcasecmp(argv[1], "off") == 0) ||
                 (strcasecmp(argv[1], "0") == 0))
        {
            CLIENT_setAutoRun(false);
            ret = RET_OK;
        }
    }

    return ret;
}

RET_VALUE SHELL_CLIENT_connect(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret;
    uint8_t     ip[4];
    uint16_t    port;
    
    if (argc != 3)
    {
        SHELL_printf("Invalid arguments.\n");
        return  RET_INVALID_ARGUMENT;
    }
    
    if (!strToIP(argv[1], ip))
    {
        SHELL_printf("Invalid arguments.\n");
        return  RET_INVALID_ARGUMENT;
    }
    
    port = strtoul(argv[2], 0, 10);

    CLIENT_setServerInfo(ip, port);
    ret = CLIENT_MQTT_connect();
        
    return  ret;
}

RET_VALUE SHELL_CLIENT_ip(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        uint8_t ip[4];
        
        CLIENT_getServerIP(ip);
        
        SHELL_printf("%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    }
    else if (argc == 2)
    {
        uint8_t ip[4];
        if (strToIP(argv[1], ip))                    
        {
            ret = CLIENT_setServerIP(ip);
            if (ret == RET_OK)
            {
                SHELL_printf("Server IP changed to %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
            }
        }
    }
    
    return  ret;
}

RET_VALUE SHELL_CLIENT_port(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%d\n", CLIENT_getServerPort());
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "help") == 0)
        {
            SHELL_printf("USAGE\n");
            SHELL_printf("  %s <PORT>\n", argv[0]);
            SHELL_printf("PARAMETERS\n");
            SHELL_printf("  PORT - 1 ~ 65535\n");
            ret = RET_OK;
        }
        else 
        {
            uint16_t    port;
         
            if (strToUint16(argv[1], &port))
            {
                ret = CLIENT_setServerPort(port);
                if (ret == RET_OK)
                {
                    SHELL_printf("Server port changed to %d\n", CLIENT_getServerPort());
                }
            }
        }
    }
        
    return  ret;
}

RET_VALUE SHELL_CLIENT_userId(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", CLIENT_getUserId());
    }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "help") == 0)
        {
            SHELL_printf("USAGE\n");
            SHELL_printf("  %s <USER_ID>\n", argv[0]);
            SHELL_printf("PARAMETERS\n");
            SHELL_printf("  USER_ID - \"\" for delete User ID\n");
            ret = RET_OK;
        }
        else 
        {
            if (strcmp(argv[1], "\"\"") == 0)
            {
                ret = CLIENT_setUserId("");
                if (ret == RET_OK)
                {
                    SHELL_printf("User ID removed\n");
                }
            }
            else
            {
                ret = CLIENT_setUserId(argv[1]);
                if (ret == RET_OK)
                {
                    SHELL_printf("User ID changed to %s\n", CLIENT_getUserId());
                }
            }
        }
    }
        
    return  ret;
}

RET_VALUE SHELL_CLIENT_topic(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", CLIENT_getTopic());
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "help") == 0)
        {
            SHELL_printf("USAGE\n");
            SHELL_printf("  %s <TOPIC>\n", argv[0]);
            SHELL_printf("PARAMETERS\n");
            SHELL_printf("  TOPIC - MQTT Topic\n");
            ret = RET_OK;
       }
        else
        {
            ret = CLIENT_setTopic(argv[1]);
            if (ret == RET_OK)
            {
                SHELL_printf("Topic changed to %s\n", CLIENT_getTopic());
            }
        }
    }
        
    return  ret;
}

RET_VALUE SHELL_CLIENT_mode(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        switch(CLIENT_getMode())
        {
        case    CLIENT_MODE_IDLE:
            {
                SHELL_printf("%16s : %s\n", "Mode", "IDLE");
            }
            break;
            
        case    CLIENT_MODE_SLEEP_WITH_INTERVAL:
            {
                SHELL_printf("%16s : %s\n", "Mode", "Interval");
                SHELL_printf("%16s : %s\n", "Start Time", FI_TIME_toString(CLIENT_getIntervalStartTime(), "%Y-%m-%d %H:%M:%S"));
                SHELL_printf("%16s : %d\n", "Period", CLIENT_getIntervalPeriod());
            }
            break;
            
        case    CLIENT_MODE_SLEEP_WITH_ALARM:
            {
                char    buffer[32];
                memset(buffer, 0, sizeof(buffer));

                SHELL_printf("%16s : %s\n", "Mode", "Alarm");
                for(int i = 0 ; i < CLIENT_getAlarmCount() ; i++)
                {   
                    uint32_t   alarm;
                    
                    CLIENT_getAlarm(i, &alarm);
                    SHELL_printf("%16d : %02d:%02d:%02d\n", i+1, alarm / SECONDS_OF_HOUR, (alarm / SECONDS_OF_MINUTE) % MINUTES_OF_HOUR , alarm % SECONDS_OF_MINUTE);
                }
            }
            break;
        }
        ret = RET_OK;
    }
    else 
    {
        if (strcasecmp(argv[1], "help") == 0)
        {
            SHELL_printf("USAGE\n");
            SHELL_printf(" %s [alarm|interval] ...\n", argv[0]);
            SHELL_printf(" %s alarm [<TIME> <TIME> ...]\n", argv[0]);
            SHELL_printf(" %s interval <START_TIME> [PERIOD]\n", argv[0]);
            SHELL_printf("PARAMETERS\n");
            SHELL_printf(" TIME - 11:32:00 or 113200\n");
            SHELL_printf(" START_TIME - 20190210110000\n");
            SHELL_printf(" PERIOD - seconds\n");
            ret = RET_OK;
        }
        else if (strcasecmp(argv[1], "alarm") == 0)
        {
            if (argc >= 3)
            {
                bool    invalid = false;
                char*   fields[3];
                uint32_t    alarmCount =0;
                uint32_t    alarmList[24];
                
                for(int i = 2 ; (i < argc) && !invalid ; i++)
                {
                    uint32_t    hour;
                    uint32_t    min;
                    uint32_t    sec;

                    if (strlen(argv[i]) == 6)
                    {
                        sec = strtoul(&argv[i][4], 0, 10);
                        argv[i][4] = NULL;
                        min = strtoul(&argv[i][2], 0, 10);
                        argv[i][2] = NULL;
                        hour = strtoul(&argv[i][0], 0, 10);
                        
                        if ((hour >= 24) || (min >= 60) || (sec >= 60))
                        {
                            invalid = true;
                        }
                        else
                        {
                            alarmList[alarmCount++] = ((hour * 60) + min) * 60 + sec;
                        }
                    }
                    else if ((strlen(argv[i]) == 8) && (seperateString(argv[i], ":", fields, 3) == 3))
                    {
                        hour = strtoul(fields[0], 0, 10);
                        min = strtoul(fields[1], 0, 10);
                        sec = strtoul(fields[2], 0, 10);

                        if ((hour >= 24) || (min >= 60) || (sec >= 60))
                        {
                            invalid = true;
                        }
                    }
                    else 
                    {
                        invalid = true;
                    }
                    
                    if (!invalid)
                    {
                        alarmList[alarmCount++] = ((hour * 60) + min) * 60 + sec;
                    }
                }

                if (!invalid)
                {
                    CLIENT_deleteAllAlarms();

                    for(int i = 0 ; i < alarmCount ; i++)
                    {
                        CLIENT_addAlarm(alarmList[i]);
                    }
                    
                    CLIENT_setMode(CLIENT_MODE_SLEEP_WITH_ALARM);
                    ret = RET_OK;
                }
            }
        }
        else if (strcasecmp(argv[1], "interval") == 0)
        {
            if (argc == 3)
            {
                uint32_t    period;
                
                period = strtoul(argv[2], 0, 10);
                if (period <= 24 * 60 * 60 )
                {
                    ret =  CLIENT_setIntervalPeriod(period);
                    if (ret == RET_OK)
                    {
                        CLIENT_setMode(CLIENT_MODE_SLEEP_WITH_INTERVAL);
                        SHELL_printf("Client interval period changed to %d\n", CLIENT_getIntervalPeriod());
                    }
                    else 
                    {
                        SHELL_printf("Maximum period[%d] exceeded.\n", 24*60*60);
                    }
                }
            }
            else if (argc == 4)
            {
                FI_TIME     startTime;
                uint32_t    period;
                bool        invalid = false;
                
                if (strlen(argv[2]) != 14)
                {
                    invalid = true;
                }
                else 
                {
                    for(int i = 0 ; i < strlen(argv[2]) ; i++)
                    {
                        if (!isdigit(argv[2][i]))
                        {
                            invalid = true;
                            break;
                        }
                    }
                }
                
                if (!invalid)
                {
                    struct tm time;
                    
                    time.tm_sec = strtoul(&argv[2][12], 0, 10);
                    argv[2][12] = 0;
                    time.tm_min = strtoul(&argv[2][10], 0, 10);
                    argv[2][10] = 0;
                    time.tm_hour = strtoul(&argv[2][8], 0, 10);
                    argv[2][8] = 0;
                    time.tm_mday = strtoul(&argv[2][6], 0, 10);
                    argv[2][6] = 0;
                    time.tm_mon = strtoul(&argv[2][4], 0, 10) - 1;
                    argv[2][4] = 0;
                    time.tm_year = strtoul(&argv[2][0], 0, 10) - 1900;
                    
                    if ((time.tm_sec < 0) || (time.tm_sec > 59) ||
                        (time.tm_min < 0) || (time.tm_min > 59) ||
                        (time.tm_hour < 0) || (time.tm_hour > 23) ||
                        (time.tm_mday < 1) || (time.tm_mday > 31) ||
                        (time.tm_mon < 0) || (time.tm_mon > 11) ||
                        (time.tm_year < 0) || (time.tm_year > 200))
                    {
                        invalid = true;
                    }
                    
                    startTime = mktime(&time) - FI_TIME_getTimeZone();

                }
                
                if (!invalid)
                {
                    for(int i = 0 ; i < strlen(argv[3]) ; i++)
                    {
                        if (!isdigit(argv[3][i]))
                        {
                            invalid = true;
                            break;
                        }
                    }
                    
                    period = strtoul(argv[3], 0, 10);
                    if (period > SECONDS_OF_DAY)
                    {
                        SHELL_printf("Maximum period[%d] exceeded.\n", 24*60*60);
                        invalid = true;
                    }
                }
                
                if (!invalid)
                {
                    CLIENT_setIntervalStartTime(startTime);
                    CLIENT_setIntervalPeriod(period);
                    CLIENT_setMode(CLIENT_MODE_SLEEP_WITH_INTERVAL);
                    SHELL_printf("Client interval period changed to %d\n", CLIENT_getIntervalPeriod());
                    ret = RET_OK;
                }
            }
        }
    }
    
    return  ret;
}


RET_VALUE SHELL_CLIENT_alarm(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    uint32_t    time;
    
    if (argc == 1)
    {
        uint32_t    i;
        
        FI_TIME_get(&time);
        SHELL_printf("%16s : %s\n", "Current Time", FI_TIME_toString(time, "%Y-%m-%d %H:%M:%S"));
        SHELL_printf("%16s : %d\n", "Alarm List", CLIENT_getAlarmCount());
        for(i = 0 ; i < CLIENT_getAlarmCount() ; i++)
        {
            uint32_t    alarm;
            
            if (CLIENT_getAlarm(i, &alarm) == RET_OK)
            {
                SHELL_printf("%16d : %02d:%02d:%02d\n", i+1, alarm / SECONDS_OF_HOUR, alarm / SECONDS_OF_MINUTE % MINUTES_OF_HOUR,  alarm % SECONDS_OF_MINUTE);
            }
            else 
            {
                SHELL_printf("%16d : Error!\n", i+1);
            }            
        }        
    }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "next") == 0)
        {
            RET_VALUE   ret;
            FI_TIME     currentTime;
            FI_CLOCK    nextAlarm;
            
            FI_TIME_get(&currentTime);
            SHELL_printf("%16s : %s(%d)\n", "Current Time", FI_TIME_toString(currentTime, "%Y-%m-%d %H:%M:%S"), currentTime);
            ret = CLIENT_getNextAlarm(currentTime, &nextAlarm);
            if (ret == RET_OK)
            {
                SHELL_printf("%16s : %02d:%02d:%02d\n", "Next Alarm Time", nextAlarm / SECONDS_OF_HOUR, (nextAlarm / SECONDS_OF_MINUTE) % MINUTES_OF_HOUR, nextAlarm % SECONDS_OF_MINUTE);
            }

        }
        else if (strcasecmp(argv[1], "test") == 0)
        {
            RET_VALUE   ret;
            FI_TIME     currentTime;
            FI_CLOCK    nextAlarm;
            
            FI_TIME_get(&currentTime);
            SHELL_printf("%16s : %s\n", "Current Time", FI_TIME_toString(currentTime, "%Y-%m-%d %H:%M:%S"));
            
            ret = CLIENT_getNextAlarm(currentTime, &nextAlarm);
            if (ret == RET_OK)
            {
                SHELL_printf("%16s : %02d:%02d:%02d\n", "Next Alarm Time", nextAlarm / SECONDS_OF_HOUR, (nextAlarm / SECONDS_OF_MINUTE) % MINUTES_OF_HOUR, nextAlarm % SECONDS_OF_MINUTE);
                if (FI_TIME_setAlarm(nextAlarm) == RET_OK)
                {
                    SHELL_printf("Alarm setting succeeded.\n");
                }
                else 
                {
                    SHELL_printf("Alarm setting failed.\n");
                }    
            }
        }
    }
    else if (argc == 3)
    {
        if (strcasecmp(argv[1], "add") == 0)
        {
            char*   fields[3];
            
            if (seperateString(argv[2], ":", fields, 3) == 3)
            {
                uint32_t    hour = strtoul(fields[0], 0, 10);
                uint32_t    min = strtoul(fields[1], 0, 10);
                uint32_t    sec = strtoul(fields[2], 0, 10);
                
                if ((hour < 24) && (min < 60) && (sec < 60))
                {
                    uint32_t    alarm = ((hour * 60) + min) * 60 + sec;
                    
                    if (CLIENT_addAlarm(alarm) == RET_OK)
                    {
                        SHELL_printf("New alarm added successfully.\n");
                    }
                    else 
                    {
                        SHELL_printf("Failed to add new alarm.\n");
                    }
                }
                else 
                {
                    SHELL_printf("Invalid alarm! : %s\n", argv[2]);
                }
            }
            else 
            {
                SHELL_printf("Invalid alarm! : %s\n", argv[2]);
            }

        }
        else if (strcasecmp(argv[1], "del") == 0)
        {
            uint32_t    index = strtoul(argv[2], 0, 10) - 1;
            
            if (index < CLIENT_getAlarmCount())
            {
                CLIENT_delAlarm(index);
            }
            else 
            {
                SHELL_printf("Invalid alarm index! [ < %d ]\n", CLIENT_getAlarmCount());
            }
        }
    }

    return  RET_OK;
}


RET_VALUE SHELL_CLIENT_pub(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret;
    
    if (argc != 3)
    {
        SHELL_printf("Invalid arguments.\n");
        return  RET_INVALID_ARGUMENT;
    }
    
    ret = CLIENT_MQTT_pub(argv[1], argv[2]);
        
    return  ret;
}

RET_VALUE SHELL_CLIENT_retryCount(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%d\n", CLIENT_getRetryCount());
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint32_t    count;
        
        if (strToUint32(argv[1], &count))
        {
            ret = CLIENT_setRetryCount(count);
            SHELL_printf("%d\n", CLIENT_getRetryCount());
        }
    }
    
    return  ret;
}

RET_VALUE SHELL_CLIENT_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_COMMAND*   subcommand = commandSet_;
    if (argc == 1)
    {
        while(subcommand->name != NULL)
        {
            SHELL_printf("%-8s : %s\n", subcommand->name, subcommand->shortHelp, subcommand);
            
            subcommand++;
        }
    }
    else
    {
    }
    return  RET_OK;
}

RET_VALUE   SHELL_CLIENT_printConfig(CLIENT_CONFIG* config)
{
    SHELL_printf("%16s : %s\n", "Auto Run", (config->autoRun)?"ON":"OFF");
    SHELL_printf("%16s : %d\n", "Retry Count", config->maxRetryCount);
    SHELL_printf("%16s : %d.%d.%d.%d\n", "Server IP", config->server.ip[0], config->server.ip[1], config->server.ip[2], config->server.ip[3]);
    SHELL_printf("%16s : %d\n", "Server Port", config->server.port);
    SHELL_printf("%16s : %s\n", "Client ID", config->id);
    SHELL_printf("%16s : %s\n", "User ID", config->server.userId);
    SHELL_printf("%16s : %s\n", "Topic", config->server.topic);

    switch(config->opMode.mode)
    {
    case    CLIENT_MODE_IDLE:
        {
            SHELL_printf("%16s : %s\n", "Mode", "IDLE");
        }
        break;
        
    case    CLIENT_MODE_SLEEP_WITH_INTERVAL:
        {
            SHELL_printf("%16s : %s\n", "Mode", "Interval");
            SHELL_printf("%16s : %s\n", "Start Time", FI_TIME_toString(config->opMode.interval.start, "%Y-%m-%d %H:%M:%S"));
            SHELL_printf("%16s : %d\n", "Period", config->opMode.interval.period);
        }
        break;
        
    case    CLIENT_MODE_SLEEP_WITH_ALARM:
        {
            SHELL_printf("%16s : %s\n", "Mode", "Alarm");
            for(int i = 0 ; i < config->opMode.alarm.count ; i++)
            {
                SHELL_printf("%16d : %02d:%02d:%02d\n", i+1, config->opMode.alarm.times[i] / SECONDS_OF_HOUR, config->opMode.alarm.times[i] / SECONDS_OF_MINUTE % MINUTES_OF_HOUR, config->opMode.alarm.times[i] % SECONDS_OF_MINUTE);
            }
        }
        break;
    }

    SHELL_printf("%16s : %s\n", "Delay", config->delay.enable?"Enable":"Disable");
    if (config->delay.enable)
    {
        SHELL_printf("%16s : %d\n", "Base", config->delay.base);
        if (config->delay.mode == CLIENT_DELAY_MODE_TRANSTER_DELAY)
        {
            SHELL_printf("%16s : %s\n", "Mode", "Transfer Delay");
        }
        else 
        {
            SHELL_printf("%16s : %s\n", "Mode", "WakeUp Delay");
        }
        SHELL_printf("%16s : %d\n", "Period", config->delay.period);
        SHELL_printf("%16s : %d\n", "Member", config->delay.member);
        SHELL_printf("%16s : %d\n", "Offset", config->delay.offset);
    }
    
    return  RET_OK;
}

RET_VALUE SHELL_CLIENT_Id(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", CLIENT_getId());
    }
    else if (argc == 2)
    {
        if (strcasecmp(argv[1], "help") == 0)
        {
            SHELL_printf("USAGE\n");
            SHELL_printf("  %s <ID>\n", argv[0]);
            SHELL_printf("PARAMETERS\n");
            SHELL_printf("  ID - \"\" for delete ID\n");
            ret = RET_OK;
        }
        else 
        {
            if (strcmp(argv[1], "\"\"") == 0)
            {
                ret = CLIENT_setId("");
                if (ret == RET_OK)
                {
                    SHELL_printf("ID removed\n");
                }
            }
            else
            {
                ret = CLIENT_setId(argv[1]);
                if (ret == RET_OK)
                {
                    SHELL_printf("ID changed to %s\n", CLIENT_getId());
                }
            }
        }
    }
        
    return  ret;
}


RET_VALUE SHELL_CLIENT_delay(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    CLIENT_CONFIG   config;
    
    if (argc == 1)
    {
        CLIENT_getConfig(&config);

        SHELL_printf("%16s : %s\n", "Delay", config.delay.enable?"Enable":"Disable");
        if (config.delay.enable)
        {
            SHELL_printf("%16s : %d\n", "Base", config.delay.base);
            if (config.delay.mode == CLIENT_DELAY_MODE_TRANSTER_DELAY)
            {
                SHELL_printf("%16s : %s\n", "Mode", "Transfer Delay");
            }
            else 
            {
                SHELL_printf("%16s : %s\n", "Mode", "WakeUp Delay");
            }
            SHELL_printf("%16s : %d\n", "Period", config.delay.period);
            SHELL_printf("%16s : %d\n", "Member", config.delay.member);
            SHELL_printf("%16s : %d\n", "Offset", config.delay.offset);
        }
    }
    else if (argc == 3)
    {
        if (strcasecmp(argv[1], "base") == 0)
        {
            uint32_t    value;
            if (strToUint32(argv[2], &value))
            {
                ret = CLIENT_setDelayBase(value);
            }
        }
        else if (strcasecmp(argv[1], "mode") == 0)
        {
            uint32_t    value;
            if (strToUint32(argv[2], &value))
            {
                ret = CLIENT_setDelayMode((CLIENT_DELAY_MODE)value);
            }
        }
        else if (strcasecmp(argv[1], "period") == 0)
        {
            uint32_t    value;
            if (strToUint32(argv[2], &value))
            {
                ret = CLIENT_setDelayPeriod(value);
            }
        }
        else if (strcasecmp(argv[1], "member") == 0)
        {
            uint32_t    value;
            if (strToUint32(argv[2], &value))
            {
                ret = CLIENT_setDelayMember(value);
            }
        }
        else if (strcasecmp(argv[1], "offset") == 0)
        {
            uint32_t    value;
            if (strToUint32(argv[2], &value))
            {
                ret = CLIENT_setDelayOffset(value);
            }
        }
        
    }
        
    return  ret;
}
