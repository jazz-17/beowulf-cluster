```c
// --- Pre-computation (can be done serially or by master) ---
W_total = n * (n + 1) / 2;
W_target_per_core = W_total / p; // Target work units per core

current_first_i = 0;
cumulative_work = 0;

// Array or structure to store ranges for each core
core_ranges[p]; // Each element stores {first_i, last_i}

// --- Determine ranges for each core ---
for (core_j = 0; core_j < p - 1; core_j++) {
    core_ranges[core_j].first_i = current_first_i;

    target_cumulative_work = round((core_j + 1) * W_target_per_core); // Where this core should ideally end its work
    // Using round helps distribute remainder more evenly.
    // Alternatively: target_cumulative_work = ((core_j + 1) * W_total) / p using integer math

    // Find the last index for this core
    work_for_this_core = 0;
    search_i = current_first_i;
    while (cumulative_work < target_cumulative_work && search_i < n) {
         work_of_iteration = search_i + 1; // Work units for index search_i
         cumulative_work += work_of_iteration;
         search_i++;
         // Optional: Could also track work_for_this_core here if needed,
         // but cumulative_work is sufficient to find the boundary.
    }

    // search_i is now the first index for the *next* core
    core_ranges[core_j].last_i = search_i;
    current_first_i = search_i; // Update for the next iteration of the outer loop
}

// --- Assign range for the last core ---
// It gets whatever is left to ensure all work is covered
core_ranges[p-1].first_i = current_first_i;
core_ranges[p-1].last_i = n;

// --- Parallel Execution (Pseudocode similar to before) ---

// start parallel (each core 'i' knows its index, now 0 to p-1)
my_sum = 0;

// Each core retrieves its pre-calculated range
my_first_i = core_ranges[i].first_i;
my_last_i = core_ranges[i].last_i;

for ( my_i = my_first_i; my_i < my_last_i; my_i++) {
    // The actual computation still uses Compute_next_value
    // The time it takes varies, but we've balanced the *number*
    // of varying-time computations assigned to each core.
    my_x = Compute_next_value ( ... ) ; // Pass 'my_i' if needed by the function
    my_sum += my_x ;
}
//end parallel

// --- Reduction (Same as before) ---
if ( I’m the master core ) {
    sum = my_sum ;
    for each core other than myself {
        receive value from core ;
        sum += value ;
    }
} else {
    send my_sum to the master ;
}

```