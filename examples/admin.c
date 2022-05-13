/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <margo.h>
#include <cachercize/cachercize-admin.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <server address> <provider id>\n", argv[0]);
        exit(0);
    }

    hg_return_t hret;
    cachercize_return_t ret;
    cachercize_admin_t admin;
    hg_addr_t svr_addr;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    cachercize_cache_id_t id;

    margo_instance_id mid = margo_init("tcp", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed (ret = %d)", hret);
    }

    margo_info(mid,"Initializing admin");
    ret = cachercize_admin_init(mid, &admin);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_admin_init failed (ret = %d)", ret);
    }

    margo_info(mid,"Creating cache");
    ret = cachercize_create_cache(admin, svr_addr, provider_id, NULL,
                                "dummy", "This is a config", &id);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_create_cache failed (ret = %d)", ret);
    }

    margo_info(mid,"Listing caches");
    cachercize_cache_id_t ids[4];
    size_t count = 4;
    ret = cachercize_list_caches(admin, svr_addr, provider_id, NULL,
                               ids, &count);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_list_caches failed (ret = %d)", ret);
    }
    margo_info(mid,"Returned %ld cache ids", count);

    unsigned i;
    for(i=0; i < count; i++) {
        char id_str[37];
        cachercize_cache_id_to_string(ids[i], id_str);
        margo_info(mid,"ID %d = %s", i, id_str);
    }

    margo_info(mid,"Finalizing admin");
    ret = cachercize_admin_finalize(admin);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_admin_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_free failed (ret = %d)", ret);
    }

    margo_finalize(mid);

    return 0;
}
