#include "thread.h"

Thread::Thread()
{
    m_base = event_base_new();
    if(!m_base)
    {
        printf("Couldn't create an event_base: exiting!\n");
        exit(-1);
    }

    //创建管道
    int fd[2];
    if(pipe(fd) == -1)
    {
        perror("pipe");
        exit(-1);
    }

    m_pipeReadFd = fd[0];
    m_pipeWriteFd = fd[1];

    //管道事件监听管道的读端
    event_set(&m_pipeEvent, m_pipeReadFd, EV_READ|EV_PERSIST, pipeRead, this);

    //将事件添加到m_base的集合中
    event_base_set(m_base, &m_pipeEvent);

    //开启事件的监听
    event_add(&m_pipeEvent, 0);

}

void Thread::pipeRead(evutil_socket_t, short, void *)
{


}



void Thread::start()
{
    //创建一个线程
    pthread_create(&m_threadId, NULL, worker, this);

    //线程分离
    pthread_detach(m_threadId);
}

void* Thread::worker(void* arg)
{
    Thread* p = (Thread*)arg;
    p->run();

    return NULL;
}

void Thread::run()
{
    // while(1)
    // {
    //     printf("thread:%d\n", m_threadId);
    //     sleep(2);
    // }

    //监听base事件集合
    //printf("%d: start\n", m_threadId);
    event_base_dispatch(m_base);
    event_base_free(m_base);

    //printf("%d: done\n", m_threadId);
}

struct event_base *Thread::getBase()
{
    return m_base;
}

