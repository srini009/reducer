/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include "reducer/reducer-server.h"
#include "reducer/reducer-common.h"
#include "reducer/reducer-backend.h"
#include "provider.h"
#include "types.h"
#ifdef USE_AGGREGATOR
#include <sdskv-client.h>
#endif

static void reducer_finalize_provider(void* p);

/* Admin RPCs */

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(reducer_metric_reduce_ult)
static void reducer_metric_reduce_ult(hg_handle_t h);
/* add other RPC declarations here */

int reducer_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct reducer_provider_args* args,
        reducer_provider_t* provider)
{
    struct reducer_provider_args a = REDUCER_PROVIDER_ARGS_INIT;
    if(args) a = *args;
    reducer_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    margo_info(mid, "Registering REDUCER provider with provider id %u", provider_id);

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        margo_error(mid, "Margo instance is not a server");
        return REDUCER_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "reducer_remote_metric_fetch", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        margo_error(mid, "Provider with the same provider id (%u) already register", provider_id);
        return REDUCER_ERR_INVALID_PROVIDER;
    }

    p = (reducer_provider_t)calloc(1, sizeof(*p));
    if(p == NULL) {
        margo_error(mid, "Could not allocate memory for provider");
        return REDUCER_ERR_ALLOCATION;
    }

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = a.pool;
    p->abtio = a.abtio;

    /* Admin RPCs */

    /* Client RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "reducer_metric_reduce",
            metric_reduce_in_t, metric_reduce_out_t,
            reducer_metric_reduce_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->metric_reduce_id = id;

    p->use_aggregator = 0;

    /* add other RPC registration here */
    /* ... */

    /* add backends available at compiler time (e.g. default/dummy backends) */

#ifdef USE_AGGREGATOR
    #define MAXCHAR 100
    FILE *fp_agg = NULL;
    char * aggregator_addr_file = getenv("AGGREGATOR_ADDRESS_FILE");
    char db_name[128];
    if(aggregator_addr_file) {
        char svr_addr_str[MAXCHAR];
        uint16_t p_id;
        fp_agg = fopen(aggregator_addr_file, "r");
        int i = 0;
        fscanf(fp_agg, "%d\n", &(p->num_aggregators));
        sdskv_client_init(mid, &p->aggcl);
        sdskv_provider_handle_t *aggphs = (sdskv_provider_handle_t *)malloc(sizeof(sdskv_provider_handle_t)*p->num_aggregators);
        sdskv_database_id_t *aggdbids = (sdskv_database_id_t *)malloc(sizeof(sdskv_database_id_t)*p->num_aggregators);
        while(fscanf(fp_agg, "%s %u %s\n", svr_addr_str, &p_id, db_name) != EOF) {
          hg_addr_t svr_addr; 
          int hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
          assert(hret == HG_SUCCESS);
	  hret = sdskv_provider_handle_create(p->aggcl, svr_addr, p_id, &(aggphs[i]));
	  assert(hret == SDSKV_SUCCESS);
	  hret = sdskv_open(aggphs[i], db_name, &aggdbids[i]); 
	  assert(hret == SDSKV_SUCCESS);
          i++;
        }
        p->use_aggregator = 1;
        p->aggphs = aggphs;
        p->aggdbids = aggdbids;
	fprintf(stderr, "Successfully setup aggregator support with num_aggregators: %d \n", p->num_aggregators);
    } else {
        fprintf(stderr, "AGGREGATOR_ADDRESS_FILE is not set. Continuing on without aggregator support");
    }
#endif

    if(a.push_finalize_callback)
        margo_provider_push_finalize_callback(mid, p, &reducer_finalize_provider, p);

    if(provider)
        *provider = p;
    margo_info(mid, "REDUCER provider registration done");
    return REDUCER_SUCCESS;
}

static void reducer_finalize_provider(void* p)
{
    reducer_provider_t provider = (reducer_provider_t)p;
    margo_info(provider->mid, "Finalizing REDUCER provider");
    margo_deregister(provider->mid, provider->metric_reduce_id);
    /* deregister other RPC ids ... */
    remove_all_metrics(provider);
    free(provider);
    margo_info(provider->mid, "REDUCER provider successfuly finalized");
}

int reducer_provider_destroy(
        reducer_provider_t provider)
{
    margo_instance_id mid = provider->mid;
    margo_info(mid, "Destroying REDUCER provider");
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    reducer_finalize_provider(provider);
    margo_info(mid, "REDUCER provider successfuly destroyed");
    return REDUCER_SUCCESS;
}

static void reducer_metric_reduce_ult(hg_handle_t h)
{
    hg_return_t hret;
    metric_reduce_in_t  in;
    metric_reduce_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    reducer_provider_t provider = (reducer_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = REDUCER_ERR_FROM_MERCURY;
        goto finish;
    }

    fprintf(stderr, "Trying to reduce metric with name: %s, and ns: %s\n", in.name, in.ns);

    /* set the response */
    out.ret = REDUCER_SUCCESS;

finish:
    free(b);
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}

static DEFINE_MARGO_RPC_HANDLER(reducer_metric_reduce_ult)

reducer_return_t reducer_provider_register_backend()
{
    return REDUCER_SUCCESS;
}
