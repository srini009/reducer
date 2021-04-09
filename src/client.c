/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <stdarg.h>
#include "types.h"
#include "client.h"
#include "provider.h"
#include "reducer/reducer-client.h"
#include "reducer/reducer-common.h"

reducer_return_t reducer_client_init(margo_instance_id mid, reducer_client_t* client)
{
    reducer_client_t c = (reducer_client_t)calloc(1, sizeof(*c));
    if(!c) return REDUCER_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "reducer_metric_reduce", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "reducer_metric_reduce", &c->metric_reduce_id, &flag);
    } else {
        c->metric_reduce_id = MARGO_REGISTER(mid, "reducer_metric_reduce", metric_reduce_in_t, metric_reduce_out_t, NULL);
    }

    c->num_metric_handles = 0;
    *client = c;
    return REDUCER_SUCCESS;
}

reducer_return_t reducer_client_finalize(reducer_client_t client)
{
    if(client->num_metric_handles != 0) {
        fprintf(stderr,  
                "Warning: %ld metric handles not released when reducer_client_finalize was called\n",
                client->num_metric_handles);
    }
    free(client);
    return REDUCER_SUCCESS;
}

/* APIs for remote monitoring clients */
reducer_return_t reducer_metric_reduce(const char *ns, const char *name, reducer_metric_reduction_op_t op, reducer_metric_handle_t handle)
{
    if(!ns || !name)
        return REDUCER_ERR_INVALID_NAME;
    
    hg_handle_t h;
    metric_reduce_in_t in;
    metric_reduce_out_t out;

    hg_return_t ret;
    in.ns = ns;
    in.name = name;
    in.op = op;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->metric_reduce_id, &h);
    if(ret != HG_SUCCESS)         
        return REDUCER_ERR_FROM_MERCURY; 

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
	return REDUCER_ERR_FROM_MERCURY;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
	margo_free_output(h, &out);
        margo_destroy(h);
	return REDUCER_ERR_FROM_MERCURY;
    }

    margo_free_output(h, &out);
    margo_destroy(h);

}

reducer_return_t reducer_metric_handle_create(
        reducer_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        reducer_metric_handle_t* handle)
{
    if(client == REDUCER_CLIENT_NULL)
        return REDUCER_ERR_INVALID_ARGS;

    reducer_metric_handle_t rh =
        (reducer_metric_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return REDUCER_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return REDUCER_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->refcount    = 1;

    client->num_metric_handles += 1;

    *handle = rh;
    return REDUCER_SUCCESS;
}

reducer_return_t reducer_metric_handle_ref_incr(
        reducer_metric_handle_t handle)
{
    if(handle == REDUCER_METRIC_HANDLE_NULL)
        return REDUCER_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return REDUCER_SUCCESS;
}

reducer_return_t reducer_metric_handle_release(reducer_metric_handle_t handle)
{
    if(handle == REDUCER_METRIC_HANDLE_NULL)
        return REDUCER_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_metric_handles -= 1;
        free(handle);
    }
    return REDUCER_SUCCESS;
}
