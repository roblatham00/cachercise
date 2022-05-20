/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __CACHERCISE_PROVIDER_HANDLE_H
#define __CACHERCISE_PROVIDER_HANDLE_H

#include <margo.h>
#include <cachercise/cachercise-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cachercise_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct cachercise_provider_handle* cachercise_provider_handle_t;
#define CACHERCISE_PROVIDER_HANDLE_NULL ((cachercise_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
