#ifndef __HILINK_LOG_H__
#define __HILINK_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define HILINK_LOG_LEVEL_ERROR      (1)
#define HILINK_LOG_LEVEL_WARN       (2)
#define HILINK_LOG_LEVEL_INFO       (3)
#define HILINK_LOG_LEVEL_NOTICE     (4)
#define HILINK_LOG_LEVEL_DEBUG      (5)

#define Black   0;30
#define Red     0;31
#define Green   0;32
#define Brown   0;33
#define Blue    0;34
#define Purple  0;35
#define Cyan    0;36

#define HILINK_LOG_LEVEL  HILINK_LOG_LEVEL_NOTICE

#define hilink_notice(format, ...) \
    do{\
        if(HILINK_LOG_LEVEL >= HILINK_LOG_LEVEL_NOTICE){\
            os_printf("\033[0;36m" format "\r\n", ##__VA_ARGS__);\
            os_printf("\033[0m"); \
        }\
    }while(0)

#define hilink_info(format, ...) \
    do{\
        if(HILINK_LOG_LEVEL >= HILINK_LOG_LEVEL_INFO){\
            os_printf("\033[1;36m" format "\r\n", ##__VA_ARGS__);\
            os_printf("\033[0m"); \
        }\
    }while(0)

#define hilink_error(format, ...) \
    do{\
        if(HILINK_LOG_LEVEL >= HILINK_LOG_LEVEL_ERROR){\
            os_printf("\033[0;31m[ERROR][%s][%s][%d]\n" format "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
            os_printf("\033[0m"); \
        }\
    }while(0)

#define hilink_warn(format, ...) \
    do{\
        if(HILINK_LOG_LEVEL >= HILINK_LOG_LEVEL_WARN){\
            os_printf("\033[1;33m[WARN][%s][%s][%d]\n" format "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
            os_printf("\033[0m"); \
        }\
    }while(0)

#define hilink_debug(format, ...) \
    do{\
        if(HILINK_LOG_LEVEL >= HILINK_LOG_LEVEL_DEBUG){\
            os_printf("\033[1;32m[DEBUG][%s][%s][%d]\n" format "\r\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
            os_printf("\033[0m"); \
        }\
    }while(0)

#ifdef __cplusplus
}
#endif

#endif /* __HILINK_LOGGING_H__ */