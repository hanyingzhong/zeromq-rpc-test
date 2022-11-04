#include "zmq.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
    unsigned int  dest;
    unsigned int  src;
    unsigned int  msgid;
    unsigned char version;
    unsigned char format;
    unsigned char content[1];
}msg_head_t;

typedef struct
{
    unsigned int  id;
    unsigned char msg[0];
}inproc_msg_t;


#define ROUTER_URL "tcp://*:5555"
#define FWD_URL    "inproc://forwarder"

static void dump_zmq_msg(zmq_msg_t* msg)
{
	int   size = (int)zmq_msg_size(msg);
	const char* data = (const char*)zmq_msg_data(msg);

	printf("=====%.*s\r\n", (unsigned int)zmq_msg_size(msg), (const char*)zmq_msg_data(msg));
}

static void dump_msg(const void* data, int size)
{
	unsigned char* ptr = (unsigned char*)data;
	printf("[%03d] ", size);
	int i = 0;
	for (i = 0; i < size; i++)
		printf("%02X", ptr[i]);
	printf("\n");
}

static zmq_msg_t fromAddr;

void router_handling_msg(void* sock, void* fwd_sock)
{
	zmq_msg_t from_addr;
	zmq_msg_t data;
	zmq_msg_t data2;

	inproc_msg_t *inprocmsg = NULL;

	int       ret;
	int       rcvmore = 0;
	size_t    sz = sizeof(rcvmore);

	zmq_msg_init(&from_addr);
	zmq_msg_init(&data);
	zmq_msg_init(&data2);

/*
	char from[128];
    int len = zmq_recv (sock, from, 128, 0);
*/

	ret = zmq_msg_recv(&from_addr, sock, 0);

	zmq_getsockopt(sock, ZMQ_RCVMORE, &rcvmore, &sz);
	if (rcvmore == 0) {
		return;
	}
	dump_msg(zmq_msg_data(&from_addr), ret);
	printf("%.*s\r\n", (unsigned int)zmq_msg_size(&from_addr), (const char*)zmq_msg_data(&from_addr));
    printf ("-id- : %d\r\n", *(int *) zmq_msg_data (&from_addr));

	
	zmq_msg_move (&fromAddr, &from_addr);

	/*接收到的第一帧表示对端的地址*/
	ret = zmq_msg_recv(&data, sock, 0);

	zmq_getsockopt(sock, ZMQ_RCVMORE, &rcvmore, &sz);
	if (rcvmore == 0) {
		dump_msg(zmq_msg_data(&data), ret);
		printf("%.*s\r\n", (unsigned int)zmq_msg_size(&data), (const char*)zmq_msg_data(&data));

#if 1
		zmq_msg_send(&data, fwd_sock, 0);
#else
		/*发送的时候，先发一帧对端的地址*/
		zmq_msg_send(&from_addr, sock, ZMQ_SNDMORE);
		/*然后再发送给对端消息*/
		//memcpy(zmq_msg_data(&data), "world", strlen("world"));
		zmq_msg_copy(&data2, &data);
		zmq_msg_send(&data, sock, 0);
		//下面的代码将消息转发到处理线程，并接收响应
		zmq_msg_send(&data2, fwd_sock, 0);
#endif
        //以下可以直接接收.....相当于同步处理...........
		//zmq_msg_recv(&data2, fwd_sock, 0);
		//printf("%.*s\r\n", (unsigned int)zmq_msg_size(&data2), (const char*)zmq_msg_data(&data2));
	}

}

void create_poll_thread(void* context)
{
	const char* router_url = ROUTER_URL;
	const char* fwd_url    = FWD_URL;

	void* sock_router = zmq_socket(context, ZMQ_ROUTER);
	//zmq_bind(router, "tcp://*:5555");
	zmq_bind(sock_router, router_url);

	void* sock_fwd_reply = zmq_socket(context, ZMQ_PAIR);
	zmq_bind(sock_fwd_reply, fwd_url);

	zmq_pollitem_t items[] = {
		// 参数分别为: 轮询的套接字、轮询的本机文件句柄、要轮询的时间、轮询后返回的事件
		{ sock_router,    0, ZMQ_POLLIN, 0},
		{ sock_fwd_reply, 0, ZMQ_POLLIN, 0}
	};

	while (1)
	{
		zmq_msg_t message;
		zmq_msg_init(&message);

		// 6.轮询items, 最后一个参数为-1, 因此zmq_poll()会无限期阻塞等待事件
		zmq_poll(items, 2, -1);

		//router socket receiced message
		if (items[0].revents & ZMQ_POLLIN)
		{
			printf("=====router receive message========\r\n");
			router_handling_msg(sock_router, sock_fwd_reply);
		}

		//forward socket receiced message
		if (items[1].revents & ZMQ_POLLIN)
		{
			printf("=====socket pair receive message========\r\n");
			zmq_msg_recv(&message, sock_fwd_reply, 0);
            msg_head_t *msg_reply = (msg_head_t *) (const char *) zmq_msg_data (&message);
			printf("+++%.*s\r\n", (unsigned int)zmq_msg_size(&message), (const char*)zmq_msg_data(&message));
			//此时可以将这个message通过sock_router转发出去..........................
			//实际上要将目的的IDENTITY_ID从这个消息中拔出来，分两帧发出去，一帧是目的ID，一帧是真正的消息
            zmq_msg_t reply;

			printf ("src-id = %d\r\n", msg_reply->src);
			//发送目的ID
            //zmq_msg_send (&fromAddr, sock_router, ZMQ_SNDMORE);
			//为什么是8个字节，此处需要关注
            char addr[8] = {0};
            memcpy (addr, (void *) &msg_reply->src, 8);
            zmq_send (sock_router, addr, 8, ZMQ_SNDMORE);

			//发送具体消息
            zmq_msg_send (&message, sock_router, 0);

            zmq_msg_init (&reply);
		}

	}
}

void rpc_handling(void* sock)
{
	int         ret;

	while (1) {
		zmq_msg_t msg;
		zmq_msg_t reply;

		zmq_msg_init(&msg);
		zmq_msg_init(&reply);

		ret = zmq_msg_recv(&msg, sock, 0);
		if (ret == -1) {
			continue;
		}
		int more = zmq_msg_more(&msg);
		dump_zmq_msg(&msg);
		printf("=====%.*s\r\n", (unsigned int)zmq_msg_size(&msg), (const char*)zmq_msg_data(&msg));
        msg_head_t *msg_reply = (msg_head_t *) zmq_msg_data (&msg);

		/*
		const char* response = "reply...........";
		zmq_msg_init_size(&reply, strlen(response));
		memcpy(zmq_msg_data(&reply), response, strlen(response));
		zmq_msg_send(&reply, sock, 0);
		*/
        zmq_msg_send (&msg, sock, 0);
	}
}

void create_handling_thread(void* context, char* url)
{
	void* sock_rpc_handler = zmq_socket(context, ZMQ_PAIR);

	zmq_connect(sock_rpc_handler, url);

	zmq_threadstart(rpc_handling, sock_rpc_handler);
}

void dealer_transmitter_thread(void* url)
{
	void* context = zmq_ctx_new();
	void* dealer = zmq_socket(context, ZMQ_DEALER);
	zmq_setsockopt(dealer, ZMQ_IDENTITY, "MA1S", 8);
	zmq_connect(dealer, (const char *)url);

	int ret;
	char buf[256] = "hello";
	zmq_msg_t data;

	int msg_size = sizeof (msg_head_t) + 1024;
    msg_head_t *msg = (msg_head_t *) malloc (msg_size);
    if (msg) 
	{
        memset ((void *) msg, 0, msg_size);
        memcpy ((void *) &msg->src, "MA1S", 4);
        memcpy ((void *) &msg->content[0], "123456789012345678901234567890123456789012345678901234567890", 50);
    }

	while (1)
	{
		/*
		发送一帧，不需要发送地址帧
		根据RPC协议定义，目的地址在消息中，发送方的ID也放在消息头中
		*/
        zmq_msg_init_data (&data, msg, msg_size, NULL, NULL);
		zmq_msg_send(&data, dealer, 0);
		zmq_sleep(1);
		/*接收一帧，不会接收到地址帧*/
		ret = zmq_recv(dealer, buf, sizeof(buf), 0);
		msg_head_t *msg_reply = (msg_head_t *) buf;
		dump_msg (buf, ret);
	}
	//zmq_socket_monitor();
	zmq_close(dealer);
	zmq_ctx_destroy(context);
}

int poller_main(void)
{
	void* context = zmq_ctx_new();
	//RPC server thread
	zmq_threadstart(create_poll_thread, context);

	zmq_msg_init (&fromAddr);
	//处理RPC请求的线程
	create_handling_thread(context, FWD_URL);

	//RPC请求发送方线程
	zmq_threadstart(dealer_transmitter_thread, "tcp://127.0.0.1:5555");

	while (1)
	{
		zmq_sleep(1);
	}

	zmq_ctx_destroy(context);
	return 0;
}

