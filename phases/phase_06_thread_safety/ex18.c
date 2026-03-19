// =============================================================
// ex18 — Fixed with thread-specific data and pthread_once
// =============================================================
//
// pthread_key_create creates a key shared by all threads.
// Each thread stores its own pointer under that key.
// The destructor (free) is called automatically when the thread exits.
// pthread_once ensures the key is created exactly once.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex18.c -lpthread -o ex18
// Run:      ./ex18
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 32

static pthread_key_t    tsd_key;
static pthread_once_t   key_once = PTHREAD_ONCE_INIT;

// Called exactly once, by whichever thread first reaches pthread_once().
static void create_key(void)
{
    // The second argument is the destructor: called with the thread's
    // stored value when the thread exits. Here we free the buffer.
    pthread_key_create(&tsd_key, free);
}

// THREAD-SAFE: each thread allocates and stores its own buffer.
static char *to_str(int n)
{
    char *buf;

    // Ensure the key exists — safe to call from any number of threads.
    pthread_once(&key_once, create_key);

    // Retrieve this thread's buffer (NULL on first call from this thread).
    buf = pthread_getspecific(tsd_key);
    if (buf == NULL)
    {
        // First call from this thread: allocate its private buffer.
        buf = malloc(BUF_SIZE);
        if (!buf)
            return (NULL);
        // Store the pointer so we can retrieve it on the next call.
        pthread_setspecific(tsd_key, buf);
    }

    snprintf(buf, BUF_SIZE, "%d", n);
    usleep(100);
    return (buf);
}

static void *worker(void *arg)
{
    int         id = *(int *)arg;
    const char  *result;

    result = to_str(id * 1000);
    printf("Thread %d got: %s (expected %d)\n", id, result, id * 1000);

    // The destructor (free) will be called automatically when this
    // thread exits via pthread_join / return from the start function.
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

    pthread_key_delete(tsd_key);
    return (0);
}