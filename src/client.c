/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "client.h"
#include "alpha/alpha-client.h"

alpha_return_t alpha_client_init(margo_instance_id mid, alpha_client_t* client)
{
    alpha_client_t c = (alpha_client_t)calloc(1, sizeof(*c));
    if(!c) return ALPHA_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "alpha_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "alpha_sum", &c->sum_id, &flag);
        margo_registered_name(mid, "alpha_hello", &c->hello_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "alpha_sum", sum_in_t, sum_out_t, NULL);
        c->hello_id = MARGO_REGISTER(mid, "alpha_hello", hello_in_t, void, NULL);
        margo_registered_disable_response(mid, c->hello_id, HG_TRUE);
    }

    *client = c;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_client_finalize(alpha_client_t client)
{
    if(client->num_resource_handles != 0) {
        fprintf(stderr,  
                "Warning: %ld resource handles not released when alpha_client_finalize was called\n",
                client->num_resource_handles);
    }
    free(client);
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_resource_handle_create(
        alpha_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        alpha_resource_id_t resource_id,
        alpha_resource_handle_t* handle)
{
    if(client == ALPHA_CLIENT_NULL)
        return ALPHA_ERR_INVALID_ARGS;

    alpha_resource_handle_t rh =
        (alpha_resource_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return ALPHA_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return ALPHA_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->resource_id = resource_id;
    rh->refcount    = 1;

    client->num_resource_handles += 1;

    *handle = rh;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_resource_handle_ref_incr(
        alpha_resource_handle_t handle)
{
    if(handle == ALPHA_RESOURCE_HANDLE_NULL)
        return ALPHA_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_resource_handle_release(alpha_resource_handle_t handle)
{
    if(handle == ALPHA_RESOURCE_HANDLE_NULL)
        return ALPHA_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_resource_handles -= 1;
        free(handle);
    }
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_say_hello(alpha_resource_handle_t handle)
{
    hg_handle_t   h;
    hello_in_t     in;
    hg_return_t ret;

    memcpy(&in.resource_id, &(handle->resource_id), sizeof(in.resource_id));

    ret = margo_create(handle->client->mid, handle->addr, handle->client->hello_id, &h);
    if(ret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    margo_destroy(h);
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_compute_sum(
        alpha_resource_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    alpha_return_t ret;

    memcpy(&in.resource_id, &(handle->resource_id), sizeof(in.resource_id));
    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == ALPHA_SUCCESS)
        *result = out.result;

finish:
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
