// =============================================================
// ex24 — fork() in a multithreaded process: the hazards
// =============================================================
//
// After fork(), only the calling thread survives in the child.
// A mutex held by a vanished thread stays locked — forever.
// The safe pattern: exec() immediately after fork().
//
// Compile:  cc -Wall -Wextra -g ex24.c -lpthread -o ex24
// Run:      ./ex24
// =============================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static pthread_mutex_t  mtx = PTHREAD_MUTEX_INITIALIZER;
static volatile int     thread_ready = 0;

// This thread holds the mutex when the main thread calls fork().
static void *holder(void *arg)
{
    (void)arg;
    
    pthread_mutex_lock(&mtx);
    printf("[holder] locked mutex\n");
    thread_ready = 1;
    sleep(10);  // hold the mutex for a long time
    pthread_mutex_unlock(&mtx);
    return (NULL);
}

int main(void)
{
    pthread_t   tid;
    pid_t       pid;

    pthread_create(&tid, NULL, &holder, NULL);

    // Wait until the holder thread has the mutex.
    while (!thread_ready)
        usleep(1000);

    printf("[main] forking while mutex is held by another thread\n");
    pid = fork();

    if (pid == 0)
    {
        // === CHILD PROCESS ===
        // Only the main thread survives here.
        // The holder thread vanished — but it still owns the mutex in
        // the child's copy of memory.
        // This call will block forever: no one will ever unlock mtx.
        printf("[child] trying to lock mutex... (will deadlock)\n");

        // Comment out the line below to see the safe alternative.
        // pthread_mutex_lock(&mtx);   // deadlocks

        // SAFE ALTERNATIVE: exec() immediately after fork().
        // exec() replaces the process image, eliminating the locked mutex.
        printf("[child] exec()ing instead — the safe pattern\n");
        execlp("echo", "echo", "[child exec] success", NULL);
        exit(1);
    }
    else
    {
        // === PARENT PROCESS ===
        int status;
        waitpid(pid, &status, 0);
        printf("[parent] child finished\n");

        pthread_cancel(tid);
        pthread_join(tid, NULL);
        pthread_mutex_destroy(&mtx);
    }
    return (0);
}