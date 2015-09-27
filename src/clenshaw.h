#pragma once

/**
 * represents a vector whose offsets correspond to the natural polynomial
 * representation, i.e. c_0, c_1, ..., c_d
 */
struct coefficients {
    unsigned degree;
    double *val; /* array of cardinality (degree + 1) */
};

/**
 * represents a vector of scalar points
 */
struct points {
    unsigned len;
    double *val; /* array of cardinality len */
};

void clenshaw(struct points *y, struct points *x, struct coefficients *c);
