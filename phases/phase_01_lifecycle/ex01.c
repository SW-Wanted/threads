// =============================================================
// ex01 — Create one thread and wait for it
// =============================================================
//
// What you will see:
//   - pthread_create: launch a new thread
//   - pthread_join:   block until it finishes
//
// Compile:  cc -Wall -Wextra -g -fsanitize=thread ex01.c -lpthread -o ex01
// Run:      ./ex01
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Every thread function must have this exact signature:
//   void *name(void *arg)
//
// The return type is void * so the thread can pass a value
// back to whoever calls pthread_join(). We return NULL here.
void *thread_func(void *arg)
{
    // arg is whatever pointer we passed to pthread_create.
    // We cast it back to the type we know it is.
    int id = *(int *)arg;

    printf("Hello from thread %d\n", id);

    // Returning NULL is equivalent to calling pthread_exit(NULL).
    return (NULL);
}

int main(void)
{
    pthread_t   thread; // stores the ID of the thread we create
    int         id = 1;
    int         ret;

    // pthread_create(
    //   &thread       — where to store the new thread's ID
    //   NULL          — use default attributes
    //   &thread_func  — the function the thread will run
    //   &id           — the argument passed to thread_func
    // )
    // Returns 0 on success, positive error number on failure.
    ret = pthread_create(&thread, NULL, &thread_func, &id);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_create failed: %d\n", ret);
        return (1);
    }

    // Without pthread_join here, main() could reach return(0)
    // before the thread has a chance to print anything.
    // pthread_join blocks until the specified thread finishes.
    ret = pthread_join(thread, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_join failed: %d\n", ret);
        return (1);
    }

    printf("Main thread done.\n");
    return (0);
}
