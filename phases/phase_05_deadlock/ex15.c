// =============================================================
// ex15 — Fixed: try-and-back-off with pthread_mutex_trylock
// =============================================================
//
// When the second lock fails (EBUSY), release the first and retry.
// This breaks the hold-and-wait condition.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex15.c -lpthread -o ex15
// Run:      ./ex15
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

// Acquire both mutexes without deadlock.
// Lock the first with a blocking call.
// Try the second with a non-blocking call.
// If it fails, release the first and retry after a short delay.
static void lock_both(pthread_mutex_t *first, pthread_mutex_t *second,
                      const char *name)
{
    int retries = 0;

    while (1)
    {
        pthread_mutex_lock(first);
        if (pthread_mutex_trylock(second) == 0)
            break;                  // got both locks
        pthread_mutex_unlock(first); // back off
        retries++;
        printf("[%s] back-off retry %d\n", name, retries);
        usleep(1000);               // wait before retrying
    }
}

static void *thread_a(void *arg)
{
    lock_both(&m1, &m2, "A");
    printf("[A] holds both locks\n");
    sleep(1);
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    printf("[A] released both\n");
    return (NULL);
}

static void *thread_b(void *arg)
{
    // Thread B tries in the opposite order — the pattern that caused
    // deadlock in ex13. But now trylock breaks the hold-and-wait.
    lock_both(&m2, &m1, "B");
    printf("[B] holds both locks\n");
    sleep(1);
    pthread_mutex_unlock(&m1);
    pthread_mutex_unlock(&m2);
    printf("[B] released both\n");
    return (NULL);
}

int main(void)
{
    pthread_t ta, tb;
    pthread_create(&ta, NULL, &thread_a, NULL);
    pthread_create(&tb, NULL, &thread_b, NULL);
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);
    pthread_mutex_destroy(&m1);
    pthread_mutex_destroy(&m2);
    printf("Done.\n");
    return (0);
}