#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "rpcserver.h"

// AE implements the function........
bool add1 (float p1, float p2, char *result, size_t size)
{
    *(float *) result = p1 + p2;
    return true;
}

void rpc_encode_add_request_reply (struct rpc_server_t_ *server,
                                 float *data, char *buffer, size_t *size)
{
    mpack_writer_t writer;

    mpack_writer_init (&writer, buffer, *size);

    mpack_build_array (&writer);
    mpack_write_cstr (&writer, "SUCCESS");

    mpack_build_array (&writer);
    mpack_write_float (&writer, *(float *) data);
    mpack_complete_array (&writer);

    mpack_complete_array (&writer);

    *size = mpack_writer_buffer_used (&writer);
}


//the function check the validity of the rpc request
void rpc_request_add (struct rpc_server_t_ *server, mpack_node_t root)
{
    //define parameters type
    static mpack_type_t param_types[] = {mpack_type_double, mpack_type_double};
    char requestid[128] = {0};
    size_t requestidsize = 128;
    char buffer[256] = {0};
    size_t buff_size = 256;

    mpack_node_t params = mpack_node_array_at (root, 2);

    rpc_msg_get_request_id (mpack_node_array_at (root, 0), requestid,
                           &requestidsize);
    if (requestidsize == 0) {
        return;
    }

    //check the validity of the parameters
    bool validity = rpc_check_params_validity (params, param_types, sizeof (param_types) / sizeof (param_types[0]));
    if (false == validity) {
        server->send_error_reply (server, root, "invalid parameters.");
        return;
    }

    //call the API
    float reply;
    bool result = add1 (mpack_node_float (mpack_node_array_at (params, 0)),
                        mpack_node_float (mpack_node_array_at (params, 1)),
                       (char *) &reply, sizeof (float));

    printf ("add result is %f\r\n", reply);
    rpc_encode_add_request_reply (server, &reply, buffer, &buff_size);
    //return the call reply....
    server->send_reply (server, requestid, requestidsize, buffer, buff_size);
}
