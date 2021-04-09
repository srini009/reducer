/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include "reducer/reducer-backend.h"
#include "../provider.h"
#include "dummy-backend.h"

typedef struct dummy_context {
  int dummy_member;
    /* ... */
} dummy_context;

static reducer_return_t dummy_create_metric(
        reducer_provider_t provider,
        void** context)
{
    (void)provider;

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    *context = (void*)ctx;
    return REDUCER_SUCCESS;
}

static reducer_return_t dummy_open_metric(
        reducer_provider_t provider,
        void** context)
{
    (void)provider;

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    *context = (void*)ctx;
    return REDUCER_SUCCESS;
}

static reducer_return_t dummy_close_metric(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    free(context);
    return REDUCER_SUCCESS;
}

static reducer_return_t dummy_destroy_metric(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    free(context);
    return REDUCER_SUCCESS;
}

static void dummy_say_hello(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    (void)context;
    printf("Hello World from Dummy metric\n");
}

static int32_t dummy_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static reducer_backend_impl dummy_backend = {
    .name             = "dummy",

    .create_metric  = dummy_create_metric,
    .open_metric    = dummy_open_metric,
    .close_metric   = dummy_close_metric,
    .destroy_metric = dummy_destroy_metric,

    .hello            = dummy_say_hello,
    .sum              = dummy_compute_sum
};

reducer_return_t reducer_provider_register_dummy_backend(reducer_provider_t provider)
{
    return reducer_provider_register_backend(provider, &dummy_backend);
}
