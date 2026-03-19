// =============================================================
// ex13 — A deliberate deadlock: the program hangs forever
// =============================================================
//
// Thread A locks m1 then tries m2.
// Thread B locks m2 then tries m1.
// Both block. Neither can proceed.
//
// Compile:  cc -Wall -Wextra -g ex13.c -lpthread -o ex13
// Run:      ./ex13        (hangs — kill with Ctrl+C)
// Detect:   valgrind --tool=helgrind ./ex13
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

static void *thread_a(void *arg)
{
    (void)arg;

    printf("[A] locking m1\n");
    pthread_mutex_lock(&m1);
    printf("[A] locked m1, sleeping 1s\n");
    sleep(1);  // give thread B time to lock m2
    printf("[A] trying to lock m2 — will block forever\n");
    pthread_mutex_lock(&m2);  // DEADLOCK: B holds m2
    printf("[A] locked m2 (never reached)\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    return (NULL);
}

static void *thread_b(void *arg)
{
    (void)arg;
    
    printf("[B] locking m2\n");
    pthread_mutex_lock(&m2);
    printf("[B] locked m2, sleeping 1s\n");
    sleep(1);  // give thread A time to lock m1
    printf("[B] trying to lock m1 — will block forever\n");
    pthread_mutex_lock(&m1);  // DEADLOCK: A holds m1
    printf("[B] locked m1 (never reached)\n");
    pthread_mutex_unlock(&m1);
    pthread_mutex_unlock(&m2);
    return (NULL);
}

int main(void)
{
    pthread_t ta, tb;
    pthread_create(&ta, NULL, &thread_a, NULL);
    pthread_create(&tb, NULL, &thread_b, NULL);
    pthread_join(ta, NULL);  // hangs here
    pthread_join(tb, NULL);
    pthread_mutex_destroy(&m1);
    pthread_mutex_destroy(&m2);
    printf("Done. (never reached)\n");
    return (0);
}