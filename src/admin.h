/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _ADMIN_H
#define _ADMIN_H

#include "types.h"
#include "cachercise/cachercise-admin.h"

typedef struct cachercise_admin {
   margo_instance_id mid;
   hg_id_t           create_cache_id;
   hg_id_t           open_cache_id;
   hg_id_t           close_cache_id;
   hg_id_t           destroy_cache_id;
   hg_id_t           list_caches_id;
} cachercise_admin;

#endif
