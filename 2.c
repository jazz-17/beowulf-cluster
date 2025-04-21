#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

typedef struct
{
    int first_i;
    int last_i;
} CoreRange;

// Mock computation function (replace with your actual computation)
int Compute_next_value(int i)
{
    return i + 1; // Example: work = i + 1
}

int main()
{
    int n = 30; // Total number of iterations (adjust as needed)
    int p = 4;  // Number of cores (threads)
    int i;

    // --- Pre-computation ---
    int W_total = n * (n + 1) / 2;
    double W_target_per_core = (double)W_total / p;

    int current_first_i = 0;
    int cumulative_work = 0;

    CoreRange *core_ranges = malloc(sizeof(CoreRange) * p);
    if (!core_ranges)
    {
        perror("Failed to allocate memory for core_ranges");
        return EXIT_FAILURE;
    }

    // --- Determine ranges for each core ---
    printf("=== Pre-computation: Determining ranges for each core ===\n");
    for (int core_j = 0; core_j < p - 1; core_j++)
    {
        core_ranges[core_j].first_i = current_first_i;

        int target_cumulative_work = round((core_j + 1) * W_target_per_core);
        int search_i = current_first_i;
        while (cumulative_work < target_cumulative_work && search_i < n)
        {
            int work_of_iteration = search_i + 1;
            cumulative_work += work_of_iteration;
            search_i++;
        }

        core_ranges[core_j].last_i = search_i;
        current_first_i = search_i;

        printf("Core %d range: [%d, %d) (Target work: %d, Cumulative work: %d)\n",
               core_j, core_ranges[core_j].first_i, core_ranges[core_j].last_i,
               target_cumulative_work, cumulative_work);
    }

    // Last core gets the remaining range
    core_ranges[p - 1].first_i = current_first_i;
    core_ranges[p - 1].last_i = n;
    printf("Core %d range: [%d, %d) (Final cumulative work: %d)\n",
           p - 1, core_ranges[p - 1].first_i, core_ranges[p - 1].last_i, cumulative_work);

    // --- Parallel Execution ---
    int *partial_sums = malloc(sizeof(int) * p);
    if (!partial_sums)
    {
        perror("Failed to allocate memory for partial_sums");
        free(core_ranges);
        return EXIT_FAILURE;
    }

    printf("\n=== Parallel Execution ===\n");
#pragma omp parallel num_threads(p)
    {
        int thread_id = omp_get_thread_num();
        int my_sum = 0;

        int my_first_i = core_ranges[thread_id].first_i;
        int my_last_i = core_ranges[thread_id].last_i;

        printf("Thread %d starting. Range: [%d, %d)\n", thread_id, my_first_i, my_last_i);

        for (int my_i = my_first_i; my_i < my_last_i; my_i++)
        {
            int my_x = Compute_next_value(my_i);
            my_sum += my_x;
        }

        printf("Thread %d finished with partial sum: %d\n", thread_id, my_sum);
        partial_sums[thread_id] = my_sum;
    }

    // --- Reduction ---
    int final_sum = 0;
    for (i = 0; i < p; i++)
    {
        final_sum += partial_sums[i];
    }

    printf("\n=== Reduction ===\n");
    for (i = 0; i < p; i++)
    {
        printf("Partial sum from thread %d: %d\n", i, partial_sums[i]);
    }
    printf("Final sum: %d\n", final_sum);

    // Cleanup
    free(core_ranges);
    free(partial_sums);

    return 0;
}
