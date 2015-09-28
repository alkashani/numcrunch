#include "clenshaw.h"
#include "vectorized.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @param[in]  card: number of doubles to allocate
 *
 * allocation honors alignment set with ALIGNMENT.
 *
 * @return: point to the array of doubles allocated, or NULL if unsuccessful
 */
double *
alloc_doubles(unsigned card) {
    double *val;

    if (posix_memalign((void **)&val, ALIGNMENT, card * sizeof(double)) != 0) {
        printf("fatal error: alloc failed due to OOM. exit.");
        exit(EXIT_FAILURE);
    }

    return val;
}

/**
 * @param[out] y: output points, memory should be preallocated
 * @param[in]  c: coefficient
 * @param[in]  x: input points
 *
 * values of these structs has to be 32-byte aligned!
 *
 * @return:
 */
void
clenshaw(struct points *y, struct points *x, struct coefficients *c)
{
    int i, k;
    unsigned d;
    vdouble mx, mt;
    vdouble be, bo;

    double b, b0 = 0;
    const double multi = 2.0;

    d = c->degree;
    if (d & 1) {
        /* do one assignment outside the loop to make # of iterations even */
        b = c->val[d];
        d--;
    } else {
        b = 0;
    }

    for (i = 0; i < x->len; i += 4) {
        mx = load_pd(&x->val[i]);
        mx = mul_pd(mx, broadcast_sd(&multi));
        be = broadcast_sd(&b0);
        bo = broadcast_sd(&b);

        for (k = d; k > 0; k -= 2) {
            mt = fmsub_pd(mx, bo, be);
            be = add_pd(mt, broadcast_sd(&c->val[k]));
            mt = fmsub_pd(mx, be, bo);
            bo = add_pd(mt, broadcast_sd(&c->val[k-1]));
        }

        mx = load_pd(&x->val[i]);
        mt = fmsub_pd(mx, bo, be);
        be = add_pd(mt, broadcast_sd(&c->val[0]));

        store_pd(&y->val[i], be);
    }
}
