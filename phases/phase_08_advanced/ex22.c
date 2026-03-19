// =============================================================
// ex22 — Thread stack size: set and verify
// =============================================================
//
// Compile:  cc -Wall -Wextra -g ex22.c -lpthread -o ex22
// Run:      ./ex22
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void *show_stack(void *arg)
{
    size_t  requested = *(size_t *)arg;
    printf("[thread] requested stack size: %zu bytes (%zu KB)\n",
           requested, requested / 1024);
    // We can't directly measure our own stack from inside the thread,
    // but we confirm the attribute was accepted.
    return (NULL);
}

int main(void)
{
    pthread_t       tid;
    pthread_attr_t  attr;
    size_t          stack_size;
    size_t          actual_size;
    long            minimum;

    // The system minimum stack size
    minimum = sysconf(_SC_THREAD_STACK_MIN);
    printf("[main] system minimum stack: %ld bytes\n", minimum);

    // Request a 512 KB stack
    stack_size = 512 * 1024;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stack_size);

    // Verify what was actually set (may be rounded up by the system)
    pthread_attr_getstacksize(&attr, &actual_size);
    printf("[main] actual stack size set: %zu bytes (%zu KB)\n",
           actual_size, actual_size / 1024);

    pthread_create(&tid, &attr, &show_stack, &actual_size);
    pthread_attr_destroy(&attr);
    pthread_join(tid, NULL);
    return (0);
}