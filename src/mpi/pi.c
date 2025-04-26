#include <mpi.h>
#include <stdio.h>
#include <stdlib.h> // For atoi (string to integer)
#include <math.h>   // For fabs (absolute value of float/double)
#include <unistd.h> // For gethostname (optional, for printing)

int main(int argc, char *argv[])
{
    int my_rank, num_procs;
    long long i, num_intervals; // Use long long for potentially large number of intervals
    double local_pi, global_pi;
    double width, x, sum;
    double start_time, end_time, elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // --- Argument Handling (Rank 0 reads and broadcasts) ---
    if (my_rank == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "Usage: mpirun ... %s <num_intervals>\n", argv[0]);
            num_intervals = -1; // Signal error
        }
        else
        {
            num_intervals = atoll(argv[1]); // Use atoll for long long
            if (num_intervals <= 0)
            {
                fprintf(stderr, "Error: Number of intervals must be positive.\n");
                num_intervals = -1; // Signal error
            }
        }
    }

    // Broadcast the number of intervals from rank 0 to all other processes.
    // If rank 0 encountered an error, it broadcasts -1.
    MPI_Bcast(&num_intervals, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

    // All processes check if num_intervals is valid after broadcast
    if (num_intervals <= 0)
    {
        MPI_Finalize();
        return 1; // Exit if input was invalid
    }

    // --- Calculation Setup ---
    width = 1.0 / (double)num_intervals;
    sum = 0.0;

    // --- Start Timing ---
    // Ensure all processes start calculation roughly simultaneously (optional but good practice)
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime(); // Get MPI wall clock time

    // --- Parallel Calculation Loop ---
    // Each process calculates its share of intervals using a strided approach
    // Process 'my_rank' calculates intervals: my_rank, my_rank + num_procs, my_rank + 2*num_procs, ...
    for (i = my_rank; i < num_intervals; i += num_procs)
    {
        x = (i + 0.5) * width; // Midpoint of the interval
        sum += (4.0 / (1.0 + x * x));
    }

    local_pi = width * sum; // Local contribution to Pi

    // --- Reduction ---
    // Sum up all the 'local_pi' values from each process onto process 0.
    // The result will be stored in 'global_pi' *only on process 0*.
    MPI_Reduce(&local_pi, &global_pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // --- Stop Timing ---
    end_time = MPI_Wtime();
    elapsed_time = end_time - start_time;

    // --- Output Results (Rank 0 only) ---
    if (my_rank == 0)
    {
        printf("Number of Processes: %d\n", num_procs);
        printf("Number of Intervals: %lld\n", num_intervals);
        printf("Calculated Pi:      %.16f\n", global_pi);
        printf("Reference Pi:       %.16f\n", 3.14159265358979323846); // From math.h
        printf("Error:              %.16f\n", fabs(global_pi - 3.14159265358979323846));
        printf("Elapsed Time:       %.6f seconds\n", elapsed_time);
    }

    MPI_Finalize();
    return 0;
}