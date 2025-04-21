#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
int main(int argc, char **argv)
{
    int rank, size;
    int *vector, *sub_vector;
    int local_sum = 0, global_sum = 0;
    int n = 10; // Tamaño del vector
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Asignar memoria para el vector completo (solo en el proceso 0)
    if (rank == 0)
    {
        vector = (int *)malloc(n * sizeof(int));
        for (int i = 0; i < n; i++)
        {
            vector[i] = i + 1; // Inicializar con valores de ejemplo
        }
    }
    // Distribuir el vector entre los procesos
    int sub_n = n / size;
    sub_vector = (int *)malloc(sub_n * sizeof(int));
    MPI_Scatter(vector, sub_n, MPI_INT, sub_vector, sub_n, MPI_INT, 0,
                MPI_COMM_WORLD);
    // Calcular la suma local de cada subvector
    for (int i = 0; i < sub_n; i++)
    {
        local_sum += sub_vector[i];
    }
    // Sumar todas las sumas locales para obtener la suma global
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0,
               MPI_COMM_WORLD);
    // Imprimir el resultado en el proceso 0
    if (rank == 0)
    {
        printf("La suma global es: %d\n", global_sum);
    }
    // Liberar memoria
    free(sub_vector);
    if (rank == 0)
    {
        free(vector);
    }
    MPI_Finalize();
    return 0;
}