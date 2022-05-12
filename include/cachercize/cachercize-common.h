/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_COMMON_H
#define __CACHERCIZE_COMMON_H

#include <uuid.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by CACHERCIZE functions.
 */
typedef enum cachercize_return_t {
    CACHERCIZE_SUCCESS,
    CACHERCIZE_ERR_ALLOCATION,        /* Allocation error */
    CACHERCIZE_ERR_INVALID_ARGS,      /* Invalid argument */
    CACHERCIZE_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    CACHERCIZE_ERR_INVALID_CACHE,  /* Invalid cache id */
    CACHERCIZE_ERR_INVALID_BACKEND,   /* Invalid backend type */
    CACHERCIZE_ERR_INVALID_CONFIG,    /* Invalid configuration */
    CACHERCIZE_ERR_INVALID_TOKEN,     /* Invalid token */
    CACHERCIZE_ERR_FROM_MERCURY,      /* Mercurt error */
    CACHERCIZE_ERR_FROM_ARGOBOTS,     /* Argobots error */
    CACHERCIZE_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    CACHERCIZE_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    CACHERCIZE_ERR_OTHER              /* Other error */
} cachercize_return_t;

/**
 * @brief Identifier for a cache.
 */
typedef struct cachercize_cache_id_t {
    uuid_t uuid;
} cachercize_cache_id_t;

/**
 * @brief Converts a cachercize_cache_id_t into a string.
 *
 * @param id Id to convert
 * @param out[37] Resulting null-terminated string
 */
static inline void cachercize_cache_id_to_string(
        cachercize_cache_id_t id,
        char out[37]) {
    uuid_unparse(id.uuid, out);
}

/**
 * @brief Converts a string into a cachercize_cache_id_t. The string
 * should be a 36-characters string + null terminator.
 *
 * @param in input string
 * @param id resulting id
 */
static inline void cachercize_cache_id_from_string(
        const char* in,
        cachercize_cache_id_t* id) {
    uuid_parse(in, id->uuid);
}

#ifdef __cplusplus
}
#endif

#endif
