// =============================================================
// ex10 — Producer-consumer with busy-waiting (the WRONG way)
// =============================================================
//
// The producer adds items every 500ms.
// The consumer polls in a tight loop, wasting CPU.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex10.c -lpthread -o ex10
// Run:      ./ex10
// Observe:  top or htop — this process will use ~100% of a core.
// Stop:     Ctrl+C
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int              available = 0;
static pthread_mutex_t  mtx = PTHREAD_MUTEX_INITIALIZER;

static void *producer(void *arg)
{
    int i = 0;
    while (i < 5)
    {
        usleep(500000);  // produce one item every 500ms
        pthread_mutex_lock(&mtx);
        available++;
        printf("[producer] added item. available = %d\n", available);
        pthread_mutex_unlock(&mtx);
        i++;
    }
    return (NULL);
}

static void *consumer(void *arg)
{
    int consumed = 0;
    while (consumed < 5)
    {
        // Busy-wait: keep checking until something appears.
        // This loop burns CPU the entire time it is waiting.
        pthread_mutex_lock(&mtx);
        if (available > 0)
        {
            available--;
            consumed++;
            printf("[consumer] consumed item. available = %d\n", available);
        }
        pthread_mutex_unlock(&mtx);
        // No sleep here: pure spin
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
    printf("Done.\n");
    return (0);
}