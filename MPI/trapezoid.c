#include <mpi.h>
#include <stdio.h>
#include <stdlib.h> // For atoll, exit
#include <math.h>   // For the function f(x)

// Function to integrate
double f(double x)
{
    // Example: Integrate x^2
    return x * x;
    // Example: Integrate sin(x)
    // return sin(x);
}

// Function to compute the local sum for the trapezoid rule
// Note: This computes sum(f(x_i)) for i = local_first_i to local_last_i - 1
double compute_local_sum(double local_a, double h, int local_n)
{
    double sum = 0.0;
    double x;
    int i;

    for (i = 0; i < local_n; i++)
    {
        x = local_a + i * h; // Calculate x for the current local point
        sum += f(x);
    }
    return sum;
}

int main(int argc, char *argv[])
{
    int my_rank, num_procs;
    long long n;             // Total number of trapezoids (using long long)
    double a = 0.0, b = 1.0; // Integration limits [a, b]
    double h;                // Trapezoid width
    double local_a;          // Starting point for this process
    int local_n;             // Number of trapezoids for this process
    double local_sum;        // Sum of f(x_i) calculated by this process
    double global_sum;       // Total sum obtained after reduction
    double integral;         // Final integral estimate
    int i;
    double start_time, end_time, elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // --- Argument Handling (Rank 0 reads and broadcasts n) ---
    if (my_rank == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "Usage: mpirun ... %s <num_trapezoids>\n", argv[0]);
            n = -1; // Signal error
        }
        else
        {
            n = atoll(argv[1]); // Use atoll for long long
            if (n <= 0)
            {
                fprintf(stderr, "Error: Number of trapezoids must be positive.\n");
                n = -1; // Signal error
            }
        }
        // Broadcast a and b as well, in case they were changed
        // We could broadcast n, a, b in one go using a struct or array if needed.
    }

    // Broadcast n, a, b from rank 0 to all processes
    MPI_Bcast(&n, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Check if input was valid after broadcast
    if (n <= 0)
    {
        MPI_Finalize();
        return 1;
    }

    // --- Calculation Setup ---
    h = (b - a) / (double)n; // Width of each trapezoid

    // Calculate the number of trapezoids and starting point for this process
    // This method distributes the intervals as evenly as possible
    int base_local_n = n / num_procs;
    int remainder = n % num_procs;
    if (my_rank < remainder)
    {
        local_n = base_local_n + 1;
        local_a = a + my_rank * local_n * h; // Note: This formula for local_a might be slightly off due to varying local_n
    }
    else
    {
        local_n = base_local_n;
        local_a = a + (remainder * (base_local_n + 1) + (my_rank - remainder) * base_local_n) * h; // More precise start
    }

    // A simpler, common way to calculate local range (might have slight load imbalance if n % num_procs != 0):
    // local_n = n / num_procs;
    // if (my_rank == num_procs - 1) {
    //     // Last process might take a few more trapezoids
    //     local_n = n - my_rank * (n / num_procs);
    // }
    // local_a = a + my_rank * (n / num_procs) * h;

    // --- Timing and Calculation ---
    MPI_Barrier(MPI_COMM_WORLD); // Synchronize before timing
    start_time = MPI_Wtime();

    // Each process computes its part of the sum
    // IMPORTANT: The formula h * [ f(x_0)/2 + f(x_1) + ... + f(x_{n-1}) + f(x_n)/2 ]
    // We are summing f(x_i) where x_i is the *left* endpoint of trapezoid i
    // The first point x_0 = a, last point x_n = b
    // Each process sums f(x_i) for its range of i, starting from its local_a
    local_sum = 0.0;
    for (i = 0; i < local_n; i++)
    {
        local_sum += f(local_a + i * h);
    }

    // Reduce all local sums into global_sum on rank 0
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    elapsed_time = end_time - start_time;

    // --- Final Calculation and Output (Rank 0 only) ---
    if (my_rank == 0)
    {
        // Complete the trapezoid rule formula
        integral = h * (global_sum + (f(a) - f(b)) / 2.0); // Adjust sum for endpoints

        // Note: The exact analytic integral of x^2 from 0 to 1 is 1/3
        printf("Number of Processes:  %d\n", num_procs);
        printf("Integration Limits:   [%.4f, %.4f]\n", a, b);
        printf("Number of Trapezoids: %lld\n", n);
        printf("Trapezoid Width (h):  %.10f\n", h);
        printf("Calculated Integral:  %.10f\n", integral);
        printf("Elapsed Time:         %.6f seconds\n", elapsed_time);

        // If integrating x^2 from 0 to 1:
        if (a == 0.0 && b == 1.0)
        {
            printf("Analytic Integral:    %.10f\n", 1.0 / 3.0);
            printf("Error:                %.10e\n", fabs(integral - 1.0 / 3.0));
        }
    }

    MPI_Finalize();
    return 0;
}