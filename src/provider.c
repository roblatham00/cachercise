/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "alpha/alpha-server.h"
#include "provider.h"
#include "types.h"

// backends that we want to add at compile time
#include "dummy/dummy-backend.h"

static void alpha_finalize_provider(void* p);

/* Functions to manipulate the hash of resources */
static inline alpha_resource* find_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id);

static inline alpha_return_t add_resource(
        alpha_provider_t provider,
        alpha_resource* resource);

static inline alpha_return_t remove_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id,
        int close_resource);

static inline void remove_all_resources(
        alpha_provider_t provider);

/* Functions to manipulate the list of backend types */
static inline alpha_backend_impl* find_backend_impl(
        alpha_provider_t provider,
        const char* name);

static inline alpha_return_t add_backend_impl(
        alpha_provider_t provider,
        alpha_backend_impl* backend);

/* Function to check the validity of the token sent by an admin
 * (returns 0 is the token is incorrect) */
static inline int check_token(
        alpha_provider_t provider,
        const char* token);

/* Admin RPCs */
static DECLARE_MARGO_RPC_HANDLER(alpha_create_resource_ult)
static void alpha_create_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_open_resource_ult)
static void alpha_open_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_close_resource_ult)
static void alpha_close_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_destroy_resource_ult)
static void alpha_destroy_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_list_resources_ult)
static void alpha_list_resources_ult(hg_handle_t h);

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(alpha_hello_ult)
static void alpha_hello_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_sum_ult)
static void alpha_sum_ult(hg_handle_t h);

/* add other RPC declarations here */

int alpha_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct alpha_provider_args* args,
        alpha_provider_t* provider)
{
    struct alpha_provider_args a = ALPHA_PROVIDER_ARGS_INIT;
    if(args) a = *args;
    alpha_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    margo_info(mid, "Registering ALPHA provider with provider id %u", provider_id);

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        margo_error(mid, "Margo instance is not a server");
        return ALPHA_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "alpha_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        margo_error(mid, "Provider with the same provider id (%u) already register", provider_id);
        return ALPHA_ERR_INVALID_PROVIDER;
    }

    p = (alpha_provider_t)calloc(1, sizeof(*p));
    if(p == NULL) {
        margo_error(mid, "Could not allocate memory for provider");
        return ALPHA_ERR_ALLOCATION;
    }

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = a.pool;
    p->abtio = a.abtio;
    p->token = (a.token && strlen(a.token)) ? strdup(a.token) : NULL;

    /* Admin RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "alpha_create_resource",
            create_resource_in_t, create_resource_out_t,
            alpha_create_resource_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->create_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_open_resource",
            open_resource_in_t, open_resource_out_t,
            alpha_open_resource_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->open_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_close_resource",
            close_resource_in_t, close_resource_out_t,
            alpha_close_resource_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->close_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_destroy_resource",
            destroy_resource_in_t, destroy_resource_out_t,
            alpha_destroy_resource_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->destroy_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_list_resources",
            list_resources_in_t, list_resources_out_t,
            alpha_list_resources_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->list_resources_id = id;

    /* Client RPCs */

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_hello",
            hello_in_t, void,
            alpha_hello_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->hello_id = id;
    margo_registered_disable_response(mid, id, HG_TRUE);

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_sum",
            sum_in_t, sum_out_t,
            alpha_sum_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;

    /* add other RPC registration here */
    /* ... */

    /* add backends available at compiler time (e.g. default/dummy backends) */
    alpha_provider_register_dummy_backend(p); // function from "dummy/dummy-backend.h"

    margo_provider_push_finalize_callback(mid, p, &alpha_finalize_provider, p);

    if(provider)
        *provider = p;
    margo_info(mid, "ALPHA provider registration done");
    return ALPHA_SUCCESS;
}

static void alpha_finalize_provider(void* p)
{
    alpha_provider_t provider = (alpha_provider_t)p;
    margo_info(provider->mid, "Finalizing ALPHA provider");
    margo_deregister(provider->mid, provider->create_resource_id);
    margo_deregister(provider->mid, provider->open_resource_id);
    margo_deregister(provider->mid, provider->close_resource_id);
    margo_deregister(provider->mid, provider->destroy_resource_id);
    margo_deregister(provider->mid, provider->list_resources_id);
    margo_deregister(provider->mid, provider->hello_id);
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    remove_all_resources(provider);
    free(provider->backend_types);
    free(provider->token);
    margo_instance_id mid = provider->mid;
    free(provider);
    margo_info(mid, "ALPHA provider successfuly finalized");
}

int alpha_provider_destroy(
        alpha_provider_t provider)
{
    margo_instance_id mid = provider->mid;
    margo_info(mid, "Destroying ALPHA provider");
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    alpha_finalize_provider(provider);
    margo_info(mid, "ALPHA provider successfuly destroyed");
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_provider_register_backend(
        alpha_provider_t provider,
        alpha_backend_impl* backend_impl)
{
    margo_info(provider->mid, "Adding backend implementation \"%s\" to ALPHA provider",
             backend_impl->name);
    return add_backend_impl(provider, backend_impl);
}

static void alpha_create_resource_ult(hg_handle_t h)
{
    hg_return_t hret;
    alpha_return_t ret;
    create_resource_in_t  in;
    create_resource_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(provider->mid, "Invalid token");
        out.ret = ALPHA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    alpha_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(provider->mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = ALPHA_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new resource */
    alpha_resource_id_t id;
    uuid_generate(id.uuid);

    /* create the new resource's context */
    void* context = NULL;
    ret = backend->create_resource(provider, in.config, &context);
    if(ret != ALPHA_SUCCESS) {
        out.ret = ret;
        margo_error(provider->mid, "Could not create resource, backend returned %d", ret);
        goto finish;
    }

    /* allocate a resource, set it up, and add it to the provider */
    alpha_resource* resource = (alpha_resource*)calloc(1, sizeof(*resource));
    resource->fn  = backend;
    resource->ctx = context;
    resource->id  = id;
    add_resource(provider, resource);

    /* set the response */
    out.ret = ALPHA_SUCCESS;
    out.id = id;

    char id_str[37];
    alpha_resource_id_to_string(id, id_str);
    margo_debug(provider->mid, "Created resource %s of type \"%s\"", id_str, in.type);

finish:
    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_create_resource_ult)

static void alpha_open_resource_ult(hg_handle_t h)
{
    hg_return_t hret;
    alpha_return_t ret;
    open_resource_in_t  in;
    open_resource_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = ALPHA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    alpha_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = ALPHA_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new resource */
    alpha_resource_id_t id;
    uuid_generate(id.uuid);

    /* create the new resource's context */
    void* context = NULL;
    ret = backend->open_resource(provider, in.config, &context);
    if(ret != ALPHA_SUCCESS) {
        margo_error(mid, "Backend failed to open resource");
        out.ret = ret;
        goto finish;
    }

    /* allocate a resource, set it up, and add it to the provider */
    alpha_resource* resource = (alpha_resource*)calloc(1, sizeof(*resource));
    resource->fn  = backend;
    resource->ctx = context;
    resource->id  = id;
    add_resource(provider, resource);

    /* set the response */
    out.ret = ALPHA_SUCCESS;
    out.id = id;

    char id_str[37];
    alpha_resource_id_to_string(id, id_str);
    margo_debug(mid, "Created resource %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_open_resource_ult)

static void alpha_close_resource_ult(hg_handle_t h)
{
    hg_return_t hret;
    alpha_return_t ret;
    close_resource_in_t  in;
    close_resource_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = ALPHA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* remove the resource from the provider 
     * (its close function will be called) */
    ret = remove_resource(provider, &in.id, 1);
    out.ret = ret;

    char id_str[37];
    alpha_resource_id_to_string(in.id, id_str);
    margo_debug(mid, "Removed resource with id %s", id_str);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_close_resource_ult)

static void alpha_destroy_resource_ult(hg_handle_t h)
{
    hg_return_t hret;
    destroy_resource_in_t  in;
    destroy_resource_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = ALPHA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the resource */
    alpha_resource* resource = find_resource(provider, &in.id);
    if(!resource) {
        margo_error(mid, "Could not find resource");
        out.ret = ALPHA_ERR_INVALID_RESOURCE;
        goto finish;
    }

    /* destroy the resource's context */
    resource->fn->destroy_resource(resource->ctx);

    /* remove the resource from the provider 
     * (its close function will NOT be called) */
    out.ret = remove_resource(provider, &in.id, 0);

    if(out.ret == ALPHA_SUCCESS) {
        char id_str[37];
        alpha_resource_id_to_string(in.id, id_str);
        margo_debug(mid, "Destroyed resource with id %s", id_str);
    } else {
        margo_error(mid, "Could not destroy resource, resource may be left in an invalid state");
    }


finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_destroy_resource_ult)

static void alpha_list_resources_ult(hg_handle_t h)
{
    hg_return_t hret;
    list_resources_in_t  in;
    list_resources_out_t out;
    out.ids = NULL;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = ALPHA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* allocate array of resource ids */
    out.ret   = ALPHA_SUCCESS;
    out.count = provider->num_resources < in.max_ids ? provider->num_resources : in.max_ids;
    out.ids   = (alpha_resource_id_t*)calloc(provider->num_resources, sizeof(*out.ids));

    /* iterate over the hash of resources to fill the array of resource ids */
    unsigned i = 0;
    alpha_resource *r, *tmp;
    HASH_ITER(hh, provider->resources, r, tmp) {
        out.ids[i++] = r->id;
    }

    margo_debug(mid, "Listed resources");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    free(out.ids);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_list_resources_ult)

static void alpha_hello_ult(hg_handle_t h)
{
    hg_return_t hret;
    hello_in_t in;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        goto finish;
    }

    /* find the resource */
    alpha_resource* resource = find_resource(provider, &in.resource_id);
    if(!resource) {
        margo_error(mid, "Could not find requested resource");
        goto finish;
    }

    /* call hello on the resource's context */
    resource->fn->hello(resource->ctx);

    margo_debug(mid, "Called hello RPC");

finish:
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_hello_ult)

static void alpha_sum_ult(hg_handle_t h)
{
    hg_return_t hret;
    sum_in_t     in;
    sum_out_t   out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* find the resource */
    alpha_resource* resource = find_resource(provider, &in.resource_id);
    if(!resource) {
        margo_error(mid, "Could not find requested resource");
        out.ret = ALPHA_ERR_INVALID_RESOURCE;
        goto finish;
    }

    /* call hello on the resource's context */
    out.result = resource->fn->sum(resource->ctx, in.x, in.y);
    out.ret = ALPHA_SUCCESS;

    margo_debug(mid, "Called sum RPC");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_sum_ult)

static inline alpha_resource* find_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id)
{
    alpha_resource* resource = NULL;
    HASH_FIND(hh, provider->resources, id, sizeof(alpha_resource_id_t), resource);
    return resource;
}

static inline alpha_return_t add_resource(
        alpha_provider_t provider,
        alpha_resource* resource)
{
    alpha_resource* existing = find_resource(provider, &(resource->id));
    if(existing) {
        return ALPHA_ERR_INVALID_RESOURCE;
    }
    HASH_ADD(hh, provider->resources, id, sizeof(alpha_resource_id_t), resource);
    provider->num_resources += 1;
    return ALPHA_SUCCESS;
}

static inline alpha_return_t remove_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id,
        int close_resource)
{
    alpha_resource* resource = find_resource(provider, id);
    if(!resource) {
        return ALPHA_ERR_INVALID_RESOURCE;
    }
    alpha_return_t ret = ALPHA_SUCCESS;
    if(close_resource) {
        ret = resource->fn->close_resource(resource->ctx);
    }
    HASH_DEL(provider->resources, resource);
    free(resource);
    provider->num_resources -= 1;
    return ret;
}

static inline void remove_all_resources(
        alpha_provider_t provider)
{
    alpha_resource *r, *tmp;
    HASH_ITER(hh, provider->resources, r, tmp) {
        HASH_DEL(provider->resources, r);
        r->fn->close_resource(r->ctx);
        free(r);
    }
    provider->num_resources = 0;
}

static inline alpha_backend_impl* find_backend_impl(
        alpha_provider_t provider,
        const char* name)
{
    size_t i;
    for(i = 0; i < provider->num_backend_types; i++) {
        alpha_backend_impl* impl = provider->backend_types[i];
        if(strcmp(name, impl->name) == 0)
            return impl;
    }
    return NULL;
}

static inline alpha_return_t add_backend_impl(
        alpha_provider_t provider,
        alpha_backend_impl* backend)
{
    provider->num_backend_types += 1;
    provider->backend_types = realloc(provider->backend_types,
                                      provider->num_backend_types);
    provider->backend_types[provider->num_backend_types-1] = backend;
    return ALPHA_SUCCESS;
}

static inline int check_token(
        alpha_provider_t provider,
        const char* token)
{
    if(!provider->token) return 1;
    return !strcmp(provider->token, token);
}
