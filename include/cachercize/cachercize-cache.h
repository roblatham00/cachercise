/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_CACHE_H
#define __CACHERCIZE_CACHE_H

#include <margo.h>
#include <cachercize/cachercize-common.h>
#include <cachercize/cachercize-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cachercize_cache_handle *cachercize_cache_handle_t;
#define CACHERCIZE_CACHE_HANDLE_NULL ((cachercize_cache_handle_t)NULL)

/**
 * @brief Creates a CACHERCIZE cache handle.
 *
 * @param[in] client CACHERCIZE client responsible for the cache handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] handle cache handle
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_cache_handle_create(
        cachercize_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        cachercize_cache_id_t cache_id,
        cachercize_cache_handle_t* handle);

/**
 * @brief Increments the reference counter of a cache handle.
 *
 * @param handle cache handle
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_cache_handle_ref_incr(
        cachercize_cache_handle_t handle);

/**
 * @brief Releases the cache handle. This will decrement the
 * reference counter, and free the cache handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle cache handle to release.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_cache_handle_release(cachercize_cache_handle_t handle);

/**
 * @brief Makes the target CACHERCIZE cache print Hello World.
 *
 * @param[in] handle cache handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_say_hello(cachercize_cache_handle_t handle);

/**
 * @brief Makes the target CACHERCIZE cache compute the sum of the
 * two numbers and return the result.
 *
 * @param[in] handle cache handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_compute_sum(
        cachercize_cache_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

enum {
 CACHERCIZE_WRITE,
 CACHERCIZE_READ
};

#define cachercize_read(h, b, c, o) cachercize_io((h), (b), (c), (o), CACHERCIZE_READ);
#define cachercize_write(h, b, c, o) cachercize_io((h), (b), (c), (o), CACHERCIZE_WRITE);
cachercize_return_t cachercize_io(
        cachercize_cache_handle_t handle,
        void *buf,
        uint64_t count,
        int64_t offset,
        int kind);
#ifdef __cplusplus
}
#endif

#endif
