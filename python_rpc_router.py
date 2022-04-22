import gevent
import zmq
import msgpack
import umsgpack


def serve_echo():
    context = zmq.Context()
    socket = context.socket(zmq.ROUTER)
    socket.bind("tcp://*:5560")
    while True:
        client_id, dest_id, msg = socket.recv_multipart()
        print("Received request: ", client_id, type(msg), msg)
        # a = umsgpack.unpackb(msg)
        a = msgpack.unpackb(msg)
        print(dest_id, a)
        # gevent.sleep(0.3)
        response = msgpack.packb({"a": "ddddddddd\0", "b": 15})
        print(response)
        # socket.send_multipart([client_id, b"", response])
        socket.send_multipart([client_id, b"", response])


def serve_route():
    context = zmq.Context()
    socket = context.socket(zmq.ROUTER)
    socket.bind("tcp://*:5560")
    while True:
        client_id, dest_id, msg = socket.recv_multipart()
        print("recv:from {} to : {} : {}", client_id, dest_id, msg)
        socket.send_multipart([dest_id, client_id, msg])


gevent.spawn(serve_route).join()
