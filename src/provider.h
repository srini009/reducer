/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __PROVIDER_H
#define __PROVIDER_H

#include <margo.h>
#include <abt-io.h>
#include "uthash.h"
#include "types.h"
#ifdef USE_AGGREGATOR
#include <sdskv-client.h>
#endif

typedef struct reducer_provider {
    /* Margo/Argobots/Mercury environment */
    margo_instance_id  mid;                 // Margo instance
    uint16_t           provider_id;         // Provider id
    ABT_pool           pool;                // Pool on which to post RPC requests
    abt_io_instance_id abtio;               // ABT-IO instance
    /* Resources and backend types */
    size_t               num_metrics;     // number of metrics
    reducer_metric*      metrics;         // hash of metrics by id
    /* RPC identifiers for clients */
    hg_id_t metric_reduce_id;
    /* ... add other RPC identifiers here ... */
    uint8_t use_aggregator;
    uint32_t num_aggregators;
#ifdef USE_AGGREGATOR
    sdskv_client_t aggcl;
    sdskv_provider_handle_t * aggphs;
    sdskv_database_id_t * aggdbids;
#endif
} reducer_provider;

reducer_return_t reducer_provider_metric_reduce(reducer_metric_t m, reducer_provider_t provider);

#endif
