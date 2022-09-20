#ifndef __GT911_H
/*
* Copyright © 2021 Sturnus Inc.

* Permission is hereby granted, free of charge, to any person obtaining a copy of this 
* software and associated documentation files (the “Software”), to deal in the Software 
* without restriction, including without limitation the rights to use, copy, modify, merge, 
* publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
* to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or 
* substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
* SOFTWARE.
*/

#define __GT911_H

#include <stdint.h>
#include <stdbool.h>
/*
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
*/
#ifdef __cplusplus
extern "C" {
#endif

#define GT911_I2C_SLAVE_ADDR   0x5D

#define GT911_PRODUCT_ID_LEN   4

/* Register Map of GT911 */
#define GT911_PRODUCT_ID1             0x8140
#define GT911_PRODUCT_ID2             0x8141
#define GT911_PRODUCT_ID3             0x8142
#define GT911_PRODUCT_ID4             0x8143
#define GT911_FIRMWARE_VER_L          0x8144
#define GT911_FIRMWARE_VER_H          0x8145
#define GT911_X_COORD_RES_L           0x8146
#define GT911_X_COORD_RES_H           0x8147
#define GT911_Y_COORD_RES_L           0x8148
#define GT911_Y_COORD_RES_H           0x8149

#define GT911_X_OUTPUT_MAX_LOW         0x8048
#define GT911_X_OUTPUT_MAX_HIGH        0x8049
#define GT911_Y_OUTPUT_MAX_LOW         0x804A
#define GT911_Y_OUTPUT_MAX_HIGH        0x804B
        
#define GT911_VENDOR_ID               0x814A

#define GT911_STATUS_REG              0x814E
#define   GT911_STATUS_REG_BUF        0x80
#define   GT911_STATUS_REG_LARGE      0x40
#define   GT911_STATUS_REG_PROX_VALID 0x20
#define   GT911_STATUS_REG_HAVEKEY    0x10
#define   GT911_STATUS_REG_PT_MASK    0x0F

#define GT911_TRACK_ID1               0x814F
#define GT911_PT1_X_COORD_L           0x8150
#define GT911_PT1_X_COORD_H           0x8151
#define GT911_PT1_Y_COORD_L           0x8152
#define GT911_PT1_Y_COORD_H           0x8153
#define GT911_PT1_X_SIZE_L            0x8154
#define GT911_PT1_X_SIZE_H            0x8155


#define GOODIX_OK   0

// 0x28/0x29 (0x14 7bit)
#define GOODIX_I2C_ADDR_28  0x14
// 0xBA/0xBB (0x5D 7bit)
#define GOODIX_I2C_ADDR_BA  0x5D

#define GOODIX_MAX_HEIGHT   4096
#define GOODIX_MAX_WIDTH    4096
#define GOODIX_INT_TRIGGER    1
#define GOODIX_CONTACT_SIZE   8
#define GOODIX_MAX_CONTACTS   5

#define GOODIX_CONFIG_MAX_LENGTH  240
//#define GOODIX_CONFIG_911_LENGTH  186
#define GOODIX_CONFIG_911_LENGTH  (0x80FE - 0x8047) + 1

/* Register defines */
#define GT_REG_CMD  0x8040

#define GT_REG_CFG  0x8047
#define GT_REG_DATA 0x8140


// Write only registers
#define GOODIX_REG_COMMAND        0x8040
#define GOODIX_REG_LED_CONTROL    0x8041
#define GOODIX_REG_PROXIMITY_EN   0x8042

// Read/write registers
// The version number of the configuration file
#define GOODIX_REG_CONFIG_DATA     0x8047
// X output maximum value (LSB 2 bytes)
#define GOODIX_REG_MAX_X        0x8048
// Y output maximum value (LSB 2 bytes)
#define GOODIX_REG_MAX_Y        0x804A
// Maximum number of output contacts: 1~5 (4 bit value 3:0, 7:4 is reserved)
#define GOODIX_REG_MAX_TOUCH    0x804C

// Module switch 1
// 7:6 Reserved, 5:4 Stretch rank, 3 X2Y, 2 SITO (Single sided ITO touch screen), 1:0 INT Trigger mode */
#define GOODIX_REG_MOD_SW1      0x804D
// Module switch 2
// 7:1 Reserved, 0 Touch key */
#define GOODIX_REG_MOD_SW2      0x804E

// Number of debuffs fingers press/release
#define GOODIX_REG_SHAKE_CNT    0x804F



// ReadOnly registers (device and coordinates info)
// Product ID (LSB 4 bytes, GT9110: 0x06 0x00 0x00 0x09)
#define GOODIX_REG_ID           0x8140
// Firmware version (LSB 2 bytes)
#define GOODIX_REG_FW_VER       0x8144

// Current output X resolution (LSB 2 bytes)
#define GOODIX_READ_X_RES       0x8146
// Current output Y resolution (LSB 2 bytes)
#define GOODIX_READ_Y_RES       0x8148
// Module vendor ID
#define GOODIX_READ_VENDOR_ID   0x814A

#define GOODIX_READ_COORD_ADDR  0x814E

/* Commands for REG_COMMAND */
//0: read coordinate state
#define GOODIX_CMD_READ         0x00
// 1: difference value original value
#define GOODIX_CMD_DIFFVAL      0x01
// 2: software reset
#define GOODIX_CMD_SOFTRESET    0x02
// 3: Baseline update
#define GOODIX_CMD_BASEUPDATE   0x03
// 4: Benchmark calibration
#define GOODIX_CMD_CALIBRATE    0x04
// 5: Off screen (send other invalid)
#define GOODIX_CMD_SCREEN_OFF   0x05

/* When data needs to be sent, the host sends command 0x21 to GT9x,
 * enabling GT911 to enter "Approach mode" and work as a transmitting terminal */
#define GOODIX_CMD_HOTKNOT_TX   0x21

#define RESOLUTION_LOC    1
#define MAX_CONTACTS_LOC  5
#define TRIGGER_LOC 6 


typedef struct {
    bool inited;
    char product_id[GT911_PRODUCT_ID_LEN];
    char product_config[GOODIX_CONFIG_911_LENGTH];
    uint16_t max_x_coord;
    uint16_t max_y_coord;
    uint8_t i2c_dev_addr;
} gt911_status_t;

/**
  * @brief  Initialize for GT911 communication via I2C
  * @param  dev_addr: Device address on communication Bus (I2C slave address of GT911).
  * @retval None
  */
void gt911_init(uint8_t dev_addr);

/**
  * @brief  Get the touch screen X and Y positions values. Ignores multi touch
  * @param  drv:
  * @param  data: Store data here
  * @retval Always false
  */
bool gt911_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

uint16_t calcChecksum();
void gt911_read_config();
void gt911_save_config();
void gt911_set_resolution(uint16_t width, uint16_t height);




#ifdef __cplusplus
}
#endif
#endif /* __GT911_H */
