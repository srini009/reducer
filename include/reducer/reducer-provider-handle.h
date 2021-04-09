/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_PROVIDER_HANDLE_H
#define __REDUCER_PROVIDER_HANDLE_H

#include <margo.h>
#include <reducer/reducer-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct reducer_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct reducer_provider_handle* reducer_provider_handle_t;
#define REDUCER_PROVIDER_HANDLE_NULL ((reducer_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
