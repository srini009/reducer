/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_SERVER_H
#define __REDUCER_SERVER_H

#include <reducer/reducer-common.h>
#include <margo.h>
#include <abt-io.h>
#ifdef USE_SYMBIOMON
#include <symbiomon/symbiomon-server.h>
#include <symbiomon/symbiomon-metric.h>
#include <symbiomon/symbiomon-common.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define REDUCER_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct reducer_provider* reducer_provider_t;
#define REDUCER_PROVIDER_NULL ((reducer_provider_t)NULL)
#define REDUCER_PROVIDER_IGNORE ((reducer_provider_t*)NULL)

struct reducer_provider_args {
    uint8_t            push_finalize_callback;
    const char*        token;  // Security token
    const char*        config; // JSON configuration
    ABT_pool           pool;   // Pool used to run RPCs
    abt_io_instance_id abtio;  // ABT-IO instance
    // ...
};

#define REDUCER_PROVIDER_ARGS_INIT { \
    .push_finalize_callback = 1,\
    .token = NULL, \
    .config = NULL, \
    .pool = ABT_POOL_NULL, \
    .abtio = ABT_IO_INSTANCE_NULL \
}

/**
 * @brief Creates a new REDUCER provider. If REDUCER_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling margo_finalize.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] args argument structure
 * @param[out] provider provider
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h
 */
int reducer_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct reducer_provider_args* args,
        reducer_provider_t* provider);

/**
 * @brief Destroys the Alpha provider and deregisters its RPC.
 *
 * @param[in] provider Alpha provider
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h
 */
int reducer_provider_destroy(
        reducer_provider_t provider);

#ifdef USE_SYMBIOMON
/* Set symbiomon_provider_t instance for metrics reporting*/
int reducer_provider_set_symbiomon(reducer_provider_t provider, symbiomon_provider_t metric_provider);
#endif

#ifdef __cplusplus
}
#endif

#endif
