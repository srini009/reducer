/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_ADMIN_H
#define __REDUCER_ADMIN_H

#include <margo.h>
#include <reducer/reducer-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct reducer_admin* reducer_admin_t;
#define REDUCER_ADMIN_NULL ((reducer_admin_t)NULL)

#define REDUCER_METRIC_ID_IGNORE ((reducer_metric_id_t*)NULL)

/**
 * @brief Creates a REDUCER admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin REDUCER admin
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h
 */
reducer_return_t reducer_admin_init(margo_instance_id mid, reducer_admin_t* admin);

/**
 * @brief Finalizes a REDUCER admin.
 *
 * @param[in] admin REDUCER admin to finalize
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h
 */
reducer_return_t reducer_admin_finalize(reducer_admin_t admin);

#endif
