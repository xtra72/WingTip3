#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "utils.h"
#include "fi_time.h"

bool    strToUint16(char* digits, uint16_t* value)
{
    if ((digits == NULL) || (strlen(digits) == 0))
    {
        return  false;
    }
    
    for(int i = 0 ; i < strlen(digits) ; i++)
    {
        if (!isdigit(digits[i]))
        {
            return  false;
        }
    }
    
    uint32_t    temp = strtoul(digits, 0, 10);
    if (temp > (uint32_t)UINT16_MAX)
    {
        return  false;
    }
        
    *value = (uint16_t)temp;
    
    return  true;
}

bool    strToUint32(char* digits, uint32_t* value)
{
    if ((digits == NULL) || (strlen(digits) == 0))
    {
        return  false;
    }
    
    for(int i = 0 ; i < strlen(digits) ; i++)
    {
        if (!isdigit(digits[i]))
        {
            return  false;
        }
    }
    
    *value = strtoul(digits, 0, 10);
    
    return  true;
}

uint32_t    seperateString(char* digits, char* delimitSet, char* seperatedStrings[], uint32_t maxSeperatedCount)
{
    uint32_t    count = 0;
    char*   token;
    
    token = strtok(digits, delimitSet);
    if (token != NULL)
    {
        seperatedStrings[count++] = token;
        while(token = strtok(NULL, delimitSet))
        {
            if (count >= maxSeperatedCount)
            {
                break;
            }
            
            seperatedStrings[count++] = token;
        }
    }
    
    return  count;
}

bool    strToIP(char* digits, uint8_t *ip)
{
    uint32_t    len;
    char        buffer[16];
    char*       seperatedStrings[4];
    
    len = strlen(digits);
    if ((len < 7) || (15 < len))
    {
        return  false;
    }
    
    strcpy(buffer, digits);
    
    if (seperateString(buffer, ".", seperatedStrings, 4) != 4)
    {
        return  false;
    }
    
    ip[0] = strtoul(seperatedStrings[0], 0, 10);
    ip[1] = strtoul(seperatedStrings[1], 0, 10);
    ip[2] = strtoul(seperatedStrings[2], 0, 10);
    ip[3] = strtoul(seperatedStrings[3], 0, 10);
    
    return  true;
}

bool    strToTime(char* digits, FI_TIME* time)
{
    ASSERT((digits != NULL) && (time != NULL));
    
    struct tm   tm;
    uint32_t    value;
    uint32_t    length = strlen(digits);
    
    if (length != 14)
    {        
        return  false;
    }
    
    for(int i = 0 ; i < length ; i++)
    {
        if (!isdigit(digits[i]))
        {
            return  false;
        }
    }

    memset(&tm, 0, sizeof(tm));
    
    value =  strtoul(&digits[12], 0, 10);
    digits[12] = 0;
    if (value < 60)
    {
        tm.tm_sec   = value;
        value = strtoul(&digits[10], 0, 10);
        digits[10] = 0;
        if (value < 60)
        {
            tm.tm_min   = value;
            value = strtoul(&digits[8], 0, 10);
            digits[8] = 0;
            if (value < 24)
            {
                tm.tm_hour  = value;
                value = strtoul(&digits[6], 0, 10);
                digits[6] = 0;
                if (1 <= value && value <= 31)
                {
                    tm.tm_mday  = value;
                    value = strtoul(&digits[4], 0, 10);
                    digits[4] = 0;
                    if (1 <= value && value <= 12)
                    {
                        tm.tm_mon   = value - 1;
                        value = strtoul(&digits[0], 0, 10);
                        if (2000 <= value && value <= 2100)
                        {
                            tm.tm_year   = value - 1900;

                            *time = mktime(&tm)  - FI_TIME_getTimeZone();

                            return  true;
                        }
                    }
                }
            }
        }
    }
    
    return  false;
}

double IEEE754_Binary32ToDouble(int value) // IEEE integer convert double.
{
        int minus = -1, exponent;
        double fraction, result;

       if((value&0x80000000) == 0)
       {
               minus = 1;
       }
       exponent = ((value & 0x7F800000) >> 23) - 127;
       fraction = (value & 0x7FFFFF) + 0x800000;
       fraction = fraction / 0x800000;
       result = minus * fraction * pow(2, exponent);
       return result;
}
