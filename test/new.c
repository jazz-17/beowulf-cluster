// Problem 2: Simple Wait/Signal using Condition Variable

// Concept Tested: Thread creation, joining, mutexes, condition variables (pthread_cond_wait, pthread_cond_signal).

// Problem Statement:

// Write a C program using Pthreads with two threads: a "Worker" thread and a "Waiter" thread.

//     The Worker thread should simulate doing some work (e.g., using sleep(2)).

//     After finishing its work, the Worker thread needs to signal the Waiter thread.

//     The Waiter thread should start and immediately wait for the signal from the Worker thread.

//     Once the Waiter thread receives the signal, it should print a message like "Waiter received signal and is proceeding." and then exit.

//     Use a shared boolean flag (or integer) protected by a mutex, and a condition variable to manage the waiting and signaling. The Waiter thread should check the flag after waking up to handle potential spurious wakeups.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For sleep()

// Shared resources
pthread_mutex_t lock;
pthread_cond_t condition;
int work_done = 0; // Shared flag (0 = not done, 1 = done)

// Worker thread function
void *worker_thread_func(void *arg)
{
    printf("Worker: Starting work...\n");
    sleep(2); // Simulate doing work
    printf("Worker: Work finished.\n");

    // --- Signal Waiter ---
    // Lock the mutex to safely modify the shared flag and signal
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("Worker: Mutex lock failed");
        pthread_exit(NULL);
    }

    work_done = 1; // Set the flag indicating work is done
    printf("Worker: Signaling condition.\n");

    // Signal the condition variable - wakes up one waiting thread (if any)
    if (pthread_cond_signal(&condition) != 0)
    {
        perror("Worker: Condition signal failed");
        // Unlock mutex even if signal fails
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
    }

    // Unlock the mutex
    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("Worker: Mutex unlock failed");
    }
    // --- End Signal ---

    pthread_exit(NULL);
}

// Waiter thread function
void *waiter_thread_func(void *arg)
{
    printf("Waiter: Waiting for signal...\n");

    // --- Wait for Signal ---
    // Lock the mutex before checking the condition and waiting
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("Waiter: Mutex lock failed");
        pthread_exit(NULL);
    }

    // IMPORTANT: Use a while loop to check the condition
    // This handles spurious wakeups and ensures the condition is truly met
    while (work_done == 0)
    {
        // pthread_cond_wait atomically unlocks the mutex and waits.
        // When woken up (by signal or spuriously), it re-acquires the mutex.
        printf("Waiter: Waiting on condition variable.\n");
        if (pthread_cond_wait(&condition, &lock) != 0)
        {
            perror("Waiter: Condition wait failed");
            // Unlock mutex before exiting on error
            pthread_mutex_unlock(&lock);
            pthread_exit(NULL);
        }
        // When woken, the loop condition (work_done == 0) is re-checked
        printf("Waiter: Woke up, checking condition again (work_done = %d).\n", work_done);
    }
    // Now we know work_done is 1 and we hold the lock

    printf("Waiter: Received signal and condition is met (work_done = %d). Proceeding.\n", work_done);

    // Unlock the mutex
    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("Waiter: Mutex unlock failed");
    }
    // --- End Wait ---

    pthread_exit(NULL);
}

int main()
{
    pthread_t worker_thread, waiter_thread;

    // 1. Initialize mutex and condition variable
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("Mutex initialization failed");
        return 1;
    }
    if (pthread_cond_init(&condition, NULL) != 0)
    {
        perror("Condition variable initialization failed");
        pthread_mutex_destroy(&lock); // Clean up mutex
        return 1;
    }

    // 2. Create threads (order doesn't strictly matter, but often waiter is created first)
    printf("Main: Creating Waiter thread.\n");
    if (pthread_create(&waiter_thread, NULL, waiter_thread_func, NULL) != 0)
    {
        perror("Waiter thread creation failed");
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&condition);
        return 1;
    }

    // Give waiter a chance to start waiting (optional, but helps visualize)
    // sleep(1);

    printf("Main: Creating Worker thread.\n");
    if (pthread_create(&worker_thread, NULL, worker_thread_func, NULL) != 0)
    {
        perror("Worker thread creation failed");
        // Consider how to handle cleanup if waiter is running
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&condition);
        return 1;
    }

    // 3. Join threads
    printf("Main: Joining Worker thread.\n");
    if (pthread_join(worker_thread, NULL) != 0)
    {
        perror("Worker thread join failed");
    }
    printf("Main: Joining Waiter thread.\n");
    if (pthread_join(waiter_thread, NULL) != 0)
    {
        perror("Waiter thread join failed");
    }

    // 4. Destroy mutex and condition variable
    printf("Main: Destroying mutex and condition variable.\n");
    if (pthread_mutex_destroy(&lock) != 0)
    {
        perror("Mutex destruction failed");
    }
    if (pthread_cond_destroy(&condition) != 0)
    {
        perror("Condition variable destruction failed");
    }

    printf("Main: Program finished.\n");
    return 0;
}

// How to Compile:
// gcc your_program_name.c -o your_program_name -pthread

// Explanation & Key Points:

//     Condition Variable: pthread_cond_t condition is used for signaling between threads based on a condition (in this case, work_done == 1).

//     Mutex Requirement: Condition variables always require an associated mutex. The mutex protects the shared state (the work_done flag) that the condition depends on.

//     pthread_cond_wait(&condition, &lock): This is the core of waiting. It atomically does three things:

//         Unlocks the lock.

//         Puts the calling thread to sleep, waiting for the condition.

//         When woken up (by pthread_cond_signal or pthread_cond_broadcast), it re-acquires the lock before returning.

//     pthread_cond_signal(&condition): Wakes up at least one thread currently waiting on the condition. The woken thread will then attempt to re-acquire the mutex associated with the wait. The signal should be done while holding the mutex that protects the shared state being changed.

//     The while Loop: pthread_cond_wait can experience spurious wakeups (waking up even if not signaled). The while (work_done == 0) loop ensures that the waiter thread re-checks the actual condition (work_done) after waking up. If the condition isn't met (e.g., spurious wakeup), it goes back to waiting. This is crucial for correctness.

//     Shared Flag: work_done is the shared state that coordinates the threads. It must only be modified while holding the lock.

// These two problems cover essential Pthread synchronization primitives and patterns often encountered in tests and real-world parallel programming. Good luck!