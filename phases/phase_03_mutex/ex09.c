// =============================================================
// ex09 — The print_lock pattern: serialised output from N threads
// =============================================================
//
// printf() is not atomic. Without a lock, output from multiple
// threads can interleave character by character, producing garbled
// lines like:
//   Thread Thread 2:1: hello
//            hello
//
// The solution is a dedicated mutex — a "print lock" — that every
// thread acquires before printing and releases immediately after.
//
// This is exactly the pattern used in the Dining Philosophers
// project (philo) to prevent log lines from overlapping.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex09.c -lpthread -o ex09
// Run:      ./ex09
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_THREADS 5

static pthread_mutex_t  print_lock = PTHREAD_MUTEX_INITIALIZER;

// Returns current time in milliseconds since program start.
// We need a shared start time so all threads use the same reference.
static long             start_ms;

static long get_time_ms(void)
{
    struct timeval  tv;

    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

// Safe print: acquire lock → print → release lock.
// No other thread can print between the lock and the unlock.
static void safe_print(int thread_id, const char *msg)
{
    pthread_mutex_lock(&print_lock);
    printf("%4ldms  thread %d  %s\n",
           get_time_ms() - start_ms, thread_id, msg);
    pthread_mutex_unlock(&print_lock);
}

static void *worker(void *arg)
{
    int id = *(int *)arg;

    safe_print(id, "started");
    usleep(100000 * id);   // each thread sleeps a different amount
    safe_print(id, "working");
    usleep(50000);
    safe_print(id, "done");
    return (NULL);
}

int main(void)
{
    pthread_t   threads[NUM_THREADS];
    int         indices[NUM_THREADS];
    int         i;

    start_ms = get_time_ms();

    i = 0;
    while (i < NUM_THREADS)
    {
        indices[i] = i + 1;
        pthread_create(&threads[i], NULL, &worker, &indices[i]);
        i++;
    }
    i = 0;
    while (i < NUM_THREADS)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    pthread_mutex_destroy(&print_lock);

    // Every line has a timestamp, a thread ID, and a message.
    // Lines never overlap. This is guaranteed by the print_lock.
    return (0);
}