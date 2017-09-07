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
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "upgrade.h"
#include "hilink_link.h"
#include "hilink_log.h"
#include "json/cJSON.h"
#include "hilink_profile.h"
#include "hilink_ota_config.h"
#include "hilink_esp_adapter.h"

/*********************global param define start ******************************/
bool latest_version_flag = false;
static char ota_server_version[16] = {0};
static os_timer_t upgrade_timer;
static uint32_t totallength = 0;
static uint32_t sumlength = 0;
static int percent_temp = 0;
static bool flash_erased = false;
static xTaskHandle* ota_task_handle = NULL;
/*********************global param define end *******************************/

/******************************************************************************
 * FunctionName : upgrade_recycle
 * Description  : recyle upgrade task, if OTA finish switch to run another bin
 * Parameters   :
 * Returns      : none
*******************************************************************************/
static void hilink_upgrade_recycle(void)
{
    totallength = 0;
    sumlength = 0;
    percent_temp = 0;
    flash_erased = false;
    bzero(ota_server_version, 16);
    system_upgrade_deinit();
    printf("[hilink_upgrade_recycle] : system_upgrade_flag_check()=0x%d\n", system_upgrade_flag_check());

    //if OTA success ,reboot to new image and run it,otherwise the APP will notify client the OTA task failed ,
    //the device will delete the ota task,and need client restart the OTA task through Hilink APP

    if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
        vTaskDelay(100 / portTICK_RATE_MS);
        ota_task_handle = NULL;
        system_upgrade_reboot();
    } else {
        vTaskDelete(ota_task_handle);
        ota_task_handle = NULL;
    }
}

/******************************************************************************
 * FunctionName : upgrade_download
 * Description  : parse http response ,and download remote data and write in flash
 * Parameters   : char *pusrdata : remote data
 *                length         : data length
 * Returns      : int
*******************************************************************************/
static int upgrade_download(char* pusrdata, unsigned short length)
{
    char* ptr = NULL;
    char* ptmp2 = NULL;
    char lengthbuffer[32];

    if (totallength == 0 && (ptr = (char*)strstr(pusrdata, "\r\n\r\n")) != NULL &&
            (ptr = (char*)strstr(pusrdata, "Content-Length")) != NULL) {
        ptr = (char*)strstr(pusrdata, "\r\n\r\n");
        length -= ptr - pusrdata;
        length -= 4;
        os_printf("upgrade file download start.\n");
        //parser the http head
        ptr = (char*)strstr(pusrdata, "Content-Length: ");

        if (ptr != NULL) {
            ptr += 16;
            ptmp2 = (char*)strstr(ptr, "\r\n");

            if (ptmp2 != NULL) {
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                memcpy(lengthbuffer, ptr, ptmp2 - ptr);
                sumlength = atoi(lengthbuffer);
                printf("sumlength = %d\n", sumlength);

                if (sumlength > 0) {
                    if (false == system_upgrade(pusrdata, sumlength)) {
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                        return -1;
                    }

                    flash_erased = true;
                    ptr = (char*)strstr(pusrdata, "\r\n\r\n");

                    if (false == system_upgrade(ptr + 4, length)) {
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                        return -1;
                    }

                    totallength += length;
                    printf("totallen = %d\n", totallength);
                    return 0;
                }
            } else {
                printf("sumlength failed\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                return -1;
            }
        } else {

            printf("Content-Length: failed\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            return -1;
        }
    } else if (sumlength != 0) {

        totallength += length;

        if (false == system_upgrade(pusrdata, length)) {
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            return -1;
        }

        // need upload progress when download the image,and when OTA is finished,
        // delay some time wait the 100% progress is upload successful then reboot
        int percent_u = (totallength * 20) / sumlength;

        if (percent_temp != percent_u) {
            if (0 != hilink_ota_rpt_prg(percent_u * 5, 120)) {
                return -1;
            }

            percent_temp = percent_u;
        }

        if (percent_temp == 20) {
            if (0 != hilink_ota_rpt_prg(100, 120)) {
                return -1;
            }

            percent_temp = 0;
        }

        if (totallength == sumlength) {
            printf("upgrade file download finished.\n");

            if (upgrade_crc_check(system_get_fw_start_sec(), sumlength) != true) {
                printf("upgrade crc check failed !\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                return -1;
            }

            system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
            return 0;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

static void hilink_ota_start(void* param)
{
    int recbytes;
    int sin_size;
    int sta_socket;
    char recv_buf[1460] = {0};

    struct sockaddr_in remote_ip;
    printf("Hello, welcome to client!\r\n");

    system_upgrade_flag_set(UPGRADE_FLAG_START);
    system_upgrade_init();

    sta_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (-1 == sta_socket) {
        close(sta_socket);
        printf("socket fail !\r\n");
        vTaskDelete(NULL);
    }

    printf("socket ok!\r\n");
    bzero(&remote_ip, sizeof(struct sockaddr_in));

    remote_ip.sin_family = AF_INET;
    remote_ip.sin_addr.s_addr = inet_addr(OTA_SERVER_ADDR);

    remote_ip.sin_port = htons(OTA_SERVER_PORT);

    if (0 != connect(sta_socket, (struct sockaddr*)(&remote_ip), sizeof(struct sockaddr))) {
        printf("connect fail!\r\n");
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
        goto recycle;
    }

    printf("connect ok!\r\n");
    char pbuf[1024];
    bzero(pbuf, 1024);
    char head_pbuf[512];
    bzero(head_pbuf, 512);

    sprintf(head_pbuf, pheadbuffer, MASTER_KEY);
    sprintf(pbuf, Download_request_url, ota_server_version, BIN_FILENAME, OTA_SERVER_ADDR, OTA_SERVER_PORT, head_pbuf);

    //send this packet to iot.espresif.cn for downloading image
    if (write(sta_socket, pbuf, strlen(pbuf)) < 0) {
        goto recycle;
    }

    while ((system_upgrade_flag_check() != UPGRADE_FLAG_FINISH) &&
            ((recbytes = read(sta_socket, recv_buf, 1460)) > 0)) {
        if (0 != upgrade_download(recv_buf, recbytes)) {
            goto recycle;
        }
    }

    if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
        close(sta_socket);
        hilink_upgrade_recycle();
        vTaskDelete(NULL);
    }

    if (recbytes <= 0) {
        printf("read data fail!\r\n");
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
        goto recycle;
    }

recycle:
    close(sta_socket);
    hilink_upgrade_recycle();
    vTaskDelete(NULL);
}

static int get_latest_version(void)
{
    int sta_socket;
    struct sockaddr_in remote_ip;
    char recv_buf[1024];
    int recbytes;
    cJSON* root = NULL;
    cJSON* p_json = NULL;
    sta_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (-1 == sta_socket) {
        close(sta_socket);
        printf("socket fail !\r\n");
        return -1;
    }

    printf("socket ok!\r\n");
    bzero(&remote_ip, sizeof(struct sockaddr_in));

    remote_ip.sin_family = AF_INET;
    remote_ip.sin_addr.s_addr = inet_addr(OTA_SERVER_ADDR);

    remote_ip.sin_port = htons(OTA_SERVER_PORT);

    if (0 != connect(sta_socket, (struct sockaddr*)(&remote_ip), sizeof(struct sockaddr))) {
        goto recycle;
    }

    printf("connect OK\n");
    char pbuf[1024];
    bzero(pbuf, 1024);
    char head_pbuf[512];
    bzero(head_pbuf, 512);
    sprintf(head_pbuf, pheadbuffer, MASTER_KEY);
    sprintf(pbuf, Query_version_url, OTA_SERVER_ADDR, OTA_SERVER_PORT, head_pbuf);

    //send this packet to iot.espresif.cn for getting latest image version
    if (write(sta_socket, pbuf, strlen(pbuf)) < 0) {
        goto recycle;
    }

    while (1) {
        bzero(recv_buf, 1024);
        recbytes = read(sta_socket , recv_buf, 1024);

        if (recbytes <= 0) {
            goto recycle;
        }

        char* ptr = strstr(recv_buf, "\r\n\r\n");

        if (ptr != NULL) {
            root = cJSON_Parse(ptr + 4);

            if (!root) {
                goto recycle;
            }

            p_json = cJSON_GetObjectItem(root, "recommended_rom_version");

            if (!p_json) {
                goto recycle;
            } else {
                memcpy(ota_server_version, p_json->valuestring, strlen(p_json->valuestring));

                //if get the latest image version,need upload old version then upload
                // new version, don't know why,need ask HUAWEI
                if (0 != hilink_ota_rpt_ver((char* )hilink_get_device_info()->fwv, strlen(hilink_get_device_info()->fwv), NULL, 0)) {
                    goto recycle;
                }

                if (strncmp(hilink_get_device_info()->fwv, ota_server_version, strlen(ota_server_version)) !=  0) {
                    if (0 != hilink_ota_rpt_ver(ota_server_version, strlen(ota_server_version), NULL, 0)) {
                        goto recycle;
                    }
                }

                close(sta_socket);
            }

            return 0;
        } else {
            goto recycle;
        }
    }

recycle:
    close(sta_socket);
    return -1;
}

int hilink_ota_get_ver(char** version, int* ver_len)
{
    if (0 != get_latest_version()) {
        return -902;
    } else if (memcmp(ota_server_version, hilink_get_device_info()->fwv, strlen(ota_server_version)) == 0) {
        *version = NULL;
        *ver_len = 0;
        latest_version_flag = true;
    } else {
        *version = (char*)os_zalloc(16);
        sprintf(*version, "%s", ota_server_version);
        *ver_len = strlen(ota_server_version);
        latest_version_flag = false;
    }

    printf("ota_server_version %s \n", ota_server_version);
    return 0;
}

int hilink_ota_get_intro(char** introduction, int* intro_len)
{
    if (latest_version_flag == true) {
        *introduction = NULL;
        *intro_len = 0;
    } else {
        *introduction = (char*)os_zalloc(32);
        sprintf(*introduction, "%s", "default introduction");
        *intro_len = strlen("default introduction");
    }

    return 0;
}

int hilink_ota_trig(int mode)
{
    //attention :if the mode is 1 ,this API need return immediately,can not be blocked,hilink_ota_rpt_prg
    //should upload progress after hilink_ota_trig return.
    char temp_cmp[16] = {0};

    if (mode == 0) {
        if (memcmp(ota_server_version, temp_cmp, 16) == 0) {
            if (0 != get_latest_version()) {
                return -900;
            }
        }

        if (0 != hilink_ota_rpt_ver(ota_server_version, strlen(ota_server_version), NULL, 0)) {
            return -900;
        } else {
            return 0;
        }

    } else if (mode == 1) {
        if (memcmp(ota_server_version, temp_cmp, 16) == 0) {
            return -12;
        }

        if (!ota_task_handle) {
            xTaskCreate(hilink_ota_start, "hilink_ota_start_task", 1024, NULL, 2, ota_task_handle);
            os_timer_disarm(&upgrade_timer);
            os_timer_setfn(&upgrade_timer, (os_timer_func_t*)hilink_upgrade_recycle, NULL);
            os_timer_arm(&upgrade_timer, OTA_TIMEOUT, 0);
        } else {
            printf("ota task already start\n");
        }

        return 0;
    } else {
        return -12;
    }

    return 0;
}
