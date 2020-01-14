#include "tvmon_rtc.h"
#include "bsp.h"
#include "Wire.h"
#include "serial_debug.h"
#include <time.h>
#include "WString.h"

/**
 * @brief Construct a new tvmon rtc::tvmon rtc object
 * 
 * @param SDA I2C SDA pin
 * @param SCL I2C SCL pin
 * @param addr I2C address
 * @param model Model of the rtc , see uRTCLib.h
 */
TVMon_RTC::TVMon_RTC (uint8_t SDA, uint8_t SCL, uint8_t addr, uint8_t model) 
: uRTCLib(addr,model)
{
	this->m_curr_time = new date_time_t;
}

/**
 * @brief Initialize I2C bus, see arduino's Wire.h
 * 
 * @param SDA I2C SDA pin
 * @param SCL I2C SCL pin
 * @return TVMON_ERR returns ERR_NOERR always
 */
TVMON_ERR TVMon_RTC::init(int SDA, int SCL){
	Wire.begin(SDA, SCL);
	get_time();
	TVMON_DEBUG("TVMon_RTC::init() ERR_NOERR");
	return ERR_NOERR;
}

/**
 * @brief Refresh and get current time from RTC module
 * 
 * @param date Pointer to date_time_t object where the date is to be stored
 * @return TVMON_ERR Always returns ERR_NOERR
 */
TVMON_ERR TVMon_RTC::get_time(date_time_t *date){
	this->refresh();
	m_curr_time->sec = this->second();
	m_curr_time->min = this->minute();
	m_curr_time->hr  = this->hour();
	m_curr_time->day_of_wk = this->dayOfWeek();
	m_curr_time->day_of_mnth= this->day();
	m_curr_time->mnth=this->month();
	m_curr_time->yr = this->year();
//	this->print_data(date);
	
	if(date != nullptr)
		*date = *m_curr_time;

	return ERR_NOERR;
}

TVMON_ERR TVMon_RTC::get_time(String &date){
	TVMON_ERR err=this->get_time();
	if(err==ERR_NOERR)
		date=this->print_data(this->m_curr_time);

	return err;
}

TVMON_ERR TVMon_RTC::get_unix_time(uint32_t &date)
{
	this->get_time();
	timestamp32bits tstamp;	// Default 1970

	date = tstamp.timestamp(m_curr_time->yr, m_curr_time->mnth, m_curr_time->day_of_mnth, m_curr_time->hr, m_curr_time->min, m_curr_time->sec);
	return ERR_NOERR;
}

String TVMon_RTC::print_data(date_time_t *date)
{
	String dtm;
	dtm = String(date->day_of_mnth) + "/" + String(date->mnth) +"/" + String(date->yr)+" "+ String(date->hr) + ":"+ String(date->min);
	TVMON_DEBUG_nln("RTC DateTime: ");
	TVMON_DEBUG(dtm);
	return dtm;
}
/**
 * @brief Set new date to RTC module
 * 
 * @param date New date
 * @return TVMON_ERR Returns ERR_NOERR
 */
TVMON_ERR TVMon_RTC::set_date_time(date_time_t *date){
	TVMON_DEBUG_nln("\n\n Seting RTC date-time:");
	this->print_data(date);
	this->set(date->sec, date->min, date->hr,date->day_of_wk,date->day_of_mnth, date->mnth,date->yr);
	TVMON_DEBUG("\n\n Retreiving set date");
	this->get_time(date);
	//TODO: Compare saved and retrieved time values in unix
	
	return ERR_NOERR;
}


/***************** TimeStamp *******************************************/
timestamp32bits::timestamp32bits(void){
  timestamp32bits(1970);
}

timestamp32bits::timestamp32bits(uint16_t epoch_year){
  _offset = 2000 - epoch_year;
}

unsigned long timestamp32bits::timestamp(char year, char month, char day, char hour, char min, char seg){
  unsigned long _timestamp=(year+_offset)*YEAR;
  _timestamp += (day + _days[month - 1] + ((year + _offset + 2) / 4) - 1) * DAY;
	_timestamp += hour * HOUR;
	_timestamp += min * MINUTE;
	_timestamp += seg;
	return ((year % 4 == 0) && (month < 3)) ? _timestamp - DAY : _timestamp;
}
