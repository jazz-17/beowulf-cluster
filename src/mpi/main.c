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
    // char hostname[256]; // Hostname isn't used in the final print, can be removed if not needed
    // gethostname(hostname, sizeof(hostname));

    char greeting[MAX_MESSAGE_SIZE];
    MPI_Status status; // Status object is useful, especially with MPI_ANY_SOURCE

    if (my_rank != 0)
    {
        // Non-zero ranks send their greeting to rank 0
        sprintf(greeting, "Greetings from process %d of %d!", my_rank, world_size);
        MPI_Send(greeting, strlen(greeting) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    else // Rank 0's logic
    {
        // Print rank 0's own greeting directly
        printf("Process 0 (myself) says: Greetings from process %d of %d!\n", my_rank, world_size);

        // Receive greetings from all *other* processes (ranks 1 to world_size - 1)
        // We expect world_size - 1 messages in total.
        printf("Receiving greetings from other processes:\n");
        for (int i = 1; i < world_size; i++) // Loop from 1 to world_size-1
        {
            // Option 1: Receive from specific ranks (if order matters or is known)
            // MPI_Recv(greeting, MAX_MESSAGE_SIZE, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);

            // Option 2: Receive from any source (more flexible, generally preferred)
            MPI_Recv(greeting, MAX_MESSAGE_SIZE, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

            // You can get the actual sender's rank from the status object if needed:
            // printf("Received from rank %d: %s\n", status.MPI_SOURCE, greeting);
            printf("Received: %s\n", greeting); // Print the received message
        }
    }

    // Finalize the MPI environment. No MPI calls should be made after this.
    MPI_Finalize();

    return 0;
}