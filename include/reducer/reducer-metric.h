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
typedef struct reducer_metric *reducer_metric_t;
typedef struct reducer_provider* reducer_provider_t;
#define REDUCER_METRIC_HANDLE_NULL ((reducer_metric_handle_t)NULL)

/* APIs for remote clients to request for performance data reduction */
reducer_return_t reducer_metric_register(const char *ns, const char *name, reducer_metric_reduction_op_t op, reducer_metric_handle_t handle, reducer_metric_id_t* metric_id);
reducer_return_t reducer_metric_unregister(reducer_metric_id_t metric_id, reducer_metric_handle_t handle);
reducer_return_t reducer_metric_reduce(reducer_metric_id_t metric_id, reducer_metric_handle_t handle);
reducer_return_t reducer_metric_reduce_all(reducer_metric_handle_t handle);
reducer_return_t reducer_metric_handle_create(reducer_client_t client, hg_addr_t addr, uint16_t provider_id, reducer_metric_id_t metric_id, reducer_metric_handle_t* handle);
reducer_return_t reducer_metric_handle_ref_incr(reducer_metric_handle_t handle);
reducer_return_t reducer_metric_handle_release(reducer_metric_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif