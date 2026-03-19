# Phase 04 — Condition Variables

> Waiting for *something to happen*, not just for a lock.

---

## Commands introduced in this phase

```c
#include <pthread.h>

// --- Static initialisation ---
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// --- Dynamic initialisation ---
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

// --- Wait: atomically unlocks mutex and sleeps until signalled ---
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

// --- Wait with a timeout ---
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                            const struct timespec *abstime);

// --- Wake up ONE waiting thread ---
int pthread_cond_signal(pthread_cond_t *cond);

// --- Wake up ALL waiting threads ---
int pthread_cond_broadcast(pthread_cond_t *cond);

// --- Cleanup ---
int pthread_cond_destroy(pthread_cond_t *cond);
```

All functions return **0 on success** or a **positive error number** on failure.

---

## The problem with mutexes alone

A mutex tells threads *when they can enter* a critical section.
But sometimes a thread needs to wait for a specific *state* — not just for access.

Consider a producer-consumer system:

```c
// Consumer: spin-waiting (WRONG — wastes CPU)
for (;;)
{
    pthread_mutex_lock(&mtx);
    while (available > 0)
    {
        consume();
        available--;
    }
    pthread_mutex_unlock(&mtx);
    // The thread keeps looping even when there is nothing to consume.
    // This burns 100% CPU doing nothing useful.
}
```

The consumer spends most of its time locked in a tight loop checking
whether there is something to do. This is called **busy-waiting** and it is
wasteful. A condition variable allows the consumer to *sleep* until a producer
wakes it.

---

## How a condition variable works

A condition variable always has an **associated mutex**. The pattern is:

**Consumer (waiter):**
```c
pthread_mutex_lock(&mtx);
while (available == 0)              // always use while, not if
    pthread_cond_wait(&cond, &mtx); // atomically: unlock mutex + sleep
                                    // on wake-up: relock mutex
// Now: available > 0 and we hold the mutex
consume();
available--;
pthread_mutex_unlock(&mtx);
```

**Producer (signaller):**
```c
pthread_mutex_lock(&mtx);
available++;
pthread_mutex_unlock(&mtx);
pthread_cond_signal(&cond);         // wake up one waiting thread
```

`pthread_cond_wait()` does three things atomically:
1. Releases the mutex
2. Puts the thread to sleep
3. Re-acquires the mutex when awakened

The release and sleep happen **atomically** — there is no window where another
thread can signal the condition variable before this thread is sleeping.

---

## Why `while` and not `if`

The predicate must be re-checked after `pthread_cond_wait()` returns because:

1. **Another thread may have consumed the item first.** If multiple threads
   are waiting, all are woken by `pthread_cond_broadcast()`. Only one finds
   work to do; the others must go back to sleep.

2. **Spurious wake-ups.** On some systems, `pthread_cond_wait()` can return
   even when no one signalled it. The POSIX standard explicitly permits this.

**Always wrap `pthread_cond_wait()` in a `while` loop:**

```c
while (condition_is_not_met)
    pthread_cond_wait(&cond, &mtx);
```

---

## `signal` vs `broadcast`

| Function | Wakes up |
|----------|---------|
| `pthread_cond_signal()` | At least one waiting thread |
| `pthread_cond_broadcast()` | All waiting threads |

Use `signal` when exactly one thread needs to act on the change and all
waiting threads are interchangeable (they do the same job).

Use `broadcast` when multiple threads need to re-evaluate the condition,
or when different threads are waiting for different predicates on the same
condition variable.

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex10.c](ex10.c) | Producer-consumer with busy-waiting (the wrong way) |
| [ex11.c](ex11.c) | Producer-consumer with a condition variable (correct) |
| [ex12.c](ex12.c) | `pthread_cond_broadcast`: waking all waiting threads |

---

## What to observe

Compare CPU usage between `ex10.c` and `ex11.c` using `top` or `htop` while
they run. The busy-waiting version will consume near 100% of a CPU core.
The condition variable version will show near 0% between production events.