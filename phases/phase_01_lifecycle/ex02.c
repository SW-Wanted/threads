// =============================================================
// ex02 — Create N threads, each receives its own index
// =============================================================
//
// What you will see:
//   - Passing per-thread data via a struct
//   - Non-deterministic print order (run multiple times)
//   - Joining all threads in a loop
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex02.c -lpthread -o ex02
// Run:      ./ex02
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 5

// We need to pass a unique index to each thread.
// Using a plain int array and passing &array[i] would work here,
// but in real programs a struct is more flexible.
typedef struct s_args
{
    int index;
}   t_args;

static void *thread_func(void *arg)
{
    t_args  *a = (t_args *)arg;

    printf("Thread %d is running\n", a->index);
    return (NULL);
}

int main(void)
{
    pthread_t   threads[NUM_THREADS];
    t_args      args[NUM_THREADS];
    int         i;
    int         ret;

    // Create all threads first, then join them all.
    // This gives them the best chance to actually run concurrently.
    i = 0;
    while (i < NUM_THREADS)
    {
        args[i].index = i + 1;

        ret = pthread_create(&threads[i], NULL, &thread_func, &args[i]);
        if (ret != 0)
        {
            fprintf(stderr, "pthread_create failed for thread %d: %d\n", i, ret);
            return (1);
        }
        i++;
    }

    // Join every thread.
    // Note: pthread_join can only wait for one specific thread,
    // not for "whoever finishes first".
    i = 0;
    while (i < NUM_THREADS)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    printf("All threads finished.\n");

    // Try this: run the program 5 times in a row.
    // $ for i in $(seq 5); do ./ex02; echo "---"; done
    // You will likely see a different print order each time.
    // This non-determinism is the starting point of Phase 02.
    return (0);
}