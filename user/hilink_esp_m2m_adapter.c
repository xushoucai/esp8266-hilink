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

typedef struct aircondition_info_t {
    bool switch_value; //switch value
    uint8_t mode;
    uint16_t temperature;
    uint8_t wind_direction;
} aircondition_info;

aircondition_info air_info = {1, 0, 0, 0};

// this a sample adapter,the client should adapter this API as HUAWEI profile base on specific device,this code is a example of airConditioner
int hilink_put_char_state(const char* svc_id, const char* payload, unsigned int len)
{
    printf("put svc_id is %s payload is %s\n", svc_id, payload);
    char* out;
    cJSON* json = NULL;
    cJSON* p_json = NULL;

    json = cJSON_Parse(payload);

    if (strncmp(svc_id, "switch", strlen("switch")) == 0) {
        if ((p_json = cJSON_GetObjectItem(json, "on")) != NULL) {
            air_info.switch_value = p_json->valueint;
            printf(" switch value %d\n", p_json->valueint);

        }
    } else if (strncmp(svc_id, "airConditioner", strlen("airConditioner")) == 0) {
        if ((p_json = cJSON_GetObjectItem(json, "mode")) != NULL) {
            air_info.mode = p_json->valueint;
            printf(" mode value %d\n", p_json->valueint);
        }
    } else if (strncmp(svc_id, "temperature", strlen("temperature")) == 0) {
        if ((p_json = cJSON_GetObjectItem(json, "currentTemperture")) != NULL) {
            air_info.temperature = p_json->valueint;
            printf(" currentTemperturen value %d\n", p_json->valueint);
        }
    } else if (strncmp(svc_id, "wind", strlen("wind")) == 0) {
        if ((p_json = cJSON_GetObjectItem(json, "windDirection")) != NULL) {
            air_info.wind_direction = p_json->valueint;
            printf(" windDirection value %d\n", p_json->valueint);

        }
    } else if (strncmp(svc_id, "devOta", strlen("devOta")) == 0) {
        if ((p_json = cJSON_GetObjectItem(json, "action")) != NULL) {
            printf("ota trig value %d\n", p_json->valueint);
            hilink_ota_trig(p_json->valueint);
        }
    }

    return 0;
}

// this a sample adapter,the client should adapter this API as HUAWEI profile base on specific device,this code is a example of airConditioner
int hilink_get_char_state(const char* svc_id, const char* in, unsigned int in_len, char** out, unsigned int* out_len)
{
    printf("get svc_id %s in %p %s\n", svc_id, in, in);
    *out = (char*)os_zalloc(128);

    if (strncmp(svc_id, "switch", strlen("switch")) == 0) {
        sprintf(*out, "{\"on\":%d,\"name\":\"switch\"}", air_info.switch_value);
        *out_len = strlen(*out);
    } else if (strncmp(svc_id, "airConditioner", strlen("airConditioner")) == 0) {
        sprintf(*out, "{\"mode\":%d}", air_info.mode);
        printf("out %s\n", *out);
        *out_len = strlen(*out);
    } else if (strncmp(svc_id, "temperature", strlen("temperature")) == 0) {
        sprintf(*out, "{\"currentTemperture\":%d}", air_info.temperature);
        printf("out %s\n", *out);
        *out_len = strlen(*out);
    } else if (strncmp(svc_id, "wind", strlen("wind")) == 0) {
        sprintf(*out, "{\"windDirection\":%d}", air_info.wind_direction);
        printf("out %s\n", *out);
        *out_len = strlen(*out);
    }  else if (strncmp(svc_id, "faultDetection", strlen("faultDetection")) == 0) {
        sprintf(*out, "{\"status\":0,\"code\":0}");
        printf("out %s\n", *out);
        *out_len = strlen(*out);
    } else if (strncmp(svc_id, "devOta", strlen("devOta")) == 0) {
        return 0;
    }

    return 0;
}

int hilink_notify_devstatus(int status)
{
    return 0;
}

