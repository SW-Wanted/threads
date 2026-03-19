// =============================================================
// ex19 — Cancel a looping thread with pthread_cancel
// =============================================================
//
// A thread loops forever, sleeping 1 second each iteration.
// The main thread cancels it after 3 seconds.
// pthread_join returns PTHREAD_CANCELED.
//
// Compile:  cc -Wall -Wextra -g ex19.c -lpthread -o ex19
// Run:      ./ex19
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void *looping_thread(void *arg)
{
    (void)arg;
    
    int j = 1;

    printf("[thread] started\n");
    while (1)
    {
        printf("[thread] loop %d\n", j++);
        sleep(1);  // sleep() is a cancellation point
    }
    return (NULL);  // never reached
}

int main(void)
{
    pthread_t   tid;
    void        *retval;

    pthread_create(&tid, NULL, &looping_thread, NULL);

    sleep(3);  // let the thread run for 3 seconds

    printf("[main] sending cancellation request\n");
    pthread_cancel(tid);

    // Wait for the thread to process the cancellation.
    pthread_join(tid, &retval);

    if (retval == PTHREAD_CANCELED)
        printf("[main] thread was cancelled successfully\n");
    else
        printf("[main] unexpected return value\n");

    return (0);
}