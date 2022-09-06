#include "rpcserver.h"

void test_server ()
{
    rpc_server_t *server = rpc_server_init ("system", ASYNC_MSGQ_BE, SYNC_MSGQ_FE);
    server->register_method (server, "add", rpc_request_add);
    server->start (server);
}