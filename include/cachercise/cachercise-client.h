/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_CLIENT_H
#define __CACHERCISE_CLIENT_H

#include <margo.h>
#include <cachercise/cachercise-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cachercise_client* cachercise_client_t;
#define CACHERCISE_CLIENT_NULL ((cachercise_client_t)NULL)

/**
 * @brief Creates a CACHERCISE client.
 *
 * @param[in] mid Margo instance
 * @param[out] client CACHERCISE client
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_client_init(margo_instance_id mid, cachercise_client_t* client);

/**
 * @brief Finalizes a CACHERCISE client.
 *
 * @param[in] client CACHERCISE client to finalize
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_client_finalize(cachercise_client_t client);

#ifdef __cplusplus
}
#endif

#endif
