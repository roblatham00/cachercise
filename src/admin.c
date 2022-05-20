/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "cachercise/cachercise-admin.h"

cachercise_return_t cachercise_admin_init(margo_instance_id mid, cachercise_admin_t* admin)
{
    cachercise_admin_t a = (cachercise_admin_t)calloc(1, sizeof(*a));
    if(!a) return CACHERCISE_ERR_ALLOCATION;

    a->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "cachercise_create_cache", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "cachercise_create_cache", &a->create_cache_id, &flag);
        margo_registered_name(mid, "cachercise_open_cache", &a->open_cache_id, &flag);
        margo_registered_name(mid, "cachercise_close_cache", &a->close_cache_id, &flag);
        margo_registered_name(mid, "cachercise_destroy_cache", &a->destroy_cache_id, &flag);
        margo_registered_name(mid, "cachercise_list_caches", &a->list_caches_id, &flag);
        /* Get more existing RPCs... */
    } else {
        a->create_cache_id =
            MARGO_REGISTER(mid, "cachercise_create_cache",
            create_cache_in_t, create_cache_out_t, NULL);
        a->open_cache_id =
            MARGO_REGISTER(mid, "cachercise_open_cache",
            open_cache_in_t, open_cache_out_t, NULL);
        a->close_cache_id =
            MARGO_REGISTER(mid, "cachercise_close_cache",
            close_cache_in_t, close_cache_out_t, NULL);
        a->destroy_cache_id =
            MARGO_REGISTER(mid, "cachercise_destroy_cache",
            destroy_cache_in_t, destroy_cache_out_t, NULL);
        a->list_caches_id =
            MARGO_REGISTER(mid, "cachercise_list_caches",
            list_caches_in_t, list_caches_out_t, NULL);
        /* Register more RPCs ... */
    }

    *admin = a;
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_admin_finalize(cachercise_admin_t admin)
{
    free(admin);
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_create_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercise_cache_id_t* id)
{
    hg_handle_t h;
    create_cache_in_t  in;
    create_cache_out_t out;
    hg_return_t hret;
    cachercise_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->create_cache_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != CACHERCISE_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_open_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercise_cache_id_t* id)
{
    hg_handle_t h;
    open_cache_in_t  in;
    open_cache_out_t out;
    hg_return_t hret;
    cachercise_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->open_cache_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != CACHERCISE_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return CACHERCISE_SUCCESS;
}

cachercise_return_t cachercise_close_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercise_cache_id_t id)
{
    hg_handle_t h;
    close_cache_in_t  in;
    close_cache_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->close_cache_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

cachercise_return_t cachercise_destroy_cache(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercise_cache_id_t id)
{
    hg_handle_t h;
    destroy_cache_in_t  in;
    destroy_cache_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->destroy_cache_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

cachercise_return_t cachercise_list_caches(
        cachercise_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercise_cache_id_t* ids,
        size_t* count)
{
    hg_handle_t h;
    list_caches_in_t  in;
    list_caches_out_t out;
    cachercise_return_t ret;
    hg_return_t hret;

    in.token  = (char*)token;
    in.max_ids = *count;

    hret = margo_create(admin->mid, address, admin->list_caches_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCISE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCISE_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    if(ret == CACHERCISE_SUCCESS) {
        *count = out.count;
        memcpy(ids, out.ids, out.count*sizeof(*ids));
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
