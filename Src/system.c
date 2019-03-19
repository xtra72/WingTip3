/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l1xx_hal.h"
#include "cmsis_os.h"
#include "config.h"
#include "system.h"

#define TRACE(...)  TRACE_printf("SYS", __VA_ARGS__)

//static  IWDG_HandleTypeDef hiwdg;

extern  ADC_HandleTypeDef hadc;

static  SYSTEM_CONFIG  config_ = 
{
    .activation = false,
    .activationTime = 0,
    .wdog =
    {
        .enable =   true
    }
};

RET_VALUE   SYSTEM_init(SYSTEM_CONFIG* config)
{
    ASSERT(config);
    
    memcpy(&config_, config, sizeof(SYSTEM_CONFIG));

    if (config_.wdog.enable)
    {
#if 0
        hiwdg.Instance = IWDG;
        hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
        hiwdg.Init.Reload = 4095;
        
        if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
        {
            //_Error_Handler(__FILE__, __LINE__);
            return  RET_ERROR;
        }
#endif
    }
    
    return  RET_OK;    
}

RET_VALUE   SYSTEM_getConfig(SYSTEM_CONFIG* config)
{
    ASSERT(config);
    
    memcpy(config, &config_, sizeof(SYSTEM_CONFIG));

    return  RET_OK;
}

RET_VALUE   SYSTEM_setActivation(bool activation)
{
    if (config_.activation != activation)
    {
        config_.activation = activation;
        FI_TIME_get(&config_.activationTime);
    }
    
    return  RET_OK;
}

bool    SYSTEM_getActivation(void)
{
    return  config_.activation;
}

RET_VALUE   SYSTEM_getActivationTime(FI_TIME* time)
{
    ASSERT(time);

    *time = config_.activationTime;
    
    return  RET_OK;
}

RET_VALUE   SYSTEM_WDOG_setEnable(bool enable)
{
    if (config_.wdog.enable != enable)
    {
        config_.wdog.enable = enable;
    }
    
    return  RET_OK;
}

RET_VALUE   SYSTEM_WDOG_getEnable(bool* enable)
{
    ASSERT(enable);
    
    *enable = config_.wdog.enable;
    
    return  RET_OK;
}

#define NVIC_AIRCR_VECTKEY    (0x5FA << 16)   /*!< AIRCR Key for write access   */
#define NVIC_SYSRESETREQ            2         /*!< System Reset Request         */

void SYS_reset(void)
{
  SCB->AIRCR  = (NVIC_AIRCR_VECTKEY | (SCB->AIRCR & (0x700)) | (1<<NVIC_SYSRESETREQ)); /* Keep priority group unchanged */
  __DSB();                                                                                 /* Ensure completion of memory access */              
  while(1);                                                                                /* wait until reset */
}

uint32_t    SYS_getBatteryLevel(void)
{
    HAL_ADC_Start(&hadc);
    uint32_t    value;
    
    if (HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK)
    {
        return  0;
    }
    
    value = HAL_ADC_GetValue(&hadc) ;
    return  (uint32_t)(value / 4095.0 / 3.3 * 13.3 * 100);
}

bool	SYS_getExtPower18(void)
{
	return	(HAL_GPIO_ReadPin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin) == GPIO_PIN_SET);
}

RET_VALUE	SYS_setExtPower18(bool on)
{
	if (on)
  	{
    	HAL_GPIO_WritePin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin, GPIO_PIN_SET);
	}
	else 
	{
    	HAL_GPIO_WritePin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin, GPIO_PIN_RESET);
	}
	
  	return	RET_OK;
}


bool	SYS_getExtPower33(void)
{
	return	(HAL_GPIO_ReadPin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin) == GPIO_PIN_SET);
}

RET_VALUE	SYS_setExtPower33(bool on)
{
	if (on)
  	{
      	HAL_GPIO_WritePin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin, GPIO_PIN_SET);
	}
	else 
	{
      	HAL_GPIO_WritePin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin, GPIO_PIN_RESET);
	}
	
  	return	RET_OK;
}

RET_VALUE	SYS_setExtPower(bool on)
{
	if (on)
  	{
        TRACE("Exte Power : On\n");
      	HAL_GPIO_WritePin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin, GPIO_PIN_SET);
    	HAL_GPIO_WritePin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin, GPIO_PIN_SET);
	}
	else 
	{
        TRACE("Exte Power : Off\n");
      	HAL_GPIO_WritePin(EXT_3_3V_EN_GPIO_Port, EXT_3_3V_EN_Pin, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(EXT_1_8V_EN_GPIO_Port, EXT_1_8V_EN_Pin, GPIO_PIN_RESET);
	}
	
  	return	RET_OK;
}



RET_VALUE   SYSTEM_globalConfigSave()
{
    extern  bool    configSave_;

    configSave_ = true;
    
    return  RET_OK;
}