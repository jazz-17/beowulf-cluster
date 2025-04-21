```c
// As an example, suppose that we need to compute n values and add them together.
We know that this can be done with the following serial code:
sum = 0;
slice = n / p;

// Now suppose we  have p cores and p ≤ n. Then each core can form a partial sum of approximately n/p values:

      
// start parallel
my_sum = 0;
slice = n / p;
remainder = n % p;

// Calculate indices for core 'i' (assuming i ranges from 0 to p-1)
// Number of elements assigned to cores before core 'i'
my_first_i = i * slice + min(i, remainder);

// Number of elements assigned to core 'i'
int my_elements = slice + (i < remainder ? 1 : 0);

// Exclusive upper bound
my_last_i = my_first_i + my_elements;


// The loop remains the same:
for ( my_i = my_first_i; my_i < my_last_i; my_i++) {
    my_x = Compute_next_value ( ... ) ;
    my_sum += my_x ;
}
//end parallel

    

// When the cores are done computing their values of my_sum, they can form a global sum by sending their results to a designated “master” core, which can add their results:
if ( I’m the master core ) {
    sum = my_sum ;
for each core other than myself {
    receive value from core ;
    sum += value ;
}
} else {
    send my_sum tothemaster ;
}
```