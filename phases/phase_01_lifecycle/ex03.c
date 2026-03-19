// =============================================================
// ex03 — Detached thread: the OS cleans up automatically
// =============================================================
//
// What you will see:
//   - pthread_detach: tell the OS to free the thread when done
//   - pthread_self:   a thread gets its own ID
//   - Why you must NOT join a detached thread
//   - A detached thread created via attributes (no separate detach call)
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex03.c -lpthread -o ex03
// Run:      ./ex03
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// --- Example 1: thread detaches itself ---

static void *self_detach(void *arg)
{

    (void)arg;
    // A thread can detach itself using pthread_self().
    // After this call, no other thread should call pthread_join() on it.
    pthread_detach(pthread_self());

    printf("[self_detach] running, id = %lu\n", (unsigned long)pthread_self());
    sleep(1);
    printf("[self_detach] done\n");

    // The OS will free this thread's resources automatically on return.
    return (NULL);
}

// --- Example 2: thread created already detached via attributes ---

static void *attr_detach(void *arg)
{
    (void)arg;
    
    printf("[attr_detach] running, id = %lu\n", (unsigned long)pthread_self());
    sleep(1);
    printf("[attr_detach] done\n");
    return (NULL);
}

int main(void)
{
    pthread_t       t1;
    pthread_t       t2;
    pthread_attr_t  attr;

    // --- Launch a thread that detaches itself ---
    pthread_create(&t1, NULL, &self_detach, NULL);
    // We must NOT call pthread_join(t1) here — it is already detached.

    // --- Launch a thread created detached from the start ---
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t2, &attr, &attr_detach, NULL);
    pthread_attr_destroy(&attr);  // attributes object no longer needed
    // We must NOT call pthread_join(t2) either.

    // Give the detached threads time to finish before main() exits.
    // In a real program you would use a different mechanism (e.g. a semaphore)
    // to know when detached threads are done.
    sleep(2);

    printf("Main thread done.\n");

    // Key point:
    // - Joinable thread (default): you MUST join or detach it.
    // - Detached thread: resources freed automatically on termination.
    // - Calling pthread_join on a detached thread is undefined behaviour.
    return (0);
}