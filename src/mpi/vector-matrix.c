#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memset
#include <mpi.h>
#include <time.h> // For srand

// Function to print a vector (useful for debugging)
void print_vector(const char *title, double *vec, int n, int rank, MPI_Comm comm)
{
    // Gather the vector to process 0 for printing
    double *full_vec = NULL;
    if (rank == 0)
    {
        full_vec = (double *)malloc(n * sizeof(double));
        if (!full_vec)
        {
            fprintf(stderr, "Failed to allocate memory for full vector print\n");
            MPI_Abort(comm, 1);
        }
    }

    // Determine local size based on whether n is divisible by comm_sz
    int comm_sz;
    MPI_Comm_size(comm, &comm_sz);
    int local_n = n / comm_sz; // This assumes n is divisible for simplicity here

    // Note: If n wasn't divisible, we'd need MPI_Gatherv
    MPI_Gather(vec, local_n, MPI_DOUBLE, full_vec, local_n, MPI_DOUBLE, 0, comm);

    if (rank == 0)
    {
        printf("--- %s (Process 0 view) ---\n[", title);
        for (int i = 0; i < n; i++)
        {
            printf("%.2f%s", full_vec[i], (i == n - 1) ? "" : ", ");
        }
        printf("]\n---------------------------\n");
        free(full_vec);
    }
    // Ensure output is flushed before proceeding
    fflush(stdout);
    MPI_Barrier(comm); // Synchronize to avoid interleaved printing from later steps
}

int main(int argc, char *argv[])
{
    int my_rank, comm_sz;
    int m, n;               // Matrix dimensions: m rows, n columns
    int local_m;            // Number of rows per process
    int local_n;            // Number of elements of x per process (initial distribution)
    double *local_A = NULL; // Local part of matrix A (local_m x n)
    double *local_x = NULL; // Local part of vector x (local_n elements)
    double *full_x = NULL;  // Full vector x (n elements), received via Allgather
    double *local_y = NULL; // Local part of result vector y (local_m elements)

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // --- Argument Parsing / Setting Dimensions ---
    if (argc != 3)
    {
        if (my_rank == 0)
        {
            fprintf(stderr, "Usage: %s <rows_m> <cols_n>\n", argv[0]);
            fprintf(stderr, "Using default values: m=9, n=6\n");
        }
        m = 9;
        n = 6;
    }
    else
    {
        m = atoi(argv[1]);
        n = atoi(argv[2]);
    }

    // --- Check for Divisibility ---
    if (m <= 0 || n <= 0 || m % comm_sz != 0 || n % comm_sz != 0)
    {
        if (my_rank == 0)
        {
            fprintf(stderr, "Error: m (%d) and n (%d) must be positive and divisible by comm_sz (%d)\n", m, n, comm_sz);
        }
        MPI_Finalize();
        return 1;
    }

    local_m = m / comm_sz;
    local_n = n / comm_sz;

    // --- Memory Allocation ---
    local_A = (double *)malloc(local_m * n * sizeof(double));
    local_x = (double *)malloc(local_n * sizeof(double));
    full_x = (double *)malloc(n * sizeof(double)); // Needs space for the entire x
    local_y = (double *)malloc(local_m * sizeof(double));

    if (!local_A || !local_x || !full_x || !local_y)
    {
        fprintf(stderr, "Rank %d: Failed to allocate memory.\n", my_rank);
        MPI_Abort(MPI_COMM_WORLD, 1); // Abort all processes
    }

    // --- Data Initialization (Example: Each process initializes its part) ---
    // Initialize local_A with unique values based on global row/col index
    srand(time(NULL) + my_rank); // Seed random number generator differently per rank
    int first_global_row = my_rank * local_m;
    for (int i = 0; i < local_m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            // Example value: global_row_index + j / 10.0
            // local_A[i * n + j] = (first_global_row + i) + (double)j / 10.0;
            local_A[i * n + j] = (double)(rand() % 100) / 10.0; // Random doubles 0.0 to 9.9
        }
    }

    // Initialize local_x with unique values based on global index
    int first_global_x_index = my_rank * local_n;
    for (int i = 0; i < local_n; i++)
    {
        // Example value: global_index
        // local_x[i] = (double)(first_global_x_index + i);
        local_x[i] = (double)(rand() % 10); // Random integers 0 to 9
    }

    // --- Print Initial Local Data (Optional Debugging) ---
    /*
    for(int p=0; p<comm_sz; ++p) {
        if(my_rank == p) {
             printf("Rank %d: local_A[0]=%f, local_x[0]=%f\n", my_rank, local_A[0], local_x[0]);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    */

    // --- Gather the full x vector on all processes ---
    // Each process sends 'local_n' elements from 'local_x'.
    // Each process receives 'comm_sz' blocks, each of size 'local_n', into 'full_x'.
    // The total size received is comm_sz * local_n = n.
    MPI_Allgather(local_x,         // Send buffer
                  local_n,         // Count of elements to send
                  MPI_DOUBLE,      // Type of elements to send
                  full_x,          // Receive buffer
                  local_n,         // Count of elements received *per process*
                  MPI_DOUBLE,      // Type of elements to receive
                  MPI_COMM_WORLD); // Communicator

    // --- Optional: Print the gathered full_x on rank 0 ---
    if (my_rank == 0)
    {
        printf("--- Full Vector x (gathered on Rank 0) ---\n[");
        for (int i = 0; i < n; i++)
        {
            printf("%.2f%s", full_x[i], (i == n - 1) ? "" : ", ");
        }
        printf("]\n----------------------------------------\n");
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD); // Ensure rank 0 prints before others proceed

    // --- Perform Local Matrix-Vector Multiplication ---
    // Initialize local_y to zeros
    memset(local_y, 0, local_m * sizeof(double));

    // Each process calculates its part of y using its local rows of A
    // and the *full* vector x.
    for (int i = 0; i < local_m; i++)
    { // Loop through local rows
        for (int j = 0; j < n; j++)
        { // Loop through columns (full width of A/x)
            local_y[i] += local_A[i * n + j] * full_x[j];
        }
    }

    // --- Output Results ---
    // Gather the local_y results onto process 0 to print the full y vector
    // (Or just print local parts for verification)
    print_vector("Result Vector y", local_y, m, my_rank, MPI_COMM_WORLD);

    // --- Verification (Optional: Serial calculation on rank 0) ---
    if (my_rank == 0)
    {
        printf("\n--- Verification (Serial on Rank 0) ---\n");
        // Need the full A and x on rank 0
        double *full_A = (double *)malloc(m * n * sizeof(double));
        double *full_y_serial = (double *)malloc(m * sizeof(double));

        if (!full_A || !full_y_serial)
        {
            fprintf(stderr, "Rank 0: Failed to allocate memory for verification.\n");
        }
        else
        {
            // Gather the full A matrix
            MPI_Gather(local_A, local_m * n, MPI_DOUBLE, full_A, local_m * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

            // x (full_x) is already available from Allgather

            // Perform serial calculation
            memset(full_y_serial, 0, m * sizeof(double));
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    full_y_serial[i] += full_A[i * n + j] * full_x[j];
                }
            }

            // Print serial result
            printf("[");
            for (int i = 0; i < m; i++)
            {
                printf("%.2f%s", full_y_serial[i], (i == m - 1) ? "" : ", ");
            }
            printf("]\n-------------------------------------\n");

            free(full_A);
            free(full_y_serial);
        }
    }
    else
    {
        // Other ranks participate in the Gather for verification A
        MPI_Gather(local_A, local_m * n, MPI_DOUBLE, NULL, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // --- Cleanup ---
    free(local_A);
    free(local_x);
    free(full_x);
    free(local_y);

    MPI_Finalize();
    return 0;
}