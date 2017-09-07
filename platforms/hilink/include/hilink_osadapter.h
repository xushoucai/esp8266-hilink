/**---------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei Liteos may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei Liteos of U.S. and the country in which you are located.
 * Import, export and usage of Huawei Liteos in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

/** @defgroup hilink_osadapter 操作系统适配
 *@ingroup osadapter
 */
#ifndef HILINK_OSADAPTER_H_
#define HILINK_OSADAPTER_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

typedef enum {
    NW_DISCONNECTED		= 0,	/*网络连接断开*/
    NW_CONNECTED		= 1		/*网络已连接*/
} network_state_t;

/**
 *@ingroup hilink_osadapter
 *@brief 获取本机当前连接的wifi ssid。
 *
 *@par 描述:
 *<ul>
 *<li>获取本机当前连接的wifi ssid。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param pssid	   [IN/OUT]	本机ssid
 *@param pssidlen  [IN/OUT]	ssid长度
 *
 *@retval 0，获取成功
 *@retval -1，获取失败
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see 无
 *@since Huawei Liteos V100R002C00
 */
int hilink_getssid(unsigned char* pssid, unsigned char* pssidlen);

/**
*@ingroup hilink_osadapter
*@brief 获取远端主机ip地址。
*
*@par 描述:
*通过DNS域名解析，获取远端主机ip地址，此接口实现为非阻塞。
*@attention
*<ul>
*<li>无</li>
*</ul>
*
*@param hostname    [IN] 远端主机名称。
*@param ip_list     [OUT] 存放远端主机ip地址列表的数组。
*@param num         [IN] 存放远端主机ip地址列表的数组的大小，范围为[1，4]。
*
*@retval 0   获取成功
*@retval -1  获取失败
*@par 依赖:
*<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
*@see hilink_resolve_host
*@since Huawei Liteos V100R002C00
*/
extern int hilink_gethostbyname(char* hostname, char ip_list[][40], int num);

/**
 *@ingroup hilink_osadapter
 *@brief 获取网络状态。
 *
 *@par 描述:
 *<ul>
 *<li>获取网络状态。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>state返回网络状态。</li>
 *</ul>
 *
 *@param state	[IN/OUT]	网络状态,取值范围为[0,1]
 *
 *@retval 0    获取成功
 *@retval 非0  获取失败
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see 无
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_network_state(int* state);

/**
 *@ingroup hilink_osadapter
 *@brief 获取当前时间。
 *
 *@par 描述:
 *<ul>
 *<li>获取当前时间，以毫秒为单位。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param ms		[OUT]	当前时间
 *
 *@retval 0    获取成功
 *@retval -1   获取失败
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see 无
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_gettime(unsigned long* ms);



/**
 *@ingroup hilink_osadapter
 *@brief 随机数发生器的初始化函数。
 *
 *@par 描述:
 *<ul>
 *<li>根据系统提供的种子值，产生随机数。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>srand和rand()配合使用产生随机数序列。</li>
 *</ul>
 *
 *@param 无
 *
 *@retval 随机数
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_srand
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_rand(void);

/**
 *@ingroup hilink_osadapter
 *@brief 随机数发生器的初始化函数。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的seed值，产生随机数。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>srand和rand()配合使用产生随机数序列。</li>
 *</ul>
 *
 *@param seed   [IN]     产生随机数的种子值，种子值取真随机数。
 *
 *@retval 随机数
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_rand
 *@since Huawei Liteos V100R002C00
 */
extern void hilink_srand(unsigned int seed);


/**
 *@ingroup hilink_osadapter
 *@brief 向flash中写内容。
 *
 *@par 描述:
 *<ul>
 *<li>把len字节长度内容写入flash中,len最长为1024。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>指针必须指向合法内存</li>
 *</ul>
 *
 *@param buf   [IN/OUT]     缓冲区的首地址
 *@param len   [IN]     	缓冲区的长度
 *
 *@retval 0    写入成功
 *@retval -1   写入失败
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_read_flash
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_save_flash(char* buf, unsigned int len);

/**
 *@ingroup hilink_osadapter
 *@brief 从flash中读取内容。
 *
 *@par 描述:
 *<ul>
 *<li>从flash中读取len字节长度内容。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>指针必须指向合法内存。</li>
 *</ul>
 *
 *@param buf   [IN/OUT]     缓冲区的首地址
 *@param len   [IN]     	缓冲区的长度
 *
 *@retval 0    读取成功
 *@retval -1   读取失败
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_save_flash
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_read_flash(char* buf, unsigned int len);


/**
 *@ingroup hilink_osadapter
 *@brief 计算给定字符串的长度。
 *
 *@par 描述:
 *<ul>
 *<li>计算给定字符串的（unsigned int型）长度，不包括'\0'在内。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>指针必须指向合法内存。</li>
 *</ul>
 *
 *@param src   [IN]     	字符串的首地址
 *
 *@retval 字符串的长度
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see 无
 *@since Huawei Liteos V100R002C00
 */
extern unsigned int hilink_strlen(const char* src);

/**
 *@ingroup hilink_osadapter
 *@brief 复制字符串中的内容。
 *
 *@par 描述:
 *<ul>
 *<li>把src所指向的字符串中以src地址开始的前len个字节复制到dst所指的数组中，并返回dst。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>指针必须指向合法内存。</li>
 *</ul>
 *
 *@param dst   [IN]     目标字符数组
 *@param src   [IN]     源字符串的起始位置
 *@param len   [IN]     要复制的字节数
 *
 *@retval dst 目标字符数组的首地址
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_memcpy | hilink_strncat
 *@since Huawei Liteos V100R002C00
 */
extern char* hilink_strncpy(char* dst, const char* src, unsigned int len);
/**
 *@ingroup hilink_osadapter
 *@brief 连接字符串。
 *
 *@par 描述:
 *<ul>
 *<li>把src所指字符串添加到dst结尾处实现字符串的连接，连接过程覆盖dst结尾处的'/0'。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>指针必须指向合法内存</li>
 *</ul>
 *
 *@param dst   [IN]     目标字符串的指针
 *@param src   [IN]     源字符串的指针
 *
 *@retval dst  目标字符串的指针
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_strncpy
 *@since Huawei Liteos V100R002C00
 */
extern char* hilink_strncat(char* dst, const char* src, unsigned int len);
/**
 *@ingroup hilink_osadapter
 *@brief 比较两个字符串的大小。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的字符串首地址及长度，比较两个字符串的大小。指针必须指向合法内存。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param str1   [IN]     字符串1的首地址
 *@param str2   [IN]     字符串2的首地址
 *@param len	[IN]     字符串的长度
 *
 *@retval 等于0   str1等于str2
 *@retval 小于0   str1小于str2
 *@retval 大于0   str1大于str2
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_memcmp
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_strncmp(const char* str1, const char* str2, unsigned int len);

/**
 *@ingroup hilink_osadapter
 *@brief 查找一个字符在一个字符串中首次出现的位置。
 *
 *@par 描述:
 *<ul>
 *<li>查找一个字符ch在另一个字符串str中末次出现的位置，并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>变参函数。</li>
 *</ul>
 *
 *@param str   	[IN]    字符串的首地址
 *@param ch   	[IN]    要查找的字符
 *
 *@retval Null,未能找到指定字符
 *@retval 非Null,字符首次出现位置的地址
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_strrchr
 *@since Huawei Liteos V100R002C00
 */
extern char* hilink_strchr(char* str, int ch);

/**
 *@ingroup hilink_osadapter
 *@brief 查找一个字符在一个字符串中末次出现的位置。
 *
 *@par 描述:
 *<ul>
 *<li>查找一个字符c在另一个字符串str中末次出现的位置，并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>变参函数。</li>
 *</ul>
 *
 *@param str   	[IN]    字符串的首地址
 *@param c   	[IN]    要查找的字符
 *
 *@retval Null     未能找到指定字符
 *@retval 非Null   字符末次出现位置的地址
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_strchr
 *@since Huawei Liteos V100R002C00
 */
extern char* hilink_strrchr(const char* str, char c);

/**
 *@ingroup hilink_osadapter
 *@brief 把字符串转换成整型数。
 *
 *@par 描述:
 *<ul>
 *<li>如果第一个非空格字符存在，是数字或者正负号则开始做类型转换，之后检测到非数字(包括结束符 \0) 字符时停止转换。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>变参函数。</li>
 *</ul>
 *
 *@param str   		[IN]    字符串的首地址
 *
 *@retval 整型数
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see 无
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_atoi(const char* str);

/**
 *@ingroup hilink_osadapter
 *@brief 字符串格式化函数。
 *
 *@par 描述:
 *<ul>
 *<li>把格式化的数据写入某个字符串缓冲区。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>变参函数</li>
 *</ul>
 *
 *@param buf   		[IN/OUT]    指向将要写入的字符串的缓冲区
 *@param format   	[IN]     	格式控制信息
 *@param ...   		[IN]     	可选参数，可以是任何类型的数据
 *
 *@retval 小于0       写入失败
 *@retval 大于等于0   写入的字符串长度
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_sprintf
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_sprintf(char* buf, const char* format, ...);

/**
 *@ingroup hilink_osadapter
 *@brief 字符串格式化函数。
 *
 *@par 描述:
 *<ul>
 *<li>把格式化的数据写入某个字符串缓冲区。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>变参函数。</li>
 *</ul>
 *
 *@param buf   		[IN/OUT]    指向将要写入的字符串的缓冲区
 *@param size   	[IN]    	要写入的字符的最大数目(含\0)，超过size会被截断,末尾自动补\0
 *@param format   	[IN]     	格式控制信息
 *@param ...   		[IN]     	可选参数，可以是任何类型的数据
 *
 *@retval 小于0       写入失败
 *@retval 大于等于0   成功则返回欲写入的字符串长度
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_sprintf
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_snprintf(char* buf, unsigned int size, const char* format, ...);

/**
 *@ingroup hilink_osadapter
 *@brief 格式化输出函数。
 *
 *@par 描述:
 *<ul>
 *<li>格式化输出，一般用于向标准输出设备按规定格式输出信息。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>变参函数</li>
 *</ul>
 *
 *@param 	format  [IN]     格式控制信息
*@param 	...   	[IN]     可选参数，可以是任何类型的数据
 *
 *@retval 小于0       输出失败
 *@retval 大于等于0   输出的长度
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_sprintf
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_printf(const char* format, ...);


/**
 *@ingroup hilink_osadapter
 *@brief 在一段内存块中填充某个给定的值。
 *
 *@par 描述:
 *<ul>
 *<li>将dst中前len个字节用c替换并返回dst 。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>对较大的结构体或数组进行清零操作的一种最快方法</li>
 *</ul>
 *
 *@param dst   [IN/OUT]     目标的起始位置
 *@param c     [IN]     	要填充的值
 *@param len   [IN]     	要填充的值字节数
 *
 *@retval 无
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_free
 *@since Huawei Liteos V100R002C00
 */
extern void* hilink_memset(void* dst, int c, unsigned int len);

/**
 *@ingroup hilink_osadapter
 *@brief 复制内存中的内容。
 *
 *@par 描述:
 *<ul>
 *<li>从源src所指的内存地址的起始位置开始复制len个字节到目标dst所指的内存地址的起始位置中。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>指针必须指向合法内存。</li>
 *</ul>
 *
 *@param dst   [IN/OUT]     目标内存的起始位置
 *@param src   [IN]     	源内存的起始位置
 *@param len   [IN]     	要复制的字节数
 *
 *@retval 无
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_strncpy
 *@since Huawei Liteos V100R002C00
 */
extern void* hilink_memcpy(void* dst, const void* src, unsigned int len);

/**
 *@ingroup hilink_osadapter
 *@brief 比较两块内存区域。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的内存首地址及长度，比较两块内存的前len个字节。指针必须指向合法内存。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>该函数是按字节比较的。</li>
 *</ul>
 *
 *@param buf1   [IN]     内存1的首地址
 *@param buf2   [IN]     内存2的首地址
 *@param len	[IN]     要比较的字节数
 *
 *@retval 等于0   buf1等于buf2
 *@retval 小于0   buf1小于buf2
 *@retval 大于0   buf1大于buf2
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_strncmp
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_memcmp(const void* buf1, const void* buf2, unsigned int len);

/**
 *@ingroup hilink_osadapter
 *@brief 释放指针pt所占用的内存空间。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的指针pt，释放其所占用的空间。指针必须指向合法内存。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param pt   [IN]     指针
 *
 *@retval 无
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_memset
 *@since Huawei Liteos V100R002C00
 */
extern void hilink_free(void* pt);


/**
 *@ingroup hilink_osadapter
 *@brief 将主机字节顺序转换为网络字节顺序。
 *
 *@par 描述:
 *<ul>
 *<li>把用户提供的主机字节顺序的16位数转换为网络字节顺序表示的16位数。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param ns   [IN]     主机字节顺序表示的16位数
 *
 *@retval 网络字节顺序表示的16位数
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_ntohs
 *@since Huawei Liteos V100R002C00
 */
extern unsigned short hilink_htons(unsigned short hs);

/**
 *@ingroup hilink_osadapter
 *@brief 将网络字节顺序转换为主机字节顺序。
 *
 *@par 描述:
 *<ul>
 *<li>把用户提供的网络字节顺序的16位数转换为主机字节顺序表示的16位数。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param ns   [IN]     网络字节顺序表示的16位数
 *
 *@retval 主机字节顺序表示的16位数
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_htons
 *@since Huawei Liteos V100R002C00
 */
extern unsigned short hilink_ntohs(unsigned short ns);


/**
 *@ingroup hilink_osadapter
 *@brief 将字符串形式的json数据解析为json结构体类型的数据。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的字符串形式的json数据，从中解析出json结构体类型的数据。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param value   [IN]     字符串形式的json数据
 *
 *@retval NULL 解析失败
 *@retval 非NULL 解析成功，返回值为json结构体指针
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_json_delete
 *@since Huawei Liteos V100R002C00
 */
extern void* hilink_json_parse(const char* value);

/**
 *@ingroup hilink_osadapter
 *@brief 获取json结构体中的字符串value。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的json结构体指针object和需要获取的item的名字，从json结构体中获取对应的字符串value。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param object		[IN]     json结构体指针
 *@param name		[IN]     指定需要获取的item的名字
 *@param len		[OUT] 获取的字符串的长度
 *
 *@retval NULL      获取失败
 *@retval 非NULL    获取成功，返回值为字符串value首地址
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_json_get_number_value
 *@since Huawei Liteos V100R002C00
 */
extern char* hilink_json_get_string_value(void* object, \
        char* name, unsigned int* len);

/**
 *@ingroup hilink_osadapter
 *@brief 获取json结构体中的数值value。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的json结构体指针object和需要获取的item的名字，从json结构体中获取对应的数值value。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param object		[IN]     json结构体指针
 *@param name		[IN]     指定需要获取的item的名字
 *@param value		[OUT] 保存获取的item的值
 *
 *@retval 0    获取成功
 *@retval -1   获取失败
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_json_get_string_value
 *@since Huawei Liteos V100R002C00
 */
extern int hilink_json_get_number_value(void* object, \
                                        char* name, int* value);

/**
 *@ingroup hilink_osadapter
 *@brief 释放json结构体所占用的空间。
 *
 *@par 描述:
 *<ul>
 *<li>根据用户提供的json结构体指针object，释放json数据所占用的空间。json结构体指针必须指向合法内存。</li>
 *</ul>
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param object   [IN]     json结构体指针
 *
 *@retval 无
 *@par 依赖:
 *<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_json_parse
 *@since Huawei Liteos V100R002C00
 */
extern void hilink_json_delete(void* object);

/**
*@ingroup hilink_bi_get_cr
*@brief 获取BI密钥密文。
*
*@par 描述:
*<ul>
*<li>获取hilink设备BI模块预制的密钥密文。</li>
*</ul>
*@attention
*<ul>
*<li>无</li>
*</ul>
*
*@param buf   		[IN/OUT]    接收缓冲区
*@param size   	[IN]    	接收缓冲区大小(单位:byte)
*
*@retval 0      获取密钥密文成功
*@retval 小于0  获取密钥密文失败
*@par 依赖:
*<ul><li>hilink_osadapter.h：该接口声明所在的头文件。</li></ul>
*@see hilink_bi_get_cr
*@since Huawei Liteos V100R002C00
*/
extern int hilink_bi_get_cr(char* buf, unsigned int size);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif  /* HILINK_OSADAPTER_H_ */

