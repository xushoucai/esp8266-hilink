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

#ifndef __HILINK_OTA_H__
#define __HILINK_OTA_H__

#define OTA_SERVER_ADDR "115.29.202.58"
#define OTA_SERVER_PORT 80
#define BIN_FILENAME "user.bin"
#define OTA_TIMEOUT 60000

// #define MASTER_KEY "f9ce302c83d6471cdb85c156fa4cd336ff10b7e5"
#define MASTER_KEY "eaff337b3468470a12c0feea6604fa349853764c"
#define Download_request_url "GET /v1/device/rom/?action=download_rom&version=%s&filename=%s HTTP/1.0\r\n\
Host:%s:%d\r\n\
%s\r\n"

#define Query_version_url "GET /v1/device/rom/?is_format_simple=true HTTP/1.0\r\n\
Host:%s:%d\r\n\
%s\r\n"

#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3095.5 Safari/537.36\r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Charset: iso-8859-5\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n"

#endif
