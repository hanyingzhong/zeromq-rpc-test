#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "mpack.h"

//return message length
//map : json-like operation
size_t build_msg_data_new (char *data, size_t size)
{
    mpack_writer_t writer;

    mpack_writer_init (&writer, data, size);
    // write the example on the msgpack homepage
    mpack_build_map (&writer);
    mpack_write_cstr (&writer, "function");
    mpack_write_cstr (&writer, __FUNCTION__);

    mpack_write_cstr (&writer, "compact");
    mpack_write_bool (&writer, true);
    mpack_write_cstr (&writer, "schema");
    mpack_write_uint (&writer, 10);
    mpack_write_cstr (&writer, "Vref");
    mpack_write_float (&writer, 10.05);

    mpack_write_cstr (&writer, "Complex");
    mpack_build_map (&writer);
    mpack_write_cstr (&writer, "Vref");
    mpack_write_float (&writer, 1.05);
    mpack_complete_map (&writer);

    mpack_complete_map (&writer);

    size = mpack_writer_buffer_used (&writer);
    mpack_writer_destroy (&writer);

    return size;
}

void parse_msg_data_test (const char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    //mpack_node_copy_cstr

    mpack_tree_parse (&tree);
    mpack_node_t map = mpack_tree_root (&tree);
    mpack_node_t node = mpack_node_map_cstr (map, "function");
    const char *function = mpack_node_str (mpack_node_map_cstr (map, "function"));
    bool compact = mpack_node_bool (mpack_node_map_cstr (map, "compact"));
    int schema = mpack_node_uint (mpack_node_map_cstr (map, "schema"));
    float vref = mpack_node_float (mpack_node_map_cstr (map, "Vref"));
    mpack_node_t complex = mpack_node_map_cstr (map, "Complex");
    float vref2 = mpack_node_float (mpack_node_map_cstr (complex, "Vref"));
    mpack_tree_destroy (&tree);
}

// array : operation
size_t rpc_build_reset (char *data, size_t size)
{
    mpack_writer_t writer;

    mpack_writer_init (&writer, data, size);

    mpack_build_array (&writer);
    mpack_write_cstr (&writer, "function");
    mpack_write_cstr (&writer, "reset");
    mpack_complete_array (&writer);

    size = mpack_writer_buffer_used (&writer);
    return size;
}

bool rpc_parse_cmd (char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root  = mpack_tree_root (&tree);
    mpack_type_t type = mpack_node_type (root);

    if (type != mpack_type_array) {
        mpack_tree_destroy (&tree);
        return false;
    }
    size_t length = mpack_node_array_length (root);
    int idx;
    for (idx = 0; idx < length; idx++) {
        mpack_node_t tnode = mpack_node_array_at (root, idx);
        printf ("%.*s\r\n", tnode.data->len, mpack_node_data (tnode));
    }

    mpack_tree_destroy (&tree);
    return true;
}

/// <summary>
/// simpler format: read(p1,p2,p3.....)
/// </summary>
/// <param name="data"></param>
/// <param name="size"></param>
/// <returns></returns>
size_t rpc_build_multiple_simple (char *data, size_t size)
{
    mpack_writer_t writer;

    mpack_writer_init (&writer, data, size);

    mpack_write_cstr (&writer, "function");
    mpack_write_cstr (&writer, "reset");
    mpack_write_uint (&writer, 100);
    mpack_write_float (&writer, 100.1);
    mpack_write_double (&writer, 100.1);

    size = mpack_writer_buffer_used (&writer);
    return size;
}

bool rpc_parse_multi_simple (char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;
    char cstr[128];

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root = mpack_tree_root (&tree);

    mpack_node_copy_cstr (mpack_tree_root (&tree), cstr, sizeof (cstr));    

    mpack_tree_parse (&tree);
    mpack_node_copy_cstr (mpack_tree_root (&tree), cstr, sizeof (cstr));

    mpack_tree_parse (&tree);
    unsigned int result1 = mpack_node_uint (mpack_tree_root (&tree));

    mpack_tree_parse (&tree);
    float result2 = mpack_node_float (mpack_tree_root (&tree));

    mpack_tree_parse (&tree);
    double result3 = mpack_node_float (mpack_tree_root (&tree));

    mpack_tree_parse (&tree);
    mpack_node_t node1 = mpack_tree_root (&tree);
    double result4 = mpack_node_float (mpack_tree_root (&tree));

    mpack_tree_destroy (&tree);
    return true;
}

void test_mpack ()
{
    #define BUF_SIZE 1024
    char data[BUF_SIZE];

    size_t size = build_msg_data_new (data, BUF_SIZE);
    parse_msg_data_test (data, size);

    size = rpc_build_reset (data, BUF_SIZE);
    rpc_parse_cmd (data, size);
    
    size = rpc_build_multiple_simple (data, BUF_SIZE);
    rpc_parse_multi_simple (data, BUF_SIZE);
}

void parse_request (char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;
    char cstr[128];

    mpack_tree_init_pool (&tree, data, size, pool,
                          sizeof (pool) / sizeof (pool[0]));

    mpack_tree_parse (&tree);
    mpack_node_t root  = mpack_tree_root (&tree);
    mpack_node_t func  = mpack_node_array_at (root, 0);
    mpack_node_copy_cstr (func, cstr, sizeof (cstr));
    printf ("call function : %.*s\r\n", func.data->len, cstr);
    mpack_node_t paras = mpack_node_array_at (root, 1);

}
