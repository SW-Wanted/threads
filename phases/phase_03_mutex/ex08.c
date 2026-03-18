// =============================================================
// ex08 — Two mutexes protecting two independent resources
// =============================================================
//
// When you have two unrelated shared resources, you can protect
// them with separate mutexes. Threads only block each other when
// they compete for the *same* resource, not for unrelated ones.
//
// This is more efficient than using a single global mutex for
// everything, which would serialise all threads even when they
// are working on completely different data.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex08.c -lpthread -o ex08
// Run:      ./ex08
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOPS 500000
#define NUM_THREADS 4

// Two independent shared resources, each with its own mutex.
static int              counter_a = 0;
static int              counter_b = 0;
static pthread_mutex_t  mtx_a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  mtx_b = PTHREAD_MUTEX_INITIALIZER;

// Even-numbered threads increment counter_a.
// Odd-numbered  threads increment counter_b.
// Threads working on different counters never block each other.
static void *worker(void *arg)
{
    int index = *(int *)arg;
    int i = 0;

    if (index % 2 == 0)
    {
        while (i < LOOPS)
        {
            pthread_mutex_lock(&mtx_a);
            counter_a++;
            pthread_mutex_unlock(&mtx_a);
            i++;
        }
    }
    else
    {
        while (i < LOOPS)
        {
            pthread_mutex_lock(&mtx_b);
            counter_b++;
            pthread_mutex_unlock(&mtx_b);
            i++;
        }
    }
    return (NULL);
}

int main(void)
{
    pthread_t   threads[NUM_THREADS];
    int         indices[NUM_THREADS];
    int         i;

    i = 0;
    while (i < NUM_THREADS)
    {
        indices[i] = i;
        pthread_create(&threads[i], NULL, &worker, &indices[i]);
        i++;
    }
    i = 0;
    while (i < NUM_THREADS)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    pthread_mutex_destroy(&mtx_a);
    pthread_mutex_destroy(&mtx_b);

    // 2 even-numbered threads (0, 2) each did LOOPS iterations = 2 * LOOPS
    // 2 odd-numbered  threads (1, 3) each did LOOPS iterations = 2 * LOOPS
    printf("counter_a: %d (expected %d)\n", counter_a, (NUM_THREADS / 2) * LOOPS);
    printf("counter_b: %d (expected %d)\n", counter_b, (NUM_THREADS / 2) * LOOPS);
    return (0);
}