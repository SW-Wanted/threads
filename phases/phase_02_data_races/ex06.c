// =============================================================
// ex06 — The same race, detected by Helgrind and DRD
// =============================================================
//
// Helgrind and DRD are Valgrind tools that detect data races
// without needing any special compile flags.
// They are slower than TSan but can be added to any binary
// compiled with -g (debug symbols).
//
// Compile:  cc -Wall -Wextra -g ex06.c -lpthread -o ex06
//
// Run with Helgrind:
//   valgrind --tool=helgrind ./ex06
//
// Run with DRD:
//   valgrind --tool=drd ./ex06
//
// Helgrind focuses on:  data races, lock-order violations
// DRD focuses on:       data races, lock contention analysis
//
// Both will report the race on `count`.
// Compare the output format with what TSan produced in ex05.
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOPS 50000    // keep it short: valgrind is much slower than native
#define NUM_THREADS 2

static int count = 0;

static void *increment(void *arg)
{
    (void)arg;
    
    int i = 0;

    while (i < LOOPS)
    {
        count++;   // Helgrind and DRD will both flag this
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

    printf("Final count: %d (expected %d)\n", count, LOOPS * NUM_THREADS);
    return (0);
}