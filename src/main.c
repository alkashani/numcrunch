#include "clenshaw.h"
#include "vectorized.h"

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>

#define LONG_HALFWAY (LONG_MAX / 2)

struct cmdline_options {
    unsigned c_degree;  /* d */
    unsigned x_len;     /* p */
    unsigned rounds;    /* r */
    unsigned threads;   /* t */
    char *c_fname;      /* c */
    char *x_fname;      /* x */
    char *y_fname;      /* y */
} options;

struct args {
    struct points *y;
    struct points *x;
    struct coefficients *c;
};

bool simulate_mode = false;

struct points x, y;
struct coefficients c;

static void
usage(char *cmd) {
    printf("Two ways of running this program: simulate and actual:\n\n"
           "To simulate the input and skip writing output, provide:\n"
           "    1) the degree of coefficient\n"
           "    2) the number of data points\n"
           "    3) the number of repetitions to run\n"
           "actual numbers are randomly generated between -1.0 and 1.0, e.g.:\n"
           "    %s -d 1000 -p 100 -r 10\n\n"
           "To use the program to actually calculate output for set input, "
           "please provide the filenames for:\n"
           "    1) -c, the coefficients\n"
           "    2) -x, the input points\n"
           "    3) -y, the output points\n"
           "each file should start with cardinality as a 4-byte integer, e.g.:\n"
           "    %s -c c.in -x x.in -y y.out\n\n"
           "If the number of threads to use is provided (with -t), the program "
           "will attempt to use multiple threads, e.g.:\n"
           "    %s -d 1000 -p 100 -r 10 -t 4\n\n", cmd, cmd, cmd
          );
}

static void *
thread_clenshaw_main(void *arg) {
    struct args *args = (struct args *)arg;

    clenshaw(args->y, args->x, args->c);

    return NULL;
}

static void
thread_clenshaw(void)
{
    /* input may need to be divided up into up to options.threads portions */
    struct args *subargs = malloc(options.threads * sizeof(struct args));
    struct points *subx = malloc(options.threads * sizeof(struct points));
    struct points *suby = malloc(options.threads * sizeof(struct points));
    pthread_t *pthreads = malloc(options.threads * sizeof(pthread_t));
    unsigned remain = x.len;
    /* number of doubles in x taken by all but the last thread will be aligned
     * with the vector size used in vectorized.h
     */
    unsigned xmin = ALIGNMENT / sizeof(double);
    unsigned xslice = MAX(ALIGN_UP(x.len / options.threads, xmin), xmin);
    unsigned i, xl, nthread = 0;
    double *xv = x.val;
    double *yv = y.val;

    while (remain > 0) {
        xl = MIN(remain, xslice);
        remain -= xl;
        subx[nthread] = (struct points){xl, xv};
        suby[nthread] = (struct points){xl, yv};
        subargs[nthread] = (struct args){&suby[nthread], &subx[nthread], &c};
        nthread++;
        xv += xl;
        yv += xl;
    }
    nthread--;

    for (i = 1; i <= nthread; i++) {
        /* TODO: handle pthread creation error */
        pthread_create(&pthreads[i], NULL, &thread_clenshaw_main, &subargs[i]);
    }

    /* first thread is the current thread */
    clenshaw(subargs[0].y, subargs[0].x, subargs[0].c);

    for (i = 1; i <= nthread; i++) {
        pthread_join(pthreads[i], NULL);
    }
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
        if (options.threads == 0) { /* -t not specified */
            clenshaw(&y, &x, &c);
        } else {
            thread_clenshaw();
        }
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

    while ((opt = getopt(argc, argv, "ht:d:p:r:c:x:y:")) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
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
        case 't':
            options.threads = (unsigned)atoi(optarg);
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
            usage(argv[0]);
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
    if (options.threads == 0) { /* -t not specified */
        clenshaw(&y, &x, &c);
    } else {
        thread_clenshaw();
    }

    /* output */
    nbyte = fwrite(y.val, sizeof(double), y.len, fy);
    if (nbyte != y.len * sizeof(double)) {
        printf("fatal error: input size mismatches description. exit.");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
