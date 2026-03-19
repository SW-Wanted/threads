// =============================================================
// ex23 — Signal handling with a dedicated thread and sigwait
// =============================================================
//
// All threads block SIGINT. A dedicated signal thread calls
// sigwait() and handles the signal synchronously and safely.
//
// Compile:  cc -Wall -Wextra -g ex23.c -lpthread -o ex23
// Run:      ./ex23
//           Press Ctrl+C to send SIGINT.
// =============================================================

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile int running = 1;

static void *signal_handler_thread(void *arg)
{
    sigset_t    *set = (sigset_t *)arg;
    int         sig;

    printf("[sig thread] waiting for SIGINT (Ctrl+C)\n");

    // sigwait() blocks until one of the signals in `set` arrives.
    // Unlike a signal handler, this runs in a normal thread context:
    // we can call any function, use mutexes, printf, etc.
    sigwait(set, &sig);

    printf("[sig thread] received signal %d — shutting down\n", sig);
    running = 0;
    return (NULL);
}

static void *worker(void *arg)
{
    int id = *(int *)arg;

    while (running)
    {
        printf("[worker %d] running\n", id);
        sleep(1);
    }
    printf("[worker %d] exiting\n", id);
    return (NULL);
}

int main(void)
{
    pthread_t   sig_tid;
    pthread_t   workers[2];
    sigset_t    set;
    int         ids[2];

    // Block SIGINT in the main thread BEFORE creating any other threads.
    // All subsequently created threads inherit this mask.
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // The dedicated signal thread receives SIGINT via sigwait.
    pthread_create(&sig_tid, NULL, &signal_handler_thread, &set);

    ids[0] = 1; ids[1] = 2;
    pthread_create(&workers[0], NULL, &worker, &ids[0]);
    pthread_create(&workers[1], NULL, &worker, &ids[1]);

    pthread_join(sig_tid, NULL);
    pthread_join(workers[0], NULL);
    pthread_join(workers[1], NULL);
    printf("[main] all threads finished\n");
    return (0);
}