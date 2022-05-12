/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _ADMIN_H
#define _ADMIN_H

#include "types.h"
#include "alpha/alpha-admin.h"

typedef struct alpha_admin {
   margo_instance_id mid;
   hg_id_t           create_resource_id;
   hg_id_t           open_resource_id;
   hg_id_t           close_resource_id;
   hg_id_t           destroy_resource_id;
   hg_id_t           list_resources_id;
} alpha_admin;

#endif
