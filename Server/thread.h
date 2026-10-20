#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>

class Thread
{
public:
    Thread();

    void start();                 //线程运行函数
    
    //获取事件集合
    struct event_base *getBase();
    
protected:
    static void* worker(void*);  //线程的工作函数
    static void pipeRead(evutil_socket_t, short, void *);


    void run();                  //线程逻辑处理函数                                   
private:
    struct event_base *m_base;   //事件集合

    pthread_t m_threadId;        //线程ID
    
    int m_pipeReadFd;            //管道的读端
    int m_pipeWriteFd;           //管道的写端       
    struct event m_pipeEvent;    

};









#endif
