#include <stdio.h>
#include <omp.h> // OpenMP header

int main()
{
    int loco(int i = 0; i < n; i++)
    {
        x = Compute_next_value(...);
        sum += x;
    }
    int num_threads = 4; // Desired number of threads

    // Set the number of threads globally (optional)
    omp_set_num_threads(num_threads);

#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();      // Get the thread ID
        int total_threads = omp_get_num_threads(); // Get the total number of threads
        printf("Hello from thread %d of %d\n", thread_id, total_threads);
    } // End of parallel region

    return 0;
}

// gcc main.c -o output -fopenmp