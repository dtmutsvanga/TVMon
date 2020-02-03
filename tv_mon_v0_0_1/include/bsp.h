#ifndef BSP_H
#define BSP_H
#define NUM_OF_STAXNS 2
#define NUM_OF_GAMES 6
#define I2C_SDA 4
#define I2C_SCL 5
#define I2C_RTC_ADDR	0x68
#define I2C_EEPROM_ADDDR 0x50
#define BUZZER_PIN 10
#define REL1_PIN 14
#define REL2_PIN 16
#define LED_0 2

#define REL_ON_STATE false
#define BUZZER_ON_STATE true
// WiFi parameters
#define WiFi_SSID 		"CGmng"
#define WiFi_PASSWD 	"CGmng_2019"

#define OTAUSER         "admin"    // Set OTA user
#define OTAPASSWORD     "admin"    // Set OTA password
#define OTAPATH         "/firmware"// Set path for update
#define SERVERPORT      80    
#define TCP_SERVER_PORT	1616UL

// Notifications
#define ON_BEEP_TIME 300
#define OFF_BEEP_TIME 1700
#define BEEP_START_TIME 120

// Games


#endif //BSP_H
