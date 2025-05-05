#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memset
#include <mpi.h>
// #include <time.h> // No longer needed for srand

int main(int argc, char *argv[])
{
    int my_rank, comm_sz;

    // --- Hardcoded Dimensions (ensure divisible by comm_sz) ---
    // Using the defaults you mentioned m=9, n=6 which works for comm_sz=3
    const int m = 9;
    const int n = 6;

    int local_m;            // Number of rows per process
    int local_n;            // Number of elements of x per process (initial distribution)
    double *local_A = NULL; // Local part of matrix A (local_m x n)
    double *local_x = NULL; // Local part of vector x (local_n elements)
    double *full_x = NULL;  // Full vector x (n elements), received via Allgather
    double *local_y = NULL; // Local part of result vector y (local_m elements)

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // --- Check for Divisibility (Still Important!) ---
    if (m <= 0 || n <= 0 || m % comm_sz != 0 || n % comm_sz != 0)
    {
        if (my_rank == 0)
        {
            // Use fprintf for error messages
            fprintf(stderr, "Error: Hardcoded m (%d) or n (%d) is not divisible by comm_sz (%d)\n", m, n, comm_sz);
        }
        MPI_Finalize();
        return 1;
    }

    local_m = m / comm_sz;
    local_n = n / comm_sz;

    // --- Memory Allocation ---
    // Use calloc to initialize to zero, slightly simpler than malloc+memset for y
    local_A = (double *)malloc(local_m * n * sizeof(double));
    local_x = (double *)malloc(local_n * sizeof(double));
    full_x = (double *)malloc(n * sizeof(double));
    local_y = (double *)calloc(local_m, sizeof(double)); // Allocates and zeros

    if (!local_A || !local_x || !full_x || !local_y)
    {
        fprintf(stderr, "Rank %d: Failed to allocate memory (m=%d, n=%d, local_m=%d, local_n=%d).\n", my_rank, m, n, local_m, local_n);
        MPI_Abort(MPI_COMM_WORLD, 1); // Abort all processes
    }

    // --- Simplified Data Initialization ---
    // Initialize local_A (just use rank number for simplicity)
    for (int i = 0; i < local_m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            local_A[i * n + j] = (double)(my_rank + 1.0); // Each rank gets rows with value 1.0, 2.0, 3.0 etc.
        }
    }

    // Initialize local_x (just use rank number differently)
    for (int i = 0; i < local_n; i++)
    {
        local_x[i] = (double)(my_rank + 0.5); // Rank 0 gets 0.5, Rank 1 gets 1.5, etc.
    }

    if (my_rank == 0)
    {
        printf("Rank 0: Setup complete. local_m=%d, local_n=%d\n", local_m, local_n);
        fflush(stdout);
    }

    // --- Gather the full x vector on all processes ---
    MPI_Allgather(local_x,         // Send buffer
                  local_n,         // Count of elements to send
                  MPI_DOUBLE,      // Type of elements to send
                  full_x,          // Receive buffer
                  local_n,         // Count of elements received *per process*
                  MPI_DOUBLE,      // Type of elements to receive
                  MPI_COMM_WORLD); // Communicator

    // --- Minimal check after Allgather ---
    if (my_rank == 0)
    {
        printf("Rank 0: MPI_Allgather finished. First element of full_x = %.2f\n", full_x[0]);
        fflush(stdout);
    }
    // Barrier to make sure rank 0 prints before others start heavy computation
    // and potentially print their "done" message too early.
    MPI_Barrier(MPI_COMM_WORLD);

    // --- Perform Local Matrix-Vector Multiplication ---
    // local_y was already initialized to zero by calloc

    for (int i = 0; i < local_m; i++)
    { // Loop through local rows
        for (int j = 0; j < n; j++)
        { // Loop through columns (full width of A/x)
            local_y[i] += local_A[i * n + j] * full_x[j];
        }
    }

    // --- Minimal Output - Show computation finished ---
    // Stagger output slightly using a barrier and rank check
    for (int r = 0; r < comm_sz; ++r)
    {
        if (my_rank == r)
        {
            printf("Rank %d: Computation finished. local_y[0] = %.2f\n", my_rank, local_y[0]);
            fflush(stdout); // Make sure output is visible immediately
        }
        MPI_Barrier(MPI_COMM_WORLD); // Wait for current rank to print
    }

    // --- Cleanup ---
    free(local_A);
    free(local_x);
    free(full_x);
    free(local_y);

    if (my_rank == 0)
    {
        printf("Rank 0: MPI_Finalize next.\n");
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}