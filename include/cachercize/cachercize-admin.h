/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_ADMIN_H
#define __CACHERCIZE_ADMIN_H

#include <margo.h>
#include <cachercize/cachercize-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct cachercize_admin* cachercize_admin_t;
#define CACHERCIZE_ADMIN_NULL ((cachercize_admin_t)NULL)

#define CACHERCIZE_CACHE_ID_IGNORE ((cachercize_cache_id_t*)NULL)

/**
 * @brief Creates a CACHERCIZE admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin CACHERCIZE admin
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_admin_init(margo_instance_id mid, cachercize_admin_t* admin);

/**
 * @brief Finalizes a CACHERCIZE admin.
 *
 * @param[in] admin CACHERCIZE admin to finalize
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_admin_finalize(cachercize_admin_t admin);

/**
 * @brief Requests the provider to create a cache of the
 * specified type and configuration and return a cache id.
 *
 * @param[in] admin CACHERCIZE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] type type of cache to create.
 * @param[in] config Configuration.
 * @param[out] id resulting cache id.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_create_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercize_cache_id_t* id);

/**
 * @brief Requests the provider to open an existing cache of the
 * specified type and configuration and return a cache id.
 *
 * @param[in] admin CACHERCIZE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] type type of cache to open.
 * @param[in] config Configuration.
 * @param[out] id resulting cache id.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_open_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercize_cache_id_t* id);

/**
 * @brief Requests the provider to close a cache it is managing.
 *
 * @param[in] admin CACHERCIZE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting cache id.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_close_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercize_cache_id_t id);

/**
 * @brief Requests the provider to destroy a cache it is managing.
 *
 * @param[in] admin CACHERCIZE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting cache id.
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_destroy_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercize_cache_id_t id);

/**
 * @brief Lists the ids of caches available on the provider.
 *
 * @param[in] admin CACHERCIZE admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[out] ids array of cache ids.
 * @param[inout] count size of the array (in), number of ids returned (out).
 *
 * @return CACHERCIZE_SUCCESS or error code defined in cachercize-common.h
 */
cachercize_return_t cachercize_list_caches(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercize_cache_id_t* ids,
        size_t* count);

#endif
