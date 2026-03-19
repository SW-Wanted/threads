// =============================================================
// ex11 — Producer-consumer with a condition variable (correct)
// =============================================================
//
// Same logic as ex10, but the consumer sleeps while waiting.
// The producer wakes it with pthread_cond_signal().
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex11.c -lpthread -o ex11
// Run:      ./ex11
// Observe:  CPU usage is near 0% between production events.
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int              available = 0;
static pthread_mutex_t  mtx  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   cond = PTHREAD_COND_INITIALIZER;

static void *producer(void *arg)
{
    (void)arg;
    
    int i = 0;
    while (i < 5)
    {
        usleep(500000);
        pthread_mutex_lock(&mtx);
        available++;
        printf("[producer] added item. available = %d\n", available);
        pthread_mutex_unlock(&mtx);

        // Signal AFTER releasing the mutex.
        // This wakes the consumer if it is sleeping in pthread_cond_wait.
        // If no thread is waiting, the signal is simply lost — which is
        // fine here because the consumer will check `available` on its
        // next iteration and find it non-zero.
        pthread_cond_signal(&cond);
        i++;
    }
    return (NULL);
}

static void *consumer(void *arg)
{
    (void)arg;

    int consumed = 0;
    while (consumed < 5)
    {
        pthread_mutex_lock(&mtx);

        // ALWAYS use while, never if.
        // Spurious wake-ups are permitted by POSIX.
        // Another consumer could have taken the item before us.
        while (available == 0)
            pthread_cond_wait(&cond, &mtx);
        // pthread_cond_wait atomically:
        //   1. releases mtx
        //   2. sleeps until signalled
        //   3. re-acquires mtx before returning

        available--;
        consumed++;
        printf("[consumer] consumed item. available = %d\n", available);
        pthread_mutex_unlock(&mtx);
    }
    return (NULL);
}

int main(void)
{
    pthread_t prod, cons;
    pthread_create(&prod, NULL, &producer, NULL);
    pthread_create(&cons, NULL, &consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    printf("Done.\n");
    return (0);
}