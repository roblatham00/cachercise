/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_SERVER_H
#define __CACHERCISE_SERVER_H

#include <cachercise/cachercise-common.h>
#include <margo.h>
#include <abt-io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CACHERCISE_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct cachercise_provider* cachercise_provider_t;
#define CACHERCISE_PROVIDER_NULL ((cachercise_provider_t)NULL)
#define CACHERCISE_PROVIDER_IGNORE ((cachercise_provider_t*)NULL)

struct cachercise_provider_args {
    const char*        token;  // Security token
    const char*        config; // JSON configuration
    ABT_pool           pool;   // Pool used to run RPCs
    abt_io_instance_id abtio;  // ABT-IO instance
    // ...
};

#define CACHERCISE_PROVIDER_ARGS_INIT { \
    .token = NULL, \
    .config = NULL, \
    .pool = ABT_POOL_NULL, \
    .abtio = ABT_IO_INSTANCE_NULL \
}

/**
 * @brief Creates a new CACHERCISE provider. If CACHERCISE_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling margo_finalize.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] args argument structure
 * @param[out] provider provider
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
int cachercise_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct cachercise_provider_args* args,
        cachercise_provider_t* provider);

/**
 * @brief Destroys the Alpha provider and deregisters its RPC.
 *
 * @param[in] provider Alpha provider
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
int cachercise_provider_destroy(
        cachercise_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif
