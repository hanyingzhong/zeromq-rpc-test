#include <assert.h>
#include <iostream>
#include <tchar.h>
#include <zmq.h>
#include "mpack.h"
#include "rpc_comm.h"

//#C :\Users\Administrator\PycharmProjects\pythonProject\basic\rpcTest\rpcc_msgpack \
//  .py

#if 0
import zmq
import msgpack
import time

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://127.0.0.1:40000")
idx = 0
while True:
#request = msgpack.packb([ "add", {"a" : "ddddddddd\0", "b" : 15} ])
    request = msgpack.packb(["add", [100.1, 200.2]])
    socket.send_multipart([request])
    message = socket.recv_multipart()
#print(type(message), message)
    print(msgpack.unpackb(message[0]))

#endif

// AE implements the function........
bool add (float p1, float p2, char *result, size_t size)
{
    *(float *) result = p1 + p2;
    return true;
}

void rpc_send_add_request_reply (rpc_handler_t *handler,
                                 char *data,
                                 size_t length)
{
    mpack_writer_t writer;
    size_t size;
    //char buffer[BUFF_SIZE] = {0};

    char *buffer = (char *) zmq_malloc (BUFF_SIZE);
    mpack_writer_init (&writer, buffer, BUFF_SIZE);

    mpack_build_array (&writer);
    mpack_write_cstr (&writer, "SUCCESS");

    mpack_build_array (&writer);
    mpack_write_float (&writer, *(float *) data);
    mpack_complete_array (&writer);

    mpack_complete_array (&writer);

    size = mpack_writer_buffer_used (&writer);
    rpc_send_reply (handler, buffer, size);
}

//the function check the validity of the rpc request
void rpc_request_add (rpc_handler_t *handler, mpack_node_t root)
{
    //define parameters type
    static mpack_type_t param_types[] = {mpack_type_double, mpack_type_double};

    //check the validity of the parameters
    bool validity = rpc_check_param_validity (
      root, param_types, sizeof (param_types) / sizeof (param_types[0]));
    if (false == validity) {
        rpc_request_reply_opresult (handler, false);
        return;
    }

    //call the API
    float reply;
    bool result = add (mpack_node_float (mpack_node_array_at (root, 0)),
                       mpack_node_float (mpack_node_array_at (root, 1)),
                       (char *) &reply, sizeof (float));

    printf ("add result is %f\r\n", reply);

    //return the call reply....
    rpc_send_add_request_reply (handler, (char *)&reply, sizeof (reply));
}

void rpc_send_reply (rpc_handler_t *handler, char *data, size_t size)
{
    zmq_msg_t reply;

    zmq_msg_init (&reply);
    zmq_msg_init_data (&reply, data, size, zmq_free, NULL);
    zmq_msg_send (&reply, handler->socket, ZMQ_DONTWAIT);
    zmq_msg_close (&reply);
    printf ("send reply.....\r\n");
}
