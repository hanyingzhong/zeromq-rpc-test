#!/usr/bin/python
# -*-coding:utf-8-*-

import zmq
import time
import threading
import msgpack

# void zmq::xpub_t::xread_activated (pipe_t *pipe_) handling subscribe.....
# bool generic_mtrie_t<T>::add (prefix_t prefix_, size_t size_, value_t *pipe_)
# D:\source\libzmq-4.3.4\libzmq-4.3.4\src\generic_mtrie_impl.hpp

context = zmq.Context()
req = context.socket(zmq.PUB)
req.connect("tcp://127.0.0.1:5555")


def receive_reply():
    context1 = zmq.Context()
    rep = context1.socket(zmq.SUB)
    rep.setsockopt_string(zmq.SUBSCRIBE, 'REQ10000')
    rep.connect("tcp://127.0.0.1:5558")
    while True:
        msg = rep.recv_multipart()
        print(msg)


def get_reply_socket():
    context1 = zmq.Context()
    rep_sock = context1.socket(zmq.SUB)
    rep_sock.setsockopt(zmq.LINGER, 0)
    rep_sock.setsockopt(zmq.RCVTIMEO, 2000)
    rep_sock.setsockopt_string(zmq.SUBSCRIBE, 'REQ10000')
    rep_sock.setsockopt_string(zmq.SUBSCRIBE, 'handshake')
    rep_sock.connect("tcp://127.0.0.1:5558")
    return rep_sock


if __name__ == '__main__':
    # th = threading.Thread(target=receive_reply, args=())
    # th.start()
    rep = get_reply_socket()
    # wait rep socket connect to server
    time.sleep(1)

    while True:
        request_id = 'REQ10003'
        request = msgpack.packb([{'request_id': request_id}, "add", [100.1, 200.2]])
        print(request)
        req.send_multipart((b'1/0', request))
        rep.setsockopt_string(zmq.SUBSCRIBE, request_id)
        try:
            for idx in range(1):
                msg = rep.recv_multipart()
                print(msg[0], msgpack.unpackb(msg[1]))
                rep.setsockopt_string(zmq.UNSUBSCRIBE, request_id)
        except zmq.Again:
            print("=====wait response expired========")
        time.sleep(1)
