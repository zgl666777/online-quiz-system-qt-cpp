#include "tcpsocket.h"

TcpServer *TcpSocket::m_tcpServer = NULL;

TcpSocket::TcpSocket(TcpServer *server, struct bufferevent *bev, char *ip, uint16_t port)
{
    m_tcpServer = server;
    m_bev = bev;   
    m_ip = ip;
    m_port = port;    
}


void TcpSocket::readEventCb(struct bufferevent *, void *ctx)
{
    TcpSocket *s = (TcpSocket*)ctx;
    m_tcpServer->readEvent(s);
}


void TcpSocket::writeEventCb(struct bufferevent *, void *ctx)
{
    TcpSocket *s = (TcpSocket*)ctx;
    m_tcpServer->writeEvent(s);
}


void TcpSocket::closeEventCb(struct bufferevent *, short what, void *ctx)
{
    TcpSocket *s = (TcpSocket*)ctx;
    m_tcpServer->closeEvent(s, what);

    delete s;
}

char *TcpSocket::getIp()
{
    return m_ip.data();
}

    
uint16_t TcpSocket::getPort()
{
    return m_port;
}


int TcpSocket::readData(void *data, int size)
{
    return bufferevent_read(m_bev, data, size);
}


int TcpSocket::writeData(const void *data, int size)
{
    return bufferevent_write(m_bev, data, size);
}

