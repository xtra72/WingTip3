#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "target.h"

typedef struct
{
    bool            activation;
    FI_TIME         activationTime;
    
    struct  
    {
        bool        enable;
    }   wdog;
    
}   SYSTEM_CONFIG;

RET_VALUE   SYSTEM_init(SYSTEM_CONFIG* config);
RET_VALUE   SYSTEM_getConfig(SYSTEM_CONFIG* config);

bool        SYSTEM_getActivation(void);
RET_VALUE   SYSTEM_setActivation(bool activation);
RET_VALUE   SYSTEM_getActivationTime(FI_TIME* time);

RET_VALUE   SYSTEM_WDOG_setEnable(bool enable);
RET_VALUE   SYSTEM_WDOG_getEnable(bool* enable);

RET_VALUE   SYSTEM_WDOG_setTimeout(uint32_t timeout);
RET_VALUE   SYSTEM_WDOG_getTimeout(uint32_t* timeout);

uint32_t    SYS_getBatteryLevel(void);
void        SYS_reset(void);
bool	    SYS_getExtPower18(void);
RET_VALUE	SYS_setExtPower18(bool on);
bool	    SYS_getExtPower33(void);
RET_VALUE	SYS_setExtPower33(bool on);
RET_VALUE	SYS_setExtPower(bool on);

RET_VALUE   SYSTEM_globalConfigSave();

#endif