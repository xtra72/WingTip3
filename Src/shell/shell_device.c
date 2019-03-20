#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "assert.h"
#include "config.h"
#include "me_i10kl.h"
#include "client.h"
#include "shell.h"
#include "utils.h"


RET_VALUE SHELL_DEVICE_read(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_id(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_type(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_baudrate(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_parity(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_databit(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_stopbit(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_swap(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE SHELL_DEVICE_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);

RET_VALUE SHELL_DEVICE_printConfig(DEVICE_CONFIG* config);

static SHELL_COMMAND   commandSet_[] = 
{
    {
        .name = "read",     
        .function = SHELL_DEVICE_read,         
        .shortHelp = "Read data from device"
    },
    {
        .name = "id",     
        .function = SHELL_DEVICE_id,         
        .shortHelp = "Get/Set Device ID"
    },
    {
        .name = "type",     
        .function = SHELL_DEVICE_type,         
        .shortHelp = "Get/Set Device Type"
    },

    {
        .name = "baudrate",     
        .function = SHELL_DEVICE_baudrate,         
        .shortHelp = "Baudrate"
    },
#if 1
    {
        .name = "parity",     
        .function = SHELL_DEVICE_parity,         
        .shortHelp = "parity"
    },
    {
        .name = "databit",     
        .function = SHELL_DEVICE_databit,         
        .shortHelp = "Data Bit"
    },
    {
        .name = "stopbit",     
        .function = SHELL_DEVICE_stopbit,         
        .shortHelp = "Stop Bit"
    },
#endif
    {   
        .name = "swap",     
        .function = SHELL_DEVICE_swap,         
        .shortHelp = "Swap"
    },
    {   
        .name = "help",     
        .function = SHELL_DEVICE_help,         
        .shortHelp = "Help"
    },
    {
        .name = NULL,     
    }
};

RET_VALUE   SHELL_COMMAND_device(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_COMMAND;
    if (argc == 1)
    {
        DEVICE_CONFIG   config;
        
        ret = DEVICE_getConfig(&config);
        if (ret == RET_OK)
        {
            SHELL_DEVICE_printConfig(&config);
        }
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

RET_VALUE SHELL_DEVICE_read(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        DEVICE_DATA data;
        
        ret = DEVICE_readData(&data, 1000);
        if (ret == RET_OK)
        {
            SHELL_printf("Voltage\n");
            SHELL_printf("%4s : %s%d.%02d\n", "RS", (data.voltage.RS < 0)?"-":"", abs(data.voltage.RS / 100), abs(data.voltage.RS % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "ST", (data.voltage.ST < 0)?"-":"", abs(data.voltage.ST / 100), abs(data.voltage.ST % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "TR", (data.voltage.TR < 0)?"-":"", abs(data.voltage.TR / 100), abs(data.voltage.TR % 100));

            SHELL_printf("Current\n");
            SHELL_printf("%4s : %s%d.%02d\n", "R", (data.current.R < 0)?"-":"", abs(data.current.R / 100), abs(data.current.R % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "S", (data.current.S < 0)?"-":"", abs(data.current.S / 100), abs(data.current.S % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "T", (data.current.T < 0)?"-":"", abs(data.current.T / 100), abs(data.current.T % 100));

            SHELL_printf("%12s : %s%d.%02d\n", "Total Power", (data.totalPower < 0)?"-":"", abs(data.totalPower / 100), abs(data.totalPower % 100));
            //SHELL_printf("%12s : %d\n", "Total Energy", data.totalEnergy );
            SHELL_printf("%12s : %d.%02d\n", "Total Energy", abs(data.totalEnergy / 100), abs(data.totalEnergy % 100));
            /*SHELL_printf("Voltage\n");
            SHELL_printf("%4s : %3d.%02d\n", "RS", data.voltage.RS / 100, data.voltage.RS % 100);
            SHELL_printf("%4s : %3d.%02d\n", "ST", data.voltage.ST / 100, data.voltage.ST % 100);
            SHELL_printf("%4s : %3d.%02d\n", "TR", data.voltage.TR / 100, data.voltage.TR % 100);

            SHELL_printf("Current\n");
            SHELL_printf("%4s : %3d.%02d\n", "R", data.current.R / 100, data.current.R % 100);
            SHELL_printf("%4s : %3d.%02d\n", "S", data.current.S / 100, data.current.S % 100);
            SHELL_printf("%4s : %3d.%02d\n", "T", data.current.T / 100, data.current.T % 100);

            SHELL_printf("%12s : %3d.%02d\n", "Total Power", data.totalPower / 100, data.totalPower % 100 );
            SHELL_printf("%12s : %3d\n", "Total Energy", data.totalEnergy );*/
        }
    }    
    else if (argc == 2)
    {
        DEVICE_DATA data;
        uint32_t    timeout;
        
        timeout = strtoul(argv[1], 0, 10);
        
        ret = DEVICE_readData(&data, timeout);
        if (ret == RET_OK)
        {
            SHELL_printf("Voltage\n");
            SHELL_printf("%4s : %s%d.%02d\n", "RS", (data.voltage.RS < 0)?"-":"", abs(data.voltage.RS / 100), abs(data.voltage.RS % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "ST", (data.voltage.ST < 0)?"-":"", abs(data.voltage.ST / 100), abs(data.voltage.ST % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "TR", (data.voltage.TR < 0)?"-":"", abs(data.voltage.TR / 100), abs(data.voltage.TR % 100));

            SHELL_printf("Current\n");
            SHELL_printf("%4s : %s%d.%02d\n", "R", (data.current.R < 0)?"-":"", abs(data.current.R / 100), abs(data.current.R % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "S", (data.current.S < 0)?"-":"", abs(data.current.S / 100), abs(data.current.S % 100));
            SHELL_printf("%4s : %s%d.%02d\n", "T", (data.current.T < 0)?"-":"", abs(data.current.T / 100), abs(data.current.T % 100));

            SHELL_printf("%12s : %s%d.%02d\n", "Total Power", (data.totalPower < 0)?"-":"", abs(data.totalPower / 100), abs(data.totalPower % 100));
            //SHELL_printf("%12s : %d\n", "Total Energy", data.totalEnergy );
            SHELL_printf("%12s : %d.%02d\n", "Total Energy", abs(data.totalEnergy / 100), abs(data.totalEnergy % 100));
            /*
            SHELL_printf("Voltage\n");
            SHELL_printf("%4s : %3d.%02d\n", "RS", data.voltage.RS / 100, data.voltage.RS % 100);
            SHELL_printf("%4s : %3d.%02d\n", "ST", data.voltage.ST / 100, data.voltage.ST % 100);
            SHELL_printf("%4s : %3d.%02d\n", "TR", data.voltage.TR / 100, data.voltage.TR % 100);

            SHELL_printf("Current\n");
            SHELL_printf("%4s : %3d.%02d\n", "R", data.current.R / 100, data.current.R % 100);
            SHELL_printf("%4s : %3d.%02d\n", "S", data.current.S / 100, data.current.S % 100);
            SHELL_printf("%4s : %3d.%02d\n", "T", data.current.T / 100, data.current.T % 100);

            SHELL_printf("%12s : %3d.%02d\n", "Total Power", data.totalPower / 100, data.totalPower % 100 );
            SHELL_printf("%12s : %3d\n", "Total Energy", data.totalEnergy);
            */
        }
    }
    
    return  ret;
}


RET_VALUE SHELL_DEVICE_id(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;

    if (argc == 1)
    {
        uint32_t id;
        
        DEVICE_getID(&id);
        
        SHELL_printf("%d\n", id);
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint32_t id= (uint32_t)strtoul(argv[1], 0, 10);
        ret = DEVICE_setID(id);
        if (ret == RET_OK)
        {
            SHELL_printf("Device ID changed to %d\n", id);
        }
    }
    
    return  ret;
}

RET_VALUE SHELL_DEVICE_type(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;

    if (argc == 1)
    {
        DEVICE_TYPE type;
        
        ret = DEVICE_getType(&type);
        if (ret == RET_OK)
        {
            SHELL_printf("%s\n", DEVICE_getTypeName(type));
        }
        else
        {
            SHELL_printf("Failed to get Device ID\n");
        }
    }
    else if (argc == 2)
    {
        if ((strlen(argv[1]) == 1) && isdigit(argv[1][0]))
        {
            DEVICE_TYPE type = (DEVICE_TYPE)strtoul(argv[1], 0, 10);
            ret = DEVICE_setType(type);
            if (ret == RET_OK)
            {
                SHELL_printf("Device type changed.\n");
            }
        }
    }
    
    return  ret;
}


RET_VALUE SHELL_DEVICE_help(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    SHELL_COMMAND*   subcommand = commandSet_;
    while(subcommand->name != NULL)
    {
        SHELL_printf("%-8s : %s\n", subcommand->name, subcommand->shortHelp, subcommand);
        
        subcommand++;
    }

    return  RET_OK;
}

RET_VALUE SHELL_DEVICE_baudrate(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;

    if (argc == 1)
    {
        SERIAL_BAUDRATE baudrate;
        
        DEVICE_getBaudrate(&baudrate);
        
        SHELL_printf("%s\n", SERIAL_baudrateName(baudrate));
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint32_t baudrate= (uint32_t)strtoul(argv[1], 0, 10);
        SERIAL_BAUDRATE baudrate_set;

        switch(baudrate)
        {
        case 50 :       baudrate_set = SERIAL_BAUDRATE_50;      break;
        case 75 :       baudrate_set = SERIAL_BAUDRATE_75;      break;
        case 110 :      baudrate_set = SERIAL_BAUDRATE_110;		break;
        case 134 :      baudrate_set = SERIAL_BAUDRATE_134;		break;
        case 150 :      baudrate_set = SERIAL_BAUDRATE_150;     break;
        case 200 :      baudrate_set = SERIAL_BAUDRATE_200;     break;
        case 300 :      baudrate_set = SERIAL_BAUDRATE_300;		break;
        case 600 :      baudrate_set = SERIAL_BAUDRATE_600;		break;
        case 1200 :     baudrate_set = SERIAL_BAUDRATE_1200;	break;
        case 1800 :     baudrate_set = SERIAL_BAUDRATE_1800;	break;
        case 2400 :     baudrate_set = SERIAL_BAUDRATE_2400;    break;
        case 4800 :     baudrate_set = SERIAL_BAUDRATE_4800;    break;
        case 9600 :     baudrate_set = SERIAL_BAUDRATE_9600;	break;
        case 19200 :    baudrate_set = SERIAL_BAUDRATE_19200;	break;
        case 38400 :    baudrate_set = SERIAL_BAUDRATE_38400;	break;
        case 57600 :    baudrate_set = SERIAL_BAUDRATE_57600;   break;
        case 115200 :   baudrate_set = SERIAL_BAUDRATE_115200;  break;
        default :       return  RET_ERROR;                       break;
        }
        ret = DEVICE_setBaudrate(baudrate_set);
        if (ret == RET_OK)
        {
            SHELL_printf("Device Baudrate changed to %d\n", baudrate);
        }
    }
    
    return  ret;
    
}

RET_VALUE SHELL_DEVICE_parity(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;

    if (argc == 1)
    {
        SERIAL_PARITY parity;
        
        DEVICE_getParity(&parity);
        
        SHELL_printf("%s\n", SERIAL_parityName(parity));
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint8_t parity= (uint32_t)strtoul(argv[1], 0, 10);
        SERIAL_PARITY parity_set;

        switch(parity)
        {
        case 0 :        parity_set = SERIAL_PARITY_NONE;  break;
        case 1 :        parity_set = SERIAL_PARITY_ODD;   break;
        case 2 :        parity_set = SERIAL_PARITY_EVEN;	break;
        default :       return RET_ERROR;
        }
        ret = DEVICE_setParity(parity_set);
        if (ret == RET_OK)
        {
            SHELL_printf("Device parity_set changed to %s\n", SERIAL_parityName(parity_set));
        }
    }
    
    return  ret;
    
}
RET_VALUE SHELL_DEVICE_databit(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;

    if (argc == 1)
    {
        SERIAL_DATA_BITS databit;
        
        DEVICE_getDataBits(&databit);
        
        SHELL_printf("%d\n", SERIAL_dataBitsName(databit));
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint8_t databit = (uint32_t)strtoul(argv[1], 0, 10);
        SERIAL_DATA_BITS databit_set;

        switch(databit)
        {
        case 7 :        databit_set = SERIAL_DATA_BITS_7;  break;
        case 8 :        databit_set = SERIAL_DATA_BITS_8;   break;
        case 9 :        databit_set = SERIAL_DATA_BITS_9;	break;
        default :       return RET_ERROR;
        }
        ret = DEVICE_setDataBits(databit_set);
        if (ret == RET_OK)
        {
            SHELL_printf("Device databit changed to %s\n", SERIAL_dataBitsName(databit_set));
        }
    }
    
    return  ret;
}
RET_VALUE SHELL_DEVICE_stopbit(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;

    if (argc == 1)
    {
        SERIAL_STOP_BITS stopbit;
        
        DEVICE_getStopBits(&stopbit);
        
        SHELL_printf("%s\n", SERIAL_stopBitsName(stopbit));
        ret = RET_OK;
    }
    else if (argc == 2)
    {
        uint8_t stopbit= (uint32_t)strtoul(argv[1], 0, 10);
        SERIAL_STOP_BITS stopbit_set;

        switch(stopbit)
        {
        case 1 :        stopbit_set = SERIAL_STOP_BITS_1;   break;
        case 2 :        stopbit_set = SERIAL_STOP_BITS_2;	break;
        default :       return RET_ERROR;
        }
        ret = DEVICE_setStopBits(stopbit_set);
        if (ret == RET_OK)
        {
            SHELL_printf("Device stopbit changed to %s\n", SERIAL_stopBitsName(stopbit_set));
        }
    }
    
    return  ret;
}

RET_VALUE SHELL_DEVICE_swap(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command)
{
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    
    if (argc == 1)
    {
        SHELL_printf("%s\n", DEVICE_getSwap()?"ON":"OFF");
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
            DEVICE_setSwap(true);
            ret = RET_OK;
        }
        else if ((strcasecmp(argv[1], "disable") == 0) ||
                 (strcasecmp(argv[1], "off") == 0) ||
                 (strcasecmp(argv[1], "0") == 0))
        {
            DEVICE_setSwap(false);
            ret = RET_OK;
        }
    }

    return ret;
}

RET_VALUE   SHELL_DEVICE_printConfig(DEVICE_CONFIG* config)
{
    ASSERT(config != NULL);
    
    SHELL_printf("%16s : %d\n", "ID", config->id);
    SHELL_printf("%16s : %s\n", "SWAP", DEVICE_getSwap()?"ON":"OFF");
    SHELL_printf("%16s : %s(%d)\n", "Type", DEVICE_getTypeName(config->type), config->type);
    SHELL_printf("%16s : %s\n", "Baudrate", SERIAL_baudrateName(config->serial.baudrate));
    SHELL_printf("%16s : %s\n", "Parity", SERIAL_parityName(config->serial.parity));
    SHELL_printf("%16s : %s\n", "Data bit", SERIAL_dataBitsName(config->serial.dataBits));
    SHELL_printf("%16s : %s\n", "Stop bit", SERIAL_stopBitsName(config->serial.stopBits));
    SHELL_printf("%16s : %d ms\n", "Timeout", DEVICE_getTimeout());
    
    return  RET_OK;
}