/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "alpha/alpha-admin.h"

alpha_return_t alpha_admin_init(margo_instance_id mid, alpha_admin_t* admin)
{
    alpha_admin_t a = (alpha_admin_t)calloc(1, sizeof(*a));
    if(!a) return ALPHA_ERR_ALLOCATION;

    a->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "alpha_create_resource", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "alpha_create_resource", &a->create_resource_id, &flag);
        margo_registered_name(mid, "alpha_open_resource", &a->open_resource_id, &flag);
        margo_registered_name(mid, "alpha_close_resource", &a->close_resource_id, &flag);
        margo_registered_name(mid, "alpha_destroy_resource", &a->destroy_resource_id, &flag);
        margo_registered_name(mid, "alpha_list_resources", &a->list_resources_id, &flag);
        /* Get more existing RPCs... */
    } else {
        a->create_resource_id =
            MARGO_REGISTER(mid, "alpha_create_resource",
            create_resource_in_t, create_resource_out_t, NULL);
        a->open_resource_id =
            MARGO_REGISTER(mid, "alpha_open_resource",
            open_resource_in_t, open_resource_out_t, NULL);
        a->close_resource_id =
            MARGO_REGISTER(mid, "alpha_close_resource",
            close_resource_in_t, close_resource_out_t, NULL);
        a->destroy_resource_id =
            MARGO_REGISTER(mid, "alpha_destroy_resource",
            destroy_resource_in_t, destroy_resource_out_t, NULL);
        a->list_resources_id =
            MARGO_REGISTER(mid, "alpha_list_resources",
            list_resources_in_t, list_resources_out_t, NULL);
        /* Register more RPCs ... */
    }

    *admin = a;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_admin_finalize(alpha_admin_t admin)
{
    free(admin);
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_create_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        alpha_resource_id_t* id)
{
    hg_handle_t h;
    create_resource_in_t  in;
    create_resource_out_t out;
    hg_return_t hret;
    alpha_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->create_resource_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != ALPHA_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_open_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        alpha_resource_id_t* id)
{
    hg_handle_t h;
    open_resource_in_t  in;
    open_resource_out_t out;
    hg_return_t hret;
    alpha_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->open_resource_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != ALPHA_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_close_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        alpha_resource_id_t id)
{
    hg_handle_t h;
    close_resource_in_t  in;
    close_resource_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->close_resource_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

alpha_return_t alpha_destroy_resource(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        alpha_resource_id_t id)
{
    hg_handle_t h;
    destroy_resource_in_t  in;
    destroy_resource_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->destroy_resource_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

alpha_return_t alpha_list_resources(
        alpha_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        alpha_resource_id_t* ids,
        size_t* count)
{
    hg_handle_t h;
    list_resources_in_t  in;
    list_resources_out_t out;
    alpha_return_t ret;
    hg_return_t hret;

    in.token  = (char*)token;
    in.max_ids = *count;

    hret = margo_create(admin->mid, address, admin->list_resources_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return ALPHA_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    if(ret == ALPHA_SUCCESS) {
        *count = out.count;
        memcpy(ids, out.ids, out.count*sizeof(*ids));
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
