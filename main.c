#include "clenshaw.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define LONG_HALFWAY (LONG_MAX / 2)
#define ALIGNMENT (size_t)64

struct points x, y;
struct coefficients c;

void usage(void) {
    printf("Two ways of running this program: simulate and actual:\n\n"
           "To simulate the input and skip writing output, provide the degree"
           "of coefficient and number of data points, actual numbers are"
           "randomly generated between -1.0 and 1.0, e.g.:\n"
           "    ./clenshaw 1000 100\n\n"
           "To use the program to actually calculate output for set input, "
           "please provide the filenames for:\n"
           "    1) the coefficients\n"
           "    2) the input points\n"
           "    3) the output points\n"
           "each file should start with cardinality as a 4-byte integer, e.g.:\n"
           "    ./clenshaw c.vector x.vector y.vector\n\n"
          );

    exit(-1);
}

double *
alloc_doubles(unsigned card) {
    double *val;

    if (posix_memalign((void **)&val, ALIGNMENT, card * sizeof(double)) != 0) {
        printf("fatal error: alloc failed due to OOM. exit.");
        exit(-1);
    }

    return val;
}

double *
read_doubles(unsigned card, FILE *f)
{
    double *val;
    size_t nbyte;

    val = alloc_doubles(card);
    nbyte = fread(val, sizeof(double), card, f);
    if (nbyte != card * sizeof(double)) {
        printf("fatal error: input size mismatches description. exit.");
        exit(-1);
    }

    return val;
}

void
gen_doubles(double *val, unsigned card)
{
    unsigned i;

    for (i = 0; i < card; i++) {
        val[i] = (double)(random() - LONG_HALFWAY) / (LONG_HALFWAY + 1);
    }
}

void
simulate(void)
{
    struct timeval tv_start, tv_finish;

    c.val = alloc_doubles(c.degree + 1);
    gen_doubles(c.val, c.degree + 1);

    x.val = alloc_doubles(x.len);
    gen_doubles(x.val, x.len);

    y.len = x.len;
    y.val = alloc_doubles(y.len);

    gettimeofday(&tv_start, NULL);
    clenshaw(&y, &x, &c);
    gettimeofday(&tv_finish, NULL);

    printf("elapsed time: %fs\n", (double)(1000000 * (tv_finish.tv_sec -
           tv_start.tv_sec) + tv_finish.tv_usec - tv_start.tv_usec) / 1000000);
}


int
main (int argc, char **argv)
{
    FILE *fc, *fx, *fy;
    size_t nbyte;

    if (argc != 3 && argc != 4) {
        usage();
    }

    if (argc == 3) { /* test mode, use randomized inputs and don't bother expose results */
        c.degree = (unsigned)atoi(argv[1]);
        x.len = (unsigned)atoi(argv[2]);

        simulate();

        return 0;
    }

    /* prep */
    fc = fopen(argv[0], "r");
    fx = fopen(argv[1], "r");
    fy = fopen(argv[2], "w");

    /* use degree to store cardinality first, and the adjust its value */
    fread(&c.degree, sizeof(uint32_t), 1, fc);
    c.val = read_doubles(c.degree, fc);
    c.degree--; /* degree is one less than cardinality */

    fread(&x.len, sizeof(uint32_t), 1, fx);
    x.val = read_doubles(x.len, fx);

    y.len = x.len;
    y.val = alloc_doubles(y.len);

    /* compute */
    clenshaw(&y, &x, &c);

    /* output */
    nbyte = fwrite(y.val, sizeof(double), y.len, fy);
    if (nbyte != y.len * sizeof(double)) {
        printf("fatal error: input size mismatches description. exit.");
        exit(-1);
    }

    return 0;
}
