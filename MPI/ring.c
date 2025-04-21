#include <mpi.h>
#include <stdio.h>
#include <stdlib.h> // For exit()

int main(int argc, char *argv[])
{
    int my_rank, num_procs;
    int token; // The data being passed around
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // This example requires at least 2 processes
    if (num_procs < 2)
    {
        if (my_rank == 0)
        {
            fprintf(stderr, "Error: This program requires at least 2 MPI processes.\n");
        }
        MPI_Finalize();
        exit(1);
    }

    // Determine the rank of the next process in the ring
    int next_rank = (my_rank + 1) % num_procs;
    // Determine the rank of the previous process in the ring
    int prev_rank = (my_rank - 1 + num_procs) % num_procs; // Modulo handles wrap-around for rank 0

    // Process 0 starts the token
    if (my_rank == 0)
    {
        token = 100; // Starting value
        printf("Process %d starting with token %d, sending to process %d\n", my_rank, token, next_rank);
        MPI_Send(&token, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD); // Send to next

        // Now, wait to receive the final token back from the last process
        MPI_Recv(&token, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &status);
        printf("Process %d received final token %d from process %d\n", my_rank, token, prev_rank);
        printf("--------------------------------------------------\n");
        printf("Final token value after circulating the ring: %d\n", token);
        printf("Expected final value (if P processes): 100 + 0 + 1 + ... + (P-1)\n");
        printf("--------------------------------------------------\n");
    }
    else
    {
        // All other processes: receive, modify, send
        MPI_Recv(&token, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &status); // Receive from previous
        printf("Process %d received token %d from process %d\n", my_rank, token, prev_rank);

        token += my_rank; // Modify the token by adding own rank

        printf("Process %d sending token %d to process %d\n", my_rank, token, next_rank);
        MPI_Send(&token, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD); // Send to next
    }

    MPI_Finalize();
    return 0;
}