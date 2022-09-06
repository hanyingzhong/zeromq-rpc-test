#include <zmq.h>

typedef struct
{
    int event;
    char *desc;
} ZMQ_EVENT_TABLE;

#define ZMQ_EVENT_DEF(event)                                                   \
    {                                                                          \
        event, #event                                                          \
    }

static ZMQ_EVENT_TABLE zmq_event_table[] = {
  ZMQ_EVENT_DEF (ZMQ_EVENT_CONNECTED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_CONNECT_DELAYED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_CONNECT_RETRIED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_LISTENING),
  ZMQ_EVENT_DEF (ZMQ_EVENT_BIND_FAILED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_ACCEPTED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_ACCEPT_FAILED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_CLOSED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_CLOSE_FAILED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_DISCONNECTED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_MONITOR_STOPPED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL),
  ZMQ_EVENT_DEF (ZMQ_EVENT_HANDSHAKE_SUCCEEDED),
  ZMQ_EVENT_DEF (ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL),
  ZMQ_EVENT_DEF (ZMQ_EVENT_HANDSHAKE_FAILED_AUTH)
};


char *zmq_strevent (int event)
{
    int idx;

    for (idx = 0; idx < sizeof (zmq_event_table) / sizeof (ZMQ_EVENT_TABLE);
         idx++) {
        if (event & zmq_event_table[idx].event) {
            return zmq_event_table[idx].desc;
        }
    }

    return "UNKNOWN EVENT";
}
