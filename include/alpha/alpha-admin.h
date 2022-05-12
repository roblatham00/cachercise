/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_ADMIN_H
#define __ALPHA_ADMIN_H

#include <margo.h>
#include <alpha/alpha-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct alpha_admin* alpha_admin_t;
#define ALPHA_ADMIN_NULL ((alpha_admin_t)NULL)

#define ALPHA_RESOURCE_ID_IGNORE ((alpha_resource_id_t*)NULL)

/**
 * @brief Creates a ALPHA admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin ALPHA admin
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_admin_init(margo_instance_id mid, alpha_admin_t* admin);

/**
 * @brief Finalizes a ALPHA admin.
 *
 * @param[in] admin ALPHA admin to finalize
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_admin_finalize(alpha_admin_t admin);

/**
 * @brief Requests the provider to create a resource of the
 * specified type and configuration and return a resource id.
 *
 * @param[in] admin ALPHA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] type type of resource to create.
 * @param[in] config Configuration.
 * @param[out] id resulting resource id.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_create_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        alpha_resource_id_t* id);

/**
 * @brief Requests the provider to open an existing resource of the
 * specified type and configuration and return a resource id.
 *
 * @param[in] admin ALPHA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] type type of resource to open.
 * @param[in] config Configuration.
 * @param[out] id resulting resource id.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_open_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        alpha_resource_id_t* id);

/**
 * @brief Requests the provider to close a resource it is managing.
 *
 * @param[in] admin ALPHA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting resource id.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_close_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        alpha_resource_id_t id);

/**
 * @brief Requests the provider to destroy a resource it is managing.
 *
 * @param[in] admin ALPHA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting resource id.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_destroy_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        alpha_resource_id_t id);

/**
 * @brief Lists the ids of resources available on the provider.
 *
 * @param[in] admin ALPHA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[out] ids array of resource ids.
 * @param[inout] count size of the array (in), number of ids returned (out).
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_list_resources(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        alpha_resource_id_t* ids,
        size_t* count);

#endif
