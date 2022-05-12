/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_CLIENT_H
#define __CACHERCIZE_CLIENT_H

#include <margo.h>
#include <cachercize/cachercize-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cachercize_client* cachercize_client_t;
#define CACHERCIZE_CLIENT_NULL ((cachercize_client_t)NULL)

/**
 * @brief Creates a CACHERCIZE client.
 *
 * @param[in] mid Margo instance
 * @param[out] client CACHERCIZE client
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_client_init(margo_instance_id mid, cachercize_client_t* client);

/**
 * @brief Finalizes a CACHERCIZE client.
 *
 * @param[in] client CACHERCIZE client to finalize
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_client_finalize(cachercize_client_t client);

#ifdef __cplusplus
}
#endif

#endif
