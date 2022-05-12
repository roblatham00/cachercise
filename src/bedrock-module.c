/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include "cachercize/cachercize-server.h"
#include "cachercize/cachercize-client.h"
#include "cachercize/cachercize-admin.h"
#include "cachercize/cachercize-provider-handle.h"
#include "client.h"
#include <string.h>

static int cachercize_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    uint16_t provider_id  = bedrock_args_get_provider_id(args);

    struct cachercize_provider_args cachercize_args = { 0 };
    cachercize_args.config = bedrock_args_get_config(args);
    cachercize_args.pool   = bedrock_args_get_pool(args);

    cachercize_args.abtio = (abt_io_instance_id)
        bedrock_args_get_dependency(args, "abt_io", 0);

    return cachercize_provider_register(mid, provider_id, &cachercize_args,
                                   (cachercize_provider_t*)provider);
}

static int cachercize_deregister_provider(
        bedrock_module_provider_t provider)
{
    return cachercize_provider_destroy((cachercize_provider_t)provider);
}

static char* cachercize_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    // TODO
    return strdup("{}");
}

static int cachercize_init_client(
        bedrock_args_t args,
        bedrock_module_client_t* client)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    return cachercize_client_init(mid, (cachercize_client_t*)client);
}

static int cachercize_finalize_client(
        bedrock_module_client_t client)
{
    return cachercize_client_finalize((cachercize_client_t)client);
}

static char* cachercize_get_client_config(
        bedrock_module_client_t client) {
    (void)client;
    // TODO
    return strdup("{}");
}

static int cachercize_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    cachercize_client_t c = (cachercize_client_t)client;
    cachercize_provider_handle_t tmp = calloc(1, sizeof(*tmp));
    margo_addr_dup(c->mid, address, &(tmp->addr));
    tmp->provider_id = provider_id;
    *ph = (bedrock_module_provider_handle_t)tmp;
    return BEDROCK_SUCCESS;
}

static int cachercize_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    cachercize_provider_handle_t tmp = (cachercize_provider_handle_t)ph;
    margo_addr_free(tmp->mid, tmp->addr);
    free(tmp);
    return BEDROCK_SUCCESS;
}

static struct bedrock_module cachercize = {
    .register_provider       = cachercize_register_provider,
    .deregister_provider     = cachercize_deregister_provider,
    .get_provider_config     = cachercize_get_provider_config,
    .init_client             = cachercize_init_client,
    .finalize_client         = cachercize_finalize_client,
    .get_client_config       = cachercize_get_client_config,
    .create_provider_handle  = cachercize_create_provider_handle,
    .destroy_provider_handle = cachercize_destroy_provider_handle,
    .provider_dependencies   = NULL,
    .client_dependencies     = NULL
};

BEDROCK_REGISTER_MODULE(cachercize, cachercize)
