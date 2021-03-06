/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "cachercise/cachercise-server.h"
#include "provider.h"
#include "types.h"

// backends that we want to add at compile time
#include "dummy/dummy-backend.h"

static void cachercise_finalize_provider(void* p);

/* Functions to manipulate the hash of caches */
static inline cachercise_cache* find_cache(
        cachercise_provider_t provider,
        const cachercise_cache_id_t* id);

static inline cachercise_return_t add_cache(
        cachercise_provider_t provider,
        cachercise_cache* cache);

static inline cachercise_return_t remove_cache(
        cachercise_provider_t provider,
        const cachercise_cache_id_t* id,
        int close_cache);

static inline void remove_all_caches(
        cachercise_provider_t provider);

/* Functions to manipulate the list of backend types */
static inline cachercise_backend_impl* find_backend_impl(
        cachercise_provider_t provider,
        const char* name);

static inline cachercise_return_t add_backend_impl(
        cachercise_provider_t provider,
        cachercise_backend_impl* backend);

/* Function to check the validity of the token sent by an admin
 * (returns 0 is the token is incorrect) */
static inline int check_token(
        cachercise_provider_t provider,
        const char* token);

/* Admin RPCs */
static DECLARE_MARGO_RPC_HANDLER(cachercise_create_cache_ult)
static void cachercise_create_cache_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(cachercise_open_cache_ult)
static void cachercise_open_cache_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(cachercise_close_cache_ult)
static void cachercise_close_cache_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(cachercise_destroy_cache_ult)
static void cachercise_destroy_cache_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(cachercise_list_caches_ult)
static void cachercise_list_caches_ult(hg_handle_t h);

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(cachercise_hello_ult)
static void cachercise_hello_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(cachercise_sum_ult)
static void cachercise_sum_ult(hg_handle_t h);

/* add other RPC declarations here */

static DECLARE_MARGO_RPC_HANDLER(cachercise_io_ult)
static void cachercise_io_ult(hg_handle_t h);

int cachercise_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct cachercise_provider_args* args,
        cachercise_provider_t* provider)
{
    struct cachercise_provider_args a = CACHERCISE_PROVIDER_ARGS_INIT;
    if(args) a = *args;
    cachercise_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    margo_info(mid, "Registering CACHERCISE provider with provider id %u", provider_id);

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        margo_error(mid, "Margo instance is not a server");
        return CACHERCISE_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "cachercise_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        margo_error(mid, "Provider with the same provider id (%u) already register", provider_id);
        return CACHERCISE_ERR_INVALID_PROVIDER;
    }

    p = (cachercise_provider_t)calloc(1, sizeof(*p));
    if(p == NULL) {
        margo_error(mid, "Could not allocate memory for provider");
        return CACHERCISE_ERR_ALLOCATION;
    }

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = a.pool;
    p->abtio = a.abtio;
    p->token = (a.token && strlen(a.token)) ? strdup(a.token) : NULL;

    /* Admin RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_create_cache",
            create_cache_in_t, create_cache_out_t,
            cachercise_create_cache_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->create_cache_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_open_cache",
            open_cache_in_t, open_cache_out_t,
            cachercise_open_cache_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->open_cache_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_close_cache",
            close_cache_in_t, close_cache_out_t,
            cachercise_close_cache_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->close_cache_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_destroy_cache",
            destroy_cache_in_t, destroy_cache_out_t,
            cachercise_destroy_cache_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->destroy_cache_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_list_caches",
            list_caches_in_t, list_caches_out_t,
            cachercise_list_caches_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->list_caches_id = id;

    /* Client RPCs */

    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_hello",
            hello_in_t, void,
            cachercise_hello_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->hello_id = id;
    margo_registered_disable_response(mid, id, HG_TRUE);

    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_sum",
            sum_in_t, sum_out_t,
            cachercise_sum_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;

    /* add other RPC registration here */
    id = MARGO_REGISTER_PROVIDER(mid, "cachercise_io",
            io_in_t, io_out_t,
            cachercise_io_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void *)p, NULL);
    p->io_id = id;

    /* add backends available at compiler time (e.g. default/dummy backends) */
    cachercise_provider_register_dummy_backend(p); // function from "dummy/dummy-backend.h"

    margo_provider_push_finalize_callback(mid, p, &cachercise_finalize_provider, p);

    if(provider)
        *provider = p;
    margo_info(mid, "CACHERCISE provider registration done");
    return CACHERCISE_SUCCESS;
}

static void cachercise_finalize_provider(void* p)
{
    cachercise_provider_t provider = (cachercise_provider_t)p;
    margo_info(provider->mid, "Finalizing CACHERCISE provider");
    margo_deregister(provider->mid, provider->create_cache_id);
    margo_deregister(provider->mid, provider->open_cache_id);
    margo_deregister(provider->mid, provider->close_cache_id);
    margo_deregister(provider->mid, provider->destroy_cache_id);
    margo_deregister(provider->mid, provider->list_caches_id);
    margo_deregister(provider->mid, provider->hello_id);
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    margo_deregister(provider->mid, provider->io_id);
    remove_all_caches(provider);
    free(provider->backend_types);
    free(provider->token);
    margo_instance_id mid = provider->mid;
    free(provider);
    margo_info(mid, "CACHERCISE provider successfuly finalized");
}

int cachercise_provider_destroy(
        cachercise_provider_t provider)
{
    margo_instance_id mid = provider->mid;
    margo_info(mid, "Destroying CACHERCISE provider");
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    cachercise_finalize_provider(provider);
    margo_info(mid, "CACHERCISE provider successfuly destroyed");
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_provider_register_backend(
        cachercise_provider_t provider,
        cachercise_backend_impl* backend_impl)
{
    margo_info(provider->mid, "Adding backend implementation \"%s\" to CACHERCISE provider",
             backend_impl->name);
    return add_backend_impl(provider, backend_impl);
}

static void cachercise_create_cache_ult(hg_handle_t h)
{
    hg_return_t hret;
    cachercise_return_t ret;
    create_cache_in_t  in;
    create_cache_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(provider->mid, "Invalid token");
        out.ret = CACHERCISE_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    cachercise_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(provider->mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = CACHERCISE_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new cache */
    cachercise_cache_id_t id;
    uuid_generate(id.uuid);

    /* create the new cache's context */
    void* context = NULL;
    ret = backend->create_cache(provider, in.config, &context);
    if(ret != CACHERCISE_SUCCESS) {
        out.ret = ret;
        margo_error(provider->mid, "Could not create cache, backend returned %d", ret);
        goto finish;
    }

    /* allocate a cache, set it up, and add it to the provider */
    cachercise_cache* cache = (cachercise_cache*)calloc(1, sizeof(*cache));
    cache->fn  = backend;
    cache->ctx = context;
    cache->id  = id;
    add_cache(provider, cache);

    /* set the response */
    out.ret = CACHERCISE_SUCCESS;
    out.id = id;

    char id_str[37];
    cachercise_cache_id_to_string(id, id_str);
    margo_debug(provider->mid, "Created cache %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_create_cache_ult)

static void cachercise_open_cache_ult(hg_handle_t h)
{
    hg_return_t hret;
    cachercise_return_t ret;
    open_cache_in_t  in;
    open_cache_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = CACHERCISE_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    cachercise_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = CACHERCISE_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new cache */
    cachercise_cache_id_t id;
    uuid_generate(id.uuid);

    /* create the new cache's context */
    void* context = NULL;
    ret = backend->open_cache(provider, in.config, &context);
    if(ret != CACHERCISE_SUCCESS) {
        margo_error(mid, "Backend failed to open cache");
        out.ret = ret;
        goto finish;
    }

    /* allocate a cache, set it up, and add it to the provider */
    cachercise_cache* cache = (cachercise_cache*)calloc(1, sizeof(*cache));
    cache->fn  = backend;
    cache->ctx = context;
    cache->id  = id;
    add_cache(provider, cache);

    /* set the response */
    out.ret = CACHERCISE_SUCCESS;
    out.id = id;

    char id_str[37];
    cachercise_cache_id_to_string(id, id_str);
    margo_debug(mid, "Created cache %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_open_cache_ult)

static void cachercise_close_cache_ult(hg_handle_t h)
{
    hg_return_t hret;
    cachercise_return_t ret;
    close_cache_in_t  in;
    close_cache_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = CACHERCISE_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* remove the cache from the provider 
     * (its close function will be called) */
    ret = remove_cache(provider, &in.id, 1);
    out.ret = ret;

    char id_str[37];
    cachercise_cache_id_to_string(in.id, id_str);
    margo_debug(mid, "Removed cache with id %s", id_str);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_close_cache_ult)

static void cachercise_destroy_cache_ult(hg_handle_t h)
{
    hg_return_t hret;
    destroy_cache_in_t  in;
    destroy_cache_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = CACHERCISE_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the cache */
    cachercise_cache* cache = find_cache(provider, &in.id);
    if(!cache) {
        margo_error(mid, "Could not find cache");
        out.ret = CACHERCISE_ERR_INVALID_CACHE;
        goto finish;
    }

    /* destroy the cache's context */
    cache->fn->destroy_cache(cache->ctx);

    /* remove the cache from the provider 
     * (its close function will NOT be called) */
    out.ret = remove_cache(provider, &in.id, 0);

    if(out.ret == CACHERCISE_SUCCESS) {
        char id_str[37];
        cachercise_cache_id_to_string(in.id, id_str);
        margo_debug(mid, "Destroyed cache with id %s", id_str);
    } else {
        margo_error(mid, "Could not destroy cache, cache may be left in an invalid state");
    }


finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_destroy_cache_ult)

static void cachercise_list_caches_ult(hg_handle_t h)
{
    hg_return_t hret;
    list_caches_in_t  in;
    list_caches_out_t out;
    out.ids = NULL;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = CACHERCISE_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* allocate array of cache ids */
    out.ret   = CACHERCISE_SUCCESS;
    out.count = provider->num_caches < in.max_ids ? provider->num_caches : in.max_ids;
    out.ids   = (cachercise_cache_id_t*)calloc(provider->num_caches, sizeof(*out.ids));

    /* iterate over the hash of caches to fill the array of cache ids */
    unsigned i = 0;
    cachercise_cache *r, *tmp;
    HASH_ITER(hh, provider->caches, r, tmp) {
        out.ids[i++] = r->id;
    }

    margo_debug(mid, "Listed caches");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    free(out.ids);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_list_caches_ult)

static void cachercise_hello_ult(hg_handle_t h)
{
    hg_return_t hret;
    hello_in_t in;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        goto finish;
    }

    /* find the cache */
    cachercise_cache* cache = find_cache(provider, &in.cache_id);
    if(!cache) {
        margo_error(mid, "Could not find requested cache");
        goto finish;
    }

    /* call hello on the cache's context */
    cache->fn->hello(cache->ctx);

    margo_debug(mid, "Called hello RPC");

finish:
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_hello_ult)

static void cachercise_sum_ult(hg_handle_t h)
{
    hg_return_t hret;
    sum_in_t     in;
    sum_out_t   out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* find the cache */
    cachercise_cache* cache = find_cache(provider, &in.cache_id);
    if(!cache) {
        margo_error(mid, "Could not find requested cache");
        out.ret = CACHERCISE_ERR_INVALID_CACHE;
        goto finish;
    }

    /* call hello on the cache's context */
    out.result = cache->fn->sum(cache->ctx, in.x, in.y);
    out.ret = CACHERCISE_SUCCESS;

    margo_debug(mid, "Called sum RPC");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(cachercise_sum_ult)


static void cachercise_io_ult(hg_handle_t h)
{
    hg_return_t hret;
    io_in_t in;
    io_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    cachercise_provider_t provider = (cachercise_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    /* find the cache */
    cachercise_cache* cache = find_cache(provider, &in.cache_id);
    if(!cache) {
        margo_error(mid, "Could not find requested cache");
        out.ret = CACHERCISE_ERR_INVALID_CACHE;
        goto finish;
    }

    /* call io on the cache's context */
    out.result = cache->fn->io(cache->ctx, in.count, in.offset, &(in.scratch), in.kind);
    out.ret = CACHERCISE_SUCCESS;
    out.bytes = out.result * sizeof(int64_t);
    out.scratch = in.scratch;

    margo_debug(mid, "Called I/O RPC");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);

}
static DEFINE_MARGO_RPC_HANDLER(cachercise_io_ult)

static inline cachercise_cache* find_cache(
        cachercise_provider_t provider,
        const cachercise_cache_id_t* id)
{
    cachercise_cache* cache = NULL;
    HASH_FIND(hh, provider->caches, id, sizeof(cachercise_cache_id_t), cache);
    return cache;
}

static inline cachercise_return_t add_cache(
        cachercise_provider_t provider,
        cachercise_cache* cache)
{
    cachercise_cache* existing = find_cache(provider, &(cache->id));
    if(existing) {
        return CACHERCISE_ERR_INVALID_CACHE;
    }
    HASH_ADD(hh, provider->caches, id, sizeof(cachercise_cache_id_t), cache);
    provider->num_caches += 1;
    return CACHERCISE_SUCCESS;
}

static inline cachercise_return_t remove_cache(
        cachercise_provider_t provider,
        const cachercise_cache_id_t* id,
        int close_cache)
{
    cachercise_cache* cache = find_cache(provider, id);
    if(!cache) {
        return CACHERCISE_ERR_INVALID_CACHE;
    }
    cachercise_return_t ret = CACHERCISE_SUCCESS;
    if(close_cache) {
        ret = cache->fn->close_cache(cache->ctx);
    }
    HASH_DEL(provider->caches, cache);
    free(cache);
    provider->num_caches -= 1;
    return ret;
}

static inline void remove_all_caches(
        cachercise_provider_t provider)
{
    cachercise_cache *r, *tmp;
    HASH_ITER(hh, provider->caches, r, tmp) {
        HASH_DEL(provider->caches, r);
        r->fn->close_cache(r->ctx);
        free(r);
    }
    provider->num_caches = 0;
}

static inline cachercise_backend_impl* find_backend_impl(
        cachercise_provider_t provider,
        const char* name)
{
    size_t i;
    for(i = 0; i < provider->num_backend_types; i++) {
        cachercise_backend_impl* impl = provider->backend_types[i];
        if(strcmp(name, impl->name) == 0)
            return impl;
    }
    return NULL;
}

static inline cachercise_return_t add_backend_impl(
        cachercise_provider_t provider,
        cachercise_backend_impl* backend)
{
    provider->num_backend_types += 1;
    provider->backend_types = realloc(provider->backend_types,
                                      provider->num_backend_types);
    provider->backend_types[provider->num_backend_types-1] = backend;
    return CACHERCISE_SUCCESS;
}

static inline int check_token(
        cachercise_provider_t provider,
        const char* token)
{
    if(!provider->token) return 1;
    return !strcmp(provider->token, token);
}
