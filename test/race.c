// 1 OBJETIVO:
// ● Comprender el concepto de sección crítica y condición de carrera.
// ● Aprender a utilizar mutexes para proteger secciones críticas en C con Posix Threads.
// ● Implementar un ejemplo práctico que demuestre el uso de mutexes.
// 2 CONCEPTOS CLAVE:
// 2.1 SECCIÓN CRÍTICA:
// ● Una sección crítica es un segmento de código al que varios hilos acceden a recursos
// compartidos (como variables o archivos).
// ● Si no se controla el acceso a la sección crítica, pueden ocurrir condiciones de carrera, lo que
// lleva a resultados inesperados.
// 2.2 CONDICIÓN DE CARRERA:
// ● Una condición de carrera ocurre cuando el resultado de un programa depende del orden en
// que se ejecutan los hilos.
// ● Esto puede llevar a errores difíciles de depurar, ya que el comportamiento del programa
// puede ser impredecible.
// 2.3 MUTEX (EXCLUSIÓN MUTUA):
// ● Un mutex es un mecanismo de sincronización que permite que solo un hilo acceda a una
// sección crítica a la vez.
// ● Cuando un hilo quiere entrar en una sección crítica, intenta adquirir el mutex. Si el mutex ya
// está bloqueado por otro hilo, el hilo espera hasta que el mutex esté disponible.
// ● Una vez que el hilo termina de acceder a la sección crítica, libera el mutex para que otros
// hilos puedan acceder.
// 2.4 EJEMPLO DE CONDICIÓN DE CARRERA:
// El siguiente código muestra un ejemplo de condición de carrera en C:

#include <stdio.h>
#include <pthread.h>

int contador = 0;

void *incrementar_contador(void *arg)
{
    for (int i = 0; i < 100000; i++)
    {
        contador++;
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