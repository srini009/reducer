/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include "reducer/reducer-server.h"
#include "reducer/reducer-client.h"
#include "reducer/reducer-admin.h"
#include "reducer/reducer-provider-handle.h"
#include "client.h"
#include <string.h>

static int reducer_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    uint16_t provider_id  = bedrock_args_get_provider_id(args);

    struct reducer_provider_args reducer_args = { 0 };
    reducer_args.config = bedrock_args_get_config(args);
    reducer_args.pool   = bedrock_args_get_pool(args);

    reducer_args.abtio = (abt_io_instance_id)
        bedrock_args_get_dependency(args, "abt_io", 0);

    return reducer_provider_register(mid, provider_id, &reducer_args,
                                   (reducer_provider_t*)provider);
}

static int reducer_deregister_provider(
        bedrock_module_provider_t provider)
{
    return reducer_provider_destroy((reducer_provider_t)provider);
}

static char* reducer_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    // TODO
    return strdup("{}");
}

static int reducer_init_client(
        margo_instance_id mid,
        bedrock_module_client_t* client)
{
    return reducer_client_init(mid, (reducer_client_t*)client);
}

static int reducer_finalize_client(
        bedrock_module_client_t client)
{
    return reducer_client_finalize((reducer_client_t)client);
}

static int reducer_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    reducer_client_t c = (reducer_client_t)client;
    reducer_provider_handle_t tmp = calloc(1, sizeof(*tmp));
    margo_addr_dup(c->mid, address, &(tmp->addr));
    tmp->provider_id = provider_id;
    *ph = (bedrock_module_provider_handle_t)tmp;
    return BEDROCK_SUCCESS;
}

static int reducer_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    reducer_provider_handle_t tmp = (reducer_provider_handle_t)ph;
    margo_addr_free(tmp->mid, tmp->addr);
    free(tmp);
    return BEDROCK_SUCCESS;
}

static struct bedrock_module reducer = {
    .register_provider       = reducer_register_provider,
    .deregister_provider     = reducer_deregister_provider,
    .get_provider_config     = reducer_get_provider_config,
    .init_client             = reducer_init_client,
    .finalize_client         = reducer_finalize_client,
    .create_provider_handle  = reducer_create_provider_handle,
    .destroy_provider_handle = reducer_destroy_provider_handle,
    .provider_dependencies   = NULL,
    .client_dependencies     = NULL
};

BEDROCK_REGISTER_MODULE(reducer, reducer)
