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
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "json/cJSON.h"
#include "hilink_log.h"

int hilink_printf(const char* format, ...)
{
    va_list ap;
    int ret;

    va_start(ap, format);
    ret = vprintf(format, ap);
    va_end(ap);

    return ret;
}

void* hilink_memcpy(void* dst, const void* src, unsigned int len)
{
    return memcpy(dst, src, len);
}

int hilink_sec_get_Ac(unsigned char* pAc, unsigned int ulLen)
{
    unsigned char AC[48] = {0x34, 0x48, 0x51, 0x5d, 0x3a, 0x37, 0x3a, 0x59, 0x6a, 0x54, 0x56, 0x6e, 0x33, 0x63, 0x6f, 0x79, 0x24, 0xf7, 0x84, 0xa4, 0x87, 0xf6, 0x58, 0x47, 0xf5, 0x34, 0x30, 0x6d, 0x91, 0xac, 0xd5, 0x76, 0x2c, 0x67, 0x63, 0xc5, 0x9b, 0x82, 0xaf, 0xc8, 0xac, 0xf0, 0x4d, 0xa4, 0x90, 0x3d, 0x12, 0xe5};

    if (NULL == pAc) {
        hilink_printf("\n\r invalid PARAM\n\r");
        return -1;
    }

    hilink_memcpy(pAc, AC, 48);
    return 0;
}

/*******************************************************************************
** 网络状态及参数
*******************************************************************************/

/*
 * 网络状态
 * arguments:     state[out]         网络状态，0为断开或已连接但未分配ip，
                                 1为已连接且分配ip
 * return:
 */
int hilink_network_state(int* state)
{

    struct ip_info sta_info;

    if (false == wifi_get_ip_info(STATION_IF, &sta_info)) {
        *state = 0;
        return 0;
    } else if (sta_info.ip.addr == 0) {
        *state = 0;
    } else {
        *state = 1;
    }

    return 0;
}

int hilink_getssid(unsigned char* pssid, unsigned char* pssidlen)
{
    struct station_config sta_config;
    memset(&sta_config, 0, sizeof(sta_config));

    if (false == wifi_station_get_config(&sta_config)) {
        return -1;
    }

    memcpy(pssid, sta_config.ssid, 32);
    *pssidlen = strlen(pssid);
    return 0;
}

/*
 * 获取本机ip地址
 * arguments:     local_ip[out]        本机ip地址
                  ip_len[in]            ip地址长度
 * return:
 */
int hilink_get_local_ip(char* local_ip, unsigned char ip_len)
{
    struct ip_info sta_ip_info;
    wifi_get_ip_info(STATION_IF, &sta_ip_info);
    char ip_buf[16];
    memset(ip_buf, 0, 16);
    // #define inet_ntoa(addr)       ipaddr_ntoa((ip_addr_t*)&(addr))
    sprintf(ip_buf, "%s", inet_ntoa(sta_ip_info.ip));

    strncpy(local_ip, ip_buf, strlen(ip_buf));

    return 0;
}

/*
 * dns解析ip地址
 * arguments:     hostname    [IN] 远端主机名称。
 *                ip_list     [IN] 存放远端主机ip地址列表的数组。
 *                num         [IN] 存放远端主机ip地址列表的数组的大小，范围为[1，4]。
 * return: 0 成功， -1 失败
 */
int hilink_gethostbyname(char* hostname, char ip_list[][40], int num)
{
    struct in_addr sin_addr;

    if (!inet_aton(hostname, &sin_addr)) {
        struct hostent* hp;
        hilink_info("%s\n", hostname);
        hp = gethostbyname(hostname);

        if (!hp) {
            return -1;
        }

        int i = 0;

        for (i = 0; (hp->h_addr_list[i] != 0) && (i < num); i++) {
            hilink_info("tmp ip = %s\n", inet_ntoa(*((struct in_addr*)hp->h_addr_list[i])));
            memcpy(ip_list[i], inet_ntoa(*((struct in_addr*)hp->h_addr_list[i])),
                   strlen(inet_ntoa(*((struct in_addr*)hp->h_addr_list[i]))));
        }
    }

    return 0;
}

/*******************************************************************************
** 系统时间
*******************************************************************************/

/*
 * 获取当前时间
 * arguments:     ms[out]            以毫秒为单位
 * return:
 */
int hilink_gettime(unsigned long* ms)
{
    *ms = system_get_time() / 1000;
    return 0;
}

/*******************************************************************************
** 随机数
*******************************************************************************/
int hilink_rand(void)
{
    int ret;
    ret = abs(os_random());
    return ret;
}

void hilink_srand(unsigned int value)
{
//    return
    // os_rand(value);
}

/*******************************************************************************
** flash读写
*******************************************************************************/
int hilink_save_flash(char* buf, unsigned int len)
{
    uint32_t hilink_param_start_sec = 0;
    flash_size_map size_map = system_get_flash_size_map();

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            hilink_param_start_sec = 64 - 3;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_512_512:
            hilink_param_start_sec = 128 - 3;
            break;

        case FLASH_SIZE_16M_MAP_1024_1024:
        case FLASH_SIZE_32M_MAP_1024_1024:
            hilink_param_start_sec = 256 - 3;
            break;

        default:
            hilink_param_start_sec = 0;
            break;
    }

    if (false == system_param_save_with_protect(hilink_param_start_sec, buf, len)) {
        return -1;
    } else {
        return 0;
    }
}

int hilink_read_flash(char* buf, unsigned int len)
{
    //temp
    uint32_t hilink_param_start_sec = 0;
    flash_size_map size_map = system_get_flash_size_map();

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            hilink_param_start_sec = 64 - 3;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_512_512:
            hilink_param_start_sec = 128 - 3;
            break;

        case FLASH_SIZE_16M_MAP_1024_1024:
        case FLASH_SIZE_32M_MAP_1024_1024:
            hilink_param_start_sec = 256 - 3;
            break;

        default:
            hilink_param_start_sec = 0;
            break;
    }

    int i = 0;

    if (false == system_param_load(hilink_param_start_sec, 0, buf, len)) {
        return -1;
    } else {
        return 0;
    }
}

/*******************************************************************************
** 字符串操作
*******************************************************************************/
unsigned int hilink_strlen(const char* src)
{
    return (unsigned int)strlen(src);
}

char* hilink_strncpy(char* dst, const char* src, unsigned int len)
{
    return strncpy(dst, src, len);
}

char* hilink_strncat(char* dst, const char* src, unsigned int len)
{
    return strncat(dst, src, len);
}

int hilink_strncmp(const char* str1, const char* str2, unsigned int len)
{
    return strncmp(str1, str2, len);
}

char* hilink_strchr(char* str, int ch)
{
    return strchr(str, ch);
}

char* hilink_strrchr(const char* str, char c)
{
    return strrchr(str, c);
}

int hilink_atoi(const char* str)
{
    return atoi(str);
}

int hilink_snprintf(char* buf, unsigned int maxlen, const char* format, ...)
{
    va_list args;
    int ret;
    va_start(args, format);
    ret = vsnprintf(buf, maxlen, format, args);
    va_end(args);
    return ret;
}

int hilink_sprintf(char* buf, const char* format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsprintf(buf, format, args);
    va_end(args);

    return ret;
}

/*******************************************************************************
** 内存操作
*******************************************************************************/
void* hilink_memset(void* dst, int c, unsigned int len)
{
    return memset(dst, c, len);
}

int hilink_memcmp(const void* str1, const void* str2, unsigned int len)
{
    return memcmp(str1, str2, len);
}

void hilink_free(void* pt)
{
    free(pt);
}

/*******************************************************************************
 * 字节顺序交换
*******************************************************************************/
unsigned short hilink_htons(unsigned short hs)
{
    return htons(hs);
}

unsigned short hilink_ntohs(unsigned short ns)
{
    return ntohs(ns);
}

/*******************************************************************************
** json接口
*******************************************************************************/
void* hilink_json_parse(const char* value)
{

    return cJSON_Parse(value);
}

char* hilink_json_get_string_value(void* object, \
                                   char* name, unsigned int* len)
{
    cJSON* item;
    char* str = NULL;

    item = cJSON_GetObjectItem((cJSON*)object, name);

    if (item != NULL) {
        str = item->valuestring;
        *len = (unsigned int)strlen(str);
    }

    return str;
}

int hilink_json_get_number_value(void* object, \
                                 char* name, int* value)
{
    cJSON* item;
    int ret = -1;

    item = cJSON_GetObjectItem((cJSON*)object, name);

    if (item != NULL) {
        *value = item->valueint;
        ret = 0;
    }

    return ret;
}

void hilink_json_delete(void* object)
{
    cJSON_Delete((cJSON*)object);
}

// "A0FA503EBCB122EA7B49CAFC0C9B6452D9BC2F0F3CDE6E1EB539A784B61D7E89"\
// "714F75DF272F1D2F9B4FCB5340718841141D9E8CC35B400ACC71F914E8552EB5"\
// "1D55928BF73373B83124B0989003EDFC8616D71ACD704436F3515BE7C5D5F0D6"\
// "B567AB024CB3237B044B87B077BFE77D94CB5031C2F250A86A436CA9276D0CF1"\
// "F2FAE556E794A531674197A5E455CB2A5192D82A092DF0C5C8997B526C746CD1"\
// "986F06DEE818FF5AC7774A294BE15347F3B8028B1C6664169A84A0FC7753D828"\
// "372750FD328A06AB7AD9AE387B5B94BDF5C79930568FC94794EC4C119F8F6FBF"\
// "BCA57045D50FA2A02F14C0469105A3E900C9D8D48FE2C7F264B639F991A811B8";

int hilink_bi_get_cr(char* buf, unsigned int size)
{
    char* rsa_cipher =

        "9DAD77AC4E85BC39371D669035240F39EF82E6455EA961F4E679D9EB5611F87D"\
        "C5808C624AF4623EE62DE9E08CD089F83FDAADF4C7265266B92691A276D49955"\
        "49F124584A43AEE1C35B5C74A769306C1DD72BDE6CE91334CE6E003E4C440899"\
        "9520F9A88BCA2D784A677DD12573DB4B7E77A2E5FCC42E5FA4B0ACEF3F7EF617"\
        "DCE5F03C79CC84FD2BE5E8FB8FF9095220CE4F24C901C610F24E9E1AFA8A7C7B"\
        "F9FC08C1D3A0CCE464AB83685EA9855A2FD432396133BE31DB3D0814EAE64D93"\
        "53E7665306B1F33D2B7919B0DD63D342DB8928076403083474B8B493BBE26691"\
        "E66A680599D87538A0296E8A8EF2DE4D4A65EDBAAFA0C19BA5D8DE6D2E17FEFD";

    unsigned int cr_len = strlen(rsa_cipher) + 1;

    if (cr_len <= size) {
        memcpy(buf, rsa_cipher, cr_len);
        return 0;
    } else {
        return -1;
    }
}
