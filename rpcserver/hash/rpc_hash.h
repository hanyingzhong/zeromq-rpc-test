
//
// hash.h
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#ifndef HASH
#define HASH

#include "khash.h"

// pointer hash

KHASH_MAP_INIT_STR(ptr, void *)

/*
 * Hash type.
 */

typedef khash_t(ptr) rpc_hash_t;

/*
 * Allocate a new hash.
 */

#define rpc_hash_new() kh_init (ptr)

/*
 * Destroy the hash.
 */

#define rpc_hash_free(self) kh_destroy (ptr, self)

/*
 * Hash size.
 */

#define rpc_hash_size kh_size

/*
 * Remove all pairs in the hash.
 */

#define rpc_hash_clear(self) kh_clear (ptr, self)

/*
 * Iterate hash keys and ptrs, populating
 * `key` and `val`.
 */

#define rpc_hash_each(self, block)                                             \
    { \
   const char *key; \
   void *val; \
   khiter_t k; \
    for (k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      key = kh_key(self, k); \
      val = kh_value(self, k); \
      block; \
    } \
  }

/*
 * Iterate hash keys, populating `key`.
 */

#define rpc_hash_each_key(self, block)                                         \
    { \
    const char *key; \
    khiter_t k; \
    for (k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      key = kh_key(self, k); \
      block; \
    } \
  }

/*
 * Iterate hash ptrs, populating `val`.
 */

#define rpc_hash_each_val(self, block)                                         \
    { \
    void *val; \
    khiter_t k; \
    for (k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      val = kh_value(self, k); \
      block; \
    } \
  }

// protos

void rpc_hash_set (rpc_hash_t *self, const char *key, void *val);
void *rpc_hash_get (rpc_hash_t *self, const char *key);
int rpc_hash_has (rpc_hash_t *self, const char *key);
void rpc_hash_del (rpc_hash_t *self, const char *key);

#endif /* HASH */
