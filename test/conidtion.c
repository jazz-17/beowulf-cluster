// Comprender el concepto de variables de condición y su uso en la sincronización de hilos.
// ● Aprender a utilizar variables de condición en C con Posix Threads.
// ● Implementar un ejemplo práctico que demuestre el uso de variables de condición.
// 2 CONCEPTOS CLAVE:
// 2.1 VARIABLES DE CONDICIÓN:
// ● Las variables de condición son mecanismos de sincronización que permiten a los hilos
// esperar hasta que se cumpla una determinada condición.
// ● Se utilizan en combinación con mutexes para proteger el acceso a variables compartidas y
// evitar condiciones de carrera.
// ● Permiten a los hilos dormir hasta que otro hilo les notifica que la condición que estaban
// esperando se ha cumplido.
// 2.2 MUTEXES:
// ● Como se vio en la guia anterior, los mutexes son mecanismos de exclusión mutua que
// permiten que solo un hilo acceda a una sección crítica a la vez.
// ● En el contexto de las variables de condición, los mutexes se utilizan para proteger la
// variable de condición y la variable compartida que representa la condición.
// 2.3 FUNCIONAMIENTO DE LAS VARIABLES DE CONDICIÓN:
// 2.3.1 Espera:
// ● Un hilo que espera una condición bloquea el mutex y llama a pthread_cond_wait().
// ● pthread_cond_wait() desbloquea el mutex y pone el hilo en estado de espera.
// ● El hilo permanece en estado de espera hasta que otro hilo llama a pthread_cond_signal() o
// pthread_cond_broadcast() en la misma variable de condición.
// 2.3.2 Señalización:
// ● Un hilo que cumple la condición bloquea el mutex, actualiza la variable compartida y llama a
// pthread_cond_signal() o pthread_cond_broadcast().
// ● pthread_cond_signal() despierta a un hilo que está esperando en la variable de condición.
// ● pthread_cond_broadcast() despierta a todos los hilos que están esperando en la variable
// de condición.
// ● El hilo que llama a pthread_cond_signal() o pthread_cond_broadcast() desbloquea el
// mutex.
// 2.3.3 Reanudación:
// ● El hilo que fue despertado por pthread_cond_signal() o pthread_cond_broadcast() vuelve
// a adquirir el mutex y verifica la condición.
// ● Si la condición se cumple, el hilo continúa su ejecución. De lo contrario, vuelve a llamar a
// pthread_cond_wait().

#include <stdio.h>
#include <pthread.h>

int buffer = 0;
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_buffer_lleno = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_buffer_vacio = PTHREAD_COND_INITIALIZER;

void *productor(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        pthread_mutex_lock(&mutex_buffer);
        while (buffer != 0)
        {
            pthread_cond_wait(&cond_buffer_vacio, &mutex_buffer);
        }
        buffer = i + 1;
        printf("Productor: Produjo %d\n", buffer);
        pthread_cond_signal(&cond_buffer_lleno);
        pthread_mutex_unlock(&mutex_buffer);
    }
    pthread_exit(NULL);
}

void *consumidor(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        pthread_mutex_lock(&mutex_buffer);
        while (buffer == 0)
        {
            pthread_cond_wait(&cond_buffer_lleno, &mutex_buffer);
        }
        printf("Consumidor: Consumió %d\n", buffer);
        buffer = 0;
        pthread_cond_signal(&cond_buffer_vacio);
        pthread_mutex_unlock(&mutex_buffer);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t hilo_productor, hilo_consumidor;

    pthread_create(&hilo_productor, NULL, productor, NULL);
    pthread_create(&hilo_consumidor, NULL, consumidor, NULL);

    pthread_join(hilo_productor, NULL);
    pthread_join(hilo_consumidor, NULL);

    return 0;
}

// EXPLICACIÓN:
// ● El hilo productor produce valores y los almacena en el búfer buffer.
// ● El hilo consumidor consume los valores del búfer.
// ● El mutex mutex_buffer protege el acceso al búfer.
// ● La variable de condición cond_buffer_lleno se utiliza para notificar al consumidor cuando el
// búfer tiene un valor.
// ● La variable de condición cond_buffer_vacio se utiliza para notificar al productor cuando el
// búfer está vacío.
// ● El productor espera hasta que el búfer esté vacío antes de producir un nuevo valor.
// ● El consumidor espera hasta que el búfer tenga un valor antes de consumirlo.
// 6 ANÁLISIS:
// ● Observa la salida del programa y verifica que el productor y el consumidor se sincronicen
// correctamente.
// ● Experimenta con diferentes valores y escenarios para comprender mejor el funcionamiento
// de las variables de condición.