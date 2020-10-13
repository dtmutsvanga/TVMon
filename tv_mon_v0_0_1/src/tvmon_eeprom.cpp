/**
 * @file tvmon_eeprom.c
 * @author DTMutsvanga (dtmutsvanga@yahoo.com)
 * @brief  source for tv monitor eeprom data
 * @version 0.1
 * @date 2019-12-06
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "tvmon_eeprom.h"
#include "Wire.h"
#include "uEEPROMLib.h"
#include "bsp.h"
#include "serial_debug.h"
/*******************Private functions **************************/

// uEEPROMLib eeprom;
uEEPROMLib eeprom(EEPROM_ADDR);
tvmon_eep_t tvmon_eeprom;
/**
 * @brief Initialize EEPROM
 * 
 * @return TVMON_ERR 
 */
TVMON_ERR tvmon_eeprom_init()
{
    uint8_t initd_fl = 0, wrp_arnd_flg = 0;
    uint16_t curr_wrt_addr = 0;
    bool err = false;
    Wire.begin(I2C_SDA, I2C_SCL);
    //check if eprom is initialized
    eeprom.eeprom_read(EEPROM_INITD_FLG_ADDR, &initd_fl);
    if (initd_fl != EEPROM_INITD_FLAG)
    {
        TVMON_DEBUG("Init flag missing. Initializng....");
        curr_wrt_addr = USER_DATA_START_ADDR;
        err = eeprom.eeprom_write(EEPROM_INITD_FLG_ADDR, EEPROM_INITD_FLAG);
        err &= eeprom.eeprom_write(WRAP_ARND_FLG_ADDR, WRAP_AROUND_FLAG);
        err &= eeprom.eeprom_write(LAST_WRT_ADDR_ADDR_LSB, (0x00ff & curr_wrt_addr));
        err &= eeprom.eeprom_write(LAST_WRT_ADDR_ADDR_MSB, curr_wrt_addr >> 8);
        if (!err)
        {
            TVMON_DEBUG("eeprom.eeprom_write() \t\t ERR_EEPROM_WRT \t\t tvmon_eeprom_init()");
            return ERR_EEPROM_WRT;
        }
    }
    uint8_t temp = 0x00;
    eeprom.eeprom_read(LAST_WRT_ADDR_ADDR_LSB, &temp);
    curr_wrt_addr = (0x00ff & temp);
    eeprom.eeprom_read(LAST_WRT_ADDR_ADDR_MSB, &temp);
    curr_wrt_addr |= ((uint16_t)temp << 8);
    eeprom.eeprom_read(WRAP_ARND_FLG_ADDR, &wrp_arnd_flg);

    if (curr_wrt_addr > 4095)
    {
        TVMON_DEBUG("tvmon_eeprom_init() \t\t ERR_LAST_ADDR_OUT_OF_BBOUNDS");
        return ERR_LAST_ADDR_OUT_OF_BBOUNDS;
    }

    tvmon_eeprom.initd = true;
    tvmon_eeprom.curr_wrt_addr = curr_wrt_addr;
    tvmon_eeprom.wrap_arnd = (wrp_arnd_flg == WRAP_AROUND_FLAG) ? true : false;
    TVMON_DEBUG("tvmon_eeprom_init() \t\t ERR_NOERR!");
    return ERR_NOERR;
}
/**
 * @brief  Write byte to eeprom
 * 
 * @param data byte to be written
 * @return TVMON_ERR see tvmon_err.h
 */
TVMON_ERR tvmon_eeprom_wrt_byte(uint addr, char data)
{

    if (!eeprom.eeprom_write(addr, data))
    {
        TVMON_DEBUG("tvmon_eeprom_wrt_byte() \t\t ERR_EEPROM_WRT");
        return ERR_EEPROM_WRT;
    }
    else
    {
        TVMON_DEBUG("tvmon_eeprom_wrt_byte() \t\t ERR_NOERR ");
    }
    return ERR_NOERR;
}

/**
 * @brief Read byte from eeprom
 * 
 * @param data data buff
 * @return TVMON_ERR 
 */
TVMON_ERR tvmon_eeprom_rd_byte(uint16_t addr, uint8_t *data)
{
    eeprom.eeprom_read(addr, data);
    return ERR_NOERR;
}

/**
 * @brief Write tv usage data to eeprom
 * 
 * @param buff data buffer
 * @param len  length of the data
 * @return TVMON_ERR 
 */
TVMON_ERR tvmon_eeprom_wrt_tv_ud(usage_data_t *data)
{
    bool err;
    uint byte_len = sizeof(usage_data_t);
    uint nxt_curr_addr = byte_len + tvmon_eeprom.curr_wrt_addr;

    // Roll-over address if maximum is reached
    if (nxt_curr_addr + byte_len > MAX_WRT_ADDR)
    {
        tvmon_eeprom.curr_wrt_addr = USER_DATA_START_ADDR;
    }
    // Save the tv usage data and the next wrt address
    err = eeprom.eeprom_write((unsigned int)tvmon_eeprom.curr_wrt_addr, (uint8_t *)data, byte_len);
    err &= eeprom.eeprom_write(LAST_WRT_ADDR_ADDR_LSB, (uint8_t)(0x00ff & nxt_curr_addr));
    err &= eeprom.eeprom_write(LAST_WRT_ADDR_ADDR_MSB, (uint8_t)(nxt_curr_addr >> 8));
    if (!err)
    {
        TVMON_DEBUG("tvmon_eeprom_wrt_tv_ud() \t\t ERR_EEPROM_WRT \t eeprom.eeprom_write(LAST_WRT_ADDR_ADDR_MSB)");
        return ERR_EEPROM_WRT;
    }

    tvmon_eeprom.curr_wrt_addr = nxt_curr_addr;
    tvmon_eeprom.last_save_data = *data;

    return ERR_NOERR;
}

/**
 * @brief get usage data from eeprom
 * 
 * @param buff_ptr Pointer to buffer where data is to be stored
 * @param buff_length Length of the buffer
 * @param fdbk_cb     call-back function for feedback
 * @return TVMON_ERR 
 */
TVMON_ERR tvmon_eeprom_get_tv_ud_dump(uint16_t addr, uint8_t *buff_ptr, int buff_length)
{
    int8_t fdbk_buff[2];
    fdbk_buff[0] = 7; // 7 is the command/code for eeprom feedback. Bad coding but I had no time :(
    if (buff_length + addr > MAX_WRT_ADDR)
    {
        TVMON_DEBUG("tvmon_eeprom_get_tv_ud_dump() \t\t ERR_EEPROM_LEN_DATA");
    }
    int pcnt = 0;
    int num_iter = (buff_length) / 128;
    if (num_iter > 0)
    {
        for (int j = 0; j < num_iter; j++)
        {
             pcnt = j * 100 / num_iter;
            if (tvmon_eeprom.fdbk_cb && !(j%3))
            {
                fdbk_buff[1] = pcnt;
                tvmon_eeprom.fdbk_cb(fdbk_buff);
                delay(30);
            }

            if (!eeprom.eeprom_read(addr, buff_ptr, 128))
            {
                TVMON_DEBUG("tvmon_eeprom_get_tv_ud_dump() \t\t ERR_EEPROM_LEN_DATA"); // Read returns false if read lenght is less than len_byte
                return ERR_EEPROM_LEN_DATA;
            }
            buff_ptr += 128;
            addr += 128;
           
        }
        if (!eeprom.eeprom_read(addr, buff_ptr, buff_length % 128))
        {
            TVMON_DEBUG("tvmon_eeprom_get_tv_ud_dump() \t\t ERR_EEPROM_LEN_DATA"); // Read returns false if read lenght is less than len_byte
            return ERR_EEPROM_LEN_DATA;
        }
    }
    else
    {
        if (!eeprom.eeprom_read(addr, (uint8_t *)buff_ptr, buff_length))
        {
            TVMON_DEBUG("tvmon_eeprom_get_tv_ud_dump() \t\t ERR_EEPROM_LEN_DATA"); // Read returns false if read lenght is less than len_byte
            return ERR_EEPROM_LEN_DATA;
        }
    }
    TVMON_DEBUG("tvmon_eeprom_get_tv_ud_dump() \t\t ERR_NOERR");
    return ERR_NOERR;
}
/**
 * @brief Clear data from EEPROM module
 * 
 * @param start_addr 
 * @param len 
 * @return TVMON_ERR 
 */
TVMON_ERR tvmon_eeprom_clear_data(uint16_t start_addr, int len)
{
    int max_addr = start_addr + len - 1;
    uint8_t temp[128] = {0xff};
    if (max_addr > MAX_WRT_ADDR)
    {
        TVMON_DEBUG("tvmon_eeprom_clear_data() \t\t ERR_EEPROM_LEN_DATA \t\t max_addr>MAX_LEN_DATA");
        return ERR_EEPROM_LEN_DATA;
    }
    TVMON_DEBUG("Clearing EEPROM data...");
    for (uint i = start_addr; i < (uint)len; i += 128)
    {
        if (!eeprom.eeprom_write(i, temp, 128))
        {
            TVMON_DEBUG("tvmon_eeprom_clear_data() \t\tERR_EEPROM_WRT");
            return ERR_EEPROM_WRT;
        }
        else
        {
            TVMON_DEBUG("..." + String(i));
        }
    }

    TVMON_DEBUG("tvmon_eeprom_clear_data() \t\t ERR_NOERR");
    return ERR_NOERR;
}
