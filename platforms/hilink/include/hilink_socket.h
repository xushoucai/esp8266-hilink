/*----------------------------------------------------------------------------
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
/** @defgroup hilink_socket 通信
 *@ingroup kernel
 */
#ifndef _HILINK_SOCKET_H_
#define _HILINK_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup hilink_socket
 * Hilink socket错误码
 */
typedef enum {
    HILINK_SOCKET_NO_ERROR                  = 0,
    HILINK_SOCKET_NULL_PTR                  = -1,
    HILINK_SOCKET_CREAT_UDP_FD_FAILED       = -2,
    HILINK_SOCKET_SEND_UDP_PACKET_FAILED    = -3,
    HILINK_SOCKET_READ_UDP_PACKET_FAILED    = -4,
    HILINK_SOCKET_TCP_CONNECTING            = -5,
    HILINK_SOCKET_TCP_CONNECT_FAILED        = -6,
    HILINK_SOCKET_SEND_TCP_PACKET_FAILED    = -7,
    HILINK_SOCKET_READ_TCP_PACKET_FAILED    = -8,
    HILINK_SOCKET_REMOVE_UDP_FD_FAILED      = -9,
} hilink_socket_error_t;

/**
 *@ingroup hilink_socket
 *@brief 创建新的udp连接。
 *
 *@par 描述:
 *创建非阻塞的udp连接。
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param lport [IN] udp套接字绑定的本地端口号。
 *
 *@retval #HILINK_SOCKET_CREAT_UDP_FD_FAILED   -2，创建udp连接失败
 *@retval 大于0    返回udp套接字的值，且套接字已成功绑定了本地IP和指定端口号。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_udp_new
 *@since Huawei Liteos V100R002C00
 */
int hilink_udp_new(unsigned short lport);

/**
 *@ingroup hilink_socket
 *@brief 销毁udp套接字。
 *
 *@par 描述:
 *销毁udp套接字。
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param fd   [IN] hilink_udp_new创建的套接字。
 *
 *@retval 无。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see 无
 *@since Huawei Liteos V100R002C00
 */
void hilink_udp_remove(int fd);

/**
 *@ingroup hilink_socket
 *@brief 发送udp数据
 *
 *@par 描述:
 *非阻塞方式发送udp数据
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param fd      [IN] udp套接字。
 *@param buf     [IN] 指向待发送数据缓冲区的指针。
 *@param len     [IN] 待发送数据的长度，范围为[0，512)。
 *@param rip     [IN] 接收方ip。
 *@param rport   [IN] 接收方端口号。
 *
 *@retval #HILINK_SOCKET_NULL_PTR      			-1，buf或者rip参数为空
 *@retval #HILINK_SOCKET_NO_ERROR      			0， 发送数据时阻塞。
 *@retval #HILINK_SOCKET_SEND_UDP_PACKET_FAILED -3，发送出现错误
 *@retval 大于0     发送成功，返回发送数据的字节数。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_udp_read
 *@since Huawei Liteos V100R002C00
 */
int hilink_udp_send(int fd, const unsigned char* buf, unsigned short len,
                    const char* rip, unsigned short rport);

/**
 *@ingroup hilink_socket
 *@brief 读取udp数据
 *
 *@par 描述:
 *非阻塞方式读取udp数据
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param fd      [IN] udp套接字。
 *@param buf     [IN] 指向存放接收数据缓冲区的指针。
 *@param len     [IN] 存放接收数据缓冲区的最大长度，范围为[0，512)。
 *@param rip     [IN] 指向存放数据发送方ip缓冲区的指针。
 *@param riplen  [IN] 存放数据发送方ip的缓冲区的最大长度为512。
 *@param rport   [IN] 数据发送方端口号。
 *
 *@retval #HILINK_SOCKET_NULL_PTR      			-1，buf或rip或rport参数为空
 *@retval #HILINK_SOCKET_NO_ERROR      			0， 读取数据时阻塞。
 *@retval #HILINK_SOCKET_REND_UDP_PACKET_FAILED -4，读取出现错误
 *@retval 大于0     读取成功，返回接收到数据的字节数。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_udp_send
 *@since Huawei Liteos V100R002C00
 */
int hilink_udp_read(int fd, unsigned char* buf, unsigned short len,
                    char* rip, unsigned short riplen, unsigned short* rport);

/**
 *@ingroup hilink_socket
 *@brief 创建tcp连接
 *
 *@par 描述:
 *创建非阻塞模式tcp连接
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param dst      [IN] 接收方ip地址。
 *@param port     [IN] 接收方端口号。
 *
 *@retval #HILINK_SOCKET_NULL_PTR      			-1，des为空。
 *@retval #HILINK_SOCKET_TCP_CONNECT_FAILED     -6，创建连接失败。
 *@retval 大于0     连接成功,返回tcp套接字，然后使用hilink_tcp_state判断连接是否完全建立。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_udp_new
 *@since Huawei Liteos V100R002C00
 */
int hilink_tcp_connect(const char* dst, unsigned short port);

/**
 *@ingroup hilink_socket
 *@brief tcp连接状态
 *
 *@par 描述:
 *查询tcp连接状态
 *@attention
 *<ul>
 *<li>当fd小于0时，需要可以正常处理，返回HILINK_SOCKET_TCP_CONNECT_FAILED</li>
 *</ul>
 *
 *@param fd      [IN] tcp套接字。
 *
 *@retval #HILINK_SOCKET_TCP_CONNECTING      	-5，正在连接中。
 *@retval #HILINK_SOCKET_TCP_CONNECT_FAILED     -6，连接失败。
 *@retval #HILINK_SOCKET_NO_ERROR     			0， 连接成功。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see 无。
 *@since Huawei Liteos V100R002C00
 */
int hilink_tcp_state(int fd);

/**
 *@ingroup hilink_socket
 *@brief 断开tcp连接。
 *
 *@par 描述:
 *断开tcp连接。
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param fd   [IN] hilink_tcp_connect创建的套接字。
 *
 *@retval 无。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_udp_remove
 *@since Huawei Liteos V100R002C00
 */
void hilink_tcp_disconnect(int fd);

/**
 *@ingroup hilink_socket
 *@brief 发送tcp数据
 *
 *@par 描述:
 *非阻塞发送tcp数据
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param fd      [IN] tcp套接字。
 *@param buf     [IN] 指向待发送数据缓冲区的指针。
 *@param len     [IN] 待发送数据的长度，范围为[0，512)。
 *
 *@retval #HILINK_SOCKET_NULL_PTR      			-1，buf参数为空。
 *@retval #HILINK_SOCKET_NO_ERROR      			0， 发送数据时阻塞。
 *@retval #HILINK_SOCKET_SEND_TCP_PACKET_FAILED -7，发送出现错误
 *@retval 大于0     发送成功，返回发送数据的字节数。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_tcp_read
 *@since Huawei Liteos V100R002C00
 */
int hilink_tcp_send(int fd, const unsigned char* buf, unsigned short len);

/**
 *@ingroup hilink_socket
 *@brief 读取tcp数据
 *
 *@par 描述:
 *非阻塞读取tcp数据
 *@attention
 *<ul>
 *<li>无</li>
 *</ul>
 *
 *@param fd      [IN] tcp套接字。
 *@param buf     [IN] 指向存放接收数据缓冲区的指针。
 *@param len     [IN] 存放接收数据缓冲区的最大长度，范围为[0，512)。
 *
 *@retval #HILINK_SOCKET_NULL_PTR      			-1，buf参数为空。
 *@retval #HILINK_SOCKET_NO_ERROR      			0， 读取数据时阻塞。
 *@retval #HILINK_SOCKET_READ_TCP_PACKET_FAILED -8，读取出现错误
 *@retval 大于0     读取成功，返回读取数据的字节数。
 *@par 依赖:
 *<ul><li>hilink_socket.h：该接口声明所在的头文件。</li></ul>
 *@see hilink_tcp_send
 *@since Huawei Liteos V100R002C00
 */
int hilink_tcp_read(int fd, unsigned char* buf, unsigned short len);

//============================================
//WLT code  part

extern int number;

//============================================

#ifdef __cplusplus
}
#endif

#endif
