#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "thread.h"
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include "tcpsocket.h"

class TcpSocket;
//TCP服务基类
class TcpServer
{
    friend class TcpSocket;
public:
    TcpServer(int threadNum = 8);

    int listen(int port, const char *ip = NULL);

    //服务器开始运行
    void start();

    

protected:
    //监听回调函数
    static void listenCb(struct evconnlistener *, evutil_socket_t fd, struct sockaddr *clientAdd, int socklen, void *data);

    //监听处理函数
    void listenEvent(evutil_socket_t fd, struct sockaddr_in *clientAddr);


    //虚函数 具体处理客户端的逻辑
    //客户端连接事件
    virtual void connectEvent(TcpSocket *){}

    //客户可读
    virtual void readEvent(TcpSocket *){}

    //客户可写
    virtual void writeEvent(TcpSocket *){}

    //关闭事件
    virtual void closeEvent(TcpSocket *, short ){}




private:
    int m_threadNum;                     //线程个数
    Thread *m_threadPool;                //线程池
    
    struct event_base *m_base;    
    struct evconnlistener *m_listener;   //监听客户端的连接

    int m_nextThread;                    //下一个线程
    

};













#endif 
