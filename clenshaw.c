#include "clenshaw.h"

#include <x86intrin.h>

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
    __m256d mx, mt, mc;
    __m256d be, bo;

    double b0, b;
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
        mx = _mm256_load_pd(&x->val[i]);
        mx = _mm256_mul_pd(mx, _mm256_broadcast_sd(&multi));
        be = _mm256_broadcast_sd(&b0);
        bo = _mm256_broadcast_sd(&b);

        for (k = d; k > 0; k -= 2) {
            mt = _mm256_fmsub_pd(mx, bo, be);
            be = _mm256_add_pd(mt, _mm256_broadcast_sd(&c->val[k]));
            mt = _mm256_fmsub_pd(mx, be, bo);
            bo = _mm256_add_pd(mt, _mm256_broadcast_sd(&c->val[k-1]));

            //b_even = c->val[k] + x2 * b_odd - b_even;
            //b_odd = c->val[k-1] + x2 * b_even - b_odd;
        }

        mx = _mm256_load_pd(&x->val[i]);
        mt = _mm256_fmsub_pd(mx, bo, be);
        be = _mm256_add_pd(mt, _mm256_broadcast_sd(&c->val[0]));

        _mm256_store_pd(&y->val[i], be);
        //y->val[i] = c->val[0] + x->val[i] * b_odd - b_even;
    }
}
