#include <assert.h>
#include "rpc_hash.h"

void my_test_hash_set ()
{
    rpc_hash_t *hash = rpc_hash_new ();
    assert (0 == rpc_hash_size (hash));

    rpc_hash_set (hash, "name", "tobi");
    rpc_hash_set (hash, "species", "ferret");
    assert (2 == rpc_hash_size (hash));

    assert (0 == strcmp ("tobi", (const char *) rpc_hash_get (hash, "name")));
    assert (
      0 == strcmp ("ferret", (const char *) rpc_hash_get (hash, "species")));
}
