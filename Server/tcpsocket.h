#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "tcpserver.h"
#include <string>
class TcpServer;

//负责与客户端进行通信
class TcpSocket
{
public:
    TcpSocket(TcpServer *server, struct bufferevent *, char *ip, uint16_t port);


    //可读事件回调函数
    static void readEventCb(struct bufferevent *, void *ctx);

    //可写事件回调函数
    static void writeEventCb(struct bufferevent *, void *ctx);

    //异常事件回调函数
    static void closeEventCb(struct bufferevent *, short what, void *ctx);


    //获取IP
    char *getIp();

    //获取端口
    uint16_t getPort();

    //从客户端读
    int readData(void *data, int size);

    //从客户端写
    int writeData(const void *data, int size);

private:
    static TcpServer *m_tcpServer;            //服务器类对象
    struct bufferevent *m_bev;         //与客户端通信的句柄
    std::string m_ip;
    uint16_t m_port;                   //客户端使用的端口

};


#endif
