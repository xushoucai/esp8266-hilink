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

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "hilink_socket.h"
#include "hilink_log.h"

#define MULTICAST_GROUP_ADDR4 "238.238.238.238"

int number = 0;

int hilink_udp_new(unsigned short lport)
{
    struct sockaddr_in servaddr;
    struct ip_mreq group;
    int fd = HILINK_SOCKET_NO_ERROR;
    int flags;
    int reuse;

    /*创建socket*/
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        hilink_error("creat  socket fd failed\n");
        return HILINK_SOCKET_CREAT_UDP_FD_FAILED;
    }

    /*设置非阻塞模式*/
    flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        hilink_error("fcntl: %s\n", strerror(errno));
        close(fd);
        return HILINK_SOCKET_CREAT_UDP_FD_FAILED;
    }

    if (lport != 0) {
        reuse = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                       (const char*)&reuse, sizeof(reuse)) != 0) {
            close(fd);
            hilink_error("set SO_REUSEADDR failed\n");
            return HILINK_SOCKET_CREAT_UDP_FD_FAILED;
        }

        memset(&servaddr, 0, sizeof(struct sockaddr_in));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(lport);

        if (bind(fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) < 0) {
            close(fd);
            hilink_error("bind port %u failed\n", lport);
            return HILINK_SOCKET_CREAT_UDP_FD_FAILED;
        }

        /*加入组播组*/
        memset(&group, 0, sizeof(struct ip_mreq));
        group.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP_ADDR4);
        group.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group,
                       sizeof(group)) < 0) {
            close(fd);
            hilink_error("set multi group failed\n");
            return HILINK_SOCKET_CREAT_UDP_FD_FAILED;
        }
    }

    return fd;
}

int hilink_udp_remove_multi_group(int fd)
{
    struct ip_mreq group;
    int ret = HILINK_SOCKET_NO_ERROR;

    memset(&group, 0, sizeof(struct ip_mreq));
    group.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP_ADDR4);
    group.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group,
                   sizeof(group)) < 0) {
        hilink_error("remove from multi group failed\n");
        ret = HILINK_SOCKET_REMOVE_UDP_FD_FAILED;
    }

    return ret;
}

void hilink_udp_remove(int fd)
{
    close(fd);
}

int hilink_udp_send(int fd, const unsigned char* buf, unsigned short len,
                    const char* rip, unsigned short rport)
{
    struct sockaddr_in dstaddr;
    int ret = -1;

    if (buf == NULL || rip == NULL) {
        return HILINK_SOCKET_NULL_PTR;
    }

    memset(&dstaddr, 0, sizeof(struct sockaddr_in));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_addr.s_addr = inet_addr(rip);
    dstaddr.sin_port = htons(rport);

    /*UDP发送数据需要判断错误码*/
    ret = (int)(sendto(fd, buf, len, 0, (struct sockaddr*)&dstaddr, sizeof(struct sockaddr_in)));

    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return HILINK_SOCKET_NO_ERROR;
        } else {
            return HILINK_SOCKET_SEND_UDP_PACKET_FAILED;
        }
    }

    return ret;
}

int hilink_udp_read(int fd, unsigned char* buf, unsigned short len,
                    char* rip, unsigned short riplen, unsigned short* rport)
{
    // printf("remote ip %s port %u\n",rip, *rport);
    struct sockaddr_in dstaddr;
    int length = 0;
    unsigned int addrlen = sizeof(dstaddr);

    if (buf == NULL || rip == NULL || rport == NULL) {
        return HILINK_SOCKET_NULL_PTR;
    }

    /*UDP读取数据需要判断错误码*/
    length = (int)(recvfrom(fd, buf, len, 0,
                            (struct sockaddr*)&dstaddr, &addrlen));

    if (length <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            //printf("recv no data,continue reading\n");
            return HILINK_SOCKET_NO_ERROR;
        } else {
            return HILINK_SOCKET_READ_UDP_PACKET_FAILED;
        }
    }

    buf[length] = '\0';

    strncpy(rip, inet_ntoa(dstaddr.sin_addr), riplen);
    *rport = ntohs(dstaddr.sin_port);

    return length;
}

int hilink_tcp_connect(const char* dst, unsigned short port)
{
    struct sockaddr_in servaddr;
    int fd;
    int flags;
    int reuse;

    if (NULL == dst) {
        return HILINK_SOCKET_NULL_PTR;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        hilink_error("creat socket fd failed\n");
        return HILINK_SOCKET_TCP_CONNECT_FAILED;
    }

    /*设置非阻塞模式*/
    flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        hilink_error("fcntl: %s\n", strerror(errno));
        close(fd);
        return HILINK_SOCKET_TCP_CONNECT_FAILED;
    }

    reuse = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const char*) &reuse, sizeof(reuse)) != 0) {
        close(fd);
        hilink_error("set SO_REUSEADDR failed\n");
        return HILINK_SOCKET_TCP_CONNECT_FAILED;
    }

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(dst);
    servaddr.sin_port = htons(port);

    if (connect(fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) == 0) {
        hilink_error("dst %s errno %d\n", dst, errno);
        return fd;
    } else {
        hilink_error("dst %s errno %d\n", dst, errno);

        if (errno == EINPROGRESS) {
            hilink_error("tcp conncet noblock\n");
            return fd;
        } else {
            close(fd);
            return HILINK_SOCKET_TCP_CONNECT_FAILED;
        }
    }
}

void hilink_tcp_disconnect(int fd)
{
    close(fd);
}

int hilink_tcp_send(int fd, const unsigned char* buf, unsigned short len)
{
    int ret = -1;

    if (buf == NULL) {
        return HILINK_SOCKET_NULL_PTR;
    }

    /*TCP发送数据需要判断错误码*/
    ret = (int)(send(fd, buf, len, MSG_DONTWAIT));

    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return HILINK_SOCKET_NO_ERROR;
        } else {
            hilink_error("send errno %d\n", errno);
            return HILINK_SOCKET_SEND_TCP_PACKET_FAILED;
        }
    }

    return ret;
}

int hilink_tcp_state(int fd)
{
    int errcode = HILINK_SOCKET_NO_ERROR;
    int tcp_fd = fd;

    if (tcp_fd < 0) {
        return HILINK_SOCKET_TCP_CONNECT_FAILED;
    }

    fd_set rset, wset;
    int ready_n;

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_CLR(tcp_fd, &rset);
    FD_CLR(tcp_fd, &wset);
    FD_SET(tcp_fd, &rset);
    FD_SET(tcp_fd, &wset);
    // wset = rset;

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    /*使用select机制判断tcp连接状态*/
    ready_n = select(tcp_fd + 1, &rset, &wset, NULL, &timeout);

    if (0 == ready_n) {
        hilink_error("select time out\n");
        errcode = HILINK_SOCKET_TCP_CONNECTING;
    } else if (ready_n < 0) {
        hilink_error("select error\n");
        errcode = HILINK_SOCKET_TCP_CONNECT_FAILED;
    } else {
        if (FD_ISSET(tcp_fd, &wset) != 0) {
            errcode = HILINK_SOCKET_NO_ERROR;
        } else {
            int ret;
            int len = (int) sizeof(int);;

            if (0 != getsockopt(tcp_fd, SOL_SOCKET, SO_ERROR, &ret, &len)) {
                hilink_error("getsocketopt failed\r\n");
                errcode = HILINK_SOCKET_TCP_CONNECT_FAILED;
            }

            if (0 != ret) {
                errcode = HILINK_SOCKET_TCP_CONNECT_FAILED;
            }

            errcode = HILINK_SOCKET_TCP_CONNECT_FAILED;
        }
    }

    return errcode;
}

int hilink_tcp_read(int fd, unsigned char* buf, unsigned short len)
{
    int ret = -1;

    if (buf == NULL) {
        return HILINK_SOCKET_NULL_PTR;
    }

    /*TCP读取数据需要判断错误码*/
    ret = (int)(recv(fd, buf, len, MSG_DONTWAIT));

    //printf("recv buf %s len %d\n", buf,len);
    if (ret <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return HILINK_SOCKET_NO_ERROR;
        } else {
            return HILINK_SOCKET_READ_TCP_PACKET_FAILED;
        }
    }

    return ret;
}
