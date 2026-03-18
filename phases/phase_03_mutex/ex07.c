// =============================================================
// ex07 — Fix the data race from ex04 with a mutex
// =============================================================
//
// The code is structurally identical to ex04.
// The only additions are:
//   - a mutex declaration
//   - pthread_mutex_lock / unlock around count++
//   - pthread_mutex_destroy at the end
//
// The result is always 2 000 000, regardless of how many times
// you run it. The mutex guarantees that only one thread at a
// time executes the critical section.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex07.c -lpthread -o ex07
// Run:      ./ex07
//           Run it 10 times: the result never changes.
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOPS 1000000
#define NUM_THREADS 2

static int              count = 0;

// PTHREAD_MUTEX_INITIALIZER is the static initialiser for a mutex.
// It is equivalent to calling pthread_mutex_init(&mtx, NULL),
// but only valid for global or static variables.
static pthread_mutex_t  mtx = PTHREAD_MUTEX_INITIALIZER;

static void *increment(void *arg)
{
    int i = 0;

    while (i < LOOPS)
    {
        // Lock: if another thread holds the mutex, this call blocks.
        pthread_mutex_lock(&mtx);

        // --- critical section ---
        // Only one thread at a time reaches this line.
        count++;
        // --- end critical section ---

        // Unlock: the next waiting thread can now proceed.
        pthread_mutex_unlock(&mtx);
        i++;
    }
    return (NULL);
}

int main(void)
{
    pthread_t   threads[NUM_THREADS];
    int         i;

    i = 0;
    while (i < NUM_THREADS)
    {
        pthread_create(&threads[i], NULL, &increment, NULL);
        i++;
    }
    i = 0;
    while (i < NUM_THREADS)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    // Destroy the mutex when it is no longer needed.
    // Only destroy an unlocked mutex.
    // (For PTHREAD_MUTEX_INITIALIZER mutexes this is optional but good practice.)
    pthread_mutex_destroy(&mtx);

    printf("Expected: %d\n", LOOPS * NUM_THREADS);
    printf("Actual:   %d\n", count);

    if (count == LOOPS * NUM_THREADS)
        printf("Result: correct — mutex worked\n");
    else
        printf("Result: WRONG — something is broken\n");
    return (0);
}