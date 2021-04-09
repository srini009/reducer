/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "reducer/reducer-admin.h"

reducer_return_t reducer_admin_init(margo_instance_id mid, reducer_admin_t* admin)
{
    reducer_admin_t a = (reducer_admin_t)calloc(1, sizeof(*a));
    if(!a) return REDUCER_ERR_ALLOCATION;

    a->mid = mid;

    *admin = a;
    return REDUCER_SUCCESS;
}

reducer_return_t reducer_admin_finalize(reducer_admin_t admin)
{
    free(admin);
    return REDUCER_SUCCESS;
}
