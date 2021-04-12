/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _PARAMS_H
#define _PARAMS_H

#include <mercury.h>
#include <abt.h>
#include <mercury_macros.h>
#include <mercury_proc.h>
#include <mercury_proc_string.h>
#include "reducer/reducer-common.h"
#include "uthash.h"

/* Admin RPC types */

/* Client RPC types */

MERCURY_GEN_PROC(metric_reduce_in_t,
	((hg_string_t)(name))\
	((hg_string_t)(ns))\
	((hg_string_t)(key_start))\
	((hg_size_t)(max_keys))\
        ((int32_t)(agg_id))\
        ((int32_t)(op)))

MERCURY_GEN_PROC(metric_reduce_out_t,
        ((int32_t)(ret)))

typedef struct reducer_metric {
    reducer_metric_reduction_op_t reduction_op;
    char name[128];
    char ns[128];
#ifdef USE_AGGREGATOR
    char stringify[256];
    reducer_metric_id_t aggregator_id;
#endif
    UT_hash_handle      hh;
} reducer_metric;

typedef reducer_metric* reducer_metric_t;

#endif
