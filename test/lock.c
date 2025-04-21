#include <stdio.h>
#include <pthread.h>

int contador = 0;
pthread_mutex_t mutex_contador = PTHREAD_MUTEX_INITIALIZER;

void *incrementar_contador(void *arg)
{
    for (int i = 0; i < 100000; i++)
    {
        pthread_mutex_lock(&mutex_contador); // Bloquear el mutex
        contador++;
        pthread_mutex_unlock(&mutex_contador); // Desbloquear el mutex
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t hilos[2];

    pthread_create(&hilos[0], NULL, incrementar_contador, NULL);
    pthread_create(&hilos[1], NULL, incrementar_contador, NULL);

    pthread_join(hilos[0], NULL);
    pthread_join(hilos[1], NULL);

    printf("Contador final: %d\n", contador);

    return 0;
}

// gcc seccion_critica.c -o seccion_critica -lpthread
