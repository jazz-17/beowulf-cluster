#include <mpi.h>    // Main MPI header
#include <stdio.h>  // For printf
#include <unistd.h> // For gethostname

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    // MPI_Init takes pointers to argc and argv to handle any MPI-specific command-line arguments
    MPI_Init(&argc, &argv);

    // Get the total number of processes in the communicator MPI_COMM_WORLD
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // MPI_COMM_WORLD is the default group of all processes

    // Get the rank (unique ID, 0 to world_size-1) of the current process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the hostname of the node where this process is running
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    // Print a message including hostname, rank, and total size
    printf("Hello from node %s, rank %d out of %d processors\n",
           hostname, world_rank, world_size);

    // Finalize the MPI environment. No MPI calls should be made after this.
    MPI_Finalize();

    return 0;
}