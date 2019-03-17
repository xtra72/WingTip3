#include <string.h>
#include "assert.h"
#include "lpwan.h"
#include "config.h"
#include "me_i10kl.h"
#include "shell.h"

static const LPWAN_DESCRIPTION lpwans_[] = 
{
    {
        .type       =   LPWAN_TYPE_ME_I10KL,
        
        .init       =   ME_I10KL_init,
        .final      =   ME_I10KL_final,

        .start      =   ME_I10KL_start,
        .stop       =   ME_I10KL_stop,

        .getVersion =   ME_I10KL_getVersion,
        
        .setWakeUp  =   ME_I10KL_setWakeUp,
        .getWakeUp  =   ME_I10KL_getWakeUp,
        
        .reset      =   ME_I10KL_reset,
        
        .getState0  =   ME_I10KL_getState0,
        .getNetId   =   ME_I10KL_getNetId,
        
        .getDevAddr =   ME_I10KL_getDevAddr,
        .setDevAddr =   ME_I10KL_setDevAddr,
        
        .getPort    =   ME_I10KL_getPort,
        .setPort    =   ME_I10KL_setPort,
        
        .createSocket = ME_I10KL_createSocket,

        .send       =   ME_I10KL_send,
        
        .at         =   ME_I10KL_at,
        .receive    =   ME_I10KL_receive,

        .setAutoSleep = ME_I10KL_setAutoSleep,
        .getAutoSleep = ME_I10KL_getAutoSleep,
        
        .getPSM     =   ME_I10KL_getPSM

    }
};

/*
 * The task that implements the command console processing.
 */
void LPWAN_main( void const *parameters );
int LPWAN_cmdVersion(char *argv[], uint32_t argc);


static  LPWAN_DESCRIPTION*    description_ = NULL;

RET_VALUE   LPWAN_init(void* config, uint32_t size)
{
    ASSERT((config != NULL) && (sizeof(LPWAN_CONFIG) <= size));
    
    RET_VALUE   ret = RET_INVALID_ARGUMENT;
    uint32_t    i;
    
    for(i = 0 ; i < sizeof(lpwans_) / sizeof(LPWAN_DESCRIPTION) ; i++)
    {
        if (((LPWAN_CONFIG*)config)->type == lpwans_[i].type)
        {
            description_ = &lpwans_[i];
            break;
        }
    }
    
    if (description_)
    {
        description_->init(config, size);
        ret = RET_OK;
    }
        
    return  ret;
}

RET_VALUE   LPWAN_final(void)
{
    ASSERT(description_ != NULL);
        
    if (description_->final)
    {
        return  description_->final();
    }
    
    return  RET_OK;
}

RET_VALUE   LPWAN_start(void)
{
    ASSERT(description_ != NULL);
    
    if (description_->start)
    {
        return  description_->start();
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_stop(void)
{
    ASSERT(description_ != NULL);
    
    if (description_->stop)
    {
        return  description_->stop();
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_reset(void)
{
    ASSERT(description_ != NULL);
    
    if (description_->reset)
    {
        return  description_->reset();
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getVersion(char* version, uint32_t maxVersionLength)
{
    ASSERT(description_ != NULL);
    
    if (description_->getVersion)
    {
        return  description_->getVersion(version, maxVersionLength);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getState0(bool* state)
{
    ASSERT(description_ != NULL);
    
    if (description_->getState0)
    {
        return  description_->getState0(state);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getState1(bool* state)
{
    ASSERT(description_ != NULL);
    
    if (description_->getState1)
    {
        return  description_->getState1(state);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getNetId(uint32_t* netId)
{
    ASSERT(description_ != NULL);
    
    if (description_->getNetId)
    {
        return  description_->getNetId(netId);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE    LPWAN_getDevAddr(uint32_t*   devAddr)
{
    ASSERT(description_ != NULL);
    
    if (description_->getDevAddr)
    {
        return  description_->getDevAddr(devAddr);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE    LPWAN_setDevAddr(uint32_t    devAddr)
{
    ASSERT(description_ != NULL);
    
    if (description_->setDevAddr)
    {
        return  description_->setDevAddr(devAddr);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE    LPWAN_getPort(uint8_t*   port)
{
    ASSERT(description_ != NULL);
    
    if (description_->getPort)
    {
        return  description_->getPort(port);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE    LPWAN_setPort(uint8_t    port)
{
    ASSERT(description_ != NULL);
    
    if (description_->setPort)
    {
        return  description_->setPort(port);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getChannel(uint8_t* channel)
{
    ASSERT(description_ != NULL);
    
    if (description_->getChannel)
    {
        return  description_->getChannel(channel);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_setChannel(uint8_t channel)
{
    ASSERT(description_ != NULL);
    
    if (description_->setChannel)
    {
        return  description_->setChannel(channel);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}


RET_VALUE   LPWAN_createSocket(uint32_t protocol, uint16_t port, uint32_t* socket)
{
    ASSERT(description_ != NULL);
    
    if (description_->createSocket)
    {
        return  description_->createSocket(protocol, port, socket);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_connect(uint16_t socket, uint8_t* ip, uint16_t port, uint8_t* data, uint32_t length)
{
    ASSERT(description_ != NULL);
    
    if (description_->connect)
    {
        return  description_->connect(socket, ip, port, data, length);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_join(bool force, uint32_t timeout)
{
    ASSERT(description_ != NULL);
    
    if (description_->join)
    {
        return  description_->join(force, timeout);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_send(char* message, bool confirmed)
{
    ASSERT(description_ != NULL);
    
    if (description_->send)
    {
        return  description_->send(message, confirmed);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}


RET_VALUE   LPWAN_setWakeUp(bool status)
{
    ASSERT(description_ != NULL);
    
    if (description_->setWakeUp)
    {
        return  description_->setWakeUp(status);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getWakeUp(bool* status)
{
    ASSERT(description_ != NULL);
    
    if (description_->getWakeUp)
    {
        return  description_->getWakeUp(status);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_at(char* request, char* buffer, uint32_t bufferLength, uint32_t* receivedLength)
{
    ASSERT(description_ != NULL);
    
    if (description_->at)
    {
        return  description_->at(request, buffer, bufferLength, receivedLength);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_receive(char* response, uint32_t length)
{
    ASSERT(description_ != NULL);
    
    if (description_->receive)
    {
        return  description_->receive(response, length);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_setAutoSleep(bool autoSleep)
{
    ASSERT(description_ != NULL);
    
    if (description_->setAutoSleep)
    {
        return  description_->setAutoSleep(autoSleep);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getAutoSleep(bool* state)
{
    ASSERT(description_ != NULL);
    
    if (description_->getAutoSleep)
    {
        return  description_->getAutoSleep(state);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}

RET_VALUE   LPWAN_getPSM(bool* state)
{
    ASSERT(description_ != NULL);
    
    if (description_->getPSM)
    {
        return  description_->getPSM(state);
    }
    
    return  RET_NOT_SUPPORTED_FUNCTION;
}