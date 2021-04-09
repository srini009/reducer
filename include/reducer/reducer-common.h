/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __REDUCER_COMMON_H
#define __REDUCER_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by REDUCER functions.
 */
typedef enum reducer_return_t {
    REDUCER_SUCCESS,
    REDUCER_ERR_INVALID_NAME,      /* Metric creation error - name or ns missing */
    REDUCER_ERR_ALLOCATION,        /* Allocation error */
    REDUCER_ERR_INVALID_ARGS,      /* Invalid argument */
    REDUCER_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    REDUCER_ERR_INVALID_METRIC,    /* Invalid metric id */
    REDUCER_ERR_METRIC_EXISTS,     /* Metric exists */
    REDUCER_ERR_INVALID_VALUE,     /* Invalid metric update value */
    REDUCER_ERR_INVALID_BACKEND,   /* Invalid backend type */
    REDUCER_ERR_INVALID_CONFIG,    /* Invalid configuration */
    REDUCER_ERR_INVALID_TOKEN,     /* Invalid token */
    REDUCER_ERR_FROM_MERCURY,      /* Mercury error */
    REDUCER_ERR_FROM_ARGOBOTS,     /* Argobots error */
    REDUCER_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    REDUCER_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    REDUCER_ERR_OTHER              /* Other error */
} reducer_return_t;

/**
 * @brief Identifier for a metric.
 */
typedef uint32_t reducer_metric_id_t;

typedef enum reducer_metric_reduction_op {
   REDUCER_REDUCTION_OP_NULL,
   REDUCER_REDUCTION_OP_SUM,
   REDUCER_REDUCTION_OP_AVG,
   REDUCER_REDUCTION_OP_MIN,
   REDUCER_REDUCTION_OP_MAX,
   REDUCER_REDUCTION_OP_ANOMALY
} reducer_metric_reduction_op_t;

inline uint32_t reducer_hash(char *str);

/* djb2 hash from Dan Bernstein */
inline uint32_t
reducer_hash(char *str)
{
    uint32_t hash = 5381;
    uint32_t c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

#ifdef __cplusplus
}
#endif

#endif
