/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "client.h"
#include "cachercize/cachercize-client.h"

cachercize_return_t cachercize_client_init(margo_instance_id mid, cachercize_client_t* client)
{
    cachercize_client_t c = (cachercize_client_t)calloc(1, sizeof(*c));
    if(!c) return CACHERCIZE_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "cachercize_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "cachercize_sum", &c->sum_id, &flag);
        margo_registered_name(mid, "cachercize_hello", &c->hello_id, &flag);
        margo_registered_name(mid, "cachercize_io", &c->io_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "cachercize_sum", sum_in_t, sum_out_t, NULL);
        c->hello_id = MARGO_REGISTER(mid, "cachercize_hello", hello_in_t, void, NULL);
        c->hello_id = MARGO_REGISTER(mid, "cachercize_io", io_in_t, io_out_t, NULL);
        margo_registered_disable_response(mid, c->hello_id, HG_TRUE);
    }

    *client = c;
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_client_finalize(cachercize_client_t client)
{
    if(client->num_cache_handles != 0) {
        fprintf(stderr,  
                "Warning: %ld cache handles not released when cachercize_client_finalize was called\n",
                client->num_cache_handles);
    }
    free(client);
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_cache_handle_create(
        cachercize_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        cachercize_cache_id_t cache_id,
        cachercize_cache_handle_t* handle)
{
    if(client == CACHERCIZE_CLIENT_NULL)
        return CACHERCIZE_ERR_INVALID_ARGS;

    cachercize_cache_handle_t rh =
        (cachercize_cache_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return CACHERCIZE_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->cache_id = cache_id;
    rh->refcount    = 1;

    client->num_cache_handles += 1;

    *handle = rh;
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_cache_handle_ref_incr(
        cachercize_cache_handle_t handle)
{
    if(handle == CACHERCIZE_CACHE_HANDLE_NULL)
        return CACHERCIZE_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_cache_handle_release(cachercize_cache_handle_t handle)
{
    if(handle == CACHERCIZE_CACHE_HANDLE_NULL)
        return CACHERCIZE_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_cache_handles -= 1;
        free(handle);
    }
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_say_hello(cachercize_cache_handle_t handle)
{
    hg_handle_t   h;
    hello_in_t     in;
    hg_return_t ret;

    memcpy(&in.cache_id, &(handle->cache_id), sizeof(in.cache_id));

    ret = margo_create(handle->client->mid, handle->addr, handle->client->hello_id, &h);
    if(ret != HG_SUCCESS)
        return CACHERCIZE_ERR_FROM_MERCURY;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    margo_destroy(h);
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_compute_sum(
        cachercize_cache_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    cachercize_return_t ret;

    memcpy(&in.cache_id, &(handle->cache_id), sizeof(in.cache_id));
    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = CACHERCIZE_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = CACHERCIZE_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == CACHERCIZE_SUCCESS)
        *result = out.result;

finish:
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

cachercize_return_t cachercize_io(cachercize_cache_handle_t handle,
        void * buf,
        uint64_t count,
        int64_t offset, int kind)
{
    hg_handle_t h;
    io_in_t in;
    io_out_t out;
    hg_return_t hret;
    cachercize_return_t ret;

    memcpy(&in.cache_id, &(handle->cache_id), sizeof(in.cache_id));
    in.count  = count;
    in.offset = offset;
    if (kind == CACHERCIZE_WRITE) {
        /* don't want to deal with bulk registration in this concurrency benchmark */
        if (count > sizeof (int64_t)) count = sizeof(int64_t);
        memcpy(&(in.scratch), buf, count);
    }
    in.kind   = kind;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->io_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = CACHERCIZE_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = CACHERCIZE_ERR_FROM_MERCURY;
        goto finish;
    }

    if (kind == CACHERCIZE_READ) {
        if (out.bytes > sizeof(int64_t)) out.bytes = sizeof(int64_t);
            memcpy(buf, &(out.scratch), out.bytes);
    }
    return out.bytes;

finish:
    return ret;
}
