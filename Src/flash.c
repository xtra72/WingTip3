#include "stm32l1xx_hal.h"
#include "ret_value.h"
#include "assert.h"

#define TRACE(...)  TRACE_printf("FLASH", __VA_ARGS__)

RET_VALUE   FLASH_erase(uint32_t startAddress, uint32_t size)
{
    RET_VALUE   ret;
    uint32_t    PAGEError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    HAL_FLASH_Unlock();

    /* Erase the user Flash area
    (area defined by FLASH_FLASH_START_ADDR and FLASH_FLASH_END_ADDR) ***********/

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = startAddress / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;
    EraseInitStruct.NbPages     = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) == HAL_OK)
    {
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }
    
    HAL_FLASH_Lock();
    
    return  ret;
}

RET_VALUE   FLASH_write(uint32_t address, void* data, uint32_t size)
{
    ASSERT(data);
    
    RET_VALUE   ret;
    uint32_t Address = 0, PAGEError = 0;
    uint32_t* source;
    int32_t i, count;
    __IO uint32_t data32 = 0 , MemoryProgramStatus = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    __IO uint32_t WRPR_Value = 0x0, ProtectedPages = 0x0;
    FLASH_OBProgramInitTypeDef OBInit;

    HAL_FLASH_Unlock();

    /* Unlock the Option Bytes */  
    HAL_FLASH_OB_Unlock();

    HAL_FLASHEx_OBGetConfig(&OBInit);
#if 0
    TRACE("WRPSector0To31 : %d\n", OBInit.WRPSector0To31);
    TRACE("WRPSector32To63 : %d\n", OBInit.WRPSector32To63);
#endif
    
    OBInit.OptionType = OPTIONBYTE_WRP;
    OBInit.WRPState = OB_WRPSTATE_DISABLE;
    
    HAL_FLASHEx_OBProgram(&OBInit);
    
    /* Erase the user Flash area
    (area defined by FLASH_FLASH_START_ADDR and FLASH_FLASH_END_ADDR) ***********/

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = address / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;
    EraseInitStruct.NbPages     = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    source  = (uint32_t*)data;
    count   = (size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) == HAL_OK)
    {
        ret = RET_OK;
    
        Address = EraseInitStruct.PageAddress;
        for(i = 0 ; i < count ; i++)
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address + i*4, source[i]) != HAL_OK)
            {
                TRACE("Flash write error : %08x\n", Address + i*4);
                ret = RET_ERROR;
                break;
            }
        }
    }
    else 
    {
        TRACE("Flash erase error : %08x, %d\n", address, size);
        ret = RET_ERROR;
    }
    

    HAL_FLASH_Lock();
    
    return  ret;
}
