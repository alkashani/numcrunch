#include "clenshaw.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define LONG_HALFWAY (LONG_MAX / 2)

struct cmdline_options {
    unsigned c_degree;  /* d */
    unsigned x_len;     /* p */
    unsigned rounds;    /* r */
    char *c_fname;      /* c */
    char *x_fname;      /* x */
    char *y_fname;      /* y */
} options;

bool simulate_mode = false;

struct points x, y;
struct coefficients c;

static void
usage(void) {
    printf("Two ways of running this program: simulate and actual:\n\n"
           "To simulate the input and skip writing output, provide:\n"
           "    1) the degree of coefficient\n"
           "    2) the number of data points\n"
           "    3) the number of repetitions to run\n"
           "actual numbers are randomly generated between -1.0 and 1.0, e.g.:\n"
           "    ./clenshaw -d 1000 -p 100 -r 10\n\n"
           "To use the program to actually calculate output for set input, "
           "please provide the filenames for:\n"
           "    1) -c, the coefficients\n"
           "    2) -x, the input points\n"
           "    3) -y, the output points\n"
           "each file should start with cardinality as a 4-byte integer, e.g.:\n"
           "    ./clenshaw -c c.in -x x.in -y y.out\n\n"
          );
}

static double *
read_doubles(unsigned card, FILE *f)
{
    double *val;
    size_t nbyte;

    val = alloc_doubles(card);
    nbyte = fread(val, sizeof(double), card, f);
    if (nbyte != card * sizeof(double)) {
        printf("fatal error: input size mismatches description. exit.");
        exit(EXIT_FAILURE);
    }

    return val;
}

static void
gen_doubles(double *val, unsigned card)
{
    unsigned i;

    for (i = 0; i < card; i++) {
        val[i] = (double)(random() - LONG_HALFWAY) / (LONG_HALFWAY + 1);
    }
}

static void
simulate(unsigned rounds)
{
    struct timeval tv_start, tv_finish;
    double elapsed_us;

    c.val = alloc_doubles(c.degree + 1);
    gen_doubles(c.val, c.degree + 1);

    x.val = alloc_doubles(x.len);
    gen_doubles(x.val, x.len);

    y.len = x.len;
    y.val = alloc_doubles(y.len);

    gettimeofday(&tv_start, NULL);
    for (int i = 0; i < rounds; i++) {
        clenshaw(&y, &x, &c);
    }
    gettimeofday(&tv_finish, NULL);


    elapsed_us = 1000000.0 * (tv_finish.tv_sec - tv_start.tv_sec) +
        tv_finish.tv_usec - tv_start.tv_usec;
    printf("total time:     %f s\ntime per round: %f ms\ntime per point: %f us\n",
           elapsed_us / 1000000, elapsed_us / 1000 / rounds,
           elapsed_us / rounds / x.len);
}

static void
parse_options(int argc, char **argv)
{
    char opt;

    while ((opt = getopt(argc, argv, "hd:p:r:c:x:y:")) != -1) {
        switch (opt) {
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 'd':
            simulate_mode = true;
            options.c_degree = (unsigned)atoi(optarg);
            break;
        case 'p':
            simulate_mode = true;
            options.x_len = (unsigned)atoi(optarg);
            break;
        case 'r':
            simulate_mode = true;
            options.rounds = (unsigned)atoi(optarg);
            break;
        case 'c':
            options.c_fname = optarg;
            break;
        case 'x':
            options.x_fname = optarg;
            break;
        case 'y':
            options.y_fname = optarg;
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }
}

int
main (int argc, char **argv)
{
    FILE *fc, *fx, *fy;
    size_t nbyte;

    parse_options(argc, argv);

    if (simulate_mode) {
        if (options.c_degree == 0 || options.x_len == 0 || options.rounds == 0) {
            printf("fatal error: illegal or missing arguments (-d, -p, -r)\n\n");
            exit(EXIT_FAILURE);
        }
        c.degree = options.c_degree;
        x.len = options.x_len;
        simulate(options.rounds);
        exit(EXIT_SUCCESS);
    }

    if (options.c_fname == NULL || options.x_fname == NULL ||
            options.y_fname == NULL) {
        printf("fatal error: illegal or missing arguments (-c, -x, -y)\n\n");
        exit(EXIT_FAILURE);
    }

    /* prep */
    fc = fopen(options.c_fname, "r");
    fx = fopen(options.x_fname, "r");
    fy = fopen(options.y_fname, "w");

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
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
