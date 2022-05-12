/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_SERVER_H
#define __CACHERCIZE_SERVER_H

#include <cachercize/cachercize-common.h>
#include <margo.h>
#include <abt-io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CACHERCIZE_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct cachercize_provider* cachercize_provider_t;
#define CACHERCIZE_PROVIDER_NULL ((cachercize_provider_t)NULL)
#define CACHERCIZE_PROVIDER_IGNORE ((cachercize_provider_t*)NULL)

struct cachercize_provider_args {
    const char*        token;  // Security token
    const char*        config; // JSON configuration
    ABT_pool           pool;   // Pool used to run RPCs
    abt_io_instance_id abtio;  // ABT-IO instance
    // ...
};

#define CACHERCIZE_PROVIDER_ARGS_INIT { \
    .token = NULL, \
    .config = NULL, \
    .pool = ABT_POOL_NULL, \
    .abtio = ABT_IO_INSTANCE_NULL \
}

/**
 * @brief Creates a new CACHERCIZE provider. If CACHERCIZE_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling margo_finalize.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] args argument structure
 * @param[out] provider provider
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
int cachercize_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct cachercize_provider_args* args,
        cachercize_provider_t* provider);

/**
 * @brief Destroys the Alpha provider and deregisters its RPC.
 *
 * @param[in] provider Alpha provider
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
int cachercize_provider_destroy(
        cachercize_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif
