#include "rpc_hash.h"

/*
 * Set hash `key` to `val`.
 */

void rpc_hash_set (rpc_hash_t *self, const char *key, void *val)
{
    int ret;
    khiter_t k = kh_put(ptr, self, key, &ret);
    kh_value(self, k) = val;
}

/*
 * Get hash `key`, or NULL.
 */

void *rpc_hash_get (rpc_hash_t *self, const char *key)
{
    khiter_t k = kh_get(ptr, self, key);
    return k == kh_end(self) ? NULL : kh_value(self, k);
}

/*
 * Check if hash `key` exists.
 */

int rpc_hash_has (rpc_hash_t *self, const char *key)
{
    if (!rpc_hash_size (self))
        return 0;
    khiter_t k = kh_get(ptr, self, key);
    return k != kh_end(self);
}

/*
 * Remove hash `key`.
 */

void rpc_hash_del (rpc_hash_t *self, const char *key)
{
    khiter_t k = kh_get(ptr, self, key);
    kh_del(ptr, self, k);
}
