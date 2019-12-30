#ifndef TVMON_WIFI_H
#define TVMON_WIFI_H
#include "bsp.h"
#include "stdint.h"
#include "serial_debug.h"
#include "tvmon_err.h"
class ESP8266WebServer;
class ESP8266HTTPUpdateServer;

class TVMon_WiFi {
	public:
	typedef TVMON_ERR (*TVMON_WIFI_CB)(void*);
	TVMon_WiFi(char *SSID=WiFi_SSID, char *pswd=WiFi_PASSWD, 
				char *OTA_pswd=OTAPASSWORD, char *OTA_USER_NAME=OTAUSER, char *OTA_path=OTAPATH, 
						uint8_t TCP_server_port=TCP_SERVER_PORT);

	void set_call_backs(TVMON_WIFI_CB admin_setting, TVMON_WIFI_CB get_date_tm, TVMON_WIFI_CB set_date_time, TVMON_WIFI_CB get_dump, TVMON_WIFI_CB get_st1_tm, TVMON_WIFI_CB get_st2_tm, 
	TVMON_WIFI_CB set_staxn_tm, TVMON_WIFI_CB clear_eeprom, TVMON_WIFI_CB set_tv_usage_data);
	void handle_client();
	/*void set_save_admin_settings_CB(TVMON_WIFI_CB fxn);
	void set_get_date_time_CB(TVMON_WIFI_CB fxn);
	void set_get_all_data(TVMON_WIFI_CB fxn);
	void set_get_st1_tm_CB(TVMON_WIFI_CB fxn);
	void set_get_st2_tm_CB(TVMON_WIFI_CB fxn);
	void set_set_time_CB(TVMON_WIFI_CB tm);
	void set_clear_eeprom_CB(TVMON_WIFI_CB fxn);
	void set_set_usage_data_CB(TVMON_WIFI_CB fxn);
	*/

	protected:
	static void handle_root();
	static void handle_get_date_time();
	static void handle_get_all_data();
	static void handle_get_st1_tm();
	static void handle_get_st2_tm();
	static void handle_set_game_data();
	static void handle_not_found();
	static void handle_admin_settings();

	private:
	String m_wifi_ssid;
	String m_wifi_paswd;
	String m_OTA_paswd;
	String m_OTA_path;
	String m_OTA_user;
	uint8_t m_tcp_server_port;
	ESP8266WebServer *m_server;
	ESP8266HTTPUpdateServer *m_update_server;
	const char* m_access_page;
	const char* m_admin_page;
	String* m_tx_buff;
	void server_init();
	// Callback functions
	TVMON_WIFI_CB save_admin_settings;
	TVMON_WIFI_CB get_date_time;
	TVMON_WIFI_CB get_all_data;
	TVMON_WIFI_CB get_st1_tm;
	TVMON_WIFI_CB get_st2_tm;
	TVMON_WIFI_CB set_date_time;
	TVMON_WIFI_CB set_staxn_time;
	TVMON_WIFI_CB clear_eeprom;
	TVMON_WIFI_CB set_usage_data;


};

#endif
