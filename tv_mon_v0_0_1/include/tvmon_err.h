#ifndef TVMON_ERR_H
#define TVMON_ERR_H
#include "stdint.h"

typedef enum 
{
    ERR_NOERR,
    ERR_EEPROM_WRT,
    ERR_EEPROM_RD,
    ERR_EEPROM_INIT,
    ERR_EEPROM_DATA_INVALID,
    ERR_EEPROM_LEN_DATA,
    
    ERR_RTC_RD,
    ERR_RTC_WRT,
    ERR_RTC_INIT,

    ERR_LAST_ADDR_OUT_OF_BBOUNDS
} TVMON_ERR;

#endif // TVMON_ERR_H
