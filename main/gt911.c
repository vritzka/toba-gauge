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

#include <esp_log.h>

#include <lvgl.h>

#include "gt911.h"

#include "i2c_manager.h"

#define TAG "GT911"

gt911_status_t gt911_status;

#define CONFIG_LV_I2C_TOUCH_PORT 0

//TODO: handle multibyte read and refactor to just one read transaction
esp_err_t gt911_i2c_read(uint8_t slave_addr, uint16_t register_addr, uint8_t *data_buf, uint8_t len) {
    return i2c_manager_read(CONFIG_LV_I2C_TOUCH_PORT, slave_addr, register_addr | I2C_REG_16, data_buf, len);
}

esp_err_t gt911_i2c_write8(uint8_t slave_addr, uint16_t register_addr, uint8_t data) {
    uint8_t buffer = data;
    return i2c_manager_write(CONFIG_LV_I2C_TOUCH_PORT, slave_addr, register_addr | I2C_REG_16, &buffer, 1);
}


void gt911_read_config() {
    
        uint8_t data_buf;
    
        // Read full config from 0x8047 - 0x80FE
        for (int i = 0; i < GOODIX_CONFIG_911_LENGTH; i++) {
            gt911_i2c_read(GT911_I2C_SLAVE_ADDR, (GOODIX_REG_CONFIG_DATA + i), &data_buf, 1);
            gt911_status.product_config[i] = data_buf;
        }
        
        return;
    
} 


uint16_t gt911_calc_checksum() {
    
        uint16_t checkSum = 0x00;
    
        // add up all bits in config to get the checksum
        for (int i = 0; i < GOODIX_CONFIG_911_LENGTH; i++) {            
            checkSum = checkSum + gt911_status.product_config[i];
            //ESP_LOGI(TAG, "\tConfig: %d Register 0x%02x = 0x%02x", i, (GOODIX_REG_CONFIG_DATA + i), gt911_status.product_config[i]);
        }
        
        checkSum = (~checkSum) + 1;
        
        return checkSum;
}

void gt911_set_resolution(uint16_t width, uint16_t height) {
        gt911_status.product_config[1] = (width & 0xff);
        gt911_status.product_config[2] = (width >> 8);
        gt911_status.product_config[3] = (height & 0xff);
        gt911_status.product_config[4] = (height >> 8);
        
        gt911_save_config();
        
}

void gt911_save_config() {
    
    for (int i = 0; i < GOODIX_CONFIG_911_LENGTH; i++) {            
        gt911_i2c_write8(GT911_I2C_SLAVE_ADDR, (GOODIX_REG_CONFIG_DATA + i), gt911_status.product_config[i]);
    }
    //Checksum
    gt911_i2c_write8(GT911_I2C_SLAVE_ADDR, 0x80FF, gt911_calc_checksum());
    //config fresh
    gt911_i2c_write8(GT911_I2C_SLAVE_ADDR, 0x8100, 1);

}



void gt911_init(uint8_t dev_addr) {
    if (!gt911_status.inited) {
        gt911_status.i2c_dev_addr = dev_addr;
        uint8_t data_buf;
        esp_err_t ret;

        ESP_LOGI(TAG, "Checking for GT911 Touch Controller");
        if ((ret = gt911_i2c_read(dev_addr, GT911_PRODUCT_ID1, &data_buf, 1) != ESP_OK)) {
            ESP_LOGE(TAG, "Error reading from device: %s",
                        esp_err_to_name(ret));    // Only show error the first time
            return;
        }

        // Read 4 bytes for Product ID in ASCII
        for (int i = 0; i < GT911_PRODUCT_ID_LEN; i++) {
            gt911_i2c_read(dev_addr, (GT911_PRODUCT_ID1 + i), (uint8_t *)&(gt911_status.product_id[i]), 1);
        }
        
        ESP_LOGI(TAG, "\tProduct ID: %s", gt911_status.product_id);
        
        gt911_i2c_read(dev_addr, GT911_VENDOR_ID, &data_buf, 1);
        ESP_LOGI(TAG, "\tVendor ID: 0x%02x", data_buf);

        gt911_i2c_read(dev_addr, GT911_X_COORD_RES_L, &data_buf, 1);
        gt911_status.max_x_coord = data_buf;
        gt911_i2c_read(dev_addr, GT911_X_COORD_RES_H, &data_buf, 1);
        gt911_status.max_x_coord |= ((uint16_t)data_buf << 8);
        ESP_LOGI(TAG, "\tX Resolution: %d", gt911_status.max_x_coord);

        gt911_i2c_read(dev_addr, GT911_Y_COORD_RES_L, &data_buf, 1);
        gt911_status.max_y_coord = data_buf;
        gt911_i2c_read(dev_addr, GT911_Y_COORD_RES_H, &data_buf, 1);
        gt911_status.max_y_coord |= ((uint16_t)data_buf << 8);
        ESP_LOGI(TAG, "\tY Resolution: %d", gt911_status.max_y_coord);
        gt911_status.inited = true;
    }
}

/**
  * @brief  Get the touch screen X and Y positions values. Ignores multi touch
  * @param  drv:
  * @param  data: Store data here
  * @retval Always false
  */
bool gt911_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {\
    
    uint8_t touch_pnt_cnt;        // Number of detected touch points
    static int16_t last_x = 0;  // 12bit pixel value
    static int16_t last_y = 0;  // 12bit pixel value
    uint8_t data_buf;
    uint8_t status_reg;
    uint8_t buffer_status;

    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_STATUS_REG, &status_reg, 1);
    
    ESP_LOGI(TAG, "\tstatus: 0x%02x", status_reg);
    touch_pnt_cnt = status_reg & 0x0F;

    buffer_status = status_reg >> 7;
    
    ESP_LOGI(TAG, "\tBuffer Status %d", buffer_status );
    
    if(buffer_status == 1) {
        gt911_i2c_write8(gt911_status.i2c_dev_addr, GT911_STATUS_REG, 0x00);
    }
    
    ESP_LOGI(TAG, "\tTouch Point count %d", touch_pnt_cnt );
    
    if (touch_pnt_cnt > 1) {    // ignore multi touch
        data->state = LV_INDEV_STATE_RELEASED;
        return false;
    }
    
    if (touch_pnt_cnt == 0) { //ignore no touch
        data->state = LV_INDEV_STATE_RELEASED;
        return false;
    }
    
//    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_TRACK_ID1, &data_buf, 1);
//    ESP_LOGI(TAG, "\ttrack_id: %d", data_buf);

    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_PT1_X_COORD_L, &data_buf, 1);
    last_x = data_buf;
    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_PT1_X_COORD_H, &data_buf, 1);
    last_x |= ((uint16_t)data_buf << 8);

    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_PT1_Y_COORD_L, &data_buf, 1);
    last_y = data_buf;
    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_PT1_Y_COORD_H, &data_buf, 1);
    last_y |= ((uint16_t)data_buf << 8);


    data->point.x = last_x;
    data->point.y = last_y;
    data->state = LV_INDEV_STATE_PRESSED;
    ESP_LOGI(TAG, "X=%u Y=%u", data->point.x, data->point.y);
    ESP_LOGV(TAG, "X=%u Y=%u", data->point.x, data->point.y);
    
    gt911_i2c_write8(gt911_status.i2c_dev_addr, GT911_STATUS_REG, 0x00);
    return false;
}
