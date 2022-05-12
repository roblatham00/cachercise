/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_BACKEND_H
#define __ALPHA_BACKEND_H

#include <alpha/alpha-server.h>
#include <alpha/alpha-common.h>

typedef alpha_return_t (*alpha_backend_create_fn)(alpha_provider_t, const char*, void**);
typedef alpha_return_t (*alpha_backend_open_fn)(alpha_provider_t, const char*, void**);
typedef alpha_return_t (*alpha_backend_close_fn)(void*);
typedef alpha_return_t (*alpha_backend_destroy_fn)(void*);

/**
 * @brief Implementation of an ALPHA backend.
 */
typedef struct alpha_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    alpha_backend_create_fn   create_resource;
    alpha_backend_open_fn     open_resource;
    alpha_backend_close_fn    close_resource;
    alpha_backend_destroy_fn  destroy_resource;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
} alpha_backend_impl;

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
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h 
 */
alpha_return_t alpha_provider_register_backend(
        alpha_provider_t provider,
        alpha_backend_impl* backend_impl);

#endif
