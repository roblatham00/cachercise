/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "cachercize/cachercize-admin.h"

cachercize_return_t cachercize_admin_init(margo_instance_id mid, cachercize_admin_t* admin)
{
    cachercize_admin_t a = (cachercize_admin_t)calloc(1, sizeof(*a));
    if(!a) return CACHERCIZE_ERR_ALLOCATION;

    a->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "cachercize_create_cache", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "cachercize_create_cache", &a->create_cache_id, &flag);
        margo_registered_name(mid, "cachercize_open_cache", &a->open_cache_id, &flag);
        margo_registered_name(mid, "cachercize_close_cache", &a->close_cache_id, &flag);
        margo_registered_name(mid, "cachercize_destroy_cache", &a->destroy_cache_id, &flag);
        margo_registered_name(mid, "cachercize_list_caches", &a->list_caches_id, &flag);
        /* Get more existing RPCs... */
    } else {
        a->create_cache_id =
            MARGO_REGISTER(mid, "cachercize_create_cache",
            create_cache_in_t, create_cache_out_t, NULL);
        a->open_cache_id =
            MARGO_REGISTER(mid, "cachercize_open_cache",
            open_cache_in_t, open_cache_out_t, NULL);
        a->close_cache_id =
            MARGO_REGISTER(mid, "cachercize_close_cache",
            close_cache_in_t, close_cache_out_t, NULL);
        a->destroy_cache_id =
            MARGO_REGISTER(mid, "cachercize_destroy_cache",
            destroy_cache_in_t, destroy_cache_out_t, NULL);
        a->list_caches_id =
            MARGO_REGISTER(mid, "cachercize_list_caches",
            list_caches_in_t, list_caches_out_t, NULL);
        /* Register more RPCs ... */
    }

    *admin = a;
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_admin_finalize(cachercize_admin_t admin)
{
    free(admin);
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_create_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercize_cache_id_t* id)
{
    hg_handle_t h;
    create_cache_in_t  in;
    create_cache_out_t out;
    hg_return_t hret;
    cachercize_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->create_cache_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != CACHERCIZE_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_open_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        cachercize_cache_id_t* id)
{
    hg_handle_t h;
    open_cache_in_t  in;
    open_cache_out_t out;
    hg_return_t hret;
    cachercize_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->open_cache_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != CACHERCIZE_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return CACHERCIZE_SUCCESS;
}

cachercize_return_t cachercize_close_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercize_cache_id_t id)
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
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

cachercize_return_t cachercize_destroy_cache(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercize_cache_id_t id)
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
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

cachercize_return_t cachercize_list_caches(
        cachercize_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        cachercize_cache_id_t* ids,
        size_t* count)
{
    hg_handle_t h;
    list_caches_in_t  in;
    list_caches_out_t out;
    cachercize_return_t ret;
    hg_return_t hret;

    in.token  = (char*)token;
    in.max_ids = *count;

    hret = margo_create(admin->mid, address, admin->list_caches_id, &h);
    if(hret != HG_SUCCESS)
        return CACHERCIZE_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return CACHERCIZE_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    if(ret == CACHERCIZE_SUCCESS) {
        *count = out.count;
        memcpy(ids, out.ids, out.count*sizeof(*ids));
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
