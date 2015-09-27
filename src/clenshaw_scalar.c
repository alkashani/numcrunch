#include "clenshaw.h"

/**
 * @param[out] y: output points, memory should be preallocated
 * @param[in]  c: coefficient
 * @param[in]  x: input points
 *
 * @return:
 */
void
clenshaw(struct points *y, struct points *x, struct coefficients *c)
{
    int i, k;
    unsigned d;
    double b, x2;
    double b_even, b_odd;

    d = c->degree;
    if (d & 1) {
        /* do one assignment outside the loop to make # of iterations even */
        b = c->val[d];
        d--;
    } else {
        b = 0;
    }

    for (i = 0; i < x->len; i++) {
        x2 = 2 * x->val[i];
        b_even = 0;
        b_odd = b;

        for (k = d; k > 0; k -= 2) {
            b_even = c->val[k] + x2 * b_odd - b_even;
            b_odd = c->val[k-1] + x2 * b_even - b_odd;
        }
        y->val[i] = c->val[0] + x->val[i] * b_odd - b_even;
    }
}
