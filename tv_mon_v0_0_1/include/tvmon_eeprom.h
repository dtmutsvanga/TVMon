
#ifndef TVMON_EEPROM_H
#define TVMON_EEPROM_H
#include "stdint.h"
#include "tvmon_err.h"

#define EEPROM_ADDR         0x50
#define MAX_WRT_ADDR        4095// Maximum write address
#define MAX_LEN_DATA        MAX_WRT_ADDR+1
#define SYSDATA_RESERVE_LEN 10  // System resrved address length
#define USER_DATA_START_ADDR SYSDATA_RESERVE_LEN

#define EEPROM_INITD_FLG_ADDR 0 // Flag used to check if epom is initialized
#define WRAP_ARND_FLG_ADDR  1   // Used to see if we've reached max addr and rewritten over lower addresses
#define LAST_WRT_ADDR_ADDR_LSB  2   // Last write address length
#define LAST_WRT_ADDR_ADDR_MSB  3
#define EEPROM_INITD_FLAG   0xbc
#define WRAP_AROUND_FLAG    0xee


// TV usage data struct. 
//\ Do not re-arrange! Bit-fields packed
/*typedef struct tv_usage_data {
    uint8_t game_time:8;        // game time
    uint8_t cons_id:2;          // console id
    uint8_t min:6;              // time:min
    uint8_t pers_id:2;          // personell id
    uint8_t day:5;              // time: day of month
    uint8_t game_id:3;          // game id
    uint8_t hr:5;               // time :hr
    uint8_t mnth:4;             // time: month of yr
    uint8_t soft_ver:4;         // Software version
} usage_data_t;*/
typedef struct tv_usage_data {
    uint8_t day;              // time: day of month
    uint8_t mnth;             // time: month of yr
    uint8_t hr;               // time :hr
    uint8_t min;              // time:min
    uint8_t cons_id;          // console id
    uint8_t game_id;          // game id
    uint8_t game_time;        // game time
    uint8_t pers_id;          // personell id
    uint8_t soft_ver;         // Software version
} usage_data_t;
typedef struct tvmon_eep {
    uint16_t curr_wrt_addr;
    usage_data_t last_save_data;
    usage_data_t *tx_data_buff;
    int tx_buff_len;
    bool wrap_arnd;
    bool initd;
} tvmon_eep_t;

extern tvmon_eep_t tvmon_eeprom; 
TVMON_ERR tvmon_eeprom_init();
TVMON_ERR tvmon_eeprom_wrt_byte(char data);
TVMON_ERR tvmon_eeprom_rd_byte(uint16_t addr, uint8_t *data);
TVMON_ERR tvmon_eeprom_wrt_tv_ud( usage_data_t *buff);
TVMON_ERR tvmon_eeprom_get_tv_ud_dump(uint16_t addr, uint8_t* buff_ptr, int buff_length);
TVMON_ERR tvmon_eeprom_clear_data(uint16_t start_addr=0, int len=MAX_WRT_ADDR);
#endif //TVMON_EEPROM_H
