/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "client.h"
#include "cachercise/cachercise-client.h"

cachercise_return_t cachercise_client_init(margo_instance_id mid, cachercise_client_t* client)
{
    cachercise_client_t c = (cachercise_client_t)calloc(1, sizeof(*c));
    if(!c) return CACHERCISE_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "cachercise_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "cachercise_sum", &c->sum_id, &flag);
        margo_registered_name(mid, "cachercise_hello", &c->hello_id, &flag);
        margo_registered_name(mid, "cachercise_io", &c->io_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "cachercise_sum", sum_in_t, sum_out_t, NULL);
        c->hello_id = MARGO_REGISTER(mid, "cachercise_hello", hello_in_t, void, NULL);
        c->io_id = MARGO_REGISTER(mid, "cachercise_io", io_in_t, io_out_t, NULL);
        margo_registered_disable_response(mid, c->hello_id, HG_TRUE);
    }

    *client = c;
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_client_finalize(cachercise_client_t client)
{
    if(client->num_cache_handles != 0) {
        fprintf(stderr,  
                "Warning: %ld cache handles not released when cachercise_client_finalize was called\n",
                client->num_cache_handles);
    }
    free(client);
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_cache_handle_create(
        cachercise_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        cachercise_cache_id_t cache_id,
        cachercise_cache_handle_t* handle)
{
    if(client == CACHERCISE_CLIENT_NULL)
        return CACHERCISE_ERR_INVALID_ARGS;

    cachercise_cache_handle_t rh =
        (cachercise_cache_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return CACHERCISE_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->cache_id = cache_id;
    rh->refcount    = 1;

    client->num_cache_handles += 1;

    *handle = rh;
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_cache_handle_ref_incr(
        cachercise_cache_handle_t handle)
{
    if(handle == CACHERCISE_CACHE_HANDLE_NULL)
        return CACHERCISE_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_cache_handle_release(cachercise_cache_handle_t handle)
{
    if(handle == CACHERCISE_CACHE_HANDLE_NULL)
        return CACHERCISE_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_cache_handles -= 1;
        free(handle);
    }
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_say_hello(cachercise_cache_handle_t handle)
{
    hg_handle_t   h;
    hello_in_t     in;
    hg_return_t ret;

    memcpy(&in.cache_id, &(handle->cache_id), sizeof(in.cache_id));

    ret = margo_create(handle->client->mid, handle->addr, handle->client->hello_id, &h);
    if(ret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    margo_destroy(h);
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_compute_sum(
        cachercise_cache_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    cachercise_return_t ret;

    memcpy(&in.cache_id, &(handle->cache_id), sizeof(in.cache_id));
    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == CACHERCISE_SUCCESS)
        *result = out.result;

finish:
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

cachercise_return_t cachercise_io(
        cachercise_cache_handle_t handle,
        void * buf,
        uint64_t count,
        int64_t offset,
        int kind)
{
    hg_handle_t h;
    io_in_t in;
    io_out_t out;
    hg_return_t hret;
    cachercise_return_t ret;

    memcpy(&in.cache_id, &(handle->cache_id), sizeof(in.cache_id));
    in.count  = count;
    in.offset = offset;
    if (kind == CACHERCISE_WRITE) {
        /* don't want to deal with bulk registration in this concurrency benchmark */
        if (count > sizeof (int64_t)) count = sizeof(int64_t);
        memcpy(&(in.scratch), buf, count);
    }
    in.kind   = kind;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->io_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = CACHERCISE_ERR_FROM_MERCURY;
        goto finish;
    }

    if (kind == CACHERCISE_READ) {
        if (out.bytes > sizeof(int64_t)) out.bytes = sizeof(int64_t);
	memcpy(buf, &(out.scratch), out.bytes);
    }
    return out.bytes;

finish:
    return ret;
}
