/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _CLIENT_H
#define _CLIENT_H

#include "types.h"
#include "cachercise/cachercise-client.h"
#include "cachercise/cachercise-cache.h"

typedef struct cachercise_client {
   margo_instance_id mid;
   hg_id_t           hello_id;
   hg_id_t           sum_id;
   hg_id_t           io_id;
   uint64_t          num_cache_handles;
} cachercise_client;

typedef struct cachercise_cache_handle {
    cachercise_client_t      client;
    hg_addr_t           addr;
    uint16_t            provider_id;
    uint64_t            refcount;
    cachercise_cache_id_t cache_id;
} cachercise_cache_handle;

#endif
