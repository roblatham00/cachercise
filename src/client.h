/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _CLIENT_H
#define _CLIENT_H

#include "types.h"
#include "cachercize/cachercize-client.h"
#include "cachercize/cachercize-cache.h"

typedef struct cachercize_client {
   margo_instance_id mid;
   hg_id_t           hello_id;
   hg_id_t           sum_id;
   uint64_t          num_cache_handles;
} cachercize_client;

typedef struct cachercize_cache_handle {
    cachercize_client_t      client;
    hg_addr_t           addr;
    uint16_t            provider_id;
    uint64_t            refcount;
    cachercize_cache_id_t cache_id;
} cachercize_cache_handle;

#endif
