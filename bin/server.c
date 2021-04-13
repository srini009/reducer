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

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    margo_instance_id mid = margo_init("ofi+verbs://", MARGO_SERVER_MODE, 0, 0);
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

    margo_wait_for_finalize(mid);

    return 0;
}
