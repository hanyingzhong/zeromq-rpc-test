#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "rpcserver.h"

bool rpc_check_params_validity (mpack_node_t root,
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

/// <summary>
/// root is the original msg after topic encoded with msgpack
/// </summary>
/// <param name="root"></param>
/// <returns></returns>
bool rpc_check_request_id_validity (mpack_node_t root)
{
    char requestid[128] = {0};
    size_t requestidsize = 128;

    rpc_msg_get_request_id (mpack_node_array_at (root, 0), requestid,
                            &requestidsize);
    if (requestidsize == 0) {
        return false;
    }

    return true;
}
