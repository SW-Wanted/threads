// =============================================================
// ex14 — Fixed: enforce a fixed lock order (m1 always before m2)
// =============================================================
//
// Both threads now acquire locks in the same order: m1 then m2.
// Circular wait is impossible. Deadlock cannot occur.
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex14.c -lpthread -o ex14
// Run:      ./ex14
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

// Both threads follow the rule: always lock m1 before m2.
// When thread A holds m1 and waits for m2, thread B is also
// waiting for m1 (not holding m2 yet). Thread A gets m2, finishes,
// releases both. Then thread B can proceed.

static void *thread_a(void *arg)
{
    (void)arg;
    
    printf("[A] locking m1 then m2\n");
    pthread_mutex_lock(&m1);
    sleep(1);
    pthread_mutex_lock(&m2);
    printf("[A] holds both locks\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    printf("[A] released both\n");
    return (NULL);
}

static void *thread_b(void *arg)
{
    (void)arg;

    printf("[B] locking m1 then m2\n");
    pthread_mutex_lock(&m1);  // same order as A
    sleep(1);
    pthread_mutex_lock(&m2);
    printf("[B] holds both locks\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
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