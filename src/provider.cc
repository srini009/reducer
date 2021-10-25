/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include <unistd.h>
#include "reducer/reducer-server.h"
#include "reducer/reducer-common.h"
#include "reducer/reducer-backend.h"
#include "provider.h"
#include "types.h"
#ifdef USE_AGGREGATOR
#include <sdskv-client.h>
#endif
#ifdef USE_SYMBIOMON
#include <symbiomon/symbiomon-metric.h>
#include <symbiomon/symbiomon-common.h>
#include <symbiomon/symbiomon-server.h>
#endif

static void reducer_finalize_provider(void* p);

/* Admin RPCs */

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(reducer_metric_reduce_ult)
static void reducer_metric_reduce_ult(hg_handle_t h);
/* add other RPC declarations here */

extern "C" int reducer_provider_register(
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

    margo_provider_registered_name(mid, "reducer_metric_reduce", provider_id, &id, &flag);
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
    margo_registered_disable_response(p->mid, p->metric_reduce_id, HG_TRUE); 

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
    free(provider);
    margo_info(provider->mid, "REDUCER provider successfuly finalized");
}

extern "C" int reducer_provider_destroy(
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
    bool trigger_metric_file_write = false;

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    reducer_provider_t provider = (reducer_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = REDUCER_ERR_FROM_MERCURY;
        hret = margo_free_input(h, &in);
        margo_destroy(h);
    }

    std::string keys_after(in.key_start);
    keys_after += "_";

    switch(in.op) {
        case REDUCER_REDUCTION_OP_SUM:
 	    keys_after += "SUM";
            break;
        case REDUCER_REDUCTION_OP_MIN:
 	    keys_after += "MIN";
            break;
        case REDUCER_REDUCTION_OP_MAX:
 	    keys_after += "MAX";
            break;
        case REDUCER_REDUCTION_OP_AVG:
 	    keys_after += "AVG";
            break;
        case REDUCER_REDUCTION_OP_ANOMALY:
 	    keys_after += "ANOMALY";
            break;
    }

    /* Prepare for the SDSKV call */
    std::string metric_name(in.name);
    metric_name += "_GLOBAL_";

    size_t max_keys = in.max_keys;
    size_t max_key_size = 256; //max size of stringified metric string
    size_t max_val_size = 8;  //put a limit on the max number of doubles you expect to receive
    std::vector<std::vector<char>> key_strings(max_keys, std::vector<char>(max_key_size+1));
    std::vector<std::vector<double>> val_doubles(max_keys, std::vector<double>(max_val_size+1, 0.0));
    std::vector<void*> keys(max_keys);
    std::vector<void*> vals(max_keys);
    std::vector<hg_size_t> ksizes(max_keys, max_key_size+1);
    std::vector<hg_size_t> vsizes(max_keys, max_val_size+1);

    for(unsigned i=0; i<max_keys; i++) {
        keys[i] = (void*)key_strings[i].data();
        vals[i] = (void*)val_doubles[i].data();
    } 

    std::string prefix(in.ns);
    prefix += "_";
    prefix += in.name;
    /* Make the SDSKV call */
    int ret = sdskv_list_keyvals(provider->aggphs[in.agg_id], provider->aggdbids[in.agg_id], 
                (const void*)keys_after.c_str(), keys_after.size()+1,
                keys.data(), ksizes.data(), vals.data(), vsizes.data(), &max_keys);
    assert(ret == SDSKV_SUCCESS);

#ifdef USE_SYMBIOMON
    symbiomon_taglist_t taglist;
    symbiomon_taglist_create(&taglist, 0);
    /* Perform global reduction */
    switch(in.op) {
        case REDUCER_REDUCTION_OP_SUM: {
            double sum = 0.0;
            symbiomon_metric_t m;
            for(unsigned int i = 0; i < max_keys; i++) {
              for(unsigned int j = 0; j < max_val_size; j++)
                sum += val_doubles[i][j];
            }
            metric_name += "SUM";
    	    ret = symbiomon_metric_create(in.ns, metric_name.c_str(), SYMBIOMON_TYPE_GAUGE, metric_name.c_str(), taglist, &m, provider->metric_provider);
            if(!ret) trigger_metric_file_write = true;
            symbiomon_metric_update(m, sum);
            break;
        }
        case REDUCER_REDUCTION_OP_MIN: {
              
            double min = 999999999999.0;
            if(val_doubles[0][0]) {min = val_doubles[0][0];}
            symbiomon_metric_t m;
            for(unsigned int i = 1; i < max_keys; i++) {
              for(unsigned int j = 0; j < max_val_size; j++) {
                if(!val_doubles[i][j]) { continue; }
                min = (min > val_doubles[i][j] ? val_doubles[i][j] : min);
              }
            }
            metric_name += "MIN";
    	    ret = symbiomon_metric_create(in.ns, metric_name.c_str(), SYMBIOMON_TYPE_GAUGE, metric_name.c_str(), taglist, &m, provider->metric_provider);
            if(!ret) trigger_metric_file_write = true;
            symbiomon_metric_update(m, min);
            break;
        }
        case REDUCER_REDUCTION_OP_MAX: {
            double max = val_doubles[0][0];
            symbiomon_metric_t m;
            for(unsigned int i = 1; i < max_keys; i++) {
              for(unsigned int j = 0; j < max_val_size; j++) {
                max = (max < val_doubles[i][j] ? val_doubles[i][j] : max);
              }
            }
            metric_name += "MAX";
    	    ret = symbiomon_metric_create(in.ns, metric_name.c_str(), SYMBIOMON_TYPE_GAUGE, metric_name.c_str(), taglist, &m, provider->metric_provider);
            if(!ret) trigger_metric_file_write = true;
            ret = symbiomon_metric_update(m, max);
            break;
        }
        case REDUCER_REDUCTION_OP_AVG: {
            double sum = 0.0, avg = 0.0;
            symbiomon_metric_t m;
            for(unsigned int i = 0; i < max_keys; i++) {
              for(unsigned int j = 0; j < max_val_size; j++)
                sum += val_doubles[i][j];
            }
            avg = sum/(double)max_keys;
            metric_name += "AVG";
    	    ret = symbiomon_metric_create(in.ns, metric_name.c_str(), SYMBIOMON_TYPE_GAUGE, metric_name.c_str(), taglist, &m, provider->metric_provider);
            if(!ret) trigger_metric_file_write = true;
            symbiomon_metric_update(m, avg);
            break;
        }
        case REDUCER_REDUCTION_OP_ANOMALY: {
            std::vector<double> flattened_data;
            symbiomon_metric_t m;
            double sum = 0, avg = 0, sd = 0;
            unsigned int num_actual_vals = 0;
	    flattened_data.reserve(max_keys*max_val_size);
            for(unsigned int i = 0; i < max_keys; i++) {
              for(unsigned int j = 0; j < max_val_size; j++) {
		if(val_doubles[i][j]) {
                  flattened_data.push_back(val_doubles[i][j]);
                  sum += val_doubles[i][j];
                  num_actual_vals++;
                }
              }
            }
            unsigned int current_index = num_actual_vals;
            if(!current_index) break;
	    avg = sum/(double)(current_index);
            for(unsigned int i=0; i < current_index; i++)
                sd += pow(flattened_data[i] - avg, 2);

            metric_name += "ANOMALY";
    	    ret = symbiomon_metric_create(in.ns, metric_name.c_str(), SYMBIOMON_TYPE_GAUGE, metric_name.c_str(), taglist, &m, provider->metric_provider);
            if(!ret) trigger_metric_file_write = true;
	    for(unsigned int i=0; i < current_index; i++) {
                if ((flattened_data[i] < avg-3*sd) || (flattened_data[i] > avg+3*sd)) {
                    symbiomon_metric_update(m, flattened_data[i]);
                }
            }
            break;
        }
    }

    if(trigger_metric_file_write == true) symbiomon_metric_list_all(provider->metric_provider, "reducer.metric_list"); 
#endif
    std::vector<std::string> res_k;
    for(auto ptr : keys) {
        res_k.push_back(std::string((const char*)ptr));
    }

    //for(unsigned int i = 0; i < max_keys; i++)
    //    std::cout << "Received val: " << val_doubles[i][0] << std::endl;
    /* set the response */
    out.ret = REDUCER_SUCCESS;
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}

static DEFINE_MARGO_RPC_HANDLER(reducer_metric_reduce_ult)

extern "C" reducer_return_t reducer_provider_register_backend()
{
    return REDUCER_SUCCESS;
}

#ifdef USE_SYMBIOMON
extern "C" int reducer_provider_set_symbiomon(reducer_provider_t provider, symbiomon_provider_t metric_provider)
{
    provider->metric_provider = metric_provider;
    return REDUCER_SUCCESS;
}
#endif
