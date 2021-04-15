/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_METRIC_H
#define __REDUCER_METRIC_H

#include <margo.h>
#include <reducer/reducer-common.h>
#include <reducer/reducer-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct reducer_metric_handle *reducer_metric_handle_t;
typedef struct reducer_provider* reducer_provider_t;
#define REDUCER_METRIC_HANDLE_NULL ((reducer_metric_handle_t)NULL)

/* APIs for remote clients to request for performance data reduction */
reducer_return_t reducer_metric_reduce(const char *ns, const char *name, const char *key_start, uint32_t agg_id, reducer_metric_reduction_op_t op, reducer_metric_handle_t handle, size_t num_vals, size_t cohort_size);
reducer_return_t reducer_metric_handle_create(reducer_client_t client, hg_addr_t addr, uint16_t provider_id, reducer_metric_handle_t* handle);
reducer_return_t reducer_metric_handle_ref_incr(reducer_metric_handle_t handle);
reducer_return_t reducer_metric_handle_release(reducer_metric_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
