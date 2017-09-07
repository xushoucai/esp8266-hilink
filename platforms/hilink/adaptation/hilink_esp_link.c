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

/*
 *  hilink_link_sample.c -- provide hilink link API usage.
 */
#include "esp_common.h"
#include "hilink_link.h"
#include "hilink_log.h"
#include "hilink_profile.h"
#include "hilink_esp_adapter.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#define WIFI_RX_LEN0_OPEN    (66)
#define WIFI_RX_LEN0_WEP     (66+8)
#define WIFI_RX_LEN0_TKIP    (66+20)
#define WIFI_RX_LEN0_AES     (66+16)

extern dev_info_t g_dev_info;
extern svc_info_t g_svc_info;

typedef enum _encrytion_mode {
    ENCRY_NONE           = 1,
    ENCRY_WEP,
    ENCRY_TKIP,
    ENCRY_CCMP
} ENCYTPTION_MODE;

struct router_info {
    SLIST_ENTRY(router_info)     next;
    uint8_t  bssid[6];
    uint8_t  channel;
    uint8_t  authmode;
    uint16_t rx_seq;
    uint8_t  encrytion_mode;
    uint8_t  iv[8];
    uint8_t  iv_check;
};

struct RxControl {
    signed rssi: 8;
    unsigned rate: 4;
    unsigned is_group: 1;
    unsigned: 1;
    unsigned sig_mode: 2;
    unsigned legacy_length: 12;
    unsigned damatch0: 1;
    unsigned damatch1: 1;
    unsigned bssidmatch0: 1;
    unsigned bssidmatch1: 1;
    unsigned MCS: 7;
    unsigned CWB: 1;
    unsigned HT_length: 16;
    unsigned Smoothing: 1;
    unsigned Not_Sounding: 1;
    unsigned: 1;
    unsigned Aggregation: 1;
    unsigned STBC: 2;
    unsigned FEC_CODING: 1;
    unsigned SGI: 1;
    unsigned rxend_state: 8;
    unsigned ampdu_cnt: 8;
    unsigned channel: 4;
    unsigned: 12;
};

struct Ampdu_Info {
    uint16_t length;
    uint16_t seq;
    uint8_t  address3[6];
};

struct sniffer_buf {
    struct RxControl rx_ctrl;
    uint8_t  buf[36];
    uint16_t cnt;
    struct Ampdu_Info ampdu_info[1];
};

struct sniffer_buf2 {
    struct RxControl rx_ctrl;
    uint8_t buf[112];
    uint16_t cnt;
    uint16_t len; //length of packet
};

#define ETH_ALEN 6
struct ieee80211_hdr {
    uint16_t frame_control;
    uint16_t duration_id;
    uint8_t addr1[ETH_ALEN];
    uint8_t addr2[ETH_ALEN];
    uint8_t addr3[ETH_ALEN];
    uint16_t seq_ctrl;
};

char match_net_f = 0;

#define HILINK_BEACON_PERIOD    50     /* 50ms */

/* Hilink SmartLink config result */
static hilink_s_result hilink_s_res;

/* Hilink SmartLink runtime context instantiation */
static hilink_s_context hilink_s_ctx;

typedef struct wifi_802_11_hdr {
    uint8_t frame_control[4];
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t sequence;
} __packed wifi_802_11_hdr_t ;

typedef struct wifi_802_11_beacon {
    uint32_t timestamp[2];
    uint16_t interval;
    uint8_t cp_info_ess;
    uint8_t cp_info_sltotime;
} __packed wifi_802_11_beacon_t ;

typedef struct wifi_802_11_ei {
    uint8_t eid;
    uint8_t len;
    uint8_t data[];
} __packed wifi_802_11_ei_t ;

typedef struct wifi_link_info {
    const char* device_id;
    const char* device_sn;
    uint8_t sec_key[23];
    char devicessid[33];
    uint32_t devicessid_len;
} link_info_t ;

SLIST_HEAD(router_info_head, router_info) router_list;
static uint16_t channel_bits;
static uint8_t s_cur_channel = 0;
static os_timer_t channel_timer;
static os_timer_t beacon_send_timer;
static link_info_t link_info;
static bool smt_done = FALSE;
static wifi_hilink_event_handler_cb_t hilink_adapter_wifi_event_cb = NULL;

/*
*  Some libc adapter interface implementation:
*  1) hi_memset --> memset
*  2) hi_memcpy -> memcpy
*  3) hi_memcmp -> memcmp
*  4) hi_printf -> don't care
*  5) hi_getsystime -> dna_ticks_to_msec
*/

void* hi_memset(void* s, int c, unsigned int n)
{
    return memset(s, c, n);
}

void* hi_memcpy(void* dest, const void* src, unsigned int n)
{
    return memcpy(dest, src, n);
}

int hi_memcmp(const void* s1, const void* s2, unsigned int n)
{
    return memcmp(s1, s2, n);
}

int hi_printf(const char* format, ...)
{
    /* Don't care */
    return 0;
}

unsigned int hi_getsystime(void)
{
    /* The number of milliseconds since system bootup */
    //return dna_ticks_to_msec(dna_system_ticks());
    return system_get_time() / 1000;
}
int hilink_get_wifi_connect(void)
{
    if (true == wifi_station_connect()) {
        return 0;
    } else {
        return -1;
    }
}

int esp_smtlink_complete(void)
{
    struct station_config sta_conf;
    int status = 0;
    memset(&hilink_s_res, 0, sizeof(hilink_s_result));
    status = hilink_link_get_result(&hilink_s_res);

    if (status == 0) {
        memset(&sta_conf, 0, sizeof(sta_conf));
        memcpy(sta_conf.ssid, hilink_s_res.ssid, hilink_s_res.ssid_len);
        memcpy(sta_conf.password, hilink_s_res.pwd, hilink_s_res.pwd_len);

        hilink_info(" ssid %s pwd %s\r\n", hilink_s_res.ssid, hilink_s_res.pwd);
        wifi_station_set_config(&sta_conf);
        wifi_station_disconnect();
        wifi_station_connect();
    }

    return status;
}

/* Try to send notify packet to server after wifi connected
*  suggestion: TCP run 30S , UDP 15S to ensure Success
*/
void esp_smtlink_online_notice(void* param)
{
    int ret      = 0;
    int sockfd   = -1;
    int retry    = 0;
    int sleep_t  = 0;
    unsigned int   ip  = 0;
    unsigned int   len = 0;
    unsigned char  packet[128];
    struct sockaddr_in addr;

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(hilink_s_res.SendPort);

    hilink_save_revoke_flag(); // revoke device first

    if (1 == hilink_s_res.sendtype) { /**UDP broadcast*/
        retry   = 10;
        sleep_t = 300;
        memcpy(&addr.sin_addr.s_addr, "\xff\xff\xff\xff", 4);  //Set broadcast IP address
    } else { /**TCP socket*/
        retry   = 30;
        sleep_t = 100;
        ip = *((unsigned int*)hilink_s_res.SendIP);
        addr.sin_addr.s_addr = ip;
    }

    /* Create notify packet */
    memset(packet, 0, sizeof(packet));
    hilink_link_get_notifypacket(packet, &len);
    hilink_info("\r\n[Hilink] connect ip :%s  port:[%d], sendtype[%d]  \r\n",
                inet_ntoa(addr.sin_addr.s_addr),
                hilink_s_res.SendPort,
                hilink_s_res.sendtype);

    /* Send to server */
    if (1 == hilink_s_res.sendtype) {
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (sockfd < 0) {
            hilink_info("[DEBUG]:socket fail \r\n");
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        while ((retry--) && (len > 0)) {
            ret = sendto(sockfd, (void*)packet, len, 0, (struct sockaddr*)&addr, sizeof(addr));
            hilink_info("[HiLink] send broadcast times[%d]!!!\r\n", retry + 1);
            vTaskDelay(sleep_t / portTICK_RATE_MS);

            if (ret < 0) {
                hilink_info("send notify packet fail!!!\r\n");
            } else {
                hilink_info("send notify packet OK!!!\r\n");
            }
        }
    }
//TCP -----------------
    else {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (sockfd < 0) {
            hilink_info("[DEBUG]:socket fail \r\n");
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        ret = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

        if (ret == 0) {
            /* Send packet to server */
            while ((retry--) && (len > 0)) {
                hilink_info("[HiLink] send notify packet to server[%s]\r\n", packet);
                ret = write(sockfd, packet, len);
                vTaskDelay(sleep_t / portTICK_RATE_MS);

                if (ret < 0) {
                    hilink_info("send notify packet to server fail!!!\r\n");
                } else {
                    /* Success,  exit loop */
                    hilink_info("send notify packet to server OK!!!\r\n");
                }
            }
        }

        hilink_info("[HiLink] connect to server try[%d] fail errno[%d]!!!\r\n", retry + 1, errno);
    }

    close(sockfd);
    hilink_esp_m2m_main();
    vTaskDelete(NULL);
}

int esp_smtlink_parse_cb(uint8_t* frame, uint16_t len)
{
    int ret;

    ret = hilink_link_parse(frame, len);

    if (ret == HI_WIFI_STATUS_FINISH) {
        hilink_info("hilink_link_parse[%d]\n\r", HI_WIFI_STATUS_FINISH);
        os_timer_disarm(&beacon_send_timer);
        os_timer_disarm(&channel_timer);
        wifi_promiscuous_enable(0);
        esp_smtlink_complete();
    } else if (ret == HI_WIFI_STATUS_CHANNEL_LOCKED) {
        hilink_info("hilink_link_parse[%d]\n\r", HI_WIFI_STATUS_CHANNEL_LOCKED);
        s_cur_channel =  wifi_get_channel();
        os_timer_disarm(&channel_timer);
        wifi_set_channel(s_cur_channel);
    } else if (ret == HI_WIFI_STATUS_CHANNEL_UNLOCKED) {
        /**to release channel*/
        hilink_debug("hilink_link_parse[%d]\n\r", HI_WIFI_STATUS_CHANNEL_UNLOCKED);
        os_timer_arm(&channel_timer, 10, 0);

    }

    return ret;
}

void sniffer_wifi_promiscuous_rx(uint8_t* buf, uint16_t buf_len)
{
    uint8_t* data;
    uint16_t i;
    uint16_t len;
    uint16_t cnt = 0;
    struct sniffer_buf2* buf2;

    if (buf_len == sizeof(struct RxControl)) {
        struct RxControl* rxCtrl = (struct RxControl*)buf;
        return;
    } else if (buf_len == sizeof(struct sniffer_buf2)) {
        buf2 = (struct sniffer_buf2*)buf;
        esp_smtlink_parse_cb(buf2->buf, buf_len);
        return; //manage pack
    } else {
        struct router_info* info = NULL;
        struct sniffer_buf* sniffer = (struct sniffer_buf*)buf;

        data = buf + sizeof(struct RxControl);

        struct ieee80211_hdr* hdr = (struct ieee80211_hdr*)data;

        if (sniffer->cnt == 1) {
            len = sniffer->ampdu_info[0].length;
            esp_smtlink_parse_cb((uint8_t*)hdr, len);
        } else {
            int i;

            for (i = 0; i < sniffer->cnt; i++) {
                hdr->seq_ctrl = sniffer->ampdu_info[i].seq;
                memcpy(&hdr->addr3, sniffer->ampdu_info[i].address3, 6);
                len = sniffer->ampdu_info[i].length;
                //get ieee80211_hdr/data len to do user task
                esp_smtlink_parse_cb((uint8_t*)hdr, len);
            }
        }
    }
}

static void esp_channel_change_cb(void* pData)
{
    uint8_t i;

    if (smt_done == true) {
        return;
    }

    if (0 == hilink_link_get_lock_ready()) {
        for (i = s_cur_channel; i < 14; i++) {
            if ((channel_bits & (1 << i)) != 0) {
                s_cur_channel = i + 1;
                hilink_debug("channel %d\n", i);
                wifi_set_channel(i);
                hilink_link_reset();
                os_timer_arm(&channel_timer, 300, 0);
                break;
            }
        }

        if (i == 14) {
            s_cur_channel = 1;

            for (i = s_cur_channel; i < 14; i++) {
                if ((channel_bits & (1 << i)) != 0) {
                    s_cur_channel = i + 1;
                    hilink_debug("channel %d\n", i);
                    wifi_set_channel(i);
                    hilink_link_reset();
                    os_timer_arm(&channel_timer, 300, 0);
                    break;
                }
            }
        }
    }
}

int hilink_init_linkinfo(void)
{
    link_info.device_id = g_dev_info.prodId;
    link_info.device_sn = g_dev_info.sn;

    memset(&link_info.sec_key[0], '0', 22);
    char mac_str[17] = {0};
    uint8_t hwaddr[6];
    wifi_get_macaddr(STATION_IF, hwaddr);
    sprintf(mac_str, MACSTR , MAC2STR(hwaddr));

    link_info.sec_key[17] = mac_str[10];    //d
    memcpy(&link_info.sec_key[18], &mac_str[12], 2);  //c3
    memcpy(&link_info.sec_key[20], &mac_str[15], 2);  //55
    link_info.sec_key[22] = 0;
}

void esp_get_beacon_frame(uint8_t payload[1024] , uint16_t* payload_len)
{
    uint8_t supportrate_list[] = {0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c};
    uint8_t externsupportrate_list[] = {0x0c, 0x18, 0x24, 0x60};
    static uint16_t seq = 0;
    uint8_t mac[6];
    uint8_t  channel;
    static int i = 0;

    channel = wifi_get_channel();

    wifi_802_11_hdr_t* header;
    wifi_802_11_beacon_t* beacon;
    wifi_802_11_ei_t* ei;
    *payload_len = 0;
    memset(payload, 0, sizeof(payload));

    header = (wifi_802_11_hdr_t*)payload;
    /* Init 802.11 packet header part */
    uint8_t ieee80211_hdr_control[4] = {0x80, 0x00, 0x00, 0x00};
    memcpy(&header->frame_control, ieee80211_hdr_control, 4);

    memset(header->addr1, 0xFF, sizeof(header->addr1));

    //wifi_get_macaddr(SOFTAP_IF,mac);
    wifi_get_macaddr(STATION_IF, mac);

    memcpy(header->addr2, mac, sizeof(header->addr2));
    memcpy(header->addr3, mac, sizeof(header->addr3));
    header->sequence = (seq++) & 0x0FFF;
    *payload_len += sizeof(wifi_802_11_hdr_t);

    /* Init 802.11 packet data body part */
    beacon = (wifi_802_11_beacon_t*)(payload + *payload_len);
    //beacon->timestamp = dna_system_ticks() << 10;
    beacon->timestamp[0] = 0;
    beacon->timestamp[1] = system_get_time();

    beacon->interval = HILINK_BEACON_PERIOD;
    beacon->cp_info_ess = 1;

    beacon->cp_info_sltotime = 1;

    *payload_len += sizeof(wifi_802_11_beacon_t);

    /* Insert SSID Element info */
    ei = (wifi_802_11_ei_t*)(payload + *payload_len);
    ei->eid = 0;
    ei->len = link_info.devicessid_len;
    memcpy(ei->data, link_info.devicessid, ei->len);
    *payload_len += sizeof(wifi_802_11_ei_t) + ei->len;

    /* Insert supported rate */
    ei = (wifi_802_11_ei_t*)(payload + *payload_len);
    ei->eid = 1;
    ei->len = sizeof(supportrate_list);
    memcpy(ei->data, supportrate_list, sizeof(supportrate_list));
    *payload_len += sizeof(wifi_802_11_ei_t) + ei->len;

    /* Insert extern supported rate */
    ei = (wifi_802_11_ei_t*)(payload + *payload_len);
    ei->eid = 50;
    ei->len = sizeof(externsupportrate_list);
    memcpy(ei->data, externsupportrate_list, sizeof(externsupportrate_list));
    *payload_len += sizeof(wifi_802_11_ei_t) + ei->len;

    /* Insert ds */
    ei = (wifi_802_11_ei_t*)(payload + *payload_len);
    ei->eid = 3;
    ei->len = 1;
    memcpy(ei->data, &channel, 1);
    *payload_len += sizeof(wifi_802_11_ei_t) + ei->len;

    /* Insert erp */
    ei = (wifi_802_11_ei_t*)(payload + *payload_len);
    ei->eid = 42;
    ei->len = 1;
    ei->data[0] = 0x06;
    *payload_len += sizeof(wifi_802_11_ei_t) + ei->len;
}

void esp_beacon_send_cb(void* arg)
{
    uint8_t payload[1024];
    uint16_t payload_len;

    if (smt_done == true) {
        return;
    }

    /* Send this beacon packet in the specified channel */
    esp_get_beacon_frame(payload, &payload_len);
    int ret = wifi_send_pkt_freedom(payload, payload_len, 1);
    os_timer_arm(&beacon_send_timer, HILINK_BEACON_PERIOD, 0);
}

/** This is esp hilink init entry*/
int esp_smtlink_init(void)
{
    int ret;
    portBASE_TYPE state;
    hilink_s_pkt0len st_pkt0len;

    /* Print hilink version info */
    hilink_info("[HiLink] %s\r\n", hilink_link_get_version());

    /* Init runtime context */
    memset(&hilink_s_ctx, 0, sizeof(hilink_s_context));
    memset(&st_pkt0len,   0, sizeof(hilink_s_pkt0len));
    ret = hilink_link_init(&hilink_s_ctx);

    if (ret == 0) {
        hilink_link_reset();
        st_pkt0len.len_open = WIFI_RX_LEN0_OPEN;
        st_pkt0len.len_wep  = WIFI_RX_LEN0_WEP;
        st_pkt0len.len_tkip = WIFI_RX_LEN0_TKIP;
        st_pkt0len.len_aes  = WIFI_RX_LEN0_AES;
        (void)hilink_link_set_pkt0len(&st_pkt0len);

        memset(&link_info, 0, sizeof(link_info_t));

        if (0 != hilink_init_linkinfo()) {
            return -1;
        }

        if (link_info.device_id == NULL) {
            hilink_error("get link info failed");
            return -1;
        }

        hilink_info("link_info.device_id %s link_info.sec_key %s link_info.device_sn+11 %s\n", link_info.device_id , link_info.sec_key, link_info.device_sn + 11);

        hilink_link_get_devicessid("11", link_info.device_id, link_info.device_sn + 11,
                                   "B", link_info.sec_key, link_info.devicessid, &link_info.devicessid_len);
        /* output e.g  device ssid:HiG1100a1B000000000000000002FE01,ssidlen:32 */

        os_timer_disarm(&beacon_send_timer);
        os_timer_setfn(&beacon_send_timer, esp_beacon_send_cb, NULL);
        os_timer_arm(&beacon_send_timer, HILINK_BEACON_PERIOD, 0);
    } else {
        hilink_error("hilink_link_init error");
    }

    return ret;
}

void hilink_wifi_scan_done(void* arg, STATUS status)
{
    uint8_t ssid[33];

    channel_bits = 0;
    s_cur_channel = 1;

    struct router_info* info = NULL;

    while ((info = SLIST_FIRST(&router_list)) != NULL) {
        SLIST_REMOVE_HEAD(&router_list, next);
        os_free(info);
    }

    if (status == OK) {
        uint8_t i;
        struct bss_info* bss = (struct bss_info*)arg;

        while (bss != NULL) {
            if (bss->channel != 0) {

                hilink_debug("ssid %s, channel %d, authmode %d, rssi %d\n",
                             bss->ssid, bss->channel, bss->authmode, bss->rssi);
                channel_bits |= 1 << (bss->channel);

                info = (struct router_info*)os_zalloc(sizeof(struct router_info));
                info->authmode = bss->authmode;
                info->channel = bss->channel;
                memcpy(info->bssid, bss->bssid, 6);

                SLIST_INSERT_HEAD(&router_list, info, next);
            }

            bss = STAILQ_NEXT(bss, next);
        }

        for (i = s_cur_channel; i < 14; i++) {
            if ((channel_bits & (1 << i)) != 0) {
                s_cur_channel = i + 1;
                wifi_set_channel(i);
                hilink_link_reset();
                break;
            }
        }

        wifi_set_opmode(STATION_MODE);

        wifi_promiscuous_enable(0);
        wifi_set_promiscuous_rx_cb(sniffer_wifi_promiscuous_rx);
        wifi_promiscuous_enable(1);

        os_timer_disarm(&channel_timer);
        os_timer_setfn(&channel_timer, esp_channel_change_cb, NULL);
        os_timer_arm(&channel_timer, 300, 0);

        int ret = esp_smtlink_init();

        if (0 != ret) {
            hilink_error("hilink esp smart init fail!\r\n");
        }
    } else {
        hilink_error("err, scan status %d\n", status);
    }
}

void hilink_smt_task(void* pData)
{
    SLIST_INIT(&router_list);

    wifi_set_opmode(STATION_MODE);

    wifi_station_scan(NULL, hilink_wifi_scan_done);

    vTaskDelete(NULL);
}


void hilink_wifi_cb(System_Event_t* event)
{
    if (event == NULL) {
        return;
    }

    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
            if (smt_done == false) {
                xTaskCreate(esp_smtlink_online_notice, "esp_smtlink_online_notice", 512, NULL, 2, NULL);
                smt_done = true;
            } else {
                hilink_esp_m2m_main();
            }

            break;

        case EVENT_SOFTAPMODE_STADISCONNECTED:
            break;

        default:
            break;
    }

    if (hilink_adapter_wifi_event_cb) {
        hilink_adapter_wifi_event_cb(event);
    }
}

void wifi_set_adapter_event_handler_cb(wifi_hilink_event_handler_cb_t cb)
{
    hilink_adapter_wifi_event_cb = cb;
}


/** This is hilink smart link entry function*/
int hilink_esp_smtlink_main(void)
{
    wifi_set_event_handler_cb(hilink_wifi_cb);            //set wifi handler event cb
    struct station_config sta_config;
    memset(&sta_config, 0, sizeof(sta_config));
    wifi_station_get_config(&sta_config);

    os_printf("ssid = %s,passwd = %s\n", sta_config.ssid, sta_config.password);

    if (sta_config.ssid[0] == 0) {
        smt_done = false;      //whether the ap had connnected  , smt_done =ture if connected ,  or false
        match_net_f = 1;
    } else {
        smt_done = true;
    }

    if (smt_done == false) {
        os_printf("create config thread\n");
        xTaskCreate(hilink_smt_task, "hilink_smartlink_task", 1024, NULL, 2, NULL);
    }
}

