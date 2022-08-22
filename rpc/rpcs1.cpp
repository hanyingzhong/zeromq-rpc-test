#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "mpack.h"
#include <zmq.h>

void dump_msg (const void *data, int size)
{
    unsigned char *ptr = (unsigned char *) data;
    printf ("[%03d] ", size);
    int i = 0;
    for (i = 0; i < size; i++)
        printf ("%02X", ptr[i]);
    printf ("\n");
}

void parse_request (char *data, size_t size);

void worker_thread (void *s)
{
    zmq_msg_t request;
    zmq_msg_t reply;
    int ret;
    char *data = NULL;

    while (1) {
        zmq_msg_init (&request);
        ret = zmq_msg_recv (&request, s, 0);

        dump_msg (zmq_msg_data (&request), ret);
        //parse_msg_data ((const char *) zmq_msg_data (&msg), ret);
        parse_request ((char *)zmq_msg_data (&request), ret);

        zmq_msg_init (&reply);
        zmq_msg_init_data (&reply, "return string", strlen ("return string"),
                           NULL, NULL);
        zmq_msg_send (&reply, s, ZMQ_DONTWAIT);
        zmq_msg_close (&reply);

        zmq_msg_close (&request);
    }
    free (data);
}

void *start_rpcs_thread (void *ctx)
{
    int rc;
    void *s;

    s = zmq_socket (ctx, ZMQ_REP);
    if (!s) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
        return NULL;
    }

    rc = zmq_connect (s, "tcp://127.0.0.1:40001");
    if (rc != 0) {
        printf ("error in zmq_bind: %s\n", zmq_strerror (errno));
        zmq_close (s);
        return NULL;
    }

    return zmq_threadstart (worker_thread, s);
}
