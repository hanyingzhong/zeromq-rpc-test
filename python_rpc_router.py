import gevent
import zmq
import msgpack
import umsgpack


def serve():
    context = zmq.Context()
    socket = context.socket(zmq.DEALER)
    socket.setsockopt(zmq.IDENTITY, b'SLOT12')
    socket.connect("tcp://127.0.0.1:5560")
    while True:
        client_id, msg = socket.recv_multipart()
        #print("Received request: ", client_id, type(msg), msg)
        # a = umsgpack.unpackb(msg)
        a = msgpack.unpackb(msg)
        # print(a)
        # gevent.sleep(0.3)
        response = msgpack.packb({"a": "ddddddddd\0", "b": 15})
        # print(response)
        socket.send_multipart([client_id, response])


gevent.spawn(serve).join()
