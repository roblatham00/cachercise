/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include <json-c/json.h>
#include "cachercise/cachercise-backend.h"
#include "../provider.h"
#include "dummy-backend.h"
#include "../hoard-c.h"

typedef struct dummy_context {
    struct json_object* config;
    hoard_t h;
    ABT_mutex hoard_mutex;
    /* ... */
} dummy_context;


static cachercise_return_t dummy_create_cache(
        cachercise_provider_t provider,
        const char* config_str,
        void** context)
{
    (void)provider;
    struct json_object* config = NULL;

    // read JSON config from provided string argument
    if (config_str) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, config_str,
                strlen(config_str));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(provider->mid, "JSON parse error: %s",
                      json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return CACHERCISE_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = config;
    ctx->h      = hoard_init();
    ABT_mutex_create(&ctx->hoard_mutex);
    *context = (void*)ctx;
    return CACHERCISE_SUCCESS;
}

static cachercise_return_t dummy_open_cache(
        cachercise_provider_t provider,
        const char* config_str,
        void** context)
{
    (void)provider;

    struct json_object* config = NULL;

    // read JSON config from provided string argument
    if (config_str) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, config_str,
                strlen(config_str));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(provider->mid, "JSON parse error: %s",
                      json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return CACHERCISE_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = config;
    *context = (void*)ctx;
    return CACHERCISE_SUCCESS;
}

static cachercise_return_t dummy_close_cache(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_object_put(context->config);
    hoard_finalize(context->h);
    free(context);
    return CACHERCISE_SUCCESS;
}

static cachercise_return_t dummy_destroy_cache(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_object_put(context->config);
    hoard_finalize(context->h);
    ABT_mutex_free(&(context->hoard_mutex));
    free(context);
    return CACHERCISE_SUCCESS;
}

static void dummy_say_hello(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    (void)context;
    printf("Hello World from Dummy cache\n");
}

static int32_t dummy_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static int64_t dummy_io(void *ctx, uint64_t count, int64_t offset, int64_t *scratch, int kind)
{
    dummy_context* context = (dummy_context*)ctx;
    int64_t ret;
    if (kind == CACHERCISE_WRITE) {
        ABT_mutex_lock(context->hoard_mutex);
        ret = hoard_put(context->h, scratch, count/sizeof(int64_t), offset);
        ABT_mutex_unlock(context->hoard_mutex);
        return ret;
    } else {
        ret = hoard_get(context->h, scratch, count/sizeof(int64_t), offset);
        return ret;
    }
}

static cachercise_backend_impl dummy_backend = {
    .name             = "dummy",

    .create_cache  = dummy_create_cache,
    .open_cache    = dummy_open_cache,
    .close_cache   = dummy_close_cache,
    .destroy_cache = dummy_destroy_cache,

    .hello            = dummy_say_hello,
    .sum              = dummy_compute_sum,
    .io               = dummy_io
};

cachercise_return_t cachercise_provider_register_dummy_backend(cachercise_provider_t provider)
{
    return cachercise_provider_register_backend(provider, &dummy_backend);
}
