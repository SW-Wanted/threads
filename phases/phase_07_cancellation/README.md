# Phase 07 — Thread Cancellation

> Stopping a thread from the outside — safely.

---

## Commands introduced in this phase

```c
#include <pthread.h>

// --- Send a cancellation request to a thread ---
int pthread_cancel(pthread_t thread);

// --- Control whether the calling thread can be cancelled ---
int pthread_setcancelstate(int state, int *oldstate);
//   state: PTHREAD_CANCEL_ENABLE (default) | PTHREAD_CANCEL_DISABLE

// --- Control when cancellation takes effect ---
int pthread_setcanceltype(int type, int *oldtype);
//   type: PTHREAD_CANCEL_DEFERRED (default) | PTHREAD_CANCEL_ASYNCHRONOUS

// --- Insert an explicit cancellation point in compute-bound code ---
void pthread_testcancel(void);

// --- Register / remove cleanup handlers ---
void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);
```

`pthread_cancel()` returns 0 on success or a positive error number on failure.
When a cancelled thread is joined, `pthread_join()` stores `PTHREAD_CANCELED`
in its `retval` argument.

---

## How cancellation works

`pthread_cancel()` sends a **cancellation request** to the target thread.
The request does not terminate the thread immediately — what happens next
depends on the thread's **cancellation state** and **cancellation type**.

```
pthread_cancel(tid)
       │
       ▼
Is PTHREAD_CANCEL_ENABLE?
   No  → request pending until re-enabled
   Yes →
       Is PTHREAD_CANCEL_DEFERRED (default)?
           Yes → thread terminates at the next cancellation point
           No  → thread terminates immediately (async — dangerous)
```

---

## Cancellation points

A **cancellation point** is a system call where a pending cancellation request
is acted upon. Common cancellation points include: `sleep()`, `usleep()`,
`read()`, `write()`, `pthread_cond_wait()`, `printf()` (maybe), and many others.

For code that never calls any of these (a pure compute loop), add explicit
points with `pthread_testcancel()`:

```c
for (long i = 0; i < 1e9; i++)
{
    compute(i);
    if (i % 10000 == 0)
        pthread_testcancel();  // honour pending cancellation here
}
```

---

## Cleanup handlers

When a thread is cancelled (or calls `pthread_exit()`), its stack of
**cleanup handlers** is executed in reverse registration order.
Use them to release resources (unlock mutexes, free memory) that would
otherwise be leaked.

```c
static void unlock_on_cancel(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

void *my_thread(void *arg)
{
    pthread_mutex_lock(&mtx);
    pthread_cleanup_push(unlock_on_cancel, &mtx);  // register handler

    do_something_cancellable();  // if cancelled here, mtx is unlocked

    pthread_cleanup_pop(1);  // pop and execute (1) or just pop (0)
    // pthread_mutex_unlock(&mtx) is called by cleanup_pop(1)
    return (NULL);
}
```

`pthread_cleanup_push` and `pthread_cleanup_pop` must be paired in the
**same lexical block** — on Linux they expand to `{` and `}` respectively.

---

## When to use (and when not to)

Cancellation is useful for stopping a long-running worker thread (e.g. a
download, a search) when the user no longer needs the result.

Avoid **asynchronous** cancellation (`PTHREAD_CANCEL_ASYNCHRONOUS`) in almost
all cases. The thread can be cancelled between any two instructions, including
inside `malloc()`, which can corrupt the heap. Deferred cancellation is safe
because it only triggers at well-defined points.

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex19.c](ex19.c) | Cancel a looping thread with `pthread_cancel` |
| [ex20.c](ex20.c) | `PTHREAD_CANCEL_DISABLE` protects a critical section |
| [ex21.c](ex21.c) | Cleanup handler unlocks a mutex on cancellation |