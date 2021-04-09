/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _CLIENT_H
#define _CLIENT_H

#include "types.h"
#include "reducer/reducer-client.h"
#include "reducer/reducer-metric.h"

typedef struct reducer_client {
   margo_instance_id mid;
   hg_id_t           metric_reduce_id;
   uint64_t          num_metric_handles;
} reducer_client;

typedef struct reducer_metric_handle {
    reducer_client_t    client;
    hg_addr_t           addr;
    uint16_t            provider_id;
    uint64_t            refcount;
} reducer_metric_handle;

#endif
