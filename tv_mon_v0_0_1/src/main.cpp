#include "main.h"
/************************  FORWARD DECLARATIONS  ***************************************/
void init_outputs();
void init_TVMon_drvr();
void init_http_webserver();
void init_WiFi();
// server callback functions
void handle_root();
void handle_get_date_time();
void handle_get_all_data();
void handle_get_st1_tm();
void handle_get_st2_tm();
void handle_reset_st1();
void handle_reset_st2();
void handle_set_game_data();
void handle_not_found();
void handle_admin_settings();
void handle_clear_eeprom();
void handle_set_new_date();
void track_station_time();
void set_relays();
void set_notification();
void wifi_reconnect(int timeout);

/****************************************************************************************/
/***********************  ARDUINO FUNCTIONS   *****************************************/
/****************************************************************************************/
void setup()
{
  // Initilalize
  init_outputs(); //outputs
  TVMON_DEBUG_INIT(9600);
  init_WiFi();
  init_TVMon_drvr();       // driver
  drvr.rtc_module->init(); //rtc
  tvmon_eeprom_init();     //eeprom
  init_http_webserver();   //http_webserver
}

void loop()
{
  drvr.WiFi_server.server->handleClient();
  MDNS.update();
  wifi_reconnect(3);
  track_station_time();
  set_relays();
  set_notification();
}

/****************************************************************************************/
/********************* FUNCTION DEFINITIONS *********************************************/
/****************************************************************************************/

void set_relays()
{
  digitalWrite(REL1_PIN, drvr.io_manager.st1_state);
  digitalWrite(REL2_PIN, drvr.io_manager.st2_state);
}

void set_notification()
{
  static unsigned long int prev_beep = millis();
  static bool state = BUZZER_ON_STATE;

  if (drvr.io_manager.beep)
  {
    unsigned long int curr_wait = (state == BUZZER_ON_STATE) ? ON_BEEP_TIME : OFF_BEEP_TIME;
    if (millis() - prev_beep > curr_wait)
    {
      state = !state;
      digitalWrite(BUZZER_PIN, state);
      prev_beep = millis();
    }
  }
  else
  {
    state = BUZZER_ON_STATE;
    digitalWrite(BUZZER_PIN, !BUZZER_ON_STATE);
  }
}
void track_station_time()
{
  static uint64_t &st1_tm = drvr.io_manager.st1_tm;
  static uint64_t &st2_tm = drvr.io_manager.st2_tm;

  static bool &st1_state = drvr.io_manager.st1_state;
  static bool &st2_state = drvr.io_manager.st2_state;

  static unsigned long long prev_time = millis();
  unsigned long int elapsed_time = millis() - prev_time;

  if (elapsed_time > 1000)
  {
    prev_time = millis();
    if (st1_tm > 0)
    {
      st1_tm -= elapsed_time / 1000;
      // Avoid rollover
      st1_tm = (st1_tm > 30600) ? 0 : st1_tm; // Avoid rollover. 30600  = 510 mins * 60 secs
      st1_state = REL_ON_STATE;
    }
    else //st1_tm == 0
    {

      st1_state = !REL_ON_STATE;
    }
    if (st2_tm > 0)
    {
      st2_tm -= elapsed_time / 1000;
      st2_tm = (st2_tm > 30600) ? 0 : st2_tm; // Avoid rollover. 30600  = 510 mins * 60 secs
      st2_state = REL_ON_STATE;
    }
    else
    {
      st2_state = !REL_ON_STATE;
    }
  }

  // Track beeps
  if ((st1_state == REL_ON_STATE && st1_tm < BEEP_START_TIME) || (st2_state == REL_ON_STATE && st2_tm < BEEP_START_TIME))
  {
    drvr.io_manager.beep = true;
  }
  else
  {
    drvr.io_manager.beep = false;
  }
}

void wifi_reconnect(int timeout = 0)
{
  // Wait for connection
  if (WiFi.status() == WL_CONNECTED)
    return;
  WiFi.reconnect();
  if (!timeout)
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      TVMON_DEBUG(".");
    }
  else
  {
    int elapsed_time = 0;
    while (WiFi.status() != WL_CONNECTED && elapsed_time / 2 < timeout)
    {
      elapsed_time++;
      delay(500);
      TVMON_DEBUG(".");
    }
  }
}
void init_WiFi()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi_SSID, WiFi_PASSWD);
  TVMON_DEBUG("");
  wifi_reconnect();
  TVMON_DEBUG("");
  TVMON_DEBUG("Connected to ");
  TVMON_DEBUG(WiFi_SSID);
  TVMON_DEBUG("IP address: ");
  TVMON_DEBUG(WiFi.localIP());

  if (MDNS.begin("connectGaming"))
  {
    TVMON_DEBUG("MDNS responder started");
  }
}

/**
 * @brief Initialize outputs of module
 * 
 */
void init_outputs()
{

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(REL1_PIN, OUTPUT);
  pinMode(REL2_PIN, OUTPUT);
  pinMode(LED_0, OUTPUT);

  // Set initial levels
  digitalWrite(LED_0, !BUZZER_ON_STATE);
  digitalWrite(BUZZER_PIN, !BUZZER_ON_STATE);
  digitalWrite(REL1_PIN, !REL_ON_STATE);
  digitalWrite(REL2_PIN, !REL_ON_STATE);
}

/**
 * @brief Initialize TVMon structure 
 * 
 */
void init_TVMon_drvr()
{
  // Allocate memory
  drvr.rtc_module = new TVMon_RTC();
  drvr.eeprom = new tvmon_eep_t;
  drvr.WiFi_server.httpUpdater = new ESP8266HTTPUpdateServer(true);
  drvr.WiFi_server.server = new ESP8266WebServer(80);
  //drvr.WiFi_server.tcp_server       = new ESP8266WebServer(TCP_SERVER_PORT); TODO: Write TCP server
  drvr.WiFi_server.html_access_page = html_access_page;
  drvr.WiFi_server.admin_page = html_admin_page;

  // Initilize TX buffer
  memset(drvr.rx_tx_data.tx_data, 0, TX_BUFF_LEN);
  drvr.eeprom->tx_data_buff = drvr.rx_tx_data.tx_data;
  drvr.eeprom->tx_buff_len = TX_BUFF_LEN;

  // Init io
  drvr.io_manager.st1_state = !REL_ON_STATE;
  drvr.io_manager.st2_state = !REL_ON_STATE;
  drvr.io_manager.beep = !BUZZER_ON_STATE;
}

/**
 * @brief Initialize the webserver and 
 * 
 */
void init_http_webserver()
{
  ESP8266WebServer &server = *drvr.WiFi_server.server;
  drvr.WiFi_server.httpUpdater->setup(&server, OTAPATH, OTAUSER, OTAPASSWORD);
  server.on("/", handle_root);
  server.on("/date_time", handle_get_date_time);
  server.on("/get_all_data", handle_get_all_data);
  server.on("/st1_tm", handle_get_st1_tm);
  server.on("/st2_tm", handle_get_st2_tm);
  server.on("/reset_st1", handle_reset_st1);
  server.on("/reset_st2", handle_reset_st2);
  server.on("/gameData", handle_set_game_data);
  server.on("/set_date_time", handle_set_new_date);
  server.on("/clear_eeprom", handle_clear_eeprom);
  server.on("/admin", handle_admin_settings);
  server.onNotFound(handle_not_found);
  server.begin();
  TVMON_DEBUG("HTTP server started");
}

/*******************************    SERVER CALLBACK FUNCTIONS   **********************************/
void handle_root()
{
  TVMON_DEBUG("Handle root...");
  drvr.WiFi_server.server->send_P(200, "text/html", drvr.WiFi_server.html_access_page);
  TVMON_DEBUG("Handle root sent!");
}
void handle_get_date_time()
{
  // Get date and time from RTC module
  String curr_date;
  drvr.rtc_module->get_time(curr_date);
  drvr.WiFi_server.server->sendContent(curr_date);
}
void handle_get_all_data()
{
  //TODO:  rite hhandle_get_all_data_fxn
}
void handle_get_st1_tm()
{
  int tm = drvr.io_manager.st1_tm / 60;
  int sec = drvr.io_manager.st1_tm % 60;
  drvr.WiFi_server.server->sendContent(String(tm) + String(":") + String(sec));

  TVMON_DEBUG("St1_tm=" + String(tm));
}
void handle_get_st2_tm()
{
  int tm = drvr.io_manager.st2_tm / 60;
  int sec = drvr.io_manager.st2_tm % 60;
  drvr.WiFi_server.server->sendContent(String(tm) + String(":") + String(sec));
  TVMON_DEBUG("St2_tm=" + String(tm));
}

void create_usage_data(usage_data_t &udata, int g_time, int staxn, int g_id)
{
  date_time_t curr_date;
  udata.cons_id = staxn;
  udata.game_id = g_id;
  udata.game_time = g_time;
  udata.pers_id = 0;
  drvr.rtc_module->get_time(&curr_date);
  udata.min = curr_date.min;
  udata.hr = curr_date.hr;
  udata.day = curr_date.day_of_mnth;
  udata.mnth = curr_date.mnth;
}
void handle_set_game_data()
{
  // 1. Get data from server
  String data = drvr.WiFi_server.server->arg(0);
  int chckd_staxn = data.charAt(0) - '0';
  int chckd_game = data.charAt(1) - '0';
  int game_time = data.substring(2).toInt();

  if (chckd_game > 3 || chckd_staxn > 1 || game_time > 510)
  {
    drvr.WiFi_server.server->sendContent("Erroneous data received! Please check inputs!");
    return;
  }
  // 2. Copy data to usage data struct
  usage_data_t usage_data;
  create_usage_data(usage_data, game_time, chckd_staxn, chckd_game);
  // 3. If st1 is on, append time (ie add set time to time)
  if (chckd_staxn == 0)
  {
    drvr.io_manager.st1_tm += game_time * 60;
  }
  else
  {
    drvr.io_manager.st2_tm += game_time * 60;
  }

  // Save the data
  String msg;
  TVMON_ERR err = tvmon_eeprom_wrt_tv_ud(&usage_data);
  if (err != ERR_NOERR)
  {
    msg = "Could not save usage data! Contact system administor!";
  }
  else
  {
    msg = "GAME ON!. \n.Station " + String(chckd_staxn) + " turned on with time = " + String(game_time);
  }
  drvr.WiFi_server.server->sendContent(msg);
  TVMON_DEBUG(msg);
}

void handle_reset_st1()
{
  String msg = "Station 1 turned off";
  drvr.io_manager.st1_tm = 0;
  drvr.WiFi_server.server->sendContent(msg);
}

void handle_reset_st2()
{
  String msg = "Station 2 turned off";
  drvr.io_manager.st2_tm = 0;
  drvr.WiFi_server.server->sendContent(msg);
}
void handle_not_found()
{
  ESP8266WebServer &server = *drvr.WiFi_server.server;

  String message = "This page does not exist on this server\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
void handle_admin_settings()
{
  // TODO: Admin settings
  TVMON_DEBUG("Handle root...");
  drvr.WiFi_server.server->send_P(200, "text/html", drvr.WiFi_server.admin_page);
  TVMON_DEBUG("Handle root sent!");
}

void handle_clear_eeprom()
{
  tvmon_eeprom_clear_data();
  drvr.WiFi_server.server->sendContent("EEPROM cleared! Restarting device...");
  // Reset the esp8266
  ESP.reset();
}
void handle_set_new_date()
{
  TVMON_DEBUG("Handle new date called...");
  date_time_t new_dtm;
  String data = drvr.WiFi_server.server->arg(0);
  new_dtm.yr = data.substring(2, 4).toInt();
  new_dtm.mnth = data.substring(5, 7).toInt();
  new_dtm.day_of_mnth = data.substring(8, 10).toInt();
  new_dtm.hr = data.substring(11, 13).toInt();
  new_dtm.min = data.substring(14, 16).toInt();

  drvr.rtc_module->set_date_time(&new_dtm);
  String nw_date;
  drvr.rtc_module->get_time(nw_date);
  String resp = "New date set. New date is now :" + nw_date;
  drvr.WiFi_server.server->sendContent(resp);
  TVMON_DEBUG(resp);
}
