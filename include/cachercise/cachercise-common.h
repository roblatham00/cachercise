/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_COMMON_H
#define __CACHERCISE_COMMON_H

#include <uuid.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by CACHERCISE functions.
 */
typedef enum cachercise_return_t {
    CACHERCISE_SUCCESS,
    CACHERCISE_ERR_ALLOCATION,        /* Allocation error */
    CACHERCISE_ERR_INVALID_ARGS,      /* Invalid argument */
    CACHERCISE_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    CACHERCISE_ERR_INVALID_CACHE,  /* Invalid cache id */
    CACHERCISE_ERR_INVALID_BACKEND,   /* Invalid backend type */
    CACHERCISE_ERR_INVALID_CONFIG,    /* Invalid configuration */
    CACHERCISE_ERR_INVALID_TOKEN,     /* Invalid token */
    CACHERCISE_ERR_FROM_MERCURY,      /* Mercurt error */
    CACHERCISE_ERR_FROM_ARGOBOTS,     /* Argobots error */
    CACHERCISE_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    CACHERCISE_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    CACHERCISE_ERR_OTHER              /* Other error */
} cachercise_return_t;

/**
 * @brief Identifier for a cache.
 */
typedef struct cachercise_cache_id_t {
    uuid_t uuid;
} cachercise_cache_id_t;

/**
 * @brief Converts a cachercise_cache_id_t into a string.
 *
 * @param id Id to convert
 * @param out[37] Resulting null-terminated string
 */
static inline void cachercise_cache_id_to_string(
        cachercise_cache_id_t id,
        char out[37]) {
    uuid_unparse(id.uuid, out);
}

/**
 * @brief Converts a string into a cachercise_cache_id_t. The string
 * should be a 36-characters string + null terminator.
 *
 * @param in input string
 * @param id resulting id
 */
static inline void cachercise_cache_id_from_string(
        const char* in,
        cachercise_cache_id_t* id) {
    uuid_parse(in, id->uuid);
}

#ifdef __cplusplus
}
#endif

#endif
