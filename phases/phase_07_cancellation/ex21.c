// =============================================================
// ex21 — Cleanup handler: unlock a mutex on cancellation
// =============================================================
//
// Without a cleanup handler, a cancelled thread that holds a mutex
// leaves it locked forever, causing all other threads waiting on
// it to deadlock.
//
// pthread_cleanup_push registers a function that runs automatically
// if the thread is cancelled OR calls pthread_exit().
//
// Compile:  cc -Wall -Wextra -g ex21.c -lpthread -o ex21
// Run:      ./ex21
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// This cleanup handler is called when the thread is cancelled
// while it holds the mutex.
static void release_mutex(void *arg)
{
    printf("[cleanup] unlocking mutex\n");
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

static void *worker(void *arg)
{
    pthread_mutex_lock(&mtx);
    printf("[thread] locked mutex\n");

    // Register the cleanup handler.
    // If the thread is cancelled before pthread_cleanup_pop,
    // release_mutex(&mtx) is called automatically.
    pthread_cleanup_push(release_mutex, &mtx);

    printf("[thread] about to sleep (cancellation point)\n");
    sleep(10);  // main will cancel us here

    // If we reach this point normally, pop and execute the handler (1).
    // If cancelled, the handler runs automatically — we never reach here.
    pthread_cleanup_pop(1);
    return (NULL);
}

static void *waiter(void *arg)
{
    // This thread tries to acquire the same mutex.
    // Without the cleanup handler, it would wait forever after the
    // worker is cancelled.
    printf("[waiter] waiting for mutex\n");
    pthread_mutex_lock(&mtx);
    printf("[waiter] acquired mutex\n");
    pthread_mutex_unlock(&mtx);
    return (NULL);
}

int main(void)
{
    pthread_t tw, wt;

    pthread_create(&tw, NULL, &worker, NULL);
    sleep(1);
    pthread_create(&wt, NULL, &waiter, NULL);
    sleep(1);

    printf("[main] cancelling worker\n");
    pthread_cancel(tw);

    pthread_join(tw, NULL);
    pthread_join(wt, NULL);
    pthread_mutex_destroy(&mtx);
    printf("[main] done\n");
    return (0);
}