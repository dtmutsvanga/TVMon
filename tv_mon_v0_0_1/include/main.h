#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DNSServer.h"
#include <ESP8266HTTPUpdateServer.h>
#include <WebSocketsServer.h>
#include "tvmon_eeprom.h"
#include "tvmon_rtc.h"
#include "tvmon_wifi.h"
#include "serial_debug.h"
#include "proto.h"
#include "access_page.h"
#include "admin_settings.h"
#include "access_page.h"


#define TX_BUFF_LEN 50
/************************ DEFs and Strcuts **********************************************/
typedef struct TVMon
{
  TVMon_RTC *rtc_module;
  tvmon_eep_t *eeprom;
  uint8_t eeprm_mem_cpy[MAX_LEN_DATA+1];
  struct WiFi_server
  {
    ESP8266WebServer *server;
    ESP8266WebServer *tcp_server;
    ESP8266HTTPUpdateServer *httpUpdater;
    WebSocketsServer *ws;
    const char* html_access_page;
    const char* admin_page;

    uint8_t ws_tx_buff[TX_BUFF_LEN];
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

IPAddress         apIP(192, 168, 43, 71);    // Private network for server
static TVMon_drvr drvr = {0};
static DNSServer dns_server;
