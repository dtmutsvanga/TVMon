#include "main.h"
/************************  FORWARD DECLARATIONS  ***************************************/
void init_outputs();
void init_TVMon_drvr();
void init_http_webserver();
void init_WiFi();
void init_websockets_server();
// server callback functions
void handle_root();
TVMON_ERR handle_get_date_time(uint8_t *buff, int &len);
TVMON_ERR handle_get_all_data( uint8_t *buff, int len);
TVMON_ERR handle_get_st1_tm(uint8_t *buff, int &len);
TVMON_ERR handle_get_st2_tm(uint8_t *buff, int &len);
TVMON_ERR handle_reset_stX(uint8_t num, uint8_t *payload, size_t len);
void handle_ws_payload(uint8_t num, uint8_t *payload, size_t len);
//void handle_reset_st2();
TVMON_ERR handle_set_game_data(int num, uint8_t *data, int len);
void handle_not_found();
void handle_admin_settings();
void handle_clear_eeprom();
void handle_set_new_date();
void track_station_time();
void set_relays();
void set_notification();
void wifi_reconnect(int timeout);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void eeprom_fdbk_cb(int8_t buff[2]);
/***********************  ARDUINO FUNCTIONS   *****************************************/
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
  init_websockets_server();
  dns_server.start(53, "*", WiFi.localIP());
}

void loop()
{
  drvr.WiFi_server.server->handleClient();
  drvr.WiFi_server.ws->loop();
  MDNS.update();
  dns_server.processNextRequest();
  wifi_reconnect(3);
  track_station_time();
  set_relays();
  set_notification();
}

/********************* FUNCTION DEFINITIONS *********************************************/
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
  //dns_server.start(53, "*", WiFi.localIP());
}
void init_WiFi()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(WiFi_SSID_ESP, WiFi_PASSWD);
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
  drvr.eeprom = &tvmon_eeprom;
  drvr.WiFi_server.httpUpdater = new ESP8266HTTPUpdateServer(true);
  drvr.WiFi_server.server = new ESP8266WebServer(80);
  drvr.WiFi_server.ws = new WebSocketsServer(81);

  drvr.eeprom->fdbk_cb = eeprom_fdbk_cb;
  //drvr.WiFi_server.tcp_server       = new ESP8266WebServer(TCP_SERVER_PORT); TODO: Write TCP server
  drvr.WiFi_server.html_access_page = html_access_page;
  drvr.WiFi_server.admin_page = html_admin_page;

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
  server.on("/set_date_time", handle_set_new_date);
  server.on("/clear_eeprom", handle_clear_eeprom);
  server.on("/admin", handle_admin_settings);
  server.onNotFound(handle_root);
  server.begin();
  TVMON_DEBUG("HTTP server started");
}

void init_websockets_server()
{
  drvr.WiFi_server.ws->begin();
  drvr.WiFi_server.ws->onEvent(webSocketEvent);
}

/*******************************    SERVER CALLBACK FUNCTIONS   **********************************/
void handle_root()
{
  TVMON_DEBUG("Handle root...");
  drvr.WiFi_server.server->send_P(200, "text/html", drvr.WiFi_server.html_access_page);
  TVMON_DEBUG("Handle root sent!");
}
TVMON_ERR handle_get_date_time(uint8_t *buff, int &len)
{
  // Get date and time from RTC module
  String curr_date;
  drvr.rtc_module->get_time(curr_date);
  curr_date.toCharArray((char *)buff, TX_BUFF_LEN - 1);
  len = curr_date.length();
  return ERR_NOERR;
}

TVMON_ERR handle_get_all_data( uint8_t *buff, int len)
{
  //Get data
  digitalWrite(BUZZER_PIN, BUZZER_ON_STATE);
  tvmon_eeprom_get_tv_ud_dump(USER_DATA_START_ADDR, buff, len);
  delay(100);
  digitalWrite(BUZZER_PIN, !BUZZER_ON_STATE);
  return ERR_NOERR;
}

TVMON_ERR handle_get_st1_tm(uint8_t *buff, int &len)
{
  int tm = drvr.io_manager.st1_tm / 60;
  int sec = (drvr.io_manager.st1_tm) % 60;
  String data = (tm < 10) ? ("0" + String(tm)) : String(tm);
  data += String(":");
  data += (sec < 10) ? ("0" + String(sec)) : String(sec);
  data.toCharArray((char *)buff, TX_BUFF_LEN);
  len = data.length();
  TVMON_DEBUG("St1_tm=" + data);
  return ERR_NOERR;
  }
  TVMON_ERR handle_get_st2_tm(uint8_t * buff, int &len)
  {
    int tm = drvr.io_manager.st2_tm / 60;
    int sec = (drvr.io_manager.st2_tm) % 60;
    String data = (tm < 10) ? ("0" + String(tm)) : String(tm);
    data += String(":");
    data += (sec < 10) ? ("0" + String(sec)) : String(sec);
    data.toCharArray((char *)buff, TX_BUFF_LEN);
    len = data.length();
    TVMON_DEBUG("St2_tm=" + data);
    return ERR_NOERR;
  }

  void create_usage_data(usage_data_t & udata, int g_time, int staxn, int g_id, int amnt)
  {
    date_time_t curr_date;
    udata.cons_id = staxn;
    udata.game_id = g_id;
    udata.game_time = g_time;
    udata.pers_id = 0;
    udata.amnt_paid = amnt;
    drvr.rtc_module->get_time(&curr_date);
    udata.min = curr_date.min;
    udata.hr = curr_date.hr;
    udata.day = curr_date.day_of_mnth;
    udata.mnth = curr_date.mnth;
  }
  TVMON_ERR handle_set_game_data(int num, uint8_t *payload, int len)
  {
    String msg;
    int chckd_staxn = payload[0];
    int chckd_game = payload[1];
    int game_time = payload[2]; //Actual time divided by two
    int amnt = payload[3];
    TVMON_DEBUG_nln("Checked station: " + String(chckd_staxn));
    TVMON_DEBUG_nln("\tChecked game: " + String(chckd_game));
    TVMON_DEBUG_nln("\tGame Time:" + String(game_time));
    if (chckd_game > (NUM_OF_GAMES-1) || chckd_staxn > (NUM_OF_STAXNS - 1) || game_time > 510)
    {
      msg = "Erroneous data rceived by server. Please check inputs an try again.";
      drvr.WiFi_server.ws_tx_buff[0] = SERV_CMD ::ALERT;
      msg.toCharArray(reinterpret_cast<char *>(drvr.WiFi_server.ws_tx_buff + 1), TX_BUFF_LEN);
      drvr.WiFi_server.ws->sendBIN(num, drvr.WiFi_server.ws_tx_buff, 1 + msg.length());

      return ERR_GAME_DATA;
    }
    // 2. Copy data to usage data struct
    usage_data_t usage_data;
    create_usage_data(usage_data, game_time, chckd_staxn, chckd_game, amnt);
    // 3. If st1 is on, append time (ie add set time to time)
    if (chckd_staxn == 0)
    {
      drvr.io_manager.st1_tm += game_time * 2 * 60;
    }
    else
    {
      drvr.io_manager.st2_tm += game_time * 2 * 60;
    }

    // Save the data

    TVMON_ERR err = tvmon_eeprom_wrt_tv_ud(&usage_data);
    if (err != ERR_NOERR)
    {
      msg = "Could not save usage data! Contact system administor!";
    }
    else
    {
      msg = "GAME ON!. \nStation " + String(chckd_staxn) + " turned on with time = " + String(game_time * 2);
    }
    //drvr.WiFi_server.server->sendContent(msg);
    drvr.WiFi_server.ws_tx_buff[0] = SERV_CMD ::ALERT;
    msg.toCharArray(reinterpret_cast<char *>(drvr.WiFi_server.ws_tx_buff + 1), TX_BUFF_LEN);
    drvr.WiFi_server.ws->sendBIN(num, drvr.WiFi_server.ws_tx_buff, 1 + msg.length());
    TVMON_DEBUG(msg);
    return ERR_NOERR;
  }

  TVMON_ERR handle_reset_stX(uint8_t num, uint8_t *payload, size_t len)
  {
    int x = payload[0];
    String msg = "Station " + String(x + 1) + " turned off successfully!";
    if (x < NUM_OF_STAXNS)
    {
      *(&drvr.io_manager.st1_tm + x) = 0;
      // Create usage data
      usage_data_t udata;
      create_usage_data(udata, 0, x, 15, 0);  // 15 ==  Reset station

      TVMON_ERR err = tvmon_eeprom_wrt_tv_ud(&udata);
      if (err != ERR_NOERR)
      {
        msg = "Could not station reset! Contact system administor!";
      }
      // Form data accodng to protocol
      drvr.WiFi_server.ws_tx_buff[0] = SERV_CMD ::ALERT;
      msg.toCharArray(reinterpret_cast<char *>(drvr.WiFi_server.ws_tx_buff + 1), TX_BUFF_LEN);
      drvr.WiFi_server.ws->sendBIN(num, drvr.WiFi_server.ws_tx_buff, 1 + msg.length());
      TVMON_DEBUG("Handle station reset :" + msg);
    }
    else
    {
      TVMON_DEBUG("ERROR: Handle station reset!");
      return TVMON_ERR::ERR_DATA;
    }
    return ERR_NOERR;
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
    digitalWrite(BUZZER_PIN, BUZZER_ON_STATE);
    tvmon_eeprom_clear_data();
    digitalWrite(BUZZER_PIN, !BUZZER_ON_STATE);
    drvr.WiFi_server.server->sendContent("EEPROM cleared! Restarting device...");
    delay(5000);
    // Reset the esp8266
    ESP.restart();
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

  // Websockets

  void handle_ws_payload(uint8_t num, uint8_t * payload, size_t len)
  {
    TVMON_DEBUG_nln("handle_ws_payload:\t");
    if (len == 0)
    {
      TVMON_DEBUG_nln("ERROR WS Payload = 0 ");
      return;
    }

    WebSocketsServer *ws = drvr.WiFi_server.ws;
    int tx_len = 0;
    TVMON_DEBUG_nln("\tCOMMAND=" + String(payload[0]));
    switch (payload[0])
    {
    case SERV_CMD::GET_ST1_TM:
      TVMON_DEBUG("\tGET_ST1_TM");
      drvr.WiFi_server.ws_tx_buff[0] = SERV_CMD::GET_ST1_TM;
      handle_get_st1_tm(drvr.WiFi_server.ws_tx_buff + 1, tx_len);
      tx_len++;
  
      ws->sendBIN(num, drvr.WiFi_server.ws_tx_buff, tx_len);
      break;
    case SERV_CMD::GET_ST2_TM:
      TVMON_DEBUG("GET_ST2_TM");
      drvr.WiFi_server.ws_tx_buff[0] = SERV_CMD::GET_ST2_TM;

      handle_get_st2_tm(drvr.WiFi_server.ws_tx_buff + 1, tx_len);
      tx_len++;
      ws->sendBIN(num, drvr.WiFi_server.ws_tx_buff, tx_len);
      TVMON_DEBUG("Sent " + String(tx_len));
      break;
    case SERV_CMD::RESET_STX_T:
      TVMON_DEBUG("RESET_STX_T");
      handle_reset_stX(num, payload + 1, len);
      break;
    case SERV_CMD::SET_DATA:
      TVMON_DEBUG("SET_DATA");
      handle_set_game_data(num, payload + 1, len);
      break;
    case SERV_CMD::GET_CURR_TM:
      drvr.WiFi_server.ws_tx_buff[0] = SERV_CMD::GET_CURR_TM;
      handle_get_date_time(drvr.WiFi_server.ws_tx_buff + 1, tx_len);
      ws->sendBIN(num, drvr.WiFi_server.ws_tx_buff, tx_len + 1);
      break;
      case SERV_CMD::GET_ALL_DATA:
      drvr.eeprm_mem_cpy[0]=GET_ALL_DATA;
      handle_get_all_data(drvr.eeprm_mem_cpy+1, MAX_LEN_DATA-SYSDATA_RESERVE_LEN);
      ws->sendBIN(num, drvr.eeprm_mem_cpy, MAX_LEN_DATA+1);
      break;
    default:
      TVMON_DEBUG("Unknown cmd");
      break;
    }
  }
  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
  {
    WebSocketsServer *ws = drvr.WiFi_server.ws;
    switch (type)
    {
    case WStype_DISCONNECTED:
      //TVMON_DEBUG_PF("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
    {
      IPAddress ip = ws->remoteIP(num);
      //TVMON_DEBUG_PF("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      ws->sendTXT(num, "Connected");
    }
    break;
    case WStype_TEXT:
      //TVMON_DEBUG_PF("[%u] get Text: %s\n", num, payload);

      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      //TVMON_DEBUG_PF("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);
      handle_ws_payload(num, payload, length);
      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
    }
  }

void eeprom_fdbk_cb(int8_t buff[2]){
  drvr.WiFi_server.ws->broadcastBIN((uint8_t*)buff, 2);
}
