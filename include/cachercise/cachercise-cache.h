/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_CACHE_H
#define __CACHERCISE_CACHE_H

#include <margo.h>
#include <cachercise/cachercise-common.h>
#include <cachercise/cachercise-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cachercise_cache_handle *cachercise_cache_handle_t;
#define CACHERCISE_CACHE_HANDLE_NULL ((cachercise_cache_handle_t)NULL)

/**
 * @brief Creates a CACHERCISE cache handle.
 *
 * @param[in] client CACHERCISE client responsible for the cache handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] handle cache handle
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_cache_handle_create(
        cachercise_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        cachercise_cache_id_t cache_id,
        cachercise_cache_handle_t* handle);

/**
 * @brief Increments the reference counter of a cache handle.
 *
 * @param handle cache handle
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_cache_handle_ref_incr(
        cachercise_cache_handle_t handle);

/**
 * @brief Releases the cache handle. This will decrement the
 * reference counter, and free the cache handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle cache handle to release.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_cache_handle_release(cachercise_cache_handle_t handle);

/**
 * @brief Makes the target CACHERCISE cache print Hello World.
 *
 * @param[in] handle cache handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_say_hello(cachercise_cache_handle_t handle);

/**
 * @brief Makes the target CACHERCISE cache compute the sum of the
 * two numbers and return the result.
 *
 * @param[in] handle cache handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_compute_sum(
        cachercise_cache_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

enum {
 CACHERCISE_WRITE,
 CACHERCISE_READ
};

#define cachercise_read(h, b, c, o) cachercise_io((h), (b), (c), (o), CACHERCISE_READ);
#define cachercise_write(h, b, c, o) cachercise_io((h), (b), (c), (o), CACHERCISE_WRITE);
cachercise_return_t cachercise_io(
        cachercise_cache_handle_t handle,
        void *buf,
        uint64_t count,
        int64_t offset,
        int kind);
#ifdef __cplusplus
}
#endif

#endif
