# Phase 08 — Advanced Topics

> Thread attributes, signals in threaded programs, and fork().

---

## Commands introduced in this phase

```c
#include <pthread.h>

// --- Stack size ---
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);

// --- Signals in threaded programs ---
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
int pthread_kill(pthread_t thread, int sig);

// --- Wait synchronously for a signal ---
int sigwait(const sigset_t *set, int *sig);

// --- Fork handlers (run before/after fork in multithreaded code) ---
int pthread_atfork(void (*prepare)(void),
                   void (*parent)(void),
                   void (*child)(void));
```

---

## Thread stack size

Each thread has its own stack. On Linux/x86-64, the default is **8 MB** for
non-main threads. For most programs this is ample, but two situations
require changing it:

- A thread allocates a very large array on the stack (local variable).
- You need thousands of threads and want to reduce memory usage.

```c
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, 1024 * 1024);  // 1 MB stack
pthread_create(&tid, &attr, my_func, NULL);
pthread_attr_destroy(&attr);
```

The minimum stack size is available via `sysconf(_SC_THREAD_STACK_MIN)`.

---

## Signals in multithreaded programs

The POSIX signal model predates threads and was designed for single-threaded
processes. Combining threads and signals requires care.

**Key rules:**

1. When a signal is sent to a process (not to a specific thread), the kernel
   delivers it to **one thread** — whichever one is not blocking it.

2. Each thread has its own **signal mask**. Use `pthread_sigmask()` to control
   which signals a thread blocks (same as `sigprocmask()` but per-thread).

3. To send a signal to a specific thread, use `pthread_kill()`.

**The recommended pattern** for handling asynchronous signals in multithreaded
programs:

```c
// In main(), before creating any threads:
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
sigaddset(&set, SIGTERM);
pthread_sigmask(SIG_BLOCK, &set, NULL);  // block in main thread

// All subsequently created threads inherit this mask.
// Create a dedicated signal-handling thread:
pthread_create(&sig_tid, NULL, signal_thread, &set);
```

```c
// The dedicated signal thread:
static void *signal_thread(void *arg)
{
    sigset_t *set = (sigset_t *)arg;
    int      sig;

    sigwait(set, &sig);  // wait synchronously for a signal
    printf("Received signal %d\n", sig);
    // Handle it safely here — no async-signal-safety constraints.
    return (NULL);
}
```

---

## `fork()` in a multithreaded program

When a multithreaded process calls `fork()`, only the **calling thread** is
replicated in the child. All other threads vanish without running destructors
or cleanup handlers.

This creates serious problems:
- Mutexes held by vanished threads remain locked in the child.
- Thread-specific data allocated by vanished threads is leaked.

**The only safe use of `fork()` in a multithreaded program is to immediately
call `exec()` in the child.** This overwrites the child's memory, eliminating
all the corrupted state.

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex22.c](ex22.c) | Setting and getting thread stack size |
| [ex23.c](ex23.c) | Signal handling with a dedicated signal thread and `sigwait` |
| [ex24.c](ex24.c) | `fork()` in a multithreaded process — the hazards |