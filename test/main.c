#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define NUM_THREADS 3
// Estructura para pasar argumentos a los hilos
typedef struct
{
    int thread_id;
    int rows;
    int cols;
    double *matrix;
    double *vector;
    double *result;
} ThreadData;

// Función para multiplicar una porción de la matriz por el vector
void *multiply_matrix_vector_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int start_row = data->thread_id * (data->rows / NUM_THREADS);
    int end_row = (data->thread_id + 1) * (data->rows / NUM_THREADS);

    if (data->thread_id == NUM_THREADS - 1)
    {
        end_row = data->rows; // Asegurar que el último hilo procese las filas restantes
    }

    for (int i = start_row; i < end_row; i++)
    {
        data->result[i] = 0;
        for (int j = 0; j < data->cols; j++)
        {
            data->result[i] += data->matrix[i * data->cols + j] * data->vector[j];
        }
    }
    pthread_exit(NULL);
}

// Función para multiplicar la matriz por el vector de forma secuencial
void multiply_matrix_vector_sequential(double *matrix, double *vector, double *result, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        result[i] = 0;
        for (int j = 0; j < cols; j++)
        {
            result[i] += matrix[i * cols + j] * vector[j];
        }
    }
}

int main()
{
    int rows = 1000;
    int cols = 1000;

    // Asignación de memoria para la matriz, el vector y el resultado
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    double *vector = (double *)malloc(cols * sizeof(double));
    double *result_sequential = (double *)malloc(rows * sizeof(double));
    double *result_parallel = (double *)malloc(rows * sizeof(double));

    // Inicialización de la matriz y el vector con valores aleatorios
    for (int i = 0; i < rows * cols; i++)
    {
        matrix[i] = (double)rand() / RAND_MAX;
    }

    for (int i = 0; i < cols; i++)
    {
        vector[i] = (double)rand() / RAND_MAX;
    }

    // Multiplicación secuencial
    clock_t start_sequential = clock();
    multiply_matrix_vector_sequential(matrix, vector, result_sequential, rows, cols);
    clock_t end_sequential = clock();
    double time_sequential = (double)(end_sequential - start_sequential) / CLOCKS_PER_SEC;

    // Multiplicación paralela con Pthreads
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    clock_t start_parallel = clock();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].rows = rows;
        thread_data[i].cols = cols;
        thread_data[i].matrix = matrix;
        thread_data[i].vector = vector;
        thread_data[i].result = result_parallel;
        pthread_create(&threads[i], NULL, multiply_matrix_vector_thread, (void *)&thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    clock_t end_parallel = clock();
    double time_parallel = (double)(end_parallel - start_parallel) / CLOCKS_PER_SEC;

    // Impresión de los tiempos de ejecución
    printf("Tiempo de ejecución secuencial: %f segundos\n", time_sequential);
    printf("Tiempo de ejecución paralelo: %f segundos\n", time_parallel);

    // Liberación de memoria
    free(matrix);
    free(vector);
    free(result_sequential);
    free(result_parallel);
    return 0;
}
