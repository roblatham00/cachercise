/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include "cachercise/cachercise-server.h"
#include "cachercise/cachercise-client.h"
#include "cachercise/cachercise-admin.h"
#include "cachercise/cachercise-provider-handle.h"
#include "client.h"
#include <string.h>

static int cachercise_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    uint16_t provider_id  = bedrock_args_get_provider_id(args);

    struct cachercise_provider_args cachercise_args = { 0 };
    cachercise_args.config = bedrock_args_get_config(args);
    cachercise_args.pool   = bedrock_args_get_pool(args);

    cachercise_args.abtio = (abt_io_instance_id)
        bedrock_args_get_dependency(args, "abt_io", 0);

    return cachercise_provider_register(mid, provider_id, &cachercise_args,
                                   (cachercise_provider_t*)provider);
}

static int cachercise_deregister_provider(
        bedrock_module_provider_t provider)
{
    return cachercise_provider_destroy((cachercise_provider_t)provider);
}

static char* cachercise_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    // TODO
    return strdup("{}");
}

static int cachercise_init_client(
        bedrock_args_t args,
        bedrock_module_client_t* client)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    return cachercise_client_init(mid, (cachercise_client_t*)client);
}

static int cachercise_finalize_client(
        bedrock_module_client_t client)
{
    return cachercise_client_finalize((cachercise_client_t)client);
}

static char* cachercise_get_client_config(
        bedrock_module_client_t client) {
    (void)client;
    // TODO
    return strdup("{}");
}

static int cachercise_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    cachercise_client_t c = (cachercise_client_t)client;
    cachercise_provider_handle_t tmp = calloc(1, sizeof(*tmp));
    margo_addr_dup(c->mid, address, &(tmp->addr));
    tmp->provider_id = provider_id;
    *ph = (bedrock_module_provider_handle_t)tmp;
    return BEDROCK_SUCCESS;
}

static int cachercise_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    cachercise_provider_handle_t tmp = (cachercise_provider_handle_t)ph;
    margo_addr_free(tmp->mid, tmp->addr);
    free(tmp);
    return BEDROCK_SUCCESS;
}

static struct bedrock_module cachercise = {
    .register_provider       = cachercise_register_provider,
    .deregister_provider     = cachercise_deregister_provider,
    .get_provider_config     = cachercise_get_provider_config,
    .init_client             = cachercise_init_client,
    .finalize_client         = cachercise_finalize_client,
    .get_client_config       = cachercise_get_client_config,
    .create_provider_handle  = cachercise_create_provider_handle,
    .destroy_provider_handle = cachercise_destroy_provider_handle,
    .provider_dependencies   = NULL,
    .client_dependencies     = NULL
};

BEDROCK_REGISTER_MODULE(cachercise, cachercise)
