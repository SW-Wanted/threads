// =============================================================
// ex05 — The same race, detected by ThreadSanitizer
// =============================================================
//
// The code is identical to ex04. The only difference is the
// compile flags: -fsanitize=thread.
//
// TSan instruments every memory access at compile time.
// At runtime it tracks which thread last wrote to each location.
// When a conflict is detected, it prints a detailed report.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex05.c -lpthread -o ex05
// Run:      ./ex05
//
// You will see output like:
//   WARNING: ThreadSanitizer: data race
//     Write of size 4 at 0x... by thread T2
//     Previous write of size 4 at 0x... by thread T1
//
// TSan has near-zero false positives. If it reports a race, trust it.
// Do NOT use valgrind together with -fsanitize=thread.
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOPS 100000   // fewer loops: TSan is slower, we only need one race
#define NUM_THREADS 2

static int count = 0;

static void *increment(void *arg)
{
    (void)arg;
    
    int i = 0;

    while (i < LOOPS)
    {
        count++;   // TSan will flag this
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