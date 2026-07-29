#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Implemented in accel.asm
// mat = pointer to input matrix (n rows x 3 doubles: Vi, Vf, T)
// out = pointer to output array (n ints)
// n   = number of rows
extern void compute_accel(const double* mat, int* out, long long n);

static int reference_accel(double vi, double vf, double t) {
    double accel_mps2 = ((vf - vi) * (1000.0 / 3600.0)) / t;
    // round to nearest int, matching cvtsd2si's round-to-nearest behavior
    return (int)round(accel_mps2);
}

int main(void) {
    long long n;

    printf("Enter number of cars: ");
    if (scanf("%lld", &n) != 1 || n <= 0) {
        printf("Invalid input.\n");
        return 1;
    }

    double* mat = (double*)malloc((size_t)(n * 3) * sizeof(double));
    int* out = (int*)malloc((size_t)n * sizeof(int));

    if (mat == NULL || out == NULL) {
        printf("Memory allocation failed.\n");
        free(mat);
        free(out);
        return 1;
    }

    printf("Enter %lld rows of \"Vi, Vf, T\":\n", n);
    for (long long i = 0; i < n; i++) {
        if (scanf("%lf , %lf , %lf",
            &mat[i * 3 + 0],
            &mat[i * 3 + 1],
            &mat[i * 3 + 2]) != 3) {
            printf("Invalid row at index %lld.\n", i);
            free(mat);
            free(out);
            return 1;
        }
    }

   
    compute_accel(mat, out, n);

    // Correctness check C vs ASM
    printf("\n%-6s %-12s %-12s %-8s\n", "Row", "EXPECTED", "ACTUAL", "RESULT");
    printf("------------------------------------------\n");

    int all_match = 1;
    for (long long i = 0; i < n; i++) {
        double vi = mat[i * 3 + 0];
        double vf = mat[i * 3 + 1];
        double t = mat[i * 3 + 2];

        int expected = reference_accel(vi, vf, t);
        int actual = out[i];
        int ok = (expected == actual);
        if (!ok) all_match = 0;

        printf("%-6lld %-12d %-12d %-8s\n",
            i, expected, actual, ok ? "OK" : "MISMATCH");
    }

    printf("------------------------------------------\n");
    printf("Overall: %s\n", all_match ? "ALL ROWS MATCH" : "MISMATCH FOUND");

    free(mat);
    free(out);
    return 0;
}