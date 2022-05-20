/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __PROVIDER_H
#define __PROVIDER_H

#include <margo.h>
#include <abt-io.h>
#include <uuid.h>
#include "cachercise/cachercise-backend.h"
#include "uthash.h"

typedef struct cachercise_cache {
    cachercise_backend_impl* fn;  // pointer to function mapping for this backend
    void*               ctx; // context required by the backend
    cachercise_cache_id_t id;  // identifier of the backend
    UT_hash_handle      hh;  // handle for uthash
} cachercise_cache;

typedef struct cachercise_provider {
    /* Margo/Argobots/Mercury environment */
    margo_instance_id  mid;                 // Margo instance
    uint16_t           provider_id;         // Provider id
    ABT_pool           pool;                // Pool on which to post RPC requests
    abt_io_instance_id abtio;               // ABT-IO instance
    char*              token;               // Security token
    /* Resources and backend types */
    size_t               num_backend_types; // number of backend types
    cachercise_backend_impl** backend_types;     // array of pointers to backend types
    size_t               num_caches;     // number of caches
    cachercise_cache*      caches;         // hash of caches by uuid
    /* RPC identifiers for admins */
    hg_id_t create_cache_id;
    hg_id_t open_cache_id;
    hg_id_t close_cache_id;
    hg_id_t destroy_cache_id;
    hg_id_t list_caches_id;
    /* RPC identifiers for clients */
    hg_id_t hello_id;
    hg_id_t sum_id;
    /* ... add other RPC identifiers here ... */
    hg_id_t io_id;
} cachercise_provider;

#endif
