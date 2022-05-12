/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <assert.h>
#include <cachercize/cachercize-client.h>
#include <cachercize/cachercize-cache.h>

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

    cachercize_return_t ret;
    hg_return_t hret;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    const char* id_str       = argv[3];
    if(strlen(id_str) != 36) {
        FATAL(MARGO_INSTANCE_NULL,"id should be 36 character long");
    }

    margo_instance_id mid = margo_init("tcp", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hg_addr_t svr_addr;
    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed for address %s", svr_addr_str);
    }

    cachercize_client_t cachercize_clt;
    cachercize_cache_handle_t cachercize_rh;

    margo_info(mid, "Creating CACHERCIZE client");
    ret = cachercize_client_init(mid, &cachercize_clt);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_client_init failed (ret = %d)", ret);
    }

    cachercize_cache_id_t cache_id;
    cachercize_cache_id_from_string(id_str, &cache_id);

    margo_info(mid, "Creating cache handle for cache %s", id_str);
    ret = cachercize_cache_handle_create(
            cachercize_clt, svr_addr, provider_id,
            cache_id, &cachercize_rh);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_cache_handle_create failed (ret = %d)", ret);
    }

    margo_info(mid, "Saying Hello to server");
    ret = cachercize_say_hello(cachercize_rh);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_say_hello failed (ret = %d)", ret);
    }

    margo_info(mid, "Computing sum");
    int32_t result;
    ret = cachercize_compute_sum(cachercize_rh, 45, 23, &result);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_compute_sum failed (ret = %d)", ret);
    }
    margo_info(mid, "45 + 23 = %d", result);

    margo_info(mid, "Releasing cache handle");
    ret = cachercize_cache_handle_release(cachercize_rh);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_cache_handle_release failed (ret = %d)", ret);
    }

    margo_info(mid, "Finalizing client");
    ret = cachercize_client_finalize(cachercize_clt);
    if(ret != CACHERCIZE_SUCCESS) {
        FATAL(mid,"cachercize_client_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"Could not free address (margo_addr_free returned %d)", hret);
    }

    margo_finalize(mid);

    return 0;
}
