#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

// Implemented in accel.asm
extern void compute_accel(const double* mat, int* out, long long n);

#define NUM_RUNS 30

// Fill a matrix with random-but-plausible Vi, Vf, T values.
// Vi: 0-60 km/h, Vf: Vi+20 to Vi+150 km/h, T: 3-15 seconds
static void generate_random_data(double* mat, long long n) {
    for (long long i = 0; i < n; i++) {
        double vi = (rand() % 601) / 10.0;                 // 0.0 - 60.0
        double vf = vi + 20.0 + (rand() % 1301) / 10.0;     // vi+20.0 - vi+150.0
        double t = 3.0 + (rand() % 121) / 10.0;            // 3.0 - 15.0

        mat[i * 3 + 0] = vi;
        mat[i * 3 + 1] = vf;
        mat[i * 3 + 2] = t;
    }
}

// Times NUM_RUNS individual calls to compute_accel for a given n.
// Prints every individual run time, then the average at the end.
// Returns the average seconds per call, or -1.0 on allocation failure.
static double run_benchmark(long long n, LARGE_INTEGER freq) {
    double* mat = (double*)malloc((size_t)(n * 3) * sizeof(double));
    int* out = (int*)malloc((size_t)n * sizeof(int));

    if (mat == NULL || out == NULL) {
        printf("Allocation failed for n=%lld\n", n);
        free(mat);
        free(out);
        return -1.0;
    }

    generate_random_data(mat, n);

    // Warm-up call (not timed) - avoids first-call overhead skewing results
    compute_accel(mat, out, n);

    printf("\n--- Y = %lld ---\n", n);

    double total_seconds = 0.0;
    LARGE_INTEGER start, end;

    for (int run = 0; run < NUM_RUNS; run++) {
        QueryPerformanceCounter(&start);
        compute_accel(mat, out, n);
        QueryPerformanceCounter(&end);

        double run_seconds = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
        total_seconds += run_seconds;

        printf("  Run %2d: %.9f s\n", run + 1, run_seconds);
    }

    double avg_seconds = total_seconds / NUM_RUNS;
    printf("  Average over %d runs: %.9f s\n", NUM_RUNS, avg_seconds);

    free(mat);
    free(out);
    return avg_seconds;
}

int main(void) {
    srand((unsigned int)time(NULL));

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq); // ticks per second, queried once

    long long sizes[] = { 10, 100, 1000, 10000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("Benchmarking compute_accel: %d individual runs per size (QueryPerformanceCounter)\n", NUM_RUNS);

    double averages[4];
    for (int i = 0; i < num_sizes; i++) {
        averages[i] = run_benchmark(sizes[i], freq);
    }

    printf("\n=== Summary ===\n");
    printf("%-10s %-24s\n", "Y (rows)", "Avg time (seconds)");
    printf("--------------------------------------------\n");
    for (int i = 0; i < num_sizes; i++) {
        if (averages[i] >= 0.0) {
            printf("%-10lld %-24.9f\n", sizes[i], averages[i]);
        }
    }

    return 0;
}