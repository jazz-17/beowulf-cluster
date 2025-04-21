#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Función para calcular el producto punto secuencial (para verificación)
double sequential_dot_product(double *a, double *b, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; i++)
    {
        sum += a[i] * b[i];
    }
    return sum;
}

int main(int argc, char **argv)
{
    int rank, size;
    int n = 8; // Tamaño de los vectores
    int sub_n; // Tamaño de los subvectores

    double *a = NULL, *b = NULL;     // Vectores completos (solo en rank 0)
    double global_dot_product = 0.0; // Producto punto final (resultado en rank 0)

    double *sub_a, *sub_b;          // Subvectores locales (en todos los procesos)
    double local_dot_product = 0.0; // Producto punto parcial de cada proceso

    // --- Inicialización de MPI ---
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener el rank del proceso actual
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtener el número total de procesos

    // --- Verificación y Cálculo del Tamaño del Subvector ---
    if (n % size != 0)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Error: El tamaño del vector (N=%d) no es divisible por el número de procesos (size=%d).\n", n, size);
            fprintf(stderr, "Este ejemplo simple requiere divisibilidad. Modifique N o el número de procesos.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Termina todos los procesos MPI
    }

    sub_n = n / size; // Calcular cuántos elementos tendrá cada proceso

    // --- Asignación de Memoria y Creación de Datos (Solo en Rank 0) ---
    if (rank == 0)
    {
        a = (double *)malloc(n * sizeof(double));
        b = (double *)malloc(n * sizeof(double));
        if (a == NULL || b == NULL)
        {
            fprintf(stderr, "Error al asignar memoria para los vectores completos en rank 0.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Inicializar vectores con valores simples
        for (int i = 0; i < n; i++)
        {
            a[i] = (double)(i + 1);
            b[i] = (double)(n - i);
        }

        printf("Calculando producto punto para N=%d elementos con %d procesos.\n", n, size);
        printf("a = ");
        for (int k = 0; k < n; k++)
        {
            printf("%.2f ", a[k]); // prints with 2 decimal places
        }
        printf("\nb = ");
        for (int k = 0; k < n; k++)
        {
            printf("%.2f ", b[k]); // prints with 2 decimal places
        }
    }

    // --- Asignación de Memoria para Subvectores (Todos los Procesos) ---
    sub_a = (double *)malloc(sub_n * sizeof(double));
    sub_b = (double *)malloc(sub_n * sizeof(double));
    if (sub_a == NULL || sub_b == NULL)
    {
        fprintf(stderr, "Error al asignar memoria para los subvectores en rank %d.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // --- Distribución de los Vectores (Scatter) ---
    // El proceso 0 (root) envía partes de 'a' a los 'sub_a' de todos los procesos.
    MPI_Scatter(
        a,             // Puntero al buffer de envío (solo relevante en root=0)
        sub_n,         // Número de elementos a ENVIAR a CADA proceso
        MPI_DOUBLE,    // Tipo de dato de los elementos enviados
        sub_a,         // Puntero al buffer de recepción (donde cada proceso guarda su parte)
        sub_n,         // Número de elementos a RECIBIR por cada proceso
        MPI_DOUBLE,    // Tipo de dato de los elementos recibidos
        0,             // Rank del proceso que distribuye (el root)
        MPI_COMM_WORLD // Comunicador
    );

    // El proceso 0 (root) envía partes de 'b' a los 'sub_b' de todos los procesos.
    MPI_Scatter(
        b, sub_n, MPI_DOUBLE,     // Envío desde b
        sub_b, sub_n, MPI_DOUBLE, // Recepción en sub_b
        0, MPI_COMM_WORLD);

    // --- Cálculo Local del Producto Punto ---
    // Cada proceso calcula el producto punto de los subvectores que recibió.
    for (int i = 0; i < sub_n; i++)
    {
        local_dot_product += sub_a[i] * sub_b[i];
    }

    // --- Reducción (Suma) de los Resultados Parciales ---
    // Se suman todos los 'local_dot_product' de cada proceso, y el resultado
    // final ('global_dot_product') se almacena solo en el proceso root (rank 0).
    MPI_Reduce(
        &local_dot_product,  // Puntero al dato a ENVIAR desde cada proceso
        &global_dot_product, // Puntero al buffer donde el root RECIBE el resultado final
        1,                   // Número de elementos a reducir (solo 1 double)
        MPI_DOUBLE,          // Tipo de dato de los elementos
        MPI_SUM,             // Operación de reducción a aplicar (suma)
        0,                   // Rank del proceso que recibirá el resultado (el root)
        MPI_COMM_WORLD       // Comunicador
    );

    // --- Impresión del Resultado (Solo en Rank 0) ---
    if (rank == 0)
    {
        printf("--------------------------------------------\n");
        printf("Producto Punto Global (MPI): %f\n", global_dot_product);
        printf("--------------------------------------------\n");
    }

    // --- Limpieza de Memoria ---
    free(sub_a); // Todos los procesos liberan sus subvectores
    free(sub_b);
    if (rank == 0)
    {
        free(a); // Solo el proceso 0 libera los vectores completos
        free(b);
    }

    // --- Finalización de MPI ---
    MPI_Finalize();

    return 0;
}