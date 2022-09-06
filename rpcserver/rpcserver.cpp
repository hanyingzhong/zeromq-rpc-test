#include "rpcserver.h"

static int  rpc_server_start (rpc_server_t *server);
static void rpc_recv_thread (void *param);
static void rpc_handle_req (struct rpc_server_t_ *server);
static void rpc_send_reply (struct rpc_server_t_ *server,
                            char *topic,
                            size_t topic_len,
                            char *msg,
                            size_t msg_len);
static void rpc_register_method (struct rpc_server_t_ *server,
                                        char *method,
                                        rpc_request_func func);
static void rpc_send_error_reply (struct rpc_server_t_ *server,
                                  mpack_node_t root,
                                  char *cause);
static rpc_request_func rpc_get_callfunc (struct rpc_server_t_ *server,
                                          const char *name);

rpc_server_t *rpc_server_init (const char *name, const char *sub_url, const char *pub_url)
{
    rpc_server_t *server = (rpc_server_t *) malloc (sizeof (rpc_server_t));
    
    if (server == NULL) {
        return NULL;
    }
    memset ((void *)server, 0, sizeof (rpc_server_t));
    strncpy (server->name, name, RPC_SERVER_NAME_SIZE);

    server->sub_url = (sub_url == NULL) ? ASYNC_MSGQ_BE : sub_url;
    server->pub_url = (pub_url == NULL) ? SYNC_MSGQ_FE : pub_url;

    server->context = zmq_ctx_new ();
    server->sub     = zmq_socket (server->context, ZMQ_SUB);
    server->pub     = zmq_socket (server->context, ZMQ_PUB);

    server->rpc_hashmgr      = rpc_hash_new ();
    server->start            = rpc_server_start;
    server->handle_req       = rpc_handle_req;
    server->send_reply       = rpc_send_reply;
    server->send_error_reply = rpc_send_error_reply;

    server->register_method = rpc_register_method;
    return server;
}

static int rpc_server_start (rpc_server_t *server)
{
    zmq_connect (server->pub, server->pub_url);
    zmq_connect (server->sub, server->sub_url);

    zmq_setsockopt (server->sub, ZMQ_SUBSCRIBE, "1/", 2);

    rpc_recv_thread ((void *)server);
    return 0;
}

static void rpc_recv_thread (void *param)
{
    rpc_server_t *server = (rpc_server_t *) param;
    int ret;

    zmq_pollitem_t items[] = {
      {server->sub, 0, ZMQ_POLLIN, 0},
      //{server->pub, 0, ZMQ_POLLOUT, 0},
    };
    int numitems = 1;


    while (1) {
        ret = zmq_poll (items, numitems, -1);
        if (ret < 0) {
            continue;
        }
        
        if (items[0].revents & ZMQ_POLLIN) {
            if (server->handle_req) {
                server->handle_req (server);
            }
            continue;
        }
        /*
        if (items[1].revents & ZMQ_POLLOUT) {
            continue;
        }*/
    }
}

static void rpc_send_reply (struct rpc_server_t_ *server, char *topic, size_t topic_len, char *msg, size_t msg_len)
{
    zmq_msg_t reply;

    zmq_msg_init_size (&reply, topic_len);
    memcpy (zmq_msg_data (&reply), topic, topic_len);
    zmq_msg_send (&reply, server->pub, ZMQ_SNDMORE);
    zmq_msg_close (&reply);

    zmq_msg_init_size (&reply, msg_len);
    memcpy (zmq_msg_data (&reply), msg, msg_len);
    zmq_msg_send (&reply, server->pub, 0);
    zmq_msg_close (&reply);
} 

static void rpc_encode_error_request_reply (struct rpc_server_t_ *server,
                                     char *cause,
                                     char *buffer,
                                     size_t *size)
{
    mpack_writer_t writer;

    mpack_writer_init (&writer, buffer, *size);

    mpack_build_array (&writer);
    mpack_write_cstr (&writer, "ERROR");

    mpack_build_map (&writer);
    mpack_write_cstr (&writer, "cause");
    mpack_write_cstr (&writer, cause);
    mpack_write_cstr (&writer, "server");
    mpack_write_cstr (&writer, server->name);
    mpack_complete_map (&writer);

    mpack_complete_array (&writer);

    *size = mpack_writer_buffer_used (&writer);
}

static void rpc_send_error_reply (struct rpc_server_t_ *server,
                          mpack_node_t root,
                          char *cause)
{
    char requestid[128] = {0};
    size_t requestidsize = 128;

    char buffer[256];
    size_t buff_size = 256;

    rpc_msg_get_request_id (mpack_node_array_at (root, 0), requestid,
                            &requestidsize);
    if (0 == requestidsize) {
        return;
    }
    rpc_encode_error_request_reply (server, cause, buffer,
                                    &buff_size);
    server->send_reply (server, requestid, requestidsize, buffer, buff_size);
}

void rpc_msg_get_request_id (mpack_node_t node, char *requestid, size_t *size)
{
    if (node.data->type == mpack_type_map) {
        mpack_node_t requestid_node = mpack_node_map_cstr (node, "request_id");
        if (requestid_node.data->len == 0) {
            *size = 0;
            return;
        }

        mpack_node_copy_cstr (requestid_node, requestid, *size);
        *size = requestid_node.data->len;
    } else {
        *size = 0;
    }
}

void rpc_msg_handler (struct rpc_server_t_ *server, char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;
    char    cstr[128] = {0};
    char    requestid[128] = {0};
    size_t  requestidsize = 128;

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root = mpack_tree_root (&tree);
    if (root.data->type != mpack_type_array) {
        return;
    }

    mpack_node_t map = mpack_node_array_at (root, 0);
    if (map.data->type != mpack_type_map) {
        return;
    }

    mpack_node_t func = mpack_node_array_at (root, 1);
    mpack_node_copy_cstr (func, cstr, sizeof (cstr));
    printf ("call function : %.*s\r\n", func.data->len, cstr);
    mpack_node_t paras = mpack_node_array_at (root, 2);

    rpc_request_func rpcfunc = (rpc_request_func) rpc_get_callfunc (server, cstr);
    if (NULL != rpcfunc) {
        rpcfunc (server, root);
    } else {
        server->send_error_reply (server, root, "unsupported function");
    }
}

/// <summary>
/// msg: [topic, msg]
/// msg: [{"request_id":id},func_name, [params]]
/// request_id is used as the reply-topic.....
/// </summary>
/// <param name="server"></param>
static void rpc_handle_req (struct rpc_server_t_ *server)
{
    void *recvsock = server->sub;

    zmq_msg_t msg;
    size_t size;
    uint8_t *data;

    zmq_msg_init (&msg);
    if (zmq_msg_recv (&msg, recvsock, 0) == -1) {
        return;
    }
    data = (uint8_t *) zmq_msg_data (&msg);
    size = zmq_msg_size (&msg);
    printf ("topic : %.*s,  ", (unsigned int)size, data);

    if (!zmq_msg_more (&msg)) {
        return;
    }

    zmq_msg_init (&msg);
    if (zmq_msg_recv (&msg, recvsock, 0) == -1) {
        return;
    }

    data = (uint8_t *) zmq_msg_data (&msg);
    size = zmq_msg_size (&msg);
    //printf ("msg : %.*s\r\n", (unsigned int) size, data);
    rpc_msg_handler (server, (char *)data, size);
}

static void rpc_register_method (struct rpc_server_t_ *server,
                         char *method,
                         rpc_request_func func)
{
    if (server && server->rpc_hashmgr) {
        rpc_hash_set (server->rpc_hashmgr, method, func);
    }
}

static rpc_request_func rpc_get_callfunc (struct rpc_server_t_ *server,
                                   const char *name)
{
    return (rpc_request_func) rpc_hash_get (server->rpc_hashmgr,name);
}