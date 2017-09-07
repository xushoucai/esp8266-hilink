/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "hilink_link.h"
#include "hilink_profile.h"
#include "hilink_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_common.h"

#define SNSTR "%02x%02x%02x%02x%02x%02x"

char hilink_dev_mac[32]; //must be global parameter
char hilink_dev_sn[32]; //must be global parameter

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());

    memset(hilink_dev_mac, 0, 32);
    memset(hilink_dev_sn, 0, 32);
    char esp_mac_get[6] = {0};
    wifi_get_macaddr(STATION_IF, esp_mac_get);
    sprintf(hilink_dev_mac, MACSTR, MAC2STR(esp_mac_get));
    sprintf(hilink_dev_sn, SNSTR, MAC2STR(esp_mac_get));

    dev_info_t product_aircon = {
        .sn = hilink_dev_sn,                  /**<设备唯一标识，比如sn号，长度范围（0,40]*/
        .prodId = "9001",                     /**<设备HiLink认证号，长度范围（0,5]*/
        .model = "airCondition",              /**<设备型号，长度范围（0,32]*/
        .dev_t = "012",                       /**<设备类型，长度范围（0,4]*/
        .manu = "001",                        /**<设备制造商，长度范围（0,4]*/
        .mac = hilink_dev_mac,                /**<设备MAC地址，固定32字节*/
        .hiv = "1.0",                       /**<设备Hilink协议版本，长度范围（0,32]*/
        .fwv = "1.30",                       /**<设备固件版本，长度范围[0,64]*/
        .hwv = "20000",                       /**<设备硬件版本，长度范围[0,64]*/
        .swv = "1.0.3",                       /**<设备软件版本，长度范围[0,64] it's HILINK SDK version,now use hilink sdk 1.0.3*/
        .prot_t = 1,                          /**<设备协议类型，取值范围[1,3]*/
    };

    /*init service info as huawei profile*/
    svc_info_t svc_aircon[6] = {
        {"binarySwitch", "switch"},
        {"airConditioner", "airConditioner"},
        {"temperature", "temperature"},
        {"wind", "wind"},
        {"devOta", "devOta"},
        {"faultDetection", "faultDetection"}
    };

    hilink_init_device_info(&product_aircon, &svc_aircon, 6);

    hilink_esp_smtlink_main();
}

