/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCIZE_PROVIDER_HANDLE_H
#define __CACHERCIZE_PROVIDER_HANDLE_H

#include <margo.h>
#include <cachercize/cachercize-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cachercize_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct cachercize_provider_handle* cachercize_provider_handle_t;
#define CACHERCIZE_PROVIDER_HANDLE_NULL ((cachercize_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
