#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "mpack.h"
#include <zmq.h>
#include "rpc_comm.h"

void dump_msg1 (const void *data, int size)
{
    unsigned char *ptr = (unsigned char *) data;
    printf ("[%03d] ", size);
    int i = 0;
    for (i = 0; i < size; i++)
        printf ("%02X", ptr[i]);
    printf ("\n");
}

void msg_handler (rpc_handler_t *handler, char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;
    char cstr[128] = {0};

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root = mpack_tree_root (&tree);
    mpack_node_t func = mpack_node_array_at (root, 0);
    mpack_node_copy_cstr (func, cstr, sizeof (cstr));
    printf ("call function : %.*s\r\n", func.data->len, cstr);
    mpack_node_t paras = mpack_node_array_at (root, 1);

    rpc_request_func rpcfunc = (rpc_request_func) rpc_get_func (cstr);
    if (NULL != rpcfunc) {
        rpcfunc (handler, paras);
    } else {
        rpc_request_reply_opresult2 (handler, false, "unknown entry");
    }

    //    rpc_request_add (handler, paras);

 #if 0
    zmq_msg_t reply;

    zmq_msg_init (&reply);
    zmq_msg_init_data (&reply, "return string", strlen ("return string"), NULL,
                       NULL);
    zmq_msg_send (&reply, handler->socket, ZMQ_DONTWAIT);
    zmq_msg_close (&reply);
 #endif
}

void openrpc_msg_handler (rpc_handler_t *handler, char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;
    char cstr[128] = {0};

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root = mpack_tree_root (&tree);
    size_t length = mpack_node_array_length (root);
    int idx;

    for (idx = 0; idx < length; idx++) {
        mpack_node_t node = mpack_node_array_at (root, idx);
        if (node.data->type == mpack_type_map){
            mpack_node_t mnode = mpack_node_map_cstr (node, "message_id");
        }
    }
    mpack_node_t func = mpack_node_array_at (root, 1);
    mpack_node_copy_cstr (func, cstr, sizeof (cstr));
    printf ("call function : %.*s\r\n", func.data->len, cstr);
}

void rpcs_worker_thread (void *s)
{
    zmq_msg_t request;
    int ret;
    char *data = NULL;
    rpc_handler_t handler = {0};

    handler.socket = s;

    while (1) {
        zmq_msg_init (&request);
        ret = zmq_msg_recv (&request, s, 0);

        dump_msg1 (zmq_msg_data (&request), ret);
        msg_handler (&handler, (char *) zmq_msg_data (&request), ret);
        //openrpc_msg_handler (&handler, (char *) zmq_msg_data (&request), ret);
        zmq_msg_close (&request);
    }
    free (data);
}

void *rpcs_thread_start (void *ctx)
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

    return zmq_threadstart (rpcs_worker_thread, s);
}