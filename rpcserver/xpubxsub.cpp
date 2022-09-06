// wuproxy.c
// 源码链接: https://github.com/dongyusheng/csdn-code/blob/master/ZeroMQ/wuproxy.c
// https://blog.51cto.com/u_15346415/category7
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zmq.h>

/*
#define ASYNC_MSGQ_FE   "tcp://127.0.0.1:5555"
#define ASYNC_MSGQ_BE   "tcp://127.0.0.1:5556"

#define SYNC_MSGQ_FE    "tcp://127.0.0.1:5557"
#define SYNC_MSGQ_BE    "tcp://127.0.0.1:5558"
*/

#define ASYNC_MSGQ_FE "tcp://*:5555"
#define ASYNC_MSGQ_BE "tcp://*:5556"

#define SYNC_MSGQ_FE "tcp://*:5557"
#define SYNC_MSGQ_BE "tcp://*:5558"

#define ASYNC_MSGQ_FE_MON "inproc://ASYNC_MSGQ_FE_MON"
#define ASYNC_MSGQ_BE_MON "inproc://ASYNC_MSGQ_BE_MON"
#define SYNC_MSGQ_FE_MON  "inproc://SYNC_MSGQ_FE_MON"
#define SYNC_MSGQ_BE_MON  "inproc://SYNC_MSGQ_BE_MON"

char *zmq_strevent (int event);

void monitor_get_time (char *strtime, size_t size)
{
    time_t tt_;
    struct tm tm_;

    memset (strtime, 0, size);
    time (&tt_);              //get seconds
    localtime_s (&tm_, &tt_); //convert seconds to local time

    snprintf (strtime, size, "%d%02d%02d %02d:%02d:%02d", 1900 + tm_.tm_year,
              tm_.tm_mon + 1, tm_.tm_mday, tm_.tm_hour, tm_.tm_min, tm_.tm_sec);
}

void monitor_async_fe_event (void *monitor)
{
    char        local[512];
    char        remote[512];
    char        strtime[128];
    uint8_t    *data;
    uint16_t    event;
    size_t      size;
    int         value;

    while (1) {
        zmq_msg_t msg;

        zmq_msg_init (&msg);
        if (zmq_msg_recv (&msg, monitor, 0) == -1) {
            return;
        }
        monitor_get_time (strtime, 128);

        data  = (uint8_t *) zmq_msg_data (&msg);
        event = *(uint16_t *) (data);
        value = *(uint32_t *) (data + 2);

        if (!zmq_msg_more (&msg)) {
            continue;
        }
        zmq_msg_init (&msg);
        if (zmq_msg_recv (&msg, monitor, 0) == -1) {
            continue;
        }

        data = (uint8_t *) zmq_msg_data (&msg);
        size = zmq_msg_size (&msg);
        memcpy (local, data, size);
        local[size] = 0;

        if (!zmq_msg_more (&msg)) {
            printf ("%s: EVENT: %-37s%5d  %.*s\r\n", strtime, zmq_strevent (event),
                    value, (int) size, local);
            continue;
        }
        zmq_msg_init (&msg);
        if (zmq_msg_recv (&msg, monitor, 0) == -1) {
            continue;
        }

        data = (uint8_t *) zmq_msg_data (&msg);
        size = zmq_msg_size (&msg);
        memcpy (remote, data, size);
        remote[size] = 0;

        printf ("<%s>  %-37s%5d  %.*s  %.*s\r\n", strtime, zmq_strevent (event), 
            value, (int) size, local, (int) size, remote);
    }

    zmq_close (monitor);
}

void start_monitor_manager (void *context, void *socket, char *url)
{
    void *client_mon = zmq_socket (context, ZMQ_PAIR);

    zmq_connect (client_mon, url);
    zmq_threadstart (monitor_async_fe_event, client_mon);

    zmq_socket_monitor (socket, url, ZMQ_EVENT_ALL);
}

void msgq_fe_thread_async (void *context1)
{
    // 1.创建新的上下文
    void *context = zmq_ctx_new ();

    // 2.前端套接字, 用于连接内部的天气服务器
    void *frontend = zmq_socket (context, ZMQ_XSUB);
    zmq_bind (frontend, ASYNC_MSGQ_FE);

    // 3.后端套接字, 用来处理外部的订阅者的请求
    void *backend = zmq_socket (context, ZMQ_XPUB);
    zmq_bind (backend, ASYNC_MSGQ_BE);

    start_monitor_manager (context, frontend, ASYNC_MSGQ_FE_MON);
    start_monitor_manager (context, backend, ASYNC_MSGQ_BE_MON);

    // 4.持续运行代理
    zmq_proxy (frontend, backend, NULL);

    // 5.关闭套接字、清除上下文
    zmq_close (frontend);
    zmq_close (backend);
}

void msgq_be_thread_sync (void *context1)
{
    // 1.创建新的上下文
    void *context = zmq_ctx_new ();

    // 2.前端套接字, 用于连接内部的天气服务器
    void *frontend = zmq_socket (context, ZMQ_XSUB);
    zmq_bind (frontend, SYNC_MSGQ_FE);

    // 3.后端套接字, 用来处理外部的订阅者的请求
    void *backend = zmq_socket (context, ZMQ_XPUB);
    zmq_bind (backend, SYNC_MSGQ_BE);

    start_monitor_manager (context, frontend, SYNC_MSGQ_FE_MON);
    start_monitor_manager (context, backend,  SYNC_MSGQ_BE_MON);

    // 4.持续运行代理
    zmq_proxy (frontend, backend, NULL);

    // 5.关闭套接字、清除上下文
    zmq_close (frontend);
    zmq_close (backend);
}

void test_server ();

int main ()
{
    test_server ();

    zmq_threadstart (msgq_fe_thread_async, NULL);
    msgq_be_thread_sync(NULL);
    return 0;
}
