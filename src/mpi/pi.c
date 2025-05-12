#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> // For timing

int main(int argc, char *argv[])
{
    int rank, size;
    long long i;
    long long num_intervals; // Use long long for potentially large numbers
    double step, x, sum, pi, local_pi;
    double start_time, end_time, elapsed_time, total_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // --- Argument Handling (Get number of intervals from command line) ---
    if (argc > 1)
    {
        num_intervals = atoll(argv[1]); // Use atoll for long long
    }
    else
    {
        num_intervals = 100000000; // Default to a reasonably large number
    }

    if (rank == 0)
    {
        printf("Calculating Pi using %lld intervals across %d processes.\n", num_intervals, size);
    }

    // Start timing AFTER initialization and argument parsing
    MPI_Barrier(MPI_COMM_WORLD); // Synchronize before starting timer
    start_time = MPI_Wtime();

    // --- Broadcast the number of intervals to all processes ---
    // Although passed as arg, broadcasting ensures consistency if logic changed
    MPI_Bcast(&num_intervals, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // --- Calculation ---
    sum = 0.0;
    step = 1.0 / (double)num_intervals;

    // Each process calculates its portion of the intervals
    for (i = rank; i < num_intervals; i += size)
    {
        x = (i + 0.5) * step;       // Midpoint of the interval
        sum += 4.0 / (1.0 + x * x); // Formula derived from integral of 4/(1+x^2) dx from 0 to 1
    }
    local_pi = step * sum;

    // --- Reduction ---
    // Sum up all the local_pi values calculated by each process onto the root process (rank 0)
    MPI_Reduce(&local_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    elapsed_time = end_time - start_time;

    // Get the maximum elapsed time across all processes for accurate reporting
    MPI_Reduce(&elapsed_time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // --- Output Result (only on root process) ---
    if (rank == 0)
    {
        printf("Calculated Pi = %.15f\n", pi);
        printf("Reference Pi  = %.15f\n", M_PI); // From math.h
        printf("Error         = %.15f\n", fabs(pi - M_PI));
        printf("Total execution time: %f seconds\n", total_time);
    }

    MPI_Finalize();
    return 0;
}