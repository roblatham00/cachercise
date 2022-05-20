/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_BACKEND_H
#define __CACHERCISE_BACKEND_H

#include <cachercise/cachercise-server.h>
#include <cachercise/cachercise-common.h>

typedef cachercise_return_t (*cachercise_backend_create_fn)(cachercise_provider_t, const char*, void**);
typedef cachercise_return_t (*cachercise_backend_open_fn)(cachercise_provider_t, const char*, void**);
typedef cachercise_return_t (*cachercise_backend_close_fn)(void*);
typedef cachercise_return_t (*cachercise_backend_destroy_fn)(void*);

/**
 * @brief Implementation of an CACHERCISE backend.
 */
typedef struct cachercise_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    cachercise_backend_create_fn   create_cache;
    cachercise_backend_open_fn     open_cache;
    cachercise_backend_close_fn    close_cache;
    cachercise_backend_destroy_fn  destroy_cache;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
    int64_t (*io)(void*, int64_t, int64_t, int64_t, int);

} cachercise_backend_impl;

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
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h 
 */
cachercise_return_t cachercise_provider_register_backend(
        cachercise_provider_t provider,
        cachercise_backend_impl* backend_impl);

#endif
