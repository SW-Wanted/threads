// =============================================================
// ex04 — A live data race: the counter that loses increments
// =============================================================
//
// This program is INTENTIONALLY BROKEN.
// Two threads increment a shared counter 1 000 000 times each.
// The expected result is 2 000 000.
// The actual result will almost always be less.
//
// Do NOT compile with -fsanitize=thread here — you want to see
// the wrong result first, without the sanitizer interfering.
//
// Compile:  cc -Wall -Wextra -g ex04.c -lpthread -o ex04
// Run:      ./ex04
//           Run it several times. The result changes every time.
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOPS 1000000
#define NUM_THREADS 2

// This variable is shared between all threads.
// It is the source of the race condition.
static int count = 0;

static void *increment(void *arg)
{
    int i = 0;

    while (i < LOOPS)
    {
        // count++ expands to three CPU instructions:
        //   1. load:  register = count
        //   2. add:   register = register + 1
        //   3. store: count = register
        //
        // If Thread 2 executes step 1 between Thread 1's steps 1 and 3,
        // both threads read the same value and one increment is lost.
        count++;
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

    printf("Expected: %d\n", LOOPS * NUM_THREADS);
    printf("Actual:   %d\n", count);

    if (count == LOOPS * NUM_THREADS)
        printf("Result: correct (got lucky this time)\n");
    else
        printf("Result: WRONG — lost %d increments\n",
               LOOPS * NUM_THREADS - count);
    return (0);
}