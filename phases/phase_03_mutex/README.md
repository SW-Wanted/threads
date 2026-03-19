# Phase 03 — Mutex

> Mutual exclusion: only one thread at a time.

---

## Commands introduced in this phase

```c
#include <pthread.h>

// --- Static initialisation (for global/static mutexes) ---
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// --- Dynamic initialisation (required for heap/stack mutexes) ---
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

// --- Locking and unlocking ---
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

// --- Non-blocking lock attempt (returns EBUSY if already locked) ---
int pthread_mutex_trylock(pthread_mutex_t *mutex);

// --- Lock attempt with timeout ---
int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex,
                             const struct timespec *restrict abstime);

// --- Cleanup ---
int pthread_mutex_destroy(pthread_mutex_t *mutex);

// --- Mutex type attributes ---
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
```

All functions return **0 on success** or a **positive error number** on failure.

---

## What is a mutex?

A **mutex** (mutual exclusion) is a lock. Before accessing a shared variable,
a thread locks the mutex. When it is done, it unlocks it. Any other thread
that tries to lock the same mutex while it is held will **block** until the
owner unlocks it.

Think of it as the lock on a bathroom door. Once someone is inside (locked),
others wait in the queue. When the door opens (unlocked), the next person enters.

This guarantees that the **critical section** — the code that accesses the
shared variable — is executed by only one thread at a time.

---

## Static vs dynamic initialisation

For **global or static** mutexes, use the static initialiser:

```c
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
```

For mutexes allocated on the **heap** or **stack**, or for mutexes that need
non-default attributes, use `pthread_mutex_init()`:

```c
pthread_mutex_t *mtx = malloc(sizeof(pthread_mutex_t));
pthread_mutex_init(mtx, NULL);
// ... use mtx ...
pthread_mutex_destroy(mtx);
free(mtx);
```

**Rule:** every mutex initialised with `pthread_mutex_init()` must eventually
be destroyed with `pthread_mutex_destroy()`. Only destroy an **unlocked** mutex.

---

## The critical section pattern

The fundamental pattern for protecting shared data:

```c
pthread_mutex_lock(&mtx);
/* --- critical section begins --- */
shared_variable++;
/* --- critical section ends --- */
pthread_mutex_unlock(&mtx);
```

Keep the critical section **as short as possible**. The longer a thread holds
the mutex, the longer every other thread waits. A mutex held across a `sleep()`
or a network call will starve all other threads.

---

## `pthread_mutex_trylock`

Unlike `pthread_mutex_lock()`, which blocks indefinitely, `pthread_mutex_trylock()`
returns immediately with `EBUSY` if the mutex is already locked:

```c
if (pthread_mutex_trylock(&mtx) == 0)
{
    // we got the lock
    pthread_mutex_unlock(&mtx);
}
else
{
    // mutex was busy, do something else
}
```

Use this sparingly. Polling a mutex in a tight loop wastes CPU and can starve
other threads that are waiting via `pthread_mutex_lock()`.

---

## Mutex types

The default mutex type (`PTHREAD_MUTEX_DEFAULT` / `PTHREAD_MUTEX_NORMAL`) has
undefined behaviour if the same thread tries to lock it twice — on Linux it
deadlocks. For debugging, use `PTHREAD_MUTEX_ERRORCHECK`:

```c
pthread_mutex_t     mtx;
pthread_mutexattr_t attr;

pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
pthread_mutex_init(&mtx, &attr);
pthread_mutexattr_destroy(&attr);
```

With `ERRORCHECK`, attempting to lock a mutex you already own returns `EDEADLK`
instead of freezing the program.

| Type | Double-lock behaviour |
|------|-----------------------|
| `PTHREAD_MUTEX_NORMAL` | Deadlock (undefined behaviour) |
| `PTHREAD_MUTEX_ERRORCHECK` | Returns `EDEADLK` |
| `PTHREAD_MUTEX_RECURSIVE` | Allowed — increments a lock count |

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex07.c](ex07.c) | Fix the ex04 race with a mutex — correct result every time |
| [ex08.c](ex08.c) | Two mutexes protecting two independent resources |
| [ex09.c](ex09.c) | A `print_lock` pattern — serialised output from N threads |

---

## What to observe

In `ex07.c`, run the corrected program multiple times. The result is always
`2 000 000`. Now compare the runtime with `ex04.c` — the mutex version is
noticeably slower. This is the **cost of synchronisation**.

The goal is to protect exactly what needs protecting — no more, no less.

When you are ready, continue to [Phase 04 — Condition variables](../phase_04_condition_variables/).