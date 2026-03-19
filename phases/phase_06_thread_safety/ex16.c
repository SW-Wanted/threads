// =============================================================
// ex16 — A non-thread-safe function: shared static buffer
// =============================================================
//
// two_digit_str() converts a number to a string using a static buffer.
// When two threads call it simultaneously, they share that buffer
// and corrupt each other's result.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex16.c -lpthread -o ex16
// Run:      ./ex16  (results will be garbled)
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// NOT THREAD-SAFE: static buffer shared by all threads.
static char *to_str(int n)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "%d", n);
    usleep(100);  // exaggerate the race window
    return (buf);
}

static void *worker(void *arg)
{
    int         id = *(int *)arg;
    const char  *result;

    result = to_str(id * 1000);
    // By the time we print, another thread may have overwritten buf.
    printf("Thread %d got: %s (expected %d)\n", id, result, id * 1000);
    return (NULL);
}

int main(void)
{
    pthread_t   threads[4];
    int         ids[4];
    int         i;

    i = 0;
    while (i < 4)
    {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, &worker, &ids[i]);
        i++;
    }
    i = 0;
    while (i < 4)
    {
        pthread_join(threads[i], NULL);
        i++;
    }
    return (0);
}