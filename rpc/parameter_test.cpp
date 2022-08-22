#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "mpack.h"

#if 0

#define FUNCNAME_SIZE   64
#define NODE_NUM        128
#define BUFF_SIZE       128
#define TID_SIZE        (32+1)

  //from another header file
bool add (float p1, float p2, char *result, size_t size);

typedef struct
{
    mpack_type_t type;
    char *data;
    uint32_t size;
}rpc_result_params_t;

typedef struct
{
    void *socket;
    char tid[TID_SIZE];
    int tid_size;
}rpc_handler_t;

bool rpc_request_reply_opresult (rpc_handler_t *handler, bool result);

void rpc_send_add_request_reply (rpc_handler_t *handler,
                                 char *data,
                                 size_t size);

size_t rpc_build_response (char *buffer, size_t buf_size, rpc_result_params_t *params, size_t numOfparams)
{
    mpack_writer_t writer;
    size_t size;
    int idx;

    mpack_writer_init (&writer, buffer, buf_size);
    mpack_build_array (&writer);

    for (idx = 0; idx < numOfparams; idx++) {    
        if (params->type == mpack_type_int) {
            mpack_write_int (&writer, (int64_t) params->data);    
        }
        if (params->type == mpack_type_float) {
            mpack_write_float (&writer, (float)atof (params->data));
        }
        if (params->type == mpack_type_str) {
            mpack_write_cstr (&writer, (const char *) params->data);
        }
        if (params->type == mpack_type_bin) {
            mpack_write_bin (&writer, (const char *) params->data, params->size);
        }
    }
    
    mpack_complete_array (&writer);
    size = mpack_writer_buffer_used (&writer);
    return size;
}

bool rpc_check_param_validity (mpack_node_t root,
                               mpack_type_t *param_types,
                               size_t size)
{
    mpack_type_t type = mpack_node_type (root);
    int idx;

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

//the function check the validity of the rpc request
void rpc_request_add (rpc_handler_t *handler, mpack_node_t root)
{
    //define parameters type
    static mpack_type_t param_types[] = {mpack_type_float, mpack_type_float};

    //check the validity of the parameters
    bool validity = rpc_check_param_validity (root, param_types,
                              sizeof (param_types) / sizeof (param_types[0]));
    if (false == validity) {
        rpc_request_reply_opresult (handler, false);
        return;
    }

    //call the API
    float reply;
    bool result = add (mpack_node_float (mpack_node_array_at (root, 0)),
           mpack_node_float (mpack_node_array_at (root, 1)), (char *)&reply, sizeof(float));

    printf ("add result is %f\r\n", reply);

    //return the call reply....
    rpc_send_add_request_reply (handler, (char *)&reply, sizeof (reply));
}

void rpc_send_reply (rpc_handler_t *handler, char *data, size_t size)
{
    printf ("send reply.....\r\n");
}

void rpc_send_add_request_reply (rpc_handler_t *handler, char *data, size_t length)
{
    mpack_writer_t writer;
    size_t size;
    char buffer[BUFF_SIZE] = {0};

    mpack_writer_init (&writer, buffer, BUFF_SIZE);
    mpack_write_cstr (&writer, "SUCCESS");
    mpack_build_array (&writer);
    mpack_write_float (&writer, *(float *)data);
    mpack_complete_array (&writer);

    size = mpack_writer_buffer_used (&writer);
    rpc_send_reply (handler, buffer, size);
}

bool rpc_request_reply_opresult (rpc_handler_t *handler, bool result)
{
    mpack_writer_t writer;
    size_t size;
    char buffer[BUFF_SIZE] = {0};

    mpack_writer_init (&writer, buffer, BUFF_SIZE);
    mpack_write_cstr (&writer, result ? "SUCCESS" : "FAILED");
    size = mpack_writer_buffer_used (&writer);
    rpc_send_reply (handler, buffer, size);
    return true;
}

// AE implements the function........
bool add (float p1, float p2, char *result, size_t size)
{
    *(float *) result = p1 + p2;
    return true;
}

//synchronized in receive thread
void handle_rpc_request (rpc_handler_t *handler, char *msg, size_t size)
{
    mpack_node_data_t pool[NODE_NUM];
    mpack_tree_t tree;
    char function_name[FUNCNAME_SIZE] = {0};

    mpack_tree_init_pool (&tree, msg, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root = mpack_tree_root (&tree);
    mpack_type_t type = mpack_node_type (root);

    if (type != mpack_type_str) {
        mpack_tree_destroy (&tree);
        return;
    }

    //get function name
    mpack_node_copy_data (root, function_name, FUNCNAME_SIZE);

    mpack_tree_parse (&tree);
    //find the entry of the function by function name.
    rpc_request_add (handler, mpack_tree_root (&tree));

    mpack_tree_destroy (&tree);
}
/// <summary>
/// //////////////////////////////////////////////////////////////////////////
/// </summary>
///  

size_t rpc_build_add_request (char *data, size_t size)
{
    mpack_writer_t writer;

    mpack_writer_init (&writer, data, size);
    mpack_write_cstr (&writer, "add");

    mpack_build_array (&writer);
    mpack_write_float (&writer, 100.1);
    mpack_write_float (&writer, 200.2);
    mpack_complete_array (&writer);

    size = mpack_writer_buffer_used (&writer);
    return size;
}


void test_rpc_request ()
{
    char buffer[BUFF_SIZE];
    rpc_handler_t handler;

    size_t size = rpc_build_add_request (buffer, BUFF_SIZE);

    handle_rpc_request (&handler, buffer, size);

}

#endif
