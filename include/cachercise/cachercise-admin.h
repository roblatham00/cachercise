/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_ADMIN_H
#define __CACHERCISE_ADMIN_H

#include <margo.h>
#include <cachercise/cachercise-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct cachercise_admin* cachercise_admin_t;
#define CACHERCISE_ADMIN_NULL ((cachercise_admin_t)NULL)

#define CACHERCISE_CACHE_ID_IGNORE ((cachercise_cache_id_t*)NULL)

/**
 * @brief Creates a CACHERCISE admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin CACHERCISE admin
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_admin_init(margo_instance_id mid, cachercise_admin_t* admin);

/**
 * @brief Finalizes a CACHERCISE admin.
 *
 * @param[in] admin CACHERCISE admin to finalize
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_admin_finalize(cachercise_admin_t admin);

/**
 * @brief Requests the provider to create a cache of the
 * specified type and configuration and return a cache id.
 *
 * @param[in] admin CACHERCISE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] type type of cache to create.
 * @param[in] config Configuration.
 * @param[out] id resulting cache id.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_create_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercise_cache_id_t* id);

/**
 * @brief Requests the provider to open an existing cache of the
 * specified type and configuration and return a cache id.
 *
 * @param[in] admin CACHERCISE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] type type of cache to open.
 * @param[in] config Configuration.
 * @param[out] id resulting cache id.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_open_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercise_cache_id_t* id);

/**
 * @brief Requests the provider to close a cache it is managing.
 *
 * @param[in] admin CACHERCISE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting cache id.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_close_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercise_cache_id_t id);

/**
 * @brief Requests the provider to destroy a cache it is managing.
 *
 * @param[in] admin CACHERCISE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting cache id.
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_destroy_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercise_cache_id_t id);

/**
 * @brief Lists the ids of caches available on the provider.
 *
 * @param[in] admin CACHERCISE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[out] ids array of cache ids.
 * @param[inout] count size of the array (in), number of ids returned (out).
 *
 * @return CACHERCISE_SUCCESS or error code defined in cachercise-common.h
 */
cachercise_return_t cachercise_list_caches(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercise_cache_id_t* ids,
        size_t* count);

#endif
