// =============================================================
// ex17 — Fixed with __thread (thread-local storage)
// =============================================================
//
// Adding __thread to the static buffer gives each thread its
// own independent copy. No mutex needed. No race condition.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex17.c -lpthread -o ex17
// Run:      ./ex17  (results are always correct)
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// THREAD-SAFE: each thread has its own buf.
// __thread must appear immediately after static or extern.
static char *to_str(int n)
{
    static __thread char buf[32];
    snprintf(buf, sizeof(buf), "%d", n);
    usleep(100);
    return (buf);
}

static void *worker(void *arg)
{
    int         id = *(int *)arg;
    const char  *result;

    result = to_str(id * 1000);
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