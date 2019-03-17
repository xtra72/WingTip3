#include "stm32l1xx_hal.h"
#include "config.h"
#include "crc16.h"
#include "log.h"
#include "flash.h"

#define FLASH_LOG_START_ADDR 0x08022000  /* Start @ of user Flash area */
#define FLASH_LOG_SIZE       0x00000100

#define TRACE(...)  TRACE_printf("LOG", __VA_ARGS__)

RET_VALUE   LOG_saveAt(uint32_t index);

static  LOG_CONFIG  config_ = 
{
    .startAddress   =   CONFIG_LOG_START_ADDRESS,
    .size           =   CONFIG_LOG_SIZE,
    .maxCount       =   CONFIG_LOG_COUNT_MAX,
    .enable         =   true
};

static  LOG_SLOT* pool_ = NULL;
static  int8_t  first_ = -1;
static  int8_t  last_ = -1;
static  int8_t  count_ = 0;
static  LOG_SLOT    slot;

RET_VALUE   LOG_init(LOG_CONFIG* config)
{
    ASSERT(config);

    if (LOG_isValidConfig(config) == RET_OK)
    {
        memcpy(&config_, config, sizeof(LOG_CONFIG));
    }
    
    pool_ = (LOG_SLOT*)config_.startAddress;
    for(uint32_t i = 0 ; i < config_.maxCount; i++)
    {
        uint16_t    crc = CRC16_calc(&pool_[i], sizeof(LOG_SLOT) - sizeof(uint32_t));
        if (crc == pool_[i].crc)
        {
            if (first_ < 0)
            {
                first_ = i;
            }
            else 
            {
                if (pool_[i].index < pool_[first_].index)
                {
                    first_ = i;
                }
            }

            if (last_ < 0)
            {
                last_ = i;
            }
            else 
            {
                if (pool_[i].index > pool_[last_].index)
                {
                    last_ = i;
                }
            }
        }
    }
    
    if (first_ == -1)
    {
        first_ = 0;
        last_ = 0;
        count_ = 0;
    }
    else 
    {
        last_ = (last_ + 1) % config_.maxCount;
        if (last_ == first_)
        {
            first_ ++;
        }
        count_ = ((last_ + config_.maxCount) - first_) % config_.maxCount;
    }
    
    return  RET_OK;
}

RET_VALUE   LOG_getConfig(LOG_CONFIG* config)
{
    ASSERT(config);
    
    memcpy(config, &config_, sizeof(LOG_CONFIG));
    
    return  RET_OK;
}

RET_VALUE   LOG_isValidConfig(LOG_CONFIG* config)
{
    if (config == NULL)
    {
        return  RET_ERROR;
    }
    
    if ((config->maxCount == 0) || (config->startAddress < CONFIG_USER_MEMORY_START) || (CONFIG_USER_MEMORY_END < config->startAddress + config->size * config->maxCount))
    {
        TRACE("Out of available memory area.\n"); 
        return  RET_ERROR;
    }
    
    return  RET_OK;
}

RET_VALUE   LOG_setEnable(bool enable)
{
    config_.enable = enable;
    
    return  RET_OK;
}

RET_VALUE    LOG_getCount(uint32_t* count)
{
    ASSERT(count);
    
    *count = count_;
    
    return  RET_OK;
}

RET_VALUE   LOG_getAt(uint32_t index, LOG* log)
{
    ASSERT(log);
    
    if (count_ <= index)
    {
        return  RET_ERROR;
    }
    
    memcpy(log, &pool_[(first_ + index) % CONFIG_LOG_COUNT_MAX].log, sizeof(LOG));
    
    return  RET_OK;
}

RET_VALUE   LOG_message(char* message)
{
    LOG*    log = pvPortMalloc(sizeof(LOG));
 
    log->type = LOG_TYPE_MESSAGE;
    FI_TIME_get(&log->time);
    strncpy(log->body.message, message, sizeof(log->body.message));
    LOG_save(log);
    
    vPortFree(log);
    
    return  RET_OK;
}

RET_VALUE   LOG_raw(void* data, uint32_t length)
{
    LOG*    log = pvPortMalloc(sizeof(LOG));
 
    log->type = LOG_TYPE_RAW;
    FI_TIME_get(&log->time);
    if (length > sizeof(log->body.raw.data))
    {
        length = sizeof(log->body.raw.data);
    }
    
    log->body.raw.length = length;
    memcpy(log->body.raw.data, data, length);
    
    LOG_save(log);
    
    vPortFree(log);
    
    return  RET_OK;
}

RET_VALUE   LOG_save(LOG* log)
{
    RET_VALUE   ret;

    ASSERT(log != NULL);
    
    memcpy(&slot.log, log, sizeof(LOG));
    
    if (count_ == 0)
    {
        slot.index = 1;
    }
    else
    {
        slot.index = pool_[(last_ - 1 + CONFIG_LOG_COUNT_MAX) % CONFIG_LOG_COUNT_MAX].index + 1;
    }
    slot.crc = CRC16_calc(&slot, sizeof(LOG_SLOT) - sizeof(uint32_t));
    
    ret = FLASH_write(FLASH_LOG_START_ADDR + FLASH_LOG_SIZE * last_, &slot, sizeof(slot));
    if (ret == RET_OK)
    {
        last_ = (last_ + 1) % CONFIG_LOG_COUNT_MAX;
        if (first_ == last_)
        {
            first_ = (first_ + 1) % CONFIG_LOG_COUNT_MAX;
        }
        else
        {
            count_++;
        }        
    }
    
//    LOG_showDebug();
    
    return  ret;
}


RET_VALUE   LOG_clear(void)
{
    RET_VALUE ret;
    
    ret = FLASH_erase(FLASH_LOG_START_ADDR, CONFIG_LOG_COUNT_MAX * FLASH_LOG_SIZE);
    if (ret == RET_OK)
    {
        first_ = 0;
        last_ = 0;
        count_ = 0;
    }
    
    return  ret;
}

bool    LOG_isValid(LOG_SLOT* slot)
{
    ASSERT(slot!= NULL);
    
    return  slot->crc == CRC16_calc(&slot, sizeof(LOG_SLOT) - sizeof(slot->crc));
}

static char     buffer_[256];

RET_VALUE    LOG_printf
(
    const char *format, 
    ... 
)
{
    if (config_.enable)
    {
        va_list  ap;

        va_start(ap, format);
        vsnprintf(buffer_, sizeof(buffer_),  (char *)format, ap );
        va_end(ap);

        LOG_message(buffer_);
    }
    
    return  RET_OK;
}

RET_VALUE   LOG_showDebug()
{
    SHELL_printf("First : %d\n", first_);
    SHELL_printf("Last : %d\n", last_);
    SHELL_printf("Count : %d\n", count_);
    
    return  RET_OK;
}