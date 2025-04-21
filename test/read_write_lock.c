// OBJETIVO:
// ● Comprender el concepto de bloqueos de lectura y escritura.
// ● Aprender a utilizar bloqueos de lectura y escritura en C con Posix Threads.
// ● Implementar un ejemplo práctico que demuestre el uso de bloqueos de lectura y escritura.
// 2 CONCEPTOS CLAVE:
// 2.1 BLOQUEOS DE LECTURA Y ESCRITURA (READ-WRITE LOCKS):
// ● Son mecanismos de sincronización que permiten que varios hilos lean un recurso
// compartido simultáneamente, pero solo permiten que un hilo escriba el recurso a la vez.
// ● Son útiles en situaciones donde las operaciones de lectura son mucho más frecuentes que
// las operaciones de escritura.
// ● Mejoran el rendimiento al permitir la concurrencia de lectura, mientras que aún garantizan
// la integridad de los datos durante la escritura.
// 2.2 POSIX PTHREADS:
// ● Es una interfaz de programación de subprocesos (threads) definida por el estándar POSIX.
// ● Proporciona funciones para crear y administrar hilos en sistemas operativos compatibles
// con POSIX.
// 2.3 FUNCIONAMIENTO DE LOS BLOQUEOS DE LECTURA Y ESCRITURA:
// 2.3.1 Bloqueo de lectura:
// ● Un hilo que quiere leer el recurso compartido intenta adquirir un bloqueo de lectura.
// ● Si no hay hilos escribiendo, el hilo adquiere el bloqueo de lectura y puede leer el recurso.
// ● Varios hilos pueden adquirir bloqueos de lectura simultáneamente.
// 2.3.2 Bloqueo de escritura:
// ● Un hilo que quiere escribir el recurso compartido intenta adquirir un bloqueo de escritura.
// ● Si no hay hilos leyendo o escribiendo, el hilo adquiere el bloqueo de escritura y puede
// escribir el recurso.
// ● Solo un hilo puede adquirir un bloqueo de escritura a la vez.
// 2.3.3 Desbloqueo:
// ● Cuando un hilo termina de leer o escribir el recurso compartido, libera el bloqueo

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int recurso_compartido = 0;
pthread_rwlock_t rwlock_recurso;

void *lector(void *arg)
{
    pthread_rwlock_rdlock(&rwlock_recurso); // Bloquear para lectura
    printf("Lector %ld: Leyendo recurso = %d\n", (long)pthread_self(), recurso_compartido);
    pthread_rwlock_unlock(&rwlock_recurso); // Desbloquear
    pthread_exit(NULL);
}

void *escritor(void *arg)
{
    pthread_rwlock_wrlock(&rwlock_recurso); // Bloquear para escritura
    recurso_compartido++;
    printf("Escritor %ld: Escribiendo recurso = %d\n", (long)pthread_self(), recurso_compartido);
    pthread_rwlock_unlock(&rwlock_recurso); // Desbloquear
    pthread_exit(NULL);
}

int main()
{
    pthread_t hilos[5];
    pthread_rwlock_init(&rwlock_recurso, NULL);

    for (int i = 0; i < 3; i++)
    {
        pthread_create(&hilos[i], NULL, lector, NULL);
    }

    for (int i = 3; i < 5; i++)
    {
        pthread_create(&hilos[i], NULL, escritor, NULL);
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_join(hilos[i], NULL);
    }
    pthread_rwlock_destroy(&rwlock_recurso);
    return 0;
}

// 4 EXPLICACIÓN:
// ● Se utiliza pthread_rwlock_t para declarar la variable de bloqueo de lectura y escritura.
// ● pthread_rwlock_init() inicializa el bloqueo de lectura y escritura.
// ● pthread_rwlock_rdlock() bloquea el recurso para lectura.
// ● pthread_rwlock_wrlock() bloquea el recurso para escritura.
// ● pthread_rwlock_unlock() desbloquea el recurso.
// ● pthread_rwlock_destroy() destruye el bloqueo cuando ya no es necesario.
// ● El código crea 3 hilos lectores y 2 hilos escritores, que acceden al recurso compartido
// protegido por el bloqueo de lectura y escritura.
// 5 PROCEDIMIENTO:
// 5.1 COMPILACIÓN:
// ● Guarda el código en un archivo de C (por ejemplo, rwlocks.c).
// ● Compila el código con el siguiente comando:
// Bash
// gcc rwlocks.c -o rwlocks -lpthread
// 5.2 EJECUCIÓN:
// ● Ejecuta el programa con el siguiente comando:
// Bash
// ./rwlocks
// 5.3 ANÁLISIS:
// Observa la salida del programa y verifica que los hilos lectores y escritores se sincronicen
// correctamente.
// Experimenta con diferentes números de hilos y escenarios para comprender mejor el
// funcionamiento de los bloqueos de lectura y escritura.