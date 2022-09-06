#include "rpc_comm.h"

void *zmq_malloc (size_t size)
{
    void *data = malloc (size);
    printf ("apply memory %p\r\n", data);
    return data;
}

void zmq_free (void *data, void *hint)
{
    (hint);
    printf ("free memory %p\r\n", data);
    free (data);
}

#if 1

bool rpc_request_reply_opresult (rpc_handler_t *handler, bool result)
{
    mpack_writer_t writer;
    size_t size;
    char *buffer = (char *) zmq_malloc (BUFF_SIZE);

    mpack_writer_init (&writer, buffer, BUFF_SIZE);
    mpack_build_array (&writer);
    mpack_write_cstr (&writer, result ? "SUCCESS" : "FAILED");
    mpack_complete_array (&writer);
    size = mpack_writer_buffer_used (&writer);
    rpc_send_reply (handler, buffer, size);
    return true;
}

bool rpc_request_reply_opresult2 (rpc_handler_t *handler,
                                 bool result,
                                 const char *cause)
{
    mpack_writer_t writer;
    size_t size;

    char *buffer = (char *) zmq_malloc (BUFF_SIZE);
    mpack_writer_init (&writer, buffer, BUFF_SIZE);
    mpack_build_array (&writer);
    mpack_write_cstr (&writer, result ? "SUCCESS" : "FAILED");
    if (NULL != cause) {
        mpack_write_cstr (&writer, cause);
    }
    mpack_complete_array (&writer);
    size = mpack_writer_buffer_used (&writer);
    rpc_send_reply (handler, buffer, size);
    return true;
}

bool rpc_check_param_validity (mpack_node_t root,
                               mpack_type_t *param_types,
                               size_t size)
{
    mpack_type_t type = mpack_node_type (root);
    int idx;

    if (type != mpack_type_array) {
        return false;
    }

    size_t length = mpack_node_array_length (root);
    if (length != size) {
        return false;
    }

    for (idx = 0; idx < length; idx++) {
        mpack_node_t tnode = mpack_node_array_at (root, idx);

        if (tnode.data->type != param_types[idx]) {
            printf ("the %d parameter is wrong, required type is %d, actual "
                    "type is %d\r\n",
                    idx, param_types[idx], tnode.data->type);
            return false;
        }
    }

    return true;
}

#endif