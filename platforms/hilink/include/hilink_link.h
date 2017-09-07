/*******************************************************************************
 *               Copyright (C) 2015, Huawei Tech. Co., Ltd.
 *                      ALL RIGHTS RESERVED
 *******************************************************************************/

/** @defgroup hilink.h

 *  @author  huawei
 *  @version 1.0
 *  @date    2015/11/26 \n

 *  history:\n
 *          1. 2015/11/26, huawei, create this file\n\n
 *
 *  此文件的详细描述...
 *
 * @{
 */

/* 注: mainpage对于多个文件生成一份chm时，这部分是可选的，请在上面的详细描述中填写完整内容 */
/*! \mainpage
 *
 *  模块说明:\n

 *  \section intro_sec 本模块提供的接口:
 *  - ::
 */

#ifndef __HILINK_LINK_H__
#define __HILINK_LINK_H__

/*******************************************************************************
 *   本文件需要包含的其它头文件
*******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /*  __cpluscplus */
#endif /*  __cpluscplus */

#ifndef CFG_XIP_ENABLE
#define CFG_XIP_ENABLE 0
#endif

#if (CFG_XIP_ENABLE)
#define  XIP_ATTRIBUTE(x)    __attribute__ ((section(x)))
#else
#define  XIP_ATTRIBUTE(x)
#endif

#define  XIP_ATTRIBUTE_VAR    XIP_ATTRIBUTE (".xipsec1")
#define  XIP_ATTRIBUTE_FUN    XIP_ATTRIBUTE (".xipsec0")

/*******************************************************************************
 *   枚举类型定义区
*******************************************************************************/
typedef enum tagHILINK_E_ERR_CODE {
    HILINK_E_ERR_TOP = 100,
    HILINK_E_ERR_PARAM,           /**< invalid param */
    HILINK_E_ERR_MEM,             /**< mem operation error */
    HILINK_E_ERR_FRAME,           /**< invalid frame */
    HILINK_E_ERR_PARSE_ARRAY,     /**< parse array error*/
    HILINK_E_ERR_DERCYPT,         /**< decrypt error*/
    HILINK_E_ERR_DERCYPT_W,       /**< decrypt error*/
    HILINK_E_ERR_PARSE_CAP,       /**< parse cap error*/
    HILINK_E_ERR_PARSE_MIX,       /**< parse app ip port token error*/
    HILINK_E_ERR_PARSE_SSID,      /**< parse ssid error*/
    HILINK_E_ERR_PARSE_PW,        /**< parse password error*/
    HILINK_E_ERR_PARSE_DEVICESN,  /**< parse device sn error*/
    HILINK_E_ERR_PACK,            /**< pack buffer for online error*/
    HILINK_E_ERR_BUTT
} HILINK_E_ERR_CODE;

/*
 * hilink_link_parse()正常情况下的返回值
 */
typedef enum tagHILINK_WIFI_STATUS {
    HI_WIFI_STATUS_RECEIVING = 0,        /* 收包正常 */
    HI_WIFI_STATUS_CHANNEL_LOCKED = 1,   /* 锁定wifi信道 */
    HI_WIFI_STATUS_FINISH = 2,           /* 收包完成 */
    HI_WIFI_STATUS_CHANNEL_UNLOCKED = 3, /* 释放wifi信道 */
} HILINK_WIFI_STATUS;

typedef enum tagHILINK_WIFI_ENCTYPE {
    HI_WIFI_ENC_OPEN = 0,         /* OPEN */
    HI_WIFI_ENC_WEP = 1,          /* WEP */
    HI_WIFI_ENC_TKIP = 2,         /* WPA */
    HI_WIFI_ENC_AES = 3,          /* WPA2 */
    HI_WIFI_ENC_UNKNOWN = 4,      /* unknown */
    HI_WIFI_ENC_BUTT
} HILINK_WIFI_ENCTYPE;

/*******************************************************************************
 *   数据结构类型和联合体类型定义
*******************************************************************************/
/*
 * hilink_link_init()需要传入此参数，全局使用
 */
typedef struct {
    unsigned char  chaos[1024];
} hilink_s_context;

/*
 * hilink_link_get_result()需要传入此参数获取结果
 */
typedef struct {
    unsigned char   SendIP[4];              /** SendIP */
    unsigned char   ssid[64];               /** wifi ssid */
    unsigned char   pwd[128];               /** wifi password */
    unsigned char   ssid_len;               /** wifi ssid length */
    unsigned char   pwd_len;                /** wifi pwd length */
    unsigned char   enc_type;               /** wifi enc type */
    unsigned char   sendtype;               /** send back by UDP1 TCP2  */
    unsigned short  SendPort;               /** SendPort */
    unsigned char   reserved[2];            /** reserved  */
} hilink_s_result;

/*
 * hilink_link_get_result()需要传入此参数
 */
typedef struct {
    unsigned char   len_open;
    unsigned char   len_wep;
    unsigned char   len_tkip;
    unsigned char   len_aes;
} hilink_s_pkt0len;

/*******************************************************************************
 *   全局函数声明(extern)区
*******************************************************************************/
/**
* Begin 需要外部适配的函数
*
* void *hilink_memset(void *s, int c, unsigned int n);
* void *hilink_memcpy(void *dest, const void *src, unsigned int n);
* int   hilink_memcmp(const void *s1, const void *s2, unsigned int n);
* int   hilink_printf(const char* format, ...);
* int   hilink_gettime(unsigned long * ms);
* 瘦设备适配函数，Ac长度固定48字节，AC文件在设备做HiLink认证时由华为认证时分发
* int   hilink_sec_get_Ac(unsigned char* pAc, unsigned int ulLen)
* End 外部适配函数
*/

/**
* 设置wifi不同加密类型的QOS Data的0基准长度
*  @param[in]  pst_pkt0len refer to struct hilink_s_pkt0len
*
*  @retval :: 0 success
*/
int hilink_link_set_pkt0len(hilink_s_pkt0len* pst_pkt0len) XIP_ATTRIBUTE_FUN;

/**
* 获取版本号
*/
const char* hilink_link_get_version(void) XIP_ATTRIBUTE_FUN;

/**
* 初始化link库,hilink_s_context由外部使用者申请传入
*  @param[in]  pcontext refer to struct hilink_s_context
*
*  @retval :: 0 success
*/
int hilink_link_init(hilink_s_context* pcontext) XIP_ATTRIBUTE_FUN;

/**
* 切换信道以后，调用本接口清缓存
*/
int hilink_link_reset(void) XIP_ATTRIBUTE_FUN;

/** 锁信道是否ready状态
*
*  @param[in]  none
*
*  @retval :: 0:可以切换信道,非0不能切换信道
*/
int hilink_link_get_lock_ready(void) XIP_ATTRIBUTE_FUN;


/** 接收组播报文
*
*  @param[in]  frame    802.11帧
*  @param[in]  len      报文长度
*
*  @retval :: refer to enum HILINK_WIFI_STATUS
*/
int hilink_link_parse(const void* frame, unsigned int len) XIP_ATTRIBUTE_FUN;

/** 获取组播报文结果,在hilink_link_parse 返回 HI_WIFI_STATUS_COMPLETE调用
*
*  @in ackey 授权码 由huawei发放,非法key将不能获得结果
*  @in ackeylen 必须为48字节
*  @param[out] pst_result  解析成功后的结果
*
*  @retval :: 0 success
*/
int hilink_link_get_result(hilink_s_result* pst_result) XIP_ATTRIBUTE_FUN;

/** 【可选接口】如果设备具有AP功能，可通过该接口获取到AP的名称，用于手机侧通过该名字发现设备。
    配置link的相关信息,返回device的ssid
*
*  @param[in]   ssid_type           ssid类型，长度必须为2字节，跟设备状态有关
*  @param[in]   device_id           HiLink设备认证号，4字节
*  @param[in]   device_sn           设备SN号，SN的后1位,1字节
*  @param[in]   sec_type            设备支持的加密模式,1字节
*  @param[in]   sec_key             key字符，22字节
*  @param[out]  device_ssid_out     SSID名字，用于设备自己启动AP时配置进去
*  @param[out]  ssid_len_out        SSID长度，最长32字节
*
*/
int hilink_link_get_devicessid(const char* ssid_type,
                               const char* device_id,
                               const char* device_sn,
                               const char* sec_type,
                               const char* sec_key,
                               char* device_ssid_out,
                               unsigned int* ssid_len_out) XIP_ATTRIBUTE_FUN;

/** 设备连上WiFi后，需要通知手机设备上线。通过本接口组装发送的通知数据
*
*  @param[out]  buffer              返回组装后的bufer，用于socket发送  最少128字节
*  @param[out]  buffer_len          buffer长度
*
*/
int hilink_link_get_notifypacket(char* buffer_out, unsigned int* len_out) XIP_ATTRIBUTE_FUN;

//-----------------------------------------------
//---------------WLT  PART

void hilink_smt_task_uart(void* pData) XIP_ATTRIBUTE_FUN;
extern void  hilink_m2m_task(void* pData);


extern  char check_smart_timeout_f;

extern char match_net_f;
//-----------------------------------------------

#ifdef __cplusplus
#if __cplusplus
}
#endif /*  __cpluscplus */
#endif /*  __cpluscplus */


#endif  /* __HILINK_LINK_H__ */

/** @}*/
