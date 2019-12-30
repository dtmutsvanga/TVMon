#ifndef TVMON_RTC_H
#define TVMON_RTC_H
#include <Arduino.h>
#include "uRTCLib.h"
#include "tvmon_err.h"
#include "stdint.h"
#include "bsp.h"

typedef struct date_time 
	{
		uint8_t sec;
		uint8_t min;
		uint8_t hr;
		uint8_t day_of_wk;
		uint8_t day_of_mnth;
		uint8_t mnth;
		uint8_t yr;
	} date_time_t;
	
class TVMon_RTC : public  uRTCLib {
 public:
	TVMon_RTC(uint8_t SDA=I2C_SDA, uint8_t SCL=I2C_SCL, uint8_t addr=I2C_RTC_ADDR,  uint8_t model=URTCLIB_MODEL_DS1307);
	TVMON_ERR init(int SDA=I2C_SDA, int SCL=I2C_SCL);
	TVMON_ERR get_time(date_time_t *date=nullptr);
	TVMON_ERR get_time(String &date);
	TVMON_ERR get_unix_time(uint32_t &date);
	TVMON_ERR set_date_time(date_time_t *date);

	private:
	String print_data(date_time_t *date);
	date_time_t *m_curr_time;
};

/****************************** TimeStamp functions, curtesy of see https://github.com/kosme/timestamp32bits/blob/master/src/timestamp32bits.h#L23 */
#define YEAR 31536000UL
#define DAY 86400UL
#define HOUR 3600UL
#define MINUTE 60UL

class timestamp32bits{
public:
  timestamp32bits(void);
  timestamp32bits(uint16_t epoch_year);
  unsigned long timestamp(char year, char month, char day, char hour, char min, char seg);

private:
  const short _days[12]={0,31,59,90,120,151,181,212,243,273,304,334};
  signed char _offset = 30;
};

#endif
