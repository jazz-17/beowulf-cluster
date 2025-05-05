#include <mpi.h>
#include <stdio.h>
#include <unistd.h> // For gethostname
#include <string.h>

#define MAX_MESSAGE_SIZE 100

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the total number of processes in the communicator MPI_COMM_WORLD
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // MPI_COMM_WORLD is the default group of all processes

    // Get the rank (unique ID, 0 to world_size-1) of the current process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Get the hostname of the node where this process is running
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    char greeting[MAX_MESSAGE_SIZE];

    if (my_rank != 0)
    {
        sprintf(greeting, "Greetings from process %d of %d !", my_rank, world_size);
        MPI_Send(greeting, strlen(greeting) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        printf("Greetings from process %d of %d !", my_rank, world_size);
        for (int i = 1; i < world_size; i++)
        {
            MPI_Recv(greeting, MAX_MESSAGE_SIZE, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%s\n", greeting);
        }
    }
    // // Print a message including hostname, rank, and total size
    // printf("Hello from node %s, rank %d out of %d processors\n",
    //        hostname, my_rank, world_size);

    // Finalize the MPI environment. No MPI calls should be made after this.
    MPI_Finalize();

    return 0;
}