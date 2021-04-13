/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_BACKEND_H
#define __REDUCER_BACKEND_H

#include <reducer/reducer-server.h>
#include <reducer/reducer-common.h>

typedef reducer_return_t (*reducer_backend_create_fn)(reducer_provider_t, void**);
typedef reducer_return_t (*reducer_backend_open_fn)(reducer_provider_t, void**);
typedef reducer_return_t (*reducer_backend_close_fn)(void*);
typedef reducer_return_t (*reducer_backend_destroy_fn)(void*);

/**
 * @brief Implementation of an REDUCER backend.
 */
typedef struct reducer_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    reducer_backend_create_fn   create_metric;
    reducer_backend_open_fn     open_metric;
    reducer_backend_close_fn    close_metric;
    reducer_backend_destroy_fn  destroy_metric;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
} reducer_backend_impl;

/**
 * @brief Registers a backend implementation to be used by the
 * specified provider.
 *
 * Note: the backend implementation will not be copied; it is
 * therefore important that it stays valid in memory until the
 * provider is destroyed.
 *
 * @param provider provider.
 * @param backend_impl backend implementation.
 *
 * @return REDUCER_SUCCESS or error code defined in reducer-common.h 
 */
reducer_return_t reducer_provider_register_backend();

#endif
