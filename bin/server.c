/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <reducer/reducer-server.h>
#include <reducer/reducer-metric.h>
#include <reducer/reducer-common.h>
#ifdef USE_SYMBIOMON
#include <symbiomon/symbiomon-server.h>
#endif

int main(int argc, char** argv)
{
    margo_instance_id mid = margo_init(argv[1], MARGO_SERVER_MODE, 1, 2);
    assert(mid);

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid,my_address);

    FILE *fp = fopen("reducer.add", "w");
    fprintf(fp, "%s %d\n", addr_str, 42);
    fclose(fp);

    struct reducer_provider_args args = REDUCER_PROVIDER_ARGS_INIT;

    reducer_provider_t provider;
    reducer_provider_register(mid, 42, &args, &provider);
#ifdef USE_SYMBIOMON
    /* initialize SYMBIOMON */
    struct symbiomon_provider_args symbiomon_args = SYMBIOMON_PROVIDER_ARGS_INIT;
    symbiomon_args.push_finalize_callback = 0;

    symbiomon_provider_t metric_provider;
    int ret = symbiomon_provider_register(mid, 42, &symbiomon_args, &metric_provider);
    if(ret != 0) {
        fprintf(stderr, "Error: symbiomon_provider_register() failed. Continuing on.\n");
    }

    ret = reducer_provider_set_symbiomon(provider, metric_provider);
    if(ret != 0)
        fprintf(stderr, "Error: reducer_provider_set_symbiomon() failed. Contuinuing on.\n");
#endif

    margo_wait_for_finalize(mid);

    return 0;
}
