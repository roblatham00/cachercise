/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_PROVIDER_HANDLE_H
#define __ALPHA_PROVIDER_HANDLE_H

#include <margo.h>
#include <alpha/alpha-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct alpha_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct alpha_provider_handle* alpha_provider_handle_t;
#define ALPHA_PROVIDER_HANDLE_NULL ((alpha_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
