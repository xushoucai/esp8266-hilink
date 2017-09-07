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

#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "json/cJSON.h"
#include "hilink_profile.h"
#include "hilink_log.h"

xTaskHandle esp_m2m_handle = NULL;
dev_info_t g_dev_info;
svc_info_t g_svc_info[HILINK_M2M_MAX_SVC_NUM];
int g_svc_num;

int hilink_notify_wifi_param(char* ssid, unsigned int ssid_len,
                             char* pwd, unsigned int pwd_len, int mode)
{
    hilink_info("mode %d ssid %s pwd %s\n", mode, ssid, pwd);

    if ((ssid == NULL) && (pwd == NULL) && (mode == -1)) {
        system_restore();

        system_restart();
        return 0;
    } else if ((ssid == NULL) && (pwd == NULL) && (mode != -1)) {
        return 0;
    } else {
        struct station_config sta_conf;
        memset(&sta_conf, 0, sizeof(sta_conf));
        memcpy(sta_conf.ssid, ssid, ssid_len);
        memcpy(sta_conf.password, pwd, pwd_len);

        wifi_station_set_config(&sta_conf);
        wifi_station_disconnect();
        wifi_station_connect();
    }

    return 0;

}

void hilink_m2m_task(void* pData)
{
    int ret = 0;
    ret = hilink_m2m_init(&g_dev_info, (svc_info_t*)&g_svc_info, g_svc_num);    //\B3\F5ʼ\BB\AFHilink Device SDK
#ifdef ENABLE_NEW_HILINK
    hilink_m2m_set_taskid(0);
#endif

    while (1) {
        //\C8\E7\B9\FB\D3\D0\C5\E4\CD\F8ָ\C1\B9\FD\C0\B4\A3\AC\CF\C8ֹͣ\B5\B1ǰ\C8\CE\CE\F1
        //\C6\F4\B6\AFHilink Device SDK\BF\AAʼ״̬\BB\FA\B9\DC\C0\ED,\B3\D6\D0\F8\BB\F1Hilink Device SDK\D4\CB\D0\D0״̬
        ret = hilink_m2m_process();

        if (ret != 0) {

            hilink_error("process return %d\n", ret);
        }

        vTaskDelay(50 / portTICK_RATE_MS);

    }

}

bool hilink_init_device_info(dev_info_t* dev_info, svc_info_t* svc_info, uint8_t svc_num)
{
    uint8_t index = 0;
    
    if (!dev_info || !svc_info || svc_num > HILINK_M2M_MAX_SVC_NUM || svc_num == 0) {
        hilink_error("error arg");
        return false;
    }

    if (strlen(dev_info->sn) > 40     ||
            strlen(dev_info->prodId) > 5  ||
            strlen(dev_info->model) > 32  ||
            strlen(dev_info->dev_t) > 4   ||
            strlen(dev_info->manu) > 4    ||
            strlen(dev_info->mac) > 17    ||
            strlen(dev_info->hiv) > 32    ||
            strlen(dev_info->fwv) > 64    ||
            strlen(dev_info->hwv) > 64    ||
            strlen(dev_info->swv) > 64    ||
            dev_info->prot_t > 3) {
        hilink_error("error dev_info");
        return false;
    }

    for (index = 0; index < svc_num; index++) {
        if (strlen(svc_info[index].st) > 32 ||
                strlen(svc_info[index].svc_id) > 64) {
            hilink_error("error svc_info");
            return false;
        }
    }

    g_svc_num = svc_num;
    memcpy(&g_dev_info, dev_info, sizeof(dev_info_t));
    memcpy(&g_svc_info, svc_info, sizeof(svc_info_t) * svc_num);
    return true;
}

dev_info_t* hilink_get_device_info(void)
{
    return &g_dev_info;
}


/** This is hilink m2m entry function*/
int hilink_esp_m2m_main(void)
{
    if (!esp_m2m_handle) {
        return xTaskCreate(hilink_m2m_task, "hilink_m2m_task", 2048, NULL, 3, &esp_m2m_handle);
    } else {
        return 0;
    }
}

