#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // For srand

// Define matrix dimensions (make N divisible by number of processes for this simple example)
// If N is not divisible, the root handles the remainder, making the logic slightly more complex.
// Let's assume N is large enough and potentially divisible for simplicity.
#define N 8 // Dimension of the square matrix and vector

// Function to print a matrix (optional, for debugging)
void print_matrix(double mat[N][N], int rows, int cols)
{
    printf("Matrix:\n");
    for (int i = 0; i < rows; i++)
    {
        printf("  [");
        for (int j = 0; j < cols; j++)
        {
            printf("%6.2f%s", mat[i][j], (j == cols - 1) ? "" : ", ");
        }
        printf("]\n");
    }
}

// Function to print a vector (optional, for debugging)
void print_vector(double vec[], int size)
{
    printf("Vector: [");
    for (int i = 0; i < size; i++)
    {
        printf("%6.2f%s", vec[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main(int argc, char *argv[])
{
    int rank, size;
    double matrix_A[N][N];
    double vector_x[N];
    double result_b[N];          // Final result vector (only rank 0 needs the full one)
    double local_rows[N / 2][N]; // Buffer for rows received by workers (adjust size if needed)
    double local_result[N / 2];  // Buffer for partial results calculated by workers

    MPI_Status status;

    MPI_Init(&argc, &argv);               // Initialize MPI environment
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank (ID) of the current process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes

    // Basic check: Ensure N is somewhat reasonable compared to size
    if (N < size)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Error: Matrix dimension N (%d) should ideally be >= number of processes (%d) for good distribution.\n", N, size);
        }
        MPI_Finalize();
        return 1;
    }

    // --- Root Process (Rank 0): Initialize data and distribute ---
    if (rank == 0)
    {
        printf("MPI Matrix-Vector Multiplication (N=%d, Processes=%d)\n", N, size);
        srand(time(NULL)); // Seed random number generator

        // Initialize matrix A and vector x with some values (e.g., random or sequential)
        printf("Initializing matrix A and vector x...\n");
        for (int i = 0; i < N; i++)
        {
            vector_x[i] = (double)(i + 1); // Example: 1, 2, 3, ...
            for (int j = 0; j < N; j++)
            {
                // matrix_A[i][j] = (double)(rand() % 10); // Random 0-9
                matrix_A[i][j] = (double)(i * N + j + 1); // Example: 1, 2, .. N*N
            }
        }

        // Optional: Print initial matrix and vector
        // print_matrix(matrix_A, N, N);
        // print_vector(vector_x, N);
        printf("Initialization complete.\n");

        // --- Distribute vector x to all processes ---
        printf("Broadcasting vector x...\n");
        MPI_Bcast(vector_x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // --- Distribute rows of matrix A to worker processes ---
        printf("Distributing matrix A rows...\n");
        int rows_per_proc = N / size;
        int remainder = N % size; // Rows left over if not perfectly divisible
        int rows_sent = 0;
        int current_row_index = 0;

        // Calculate rows for the root process itself
        int root_rows = rows_per_proc + remainder; // Root handles the remainder
        rows_sent = root_rows;                     // Keep track of rows assigned
        current_row_index += root_rows;

        // Send rows to each worker process
        for (int dest_rank = 1; dest_rank < size; dest_rank++)
        {
            int rows_to_send = rows_per_proc;
            if (rows_to_send > 0)
            { // Only send if there are rows to send
                MPI_Send(
                    &matrix_A[current_row_index][0], // Pointer to the start of the rows
                    rows_to_send * N,                // Number of elements to send (rows * columns)
                    MPI_DOUBLE,                      // Data type
                    dest_rank,                       // Destination rank
                    0,                               // Message tag
                    MPI_COMM_WORLD                   // Communicator
                );
                rows_sent += rows_to_send;
                current_row_index += rows_to_send;
            }
            else
            {
                // Handle case where N < size, some processes might get 0 rows initially.
                // We could send a dummy message or handle it in the receiving part.
                // For simplicity here, we assume N >= size and rows_per_proc >= 1 usually.
            }
        }
        printf("Distribution complete. Root keeps %d rows. Workers get %d rows each.\n", root_rows, rows_per_proc);

        // --- Root Process: Calculate its portion of the result ---
        printf("Root calculating its %d rows...\n", root_rows);
        for (int i = 0; i < root_rows; i++)
        {
            result_b[i] = 0.0;
            for (int j = 0; j < N; j++)
            {
                result_b[i] += matrix_A[i][j] * vector_x[j];
            }
        }

        // --- Root Process: Gather results from worker processes ---
        printf("Root gathering results from workers...\n");
        int received_row_index = root_rows; // Start index for placing received results
        for (int source_rank = 1; source_rank < size; source_rank++)
        {
            int rows_to_receive = rows_per_proc; // Match rows sent
            if (rows_to_receive > 0)
            {
                MPI_Recv(
                    &result_b[received_row_index], // Pointer to where to store received results
                    rows_to_receive,               // Number of elements expected
                    MPI_DOUBLE,                    // Data type
                    source_rank,                   // Source rank
                    1,                             // Message tag (use a different tag for results)
                    MPI_COMM_WORLD,                // Communicator
                    &status                        // MPI Status object
                );
                received_row_index += rows_to_receive;
            }
        }

        // --- Root Process: Print the final result vector ---
        printf("\n--- Final Result Vector b ---\n");
        print_vector(result_b, N);
        printf("-----------------------------\n");
    }
    // --- Worker Processes (Rank > 0): Receive data, compute, send back ---
    else
    {
        // --- Receive broadcast vector x ---
        MPI_Bcast(vector_x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // --- Receive rows of matrix A from root ---
        int rows_per_proc = N / size; // Worker calculates how many rows it expects
        if (rows_per_proc > 0)
        {
            MPI_Recv(
                local_rows,        // Buffer to store received rows
                rows_per_proc * N, // Number of elements expected
                MPI_DOUBLE,        // Data type
                0,                 // Source rank (root)
                0,                 // Message tag (matches send tag)
                MPI_COMM_WORLD,    // Communicator
                &status            // MPI Status object
            );

            // --- Worker Process: Calculate its portion of the result ---
            for (int i = 0; i < rows_per_proc; i++)
            {
                local_result[i] = 0.0;
                for (int j = 0; j < N; j++)
                {
                    local_result[i] += local_rows[i][j] * vector_x[j];
                }
            }

            // --- Worker Process: Send results back to root ---
            MPI_Send(
                local_result,  // Pointer to the calculated partial result
                rows_per_proc, // Number of elements to send
                MPI_DOUBLE,    // Data type
                0,             // Destination rank (root)
                1,             // Message tag (use a different tag for results)
                MPI_COMM_WORLD // Communicator
            );
        }
        else
        {
            // If N < size or N not divisible, some processes might not receive/calculate/send.
            // This simple version assumes N >= size and rows_per_proc >= 1.
        }
    }

    MPI_Finalize(); // Finalize MPI environment
    return 0;
}