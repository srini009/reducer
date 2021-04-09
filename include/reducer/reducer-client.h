/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_CLIENT_H
#define __REDUCER_CLIENT_H

#include <margo.h>
#include <reducer/reducer-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct reducer_client* reducer_client_t;
#define REDUCER_CLIENT_NULL ((reducer_client_t)NULL)

/**
 * @brief Creates a REDUCER client.
 *
 * @param[in] mid Margo instance
 * @param[out] client REDUCER client
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h
 */
reducer_return_t reducer_client_init(margo_instance_id mid, reducer_client_t* client);

/**
 * @brief Finalizes a REDUCER client.
 *
 * @param[in] client REDUCER client to finalize
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h
 */
reducer_return_t reducer_client_finalize(reducer_client_t client);

#ifdef __cplusplus
}
#endif

#endif
