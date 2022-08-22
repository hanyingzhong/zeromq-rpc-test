#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "mpack.h"
#include "test.h"

//#pragma comment(lib, "libzmq.lib")
//C:\Users\Administrator\PycharmProjects\pythonProject\basic\rpcTest\rpcc.py
//C:\Users\Administrator\PycharmProjects\pythonProject\basic\rpcTest\rpcs.py

#ifdef TEST_ZMQ
#include <zmq.h>
void main1 ()
{
    //初始化
    void *ctx;
    ctx = zmq_init (1);
    assert (ctx);

    void *routerSock;
    routerSock = zmq_socket (ctx, ZMQ_ROUTER);
    assert (routerSock);

    void *dealerSock;
    dealerSock = zmq_socket (ctx, ZMQ_DEALER);
    assert (dealerSock);

    int rc;
    rc = zmq_bind (routerSock, "tcp://*:40000");
    assert (0 == rc);

    rc = zmq_bind (dealerSock, "tcp://*:40001");
    assert (0 == rc);
    
    //where you can create more thread.......
    //start_rpcs_thread (ctx);
    rpcs_thread_start (ctx);
    zmq_proxy (routerSock, dealerSock, NULL);

    //关闭
    zmq_close (routerSock);
    zmq_close (dealerSock);
    zmq_term (ctx);
}
#endif
