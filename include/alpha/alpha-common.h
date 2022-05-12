/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_COMMON_H
#define __ALPHA_COMMON_H

#include <uuid.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by ALPHA functions.
 */
typedef enum alpha_return_t {
    ALPHA_SUCCESS,
    ALPHA_ERR_ALLOCATION,        /* Allocation error */
    ALPHA_ERR_INVALID_ARGS,      /* Invalid argument */
    ALPHA_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    ALPHA_ERR_INVALID_RESOURCE,  /* Invalid resource id */
    ALPHA_ERR_INVALID_BACKEND,   /* Invalid backend type */
    ALPHA_ERR_INVALID_CONFIG,    /* Invalid configuration */
    ALPHA_ERR_INVALID_TOKEN,     /* Invalid token */
    ALPHA_ERR_FROM_MERCURY,      /* Mercurt error */
    ALPHA_ERR_FROM_ARGOBOTS,     /* Argobots error */
    ALPHA_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    ALPHA_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    ALPHA_ERR_OTHER              /* Other error */
} alpha_return_t;

/**
 * @brief Identifier for a resource.
 */
typedef struct alpha_resource_id_t {
    uuid_t uuid;
} alpha_resource_id_t;

/**
 * @brief Converts a alpha_resource_id_t into a string.
 *
 * @param id Id to convert
 * @param out[37] Resulting null-terminated string
 */
static inline void alpha_resource_id_to_string(
        alpha_resource_id_t id,
        char out[37]) {
    uuid_unparse(id.uuid, out);
}

/**
 * @brief Converts a string into a alpha_resource_id_t. The string
 * should be a 36-characters string + null terminator.
 *
 * @param in input string
 * @param id resulting id
 */
static inline void alpha_resource_id_from_string(
        const char* in,
        alpha_resource_id_t* id) {
    uuid_parse(in, id->uuid);
}

#ifdef __cplusplus
}
#endif

#endif
