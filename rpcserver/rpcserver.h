#ifndef RPC_SERVER_H_
#define RPC_SERVER_H_

#include <zmq.h>
#include "rpc_hash.h"
#include "mpack.h"

#define ASYNC_MSGQ_FE "tcp://127.0.0.1:5555"
#define ASYNC_MSGQ_BE "tcp://127.0.0.1:5556"

#define SYNC_MSGQ_FE  "tcp://127.0.0.1:5557"
#define SYNC_MSGQ_BE  "tcp://127.0.0.1:5558"

struct rpc_server_t_;

typedef void (*rpc_request_func) (struct rpc_server_t_ *handler,
                                  mpack_node_t root);
#define RPC_SERVER_NAME_SIZE 32

typedef struct rpc_server_t_
{
    char name[RPC_SERVER_NAME_SIZE];
    void *context;
    char *sub_url;
    char *pub_url;
    void *sub;
    void *pub;
    void (*handle_req) (struct rpc_server_t_ *server);
    rpc_hash_t *rpc_hashmgr;
    int timeout;
    int (*send_async) (struct rpc_server_t_ *server,
                       char *topic,
                       size_t topic_len,
                       char *msg,
                       size_t msg_len);
    int (*send_sync) (struct rpc_server_t_ *server,
                      char *topic,
                      size_t topic_len,
                      char *msg,
                      size_t msg_len,
                      char *reply,
                      size_t reply_len);

    int (*start) (struct rpc_server_t_ *server);

    void (*register_method) (struct rpc_server_t_ *server,
                             char *method,
                             rpc_request_func func);
    void (*send_error_reply) (struct rpc_server_t_ *server, mpack_node_t root, char *cause);
    void (*send_reply) (struct rpc_server_t_ *server,
                        char *topic,
                        size_t topic_len,
                        char *msg,
                        size_t msg_len);
    void (*send_reply2) (struct rpc_server_t_ *server,
                         mpack_node_t root,
                        char *msg,
                        size_t msg_len);
} rpc_server_t;

rpc_server_t *rpc_server_init (const char *name, const char *sub_url, const char *pub_url);
void rpc_msg_get_request_id (mpack_node_t node, char *requestid, size_t *size);
bool rpc_check_request_id_validity (mpack_node_t root);
bool rpc_check_params_validity (mpack_node_t root, mpack_type_t *param_types, size_t size);

  /// <summary>
/// 
/// </summary>
/// <param name="server"></param>
/// <param name="root"></param>

void rpc_request_add (struct rpc_server_t_ *server, mpack_node_t root);

#endif