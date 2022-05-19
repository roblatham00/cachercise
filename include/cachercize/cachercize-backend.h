/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_BACKEND_H
#define __CACHERCIZE_BACKEND_H

#include <cachercize/cachercize-server.h>
#include <cachercize/cachercize-common.h>

typedef cachercize_return_t (*cachercize_backend_create_fn)(cachercize_provider_t, const char*, void**);
typedef cachercize_return_t (*cachercize_backend_open_fn)(cachercize_provider_t, const char*, void**);
typedef cachercize_return_t (*cachercize_backend_close_fn)(void*);
typedef cachercize_return_t (*cachercize_backend_destroy_fn)(void*);

/**
 * @brief Implementation of an CACHERCIZE backend.
 */
typedef struct cachercize_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    cachercize_backend_create_fn   create_cache;
    cachercize_backend_open_fn     open_cache;
    cachercize_backend_close_fn    close_cache;
    cachercize_backend_destroy_fn  destroy_cache;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
    int64_t (*io)(void*, int64_t, int64_t, int64_t, int);

} cachercize_backend_impl;

/**
 * @brief Registers a backend implementation to be used by the
 * specified provider.
 *
 * Note: the backend implementation will not be copied; it is
 * therefore important that it stays valid in memory until the
 * provider is destroyed.
 *
 * @param provider provider.
 * @param backend_impl backend implementation.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h 
 */
cachercize_return_t cachercize_provider_register_backend(
        cachercize_provider_t provider,
        cachercize_backend_impl* backend_impl);

#endif
