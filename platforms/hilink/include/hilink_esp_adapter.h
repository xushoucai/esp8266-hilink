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

#ifndef __HILINK_ADAPTER_H__
#define __HILINK_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_common.h"

/**
  * @brief      The hilink Wi-Fi event handler.
  *
  * @attention  No complex operations are allowed in callback.
  *             If users want to execute any complex operations, please post message to another task instead.
  *
  * @param      System_Event_t *event : WiFi event
  *
  * @return     null
  */
typedef void (* wifi_hilink_event_handler_cb_t)(System_Event_t* event);

/**
  * @brief  Register the Wi-Fi event handler.
  *
  * @param  wifi_event_handler_cb_t cb : callback function
  *
  * @return true  : succeed
  * @return false : fail
  */
void wifi_set_adapter_event_handler_cb(wifi_hilink_event_handler_cb_t cb);

/**
  * @brief  init hilink device info and service info.
  *
  * @param  dev_info_t* dev_info : device info
  * @param  dev_info_t* svc : service info
  * @param  int svc_num : service number
  *
  * @return true  : succeed
  * @return false : fail
  */
bool hilink_init_device_info(dev_info_t* dev_info, svc_info_t* svc, int svc_num);

/**
  * @brief  get hilink device info .
  *
  * @param  dev_info_t* dev_info : point to device info 
  *
  * @return none  
  */
dev_info_t* hilink_get_device_info(void);

#ifdef __cplusplus
}
#endif

#endif