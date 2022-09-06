#include "rpc_hash.h"
#include "rpc_comm.h"

rpc_hash_t *rpc_hashmgr = NULL;

#define SIZE 32

#include "wyhash.h"

void rpc_hash_mgr_init ()
{
//    uint64_t _wyp[4];
//    make_secret (time (NULL), _wyp);

    rpc_hashmgr = rpc_hash_new ();
    rpc_hash_mgr_add_entry ("add", rpc_request_add);
}

void rpc_hash_mgr_add_entry (const char *name, rpc_request_func func)
{
    rpc_hash_set (rpc_hashmgr, name, func);
}

rpc_request_func rpc_get_func (const char *name)
{
    return (rpc_request_func) rpc_hash_get (rpc_hashmgr, name);
}