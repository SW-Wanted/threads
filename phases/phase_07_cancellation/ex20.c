// =============================================================
// ex20 — PTHREAD_CANCEL_DISABLE protects a critical section
// =============================================================
//
// During the critical section (update of a data structure),
// cancellation is disabled. This ensures the structure is never
// left in a partially updated state.
//
// Compile:  cc -Wall -Wextra -g ex20.c -lpthread -o ex20
// Run:      ./ex20
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int  data_a = 0;
static int  data_b = 0;

static void *worker(void *arg)
{
    (void)arg;
    
    int i = 0;

    while (i < 10)
    {
        int old_state;

        // Disable cancellation before the critical section.
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);

        // Both fields must be updated together atomically.
        // If cancelled mid-update, data_a and data_b would be inconsistent.
        data_a = i;
        usleep(1000);   // simulate work between the two updates
        data_b = i;

        // Restore the previous cancellation state.
        pthread_setcancelstate(old_state, NULL);

        // Cancellation can now be delivered here (a safe point).
        pthread_testcancel();

        printf("[thread] updated: data_a=%d data_b=%d\n", data_a, data_b);
        i++;
    }
    return (NULL);
}

int main(void)
{
    pthread_t   tid;
    void        *retval;

    pthread_create(&tid, NULL, &worker, NULL);
    usleep(5000);  // let a few iterations run
    printf("[main] cancelling\n");
    pthread_cancel(tid);
    pthread_join(tid, &retval);

    // data_a and data_b are always equal — the critical section was atomic.
    printf("[main] final: data_a=%d data_b=%d (always equal)\n",
           data_a, data_b);
    return (0);
}