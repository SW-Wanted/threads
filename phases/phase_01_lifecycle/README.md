# Phase 01 — Thread Lifecycle

> Creating threads, waiting for them, and letting them go.

---

## Commands introduced in this phase

```c
#include <pthread.h>

// Create a new thread
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);

// Wait for a thread to finish (join it)
int pthread_join(pthread_t thread, void **retval);

// Detach a thread (let the OS clean it up automatically)
int pthread_detach(pthread_t thread);

// Get the calling thread's own ID
pthread_t pthread_self(void);

// Compare two thread IDs
int pthread_equal(pthread_t t1, pthread_t t2);

// Terminate the calling thread explicitly
void pthread_exit(void *retval);

// --- Thread attributes (optional, used to configure threads at creation) ---
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
```

All functions except `pthread_exit()` and `pthread_self()` return **0 on success**
or a **positive error number** on failure. They do not set `errno`.

---

## Creating a thread

`pthread_create()` launches a new thread that begins executing `start_routine`.

```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);
```

| Parameter | Purpose |
|-----------|---------|
| `thread` | Receives the ID of the new thread |
| `attr` | Thread attributes — pass `NULL` for defaults |
| `start_routine` | The function the thread will execute |
| `arg` | A single `void *` argument passed to `start_routine` |

The `start_routine` must always have this exact signature:

```c
void *my_function(void *arg)
{
    // ...
    return NULL;
}
```

If you need to pass multiple values, put them in a struct and pass a pointer to it.

---

## Joining a thread

By default, a thread is **joinable**. This means another thread can call
`pthread_join()` to wait for it to finish and retrieve its return value.

```c
int pthread_join(pthread_t thread, void **retval);
```

If you create a thread and never join it (and never detach it), its resources
are not freed when it finishes. This is a **thread resource leak** — similar
to a zombie process.

**Rule:** every joinable thread must eventually be either joined or detached.

---

## Detaching a thread

If you do not care about a thread's return value and don't want to call
`pthread_join()`, you can detach it:

```c
pthread_detach(pthread_self());   // thread detaches itself
pthread_detach(tid);              // another thread detaches it
```

Once detached, a thread cannot be joined. Its resources are freed automatically
when it terminates.

A thread can also be created already detached using thread attributes:

```c
pthread_t thr;
pthread_attr_t attr;

pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thr, &attr, my_function, NULL);
pthread_attr_destroy(&attr);   // attributes no longer needed after create
```

---

## Thread termination

A thread finishes when any of the following happens:

- Its `start_routine` returns
- It calls `pthread_exit()`
- It is cancelled by another thread (see [Phase 07](../phase_07_cancellation/))
- Any thread calls `exit()` — which terminates the entire process immediately

`pthread_exit()` is equivalent to returning from `start_routine`, with one
important difference: it can be called from *any* function in the call stack,
not just the top-level thread function.

If `main()` calls `pthread_exit()` instead of returning or calling `exit()`,
the other threads continue to run until they finish.

---

## Thread IDs

Every thread has a unique ID of type `pthread_t`. Retrieve your own with:

```c
pthread_t my_id = pthread_self();
```

Because `pthread_t` may be a struct on some systems, you cannot compare IDs
with `==`. Use `pthread_equal()` instead:

```c
if (pthread_equal(tid, pthread_self()))
    printf("same thread\n");
```

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex01.c](ex01.c) | One thread, `pthread_create` and `pthread_join` |
| [ex02.c](ex02.c) | N threads, each receives its own index as argument |
| [ex03.c](ex03.c) | `pthread_detach` — the thread cleans up after itself |

---

## What to observe

When you run `ex02.c` with several threads, notice that the **print order is
not guaranteed**. The OS scheduler decides when each thread runs. Run the
program several times — the order may change. This non-determinism is a
fundamental property of concurrent execution, and it is the root of all the
problems you will encounter in the next phases.

When you are ready, continue to [Phase 02 — Data Races](../phase_02_data_races/).
