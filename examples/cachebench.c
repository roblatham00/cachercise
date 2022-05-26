/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <assert.h>
#include <cachercise/cachercise-client.h>
#include <cachercise/cachercise-cache.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr,"Usage: %s <server address> <provider id> <cache id>\n", argv[0]);
        exit(-1);
    }

    cachercise_return_t ret;
    hg_return_t hret;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    const char* id_str       = argv[3];
    if(strlen(id_str) != 36) {
        FATAL(MARGO_INSTANCE_NULL,"id should be 36 character long");
    }

    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hg_addr_t svr_addr;
    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed for address %s", svr_addr_str);
    }

    cachercise_client_t cachercise_clt;
    cachercise_cache_handle_t cachercise_rh;

    margo_info(mid, "Creating CACHERCISE client");
    ret = cachercise_client_init(mid, &cachercise_clt);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_client_init failed (ret = %d)", ret);
    }

    cachercise_cache_id_t cache_id;
    cachercise_cache_id_from_string(id_str, &cache_id);

    margo_info(mid, "Creating cache handle for cache %s", id_str);
    ret = cachercise_cache_handle_create(
            cachercise_clt, svr_addr, provider_id,
            cache_id, &cachercise_rh);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_cache_handle_create failed (ret = %d)", ret);
    }

    int32_t i, result;
    for (i=0; i< 1000; i++) {
        ret = cachercise_compute_sum(cachercise_rh, 45, 32, &result);
        if (ret != CACHERCISE_SUCCESS)
        FATAL(mid,"cachercise_compute_sum failed (ret = %d)", ret);
    }
    printf("issued %d sum rpcs\n", i);

    for (i=0; i< 1000; i++ ) {
        int value=i;
        ret = cachercise_write(cachercise_rh, &value, sizeof(value), i);
    }
    printf("issued %d io RPCS\n", i);


    margo_info(mid, "Releasing cache handle");
    ret = cachercise_cache_handle_release(cachercise_rh);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_cache_handle_release failed (ret = %d)", ret);
    }

    margo_info(mid, "Finalizing client");
    ret = cachercise_client_finalize(cachercise_clt);
    if(ret != CACHERCISE_SUCCESS) {
        FATAL(mid,"cachercise_client_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"Could not free address (margo_addr_free returned %d)", hret);
    }

    margo_finalize(mid);

    return 0;
}
