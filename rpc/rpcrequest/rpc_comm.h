#pragma once
#ifndef __RPC_COMMON__
#define __RPC_COMMON__

#include <assert.h>
#include <iostream>
#include <tchar.h>
#include <zmq.h>
#include "mpack.h"

#define FUNCNAME_SIZE   64
#define NODE_NUM        128
#define BUFF_SIZE       128
#define TID_SIZE        (32 + 1)

typedef struct
{
    mpack_type_t type;
    char *data;
    uint32_t size;
} rpc_result_params_t;

typedef struct
{
    void *socket;
    char tid[TID_SIZE];
    int  tid_size;
} rpc_handler_t;

typedef void (*rpc_request_func) (rpc_handler_t *handler, mpack_node_t root);

void *zmq_malloc (size_t size);
void  zmq_free (void *data, void *hint);

bool rpc_request_reply_opresult (rpc_handler_t *handler, bool result);
bool rpc_request_reply_opresult2 (rpc_handler_t *handler,
                                  bool result,
                                  const char *cause);

void rpc_send_reply (rpc_handler_t *handler, char *data, size_t size);

bool rpc_check_param_validity (mpack_node_t root,
                               mpack_type_t *param_types,
                               size_t size);
void rpc_request_add (rpc_handler_t *handler, mpack_node_t root);

void rpc_hash_mgr_init ();
void rpc_hash_mgr_add_entry (const char *name, rpc_request_func func);
rpc_request_func rpc_get_func (const char *name);

#endif
