// Write a C program using Pthreads where multiple threads perform work in two phases. All threads must complete Phase 1 before any thread can begin Phase 2.

//     Define a constant NUM_THREADS (e.g., 5).

//     Define shared variables:

//         A counter arrived_count initialized to 0.

//         A mutex barrier_mutex.

//         A condition variable barrier_cond.

//     Create NUM_THREADS threads. Each thread will execute a worker function.

//     The worker function should:

//         Print a message indicating it's starting Phase 1 (include thread ID).

//         Simulate Phase 1 work (e.g., sleep(1) or a short random sleep).

//         Print a message indicating it has finished Phase 1 and is waiting at the barrier.

//         Call a barrier_wait() function (you need to implement this).

//         Print a message indicating it has passed the barrier and is starting Phase 2.

//         Simulate Phase 2 work (e.g., another short sleep or just printing).

//         Print a message indicating it has finished Phase 2.

//     Implement the barrier_wait() function:

//         It should take the total number of threads (NUM_THREADS) as an argument or use the global constant.

//         Inside, lock the barrier_mutex.

//         Increment the arrived_count.

//         Check if the current thread is the last thread to arrive (arrived_count == NUM_THREADS).

//             If it is the last thread: Print a message confirming this, signal all other waiting threads using pthread_cond_broadcast, and then proceed (unlock mutex).

//             If it is not the last thread: Wait on the barrier_cond using pthread_cond_wait. Crucially, the wait should be inside a while loop that checks arrived_count < NUM_THREADS. This handles spurious wakeups and ensures the thread only proceeds when all threads have actually arrived.

//         Unlock the barrier_mutex.

//     The main function should:

//         Initialize the mutex and condition variable.

//         Create the threads.

//         Wait for all threads to complete using pthread_join.

//         Destroy the mutex and condition variable.

//         Print a final message indicating all threads have completed.
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For sleep()
#include <time.h>   // For seeding rand()

#define NUM_THREADS 5

// Shared barrier variables
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_cond;
int arrived_count = 0;
const int total_threads = NUM_THREADS; // Use const int or #define

// Simple barrier implementation
void barrier_wait()
{
    // Lock the mutex to access shared count safely
    if (pthread_mutex_lock(&barrier_mutex) != 0)
    {
        perror("Barrier: Mutex lock failed");
        // In a real scenario, might need more robust error handling
        pthread_exit(NULL); // Exit thread on catastrophic failure
    }

    arrived_count++;
    printf("Thread %lu: Arrived at barrier (%d/%d).\n", pthread_self(), arrived_count, total_threads);

    if (arrived_count == total_threads)
    {
        // This is the last thread
        printf("Thread %lu: Last thread arrived. Broadcasting barrier condition!\n", pthread_self());
        // Resetting count is needed for reusable barriers, but not strictly
        // necessary for this single-use case. If reset, do it carefully.
        // For simplicity here, we won't reset, as the barrier is used once.
        // arrived_count = 0; // Optional reset for reuse
        if (pthread_cond_broadcast(&barrier_cond) != 0)
        {
            perror("Barrier: Condition broadcast failed");
            // Still need to unlock
        }
    }
    else
    {
        // Not the last thread, wait for the broadcast
        printf("Thread %lu: Waiting at barrier...\n", pthread_self());
        // Wait while the condition (all arrived) is not met
        while (arrived_count < total_threads)
        {
            // pthread_cond_wait atomically unlocks the mutex and waits.
            // When woken up, it re-acquires the mutex before returning.
            if (pthread_cond_wait(&barrier_cond, &barrier_mutex) != 0)
            {
                perror("Barrier: Condition wait failed");
                // Still need to unlock before exiting
                pthread_mutex_unlock(&barrier_mutex);
                pthread_exit(NULL);
            }
            // Optional: print when woken to see spurious wakeups vs real signal
            // printf("Thread %lu: Woke up, checking barrier condition again (arrived=%d)\n", pthread_self(), arrived_count);
        }
        printf("Thread %lu: Passed barrier check.\n", pthread_self());
    }

    // Unlock the mutex
    if (pthread_mutex_unlock(&barrier_mutex) != 0)
    {
        perror("Barrier: Mutex unlock failed");
        // State might be inconsistent
    }
}

// Worker thread function
void *worker_func(void *arg)
{
    long thread_id = (long)arg;                      // Simple way to pass an ID
    unsigned int seed = time(NULL) ^ pthread_self(); // Seed for rand_r

    // --- Phase 1 ---
    printf("Thread %lu: Starting Phase 1.\n", thread_id);
    // Simulate work with a short random sleep
    // Use rand_r for thread-safety if using random numbers
    // sleep(1 + rand_r(&seed) % 3); // Sleep 1-3 seconds
    sleep(1); // Simpler: sleep 1 second
    printf("Thread %lu: Finished Phase 1. Reaching barrier.\n", thread_id);

    // --- Barrier Synchronization ---
    barrier_wait();

    // --- Phase 2 ---
    printf("Thread %lu: Passed barrier. Starting Phase 2.\n", thread_id);
    // Simulate work
    // sleep(rand_r(&seed) % 2); // Sleep 0-1 seconds
    sleep(1); // Simpler: sleep 1 second
    printf("Thread %lu: Finished Phase 2.\n", thread_id);

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_THREADS];
    long i;

    // 1. Initialize mutex and condition variable
    if (pthread_mutex_init(&barrier_mutex, NULL) != 0)
    {
        perror("Mutex initialization failed");
        return 1;
    }
    if (pthread_cond_init(&barrier_cond, NULL) != 0)
    {
        perror("Condition variable initialization failed");
        pthread_mutex_destroy(&barrier_mutex);
        return 1;
    }

    // 2. Create threads
    printf("Main: Creating %d threads...\n", NUM_THREADS);
    for (i = 0; i < NUM_THREADS; i++)
    {
        // Pass 'i' as the argument, cast to long then void*
        if (pthread_create(&threads[i], NULL, worker_func, (void *)i) != 0)
        {
            perror("Thread creation failed");
            // Clean up initialized resources
            pthread_mutex_destroy(&barrier_mutex);
            pthread_cond_destroy(&barrier_cond);
            // Consider canceling already created threads if needed
            return 1;
        }
    }

    // 3. Join threads (wait for them to complete)
    printf("Main: Waiting for threads to complete...\n");
    for (i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Thread join failed");
            // Continue joining others if possible
        }
    }

    // 4. Destroy mutex and condition variable
    printf("Main: All threads joined. Destroying sync primitives.\n");
    if (pthread_mutex_destroy(&barrier_mutex) != 0)
    {
        perror("Mutex destruction failed");
    }
    if (pthread_cond_destroy(&barrier_cond) != 0)
    {
        perror("Condition variable destruction failed");
    }

    printf("Main: Program finished successfully.\n");
    return 0;
}

// How to Compile:
// gcc your_program_name.c -o your_program_name -pthread

// Explanation & Key Points:

//     Barrier Purpose: To synchronize threads at a specific point, ensuring no thread proceeds past the barrier until all threads have reached it.

//     Shared Counter: arrived_count tracks how many threads have reached the barrier. It's protected by barrier_mutex.

//     Last Thread's Role: The last thread to arrive (arrived_count == total_threads) has the responsibility of waking up all other waiting threads.

//     pthread_cond_broadcast(): This is used instead of pthread_cond_signal() because all waiting threads need to be woken up when the barrier condition is met, not just one.

//     pthread_cond_wait() and the while Loop: As in Problem 2, the while (arrived_count < total_threads) loop around pthread_cond_wait() is essential. Threads wait while the condition to proceed (all threads arrived) is false. This correctly handles spurious wakeups – if a thread wakes up early, it re-checks the condition and goes back to waiting if necessary.

//     Mutex Lock/Unlock: The mutex must be held when checking/modifying arrived_count and when calling pthread_cond_wait or pthread_cond_broadcast. pthread_cond_wait automatically releases and re-acquires the lock.

//     Single Use vs. Reusable: This implementation is primarily for a single barrier use. Making a barrier reusable often requires adding a generation counter or epoch to prevent race conditions where fast threads loop back to the barrier before slow threads have even left the previous wait.

// This barrier problem tests a slightly more complex coordination scenario than the simple wait/signal, requiring the use of pthread_cond_broadcast and careful handling of the wait condition.