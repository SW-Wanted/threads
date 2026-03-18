// =============================================================
// ex12 — pthread_cond_broadcast: waking ALL waiting threads
// =============================================================
//
// Three consumers are waiting. One producer adds 3 items at once
// and broadcasts. All consumers wake up, but only one per item
// gets to consume (the others go back to sleep via the while loop).
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex12.c -lpthread -o ex12
// Run:      ./ex12
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_CONSUMERS 3
#define ITEMS_PER_BATCH 3

static int              available = 0;
static int              done = 0;
static pthread_mutex_t  mtx  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   cond = PTHREAD_COND_INITIALIZER;

static void *consumer(void *arg)
{
    int id = *(int *)arg;

    while (1)
    {
        pthread_mutex_lock(&mtx);

        // Sleep until there is something to consume or we are done.
        while (available == 0 && !done)
            pthread_cond_wait(&cond, &mtx);

        if (available > 0)
        {
            available--;
            printf("[consumer %d] consumed. available = %d\n", id, available);
            pthread_mutex_unlock(&mtx);
        }
        else
        {
            // done == 1 and nothing left to consume
            pthread_mutex_unlock(&mtx);
            break;
        }
    }
    return (NULL);
}

static void *producer(void *arg)
{
    // Wait a moment to let all consumers reach pthread_cond_wait.
    sleep(1);

    pthread_mutex_lock(&mtx);
    available = ITEMS_PER_BATCH;
    printf("[producer] added %d items\n", ITEMS_PER_BATCH);
    pthread_mutex_unlock(&mtx);

    // broadcast wakes ALL waiting consumers at once.
    // Each one will re-check `available` inside the while loop.
    // Only those that find available > 0 will consume.
    pthread_cond_broadcast(&cond);

    sleep(1);

    // Signal that production is over.
    pthread_mutex_lock(&mtx);
    done = 1;
    pthread_mutex_unlock(&mtx);
    pthread_cond_broadcast(&cond);  // wake sleeping consumers so they can exit

    return (NULL);
}

int main(void)
{
    pthread_t   consumers[NUM_CONSUMERS];
    pthread_t   prod;
    int         ids[NUM_CONSUMERS];
    int         i;

    i = 0;
    while (i < NUM_CONSUMERS)
    {
        ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, &consumer, &ids[i]);
        i++;
    }
    pthread_create(&prod, NULL, &producer, NULL);

    pthread_join(prod, NULL);
    i = 0;
    while (i < NUM_CONSUMERS)
    {
        pthread_join(consumers[i], NULL);
        i++;
    }

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    printf("Done.\n");
    return (0);
}