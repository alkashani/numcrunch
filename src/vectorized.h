#include <x86intrin.h>

#define ALIGNMENT (size_t)32

typedef __m256d vdouble;

#define load_pd _mm256_load_pd
#define broadcast_sd _mm256_broadcast_sd
#define store_pd _mm256_store_pd

#define mul_pd _mm256_mul_pd
#define add_pd _mm256_add_pd
#define fmsub_pd _mm256_fmsub_pd
