#include "../include/zmq.h"
#include <stdio.h>
#include <stdlib.h>
#include "mpack/mpack.h"

int build_msg_data(char** data, size_t *size)
{
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, data, size);

    // write the example on the msgpack homepage
    mpack_build_map(&writer);

    mpack_write_cstr(&writer, "compact");
    mpack_write_bool(&writer, true);
    mpack_write_cstr(&writer, "schema");
    mpack_write_uint(&writer, 10);
    mpack_write_cstr(&writer, "Vref");
    mpack_write_float(&writer, 10.05);

    mpack_write_cstr(&writer, "Complex");
    mpack_build_map(&writer);
    mpack_write_cstr(&writer, "Vref");
    mpack_write_float(&writer, 1.05);
    mpack_complete_map(&writer);

    mpack_complete_map(&writer);

    if (mpack_writer_destroy(&writer) != mpack_ok) {
        fprintf(stderr, "An error occurred encoding the data!\n");
        return -1;
    }

    return 0;
}

typedef struct
{
    mpack_error_t error;
}decode_context_t;

void decode_error_handler(mpack_tree_t* tree, mpack_error_t error)
{
    if (tree->context) {
        ((decode_context_t*)tree->context)->error = error;
    }
}

void dump_msg(const void* data, int size)
{
    unsigned char* ptr = (unsigned char*)data;
    printf("[%03d] ", size);
    int i = 0;
    for (i = 0; i < size; i++)
        printf("%02X", ptr[i]);
    printf("\n");
}

void parse_msg_data(const char *data, size_t size)
{
    mpack_node_data_t pool[128];
    mpack_tree_t tree;
    decode_context_t context;

    mpack_tree_init_pool(&tree, data, size, pool, sizeof(pool) / sizeof(*pool));
    mpack_tree_set_context(&tree, &context);
    mpack_tree_set_error_handler(&tree, decode_error_handler);

    mpack_tree_parse(&tree);
    mpack_node_t map = mpack_tree_root(&tree);
    const char *a = mpack_node_str(mpack_node_map_cstr(map, "a"));
    dump_msg(a, mpack_node_map_cstr(map, "a").data->len);
    int b = mpack_node_u8(mpack_node_map_cstr(map, "b"));

    mpack_tree_destroy(&tree);
}

int main(int argc, char* argv[])
{
    const char* bind_to;
    int roundtrip_count;
    size_t message_size;
    void* ctx;
    void* s;
    int rc;
    int i;
    int ret;
    zmq_msg_t msg;
    char* data = NULL;
    size_t size;

    build_msg_data(&data, &size);

    if (argc != 2) {
        printf("usage: local_lat <connect-to> [default = tcp://127.0.0.1:5560]\n");
        bind_to = "tcp://127.0.0.1:5560";
    }
    else{
        bind_to = argv[1];
    }
    
    ctx = zmq_init(1);
    if (!ctx) {
        printf("error in zmq_init: %s\n", zmq_strerror(errno));
        return -1;
    }

    s = zmq_socket(ctx, ZMQ_DEALER);
    if (!s) {
        printf("error in zmq_socket: %s\n", zmq_strerror(errno));
        return -1;
    }

    rc = zmq_connect(s, bind_to);
    if (rc != 0) {
        printf("error in zmq_bind: %s\n", zmq_strerror(errno));
        return -1;
    }

    zmq_msg_init(&msg);
    char* dest = "SLOT12";
    zmq_msg_init_data(&msg, dest, strlen(dest), NULL, NULL);

	rc = zmq_msg_send(&msg, s, ZMQ_SNDMORE);
	if (rc < 0) {
		printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
		return -1;
	}
    zmq_msg_close(&msg);

    zmq_msg_init(&msg);
    zmq_msg_init_data(&msg, data, size, NULL, NULL);
    zmq_msg_send(&msg, s, 0);
    rc = zmq_msg_close(&msg);

	zmq_sleep(1);
	zmq_sleep(1);
	zmq_msg_init(&msg);
	ret = zmq_msg_recv(&msg, s, 0);
	dump_msg(zmq_msg_data(&msg), ret);

    ret = zmq_msg_recv(&msg, s, 0);
    dump_msg(zmq_msg_data(&msg), ret);
    parse_msg_data((const char *)zmq_msg_data(&msg), ret);

    rc = zmq_msg_close(&msg);
    if (rc != 0) {
        printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
        return -1;
    }

    zmq_sleep(1);
    free(data);

    rc = zmq_close(s);
    if (rc != 0) {
        printf("error in zmq_close: %s\n", zmq_strerror(errno));
        return -1;
    }

    rc = zmq_ctx_term(ctx);
    if (rc != 0) {
        printf("error in zmq_ctx_term: %s\n", zmq_strerror(errno));
        return -1;
    }

    return 0;
}
