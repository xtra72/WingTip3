#include "FreeRTOS.h"
#include <task.h>
#include <string.h>
#include <time.h>
#include "stm32l1xx_hal.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_rtc.h"
#include "cmsis_os.h"
#include "ret_value.h"
#include "fi_time.h"
#include "trace.h"

#define TRACE(...)  TRACE_printf("TIME", __VA_ARGS__)

extern  RTC_HandleTypeDef hrtc;
static  uint32_t    timeZone_ = 9*60*60;

RET_VALUE   FI_TIME_init(void)
{
#if 0
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
      return    RET_ERROR;
  }
#endif
    /* Check and handle if the system was resumed from StandBy mode */ 
    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        /* Clear Standby flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); 
    }

     /* Clear Wake-up timer flag if it is set    */
    /* Flag will set after exiting from Standby */
    if (LL_RTC_IsActiveFlag_WUT(RTC) == 1)
    {
        LL_RTC_ClearFlag_WUT(RTC);
    }

    /* Clear all related wakeup flags */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);  
  return    RET_OK;
}

RET_VALUE   FI_TIME_get(FI_TIME* value)
{
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;

  if (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
  {
      return    RET_ERROR;
  }

  if (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
  {
      return    RET_ERROR;
  }

  struct tm    tm;
  memset(&tm, 0, sizeof(struct tm));

  tm.tm_year = date.Year + 100;
  tm.tm_mon  = date.Month - 1;
  tm.tm_mday = date.Date;
  tm.tm_hour = time.Hours;
  tm.tm_min  = time.Minutes;
  tm.tm_sec  = time.Seconds;
  
  *value = mktime(&tm) - timeZone_;
  
  return    RET_OK;
}

RET_VALUE   FI_TIME_set(FI_TIME value)
{
    RTC_TimeTypeDef rtcTime;
    RTC_DateTypeDef rtcDate;

    struct tm*    tm;
    
    value += timeZone_;
    
    tm = gmtime(&value);
    
    memset(&rtcDate, 0, sizeof(rtcDate));
    memset(&rtcTime, 0, sizeof(rtcTime));
    
    rtcDate.Year = tm->tm_year - 100;
    rtcDate.Month = tm->tm_mon + 1;
    rtcDate.Date = tm->tm_mday;
    
    rtcTime.Hours = tm->tm_hour;
    rtcTime.Minutes = tm->tm_min;
    rtcTime.Seconds = tm->tm_sec;
    rtcTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtcTime.StoreOperation = RTC_STOREOPERATION_RESET;

    TRACE("Save Time : %d-%02d-%02d %02d:%02d:%02d [ %d ]\n", 2000+rtcDate.Year, rtcDate.Month, rtcDate.Date, rtcTime.Hours, rtcTime.Minutes, rtcTime.Seconds, value);
  
  if (HAL_RTC_SetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN) != HAL_OK)
  {
      return    RET_ERROR;
  }

  if (HAL_RTC_SetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN) != HAL_OK)
  {
      return    RET_ERROR;
  }

  return    RET_OK;

}

char*   FI_TIME_toString(FI_TIME time, char* format)
{
    static  char    buffer[64];
    
    time += timeZone_;

    memset(buffer, 0, sizeof(buffer));
    strftime(buffer, sizeof(buffer), format, localtime(&time));
    
    return  buffer;
}

RET_VALUE   FI_TIME_toRTCDateTime(FI_TIME value, RTC_TimeTypeDef *rtcTime, RTC_DateTypeDef *rtcDate)
{
    struct tm*    tm;
    
    value += timeZone_;

    tm = gmtime(&value);
    
    memset(rtcDate, 0, sizeof(RTC_DateTypeDef));
    memset(rtcTime, 0, sizeof(RTC_TimeTypeDef));
    
    rtcDate->Year = tm->tm_year - 100;
    rtcDate->Month = tm->tm_mon + 1;
    rtcDate->Date = tm->tm_mday;
    
    rtcTime->Hours = tm->tm_hour;
    rtcTime->Minutes = tm->tm_min;
    rtcTime->Seconds = tm->tm_sec;
    rtcTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtcTime->StoreOperation = RTC_STOREOPERATION_RESET;
  
  return    RET_OK;

}

RET_VALUE   FI_TIME_fromRTCDateTime(RTC_TimeTypeDef *rtcTime, RTC_DateTypeDef *rtcDate, FI_TIME *value)
{
  struct tm    tm;
  memset(&tm, 0, sizeof(struct tm));

  tm.tm_year = rtcDate->Year + 100;
  tm.tm_mon  = rtcDate->Month - 1;
  tm.tm_mday = rtcDate->Date;
  tm.tm_hour = rtcTime->Hours;
  tm.tm_min  = rtcTime->Minutes;
  tm.tm_sec  = rtcTime->Seconds;
  
  *value = mktime(&tm) - timeZone_;
  
  return    RET_OK;
}

uint32_t    FI_TIME_getTimeZone(void)
{
    return  timeZone_;
}
uint32_t    FI_TICK_get(void)
{
    return  (uint32_t)xTaskGetTickCount();
}

RET_VALUE   FI_TIME_getAlarm(FI_CLOCK* clock)
{
    RTC_AlarmTypeDef alarm;
    
    if (HAL_RTC_GetAlarm(&hrtc, &alarm, RTC_ALARM_A, RTC_FORMAT_BIN) != HAL_OK)
    {
        return  RET_ERROR;
    }

    *clock = ((alarm.AlarmTime.Hours * 60) + alarm.AlarmTime.Minutes) * 60 + alarm.AlarmTime.Seconds;
    
    return  RET_OK;
}

RET_VALUE    FI_TIME_setAlarm(FI_CLOCK clock)
{
    RTC_AlarmTypeDef sAlarm;
    
    sAlarm.AlarmTime.Hours = clock / SECONDS_OF_HOUR % HOURS_OF_DAY;
    sAlarm.AlarmTime.Minutes = clock / SECONDS_OF_MINUTE % MINUTES_OF_HOUR;
    sAlarm.AlarmTime.Seconds = clock % SECONDS_OF_MINUTE;
    sAlarm.AlarmTime.SubSeconds = 0x0;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;//RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_SECONDS;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 0x1;
    sAlarm.Alarm = RTC_ALARM_A;

    if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        TRACE(" WakeUp set error!\n");
        return  RET_ERROR;
    }
    
    TRACE(" WakeUp Time : %02d:%02d:%02d\n",     sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);


    /* Reactivate LSI clock if it has been stopped by system reset */
    if (LL_RCC_LSI_IsReady() != 1)
    {
        LL_RCC_LSI_Enable();
        while(LL_RCC_LSI_IsReady() != 1)
        {
        }
    }

    /* Configure the system Power */
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Enable write access to Backup domain */
    HAL_PWR_EnableBkUpAccess();

    /* Enable Ultra low power mode */
    HAL_PWREx_EnableUltraLowPower();

    /* Enable the fast wake up from Ultra low power mode */
    HAL_PWREx_EnableFastWakeUp();

    /* Check and handle if the system was resumed from StandBy mode */ 
    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        /* Clear Standby flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); 
    }

     /* Clear Wake-up timer flag if it is set    */
    /* Flag will set after exiting from Standby */
    if (LL_RTC_IsActiveFlag_WUT(RTC) == 1)
    {
        LL_RTC_ClearFlag_WUT(RTC);
    }

    /* Clear all related wakeup flags */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    return    RET_OK;
}


void RTC_WKUP_IRQHandler(void)
{
  HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}

void RTC_Alarm_IRQHandler(void)
{
    TRACE("Alarm!\n");
  HAL_RTC_AlarmIRQHandler(&hrtc);
}