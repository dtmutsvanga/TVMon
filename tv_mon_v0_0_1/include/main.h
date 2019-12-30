#include <Arduino.h>
#include "tvmon_eeprom.h"
#include "tvmon_rtc.h"
#include "tvmon_wifi.h"
#include "serial_debug.h"
#include "proto.h"
#include "access_page.h"
#include "admin_settings.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "access_page.h"
#include <ESP8266HTTPUpdateServer.h>

#define TX_BUFF_LEN 50
/************************ DEFs and Strcuts **********************************************/
typedef struct TVMon
{
  struct
  {
    usage_data_t tx_data[TX_BUFF_LEN];
    usage_data_t rx_data;
    int len_tx;
  } rx_tx_data;

  TVMon_RTC *rtc_module;
  tvmon_eep_t *eeprom;

  struct WiFi_server
  {
    ESP8266WebServer *server;
    ESP8266WebServer *tcp_server;
    ESP8266HTTPUpdateServer *httpUpdater;
    const char* html_access_page;
    const char* admin_page;
  } WiFi_server;
  
  struct io_manager{
    uint64_t st1_tm;
    uint64_t st2_tm;
    
    bool st1_state;
    bool st2_state;
    bool beep;
  }io_manager;
  uint32_t curr_time;
} TVMon_drvr;

static TVMon_drvr drvr = {0};
